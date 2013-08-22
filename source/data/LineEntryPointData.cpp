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
	m_DbEntityCollection.DrawEntityCollection(pLineEntity->m_LinePriority);
}

/**
 * 根据起始点队列（向量列表），并放置在特定的层上
 **/
bool PointDBEntityCollection::DrawEntityCollection(GlobalData::LineProirity proirity) 
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
		if( proirity == GlobalData::LINE_FIRST )
		{
			lmaWallLineObj->setColorIndex(GlobalData::INTERSET_WALLLINE_COLOR );
		}

		//保存管线实体
		SetWallLineEntity( ArxWrapper::PostToModelSpace(lmaWallLineObj, mLayerName) );
	}

	//创建管线安全范围实体
	LMASafeLineDbObject* lmaSafeLineObj = new LMASafeLineDbObject( this );

	//若是特殊管线，如自动路由管线，则显示为黄色
	if( proirity == GlobalData::LINE_FIRST )
	{
		lmaSafeLineObj->setColorIndex(GlobalData::INTERSET_WALLLINE_COLOR );
	}
	else if(  proirity == GlobalData::LINE_SECOND )
	{
		lmaSafeLineObj->setColorIndex(GlobalData::SAFELINE_COLOR );
	}
	else
	{
		//默认安全范围实体不显示
		lmaSafeLineObj->setVisibility(AcDb::kInvisible);
	}

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

void PointDBEntityCollection::SetLineWarning( bool warning )
{
	if( warning )
		acutPrintf(L"\n设置管线最外层实体的颜色.");
	else
		acutPrintf(L"\n恢复管线最外层实体的颜色.");

	if( m_WallLineEntryId.isValid())
	{
		AcDbEntity* pWallLine = ArxWrapper::GetDbObject( m_WallLineEntryId, true );
		if( pWallLine )
		{
			if( warning )
			{
				acutPrintf(L"\n设置壁厚实体的颜色为红色.");
				pWallLine->setColorIndex( GlobalData::INTERSET_COLOR );
			}
			else
			{
				acutPrintf(L"\n恢复壁厚实体的颜色.");
				pWallLine->setColorIndex( GlobalData::WALLLINE_COLOR );
			}

			pWallLine->close();
		}
	}
	else if( m_LineEntryId.isValid())
	{
		AcDbEntity* pLine = ArxWrapper::GetDbObject( m_LineEntryId, true );
		if( pLine )
		{
			if( warning )
			{
				acutPrintf(L"\n设置管线实体的颜色为红色.");
				pLine->setColorIndex( GlobalData::INTERSET_COLOR );
			}
			else
			{
				acutPrintf(L"\n恢复管线实体的颜色.");
				pLine->setColorIndex( GlobalData::LINE_COLOR );
			}

			pLine->close();
		}
	}
	else
	{
		acutPrintf(L"\n壁厚或者实体没有ID.");
	}
}

AcGePlane& PointDBEntityCollection::GetAroundPlane(int direction)
{
	if( !m_LineAroundEntity.bInitialized )
	{
		CalculatePanel();
	}

	switch(direction)
	{
	case 1:
		return m_LineAroundEntity.mFrontPlane;
	case 2:
		return m_LineAroundEntity.mBackPlane;
	case 3:
		return m_LineAroundEntity.mRightPlane;
	case 4:
		return m_LineAroundEntity.mLeftPlane;
	case 5:
		return m_LineAroundEntity.mTopPlane;
	case 6:
		return m_LineAroundEntity.mBottomPlane;
	default:
		return m_LineAroundEntity.mFrontPlane;
	}
}

void PointDBEntityCollection::CalculatePanel()
{
	double xOffset = 0, yOffset = 0, zOffset = 0;
	double radius = 0.0, height = 0.0, width = 0.0,
		wallSize = 0.0, safeSize = 0.0;

	acdbDisToF(mCategoryData->mSize.mRadius.c_str(), -1, &radius);
	radius /= 1000;

	acdbDisToF(mCategoryData->mSize.mHeight.c_str(), -1, &height);
	height /= 1000;

	acdbDisToF(mCategoryData->mSize.mWidth.c_str(), -1, &width);
	width /= 1000;

	acdbDisToF(mCategoryData->mWallSize.c_str(), -1, &wallSize);
	wallSize /= 1000;

	acdbDisToF(mCategoryData->mSafeSize.c_str(), -1, &safeSize);
	safeSize /= 1000;

	//绘制圆柱体
	if( mCategoryData->mShape == GlobalData::LINE_SHAPE_CIRCLE )
	{
		yOffset = xOffset = radius + wallSize + safeSize;
	}
	else //绘制圆柱体
	{
		xOffset = width/2 + wallSize + safeSize;
		yOffset = height/2 + wallSize + safeSize;
	}

	//长度为柱体的高度
	zOffset = this->mStartPoint.distanceTo(this->mEndPoint);

	//得到8个顶点
	AcGePoint3d topLeftFront(-xOffset,-yOffset,0),
		topRightFront(-xOffset,-yOffset,zOffset),
		bottomLeftFront(xOffset,-yOffset,0),
		bottomRightFront(xOffset,-yOffset,zOffset),
		topLeftBack(-xOffset,yOffset,0),
		topRightBack(-xOffset,yOffset,zOffset),
		bottomLeftBack(xOffset,yOffset,0),
		bottomRightBack(xOffset,yOffset,zOffset);

	//计算12条棱
	//Line Horinzional
	m_LineAroundEntity.mLineFrontTop = AcGeLineSeg3d(topLeftFront,topRightFront);
	m_LineAroundEntity.mLineFrontBottom = AcGeLineSeg3d(bottomLeftFront,bottomRightFront);
	m_LineAroundEntity.mLineBackTop = AcGeLineSeg3d(topLeftBack,topRightBack);
	m_LineAroundEntity.mLineBackBottom = AcGeLineSeg3d(topLeftFront,topRightFront);

	//Line Vertical
	m_LineAroundEntity.mLineLeftFront = AcGeLineSeg3d(topLeftFront,bottomLeftFront);
	m_LineAroundEntity.mLineRightFront = AcGeLineSeg3d(topRightFront,bottomRightFront);
	m_LineAroundEntity.mLineLeftBack = AcGeLineSeg3d(topLeftBack,bottomLeftBack);
	m_LineAroundEntity.mLineRightBack = AcGeLineSeg3d(topRightBack,bottomRightBack);

	//Line Depth
	m_LineAroundEntity.mLineLeftTop = AcGeLineSeg3d(topLeftFront,topLeftBack);
	m_LineAroundEntity.mLineLeftBottom = AcGeLineSeg3d(bottomLeftFront,bottomLeftBack);
	m_LineAroundEntity.mLineRightTop = AcGeLineSeg3d(topRightFront,topRightBack);
	m_LineAroundEntity.mLineRightBottom = AcGeLineSeg3d(bottomRightFront,bottomRightBack);

	AcGePoint3d frontPlanePointStart(xOffset, 0, 10);
	AcGePoint3d frontPlanePointEnd(xOffset, 0, 0);
	AcGePoint3d frontPlanePointEndCopy(xOffset, 10, 0);

	m_LineAroundEntity.mFrontPlane = AcGePlane( frontPlanePointStart, frontPlanePointEnd, frontPlanePointEndCopy);

	AcGeMatrix3d rotateMatrix;
	double angle;

	//得到与该线段和Z轴的坐在平面垂直的向量
	AcGeVector3d line3dVector(mEndPoint.x - mStartPoint.x, mEndPoint.y - mStartPoint.y, mEndPoint.z - mStartPoint.z);
	AcGeVector3d rotateVctor = line3dVector.crossProduct(AcGeVector3d::kZAxis);

	//得到上诉垂直向量与X轴的夹角
	angle = rotateVctor.angleTo(AcGeVector3d::kXAxis); // 线段位于第二，三象限时，垂直向量与X轴的夹角与该线段在第一，四象限时的夹角相等，估需区分
	if( mStartPoint.x < mEndPoint.x )
		angle = -angle;

	//进行旋转，放置柱体倾斜
	rotateMatrix = AcGeMatrix3d::rotation( angle, AcGeVector3d::kZAxis, AcGePoint3d::kOrigin);
	TransformBy(rotateMatrix);

	//得到柱体进行旋转的角度
	angle = -line3dVector.angleTo(AcGeVector3d::kZAxis);

	//进行旋转
	rotateMatrix = AcGeMatrix3d::rotation( angle, rotateVctor, AcGePoint3d::kOrigin);
	TransformBy(rotateMatrix);

	//得到线段的中心点
	AcGePoint3d center( mStartPoint.x + mEndPoint.x, mStartPoint.y + mEndPoint.y, mStartPoint.z + mEndPoint.z); 
	center /= 2;

	//进行偏移
	AcGeMatrix3d moveMatrix;
	moveMatrix.setToTranslation(AcGeVector3d(center.x,center.y,center.z));

	//最终成型
	TransformBy(moveMatrix);

	//Set initialized status
	this->m_LineAroundEntity.bInitialized = true;
}

void PointDBEntityCollection::TransformBy( const AcGeMatrix3d& matrix )
{
	//Line Horinzional
	m_LineAroundEntity.mLineFrontTop.transformBy(matrix);
	m_LineAroundEntity.mLineFrontBottom.transformBy(matrix);
	m_LineAroundEntity.mLineBackTop.transformBy(matrix);
	m_LineAroundEntity.mLineBackBottom.transformBy(matrix);

	//Line Vertical
	m_LineAroundEntity.mLineLeftFront.transformBy(matrix);
	m_LineAroundEntity.mLineRightFront.transformBy(matrix);
	m_LineAroundEntity.mLineLeftBack.transformBy(matrix);
	m_LineAroundEntity.mLineRightBack.transformBy(matrix);

	//Line Depth
	m_LineAroundEntity.mLineLeftTop.transformBy(matrix);
	m_LineAroundEntity.mLineLeftBottom.transformBy(matrix);
	m_LineAroundEntity.mLineRightTop.transformBy(matrix);
	m_LineAroundEntity.mLineRightBottom.transformBy(matrix);

	m_LineAroundEntity.mFrontPlane.transformBy(matrix);
}

} // end of data

} // end of assistant

} // end of guch

} // end of com