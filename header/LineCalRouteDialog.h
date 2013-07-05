#pragma once

#include <resource.h>
#include <dbsymtb.h>
#include <dbapserv.h>
#include <adslib.h>
#include <adui.h>
#include <acui.h>
#include "afxwin.h"

#include <LineEntryData.h>

using namespace com::guch::assistant::data;

// LineCalRouteDialog dialog

class LineCalRouteDialog : public CAcUiDialog
{
	DECLARE_DYNAMIC(LineCalRouteDialog)

public:

	// Dialog Data
	enum { IDD = IDD_DIALOG_CAL_ROUTE };

	LineCalRouteDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~LineCalRouteDialog();

	virtual BOOL OnInitDialog();

	static afx_msg void Reset();

protected:

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

protected:

	afx_msg void OnBnPickStartClicked();

	afx_msg void OnBnPickEndClicked();

	virtual afx_msg void OnBnClickedOk();

protected:

	//删除上次切图中产生的结果对象
	static void CutBack();

protected:

	//选取起始、截止点
	CAcUiPickButton m_PickStart;
	CAcUiPickButton m_PickEnd;

	//开始点
	CAcUiNumericEdit m_StartX;
	CAcUiNumericEdit m_StartY;
	CAcUiNumericEdit m_StartZ;

	//终止点
	CAcUiNumericEdit m_EndX;
	CAcUiNumericEdit m_EndY;
	CAcUiNumericEdit m_EndZ;

	AcGePoint3d m_startPoint;
	AcGePoint3d m_endPoint;

	//计算的管线的名称，用于生成新的图层
	static CString m_CutLayerName;

	//计算路由过程中产生的新的数据库对象
	static AcDbObjectIdArray m_CutObjects;
};
