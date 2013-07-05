// LineCalRouteDialog.cpp : implementation file
//

#include "stdafx.h"
#include "LineCalRouteDialog.h"

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

CString LineCalRouteDialog::m_CutLayerName = L"";
AcDbObjectIdArray LineCalRouteDialog::m_CutObjects = AcDbObjectIdArray();

// LineCalRouteDialog dialog

IMPLEMENT_DYNAMIC(LineCalRouteDialog, CAcUiDialog)

LineCalRouteDialog::LineCalRouteDialog(CWnd* pParent /*=NULL*/)
: CAcUiDialog(LineCalRouteDialog::IDD, pParent),
	m_startPoint(),
	m_endPoint()
{
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

	//计算两点之间的(接近)最短路由;

	//关闭对话框
	CAcUiDialog::OnOK();
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
	while( m_CutObjects.length() )
	{
		AcDbObjectId objId = m_CutObjects.at(0);

		if( objId.isValid() )
		{
			ArxWrapper::RemoveDbObject(objId);
		}

		m_CutObjects.removeAt(0);
	}
}

// LineCalRouteDialog message handlers
