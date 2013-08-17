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

typedef map<AcGePoint3dArray*, LineCalRouteDialog::CAL_STATUS>::iterator LineIter;
typedef map<LineEntity*, LineCalRouteDialog::CAL_STATUS>::iterator LineEntityIter;

CString LineCalRouteDialog::m_lineCategory = L"自动路由管线";
CString LineCalRouteDialog::m_CutLayerName = L"";
CString LineCalRouteDialog::m_lineWidth = L"100";

LineCategoryItemData* LineCalRouteDialog::m_lineInfo = NULL;

AcDbObjectIdArray* LineCalRouteDialog::m_CutObjects = NULL;

//All possible line database entities 
list<LineEntity*> LineCalRouteDialog::m_AllPossibleLineEntities = list<LineEntity*>();

//可能的路由在最初状态为0条
map<AcGePoint3dArray*, LineCalRouteDialog::CAL_STATUS> LineCalRouteDialog::m_lPossibleRoutes
	= map<AcGePoint3dArray*, LineCalRouteDialog::CAL_STATUS>();

map<LineEntity*, LineCalRouteDialog::CAL_STATUS> LineCalRouteDialog::m_lPossibleLineEntities
	= map<LineEntity*, LineCalRouteDialog::CAL_STATUS>();

LineEntityFile* LineCalRouteDialog::m_EntryFile = NULL;

// LineCalRouteDialog dialog

IMPLEMENT_DYNAMIC(LineCalRouteDialog, CAcUiDialog)

LineCalRouteDialog::LineCalRouteDialog(CWnd* pParent /*=NULL*/)
: CAcUiDialog(LineCalRouteDialog::IDD, pParent),
	m_startPoint(),
	m_endPoint(),
	m_DrawRealTime(false),
	m_CurrentRouteLineEntity(NULL),
	m_CompareLineSegmentEntity(NULL),
	m_CurrentPointVertices(NULL),
	m_ShortestPointVertices(NULL)
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
	if( m_startPoint.y > m_endPoint.y )
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
	SetupFinalResult();

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

	//创建代表当前路由的管线实体
	InitializeRouteLine();

	//Initialize the possible route lines;
	InitializePossibleLines();

	//Use the start point user selected to set up the first new possible route.
	AppendInterSegment(m_startPoint);

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

	//创建临时路由管线，用于当前的计算
	acutPrintf(L"\n创建临时路由管线");
	m_CompareLineSegmentEntity = CreateNewLineEntity();

	return true;
}

LineEntity* LineCalRouteDialog::CreateNewLineEntity()
{
	//得到实体名称
	wstring pipeName(m_CutLayerName.GetBuffer());
	acutPrintf(L"\n新的路由所在图层的名字为【%s】", pipeName.c_str());

	LineEntity* lineEntity = new LineEntity(pipeName,GlobalData::CONFIG_LINE_KIND, m_lineInfo , (new PointList()));

	//特殊管线，需要标示
	lineEntity->m_LinePriority = GlobalData::LINE_FIRST;

	//生成该项的ID
	lineEntity->m_LineID = (UINT)GetTickCount();

	//保存到数据库
	lineEntity->m_dbId = ArxWrapper::PostToNameObjectsDict(lineEntity->m_pDbEntry,LineEntity::LINE_ENTRY_LAYER);

	//清空数据库对象指针，由AutoCAD管理
	lineEntity->m_pDbEntry = NULL;

	//保存数据到管理器
	m_EntryFile->InsertLine(lineEntity);

	return lineEntity;
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

bool LineCalRouteDialog::InitializePossibleLines()
{
	if( m_DrawRealTime )
	{
		//Create the first possible rout line entity
		m_CurrentRouteLineEntity = CreateNewLineEntity();

		//Start with the first possible line
		m_lPossibleLineEntities.insert(std::pair<LineEntity*,CAL_STATUS>(m_CurrentRouteLineEntity,INIT));
	}
	else
	{
		//Create the first possible route line. 
		m_CurrentPointVertices = new AcGePoint3dArray();

		//Start with the first possible line
		m_lPossibleRoutes.insert(std::pair<AcGePoint3dArray*,CAL_STATUS>(m_CurrentPointVertices,INIT));
	}

	return true;
}

bool LineCalRouteDialog::InitializeCompareSegmentEntity( const AcGePoint3d& startPoint, const AcGePoint3d& endPoint )
{
	acutPrintf(L"\n在3D模型中划出这条管线，进行比较计算");

	//以用户选择的开始、结束点初始化这条管线
	CreateCompareLineSegement( startPoint, endPoint);

	//默认用户选择的开始点可以保存下来了
	//SaveRouteLinePoint( startPoint );

	//没有计算过的线段
	m_CheckedEntities.clear();

	return true;
}

bool LineCalRouteDialog::SaveRouteLinePoint( const AcGePoint3d& newPoint )
{
	acutPrintf(L"\n保存(X:%0.2lf,Y:%0.2lf,Z:%0.2lf)为一个起点", newPoint[X], newPoint[Y], newPoint[Z]);

	AppendInterSegment( newPoint);

	return true;
}

bool LineCalRouteDialog::CreateCompareLineSegement(const AcGePoint3d& startPoint, const AcGePoint3d& endPoint)
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
	m_CompareLineSegmentEntity->SetPoints(newPoints);

	acutPrintf(L"\n新的路由线段构造完成");

	return true;
}

/**
 * Iterator all the possible route lines, and find the unfinished 
 */
bool LineCalRouteDialog::GetPossibleStartPoint(AcGePoint3d& startPoint)
{
	bool hasStartPoint(false);

	if( m_DrawRealTime )
	{
		acutPrintf(L"\n当前可能的路由数为【%d】",m_lPossibleLineEntities.size());

		for( LineEntityIter iter = m_lPossibleLineEntities.begin(); 
			iter != m_lPossibleLineEntities.end();
			iter++ )
		{
			if( (*iter).second != DONE )
			{
				m_CurrentRouteLineEntity = (*iter).first;

				if( m_CurrentRouteLineEntity )
				{
					int length = static_cast<int>(m_CurrentRouteLineEntity->m_PointList->size());
					ads_point& curStartPoint = m_CurrentRouteLineEntity->m_PointList->at(length-1)->m_Point;

					startPoint.x = curStartPoint[X];
					startPoint.y = curStartPoint[Y];
					startPoint.z = curStartPoint[Z];

					hasStartPoint = true;
					break;
				}
			}
		}
	}
	else
	{
		acutPrintf(L"\n当前可能的路由数为【%d】",m_lPossibleRoutes.size());

		for( LineIter iter = m_lPossibleRoutes.begin(); 
			iter != m_lPossibleRoutes.end();
			iter++ )
		{
			if( (*iter).second != DONE )
			{
				m_CurrentPointVertices = (*iter).first;

				if( m_CurrentPointVertices )
				{
					int length = m_CurrentPointVertices->length();
					startPoint = m_CurrentPointVertices->at(length-1);

					hasStartPoint = true;
					break;
				}
			}
		}
	}

	if( hasStartPoint )
	{
		acutPrintf(L"\n找到一条未完成的可能路由。起始点为x【%0.2lf】y【%0.2lf】z【%0.2lf】",
						startPoint.x,startPoint.y,startPoint.z);
	}

	return hasStartPoint;
}

bool LineCalRouteDialog::SetCurrentPossibleLineDone()
{
	if( m_DrawRealTime )
	{
		for( LineEntityIter iter = m_lPossibleLineEntities.begin(); 
			iter != m_lPossibleLineEntities.end();
			iter++ )
		{
			if( (*iter).second != DONE )
			{
				if( m_CurrentRouteLineEntity == (*iter).first )
				{
					acutPrintf(L"\n设置当前可能路由的状态为已完成");
					(*iter).second = DONE;

					return true;
				}
			}
		}
	}
	else
	{
		for( LineIter iter = m_lPossibleRoutes.begin(); 
			iter != m_lPossibleRoutes.end();
			iter++ )
		{
			if( (*iter).second != DONE )
			{
				m_CurrentPointVertices = (*iter).first;

				if( (*iter).first == m_CurrentPointVertices )
				{
					acutPrintf(L"\n设置当前可能路由的状态为已完成");
					(*iter).second = DONE;

					return true;
				}
			}
		}
	}

	return false;
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
	while( GetPossibleStartPoint(m_newStartPoint) )
	{
		int count = 0;

		//递归计算
		while( CalculateShortestRoute( m_newStartPoint, m_endPoint ) == false )
		{
			acutPrintf(L"\n发现当前管线与系统中的线段有相侵的现象，继续计算");

			if( count++ >= 50 )
				break;
		}

		//current line is done
		SetCurrentPossibleLineDone();

		//One possible line route has been finished
		SetupLineRouteResult();
	}
}

bool LineCalRouteDialog::CalculateShortestRoute( const AcGePoint3d& start, const AcGePoint3d& end)
{
	acutPrintf(L"\n现在计算(X:%0.2lf,Y:%0.2lf,Z:%0.2lf)到(X:%0.2lf,Y:%0.2lf,Z:%0.2lf)的最短路由", start[X], start[Y], start[Z], end[X], end[Y], end[Z]);

	//以开始点和截止点创建默认宽度的管线
	InitializeCompareSegmentEntity(start, end);

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
		//AcGePoint3d newPoint = GetProjectPoint3d(nearestLine);
		//AcGePoint3d newPoint = GetIntersectPoint3d(nearestLine);
		AcGePoint3d newPoint = GetIntersectPoint3d(nearestLine, start, end);
		
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

LineCalRouteDialog::PASS_STATUS LineCalRouteDialog::GetPassDirecion( PointEntity *lineSegment)
{
	const wstring& throughDirection = lineSegment->m_DbEntityCollection.mCategoryData->mThroughDirection;

	int iPassDirection = PASS_NONE;

	//empty means this line is pipe
	if( throughDirection.empty() )
	{
		iPassDirection = PASS_ALL;
	}
	//Not empty means this line is block
	else	
	{
		if( throughDirection.substr(0,1) == L"1" )
		{
			iPassDirection |= PASS_LEFT;
		}

		if( throughDirection.substr(1,1) == L"1" )
		{
			iPassDirection |= PASS_RIGHT;
		}

		if( throughDirection.substr(2,1) == L"1" )
		{
			iPassDirection |= PASS_FRONT;
		}

		if( throughDirection.substr(3,1) == L"1" )
		{
			iPassDirection |= PASS_BACK;
		}

		if( throughDirection.substr(4,1) == L"1" )
		{
			iPassDirection |= PASS_UP;
		}

		if( throughDirection.substr(5,1) == L"1" )
		{
			iPassDirection |= PASS_DOWN;
		}
	}

	return (PASS_STATUS)iPassDirection;
}

AcGePoint3d LineCalRouteDialog::GetIntersectPoint3d(PointEntity* lineSegment)
{
	acutPrintf(L"\n得到管线的垂直于X面的点");

	AcGePoint3d start = lineSegment->m_DbEntityCollection.mStartPoint;
	AcGePoint3d end = lineSegment->m_DbEntityCollection.mEndPoint;
	AcGeLine3d intersectLine( start, end ); 

	AcGePoint3d resultPnt;
	m_ProjectPlane.intersectWith(intersectLine, resultPnt);
	acutPrintf(L"\n得到管线的相交点【X:%0.2lf,Y:%0.2lf,Z:%0.2lf】", resultPnt[X], resultPnt[Y], resultPnt[Z]);

	double heightOffset(0.0), stepOffset(0.0);
	GetHeightAndStep( lineSegment, heightOffset, stepOffset);
	acutPrintf(L"\n得到上下的位移【高度:%0.2lf, 步长:%0.2lf】", heightOffset, stepOffset);

	//得到穿越的方向
	PASS_STATUS passStatus = GetPassDirecion(lineSegment);

	//保存离X轴较近的点
	bool branched = false;
		
	AcGePoint3d branchFirstPoint, branchSecondPoint, nextStartPoint;

	AcGePoint3d firstPoint, secondPoint;
	firstPoint = AcGePoint3d(resultPnt.x,resultPnt.y - stepOffset,resultPnt.z);

	do
	{
		//Check the up through status
		if( passStatus & PASS_UP )
		{
			secondPoint = AcGePoint3d(firstPoint.x,firstPoint.y,firstPoint.z + heightOffset);

			//checked then remove the up status
			passStatus = (PASS_STATUS)(passStatus - PASS_UP);
		}
		else if( passStatus & PASS_DOWN )
		{
			secondPoint = AcGePoint3d(firstPoint.x,firstPoint.y,firstPoint.z - heightOffset);

			//checked then remove the down status
			passStatus = (PASS_STATUS)(passStatus - PASS_DOWN);
		} 
		else 
		{
			break;
		}

		if( !branched )
		{
			acutPrintf(L"\n这是在当前相侵点的第一个分支，下面会继续沿着这个路由计算下去");
			//Append the first inter segment

			branchFirstPoint = firstPoint;
			acutPrintf(L"\n存储的相切点【X:%0.2lf,Y:%0.2lf,Z:%0.2lf】为一个起点", firstPoint[X], firstPoint[Y], firstPoint[Z]);

			//Append the first inter segment
			branchSecondPoint = secondPoint;
			acutPrintf(L"\n返回竖起的点【X:%0.2lf,Y:%0.2lf,Z:%0.2lf】为折线点", secondPoint[X], secondPoint[Y], secondPoint[Z]);

			secondPoint.y = branchSecondPoint.y + 2 * stepOffset;
			acutPrintf(L"\n绕过柱体的点【X:%0.2lf,Y:%0.2lf,Z:%0.2lf】为新的计算起点", secondPoint[X], secondPoint[Y], secondPoint[Z]);

			//Append the first inter segment
			nextStartPoint = secondPoint;
			branched = true;
		}
		else
		{
			acutPrintf(L"\n已经存在在当前相侵点的一个分支了，先保存下来，以后再计算");

			if( m_DrawRealTime )
			{
				LineEntity* cloneLineEntity = CreateNewLineEntity();

				//start with current point
				pPointEntry currentPoint = (*m_CurrentRouteLineEntity->m_PointList)[(m_CurrentRouteLineEntity->m_PointList->size() - 1)];
				//cloneLineEntity->InsertPoint(currentPoint,true);

				//inser the inter point
				cloneLineEntity->InsertPoint(&firstPoint,true);

				//inser the second point
				cloneLineEntity->InsertPoint(&secondPoint,true);

				//next start point
				secondPoint.y = secondPoint.y + 2 * stepOffset;
				cloneLineEntity->InsertPoint(&secondPoint,true);

				m_lPossibleLineEntities.insert(std::pair<LineEntity*,CAL_STATUS>(cloneLineEntity,INIT));
			}
			else
			{
				AcGePoint3dArray* clonePoint3dArray = new AcGePoint3dArray(*m_CurrentPointVertices);

				clonePoint3dArray->append(firstPoint);

				clonePoint3dArray->append(secondPoint);

				//next start point
				secondPoint.y = secondPoint.y + 2 * stepOffset;
				clonePoint3dArray->append(secondPoint);

				m_lPossibleRoutes.insert(std::pair<AcGePoint3dArray*,CAL_STATUS>(clonePoint3dArray,INIT));
			}
		}
	}
	while(true);

	if( branched )
	{
		AppendInterSegment( branchFirstPoint );

		//Append the first inter segment
		AppendInterSegment( branchSecondPoint );

		//Append the new startsegment
		AppendInterSegment( nextStartPoint );
	}

	return nextStartPoint;
}

AcGePoint3d LineCalRouteDialog::GetIntersectPoint3d(PointEntity* lineSegment, const AcGePoint3d& throughStart, const AcGePoint3d& throughEnd)
{
	acutPrintf(L"\n得到管线的垂直于X面的点");

	AcGePoint3d start = lineSegment->m_DbEntityCollection.mStartPoint;
	AcGePoint3d end = lineSegment->m_DbEntityCollection.mEndPoint;
	AcGeLine3d intersectLine( start, end ); 

	AcGePoint3d resultPnt;
	m_ProjectPlane.intersectWith(intersectLine, resultPnt);
	acutPrintf(L"\n得到管线的相交点【X:%0.2lf,Y:%0.2lf,Z:%0.2lf】", resultPnt[X], resultPnt[Y], resultPnt[Z]);

	double heightOffset(0.0), stepOffset(0.0);
	GetHeightAndStep( lineSegment, heightOffset, stepOffset);
	acutPrintf(L"\n得到上下的位移【高度:%0.2lf, 步长:%0.2lf】", heightOffset, stepOffset);

	//得到穿越的方向
	PASS_STATUS passStatus = GetPassDirecion(lineSegment);

	//保存离X轴较近的点
	bool branched = false;
		
	AcGePoint3d branchFirstPoint, branchSecondPoint, nextStartPoint;

	AcGePoint3d firstPoint, secondPoint;
	firstPoint = AcGePoint3d(resultPnt.x,resultPnt.y - stepOffset,resultPnt.z);

	do
	{
		//Check the up through status
		if( passStatus & PASS_UP )
		{
			secondPoint = AcGePoint3d(firstPoint.x,firstPoint.y,firstPoint.z + heightOffset);

			//checked then remove the up status
			passStatus = (PASS_STATUS)(passStatus - PASS_UP);
		}
		else if( passStatus & PASS_DOWN )
		{
			secondPoint = AcGePoint3d(firstPoint.x,firstPoint.y,firstPoint.z - heightOffset);

			//checked then remove the down status
			passStatus = (PASS_STATUS)(passStatus - PASS_DOWN);
		} 
		else 
		{
			break;
		}

		if( !branched )
		{
			acutPrintf(L"\n这是在当前相侵点的第一个分支，下面会继续沿着这个路由计算下去");
			//Append the first inter segment

			branchFirstPoint = firstPoint;
			acutPrintf(L"\n存储的相切点【X:%0.2lf,Y:%0.2lf,Z:%0.2lf】为一个起点", firstPoint[X], firstPoint[Y], firstPoint[Z]);

			//Append the first inter segment
			branchSecondPoint = secondPoint;
			acutPrintf(L"\n返回竖起的点【X:%0.2lf,Y:%0.2lf,Z:%0.2lf】为折线点", secondPoint[X], secondPoint[Y], secondPoint[Z]);

			secondPoint.y = branchSecondPoint.y + 2 * stepOffset;
			acutPrintf(L"\n绕过柱体的点【X:%0.2lf,Y:%0.2lf,Z:%0.2lf】为新的计算起点", secondPoint[X], secondPoint[Y], secondPoint[Z]);

			//Append the first inter segment
			nextStartPoint = secondPoint;
			branched = true;
		}
		else
		{
			acutPrintf(L"\n已经存在在当前相侵点的一个分支了，先保存下来，以后再计算");

			if( m_DrawRealTime )
			{
				LineEntity* cloneLineEntity = CreateNewLineEntity();

				//start with current point
				pPointEntry currentPoint = (*m_CurrentRouteLineEntity->m_PointList)[(m_CurrentRouteLineEntity->m_PointList->size() - 1)];
				//cloneLineEntity->InsertPoint(currentPoint,true);

				//inser the inter point
				cloneLineEntity->InsertPoint(&firstPoint,true);

				//inser the second point
				cloneLineEntity->InsertPoint(&secondPoint,true);

				//next start point
				secondPoint.y = secondPoint.y + 2 * stepOffset;
				cloneLineEntity->InsertPoint(&secondPoint,true);

				m_lPossibleLineEntities.insert(std::pair<LineEntity*,CAL_STATUS>(cloneLineEntity,INIT));
			}
			else
			{
				AcGePoint3dArray* clonePoint3dArray = new AcGePoint3dArray(*m_CurrentPointVertices);

				clonePoint3dArray->append(firstPoint);

				clonePoint3dArray->append(secondPoint);

				//next start point
				secondPoint.y = secondPoint.y + 2 * stepOffset;
				clonePoint3dArray->append(secondPoint);

				m_lPossibleRoutes.insert(std::pair<AcGePoint3dArray*,CAL_STATUS>(clonePoint3dArray,INIT));
			}
		}
	}
	while(true);

	if( branched )
	{
		AppendInterSegment( branchFirstPoint );

		//Append the first inter segment
		AppendInterSegment( branchSecondPoint );

		//Append the new startsegment
		AppendInterSegment( nextStartPoint );
	}

	return nextStartPoint;
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

	double heightOffset(0.0), stepOffset(0.0);
	GetHeightAndStep( lineSegment, heightOffset, stepOffset);
	acutPrintf(L"\n得到上下的位移【高度:%0.2lf, 步长:%0.2lf】", heightOffset, stepOffset);

	//得到穿越的方向
	PASS_STATUS passStatus = GetPassDirecion(lineSegment);

	//保存离X轴较近的点
	bool branched = false;
	AcGePoint3d curInterPoint, nextPoint;

	do
	{
		AcGePoint3d projectPoint;
		//Check the up through status
		if( passStatus & PASS_UP )
		{
			projectPoint = AcGePoint3d(resultPnt.x,resultPnt.y - stepOffset,resultPnt.z + heightOffset);

			//checked then remove the up status
			passStatus = (PASS_STATUS)(passStatus - PASS_UP);
		}
		else if( passStatus & PASS_DOWN )
		{
			projectPoint = AcGePoint3d(resultPnt.x,resultPnt.y - stepOffset,resultPnt.z - heightOffset);

			//checked then remove the down status
			passStatus = (PASS_STATUS)(passStatus - PASS_DOWN);
		} 
		else 
		{
			break;
		}

		if( !branched )
		{
			acutPrintf(L"\n这是在当前相侵点的第一个分支，下面会继续沿着这个路由计算下去");
					
			curInterPoint = projectPoint;
			acutPrintf(L"\n存储下方的点【X:%0.2lf,Y:%0.2lf,Z:%0.2lf】为一个起点", projectPoint[X], projectPoint[Y], projectPoint[Z]);

			projectPoint.y = resultPnt.y + stepOffset;
			acutPrintf(L"\n返回上方的点【X:%0.2lf,Y:%0.2lf,Z:%0.2lf】为新的计算起点", projectPoint[X], projectPoint[Y], projectPoint[Z]);

			nextPoint = projectPoint;
			branched = true;
		}
		else
		{
			acutPrintf(L"\n已经存在在当前相侵点的一个分支了，先保存下来，以后再计算");

			if( m_DrawRealTime )
			{
				LineEntity* cloneLineEntity = CreateNewLineEntity();

				//start with current point
				pPointEntry currentPoint = (*m_CurrentRouteLineEntity->m_PointList)[(m_CurrentRouteLineEntity->m_PointList->size() - 1)];
				cloneLineEntity->InsertPoint(currentPoint,true);

				//inser the inter point
				cloneLineEntity->InsertPoint(&projectPoint,true);

				//next start point
				projectPoint.y = resultPnt.y + stepOffset;
				cloneLineEntity->InsertPoint(&projectPoint,true);

				m_lPossibleLineEntities.insert(std::pair<LineEntity*,CAL_STATUS>(cloneLineEntity,INIT));
			}
			else
			{
				AcGePoint3dArray* clonePoint3dArray = new AcGePoint3dArray(*m_CurrentPointVertices);

				clonePoint3dArray->append(projectPoint);

				//next start point
				projectPoint.y = resultPnt.y + stepOffset;
				clonePoint3dArray->append(projectPoint);

				m_lPossibleRoutes.insert(std::pair<AcGePoint3dArray*,CAL_STATUS>(clonePoint3dArray,INIT));
			}
		}
	}
	while(true);

	if( branched )
	{
		//Append the first inter segment
		AppendInterSegment( curInterPoint );

		//Append the first inter segment
		AppendInterSegment( nextPoint );
	}

	return nextPoint;
}

//Create a new line segment, for this point is a new route point
void LineCalRouteDialog::AppendInterSegment(const AcGePoint3d& newPoint)
{
	//Draw realtime means draw the possible line during the process of calculation
	if( m_DrawRealTime )
	{
		PointEntity* point = new PointEntity();

		point->m_Point[X] = newPoint.x;
		point->m_Point[Y] = newPoint.y;
		point->m_Point[Z] = newPoint.z;

		m_CurrentRouteLineEntity->InsertPoint(point, true);
	}
	else
	{
		m_CurrentPointVertices->append( newPoint );
	}
}

void LineCalRouteDialog::GetHeightAndStep(PointEntity* lineSegment, double& height, double& step)
{
	double wallSize = 0.0;
	acdbDisToF(lineSegment->m_DbEntityCollection.mCategoryData->mWallSize.c_str(), -1, &wallSize);
	wallSize /= 1000;

	double safeSize = 0.0;
	acdbDisToF(lineSegment->m_DbEntityCollection.mCategoryData->mSafeSize.c_str(), -1, &safeSize);
	safeSize /= 1000;
	
	double temp = 0.0;

	if( lineSegment->m_DbEntityCollection.mCategoryData->mShape == GlobalData::LINE_SHAPE_CIRCLE )
	{
		//圆体的话，取半径为Y,Z轴的偏离
		acdbDisToF(lineSegment->m_DbEntityCollection.mCategoryData->mSize.mRadius.c_str(), -1, &temp);
		temp /= 1000;
		temp += wallSize;
		temp += safeSize;

		height = step = temp;
	}
	else
	{
		//柱体的话，取宽度为Y轴的偏离
		acdbDisToF(lineSegment->m_DbEntityCollection.mCategoryData->mSize.mWidth.c_str(), -1, &temp);
		temp /= 2000;
		temp += wallSize;
		temp += safeSize;

		step = temp;

		//取高度的一半为Z轴的偏离
		acdbDisToF(lineSegment->m_DbEntityCollection.mCategoryData->mSize.mHeight.c_str(), -1, &temp);
		temp /= 2000;
		temp += wallSize;
		temp += safeSize;

		height = temp;
	}
}

//判断一条折线段与其他管线的相侵情况
void LineCalRouteDialog::CheckIntersect(AcArray<PointEntity*>* intersectEntities)
{
	acutPrintf(L"\n进行相侵判断.");

	PointList* pointList = m_CompareLineSegmentEntity->m_PointList;
	if( pointList == NULL 
		|| pointList->size() < 2 )
	{
		acutPrintf(L"\n临时比较管线中没有折线段，可以不进行检查");
		return;
	}

	PointEntity* checkPoint = (*pointList)[1];

	wstring& lineName = checkPoint->m_DbEntityCollection.mLayerName;
	Adesk::Int32& checkLineID = checkPoint->m_DbEntityCollection.mLineID;
	Adesk::Int32& checkSeqNO = checkPoint->m_DbEntityCollection.mSequenceNO;
	wstring& layerName = wstring(m_CutLayerName.GetBuffer());

#ifdef DEBUG
	acutPrintf(L"\n对【%s】的第【%d】条进行相侵判断.",lineName.c_str(), checkSeqNO);
#endif

	LineList* lineList = m_EntryFile->GetList();
	for( LineIterator line = lineList->begin();
			line != lineList->end();
			line++ )
	{
		if( (*line)->m_LineName == layerName )
		{
			acutPrintf(L"\n此管线为自动路由临时线段，不参与比较");
			continue;
		}

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

void LineCalRouteDialog::SetupLineRouteResult()
{
	//设置最后一段管线
	AppendInterSegment( m_endPoint );

	m_CurrentPointVertices = NULL;
	m_CurrentRouteLineEntity = NULL;

	//清除比较的那条最终线段
	m_CompareLineSegmentEntity->SetPoints(NULL);
}

void LineCalRouteDialog::SetupFinalResult()
{
	if( m_DrawRealTime )
	{
		acutPrintf(L"\n整理最终路由结果，有可能路由线路【%d】条",m_lPossibleLineEntities.size());
	}
	else
	{
		//得到最短线路
		GetShortestRoute();

		//绘制所有线路，同时最短的线路标记为红色
		DrawFinalResult();
	}
}

void LineCalRouteDialog::DrawFinalResult()
{
	acutPrintf(L"\n整理最终路由结果，路由的线路有【%d】条",m_lPossibleRoutes.size());

	for( LineIter iter = m_lPossibleRoutes.begin(); 
		iter != m_lPossibleRoutes.end();
		iter++ )
	{
		if( (*iter).second == DONE )
		{
			m_CurrentPointVertices = (*iter).first;
			acutPrintf(L"\n绘制路由，其中有线段【%d】条",m_CurrentPointVertices->length() - 1);

			PointList* newPoints = new PointList();

			for( int i = 0; i < m_CurrentPointVertices->length(); i++ )
			{
				PointEntity* point = new PointEntity();
				point->m_PointNO = i;
		
				point->m_Point[X] = m_CurrentPointVertices->at(i).x;
				point->m_Point[Y] = m_CurrentPointVertices->at(i).y;
				point->m_Point[Z] = m_CurrentPointVertices->at(i).z;

				newPoints->push_back( point );
			}

			m_CurrentRouteLineEntity = CreateNewLineEntity();
			if( m_CurrentRouteLineEntity )
			{
				if( m_CurrentPointVertices == this->m_ShortestPointVertices )
				{
					acutPrintf(L"\n当前管线为最短线路，标记为红色");
					m_CurrentRouteLineEntity->m_LinePriority = GlobalData::LINE_SECOND;
				}

				m_CurrentRouteLineEntity->SetPoints( newPoints );
			}

			//Save to possible line list, use to delete all the entities
			m_AllPossibleLineEntities.push_back( m_CurrentRouteLineEntity );

			m_CurrentPointVertices = NULL;
		}
	}
}

double LineCalRouteDialog::GetShortestRoute()
{
	double shortestLength = 0x3FFFFFFF;

	acutPrintf(L"\n在【%d】条可能线路中寻找最短的路由",m_lPossibleRoutes.size());

	for( LineIter iter = m_lPossibleRoutes.begin(); 
		iter != m_lPossibleRoutes.end();
		iter++ )
	{
		double oneLineLength = 0.0;

		if( (*iter).second == DONE )
		{
			m_CurrentPointVertices = (*iter).first;
				
			acutPrintf(L"\n计算一条新的线路长度，有线段【%d】条",m_CurrentPointVertices->length() - 1);

			AcGePoint3d* pLastPoint = NULL;
			for( int i = 0; i < m_CurrentPointVertices->length(); i++ )
			{
				if( pLastPoint == NULL )
				{
					pLastPoint = &m_CurrentPointVertices->at(i);
					continue;
				}

				double segmentLength = pLastPoint->distanceTo(m_CurrentPointVertices->at(i));
				acutPrintf(L"\n第【%d】条线段长度为【%0.2lf】",i, segmentLength);

				oneLineLength += segmentLength;
			}

			acutPrintf(L"\n当前线路的长度为【%0.2lf】，临时最短的线路长度为【%0.2lf】",oneLineLength, shortestLength);

			if( oneLineLength < shortestLength )
			{
				acutPrintf(L"\n当前线路更短一些");
				m_ShortestPointVertices = m_CurrentPointVertices;
				shortestLength = oneLineLength;
			}
		}
	}

	acutPrintf(L"\n最短距离为【%0.2lf】",shortestLength);
	return shortestLength;
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

		acutPrintf(L"\n删除计算出的路由所在的图层上的其他剩余实体");
		if( ArxWrapper::RemoveFromModelSpace(m_CutLayerName.GetBuffer()) )
		{
			acutPrintf(L"\n删除剩余实体成功");
		}

		acutPrintf(L"\n删除计算出的路由所在的图层");
		if( ArxWrapper::DeleteLayer(m_CutLayerName.GetBuffer(),true) )
		{
			acutPrintf(L"\n删除图层成功");
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
	typedef list<LineEntity*>::iterator DbLineIter;

	for( DbLineIter iter = m_AllPossibleLineEntities.begin(); 
		iter != m_AllPossibleLineEntities.end();
		iter++ )
	{
		LineEntity* lineEntity = *iter;

		//删除前一条计算路由时的管线实体
		if( lineEntity )
		{
			//从数据库删除管线本身
			ArxWrapper::DeleteFromNameObjectsDict(lineEntity->m_dbId,LineEntity::LINE_ENTRY_LAYER);

			//从数据库删除管线所有的线段
			lineEntity->EraseDbObjects();

			//删除所有的内存节点
			lineEntity->SetPoints(NULL);

			//删除线段集合
			m_EntryFile->DeleteLine(lineEntity->GetLineID());

			delete lineEntity;
			lineEntity = NULL;
		}
	}

	//删除路由管线集合
	m_AllPossibleLineEntities.clear();

	for( LineIter iter = m_lPossibleRoutes.begin();
		iter != m_lPossibleRoutes.end();
		iter++ )
	{
		if( (*iter).first )
			delete (*iter).first;
	}

	//删除折线点集合
	m_lPossibleRoutes.clear();
}

// LineCalRouteDialog message handlers
