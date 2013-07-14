// ------------------------------------------------
//                  LineManagementAssistant
// Copyright 2012-2013, Chengyong Yang & Changhai Gu. 
//               All rights reserved.
// ------------------------------------------------
//	LineEntryData.cpp
//	written by Changhai Gu
// ------------------------------------------------
// $File:\\LineManageAssitant\main\source\data\LineEntryData.h $
// $Author: Changhai Gu $
// $DateTime: 2013/1/2 01:35:46 $
// $Revision: #1 $
// ------------------------------------------------

#include <LineEntryData.h>
#include <LMAUtils.h>
#include <GlobalDataConfig.h>

#include <ArxWrapper.h>
#include <acdocman.h>
#include <acutmem.h>

#include <LineManageAssitant.h>

using namespace ::com::guch::assistant::config;
extern wstring gLmaArxLoadPath;

namespace com
{

namespace guch
{

namespace assistant
{

namespace data
{

///////////////////////////////////////////////////////////////////////////
// Implementation LineEntity

const wstring LineEntity::LINE_ENTRY_LAYER = L"管线实体字典";
const wstring LineEntity::LINE_DATA_BEGIN = L"实体数据：";

/**
 * 管线实体
 */

LineEntity::LineEntity()
	:m_LineID(0),
	m_LineName(L""),
	m_LineKind(L""),
	m_LineBasiInfo(new LineCategoryItemData()),
	m_CurrentPointNO(0),
	m_PrePointList(NULL),
	m_PointList(new PointList())
{}

LineEntity::LineEntity(const wstring& rLineName, const wstring& rLineKind,
						LineCategoryItemData* lineInfo, PointList* pointList)
	:m_LineID(0),
	m_LineName(rLineName),
	m_LineKind(rLineKind),
	m_LineBasiInfo(lineInfo),
	m_PointList(pointList),
	m_CurrentPointNO(0),
	m_PrePointList(NULL)
{
	//创建数据库代理对象
	m_pDbEntry = new LineDBEntity( this );
}

LineEntity::LineEntity( wstring& data)
{
	m_PointList = new PointList();
	m_PrePointList = NULL;

	double temp;
	int index = 0;

	const static size_t start = wcslen(LINE_DATA_BEGIN.c_str());
	wstrVector* dataColumn = vectorContructor(data,L"\t",start,data.length());

	//得到线的相关属性
	wstring& column = (*dataColumn)[index++];
	acdbDisToF(column.c_str(), -1, &temp);
	this->m_LineID = (UINT)temp;

	m_LineName = (*dataColumn)[index++];
	m_LineKind = (*dataColumn)[index++];

	//得到详细信息
	m_LineBasiInfo = new LineCategoryItemData();

	m_LineBasiInfo->mCategory = (*dataColumn)[index++];
	m_LineBasiInfo->mShape = (*dataColumn)[index++];

	m_LineBasiInfo->mSize.mRadius = (*dataColumn)[index++];
	m_LineBasiInfo->mSize.mWidth = (*dataColumn)[index++];
	m_LineBasiInfo->mSize.mHeight = (*dataColumn)[index++];
	m_LineBasiInfo->mSize.mReservedA = (*dataColumn)[index++];
	m_LineBasiInfo->mSize.mReservedB = (*dataColumn)[index++];

	m_LineBasiInfo->mWallSize = (*dataColumn)[index++];
	m_LineBasiInfo->mSafeSize = (*dataColumn)[index++];
	m_LineBasiInfo->mPlaneMark = (*dataColumn)[index++];
	m_LineBasiInfo->mCutMark = (*dataColumn)[index++];
	m_LineBasiInfo->mThroughDirection = (*dataColumn)[index++];

	//编号在点左边前面
	column = (*dataColumn)[index++];
	acdbDisToF(column.c_str(), -1, &temp);
	m_CurrentPointNO = (UINT)temp;

	//得到每个点的属性
	int size = (int)dataColumn->size();

	while( index < size )
	{
		column = (*dataColumn)[index++];

		if( column.length() > 3 )
			m_PointList->push_back(new PointEntity(column));
	}

	delete dataColumn;

	//创建数据库代理对象
	m_pDbEntry = new LineDBEntity( this );
}

LineEntity::~LineEntity()
{
	//ClearPoints();
}

void LineEntity::ClearPoints()
{
	ClearPoints(this->m_PrePointList);
	ClearPoints(this->m_PointList);
}

void LineEntity::ClearPoints( PointList* pPointList)
{
	if( pPointList )
	{
		for( PointIter iter = pPointList->begin();
				iter != pPointList->end();
				iter++ )
		{
			if(*iter)
				delete *iter;
		}

		delete pPointList;
		pPointList = NULL;
	}
}

PointIter LineEntity::FindPoint( const UINT& PointNO ) const
{
	for( PointIter iter = this->m_PointList->begin();
			iter != this->m_PointList->end();
			iter++)
	{
		if( (*iter)->m_PointNO == PointNO )
			return iter;
	}

	return m_PointList->end();
}

ContstPointIter LineEntity::FindConstPoint( const UINT& PointNO ) const
{
	for( ContstPointIter iter = this->m_PointList->begin();
			iter != this->m_PointList->end();
			iter++)
	{
		if( (*iter)->m_PointNO == PointNO )
			return iter;
	}

	return m_PointList->end();
}

int LineEntity::InsertPoint( const PointEntity& newPoint )
{
	pPointEntry point = new PointEntity(newPoint);

	point->m_PointNO = m_CurrentPointNO;

	m_PointList->push_back(point);

	m_CurrentPointNO++;

	return (int)m_PointList->size();
}

void LineEntity::UpdatePoint( const PointEntity& updatePoint )
{
	PointIter findPoint = this->FindPoint(updatePoint.m_PointNO);

	if( findPoint != this->m_PointList->end() )
	{
		delete *findPoint;
		*findPoint = new PointEntity(updatePoint);
	}
}

void LineEntity::DeletePoint( const UINT& PointNO )
{
	PointIter findPoint = this->FindPoint(PointNO);

	if( findPoint != this->m_PointList->end() )
	{
		m_PointList->erase(findPoint);
	}
}

void LineEntity::SetBasicInfo( LineCategoryItemData* lineBasiInfo )
{
	if( m_LineBasiInfo )
		delete m_LineBasiInfo;

	m_LineBasiInfo = lineBasiInfo;
}

void LineEntity::SetPoints( PointList* newPoints)
{
	//保存当前的节点列表，以用于删除以前的对象
	m_PrePointList = m_PointList;

	//新的列表，用于创建新的线段
	m_PointList = newPoints;

	Redraw();
}

wstring LineEntity::toString()
{
	wstring lineData(LINE_DATA_BEGIN);
	lineData += L"\r\n";

	CString temp;
	temp.Format(L"%d\t%s\t%s\t%s\t%d",
					m_LineID,m_LineName.c_str(),m_LineKind.c_str(),
					m_LineBasiInfo->toString().c_str(),
					(m_PointList != NULL ? m_PointList->size() : 0 ) );

	lineData += wstring(temp.GetBuffer());
	if( m_PointList )
	{
		for( ContstPointIter iter = m_PointList->begin();
				iter != m_PointList->end();
				iter++)
		{
			lineData += L"\t\r\n";
			lineData += (*iter)->toString();
		}
	}

	return lineData;
}

void LineEntity::Redraw()
{
	//删除以前的线段(从数据库中)
	EraseDbObjects(true);

	//删除以前的线段(从内存中)
	ClearPoints(m_PrePointList);
	m_PrePointList = NULL;

	//绘制新的线段
	CreateDbObjects();
}

/**
 * 根据导入线段配置，创建多线段3D折线
 **/
void LineEntity::CreateDbObjects()
{
	if( m_PointList == NULL || m_PointList->size() < 2 )
	{
		acutPrintf(L"\n管线实体【%s】无折线段", m_LineName.c_str());
		return;
	}

	acutPrintf(L"\n需要绘制【%d】条折线段",m_PointList->size()-1);

	try
	{
		//首先创建图层
		ArxWrapper::createNewLayer(m_LineName);

		//绘制3D模型
		DrawDBEntity();
	}
	catch(const Acad::ErrorStatus es)
	{
		acutPrintf(L"\n绘制线段发生异常！");
		rxErrorMsg(es);
	}
}

/**
 * 创建多线段3D折线
 **/
void LineEntity::DrawDBEntity()
{
	ads_point *pStart = NULL;

	for( ContstPointIter iter = m_PointList->begin();
		iter != m_PointList->end();
		iter++)
	{
		if( pStart == NULL )
		{
			//多线段的第一个起点
			pStart = &(*iter)->m_Point;
			
			//初始化绘制信息
			(*iter)->m_DbEntityCollection.mSequenceNO = 0;

			//不需要创建该3D模型
			continue;
		}
		else
		{
			//创建3D柱体代表直线
			(*iter)->CreateLineFrom( (void*)this, *pStart );

			//继续下一个线段
			pStart = &(*iter)->m_Point;
		}
	}
}

/**
 * 根据多线段的配置，删除3D管线
 **/
void LineEntity::EraseDbObjects( bool old )
{
	PointList* pPointList = old ? m_PrePointList : m_PointList;
	if( pPointList == NULL || pPointList->size() < 2 )
	{
		acutPrintf(L"\n管线没有【%s】的线段",(old ? L"失效" : L"当前"));
		return;
	}

	acutPrintf(L"\n删除管线【%s】所有【%s】的线段，共【%d】条", m_LineName.c_str(),(old ? L"失效" : L"当前"), (pPointList->size() - 1) );

	//锁止当前文档
	ArxWrapper::LockCurDoc();

	for( ContstPointIter iter = pPointList->begin();
		iter != pPointList->end();
		iter++)
	{
		if( iter == pPointList->begin() )
		{
			continue;
		}
		else
		{
			(*iter)->m_DbEntityCollection.DropEntityCollection();
		}
	}

	//解锁当前文档
	ArxWrapper::UnLockCurDoc();
}

///////////////////////////////////////////////////////////////////////////
// Implementation LineDBEntity

ACRX_DXF_DEFINE_MEMBERS(LineDBEntity, AcDbObject, AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent, 0, LineDBEntity, LMA);

LineDBEntity::LineDBEntity()
{
	pImplemention = new LineEntity();
}

LineDBEntity::LineDBEntity( LineEntity* implementation )
{
	pImplemention = implementation;
}

// Files data in from a DWG file.
//
Acad::ErrorStatus
LineDBEntity::dwgInFields(AcDbDwgFiler* pFiler)
{
	if( LineEntityFileManager::openingDwg == false )
	{
		acutPrintf(L"\n设置当前状态为正在打开文件");
		LineEntityFileManager::openingDwg = true;
	}

    assertWriteEnabled();

    AcDbObject::dwgInFields(pFiler);
    // For wblock filing we wrote out our owner as a hard
    // pointer ID so now we need to read it in to keep things
    // in sync.
    //
    if (pFiler->filerType() == AcDb::kWblockCloneFiler) {
        AcDbHardPointerId id;
        pFiler->readItem(&id);
    }

	if( !this->isErased() )
	{
		Adesk::UInt32 lineID;
		pFiler->readItem(&lineID);
		pImplemention->m_LineID = (UINT)lineID;

		TCHAR* tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!

		tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
		pFiler->readItem(&tmpStr);
		pImplemention->m_LineName = wstring(tmpStr);
		acutDelString(tmpStr);

		tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
		pFiler->readItem(&tmpStr);
		pImplemention->m_LineKind = wstring(tmpStr);
		acutDelString(tmpStr);

		{
			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mCategory = wstring(tmpStr);
			acutDelString(tmpStr);

			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mShape = wstring(tmpStr);
			acutDelString(tmpStr);

			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mSize.mRadius = wstring(tmpStr);
			acutDelString(tmpStr);
		
			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mSize.mWidth = wstring(tmpStr);
			acutDelString(tmpStr);

			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mSize.mHeight = wstring(tmpStr);
			acutDelString(tmpStr);

			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mSize.mReservedA = wstring(tmpStr);
			acutDelString(tmpStr);

			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mSize.mReservedB = wstring(tmpStr);
			acutDelString(tmpStr);

			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mWallSize = wstring(tmpStr);
			acutDelString(tmpStr);

			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mSafeSize = wstring(tmpStr);
			acutDelString(tmpStr);
		
			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mPlaneMark = wstring(tmpStr);
			acutDelString(tmpStr);

			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mCutMark = wstring(tmpStr);
			acutDelString(tmpStr);

			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mThroughDirection = wstring(tmpStr);
			acutDelString(tmpStr);
		}

		//得到对象的数据库ID
		pImplemention->m_dbId = this->id();

		CString filename;
		dbToStr(this->database(),filename);

#ifdef DEBUG
		acutPrintf(L"\n从文件【%s】读出管线实体 ID【%d】名称【%s】类型【%s】.",
					filename.GetBuffer(),
					pImplemention->m_LineID,
					pImplemention->m_LineName.c_str(),
					pImplemention->m_LineKind.c_str() );
#endif

		wstring fileName(filename.GetBuffer());
		LineEntityFile* entryFile = LineEntityFileManager::RegisterEntryFile(fileName);

		PointList* tempList = entryFile->TransferTempLine(pImplemention->m_LineID);
		if( tempList )
		{
			for( PointIter iter = tempList->begin();
				 iter != tempList->end();
				 iter++)
			{
				acutPrintf(L"\n追加折线段【%s】序号【%d】",(*iter)->m_DbEntityCollection.mLayerName.c_str(), (*iter)->m_PointNO);
				this->pImplemention->m_PointList->push_back( *(iter) );

				//设置基本信息
				(*iter)->m_DbEntityCollection.mCategoryData = const_cast<LineCategoryItemData*>(this->pImplemention->GetBasicInfo());
			}

			delete tempList;
		}

#ifdef DEBUG
		acutPrintf(L"\n从临时管线管理器中得到线段数据，个数为【%d】", ( pImplemention->m_PointList ? pImplemention->m_PointList->size() : 0 ) );
#endif

		//向管线文件中加入管线line
		entryFile->InsertLine(pImplemention);
	}

    return pFiler->filerStatus();
}

// Files data out to a DWG file.
//
Acad::ErrorStatus
LineDBEntity::dwgOutFields(AcDbDwgFiler* pFiler) const
{
    assertReadEnabled();

    AcDbObject::dwgOutFields(pFiler);
    // Since objects of this class will be in the Named
    // Objects Dictionary tree and may be hard referenced
    // by some other object, to support wblock we need to
    // file out our owner as a hard pointer ID so that it
    // will be added to the list of objects to be wblocked.
    //

    if (pFiler->filerType() == AcDb::kWblockCloneFiler)
        pFiler->writeHardPointerId((AcDbHardPointerId)ownerId());

#ifdef DEBUG
	acutPrintf(L"\n保存管线实体到数据库 ID【%d】名称【%s】类型【%s】.",
				pImplemention->m_LineID,
				pImplemention->m_LineName.c_str(),
				pImplemention->m_LineKind.c_str());
#endif

    pFiler->writeItem(Adesk::UInt32(pImplemention->m_LineID));
	pFiler->writeItem(pImplemention->m_LineName.c_str());
	pFiler->writeItem(pImplemention->m_LineKind.c_str());

	pFiler->writeItem(pImplemention->m_LineBasiInfo->mCategory.c_str());
	pFiler->writeItem(pImplemention->m_LineBasiInfo->mShape.c_str());

	pFiler->writeItem(pImplemention->m_LineBasiInfo->mSize.mRadius.c_str());
	pFiler->writeItem(pImplemention->m_LineBasiInfo->mSize.mWidth.c_str());
	pFiler->writeItem(pImplemention->m_LineBasiInfo->mSize.mHeight.c_str());
	pFiler->writeItem(pImplemention->m_LineBasiInfo->mSize.mReservedA.c_str());
	pFiler->writeItem(pImplemention->m_LineBasiInfo->mSize.mReservedB.c_str());

	pFiler->writeItem(pImplemention->m_LineBasiInfo->mWallSize.c_str());
	pFiler->writeItem(pImplemention->m_LineBasiInfo->mSafeSize.c_str());

	pFiler->writeItem(pImplemention->m_LineBasiInfo->mPlaneMark.c_str());
	pFiler->writeItem(pImplemention->m_LineBasiInfo->mCutMark.c_str());

	pFiler->writeItem(pImplemention->m_LineBasiInfo->mThroughDirection.c_str());

    return pFiler->filerStatus();
}

// Files data in from a DXF file.
//
Acad::ErrorStatus
LineDBEntity::dxfInFields(AcDbDxfFiler* pFiler)
{
    assertWriteEnabled();

    Acad::ErrorStatus es;
    if ((es = AcDbObject::dxfInFields(pFiler))
        != Acad::eOk)
    {
        return es;
    }

    // Check if we're at the right subclass getLineID marker.
    //
    if (!pFiler->atSubclassData(_T("LineEntryData"))) {
        return Acad::eBadDxfSequence;
    }

    struct resbuf inbuf;
    while (es == Acad::eOk) {
        if ((es = pFiler->readItem(&inbuf)) == Acad::eOk) {

			/*
			switch ( inbuf.restype )
			{
				//case AcDb::kDxfInt16:
				//	m_LineID = inbuf.resval.rint;
				//case AcDb::kDxfInt16 + 1:
					//mSequenceNO = inbuf.resval.rint;
			}
			*/
        }
    }

    return pFiler->filerStatus();
}

// Files data out to a DXF file.
//
Acad::ErrorStatus
LineDBEntity::dxfOutFields(AcDbDxfFiler* pFiler) const
{
    assertReadEnabled();

    AcDbObject::dxfOutFields(pFiler);
    pFiler->writeItem(AcDb::kDxfSubclass, _T("LineEntryData"));
    //pFiler->writeItem(AcDb::kDxfInt16, mLineID);
	//pFiler->writeItem(AcDb::kDxfInt16 + 1, mSequenceNO);

    return pFiler->filerStatus();
}

} // end of data

} // end of assistant

} // end of guch

} // end of com