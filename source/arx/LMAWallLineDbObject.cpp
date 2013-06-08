// ------------------------------------------------
//                  LineManagementAssistant
// Copyright 2012-2013, Chengyong Yang & Changhai Gu. 
//               All rights reserved.
// ------------------------------------------------
//	LMAWallLineDbObject.cpp
//	written by Changhai Gu
// ------------------------------------------------
// $File:\\LineManageAssitant\main\source\ARX\LMAWallLineDbObject.cpp $
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

ACRX_DXF_DEFINE_MEMBERS(LMAWallLineDbObject, AcDb3dSolid, 
AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent, 
0,
    LMAWallLineDbObject, LMA);

/// <summary>
/// Initializes a new instance of the <see cref="LMALineDbObject" /> class.
/// </summary>
LMAWallLineDbObject::LMAWallLineDbObject():
	LMALineDbObject()
{
}

/// <summary>
/// Initializes a new instance of the <see cref="LMALineDbObject" /> class.
/// </summary>
/// <param name="pPointInfo">The p point info.</param>
LMAWallLineDbObject::LMAWallLineDbObject( PointDBEntityCollection* pPointInfo)
: LMALineDbObject( pPointInfo, false )
{
	Init();
}

 /// <summary>
/// Inits this instance.
/// </summary>
/// <returns></returns>
bool LMAWallLineDbObject::CalculateSize()
{
	acutPrintf(L"\n计算管线壁厚大小");

	if( LMALineDbObject::CalculateSize() == false )
	{
		acutPrintf(L"\n计算管线内的大小失败");
		return false;
	}

	//安全半径与壁厚
	double wallSize(0);
	acdbDisToF(mpPointInfo->mCategoryData->mWallSize.c_str(), -1, &wallSize);
	wallSize /= 1000;

	//圆形或矩形
	if( mpPointInfo->mCategoryData->mShape == GlobalData::LINE_SHAPE_CIRCLE )
	{
		//直径的单位是毫米，而距离的单位是米
		mRadius = mRadius + wallSize;

		acutPrintf(L"\n创建壁厚【%0.2lf】半径为【%0.2lf】的圆柱", wallSize, mRadius);
	}
	else //if ( mpPointInfo->mCategoryData->mShape == GlobalData::LINE_SHAPE_SQUARE )
	{
		//直径的单位是毫米，而距离的单位是米
		mWidth = mWidth + wallSize;
		mHeight = mHeight + wallSize;

		acutPrintf(L"\n创建壁厚【%0.2lf】宽为【%0.2lf】高为【%0.2lf】的方柱", wallSize, mWidth, mHeight);
	}

	return true;
}

/// <summary>
/// Creates the pipe.
/// </summary>
/// <returns></returns>
Acad::ErrorStatus LMAWallLineDbObject::CreateDBObject()
{
	//同样也是绘制管线
	//LMALineDbObject::CreateDBObject();

	acutPrintf(L"\n绘制管线壁实体");

	const AcGePoint3d& startPoint = mpPointInfo->mStartPoint;
	const AcGePoint3d& endPoint = mpPointInfo->mEndPoint;

	//得到线段的长度
	double height = startPoint.distanceTo( endPoint );
	if( height < 0.01 )
	{
		acutPrintf(L"\n高度小于1毫米，暂不考虑这样的实体！",mRadius,height);
		return Acad::eInvalidInput;
	}

	double wallOffset = height * 0.02;
	height -= wallOffset;

	if( mpPointInfo->mCategoryData->mShape == GlobalData::LINE_SHAPE_CIRCLE )
	{
		//acutPrintf(L"\n绘制半径为【%0.2lf】长为【%0.2lf】的圆柱",mRadius,height);

		//绘制圆柱体
		this->createFrustum(height, mRadius, mRadius, mRadius);
	}
	else //if (  mpPointInfo->mCategoryData->mShape == GlobalData::LINE_SHAPE_SQUARE )
	{
		//acutPrintf(L"\n绘制宽【%0.2lf】高【%0.2lf】长【%0.2lf】的方柱体",mHeight, mWidth, height);

		//绘制圆柱体
		this->createBox(mWidth, mHeight, height);
	}

	//进行偏移
	AcGeMatrix3d wallUpMatrix;
	double zOffset = wallOffset / 8;
	wallUpMatrix.setToTranslation(AcGeVector3d(0, 0, zOffset));

	//向上移动一定的偏移量的1/4，保证内部管线实体露出
	transformBy(wallUpMatrix);

	AcGeMatrix3d rotateMatrix;
	double angle;

	//得到线段与Z轴的垂直向量
	AcGeVector3d line3dVector(endPoint.x - startPoint.x, endPoint.y - startPoint.y, endPoint.z - startPoint.z);
	AcGeVector3d rotateVctor = line3dVector.crossProduct(AcGeVector3d::kZAxis);

	//得到上诉垂直向量与X轴的夹角
	angle = rotateVctor.angleTo(AcGeVector3d::kXAxis); // 线段位于第二，三象限时，垂直向量与X轴的夹角与该线段在第一，四象限时的夹角相等，估需区分
	if( startPoint.x < endPoint.x )
		angle = -angle;
	//acutPrintf(L"\n得到倾斜的角度【%lf】",angle);

	//进行旋转，放置柱体倾斜
	rotateMatrix = AcGeMatrix3d::rotation( angle, AcGeVector3d::kZAxis, AcGePoint3d::kOrigin);
	transformBy(rotateMatrix);

	//得到旋转的角度
	angle = -line3dVector.angleTo(AcGeVector3d::kZAxis);
	//acutPrintf(L"\n得到旋转角度【%lf】",angle);

	//进行旋转
	rotateMatrix = AcGeMatrix3d::rotation( angle, rotateVctor, AcGePoint3d::kOrigin);
	transformBy(rotateMatrix);
	
	//得到线段的中心点
	AcGePoint3d center( startPoint.x + endPoint.x, startPoint.y + endPoint.y, startPoint.z + endPoint.z); 
	center /= 2;
	//acutPrintf(L"\n得到中心点【%0.2lf】【%0.2lf】【%0.2lf】",center.x,center.y,center.z);

	//进行偏移
	AcGeMatrix3d moveMatrix;
	moveMatrix.setToTranslation(AcGeVector3d(center.x,center.y,center.z));

	//最终成型
	transformBy(moveMatrix);

	//标注为黄色，用于区分
	setColorIndex( GlobalData::WALLLINE_COLOR );

	return Acad::eOk;
}

// Files data in from a DWG file.
//
Acad::ErrorStatus
LMAWallLineDbObject::dwgInFields(AcDbDwgFiler* pFiler)
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
	PointEntity *pStart(NULL), *pEnd(NULL);

	Adesk::UInt32 lineID;
    pFiler->readItem(&lineID);

	Adesk::UInt32 seqNO;
	pFiler->readItem(&seqNO);

	//从实体管理器中取出该折线段的
	CString filename;
	dbToStr(this->database(),filename);
	
	LineEntity* pLineEntity(NULL);
	if( LineEntityFileManager::RegisterLineSegment(filename.GetBuffer(),lineID, seqNO, pLineEntity, pStart, pEnd ) == false )
	{
		acutPrintf(L"\n注册失败，无效的壁厚线段");
		return Acad::eAlreadyInDb;
	}

	//得到管线信息绘制管理器
	if( pEnd )
	{
		mpPointInfo = &pEnd->m_DbEntityCollection;
	}
	else
	{
		acutPrintf(L"\n壁厚实体无对应的结束点坐标!");
		return Acad::eInvalidIndex;
	}

	//设置折线段对象数据库ID
	if( mpPointInfo )
		mpPointInfo->SetWallLineEntity(id());

#ifdef DEBUG
	acutPrintf(L"\n从DWG文件【%s】得到管线壁线段实体 ID【%d】序列号【%d】.",
					filename.GetBuffer(),mpPointInfo->mLineID,mpPointInfo->mSequenceNO );
#endif

    return pFiler->filerStatus();
}

// Files data out to a DWG file.
//
Acad::ErrorStatus
LMAWallLineDbObject::dwgOutFields(AcDbDwgFiler* pFiler) const
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

	CString filename;
	dbToStr(this->database(),filename);

#ifdef DEBUG
	acutPrintf(L"\n保存管线壁线段实体 序列号【%d】到DWG文件【%s】.",
					mpPointInfo->mSequenceNO,
					filename.GetBuffer());
#endif

    return pFiler->filerStatus();
}

// Files data in from a DXF file.
//
Acad::ErrorStatus
LMAWallLineDbObject::dxfInFields(AcDbDxfFiler* pFiler)
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
    if (!pFiler->atSubclassData(_T("LMASafeLineDbObject"))) {
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
LMAWallLineDbObject::dxfOutFields(AcDbDxfFiler* pFiler) const
{
    assertReadEnabled();

    AcDb3dSolid::dxfOutFields(pFiler);
    pFiler->writeItem(AcDb::kDxfSubclass, _T("LMASafeLineDbObject"));
    pFiler->writeItem(AcDb::kDxfInt32, mpPointInfo->mLineID);
	pFiler->writeItem(AcDb::kDxfInt32 + 1, mpPointInfo->mSequenceNO);

    return pFiler->filerStatus();
}

} // end of arx

} // end of assistant

} // end of guch

} // end of com
