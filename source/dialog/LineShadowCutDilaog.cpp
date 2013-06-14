// LineCutPosDialog.cpp : implementation file
//

#include "stdafx.h"
#include "LineShadowCutDialog.h"

#include "acedads.h"
#include "accmd.h"
#include <adscodes.h>
#include "geassign.h"
#include <adsdlg.h>
#include "dbxutil.h"
#include <dbapserv.h>
#include <dbregion.h>
#include <gepnt3d.h>
#include <actrans.h>

//symbol table
#include <dbsymtb.h>

#include <acdocman.h>

//3D Object
#include <dbsol3d.h>
#include <dbmleader.h>

#include <ArxWrapper.h>

#include <ArxCustomObject.h>

#include <GlobalDataConfig.h>

#include <LMAUtils.h>

#pragma warning(disable:4482)

using namespace com::guch::assistant::arx;

// LineShadowCutDialog dialog

IMPLEMENT_DYNAMIC(LineShadowCutDialog, CAcUiDialog)

LineShadowCutDialog::LineShadowCutDialog( int dialogId, CWnd* pParent /*=NULL*/)
	: LineCutPosDialog(dialogId, pParent),
	  m_collector(),
	  m_ShadowViewPort()
{
	memset(m_LeftDownCorner, 0 , 3 * sizeof(double));
	memset(m_RightUpCorner, 0, 3 * sizeof(double));
	memset(m_ViewPosition, 0 , 3 * sizeof(double));
	memset(m_TargetPosition, 0, 3 * sizeof(double));
}

LineShadowCutDialog::~LineShadowCutDialog()
{
}

BOOL LineShadowCutDialog::OnInitDialog()
{
	//和页面交互数据
	LineCutPosDialog::OnInitDialog();

	//默认沿轴的方向选中
	m_DirectionSame.SetCheck(BST_CHECKED);
	m_ViewDirection = 0;

	return TRUE;
}

void LineShadowCutDialog::DoDataExchange(CDataExchange* pDX)
{
	LineCutPosDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_RADIO_DIR_SAME, m_DirectionSame);
	DDX_Control(pDX, IDC_RADIO_DIR_OPPOSITE, m_DirectionOpposite);
}

BEGIN_MESSAGE_MAP(LineShadowCutDialog, CAcUiDialog)
	ON_BN_CLICKED(IDC_X, &LineShadowCutDialog::OnBnClickedX)
	ON_BN_CLICKED(IDC_Y, &LineShadowCutDialog::OnBnClickedY)
	ON_BN_CLICKED(IDC_Z, &LineShadowCutDialog::OnBnClickedZ)
	ON_BN_CLICKED(IDC_RADIO_DIR_SAME, &LineShadowCutDialog::onBnClickedSame)
	ON_BN_CLICKED(IDC_RADIO_DIR_OPPOSITE, &LineShadowCutDialog::onBnClickedOpposite)
	ON_BN_CLICKED(IDC_BUTTON_PICKCUT, &LineShadowCutDialog::OnBnPickCutPos)
	ON_BN_CLICKED(IDOK, &LineShadowCutDialog::OnBnClickedOk)
END_MESSAGE_MAP()

void LineShadowCutDialog::OnBnClickedX()
{
	LineCutPosDialog::OnBnClickedX();
}

void LineShadowCutDialog::OnBnClickedY()
{
	LineCutPosDialog::OnBnClickedY();
}

void LineShadowCutDialog::OnBnClickedZ()
{
	LineCutPosDialog::OnBnClickedZ();
}

void LineShadowCutDialog::onBnClickedSame()
{
	m_ViewDirection = 0;
}

void LineShadowCutDialog::onBnClickedOpposite()
{
	m_ViewDirection = 1;
}

void LineShadowCutDialog::OnBnPickCutPos()
{
	LineCutPosDialog::OnBnPickCutPos();
}

void LineShadowCutDialog::OnBnClickedOk()
{
	//得到用户输入的数据
	UpdateData(FALSE);

	//首先恢复视图
	CutBack();

	//得到切面
	GenerateCutPlane();

	//关闭对话框
	CAcUiDialog::OnOK();

	//得到要计算遮挡的实体集合
	LineCutPosDialog::GenerateCutRegion();

	//得到投影的视图
	GetViewPoint();

	//对切图进行投影
	GenerateShadow();

	//显示该图层
	ShowCutRegion();
}

void LineShadowCutDialog::GenerateCutPlane()
{
	LineCutPosDialog::GenerateCutPlane();

	CString cutLayerName(m_CutLayerName);
	if( m_ViewDirection == 0)
	{
		m_CutLayerName.Format(L"正方向-%s",cutLayerName.GetBuffer());
	}
	else if( m_ViewDirection == 1)
	{
		m_CutLayerName.Format(L"反方向-%s",cutLayerName.GetBuffer());
	}
}

void LineShadowCutDialog::CalculateBounds( PointEntity* pointEntity )
{
	assert(pointEntity);

	//计算左下角
	m_LeftDownCorner[0] = std::min<double>( m_LeftDownCorner[0], pointEntity->m_Point[0] );
	m_LeftDownCorner[1] = std::min<double>( m_LeftDownCorner[1], pointEntity->m_Point[1] );
	m_LeftDownCorner[2] = std::min<double>( m_LeftDownCorner[2], pointEntity->m_Point[2] );

	//计算右上角
	m_RightUpCorner[0] = std::max<double>( m_RightUpCorner[0], pointEntity->m_Point[0] );
	m_RightUpCorner[1] = std::max<double>( m_RightUpCorner[1], pointEntity->m_Point[1] );
	m_RightUpCorner[2] = std::max<double>( m_RightUpCorner[2], pointEntity->m_Point[2] );
}

void LineShadowCutDialog::GenerateCutRegion(LineEntity* lineEntry)
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
		PointEntity* pointEntity = (*pointIter);
		assert(pointEntity);

		//计算边界
		CalculateBounds(pointEntity);

		//起始点没有对应的实体
		if( pointIter == pointList->begin() )
			continue;

		acutPrintf(L"\n对第【%d】条线段进行遮挡切图！",pointEntity->m_PointNO);

		AcDbObjectId wallEntityId = pointEntity->m_DbEntityCollection.GetWallLineEntity();
		if( wallEntityId.isValid())
		{
			acutPrintf(L"\n使用其墙体！");
			m_collector.addEntity(wallEntityId);
		}
		else
		{
			AcDbObjectId lineEntityId = pointEntity->m_DbEntityCollection.GetLineEntity();
			if( lineEntityId.isValid() )
			{
				acutPrintf(L"\n使用管线体！");
				m_collector.addEntity(lineEntityId);
			}
		}
	}
}

void LineShadowCutDialog::Reset()
{
	acutPrintf(L"\n存在的切图为【%s】！",m_CutLayerName.GetBuffer());

	if( m_CutLayerName.GetLength() > 0 )
	{
		acutPrintf(L"\n首先锁定该文档");
		ArxWrapper::LockCurDoc();

		acutPrintf(L"\n恢复WCS视窗");
		acedCommand(RTSTR, _T("UCS"), RTSTR, L"W", 0);

		acutPrintf(L"\n删除切图相关的对象");
		CutBack();

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

		acutPrintf(L"\n切换至俯视");
		ArxWrapper::ChangeView(3);
	}
	else
	{
		acutPrintf(L"\n当前系统内没有切图！");
	}
}

void LineShadowCutDialog::GetViewPosition()
{
	int nPosIndex = 0;
	int nTargetIndex = 0;

	//得到垂直轴的方向
	if( m_Direction == 1 )
	{
		nPosIndex = 0;
	} 
	else if ( m_Direction == 2 )
	{
		nPosIndex = 1;
	} 
	else if( m_Direction == 3 )
	{
		nPosIndex = 2;
	}

	//得到观察的方向
	ads_point nPointPos;
	if( m_ViewDirection == 0 )
	{
		memcpy(nPointPos, m_RightUpCorner, 3 * sizeof(double));
	}
	else
	{
		memcpy(nPointPos, m_LeftDownCorner, 3 * sizeof(double));
	}

	m_ViewPosition[nPosIndex] = m_strOffset;
	m_TargetPosition[nPosIndex] = nPointPos[nPosIndex];
}

Adesk::Boolean LineShadowCutDialog::GetViewPoint()
{
	//得到观察点和目标点
	GetViewPosition();

	acdbUcs2Wcs(m_ViewPosition, m_ViewPosition, Adesk::kFalse ) ;
    acdbUcs2Wcs(m_TargetPosition, m_TargetPosition, Adesk::kFalse ) ;

    m_ShadowViewPort.setViewTarget(asPnt3d (m_TargetPosition)) ;
    m_ShadowViewPort.setViewDirection(asPnt3d (m_ViewPosition) - asPnt3d (m_TargetPosition)) ;

    m_ShadowViewPort.setFrontClipDistance(asPnt3d (m_ViewPosition).distanceTo (asPnt3d (m_TargetPosition))) ;
    m_ShadowViewPort.setBackClipDistance(0) ;

    m_ShadowViewPort.setFrontClipOn() ;    
    return (Adesk::kTrue) ;
}

void LineShadowCutDialog::GenerateShadow()
{
	//计算投影
	int control = kProject | kEntity | kBlock | kHonorInternals;
	AsdkHlrEngine hlr(&m_ShadowViewPort, control) ;
	Acad::ErrorStatus es = hlr.run(m_collector) ;

	if( es != Acad::eOk )
	{
		acutPrintf(L"\n计算投影结果失败了!");
		rxErrorMsg(es);
	}

	actrTransactionManager->startTransaction () ;

	//处理投影结果
	int nOutput =m_collector.mOutputData.logicalLength () ;
    acutPrintf (ACRX_T("\n投影结果中有【%d】个曲线"), nOutput) ;

	//创建投影所在的图层
	if( ArxWrapper::createNewLayer(m_CutLayerName.GetBuffer()) == false )
		return;

    for ( int j =0 ; j < nOutput ; j++ ) {
        AsdkHlrData *pResult = m_collector.mOutputData[j] ;
        AcDbEntity *pResultEntity =pResult->getResultEntity () ;

        AcDbObjectId id =  ArxWrapper::PostToModelSpace( pResultEntity, m_CutLayerName.GetBuffer());
		m_CutObjects.append(id);
    }

    actrTransactionManager->endTransaction();

	acutPrintf(L"\n观察点坐标为x【%02.lf】y【%02.lf】z【%02.lf】",m_ViewPosition[0],m_ViewPosition[1],m_ViewPosition[2]);
	acutPrintf(L"\n目标点坐标为x【%02.lf】y【%02.lf】z【%02.lf】",m_TargetPosition[0],m_TargetPosition[1],m_TargetPosition[2]);
}

void LineShadowCutDialog::ShowCutRegion()
{
	//只显示切面图层
	ArxWrapper::ShowLayer(m_CutLayerName.GetBuffer());

	int direction = m_Direction;
	if( m_ViewDirection == 1 )
	{
		direction +=3;
	}

	//切换视图
	ArxWrapper::ChangeView(direction);
}

void LineShadowCutDialog::CutBack()
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
