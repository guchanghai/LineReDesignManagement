#pragma once

#include <resource.h>
#include <dbsymtb.h>
#include <dbapserv.h>
#include <adslib.h>
#include <adui.h>
#include <acui.h>
#include "afxwin.h"
#include <dbents.h>

#include <LineEntryData.h>
#include <LineCutPosDialog.h>

#include <Hlr.h>

using namespace com::guch::assistant::data;

// LineCutPosDialog dialog

class LineShadowCutDialog : public LineCutPosDialog
{
	DECLARE_DYNAMIC(LineShadowCutDialog)

public:

	// Dialog Data
	enum { IDD = IDD_DIALOG_SHADOW_POS };

	LineShadowCutDialog(int dialogId, CWnd* pParent = NULL);   // standard constructor
	virtual ~LineShadowCutDialog();

	BOOL OnInitDialog();

	static afx_msg void Reset();

protected:

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

protected:
	
	afx_msg void OnBnClickedX();

	afx_msg void OnBnClickedY();

	afx_msg void OnBnClickedZ();

	afx_msg void onBnClickedSame();

	afx_msg void onBnClickedOpposite();

	virtual afx_msg void OnBnClickedOk();

	afx_msg void OnBnPickCutPos();

	//生成遮挡图所在的图层
	virtual void GenerateCutPlane();

protected:
	
	//选取的方向
	CButton m_DirectionSame;
	CButton m_DirectionOpposite;

	//观察的方向
	int m_ViewDirection;

private:

	//对一根管线进行遮挡运算，得到切割后的实体
	virtual void GenerateCutRegion(LineEntity* lineEntry);

	//对裁剪过的实体进行投影
	virtual void GenerateShadow();

	//删除上次切图中产生的结果对象
	static void CutBack();

	//得到投影的视图
	Adesk::Boolean GetViewPoint();

	//计算实体的边界
	void CalculateBounds( PointEntity* pointEntity );

	//计算观察者的位置和目标
	void GetViewPosition();

	//显示遮挡的投影所在的图层
	void ShowCutRegion();

private:

	//用于收敛所有的实体
	AsdkHlrCollector m_collector;

	//投影的视图
	AcDbViewport m_ShadowViewPort;

	//实体的控件范围（左下与右上）
	ads_point m_LeftDownCorner;
	ads_point m_RightUpCorner;

	//遮挡点的位置和目标位置
	ads_point m_ViewPosition;
	ads_point m_TargetPosition;
};
