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

#include <ArxCustomObject.h>
#include <LineManageAssitant.h>

using namespace ::com::guch::assistant::arx;
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

#define POINT_START L"坐标:"
#define POINT_END L""
#define POINTS_SEP L"-"

///////////////////////////////////////////////////////////////////////////
// Implementation PointEntity

/**
 * 管线坐标实体
 */

PointEntity::PointEntity()
:m_PointNO(0),
m_LevelKind(L""),
m_Direction(L""),
m_DbEntityCollection()
{
	m_Point[X] = 0;
	m_Point[Y] = 0;
	m_Point[Z] = 0;
}

PointEntity::PointEntity( const UINT& pointNO, const ads_point& point, 
	const wstring& levelKind, const wstring& direction, const AcDbObjectId& entityID)
:m_PointNO(pointNO),
m_LevelKind(levelKind),
m_Direction(direction),
m_DbEntityCollection()
{
	m_Point[X] = point[X];
	m_Point[Y] = point[Y];
	m_Point[Z] = point[Z];
}

PointEntity::PointEntity( const PointEntity& pointEntry)
{
	this->m_PointNO = pointEntry.m_PointNO;
	this->m_LevelKind = pointEntry.m_LevelKind;
	this->m_Direction = pointEntry.m_Direction;

	this->m_Point[X] = pointEntry.m_Point[X];
	this->m_Point[Y] = pointEntry.m_Point[Y];
	this->m_Point[Z] = pointEntry.m_Point[Z];
}

PointEntity::PointEntity( const wstring& data )
{
	double temp;

	const static size_t start = wcslen(POINT_START);
	//size_t end = data.find_first_of(POINT_END);

	int index = 0;

	wstrVector* dataColumn = vectorContructor(data,POINTS_SEP,start,data.length());
	wstring& column = (*dataColumn)[index++];

	acdbDisToF(column.c_str(), -1, &temp);
	this->m_PointNO = (UINT)temp;

	column = (*dataColumn)[index++];
	acdbDisToF(column.c_str(), -1, &m_Point[X]);

	column = (*dataColumn)[index++];
	acdbDisToF(column.c_str(), -1, &m_Point[Y]);

	column = (*dataColumn)[index++];
	acdbDisToF(column.c_str(), -1, &m_Point[Z]);

	//m_LevelKind = (*dataColumn)[index++];
	//m_Direction = (*dataColumn)[index++];

	delete dataColumn;
}

wstring PointEntity::toString() const
{
	CString temp;
	temp.Format(L"%s%d%s%0.2f%s%0.2f%s%0.2f%s%s",
				POINT_START, m_PointNO, POINTS_SEP,
				m_Point[X], POINTS_SEP,
				m_Point[Y], POINTS_SEP,
				m_Point[Z], POINTS_SEP,
				POINT_END);

	return temp.GetBuffer();
}

/**
 * 从上一折线段终点开始，绘制所需的管线
 **/
void PointEntity::CreateLineFrom(const void* lineEntity, const ads_point& start )
{
	//准备绘制折线段的所有信息
	LineEntity* pLineEntity = (LineEntity*)lineEntity;

	//管线名称既是层名，就是说每根管线都在不同的层
	m_DbEntityCollection.mLayerName = pLineEntity->GetName();
	
	//唯一标示管线的ID，折线段的所有实体都保存此值用于关联折线段与管线
	m_DbEntityCollection.mLineID = pLineEntity->GetLineID();
	
	//绘制管线时所需要的信息
	m_DbEntityCollection.mCategoryData = const_cast<LineCategoryItemData*>(pLineEntity->GetBasicInfo());
	
	//折线段的序号
	m_DbEntityCollection.mSequenceNO = m_PointNO;

	//折线段的起始和终止点
	m_DbEntityCollection.mStartPoint.set(start[X], start[Y], start[Z]);
	m_DbEntityCollection.mEndPoint.set(m_Point[X], m_Point[Y], m_Point[Z]);

	//绘制折线段所有的数据库实体
	m_DbEntityCollection.DrawEntityCollection();
}

/**
 * 根据起始点队列（向量列表），并放置在特定的层上
 **/
bool PointDBEntityCollection::DrawEntityCollection() 
{
#ifdef DEBUG
	acutPrintf(L"\n绘制柱体实例，加入到图层空间");
#endif

	//创建管线实体
	LMALineDbObject* lmaLineObj = new LMALineDbObject( this );

	//保存管线实体
	SetLineEntity( ArxWrapper::PostToModelSpace(lmaLineObj, mLayerName) );

	//判断是否需要创建壁厚实体
	if( mCategoryData->mWallSize != L"0" 
		&& mCategoryData->mWallSize.length() != 0)
	{
		acutPrintf(L"\n当前管线有壁厚");

		//创建管线实体
		LMAWallLineDbObject* lmaWallLineObj = new LMAWallLineDbObject( this );

		//保存管线实体
		SetWallLineEntity( ArxWrapper::PostToModelSpace(lmaWallLineObj, mLayerName) );
	}

	//创建管线安全范围实体
	LMASafeLineDbObject* lmaSafeLineObj = new LMASafeLineDbObject( this );

	//默认安全范围实体不显示
	lmaSafeLineObj->setVisibility(AcDb::kInvisible);

	//保存管线安全范围实体
	SetSafeLineEntity( ArxWrapper::PostToModelSpace(lmaSafeLineObj, mLayerName) );

	return true;
}

bool PointDBEntityCollection::HasEntity( const AcDbObjectId& entityId ) const
{
	if( m_LineEntryId == entityId
		|| m_SafeLineEntityId == entityId
		|| m_DimEntityId == entityId
		|| m_MarkEntityId == entityId )
	{
		return true;
	}
	else
	{
		return false;
	}
}

PointDBEntityCollection::PointDBEntityCollection()
	:mLayerName()
	,mCategoryData(NULL)
	,mLineID(0)
	,mSequenceNO(0)
	,mStartPoint()
	,mEndPoint()
	,m_LineEntryId()
	,m_WallLineEntryId()
	,m_SafeLineEntityId()
	,m_DimEntityId()
	,m_MarkEntityId()
{}

void PointDBEntityCollection::DropEntityCollection()
{
	//得到线段的数据库对象ID
	if( m_LineEntryId.isValid() )
	{
#ifdef DEBUG
		acutPrintf(L"\n线段【%s】序号【%d】 坐标 x:【%0.2lf】y:【%0.2lf】z:【%0.2lf】被删除",
					mLayerName.c_str(), mSequenceNO, mEndPoint[X], mEndPoint[Y], mEndPoint[Z]);
#endif

		//删除线段对象
		acutPrintf(L"\n删除折线段实体.");
		ArxWrapper::RemoveDbObject(m_LineEntryId);

		//删除壁厚实体
		acutPrintf(L"\n删除折线段实体.");
		ArxWrapper::RemoveDbObject(m_WallLineEntryId);

		//删除折线段安全范围实体
		acutPrintf(L"\n删除折线段实体.");
		ArxWrapper::RemoveDbObject(m_SafeLineEntityId);
	}
}

} // end of data

} // end of assistant

} // end of guch

} // end of com