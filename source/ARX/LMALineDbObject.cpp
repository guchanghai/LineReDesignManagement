// ------------------------------------------------
//                  LineManagementAssistant
// Copyright 2012-2013, Chengyong Yang & Changhai Gu. 
//               All rights reserved.
// ------------------------------------------------
//	ArxWrapper.h
//	written by Changhai Gu
// ------------------------------------------------
// $File:\\LineManageAssitant\main\source\ARX\LMALineDbObject.cpp $
// $Author: Changhai Gu $
// $DateTime: 2013/1/12 06:13:00
// $Revision: #1 $
// ------------------------------------------------

#include <ArxWrapper.h>
#include <ArxCustomObject.h>
#include <LMAUtils.h>
#include <LineEntryData.h>
#include <gelnsg3d.h>
#include <LineConfigDataManager.h>
#include <GlobalDataConfig.h>

using namespace com::guch::assistant::config;
using namespace com::guch::assistant::data;

namespace com
{

namespace guch
{

namespace assistant
{

namespace arx
{

void LMADbObjectManager::RegisterClass()
{
	LineDBEntry::rxInit();
	LMALineDbObject::rxInit();
	LMASafeLineDbObject::rxInit();
}

void LMADbObjectManager::UnRegisterClass()
{
	deleteAcRxClass(LineDBEntry::desc());
	deleteAcRxClass(LMALineDbObject::desc());
	deleteAcRxClass(LMASafeLineDbObject::desc());
}

ACRX_DXF_DEFINE_MEMBERS(LMALineDbObject, AcDb3dSolid, 
AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent, 
0,
    LMALineDbObject, LMA);

/// <summary>
/// Initializes a new instance of the <see cref="LMALineDbObject"/> class.
/// </summary>
LMALineDbObject::LMALineDbObject():
	mpPointInfo( NULL )
	,mRadius(0)
	,mWidth(0)
	,mLength(0)
{
}

	/// <summary>
/// Initializes a new instance of the <see cref="LMALineDbObject" /> class.
/// </summary>
/// <param name="pPointInfo">The p point info.</param>
LMALineDbObject::LMALineDbObject( PointDBEntityCollection* pPointInfo)
: mpPointInfo( pPointInfo )
, mRadius(0)
, mWidth(0)
, mLength(0)
{
	Init();
}

 /// <summary>
/// Inits this instance.
/// </summary>
/// <returns></returns>
Acad::ErrorStatus LMALineDbObject::Init()
{
	if( mpPointInfo == NULL ||
		mpPointInfo->mCategoryData == NULL )
	{
		acutPrintf(L"\n配置信息不合法");
		return Acad::eInvalidInput;
	}

	//圆形或矩形
	if( mpPointInfo->mCategoryData->mShape == GlobalData::LINE_SHAPE_CIRCLE )
	{
		acdbDisToF(mpPointInfo->mCategoryData->mRadius.c_str(), -1, &mRadius);

		//直径的单位是毫米，而距离的单位是米
		mRadius = mRadius / 1000;
	}
	else if ( mpPointInfo->mCategoryData->mShape == GlobalData::LINE_SHAPE_SQUARE )
	{
		acdbDisToF(mpPointInfo->mCategoryData->mHeight.c_str(), -1, &mLength);
		acdbDisToF(mpPointInfo->mCategoryData->mWidth.c_str(), -1, &mWidth);

		//直径的单位是毫米，而距离的单位是米
		mLength = mLength / 1000;
		mWidth = mWidth / 1000;
	}

	return CreateDBObject();
}

/// <summary>
/// Creates the pipe.
/// </summary>
/// <returns></returns>
Acad::ErrorStatus LMALineDbObject::CreateDBObject()
{
	acutPrintf(L"\n开始绘制实体");

	const AcGePoint3d& startPoint = mpPointInfo->mStartPoint;
	const AcGePoint3d& endPoint = mpPointInfo->mEndPoint;

	//得到线段的长度
	double height = startPoint.distanceTo( endPoint );
	if( height < 0.01 )
	{
		acutPrintf(L"\n高度小于1毫米，暂不考虑这样的实体！",mRadius,height);
		return Acad::eInvalidInput;
	}

	if( mpPointInfo->mCategoryData->mShape == GlobalData::LINE_SHAPE_CIRCLE )
	{
		acutPrintf(L"\n绘制半径为【%0.2lf】长为【%0.2lf】的圆柱",mRadius,height);

		//绘制圆柱体
		this->createFrustum(height,mRadius,mRadius,mRadius);
	}
	else if (  mpPointInfo->mCategoryData->mShape == GlobalData::LINE_SHAPE_SQUARE )
	{
		acutPrintf(L"\n绘制宽【%0.2lf】高【%0.2lf】长【%0.2lf】的方柱体",mLength, mWidth, height);

		//绘制圆柱体
		this->createBox(mLength,mWidth,height);
	}

	//得到线段与Z轴的垂直向量
	AcGeVector3d line3dVector(endPoint.x - startPoint.x, endPoint.y - startPoint.y, endPoint.z - startPoint.z);
	AcGeVector3d rotateVctor = line3dVector.crossProduct(AcGeVector3d::kZAxis);

	//得到旋转的角度
	double angle = -line3dVector.angleTo(AcGeVector3d::kZAxis);
	acutPrintf(L"\n得到旋转角度【%lf】",angle);

	//进行旋转
	AcGeMatrix3d rotateMatrix = AcGeMatrix3d::rotation( angle, rotateVctor, AcGePoint3d::kOrigin);
	transformBy(rotateMatrix);
	
	//得到线段的中心点
	AcGePoint3d center( startPoint.x + endPoint.x, startPoint.y + endPoint.y, startPoint.z + endPoint.z); 
	center /= 2;
	acutPrintf(L"\n得到中心点【%0.2lf】【%0.2lf】【%0.2lf】",center.x,center.y,center.z);

	//进行偏移
	AcGeMatrix3d moveMatrix;
	moveMatrix.setToTranslation(AcGeVector3d(center.x,center.y,center.z));

	//最终成型
	transformBy(moveMatrix);

	return Acad::eOk;
}

/// <summary>
/// Creates the dimensions.
/// </summary>
/// <returns></returns>
Acad::ErrorStatus LMALineDbObject::CreateDimensions()
{
	/*
	//创建标注体
    AcDbAlignedDimension* mAlignedDim = new AcDbAlignedDimension;

	//设置标注的文字
	AcDbText* mLineDim = new AcDbText;
	{
		CString textSeq;
		textSeq.Format(L"【%d】",mpPointInfo->mSequenceNO);
		mLineDim->setTextString(textSeq);
	}

	//得到线段长度
	double length = mStartPoint.distanceTo(mpmEndPoint);
	if( length < 0.1 )
		return Acad::eInvalidInput;

	//文字的偏移为直径距离
	double dimAlignTextOff = 0;
	if ( mCategoryData->mShape == GlobalData::LINE_SHAPE_CIRCLE )
	{
		dimAlignTextOff = mRadius*2;
	}
	else if ( mCategoryData->mShape == GlobalData::LINE_SHAPE_SQUARE )
	{
		dimAlignTextOff = mLength + mWidth;
	}

	//首先在原点处，沿X轴方向标注
	mAlignedDim->setXLine1Point(AcGePoint3d::kOrigin);
    mAlignedDim->setXLine2Point(AcGePoint3d(length,0,0));
	mAlignedDim->setTextPosition(AcGePoint3d(length/2,dimAlignTextOff,0));

	//文字【段号】
	double dimTextOff = 0,dimTextHeight = 0;

	//设置文字的高度，和Y轴位置
	if ( mCategoryData->mShape == GlobalData::LINE_SHAPE_CIRCLE )
	{
		dimTextHeight = mRadius;
		dimTextOff = mRadius/2;
	}
	else if ( mCategoryData->mShape == GlobalData::LINE_SHAPE_SQUARE )
	{
		dimTextHeight = mLength;
		dimTextOff = mLength/2;
	}

	mLineDim->setHeight(dimTextHeight); 
	mLineDim->setPosition(AcGePoint3d(length/2, dimTextOff,0));

#ifdef DEBUG
	acutPrintf(L"\n标注长度为【%0.2lf】，文字说明的位置X【%0.2lf】Y【%0.2lf】,段号说明位置高度【%0.2lf】Y【%0.2lf】",
					length, length/2, dimAlignTextOff,dimTextHeight,dimTextOff);
#endif

	//设置标注字的配置
    mAlignedDim->useSetTextPosition();    // make text go where user picked
    mAlignedDim->setDatabaseDefaults();

	//首先线段的向量
	AcGeVector3d line3dVector(mEndPoint.x - mStartPoint.x,mEndPoint.y - mStartPoint.y, mEndPoint.z-mStartPoint.z);

	{
		//然后旋转到Z轴处
		AcGeMatrix3d zMatrix = AcGeMatrix3d::rotation( -ArxWrapper::kRad90, AcGeVector3d::kYAxis, AcGePoint3d::kOrigin);
		mAlignedDim->transformBy(zMatrix);
		mLineDim->transformBy(zMatrix);

		//得到（直线与XZ平面）角度
		double xAngle = AcGeVector3d(line3dVector.x,line3dVector.y,0).angleTo(AcGeVector3d::kXAxis);
		acutPrintf(L"\n得到Z轴的旋转角度【%lf】",xAngle);

		//沿Z轴旋转
		AcGeMatrix3d rotateZMatrix = AcGeMatrix3d::rotation( xAngle, AcGeVector3d::kZAxis, AcGePoint3d::kOrigin);
		mAlignedDim->transformBy(rotateZMatrix);
		mLineDim->transformBy(rotateZMatrix);
	}

	{
		//得到旋转的角度
		double angle = -line3dVector.angleTo(AcGeVector3d::kZAxis);
		acutPrintf(L"\n得到旋转角度【%lf】",angle);

		//得到线段与Z轴组成的平面的垂直向量
		AcGeVector3d rotateVctor = line3dVector.crossProduct(AcGeVector3d::kZAxis);

		//进行旋转
		AcGeMatrix3d rotateMatrix = AcGeMatrix3d::rotation( angle, rotateVctor, AcGePoint3d::kOrigin);
		mAlignedDim->transformBy(rotateMatrix);

		mLineDim->transformBy(rotateMatrix);
	}

	{
		//进行偏移
		AcGeMatrix3d moveMatrix;
		moveMatrix.setToTranslation(AcGeVector3d(mStartPoint.x,mStartPoint.y,mStartPoint.z));

		mAlignedDim->transformBy(moveMatrix);

		mLineDim->transformBy(moveMatrix);
	}
	*/

	//以序号来标注
	/*{
		CString textSeq;
		textSeq.Format(L"%d",mSequenceNO);

		mAlignedDim->setDimensionText(textSeq);
	}*/

	/*
	//添加到模型空间
	if( mLineEntry )
	{
		acutPrintf(L"\n加入标注到数据库");

		//mAlignedDim->setLinetype( acdbHostApplicationServices()->workingDatabase()->byLayerLinetype(), true );
		ArxWrapper::PostToModelSpace(mAlignedDim,mLineEntry->m_LineName.c_str());
		mAlignedDim->getAcDbHandle(mHandleDim);

		acutPrintf(L"\n加入文字说明到数据库");
		
		//mLineDim->setLinetype( acdbHostApplicationServices()->workingDatabase()->byLayerLinetype(), true );
		ArxWrapper::PostToModelSpace(mLineDim,mLineEntry->m_LineName.c_str());
		mLineDim->getAcDbHandle(mHandleText);
	}
	else
	{
		acutPrintf(L"\n该线段归属的直线不存在，可能出错了！");
	}
	*/
	return Acad::eOk;
}

AcGePoint3d LMALineDbObject::GetCutCenter( const AcGePlane& cutPlane )
{
	//得到起点用终点形成的线段
	AcGeLineSeg3d lineSeg(mpPointInfo->mStartPoint,mpPointInfo->mEndPoint);

	//线段与直线相切的交点
	AcGePoint3d center;
	Adesk::Boolean hasIntersect = cutPlane.intersectWith(lineSeg, center);

	return center;
}

// Files data in from a DWG file.
//
Acad::ErrorStatus
LMALineDbObject::dwgInFields(AcDbDwgFiler* pFiler)
{
    assertWriteEnabled();

    AcDb3dSolid::dwgInFields(pFiler);

    // For wblock filing we wrote out our owner as a hard
    // pointer ID so now we need to read it in to keep things
    // in sync.
    //
    if (pFiler->filerType() == AcDb::kWblockCloneFiler) {
        AcDbHardPointerId id;
        pFiler->readItem(&id);
    }

	//开始和结束端点
	PointEntry *pStart, *pEnd;

	Adesk::UInt32 lineID;
    pFiler->readItem(&lineID);

	Adesk::UInt32 seqNO;
	pFiler->readItem(&seqNO);

	//从实体管理器中取出该折线段的
	CString filename;
	dbToStr(this->database(),filename);
	LineEntryFileManager::RegisterLineSegment(filename.GetBuffer(),lineID, seqNO, pStart, pEnd );
		
	//得到管线信息绘制管理器
	mpPointInfo = &pEnd->m_DbEntityCollection;

	//读取开始和结束节点
	pFiler->readPoint3d(&mpPointInfo->mStartPoint);
	pFiler->readPoint3d(&mpPointInfo->mEndPoint);

	if( seqNO == 1 )
	{
		//设置开始节点的属性
		pStart->m_Point[X] = mpPointInfo->mStartPoint.x;
		pStart->m_Point[Y] = mpPointInfo->mStartPoint.y;
		pStart->m_Point[Z] = mpPointInfo->mStartPoint.z;
	}

	//设置结束节点的属性
	pEnd->m_Point[X] = mpPointInfo->mEndPoint.x;
	pEnd->m_Point[Y] = mpPointInfo->mEndPoint.y;
	pEnd->m_Point[Z] = mpPointInfo->mEndPoint.z;

	//设置节点折线段绘制信息
	mpPointInfo->mLineID = (UINT)lineID;
	mpPointInfo->mSequenceNO = (UINT)seqNO;

	//设置折线段对象数据库ID
	mpPointInfo->SetLineEntity(id());

	//pFiler->readAcDbHandle(&mHandleDim);
	//pFiler->readAcDbHandle(&mHandleText);

#ifdef DEBUG
	acutPrintf(L"\n从DWG文件【%s】得到管线线段实体 ID【%d】序列号【%d】 起点 X:【%lf】Y:【%lf】Z:【%lf】 终点 X:【%lf】Y:【%lf】Z:【%lf】.",
					filename.GetBuffer(),mpPointInfo->mLineID,mpPointInfo->mSequenceNO,
					mpPointInfo->mStartPoint.x,mpPointInfo->mStartPoint.y,mpPointInfo->mStartPoint.z,
					mpPointInfo->mEndPoint.x,mpPointInfo->mEndPoint.y,mpPointInfo->mEndPoint.z);
#endif

    return pFiler->filerStatus();
}

// Files data out to a DWG file.
//
Acad::ErrorStatus
LMALineDbObject::dwgOutFields(AcDbDwgFiler* pFiler) const
{
    assertReadEnabled();

    AcDb3dSolid::dwgOutFields(pFiler);

    // Since objects of this class will be in the Named
    // Objects Dictionary tree and may be hard referenced
    // by some other object, to support wblock we need to
    // file out our owner as a hard pointer ID so that it
    // will be added to the list of objects to be wblocked.
    //
    if (pFiler->filerType() == AcDb::kWblockCloneFiler)
        pFiler->writeHardPointerId((AcDbHardPointerId)ownerId());

    pFiler->writeItem(Adesk::UInt32(mpPointInfo->mLineID));
	pFiler->writeItem(Adesk::UInt32(mpPointInfo->mSequenceNO));

	pFiler->writeItem(mpPointInfo->mStartPoint);
	pFiler->writeItem(mpPointInfo->mEndPoint);
	
	//pFiler->writeAcDbHandle(mHandleDim);
	//pFiler->writeAcDbHandle(mHandleText);

	CString filename;
	dbToStr(this->database(),filename);

#ifdef DEBUG
	acutPrintf(L"\n保存管线线段实体 序列号【%d】 起点 X:【%0.2lf】Y:【%0.2lf】Z:【%0.2lf】 终点 X:【%0.2lf】Y:【%0.2lf】Z:【%0.2lf】到DWG文件【%s】.",
					mpPointInfo->mSequenceNO,
					mpPointInfo->mStartPoint.x, mpPointInfo->mStartPoint.y, mpPointInfo->mStartPoint.z,
					mpPointInfo->mEndPoint.x, mpPointInfo->mEndPoint.y, mpPointInfo->mEndPoint.z,
					filename.GetBuffer());
#endif

    return pFiler->filerStatus();
}

// Files data in from a DXF file.
//
Acad::ErrorStatus
LMALineDbObject::dxfInFields(AcDbDxfFiler* pFiler)
{
    assertWriteEnabled();

    Acad::ErrorStatus es;
    if ((es = AcDb3dSolid::dxfInFields(pFiler))
        != Acad::eOk)
    {
        return es;
    }

    // Check if we're at the right subclass getLineID marker.
    //
    if (!pFiler->atSubclassData(_T("LMALineDbObject"))) {
        return Acad::eBadDxfSequence;
    }

    struct resbuf inbuf;
    while (es == Acad::eOk) {
        if ((es = pFiler->readItem(&inbuf)) == Acad::eOk) {

			switch ( inbuf.restype )
			{
				/*case AcDb::kDxfInt32:
					mLineID = inbuf.resval.rint;
				case AcDb::kDxfInt32 + 1:
					mSequenceNO = inbuf.resval.rint;*/
				default:
					break;
			}
        }
    }

    return pFiler->filerStatus();
}

// Files data out to a DXF file.
//
Acad::ErrorStatus
LMALineDbObject::dxfOutFields(AcDbDxfFiler* pFiler) const
{
    assertReadEnabled();

    AcDb3dSolid::dxfOutFields(pFiler);
    pFiler->writeItem(AcDb::kDxfSubclass, _T("LMALineDbObject"));
    pFiler->writeItem(AcDb::kDxfInt32, mpPointInfo->mLineID);
	pFiler->writeItem(AcDb::kDxfInt32 + 1, mpPointInfo->mSequenceNO);

    return pFiler->filerStatus();
}

} // end of arx

} // end of assistant

} // end of guch

} // end of com
