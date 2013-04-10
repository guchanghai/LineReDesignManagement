// LineCutPosDialog.cpp : implementation file
//

#include "stdafx.h"
#include "LineCutPosDialog.h"
//#include "afxdialogex.h"

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

// LineCutPosDialog dialog

CString LineCutPosDialog::m_CutLayerName = L"";
AcDbObjectIdArray LineCutPosDialog::m_CutObjects = AcDbObjectIdArray();

IMPLEMENT_DYNAMIC(LineCutPosDialog, CAcUiDialog)

LineCutPosDialog::LineCutPosDialog(CWnd* pParent /*=NULL*/)
	: CAcUiDialog(LineCutPosDialog::IDD, pParent),
	m_Direction(0),
	m_strOffset(0)
{
}

LineCutPosDialog::~LineCutPosDialog()
{
}

BOOL LineCutPosDialog::OnInitDialog()
{
	//和页面交互数据
	CAcUiDialog::OnInitDialog();

	//默认X轴选中
	m_DirectionX.SetCheck(BST_CHECKED);
	m_Direction = 1;

	//默认偏移为0
	m_EditOffset.SetWindowTextW(L"0.00");

	//加载图片
	m_PickCutPosButton.AutoLoad();

	return TRUE;
}

void LineCutPosDialog::DoDataExchange(CDataExchange* pDX)
{
	CAcUiDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_OFFSET, m_EditOffset);

	DDX_Control(pDX, IDC_X, m_DirectionX);
	DDX_Control(pDX, IDC_Y, m_DirectionY);
	DDX_Control(pDX, IDC_Z, m_DirectionZ);

	DDX_Control(pDX, IDC_BUTTON_PICKCUT,m_PickCutPosButton);
}

void LineCutPosDialog::OnBnClickedOk()
{
	//得到用户输入的数据
	UpdateData(FALSE);

	//生成切图
	GenereateCutRegion();

	//显示该图层
	ShowCutRegion();

	//关闭对话框
	CAcUiDialog::OnOK();
}

void LineCutPosDialog::OnBnClickedX()
{
	m_Direction = 1;
}

void LineCutPosDialog::OnBnClickedY()
{
	m_Direction = 2;
}

void LineCutPosDialog::OnBnClickedZ()
{
	m_Direction = 3;
}

BEGIN_MESSAGE_MAP(LineCutPosDialog, CAcUiDialog)
	ON_BN_CLICKED(IDOK, &LineCutPosDialog::OnBnClickedOk)
	ON_BN_CLICKED(IDC_X, &LineCutPosDialog::OnBnClickedX)
	ON_BN_CLICKED(IDC_Y, &LineCutPosDialog::OnBnClickedY)
	ON_BN_CLICKED(IDC_Z, &LineCutPosDialog::OnBnClickedZ)
	ON_BN_CLICKED(IDC_BUTTON_PICKCUT, &LineCutPosDialog::OnBnPickCutPos)
END_MESSAGE_MAP()

void Transform(AcDbEntity* pEntry, int xoffset = 0, int yoffset = 0, int zoffset = 0)
{
	AcGeVector3d vec(xoffset,yoffset,zoffset);

	AcGeMatrix3d moveMatrix;
	moveMatrix.setToTranslation(vec);

	pEntry->transformBy(moveMatrix);
}

void LineCutPosDialog::GenerateCutPlane()
{
	//创建切面
	CString offset;
	m_EditOffset.GetWindowTextW(offset);

	if( offset.GetLength())
		m_strOffset = _wtoi(offset);

	if( m_Direction == 1)
	{
		m_CutLayerName.Format(L"与X轴垂直偏移量为【%d】的切面",m_strOffset);
		m_CutPlane.set(AcGePoint3d(m_strOffset,0,0),AcGeVector3d(1,0,0));
	}
	else if( m_Direction == 2)
	{
		m_CutLayerName.Format(L"与Y轴垂直偏移量为【%d】的切面",m_strOffset);
		m_CutPlane.set(AcGePoint3d(0,m_strOffset,0),AcGeVector3d(0,1,0));
	}
	else if( m_Direction == 3)
	{
		m_CutLayerName.Format(L"与Z轴垂直偏移量为【%d】的切面",m_strOffset);
		m_CutPlane.set(AcGePoint3d(0,0,m_strOffset),AcGeVector3d(0,0,1));
	}

#ifdef DEBUG
	acutPrintf(L"\n切面为【%s】",m_CutLayerName.GetBuffer());
#endif
}

void LineCutPosDialog::GenereateCutRegion()
{
	//首先恢复视图
	CutBack();

	ArxWrapper::LockCurDoc();

	//得到切面
	GenerateCutPlane();

	//得到当前的实体文件管理器
	LineEntityFile* pLineFile = LineEntityFileManager::GetCurrentLineEntryFile();
	if( pLineFile == NULL )
	{
		acutPrintf(L"\n没能找到管线文件管理器，检查下吧！");
		return;
	}

	//得到实体列表
	LineList* lineList = pLineFile->GetList();
	if( lineList == NULL )
	{
		acutPrintf(L"\n当前文件中没有管线，无需进行切图！");
		return;
	}

	//遍历实体文件管理，针对每一条实体进行切图
	LineIterator lineIter = lineList->begin();

	for(;lineIter != lineList->end();
		lineIter++)
	{
#ifdef DEBUG
		acutPrintf(L"\n对管线【%s】进行切图！",(*lineIter)->m_LineName.c_str());
#endif
		if( *lineIter != NULL )
			GenereateCutRegion(*lineIter);
	}

	ArxWrapper::UnLockCurDoc();
}

void LineCutPosDialog::GenereateCutRegion(LineEntity* lineEntry)
{
	PointList* pointList = lineEntry->m_PointList;
	if( pointList == NULL )
	{
		acutPrintf(L"\n该管线没有线段，无需切图！");
		return;
	}

	//对所有的线段进行遍历
	PointIter pointIter = pointList->begin();
	for(;pointIter != pointList->end();pointIter++)
	{
		if( pointIter == pointList->begin() )
			continue;

		PointEntity* pointEntry = (*pointIter);

		if( pointEntry == NULL )
		{
			acutPrintf(L"\n该线段不合法，需要注意！");
			continue;
		}

#ifdef DEBUG
		acutPrintf(L"\n对第【%d】个线段进行切图！",pointEntry->m_PointNO);
#endif
		AcDbObjectId lineEntityId = pointEntry->m_DbEntityCollection.GetLineEntity();
		if( lineEntityId.isNull() )
		{
			acutPrintf(L"\n当前线段没有对应的数据库实体，不能切图！");
			continue;
		}

		AcDbEntity* pLineObj;
		Acad::ErrorStatus es = acdbOpenAcDbEntity(pLineObj, lineEntityId, AcDb::kForRead);

		if( es == Acad::eOk )
		{
			LMALineDbObject* pLMALine = LMALineDbObject::cast(pLineObj);

			if( pLMALine == NULL )
			{
				acutPrintf(L"\n当前线段不是有效的辅助系统管理的实体，不考虑切图！");
				continue;
			}

			//得到实体与切面相切的截面
			AcDbRegion *pSelectionRegion = NULL;
			pLMALine->getSection(m_CutPlane, pSelectionRegion);

			//得到注释的中心点
			AcGePoint3d centerPoint = pLMALine->GetCutCenter(m_CutPlane);

			//设置注释的内容
			CString markContent;
			markContent.Format(L"%s#%d",lineEntry->m_LineName.c_str(), pointEntry->m_PointNO);

			//关闭实体
			pLMALine->close();

			if( pSelectionRegion )
			{
				acutPrintf(L"\n生成切面区域");

				//创建切面所在的图层
				if( ArxWrapper::createNewLayer(m_CutLayerName.GetBuffer()) == false )
					return;

				//将截面加入到模型空间
				AcDbObjectId regionId = ArxWrapper::PostToModelSpace(pSelectionRegion,m_CutLayerName.GetBuffer());
				m_CutObjects.append(regionId);

				//创建该界面的填充区域
				AcDbObjectId hatchId = ArxWrapper::CreateHatch(regionId,L"NET", true, m_CutLayerName.GetBuffer(), m_CutPlane, m_strOffset);
				m_CutObjects.append(hatchId);

				//创建截图区域的注释
				AcDbObjectId mLeaderId = ArxWrapper::CreateMLeader(centerPoint, m_strOffset, m_Direction, markContent.GetBuffer(), m_CutLayerName.GetBuffer());
				m_CutObjects.append(mLeaderId);
			}
			else
			{
				acutPrintf(L"\n切面与该管线（阻隔体）无相交区域！");
			}
		}
		else
		{
			acutPrintf(L"\n打开管线实体失败！");
			rxErrorMsg(es);
		}
	}
}

void LineCutPosDialog::ShowCutRegion()
{
	//只显示切面图层
	ArxWrapper::ShowLayer(m_CutLayerName.GetBuffer());

	//切换视图
	ArxWrapper::ChangeView(m_Direction);
}

void LineCutPosDialog::CutBack()
{
	acutPrintf(L"\n存在的切图为【%s】！",m_CutLayerName.GetBuffer());

	if( m_CutLayerName.GetLength() > 0 )
	{
		acutPrintf(L"\n首先锁定该文档");
		ArxWrapper::LockCurDoc();

		//acutPrintf(L"\n恢复WCS视窗");
		//acedCommand(RTSTR, _T("UCS"), RTSTR, L"W", 0);

		acutPrintf(L"\n删除切图相关的对象");
		RemoveObectsLastCut();

		acutPrintf(L"\n删除切图所在的图层");
		if( ArxWrapper::DeleteLayer(m_CutLayerName.GetBuffer(),true) )
		{
			acutPrintf(L"\n初始化设置");
			m_CutLayerName.Format(L"");
		}

		acutPrintf(L"\n显示其他图层");
		ArxWrapper::ShowLayer(L"");

		acutPrintf(L"\n解除文档锁定");
		ArxWrapper::UnLockCurDoc();
	}
	else
	{
		acutPrintf(L"\n当前系统内没有切图！");
	}
}

void LineCutPosDialog::OnBnPickCutPos()
{
	// Hide the dialog and give control to the editor
    BeginEditorCommand();

    ads_point pt;

	CString temp;

    // Get a point
    if (acedGetPoint(NULL, _T("\n选取切割点: "), pt) == RTNORM) {
        // If the point is good, continue
        CompleteEditorCommand();

		if( m_Direction == 1 )
			temp.Format(_T("%g"), pt[X]);
		else if ( m_Direction == 2 )
			temp.Format(_T("%g"), pt[Y]);
		else if ( m_Direction == 3 )
			temp.Format(_T("%g"), pt[Z]);
    } else {
        // otherwise cancel the command (including the dialog)
        CancelEditorCommand();
    }

	m_EditOffset.SetWindowTextW(temp.GetBuffer());

	UpdateData(FALSE);
}

void LineCutPosDialog::RemoveObectsLastCut()
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

// LineCutPosDialog message handlers
