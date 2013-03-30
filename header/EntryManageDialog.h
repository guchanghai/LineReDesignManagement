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

	EntryManageDialog(CWnd* pParent,const wstring& entryKind);   // standard constructor
	virtual ~EntryManageDialog();

	// Dialog Data
	enum { IDD = IDD_DIALOG_ENTRY_MANAGE };
	typedef enum { OPER_INIT, OPER_ADD, OPER_UPDATE } OPER_TYPE;

protected:
	
	DECLARE_MESSAGE_MAP()

	static UINT GetDlgID( const wstring& entryKind );

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
	void RemoveLineFromTree( HTREEITEM& lineTreeNode );

	//按钮相应
	afx_msg void OnBnClickedButtonOK();
	afx_msg void OnBnClickedButtonAdd();
	afx_msg void OnBnClickedButtonDel();

	//控件相关
	virtual afx_msg void OnTreeSelChanged(LPNMHDR pnmhdr, LRESULT *pLResult);
	virtual afx_msg void OnCbnShapeChange();
	virtual afx_msg void OnCbnCategoryChange();
	virtual afx_msg void OnControlValueChange();

	//动态控件
	virtual void ShowDynamicControl();
	virtual void HideDynamicControl();
	virtual void FillComobBox(CComboBox& comboBox, const wstring& value);

	//穿越方向相关
	void FillLineThroughDirection( const wstring& throughDirection);
	void ClearLineThroughDirection();
	CString GetLineThrough();

	//管理控制
	void SetOperType( OPER_TYPE type ) { m_OperType = type; }

	//管线操作
	LineEntry* GetSelectLine();
	LineEntry* GetLineEntry( const UINT& ID );
	BOOL InsertLine( LineEntry* lineEntry, BOOL bInitialize = FALSE );
	BOOL UpdateLine( LineEntry* lineEntry );

	//控件控制
	void EnableDetailControl(bool enable);

	//坐标点事件
	static void LinePointModified(void* dialog, int row);
	void CheckDuplicateValue( int row, BOOL excludeLast );

private:

	//窗口类别
	wstring m_EntryKind;

	//实体列表
	CTreeCtrl m_LinesTree;

	//详细信息
	CComboBox m_LineCategory;
	CComboBox m_LineShape;

	//实体大小（由管线形状决定）
	CStatic m_StaticDynamic_1;
	CEdit m_EditDynamic_1;

	CStatic m_StaticDynamic_2;
	CEdit m_EditDynamic_2;

	CStatic m_StaticDynamic_3;
	CEdit m_EditDynamic_3;

	CStatic m_StaticDynamic_4;
	CEdit m_EditDynamic_4;

	CStatic m_StaticDynamic_5;
	CEdit m_EditDynamic_5;

	CEdit m_LineWallSize;
	CEdit m_LineSafeSize;

	CEdit m_LinePlaneDesc;
	CEdit m_LineCutDesc;

	//可以穿越的方向
	CButton m_ThroughLeft;
	CButton m_ThroughRight;
	CButton m_ThroughFront;
	CButton m_ThroughBack;
	CButton m_ThroughAbove;
	CButton m_ThroughBellow;

	CAcUiNumericEdit m_PointEdit;

	//坐标数据
	CListCtrlEx m_LineDetailList;

	//显示重复左边点提醒信息
	CStatic m_StaticDuplicateWraning;

	//操作类型
	OPER_TYPE m_OperType;

	//页面按钮
	CButton m_ButtonAdd;
	CButton m_ButtonDel;

	CButton m_ButtonOK;

	//代表的文件名
	wstring m_fileName;

	//实体文件管理器
	LineEntryFile* m_EntryFile;
};

} // end of config

} // end of assistant

} // end of guch

} // end of com
