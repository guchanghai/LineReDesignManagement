// ------------------------------------------------
//                  LineManagementAssistant
// Copyright 2012-2013, Chengyong Yang & Changhai Gu. 
//               All rights reserved.
// ------------------------------------------------
//	ArxWrapper.h
//	written by Changhai Gu
// ------------------------------------------------
// $File:\\LineManageAssitant\main\source\intersect\LineIntersectManage.cpp $
// $Author: Changhai Gu $
// $DateTime: 2013/4/13 06:13:00
// $Revision: #1 $
// ------------------------------------------------
//

#include "stdafx.h"

#include "LineIntersectManage.h"

#include "acedads.h"
#include "accmd.h"
#include <adscodes.h>

#include <adsdlg.h>

#include <dbapserv.h>

#include <dbregion.h>

#include <gepnt3d.h>

//symbol table
#include <dbsymtb.h>

#include <acdocman.h>

//3D Object
#include <dbsol3d.h>

#include <ArxWrapper.h>

#include <ArxCustomObject.h>

#include <GlobalDataConfig.h>

#include <LMAUtils.h>

using namespace com::guch::assistant::arx;

namespace com
{

namespace guch
{

namespace assistant
{

namespace Intersect
{
LineIntersectManage* LineIntersectManage::mLineIntersectInstance = NULL;

LineIntersectManage* LineIntersectManage::Instance()
{
	if( mLineIntersectInstance == NULL )
		mLineIntersectInstance = new LineIntersectManage();

	return mLineIntersectInstance;
}

LineIntersectManage::LineIntersectManage()
	:mIntersectEntities()
	,m_pCheckLine(NULL)
{
}

LineIntersectManage::~LineIntersectManage()
{
	Clear();
}

void LineIntersectManage::Clear()
{
	for( int i = 0; i < mIntersectEntities.length(); i++ )
	{
		delete mIntersectEntities[i];
	}

	mIntersectEntities.removeAll();
}

void LineIntersectManage::CheckInteract()
{
	//得到管线实体文件管理器
	m_pCheckLine = LineEntityFileManager::GetCurrentLineEntryFile();

	acutPrintf(L"\n对管线文件【%s】进行亲限判断.",m_pCheckLine->m_FileName.c_str());

	//删除上一次的判断结果
	Reset();

	//对各条管线进行判断
	CheckLineInteract(  );
}

void LineIntersectManage::CheckLineInteract()
{
	//清空已比较集合
	m_CheckedEntities.clear();

	LineList lineList = m_pCheckLine->GetList(GlobalData::KIND_LINE);
	if( lineList.size() == 0 )
	{
		acutPrintf(L"\n当前文件内无管线，可以不进行相侵检查");
		return;
	}

	for( LineIterator line = lineList.begin();
			line != lineList.end();
			line++ )
	{
		PointList* pointList = (*line)->m_PointList;
		if( pointList == NULL 
			|| pointList->size() == 0 )
		{
			acutPrintf(L"\n当前管线没有折线段，可以不进行检查");
			continue;
		}

		for( PointIter point = pointList->begin();
				point != pointList->end();
				point++ )
		{
			if( (*point)->m_DbEntityCollection.mSequenceNO == 0 )
			{
				acutPrintf(L"\n第一个点没有管线，它只是个起点.");
				continue;
			}

			//与当前文件中所有的其他直线做侵限判断
			CheckLineInteract( *point );

			//加入已比较队列，避免重复比较
			m_CheckedEntities.insert( LinePointID((*point)->m_DbEntityCollection.mLineID, (*point)->m_DbEntityCollection.mSequenceNO) );
		}
	}
}

//判断一条折线段与其他管线的相侵情况
void LineIntersectManage::CheckLineInteract( PointEntity* checkPoint )
{
	wstring& lineName = checkPoint->m_DbEntityCollection.mLayerName;
	Adesk::Int32& checkLineID = checkPoint->m_DbEntityCollection.mLineID;
	Adesk::Int32& checkSeqNO = checkPoint->m_DbEntityCollection.mSequenceNO;

#ifdef DEBUG
	acutPrintf(L"\n对【%s】的第【%d】条进行相侵判断.",lineName.c_str(), checkSeqNO);
#endif

	LineList* lineList = m_pCheckLine->GetList();
	for( LineIterator line = lineList->begin();
			line != lineList->end();
			line++ )
	{
		PointList* pointList = (*line)->m_PointList;
		if( pointList == NULL 
			|| pointList->size() == 0 )
		{
			acutPrintf(L"\n当前管线没有折线段，不需要与此折线段进行相侵判断");
			continue;
		}

		for( PointIter point = pointList->begin();
				point != pointList->end();
				point++ )
		{
			Adesk::Int32& lineID = (*point)->m_DbEntityCollection.mLineID;
			Adesk::Int32& seqNO = (*point)->m_DbEntityCollection.mSequenceNO;
			if( seqNO == 0 )
			{
				//acutPrintf(L"\n管线起始点，不需要判断");
				continue;
			}

			//acutPrintf(L"\n与【%s】的第【%d】条折线段进行判断",(*point)->m_DbEntityCollection.mLayerName.c_str(), seqNO );

			if( lineID == checkLineID && abs( seqNO - checkSeqNO ) <= 1 )
			{
				//acutPrintf(L"\n相邻线段,不进行相侵判断");
				continue;
			}

			if( m_CheckedEntities.find(LinePointID(lineID,seqNO)) != m_CheckedEntities.end() )
			{
				//acutPrintf(L"\n此折线段已被比较过,忽略");
				continue;
			}

			ArxWrapper::LockCurDoc();

			//得到两个线段的数据库安全范围对象
			AcDbEntity *pSafeLine = ArxWrapper::GetDbObject( (*point)->m_DbEntityCollection.GetSafeLineEntity(), true );
			if( pSafeLine == NULL )
			{
				acutPrintf(L"\n遍历检查侵限的管线的安全范围实体时出错");
				ArxWrapper::UnLockCurDoc();
				continue;
			}

			AcDbEntity *pCheckSafeLine = ArxWrapper::GetDbObject( checkPoint->m_DbEntityCollection.GetSafeLineEntity(), true );
			if( pCheckSafeLine == NULL )
			{
				if( pSafeLine )
					pSafeLine->close();

				acutPrintf(L"\n得到被检查侵限的管线的安全范围实体时出错");
				ArxWrapper::UnLockCurDoc();
				continue;
			}

			//得到2者的相侵的部位
			AcDb3dSolid* intersetObj = ArxWrapper::GetInterset( pSafeLine, pCheckSafeLine );

			if( intersetObj != NULL )
			{
				acutPrintf(L"\n与【%s】的第【%d】条折线段侵限，设置为警告色！",(*point)->m_DbEntityCollection.mLayerName.c_str(), seqNO );

				//保存检测结果
				IntersectStruct* pIntersect = new IntersectStruct();
				pIntersect->intersetcA = checkPoint;
				pIntersect->intersetcB = (*point);

				//隐藏的安全范围管线可见，设置为红色
				//if( pSafeLine->visibility() == AcDb::kInvisible )
				{
					//pSafeLine->setVisibility(AcDb::kVisible);
					(*point)->m_DbEntityCollection.SetLineWarning();
				}

				//if( pCheckSafeLine->visibility() == AcDb::kInvisible )
				{
					//pCheckSafeLine->setVisibility(AcDb::kVisible);
					checkPoint->m_DbEntityCollection.SetLineWarning();
				}

				//相交的区域设置为红色
				intersetObj->setColorIndex(GlobalData::INTERSET_COLOR);
				pIntersect->intersctcId = ArxWrapper::PostToModelSpace( intersetObj, lineName );

				//用于恢复操作
				mIntersectEntities.append(pIntersect);
			}

			pSafeLine->close();
			pCheckSafeLine->close();

			ArxWrapper::UnLockCurDoc();
		}
	}
}

void LineIntersectManage::Reset()
{
	//对保存的结果中进行处理
	for( int i = 0; i < mIntersectEntities.length(); i++ )
	{
		ArxWrapper::LockCurDoc();

		IntersectStruct* intersect = mIntersectEntities[i];

		//将安全范围实体隐藏
		if( intersect->intersetcA )
		{
			//AcDbObjectId intersectAId = intersect->intersetcA->m_DbEntityCollection.GetSafeLineEntity();
			//ArxWrapper::ShowDbObject(intersectAId, AcDb::kInvisible );

			intersect->intersetcA->m_DbEntityCollection.SetLineWarning(false);
		}

		if( intersect->intersetcB )
		{
			//AcDbObjectId intersectBId = intersect->intersetcB->m_DbEntityCollection.GetSafeLineEntity();
			//ArxWrapper::ShowDbObject(intersectBId, AcDb::kInvisible );

			intersect->intersetcB->m_DbEntityCollection.SetLineWarning(false);
		}

		//删除相交的实体
		ArxWrapper::RemoveDbObject(intersect->intersctcId);

		ArxWrapper::UnLockCurDoc();
	}

	//清空结果
	Clear();
}

} // end of Intersect

} // end of assistant

} // end of guch

} // end of com
