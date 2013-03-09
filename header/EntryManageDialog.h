#pragma once

#include "afxcmn.h"
#include <resource.h>

#include <dbsymtb.h>
#include <dbapserv.h>
#include <adslib.h>
#include <adui.h>
#include <acui.h>

#include <string>
#include <LineEntryData.h>
#include <ListCtrlEx.h>

using namespace std;

using namespace com::guch::assistant::data;

namespace com
{

namespace guch
{

namespace assistant
{

namespace entry
{

//实体管理窗口
class EntryManageDialog : public CDialog
{
	DECLARE_DYNAMIC(EntryManageDialog)

public:

	EntryManageDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~EntryManageDialog();

	// Dialog Data
	enum { IDD = IDD_DIALOG_ENTRY_MANAGE };
	typedef enum { OPER_INIT, OPER_ADD, OPER_UPDATE } OPER_TYPE;

protected:
	
	DECLARE_MESSAGE_MAP()

	//对话框数据
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	LineCategoryItemData* CreateEntryDetailInfo();
	PointList* CreateEntryPointList();
		
	void FillLineData( LineEntry* lineEntry );
	void ClearLineData();
	void CheckUIData();

	//初始化页面数据
	BOOL InitEntryListControl();
	BOOL InitEntryDetailControl();
	BOOL InitEntryPointsControl();
	BOOL InitEntryData();
	BOOL InitEntryPointsData(LineEntry* lineEntry);

	//左边列表相关操作
	HTREEITEM GetKindNode( const wstring& category, bool createOnDemand = false );
	HTREEITEM FindKindNode( const UINT& lineID );
	HTREEITEM InsertTreeNode( const wstring& lineCategory, const wstring& lineName );
	
	//按钮相应
	afx_msg void OnBnClickedButtonOK();
	afx_msg void OnBnClickedButtonAdd();
	afx_msg void OnBnClickedButtonDel();

	//控件相关
	virtual afx_msg void OnTreeSelChanged(LPNMHDR pnmhdr, LRESULT *pLResult);
	virtual afx_msg void OnCbnShapeChange();
	virtual afx_msg void OnControlValueChange();

	virtual void ShowControlDynamic();

	//管理控制
	void SetOperType( OPER_TYPE type ) { m_OperType = type; }

	//管线操作
	LineEntry* GetSelectLine();
	LineEntry* GetLineEntry( const UINT& ID );
	BOOL InsertLine( LineEntry* lineEntry, BOOL bInitialize = FALSE );
	BOOL UpdateLine( LineEntry* lineEntry );

	//控件控制
	void EnableDetailControl(bool enable);

private:

	//窗口类别
	wstring m_EntryKind;

	//实体列表
	CTreeCtrl m_LinesTree;

	//详细信息
	CComboBox m_LineCategory;
	CComboBox m_LineShape;

	CStatic m_StaticRadius;
	CEdit m_LineRadius;

	CStatic m_StaticWidth;
	CEdit m_LineWidth;

	CStatic m_StaticHeight;
	CEdit m_LineHeight;

	CEdit m_LineWallSize;
	CEdit m_LineSafeSize;

	CEdit m_LinePlaneDesc;
	CEdit m_LineCutDesc;

	CAcUiNumericEdit m_PointEdit;

	//坐标数据
	CListCtrlEx m_LineDetailList;

	//操作类型
	OPER_TYPE m_OperType;

	//数据是否有改变
	BOOL m_bDataDirty;

	//页面按钮
	CButton m_ButtonAdd;
	CButton m_ButtonDel;

	CButton m_ButtonOK;

	//代表的文件名
	wstring m_fileName;

	//实体文件管理器
	LineEntryFile* m_EntryFile;

	HTREEITEM m_lineRoot;
	HTREEITEM m_blockRoot;
};

} // end of config

} // end of assistant

} // end of guch

} // end of com
