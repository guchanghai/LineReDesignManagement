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

void LineCalRouteDialog::OnBnClickedOk()
{
	//得到用户输入的数据
	UpdateData(FALSE);

	//首先恢复视图
	CutBack();

	//创建前置条件（路由所在的图层《名称》，路由所代表的管线）
	SetupRouteLineEnv();

	//计算两点之间的(接近)最短路由;
	CalculateShortestRoute();

	//关闭对话框
	CAcUiDialog::OnOK();
}

bool LineCalRouteDialog::SetupRouteLineEnv()
{
	//创建图层的名称
	m_CutLayerName.Format(L"从【X:%0.2lf,Y:%0.2lf,Z:%0.2lf】到【X:%0.2lf,Y:%0,2lf,Z:%0.2lf】的最短路由",m_startPoint[X],m_startPoint[Y],m_startPoint[Z],m_endPoint[X],m_endPoint[Y],m_endPoint[Z]);

	//创建代表路由的管线实体
	InitializeRouteLine();

	return true;
}

bool LineCalRouteDialog::InitializeRouteLine()
{
	//创建管线detail信息
	InitializeRouteLineInfo();

	//得到实体名称
	wstring pipeName = m_EntryFile->GetNewPipeName(m_lineInfo, L"");

	//创建新的路由管线
	LineEntity* newLine = new LineEntity(pipeName,GlobalData::CONFIG_LINE_KIND, m_lineInfo ,NULL);

	m_RouteLineEntity = new LineEntity();

	return true;
}

//初始化代表自动路由的基本信息
bool LineCalRouteDialog::InitializeRouteLineInfo()
{
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

		categoryData->mWallSize = wstring(lineWallSize.GetBuffer());
		categoryData->mSafeSize = wstring(lineSafeSize.GetBuffer());

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
	//以用户选择的开始、结束点初始化这条管线
	AppendStartEndPoints( startPoint, endPoint);

	//默认用户选择的开始点可以保存下来了
	SaveRouteLinePoint( startPoint );

	return true;
}

bool LineCalRouteDialog::SaveRouteLinePoint( const AcGePoint3d& newPoint )
{
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
	point->m_Point[X] = endPoint[X];
	point->m_Point[X] = endPoint[X];
	point->m_Point[X] = endPoint[X];

	newPoints->push_back( point );

	//以此开始和结束点创建新的路由线段
	m_RouteLineEntity->SetPoints(newPoints);

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
	//从用户选择的其实点开始
	m_newStartPoint = m_startPoint;

	//递归计算
	while( CalculateShortestRoute( m_newStartPoint, m_endPoint ) == false )
	{
		acutPrintf(L"\n发现最后一条管线的线段有相侵的现象，继续计算");
	}
}

bool LineCalRouteDialog::CalculateShortestRoute( const AcGePoint3d& start, const AcGePoint3d& end)
{
	//以开始点和截止点创建默认宽度的管线
	InitializeStartEndPoints(start, end);

	//相侵的管线
	AcArray<PointEntity*>* intersectEntities = new AcArray<PointEntity*>();

	//与当前系统内的管线判断
	if( HasIntersect(intersectEntities) )
	{
		//有管线相侵，得到最近的一条
		PointEntity* nearestLine = GetNearestLineSegement(intersectEntities);

		//得到新起点
		AcGePoint3d newPoint = GetProjectPoint3d(nearestLine);

		//得到老起点与新起点之间的线段，加入到图层中；返回false继续
		m_newStartPoint = newPoint;

		//删除中间结果
		delete intersectEntities;

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
	if( intersectEntities->length() == 1 )
		return intersectEntities->at(0);

	return NULL;
}

AcGePoint3d LineCalRouteDialog::GetProjectPoint3d(PointEntity* lineSegment)
{
	AcGePoint3d projectPoint;

	return projectPoint;
}

//判断一条折线段与其他管线的相侵情况
bool LineCalRouteDialog::HasIntersect(AcArray<PointEntity*>* intersectEntities)
{
	PointList* pointList = m_RouteLineEntity->m_PointList;
	if( pointList == NULL 
		|| pointList->size() < 2 )
	{
		acutPrintf(L"\n当前管线没有折线段，可以不进行检查");
		return true;
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

			pSafeLine->close();
			pCheckSafeLine->close();

			ArxWrapper::UnLockCurDoc();
		}
	}

	return false;
}

bool LineCalRouteDialog::CreateRouteSegment( const AcGePoint3d& start, const AcGePoint3d& end)
{
	return true;
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
	if( m_CutObjects )
	{
		while( m_CutObjects->length() )
		{
			AcDbObjectId objId = m_CutObjects->at(0);

			if( objId.isValid() )
			{
				ArxWrapper::RemoveDbObject(objId);
			}

			m_CutObjects->removeAt(0);
		}
	}

	//删除前一条计算路由时的管线实体
	if( m_RouteLineEntity )
		delete m_RouteLineEntity;
}

// LineCalRouteDialog message handlers
