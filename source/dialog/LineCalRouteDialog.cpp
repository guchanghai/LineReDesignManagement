// LineCalRouteDialog.cpp : implementation file
//

#include "stdafx.h"
#include "LineCalRouteDialog.h"
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

// Hatch
#include <dbhatch.h>

// Leader
#include <dblead.h>
#include <dbmleader.h>

#include <ArxWrapper.h>

#include <ArxCustomObject.h>

#include <GlobalDataConfig.h>

#include <LMAUtils.h>

#pragma warning(disable:4482)

using namespace com::guch::assistant::arx;
using namespace com::guch::assistant::data;
using namespace com::guch::assistant::Intersect;

CString LineCalRouteDialog::m_lineCategory = L"自动路由管线";
CString LineCalRouteDialog::m_CutLayerName = L"";
CString LineCalRouteDialog::m_lineWidth = L"100";

LineCategoryItemData* LineCalRouteDialog::m_lineInfo = NULL;

AcDbObjectIdArray* LineCalRouteDialog::m_CutObjects = NULL;
LineEntity* LineCalRouteDialog::m_RouteLineEntity = NULL;
AcGePoint3dArray* LineCalRouteDialog::m_PointVertices = NULL;

LineEntityFile* LineCalRouteDialog::m_EntryFile = NULL;

// LineCalRouteDialog dialog

IMPLEMENT_DYNAMIC(LineCalRouteDialog, CAcUiDialog)

LineCalRouteDialog::LineCalRouteDialog(CWnd* pParent /*=NULL*/)
: CAcUiDialog(LineCalRouteDialog::IDD, pParent),
	m_startPoint(),
	m_endPoint()
{
	//得到当前管理的文档
	m_fileName = curDoc()->fileName();
	acutPrintf(L"\n对【%s】代表的管线实体计算最短路由.",m_fileName.c_str());

	//得到实体数据文件中的数据
	m_EntryFile = LineEntityFileManager::RegisterEntryFile(m_fileName);
}

LineCalRouteDialog::~LineCalRouteDialog()
{
}

BOOL LineCalRouteDialog::OnInitDialog()
{
	//和页面交互数据
	CAcUiDialog::OnInitDialog();

	//默认偏移为0
	m_StartX.SetWindowTextW(L"0.00");
	m_StartY.SetWindowTextW(L"0.00");
	m_StartZ.SetWindowTextW(L"0.00");

	m_EndX.SetWindowTextW(L"0.00");
	m_EndY.SetWindowTextW(L"0.00");
	m_EndZ.SetWindowTextW(L"0.00");

	//加载图片
	m_PickStart.AutoLoad();
	m_PickEnd.AutoLoad();

	return TRUE;
}

void LineCalRouteDialog::DoDataExchange(CDataExchange* pDX)
{
	CAcUiDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_ROUTE_START_X, m_StartX);
	DDX_Control(pDX, IDC_ROUTE_START_Y, m_StartY);
	DDX_Control(pDX, IDC_ROUTE_START_Z, m_StartZ);

	DDX_Control(pDX, IDC_ROUTE_END_X, m_EndX);
	DDX_Control(pDX, IDC_ROUTE_END_Y, m_EndY);
	DDX_Control(pDX, IDC_ROUTE_END_Z, m_EndZ);

	DDX_Control(pDX, IDC_ROUTE_PICK_START,m_PickStart);
	DDX_Control(pDX, IDC_ROUTE_PICK_END,m_PickEnd);
}

BEGIN_MESSAGE_MAP(LineCalRouteDialog, CAcUiDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)

	ON_BN_CLICKED(IDC_ROUTE_PICK_START, &LineCalRouteDialog::OnBnPickStartClicked)
	ON_BN_CLICKED(IDC_ROUTE_PICK_END, &LineCalRouteDialog::OnBnPickEndClicked)
END_MESSAGE_MAP()

void LineCalRouteDialog::OnBnPickStartClicked()
{
	// Hide the dialog and give control to the editor
	BeginEditorCommand();

	CString temp;
	ads_point pt;

	// Get a point
	if (acedGetPoint(NULL, _T("\n选取起始点: "), pt) == RTNORM) 
	{
		// If the point is good, continue
		CompleteEditorCommand();

		temp.Format(_T("%g"), pt[X]);
		m_StartX.SetWindowTextW(temp.GetBuffer());

		temp.Format(_T("%g"), pt[Y]);
		m_StartY.SetWindowTextW(temp.GetBuffer());

		temp.Format(_T("%g"), pt[Z]);
		m_StartZ.SetWindowTextW(temp.GetBuffer());

		m_startPoint.set(pt[X], pt[Y], pt[Z]);
	}
	else 
	{
		// otherwise cancel the command (including the dialog)
		CancelEditorCommand();
	}

	UpdateData(FALSE);
}

void LineCalRouteDialog::OnBnPickEndClicked()
{
	// Hide the dialog and give control to the editor
	BeginEditorCommand();

	CString temp;
	ads_point pt;

	// Get a point
	if (acedGetPoint(NULL, _T("\n选取终止点: "), pt) == RTNORM) 
	{
		// If the point is good, continue
		CompleteEditorCommand();

		temp.Format(_T("%g"), pt[X]);
		m_EndX.SetWindowTextW(temp.GetBuffer());

		temp.Format(_T("%g"), pt[Y]);
		m_EndY.SetWindowTextW(temp.GetBuffer());

		temp.Format(_T("%g"), pt[Z]);
		m_EndZ.SetWindowTextW(temp.GetBuffer());

		m_endPoint.set(pt[X], pt[Y], pt[Z]);
	}
	else 
	{
		// otherwise cancel the command (including the dialog)
		CancelEditorCommand();
	}

	UpdateData(FALSE);
}

void LineCalRouteDialog::GetStartEndPoint()
{
	CString pointX,pointY,pointZ;

	m_StartX.GetWindowTextW(pointX);
	acdbDisToF(pointX.GetBuffer(), -1, &m_startPoint.x);

	m_StartY.GetWindowTextW(pointY);
	acdbDisToF(pointY.GetBuffer(), -1, &m_startPoint.y);

	m_StartZ.GetWindowTextW(pointZ);
	acdbDisToF(pointZ.GetBuffer(), -1, &m_startPoint.z);

	m_EndX.GetWindowTextW(pointX);
	acdbDisToF(pointX.GetBuffer(), -1, &m_endPoint.x);

	m_EndY.GetWindowTextW(pointY);
	acdbDisToF(pointY.GetBuffer(), -1, &m_endPoint.y);

	m_EndZ.GetWindowTextW(pointZ);
	acdbDisToF(pointZ.GetBuffer(), -1, &m_endPoint.z);

	//当前系统运算按照从下往上计算
	//既下方为起始点，上方为截止点
	if( m_startPoint.y > m_endPoint.z )
	{
		AcGePoint3d swap(m_endPoint);

		m_endPoint.set( m_startPoint.x, m_startPoint.y, m_startPoint.z );
		m_startPoint.set( swap.x, swap.y, swap.z );
	}
}

void LineCalRouteDialog::OnBnClickedOk()
{
	//得到用户输入的数据
	UpdateData(FALSE);

	//得到起始、截止点
	GetStartEndPoint();

	//首先恢复视图
	CutBack();

	//创建前置条件（路由所在的图层《名称》，路由所代表的管线）
	SetupRouteLineEnv();

	//计算两点之间的(接近)最短路由;
	CalculateShortestRoute();

	//整理最终计算出的结果
	SetupRouteResult();

	//关闭对话框
	CAcUiDialog::OnOK();
}

bool LineCalRouteDialog::SetupRouteLineEnv()
{
	//创建图层的名称
	m_CutLayerName.Format(L"从(X:%0.2lf,Y:%0.2lf,Z:%0.2lf)到(X:%0.2lf,Y:%0.2lf,Z:%0.2lf)的最短路由",m_startPoint[X],m_startPoint[Y],m_startPoint[Z],m_endPoint[X],m_endPoint[Y],m_endPoint[Z]);
	acutPrintf(L"\n要创建的图层名称为【%s】",m_CutLayerName.GetBuffer());
	m_CutLayerName.Format(L"最短路由");

	//创建起始、终止点与X轴垂直的平面
	InitializeProjectPlace();

	//创建代表路由的管线实体
	InitializeRouteLine();

	return true;
}

void LineCalRouteDialog::InitializeProjectPlace()
{
	acutPrintf(L"\n最短路由所在的与X轴垂直的面");

	//得到起始点垂直于X面,竖直向下10000个像素
	AcGePoint3d projectPoint( m_startPoint );
	projectPoint.z -= 100;

	m_ProjectPlane = AcGePlane( m_startPoint, m_endPoint, projectPoint);
}

bool LineCalRouteDialog::InitializeRouteLine()
{
	acutPrintf(L"\n初始化路由所在的管线信息");

	//创建管线detail信息
	InitializeRouteLineInfo();

	//得到实体名称
	wstring pipeName(m_CutLayerName.GetBuffer());
	acutPrintf(L"\n新的路由所在图层的名字为【%s】", pipeName.c_str());

	//创建新的路由管线
	acutPrintf(L"\n创建新的路由管线");
	m_RouteLineEntity = new LineEntity(pipeName,GlobalData::CONFIG_LINE_KIND, m_lineInfo ,NULL);

	//生成该项的ID
	m_RouteLineEntity->m_LineID = (UINT)GetTickCount();

	//保存到数据库
	m_RouteLineEntity->m_dbId = ArxWrapper::PostToNameObjectsDict(m_RouteLineEntity->m_pDbEntry,LineEntity::LINE_ENTRY_LAYER);

	//清空数据库对象指针，由AutoCAD管理
	m_RouteLineEntity->m_pDbEntry = NULL;

	//保存数据到管理器
	m_EntryFile->InsertLine(m_RouteLineEntity);

	return true;
}

//初始化代表自动路由的基本信息
bool LineCalRouteDialog::InitializeRouteLineInfo()
{
	acutPrintf(L"\n初始化路由的基本信息");

	if( m_lineInfo == NULL )
	{
		CString lineWidth,lineHeight,lineReservedA,lineReservedB,
			lineSafeSize,lineWallSize,lineThroughDirect,
			linePlaneDesc,lineCutDesc;
	
		lineWidth = L"0";
		lineHeight = L"0";
		lineReservedA = L"0";
		lineReservedB = L"0";

		//准备配置数据结构体
		LineCategoryItemData* categoryData = new LineCategoryItemData();

		categoryData->mCategory = wstring(LineCalRouteDialog::m_lineCategory.GetBuffer());
		categoryData->mShape = GlobalData::LINE_SHAPE_CIRCLE;

		categoryData->mSize.mRadius = wstring(LineCalRouteDialog::m_lineWidth.GetBuffer());
		categoryData->mSize.mWidth = wstring(lineWidth.GetBuffer());
		categoryData->mSize.mHeight = wstring(lineHeight.GetBuffer());
		categoryData->mSize.mReservedA = wstring(lineReservedA.GetBuffer());
		categoryData->mSize.mReservedB = wstring(lineReservedB.GetBuffer());

		categoryData->mWallSize = wstring(LineCalRouteDialog::m_lineWidth.GetBuffer());
		categoryData->mSafeSize = wstring(LineCalRouteDialog::m_lineWidth.GetBuffer());

		categoryData->mPlaneMark = wstring(linePlaneDesc.GetBuffer());
		categoryData->mCutMark = wstring(lineCutDesc.GetBuffer());

		categoryData->mThroughDirection = wstring(lineThroughDirect.GetBuffer());

		//保存这个管线基本信息
		m_lineInfo = categoryData;
	}

	return true;
}

bool LineCalRouteDialog::InitializeStartEndPoints( const AcGePoint3d& startPoint, const AcGePoint3d& endPoint )
{
	acutPrintf(L"\n在3D模型中划出这条管线，进行比较计算");

	//以用户选择的开始、结束点初始化这条管线
	AppendStartEndPoints( startPoint, endPoint);

	//默认用户选择的开始点可以保存下来了
	SaveRouteLinePoint( startPoint );

	//没有计算过的线段
	m_CheckedEntities.clear();

	return true;
}

bool LineCalRouteDialog::SaveRouteLinePoint( const AcGePoint3d& newPoint )
{
	acutPrintf(L"\n保存(X:%0.2lf,Y:%0.2lf,Z:%0.2lf)为一个起点", newPoint[X], newPoint[Y], newPoint[Z]);

	if( m_PointVertices == NULL )
		m_PointVertices = new AcGePoint3dArray();

	m_PointVertices->append( newPoint);

	return true;
}

bool LineCalRouteDialog::AppendStartEndPoints(const AcGePoint3d& startPoint, const AcGePoint3d& endPoint)
{
	PointEntity* point = NULL;

	PointList* newPoints = new PointList();

	point = new PointEntity();
	point->m_PointNO =  0;
	point->m_Point[X] = startPoint[X];
	point->m_Point[Y] = startPoint[Y];
	point->m_Point[Z] = startPoint[Z];

	newPoints->push_back( point );

	point = new PointEntity();
	point->m_PointNO =  1;

	point->m_Point[X] = endPoint[X];
	point->m_Point[Y] = endPoint[Y];
	point->m_Point[Z] = endPoint[Z];

	newPoints->push_back( point );

	//以此开始和结束点创建新的路由线段
	m_RouteLineEntity->SetPoints(newPoints);

	acutPrintf(L"\n新的路由线段构造完成");

	return true;
}

/**
 * 首先连接起始和终止点之间的线段，然后判断与所有管线是否相侵。
 * 得到离自己最近的且相侵的管线。
 * 在此管线处，得到与较长线段垂直的面上高度为半径的点，连接起始点与此点，得到第一条线段。
 * 然后在此点计算新的起始点,重复上面的计算过程（递归），直到和所有的管线不相侵了，便结束了。
 */
void LineCalRouteDialog::CalculateShortestRoute()
{
	acutPrintf(L"\n开始从起点计算路由");

	//从用户选择的其实点开始
	m_newStartPoint = m_startPoint;

	int count = 0;

	//递归计算
	while( CalculateShortestRoute( m_newStartPoint, m_endPoint ) == false )
	{
		acutPrintf(L"\n发现当前管线与系统中的线段有相侵的现象，继续计算");

		if( count++ >= 1000 )
			break;
	}
}

bool LineCalRouteDialog::CalculateShortestRoute( const AcGePoint3d& start, const AcGePoint3d& end)
{
	acutPrintf(L"\n现在计算(X:%0.2lf,Y:%0.2lf,Z:%0.2lf)到(X:%0.2lf,Y:%0.2lf,Z:%0.2lf)的最短路由", start[X], start[Y], start[Z], end[X], end[Y], end[Z]);

	//以开始点和截止点创建默认宽度的管线
	InitializeStartEndPoints(start, end);

	//相侵的管线
	AcArray<PointEntity*>* intersectEntities = new AcArray<PointEntity*>();

	//与当前系统内的管线判断
	CheckIntersect(intersectEntities);
	acutPrintf(L"相交的管线有【%d】条",intersectEntities->length());

	if( intersectEntities->length() > 0  )
	{
		//有管线相侵，得到最近的一条
		PointEntity* nearestLine = GetNearestLineSegement(intersectEntities);

		//得到新起点
		AcGePoint3d newPoint = GetProjectPoint3d(nearestLine);

		//得到老起点与新起点之间的线段，加入到图层中；返回false继续
		m_newStartPoint = newPoint;

		//删除中间结果
		intersectEntities->removeAll();
		delete intersectEntities;
		intersectEntities = NULL;

		return false;
	}
	else
	{
		//如果没有管线相侵，则直接加入到图层中，返回true结束
		return true;
	}
}

PointEntity* LineCalRouteDialog::GetNearestLineSegement( AcArray<PointEntity*>* intersectEntities )
{
	acutPrintf(L"\n寻找离X轴最近的相交管线");

	if( intersectEntities == NULL )
		return NULL;

	if( intersectEntities->length() == 1 )
		return intersectEntities->at(0);

	PointEntity* nearestEntity = NULL;
	AcGePoint3d nearestPoint;
	bool findNearer = false;

	for( int i = 0; i < intersectEntities->length(); i++ )
	{
		PointEntity* pointEntity = intersectEntities->at(i);
		AcGePoint3d start = pointEntity->m_DbEntityCollection.mStartPoint;
		AcGePoint3d end = pointEntity->m_DbEntityCollection.mEndPoint;
		AcGeLine3d intersectLine( start, end ); 

		AcGePoint3d resultPnt;
		if( m_ProjectPlane.intersectWith(intersectLine, resultPnt) )
		{
			acutPrintf(L"\n交点为【X:%0.2lf,Y:%0.2lf,Z:%0.2lf】", resultPnt[X], resultPnt[Y], resultPnt[Z]);
			if( nearestEntity == NULL ||  resultPnt[Y] <nearestPoint[Y] )
			{
				acutPrintf(L"\n上次相近交点为【X:%0.2lf,Y:%0.2lf,Z:%0.2lf】", nearestPoint[X], nearestPoint[Y], nearestPoint[Z]);
				nearestEntity = pointEntity;
				nearestPoint = resultPnt;

				acutPrintf(L"\n管线【%s】的第【%d】条管线为更近的相交线",nearestEntity->m_DbEntityCollection.mLayerName.c_str(), nearestEntity->m_PointNO);
			}				
		}
	}

	//一旦某个线段被越过，则不再重复计算
	if( nearestEntity )
	{
		m_CheckedEntities.insert( LinePointID(nearestEntity->m_DbEntityCollection.mLineID, nearestEntity->m_DbEntityCollection.mSequenceNO) );
	}

	return nearestEntity;
}

AcGePoint3d LineCalRouteDialog::GetProjectPoint3d(PointEntity* lineSegment)
{
	acutPrintf(L"\n得到管线的垂直于X面的点");

	AcGePoint3d start = lineSegment->m_DbEntityCollection.mStartPoint;
	AcGePoint3d end = lineSegment->m_DbEntityCollection.mEndPoint;
	AcGeLine3d intersectLine( start, end ); 

	AcGePoint3d resultPnt;
	m_ProjectPlane.intersectWith(intersectLine, resultPnt);

	acutPrintf(L"\n得到管线的相交点【X:%0.2lf,Y:%0.2lf,Z:%0.2lf】", resultPnt[X], resultPnt[Y], resultPnt[Z]);

	double wallSize = 0.0;
	acdbDisToF(lineSegment->m_DbEntityCollection.mCategoryData->mWallSize.c_str(), -1, &wallSize);
	wallSize /= 1000;

	double safeSize = 0.0;
	acdbDisToF(lineSegment->m_DbEntityCollection.mCategoryData->mSafeSize.c_str(), -1, &safeSize);
	safeSize /= 1000;

	double yOffset = 0.0;
	double zOffset = 0.0;

	if( lineSegment->m_DbEntityCollection.mCategoryData->mShape == GlobalData::LINE_SHAPE_CIRCLE )
	{
		//圆体的话，取半径为Y,Z轴的偏离
		acdbDisToF(lineSegment->m_DbEntityCollection.mCategoryData->mSize.mRadius.c_str(), -1, &yOffset);
		yOffset /= 1000;
		yOffset += wallSize;
		yOffset += safeSize;

		zOffset = yOffset;
	}
	else
	{
		//柱体的话，取宽度为Y轴的偏离
		acdbDisToF(lineSegment->m_DbEntityCollection.mCategoryData->mSize.mWidth.c_str(), -1, &yOffset);
		yOffset /= 1000;
		yOffset += wallSize;
		yOffset += safeSize;

		//取高度的一半为Z轴的偏离
		acdbDisToF(lineSegment->m_DbEntityCollection.mCategoryData->mSize.mHeight.c_str(), -1, &zOffset);
		zOffset /= 2000;
		zOffset += wallSize;
		zOffset += safeSize;
	}

	AcGePoint3d projectPoint;
	projectPoint.x = resultPnt.x;
	projectPoint.y = resultPnt.y - yOffset;
	projectPoint.z = resultPnt.z + zOffset;

	//保存离X轴较近的点
	acutPrintf(L"\n存储下方的点【X:%0.2lf,Y:%0.2lf,Z:%0.2lf】为一个起点", projectPoint[X], projectPoint[Y], projectPoint[Z]);
	m_PointVertices->append( projectPoint );

	projectPoint.y = resultPnt.y + yOffset;

	acutPrintf(L"\n返回上方的点【X:%0.2lf,Y:%0.2lf,Z:%0.2lf】为新的计算起点", projectPoint[X], projectPoint[Y], projectPoint[Z]);

	return projectPoint;
}

//判断一条折线段与其他管线的相侵情况
void LineCalRouteDialog::CheckIntersect(AcArray<PointEntity*>* intersectEntities)
{
	acutPrintf(L"\n进行相侵判断.");

	PointList* pointList = m_RouteLineEntity->m_PointList;
	if( pointList == NULL 
		|| pointList->size() < 2 )
	{
		acutPrintf(L"\n当前管线没有折线段，可以不进行检查");
		return;
	}

	PointEntity* checkPoint = (*pointList)[1];

	wstring& lineName = checkPoint->m_DbEntityCollection.mLayerName;
	Adesk::Int32& checkLineID = checkPoint->m_DbEntityCollection.mLineID;
	Adesk::Int32& checkSeqNO = checkPoint->m_DbEntityCollection.mSequenceNO;

#ifdef DEBUG
	acutPrintf(L"\n对【%s】的第【%d】条进行相侵判断.",lineName.c_str(), checkSeqNO);
#endif

	LineList* lineList = m_EntryFile->GetList();
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
				acutPrintf(L"\n管线起始点，不需要判断");
				continue;
			}

			acutPrintf(L"\n与【%s】的第【%d】条折线段进行判断",(*point)->m_DbEntityCollection.mLayerName.c_str(), seqNO );

			if( lineID == checkLineID && abs( seqNO - checkSeqNO ) <= 1 )
			{
				acutPrintf(L"\n相邻线段,不进行相侵判断");
				continue;
			}

			if( m_CheckedEntities.find(LinePointID(lineID,seqNO)) != m_CheckedEntities.end() )
			{
				acutPrintf(L"\n此折线段已被比较过,忽略");
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

			//判断2者是否相侵
			AcDb3dSolid* intersetObj = ArxWrapper::GetInterset( pSafeLine, pCheckSafeLine );

			if( intersetObj != NULL )
			{
				acutPrintf(L"\n与【%s】的第【%d】条折线段侵限，需要记录下来！",(*point)->m_DbEntityCollection.mLayerName.c_str(), seqNO );

				//保存检测结果
				intersectEntities->append(*point);
			}
			else
			{
				acutPrintf(L"\n与【%s】的第【%d】条折线段没有相侵！",(*point)->m_DbEntityCollection.mLayerName.c_str(), seqNO );
			}

			pSafeLine->close();
			pCheckSafeLine->close();

			ArxWrapper::UnLockCurDoc();
		}
	}
}

bool LineCalRouteDialog::CreateRouteSegment( const AcGePoint3d& start, const AcGePoint3d& end)
{
	return true;
}

void LineCalRouteDialog::SetupRouteResult()
{
	m_PointVertices->append( m_endPoint);
	acutPrintf(L"\n整理最终的路由结果，有线段【%d】条",m_PointVertices->length()-1);

	PointList* newPoints = new PointList();

	CString temp;
	for( int i = 0; i < m_PointVertices->length(); i++ )
	{
		PointEntity* point = new PointEntity();
		point->m_PointNO = i;
		
		point->m_Point[X] = m_PointVertices->at(i).x;
		point->m_Point[Y] = m_PointVertices->at(i).y;
		point->m_Point[Z] = m_PointVertices->at(i).z;

		newPoints->push_back( point );
	}

	m_RouteLineEntity->SetPoints( newPoints );
}

void LineCalRouteDialog::Reset()
{
	acutPrintf(L"\n存在的临时图层为【%s】！",m_CutLayerName.GetBuffer());

	if( m_CutLayerName.GetLength() > 0 )
	{
		acutPrintf(L"\n首先锁定该文档");
		ArxWrapper::LockCurDoc();

		//acutPrintf(L"\n恢复WCS视窗");
		acedCommand(RTSTR, _T("UCS"), RTSTR, L"W", 0);

		acutPrintf(L"\n删除计算路由相关的对象");
		CutBack();

		acutPrintf(L"\n删除计算出的路由所在的图层");
		if( ArxWrapper::DeleteLayer(m_CutLayerName.GetBuffer(),true) )
		{
			acutPrintf(L"\n初始化设置");
			m_CutLayerName.Format(L"");
		}

		acutPrintf(L"\n显示其他图层");
		ArxWrapper::ShowLayer(L"");

		acutPrintf(L"\n解除文档锁定");
		ArxWrapper::UnLockCurDoc();

		acutPrintf(L"\n切换至俯视");
		ArxWrapper::ChangeView(3);
	}
	else
	{
		acutPrintf(L"\n当前系统内没有切图！");
	}
}

void LineCalRouteDialog::CutBack()
{
	//删除前一条计算路由时的管线实体
	if( m_RouteLineEntity )
	{
		//从数据库删除管线本身
		ArxWrapper::DeleteFromNameObjectsDict(m_RouteLineEntity->m_dbId,LineEntity::LINE_ENTRY_LAYER);

		//从数据库删除管线所有的线段
		m_RouteLineEntity->EraseDbObjects();

		//删除所有的内存节点
		m_RouteLineEntity->ClearPoints();

		//删除线段集合
		m_EntryFile->DeleteLine(m_RouteLineEntity->GetLineID());

		//删除折线点集合
		m_PointVertices->removeAll();

		delete m_RouteLineEntity;
	}
}

// LineCalRouteDialog message handlers
