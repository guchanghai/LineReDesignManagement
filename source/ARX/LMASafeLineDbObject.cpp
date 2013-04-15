// ------------------------------------------------
//                  LineManagementAssistant
// Copyright 2012-2013, Chengyong Yang & Changhai Gu. 
//               All rights reserved.
// ------------------------------------------------
//	ArxWrapper.h
//	written by Changhai Gu
// ------------------------------------------------
// $File:\\LineManageAssitant\main\source\ARX\LMASafeLineObject.cpp $
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

ACRX_DXF_DEFINE_MEMBERS(LMASafeLineDbObject, AcDb3dSolid, 
AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent, 
0,
    LMASafeLineDbObject, LMA);

/// <summary>
/// Initializes a new instance of the <see cref="LMALineDbObject" /> class.
/// </summary>
LMASafeLineDbObject::LMASafeLineDbObject():
	LMALineDbObject()
{
}

/// <summary>
/// Initializes a new instance of the <see cref="LMALineDbObject" /> class.
/// </summary>
/// <param name="pPointInfo">The p point info.</param>
LMASafeLineDbObject::LMASafeLineDbObject( PointDBEntityCollection* pPointInfo)
: LMALineDbObject( pPointInfo )
{
	Init();
}

 /// <summary>
/// Inits this instance.
/// </summary>
/// <returns></returns>
Acad::ErrorStatus LMASafeLineDbObject::Init()
{
	if( mpPointInfo == NULL ||
		mpPointInfo->mCategoryData == NULL )
	{
		acutPrintf(L"\n配置信息不合法");
		return Acad::eInvalidInput;
	}

	//安全半径
	double safeSize = 0;
	acdbDisToF(mpPointInfo->mCategoryData->mSafeSize.c_str(), -1, &safeSize);

	//圆形或矩形
	if( mpPointInfo->mCategoryData->mShape == GlobalData::LINE_SHAPE_CIRCLE )
	{
		acdbDisToF(mpPointInfo->mCategoryData->mSize.mRadius.c_str(), -1, &mRadius);
		
		//直径的单位是毫米，而距离的单位是米
		mRadius = (mRadius + safeSize) / 1000;
	}
	else //if ( mpPointInfo->mCategoryData->mShape == GlobalData::LINE_SHAPE_SQUARE )
	{
		acdbDisToF(mpPointInfo->mCategoryData->mSize.mHeight.c_str(), -1, &mLength);
		acdbDisToF(mpPointInfo->mCategoryData->mSize.mWidth.c_str(), -1, &mWidth);

		//直径的单位是毫米，而距离的单位是米
		mLength = ( mLength + safeSize )/ 1000;
		mWidth = ( mWidth + safeSize)/ 1000;
	}

	return CreateDBObject();
}

/// <summary>
/// Creates the pipe.
/// </summary>
/// <returns></returns>
Acad::ErrorStatus LMASafeLineDbObject::CreateDBObject()
{
	//同样也是绘制管线
	LMALineDbObject::CreateDBObject();

	//标注为绿色，用于区分
	this->setColorIndex(2);

	return Acad::eOk;
}

// Files data in from a DWG file.
//
Acad::ErrorStatus
LMASafeLineDbObject::dwgInFields(AcDbDwgFiler* pFiler)
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
	PointEntity *pStart, *pEnd;

	Adesk::UInt32 lineID;
    pFiler->readItem(&lineID);

	Adesk::UInt32 seqNO;
	pFiler->readItem(&seqNO);

	//从实体管理器中取出该折线段的
	CString filename;
	dbToStr(this->database(),filename);
	LineEntityFileManager::RegisterLineSegment(filename.GetBuffer(),lineID, seqNO, pStart, pEnd );
			
	//得到管线信息绘制管理器
	mpPointInfo = &pEnd->m_DbEntityCollection;

	//设置折线段对象数据库ID
	mpPointInfo->SetSafeLineEntity(id());

#ifdef DEBUG
	acutPrintf(L"\n从DWG文件【%s】得到管线安全范围线段实体 ID【%d】序列号【%d】.",
					filename.GetBuffer(),mpPointInfo->mLineID,mpPointInfo->mSequenceNO );
#endif

    return pFiler->filerStatus();
}

// Files data out to a DWG file.
//
Acad::ErrorStatus
LMASafeLineDbObject::dwgOutFields(AcDbDwgFiler* pFiler) const
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
	acutPrintf(L"\n保存管线安全范围线段实体 序列号【%d】到DWG文件【%s】.",
					mpPointInfo->mSequenceNO,
					filename.GetBuffer());
#endif

    return pFiler->filerStatus();
}

// Files data in from a DXF file.
//
Acad::ErrorStatus
LMASafeLineDbObject::dxfInFields(AcDbDxfFiler* pFiler)
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
LMASafeLineDbObject::dxfOutFields(AcDbDxfFiler* pFiler) const
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
