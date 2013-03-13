// EntryManageDialog.cpp : implementation file
//

#include "stdafx.h"
//#include "afxdialogex.h"

#include <GlobalDataConfig.h>
#include <EntryManageDialog.h>
#include <LMAException.h>
#include <LMAUtils.h>
#include <LineConfigDataManager.h>
#include <ArxWrapper.h>

using namespace com::guch::assistant::data;
using namespace com::guch::assistant::exception;
using namespace com::guch::assistant::config;

namespace com
{

namespace guch
{

namespace assistant
{

namespace entry
{

// EntryManageDialog dialog

IMPLEMENT_DYNAMIC(EntryManageDialog, CDialog)

EntryManageDialog::EntryManageDialog(CWnd* pParent /*=NULL*/)
	: CDialog(EntryManageDialog::IDD, pParent)
{
	//坐标点控件需要回调
	m_LineDetailList.m_Callback = EntryManageDialog::LinePointModified;
	m_LineDetailList.m_ParentDialog = (void*)this;

	//得到当前管理的文档
	m_fileName = curDoc()->fileName();
	acutPrintf(L"\n弹出对话框管理【%s】的数据.",m_fileName.c_str());

	//得到实体数据文件中的数据
	m_EntryFile = LineEntryFileManager::RegisterEntryFile(m_fileName);
	acutPrintf(L"\n当前文件有【%d】条管线.",m_EntryFile->GetList() ? m_EntryFile->GetList()->size() : 0 );
}

BOOL EntryManageDialog::OnInitDialog()
{
	//和页面交互数据
	CDialog::OnInitDialog();

	//初始化左边管线列表
	InitEntryListControl();

	//初始化管线详细控件
	InitEntryDetailControl();

	//初始化折线段信息
	InitEntryPointsControl();

	//默认控件不可用
	EnableDetailControl(false);

	//删除、确认按钮不可用
	m_ButtonDel.EnableWindow(false);
	m_ButtonOK.EnableWindow(false);

	//显示动态元素
	UpdateData(true);
	ShowDynamicControl();

	return TRUE;
}

BOOL EntryManageDialog::InitEntryListControl()
{
	acutPrintf(L"\n初始化管线实例数据.");

#ifdef _DEMO_DATA
	HTREEITEM hKindItem ,hCatogreyItem,kLineItem;

	//在根结点上添加"管线"
	hKindItem = m_LinesTree.InsertItem(L"管线",TVI_ROOT);

	//在“管线”下面插入分类数据
	hCatogreyItem = m_LinesTree.InsertItem(L"水管",hKindItem);

	//插入具体管线
	kLineItem = m_LinesTree.InsertItem(L"水管#1",hCatogreyItem);
	kLineItem = m_LinesTree.InsertItem(L"水管#2",hCatogreyItem,kLineItem);

	//插入其他种类
	hCatogreyItem = m_LinesTree.InsertItem(L"暖气",hKindItem,hCatogreyItem);//在Parent1上添加一个子结点，排在Child1_1后面
	hCatogreyItem = m_LinesTree.InsertItem(L"电线",hKindItem,hCatogreyItem);

	hKindItem = m_LinesTree.InsertItem(L"阻隔体",TVI_ROOT,hKindItem);   

	//在“阻隔体”下面插入分类数据
	hCatogreyItem = m_LinesTree.InsertItem(L"巷道",hKindItem);

	//插入具体管线
	kLineItem = m_LinesTree.InsertItem(L"巷道#1",hCatogreyItem);
	kLineItem = m_LinesTree.InsertItem(L"巷道#2",hCatogreyItem,kLineItem);

#else
	LineList* lineList = m_EntryFile->GetList();

	//初始化左边栏树形数据
	for( LineIterator iter = lineList->begin();
			iter != lineList->end();
			iter++)
	{
		InsertLine((*iter),TRUE);
	}

	//默认展开根节点
	m_LinesTree.Expand(TVI_ROOT, TVE_EXPAND);
#endif
	return TRUE;
}

BOOL EntryManageDialog::InitEntryDetailControl()
{
	//初始化管线种类列表
	{
		int index = 0;
		LineCommonConfigVector* lineKindConfig = LineConfigDataManager::Instance()->FindConfig( GlobalData::CONFIG_LINE_KIND);

		for( ConfigIterator iter = lineKindConfig->begin();
			iter != lineKindConfig->end();
			iter++)
		{
			m_LineCategory.InsertString(index++,(*iter)->mName.c_str());
		}

		delete lineKindConfig;
		m_LineCategory.SetCurSel(0);
	}

	//初始化管线切面类型列表
	{
		int index = 0;

		/* TODO From configuration file, the string is different from the hard code in source file
		LineCommonConfigVector* lineShapeKind = LineConfigDataManager::Instance()->FindConfig( GlobalData::CONFIG_SHAPE_KIND);

		for( ConfigIterator iter = lineShapeKind->begin();
			iter != lineShapeKind->end();
			iter++)
		{
			//m_LineShape.InsertString(index++,(*iter)->mName.c_str());
		}

		delete lineShapeKind;
		*/

		m_LineShape.InsertString(index++,GlobalData::LINE_SHAPE_CIRCLE.c_str());
		m_LineShape.InsertString(index++,GlobalData::LINE_SHAPE_SQUARE.c_str());
		m_LineShape.InsertString(index++,GlobalData::LINE_SHAPE_GZQPD.c_str());
		m_LineShape.InsertString(index++,GlobalData::LINE_SHAPE_GZQYG.c_str());
		m_LineShape.InsertString(index++,GlobalData::LINE_SHAPE_QQMTX.c_str());

		
		m_LineShape.SetCurSel(0);
	}

	return TRUE;
}

HTREEITEM EntryManageDialog::GetKindNode(const wstring& category, bool createOnDemand )
{
	TVITEM item;

	HTREEITEM hCurrent;

	hCurrent = m_LinesTree.GetNextItem(TVI_ROOT, TVGN_CHILD);
	while (hCurrent != NULL) 
	{
	   // Get the text for the item. Notice we use TVIF_TEXT because
	   // we want to retrieve only the text, but also specify TVIF_HANDLE
	   // because we're getting the item by its handle.
	   TCHAR szText[1024];
	   item.hItem = hCurrent;
	   item.mask = TVIF_TEXT | TVIF_HANDLE;
	   item.pszText = szText;
	   item.cchTextMax = 1024;

	   BOOL bWorked = m_LinesTree.GetItem(&item);

	   // Try to get the next item
	   hCurrent = m_LinesTree.GetNextItem(hCurrent, TVGN_CHILD);

	   // If we successfuly retrieved an item, and the item's text
	   // is the line kind
	   if (bWorked && wstring(item.pszText) == category)
	   {
#ifdef DEBUG
		   acutPrintf(L"\n在树上找到了这种类型【%s】.",category);
#endif
		  //m_LinesTree.DeleteItem(item.hItem);
		  return item.hItem;
	   }
	}

	if( createOnDemand )
	{
		//not find the kind, creat a new kind
#ifdef DEBUG
		acutPrintf(L"\n在树上没有找到了这种类型【%s】，得创建.",category.c_str());
#endif

		hCurrent = m_LinesTree.InsertItem(category.c_str(),hCurrent);
		return hCurrent;
	}
}

HTREEITEM EntryManageDialog::FindKindNode( const UINT& lineID)
{
	HTREEITEM hCurrent = m_LinesTree.GetRootItem();
	while (hCurrent != NULL) 
	{
	   // Get the ID for the item.
	   UINT currentID = (UINT)m_LinesTree.GetItemData(hCurrent);

	   // Check the ID
	   if( currentID == lineID)
	   {
#ifdef DEBUG
		   acutPrintf(L"\n在树上找到了这种ID【%d】.",lineID);
#endif
		  //m_LinesTree.DeleteItem(item.hItem);
		  return hCurrent;
	   }

	   // Try to get the next visible item
	   hCurrent = m_LinesTree.GetNextItem(hCurrent, TVGN_NEXTVISIBLE);
	}

	return hCurrent;
}

BOOL EntryManageDialog::InsertLine( LineEntry* lineEntry, BOOL bInitialize )
{
	//判断其合法性
	try
	{
		if( !lineEntry )
		{
			CString errorMsg;
			errorMsg.Format(L"数据为空");

			throw ErrorException(errorMsg.GetBuffer());
		}

		LineEntry* pLine = NULL;
		if( !bInitialize )
		{
			if( pLine = m_EntryFile->FindLineByName( lineEntry->m_LineName ) )
			{
				CString errorMsg;
				errorMsg.Format(L"已有管线【%s】也是这个名字，换一个吧",lineEntry->m_LineName.c_str());

				throw ErrorException(errorMsg.GetBuffer());
			}
		}
	}
	catch(const ErrorException& e)
	{
		MessageBoxW(e.errMsg.c_str(), GlobalData::ERROR_DIALOG_CAPTION.c_str(), MB_OK);
		return FALSE;
	}

	//得到归属的节点
	HTREEITEM parentNode = GetKindNode(lineEntry->m_LineBasiInfo->mCategory,true);
	
	//插入该实体名称
	HTREEITEM newItem = m_LinesTree.InsertItem(lineEntry->m_LineName.c_str(),parentNode);
		
	//默认插入之后，默认打开该节点的父节点
	m_LinesTree.Expand(parentNode, TVE_EXPAND);

	//保持本节点可见
	m_LinesTree.EnsureVisible(newItem);

	//如果不是从数据文件初始化，则是用户手工新增
	if( !bInitialize )
	{		
		//保存数据到管理器
		m_EntryFile->InsertLine(lineEntry);

		//保存到数据库
		ArxWrapper::PostToNameObjectsDict(lineEntry->pDbEntry,LineEntry::LINE_ENTRY_LAYER);

		//设置该项为选中
		m_LinesTree.Select(newItem, TVGN_CARET);
	}

	//设置改项的ID
	m_LinesTree.SetItemData(newItem,(DWORD_PTR)lineEntry->m_LineID);

	return TRUE;
}

BOOL EntryManageDialog::UpdateLine( LineEntry* lineEntry )
{
	//判断其合法性
	try
	{
		if( !lineEntry )
		{
			CString errorMsg;
			errorMsg.Format(L"数据为空");

			throw ErrorException(errorMsg.GetBuffer());
		}
	}
	catch(const ErrorException& e)
	{
		MessageBoxW(e.errMsg.c_str(), GlobalData::ERROR_DIALOG_CAPTION.c_str(), MB_OK);
		return FALSE;
	}

	//得到其节点
	HTREEITEM hItem = FindKindNode(lineEntry->m_LineID);

	//打开父节点
	HTREEITEM hParent = m_LinesTree.GetParentItem(hItem);
	if (hParent != NULL)
		m_LinesTree.Expand(hParent, TVE_EXPAND);

	//保持本节点可见
	m_LinesTree.EnsureVisible(hItem);

	//设置新名称
	if( hItem )
	{
		//TODO 重命名
		m_LinesTree.SetItemText(hItem, lineEntry->m_LineName.c_str());

		//保存到数据库
		ArxWrapper::PostToNameObjectsDict(lineEntry->pDbEntry,lineEntry->LINE_ENTRY_LAYER);

		//保存数据
		m_EntryFile->UpdateLine(lineEntry);

		//保存到导出文件
		m_EntryFile->Persistent();
	}

	return TRUE;
}

BOOL EntryManageDialog::InitEntryPointsControl()
{
#ifdef DEBUG
	acutPrintf(L"\n初始化管线中点坐标的数据.");
#endif

	int index = 0;

	//插入各列
	m_LineDetailList.InsertColumn(0, L"编号", LVCFMT_LEFT, 80);
	m_LineDetailList.InsertColumn(1, L"X坐标", LVCFMT_LEFT, 100);
	m_LineDetailList.InsertColumn(2, L"Y坐标", LVCFMT_LEFT, 100);
	m_LineDetailList.InsertColumn(3, L"高程", LVCFMT_LEFT, 100);

	//编号栏只读
	m_LineDetailList.SetColumnReadOnly(0);

	//默认编辑框编辑
	m_LineDetailList.SetDefaultEditor(NULL, NULL, &m_PointEdit);

	m_LineDetailList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );

	return TRUE;
}

BOOL EntryManageDialog::InitEntryPointsData(LineEntry* lineEntry)
{
	m_LineDetailList.DeleteAllItems();

	acutPrintf(L"\n初始化坐标信息.");

	if( lineEntry && lineEntry->m_PointList )
	{
		acutPrintf(L"\n共有折线段【%d】条.",lineEntry->m_PointList->size()-1);
		int index = 0;
		for( PointIter iter = lineEntry->m_PointList->begin();
			iter != lineEntry->m_PointList->end();
			iter++)
		{
			if(*iter)
			{
				CString temp;

				//编号
				temp.Format(L"%d",index+1);
				m_LineDetailList.InsertItem(index,temp);

				//x
				temp.Format(L"%0.2f",(*iter)->m_Point[X]);
				m_LineDetailList.SetItemText(index, 1, temp);

				//y
				temp.Format(L"%0.2f",(*iter)->m_Point[Y]);
				m_LineDetailList.SetItemText(index, 2, temp);
				
				//z
				temp.Format(L"%0.2f",(*iter)->m_Point[Z]);
				m_LineDetailList.SetItemText(index, 3, temp);

				index++;
			}
		}
	}

	UpdateData(FALSE);

	return TRUE;
}

EntryManageDialog::~EntryManageDialog()
{
#ifdef DEBUG
	if( m_EntryFile )
		acutPrintf(L"\n数据文件【%s】的实体管理对话框关闭了.",m_EntryFile->m_FileName.c_str());
#endif
}

void EntryManageDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	//列表
	DDX_Control(pDX, IDC_TREE_LINES, m_LinesTree);

	//详细信息
	DDX_Control(pDX, IDC_COMBO_CATEGORY, m_LineCategory);
	DDX_Control(pDX, IDC_COMBO_SHAPE, m_LineShape);

	DDX_Control(pDX, IDC_STATIC_DYNAMIC_1, m_StaticDynamic_1);
	DDX_Control(pDX, IDC_EDIT_DANAMIC_1, m_EditDynamic_1);

	DDX_Control(pDX, IDC_STATIC_DYNAMIC_2, m_StaticDynamic_2);
	DDX_Control(pDX, IDC_EDIT_DANAMIC_2, m_EditDynamic_2);

	DDX_Control(pDX, IDC_STATIC_DYNAMIC_3, m_StaticDynamic_3);
	DDX_Control(pDX, IDC_EDIT_DANAMIC_3, m_EditDynamic_3);

	DDX_Control(pDX, IDC_STATIC_DYNAMIC_4, m_StaticDynamic_4);
	DDX_Control(pDX, IDC_EDIT_DANAMIC_4, m_EditDynamic_4);

	DDX_Control(pDX, IDC_STATIC_DYNAMIC_5, m_StaticDynamic_5);
	DDX_Control(pDX, IDC_EDIT_DANAMIC_5, m_EditDynamic_5);

	DDX_Control(pDX, IDC_EDIT_WALL_SIZE,m_LineWallSize);
	DDX_Control(pDX, IDC_EDIT_SAFESIZE,m_LineSafeSize);

	DDX_Control(pDX, IDC_EDIT_PLANE_MARK,m_LinePlaneDesc);
	DDX_Control(pDX, IDC_EDIT_CUT_MARK,m_LineCutDesc);

	//折线段
	DDX_Control(pDX, IDC_EDIT_POINT, m_PointEdit);
	DDX_Control(pDX, IDC_LIST_LINE_DETAIL, m_LineDetailList);

	//按钮
	DDX_Control(pDX, IDC_BUTTON_ADD, m_ButtonAdd);
	DDX_Control(pDX, IDC_BUTTON_DEL, m_ButtonDel);

	DDX_Control(pDX, IDOK, m_ButtonOK);
}

LineEntry* EntryManageDialog::GetLineEntry( const UINT& ID )
{
	return m_EntryFile->FindLine(ID);
}

BEGIN_MESSAGE_MAP(EntryManageDialog, CDialog)

	//按钮响应
	ON_BN_CLICKED(IDOK, OnBnClickedButtonOK)

	ON_BN_CLICKED(IDC_BUTTON_ADD, OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_DEL, OnBnClickedButtonDel)
	
	//管线选中
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_LINES, OnTreeSelChanged)

	//切面类型选中
	ON_CBN_SELENDOK(IDC_COMBO_CATEGORY,		OnCbnCategoryChange)
	ON_CBN_SELCHANGE(IDC_COMBO_CATEGORY,	OnControlValueChange)

	ON_CBN_SELCHANGE(IDC_COMBO_SHAPE,		OnCbnShapeChange)
	ON_CBN_SELCHANGE(IDC_COMBO_SHAPE,		OnControlValueChange)
	
	ON_EN_CHANGE(IDC_EDIT_DANAMIC_1,		OnControlValueChange)
	ON_EN_CHANGE(IDC_EDIT_DANAMIC_2,		OnControlValueChange)
	ON_EN_CHANGE(IDC_EDIT_DANAMIC_3,		OnControlValueChange)

	ON_EN_CHANGE(IDC_EDIT_WALL_SIZE,		OnControlValueChange)
	ON_EN_CHANGE(IDC_EDIT_SAFESIZE,			OnControlValueChange)
	
	ON_EN_CHANGE(IDC_EDIT_PLANE_MARK,		OnControlValueChange)
	ON_EN_CHANGE(IDC_EDIT_CUT_MARK,			OnControlValueChange)

	ON_BN_DOUBLECLICKED(IDC_LIST_LINE_DETAIL ,		OnControlValueChange)

END_MESSAGE_MAP()

void EntryManageDialog::OnBnClickedButtonOK()
{
	//确认按钮不可用
	m_ButtonOK.EnableWindow(false);

	//得到管线详细信息
	LineCategoryItemData* detailInfo = CreateEntryDetailInfo();

	//得到折线段数据
	PointList* pointList = CreateEntryPointList();

	if( m_OperType == OPER_ADD )
	{
		//得到实体名称
		wstring pipeName = m_EntryFile->GetNewPipeName(detailInfo,L"");
		acutPrintf(L"\n新增管线【%s】,折线段【%d】条.",pipeName.c_str(),pointList->size());

		//创建新的管线
		LineEntry* newLine = new LineEntry(pipeName,GlobalData::KIND_LINE,
											detailInfo,NULL);

		//生成该项的ID
		newLine->m_LineID = (UINT)GetTickCount();

		//设置折线段(在此进行重画)
		newLine->SetPoints( pointList );

		//添加新的管线（同时添加到数据库）
		InsertLine(newLine);

		//设置操作类型为更新
		SetOperType( OPER_UPDATE );
	}
	else /*if( m_OperType == OPER_UPDATE )*/
	{
		//得到当前编辑的直线
		LineEntry* selectLine = GetSelectLine();

		//设置新的数据
		if( selectLine )
		{
			acutPrintf(L"\n更新管线【%s】,折线段【%d】条.",
				selectLine->m_LineName.c_str(),
				pointList->size());

			//得到新的实体名称
			selectLine->m_LineName = m_EntryFile->GetNewPipeName(detailInfo,selectLine->m_LineName);

			//设置基本信息
			selectLine->SetBasicInfo( detailInfo );

			//设置折线段信息
			selectLine->SetPoints( pointList );

			//更新管线
			UpdateLine(selectLine);
		}
	}

	//默认进入XY视图
	acedCommand(RTSTR, _T("._-VIEW"), RTSTR, L"TOP", 0);

	//保存到临时文件
	m_EntryFile->Persistent();
}

LineEntry* EntryManageDialog::GetSelectLine()
{
	HTREEITEM selectedItem = m_LinesTree.GetSelectedItem();

	if( selectedItem == NULL)
		 return NULL;

	UINT selectedID = (UINT)m_LinesTree.GetItemData(selectedItem);

	return GetLineEntry(selectedID);
}

void EntryManageDialog::OnBnClickedButtonAdd()
{
	//判断当前数据是否需要保存
	CheckUIData();

	//所有空间可用
	EnableDetailControl(true);

	//清空所有的页面数据
	ClearLineData();

	//设置操作类型
	SetOperType(OPER_ADD);
}

void EntryManageDialog::OnBnClickedButtonDel()
{
	//选择的行
	HTREEITEM selectedItem = m_LinesTree.GetSelectedItem();

	if( selectedItem == NULL)
		 return;

	UINT selectedID = (UINT)m_LinesTree.GetItemData(selectedItem);

	//得到选择的数据
	LineEntry* pEntry = m_EntryFile->FindLine(selectedID);

	if( pEntry )
	{
		// Initializes the variables to pass to the MessageBox::Show method.
		CString message;
		message.Format(L"确实要删除[%s]吗?",pEntry->m_LineName.c_str());

		LPCTSTR caption = L"删除实体";

		// Displays the MessageBox.
		int result = MessageBoxW(message, caption, MB_OKCANCEL);
		if ( result == IDOK )
		{
			//从数据库删除管线本身
			ArxWrapper::PostToNameObjectsDict(pEntry->pDbEntry,pEntry->LINE_ENTRY_LAYER,true);

			//从数据库删除管线所有的线段
			ArxWrapper::eraseLMALine(*pEntry);

			//删除所有的内存节点
			pEntry->ClearPoints();

			//Delete from the list
			m_EntryFile->DeleteLine(selectedID);

			// delete the selected item. 
			m_LinesTree.DeleteItem(selectedItem);

			//保存到导出文件
			m_EntryFile->Persistent();
		}
	}
}

BOOL EntryManageDialog::InitEntryData()
{
	//选择的行
	HTREEITEM selectedItem = m_LinesTree.GetSelectedItem();

	UINT selectedID = (UINT)m_LinesTree.GetItemData(selectedItem);

	//得到选择的数据
	LineEntry* pEntry = m_EntryFile->FindLine(selectedID);

	//填充数据
	FillLineData(pEntry);

	return TRUE;
}

void EntryManageDialog::OnTreeSelChanged(LPNMHDR pnmhdr, LRESULT *pLResult)
{
	//如果有某一项被选中
	if( pnmhdr->code == TVN_SELCHANGED )
	{
		//选中其他节点时首先判断
		CheckUIData();

		//填充选中的数据
		InitEntryData();

		//设置确认按钮不可用
		m_ButtonOK.EnableWindow(false);

		//各控件可用
		EnableDetailControl(true);

		//不可修改种类
		m_LineCategory.EnableWindow(false);

		//显示数据
		UpdateData(false);

		//动态显示组件
		ShowDynamicControl();
	}
}

void EntryManageDialog::FillLineData( LineEntry* lineEntry )
{
	if( lineEntry )
	{
		//设置详细数据
		m_LineCategory.SetWindowText(lineEntry->m_LineBasiInfo->mCategory.c_str());
		FillShapeKind(lineEntry->m_LineBasiInfo->mShape);

		//填充数据
		if( lineEntry->m_LineBasiInfo->mShape == GlobalData::LINE_SHAPE_CIRCLE )
		{
			m_EditDynamic_1.SetWindowText(lineEntry->m_LineBasiInfo->mRadius.c_str());
		}
		else if ( lineEntry->m_LineBasiInfo->mShape == GlobalData::LINE_SHAPE_SQUARE )
		{
			m_EditDynamic_1.SetWindowText(lineEntry->m_LineBasiInfo->mWidth.c_str());
			m_EditDynamic_2.SetWindowText(lineEntry->m_LineBasiInfo->mHeight.c_str());
		}

		m_LineWallSize.SetWindowText(lineEntry->m_LineBasiInfo->mWallSize.c_str());
		m_LineSafeSize.SetWindowText(lineEntry->m_LineBasiInfo->mSafeSize.c_str());

		m_LinePlaneDesc.SetWindowText(lineEntry->m_LineBasiInfo->mPlaneMark.c_str());
		m_LineCutDesc.SetWindowText(lineEntry->m_LineBasiInfo->mCutMark.c_str());

		//设置坐标信息
		InitEntryPointsData(lineEntry);
	}

	UpdateData(FALSE);
}

void EntryManageDialog::FillShapeKind(const wstring& shape)
{
	for( int i = 0; i < m_LineShape.GetCount(); i++ )
	{
		CString shapeEntry;
		m_LineShape.GetLBText(i,shapeEntry);

		if( wstring(shapeEntry.GetBuffer()) == shape )
		{
			acutPrintf(L"\n选中第【%d】项");
			m_LineShape.SetCurSel(i);
		}
	}
}

LineCategoryItemData* EntryManageDialog::CreateEntryDetailInfo()
{
	UpdateData(TRUE);

	CString lineCategory,lineShape,lineRadius,lineWidth,lineHeight,
			lineSafeSize,lineWallSize,linePlaneDesc,lineCutDesc;

	//得到各个输入框的信息
	m_LineCategory.GetWindowTextW(lineCategory);
	m_LineShape.GetWindowTextW(lineShape);

	if( wstring(lineShape.GetBuffer()) == GlobalData::LINE_SHAPE_CIRCLE )
	{
			m_EditDynamic_1.GetWindowTextW(lineRadius);
	}
	else if ( wstring(lineShape.GetBuffer()) == GlobalData::LINE_SHAPE_SQUARE )
	{
		m_EditDynamic_1.GetWindowTextW(lineWidth);
		m_EditDynamic_2.GetWindowTextW(lineHeight);
	}
	else
	{
		lineRadius = L"0";
		lineWidth = L"0";
		lineHeight = L"0";
	}

	m_LineWallSize.GetWindowTextW(lineWallSize);
	m_LineSafeSize.GetWindowTextW(lineSafeSize);

	m_LinePlaneDesc.GetWindowTextW(linePlaneDesc);
	m_LineCutDesc.GetWindowTextW(lineCutDesc);

	//准备配置数据结构体
	LineCategoryItemData* categoryData = new LineCategoryItemData();

	categoryData->mCategory = lineCategory;
	categoryData->mShape = lineShape;

	categoryData->mRadius = lineRadius;
	categoryData->mWidth = lineWidth;
	categoryData->mHeight = lineHeight;

	categoryData->mWallSize = lineWallSize;
	categoryData->mSafeSize = lineSafeSize;

	categoryData->mPlaneMark = linePlaneDesc;
	categoryData->mCutMark = lineCutDesc;

	return categoryData;
}

PointList* EntryManageDialog::CreateEntryPointList()
{
	UpdateData(TRUE);

	PointList* newPoints = new PointList();

	CString temp;
	for( int i = 0; i < m_LineDetailList.GetItemCount(); i++ )
	{
		PointEntry* point = new PointEntry();

		//得到当前编号（及其在列表中的序列号）
		point->m_PointNO = (UINT)i;

		temp = m_LineDetailList.GetItemText(i,1);
		acdbDisToF(temp.GetBuffer(), -1, &((point->m_Point)[X]));

		temp = m_LineDetailList.GetItemText(i,2);
		acdbDisToF(temp.GetBuffer(), -1, &((point->m_Point)[Y]));

		temp = m_LineDetailList.GetItemText(i,3);
		acdbDisToF(temp.GetBuffer(), -1, &((point->m_Point)[Z]));

		//加入到队列中
		newPoints->push_back(point);
	}

	return newPoints;
}

void EntryManageDialog::ClearLineData()
{
	//清空详细数据
	m_LineCategory.SetCurSel(0);
	CString category;
	m_LineCategory.GetWindowTextW(category);
	wstring lineCategory(category.GetBuffer());

	m_LineShape.SetCurSel(0);
	ShowDynamicControl();

	wstring defaultSize = LineConfigDataManager::Instance()->FindDefaultSize(lineCategory);
	m_EditDynamic_1.SetWindowText(defaultSize.c_str());
	m_EditDynamic_2.SetWindowText(defaultSize.c_str());
	m_EditDynamic_3.SetWindowText(L"0");
	m_EditDynamic_4.SetWindowText(L"0");
	m_EditDynamic_5.SetWindowText(L"0");

	m_LineWallSize.SetWindowText(L"0");
	m_LineSafeSize.SetWindowText(L"0");

	m_LinePlaneDesc.SetWindowText(L"无");
	m_LineCutDesc.SetWindowText(L"无");

	//清空折线点数据
	m_LineDetailList.DeleteAllItems();

	//不可删除、更新
	m_ButtonDel.EnableWindow(false);
	m_ButtonOK.EnableWindow(false);

	UpdateData(FALSE);
}

void EntryManageDialog::EnableDetailControl(bool enable)
{
	m_LineCategory.EnableWindow(enable);
	m_LineShape.EnableWindow(enable);

	m_EditDynamic_1.EnableWindow(enable);
	m_EditDynamic_2.EnableWindow(enable);
	m_EditDynamic_3.EnableWindow(enable);
	m_EditDynamic_4.EnableWindow(enable);
	m_EditDynamic_5.EnableWindow(enable);

	m_LineWallSize.EnableWindow(enable);
	m_LineSafeSize.EnableWindow(enable);

	m_LinePlaneDesc.EnableWindow(enable);
	m_LineCutDesc.EnableWindow(enable);

	m_LineDetailList.EnableWindow(enable);
}

void EntryManageDialog::OnCbnCategoryChange()
{
	if( m_OperType == OPER_ADD )
	{
		UpdateData(true);

		CString category;
		m_LineCategory.GetWindowTextW(category);
		wstring lineCategory(category.GetBuffer());

		wstring defaultSize = LineConfigDataManager::Instance()->FindDefaultSize(lineCategory);
		m_EditDynamic_1.SetWindowText(defaultSize.c_str());
		m_EditDynamic_2.SetWindowText(defaultSize.c_str());
		m_EditDynamic_3.SetWindowText(L"0");
		m_EditDynamic_4.SetWindowText(L"0");
		m_EditDynamic_5.SetWindowText(L"0");
	}
}

void EntryManageDialog::OnCbnShapeChange()
{
	ShowDynamicControl();
}

void EntryManageDialog::OnControlValueChange()
{
	//确认按钮可用
	m_ButtonOK.EnableWindow(true);

	UpdateData(false);
}

void EntryManageDialog::ShowDynamicControl()
{
	UpdateData(true);

	//首先是隐藏所有的动态控件
	HideDynamicControl();

	//分类别显示动态控件
	CString shapeSelected;
	m_LineShape.GetWindowTextW(shapeSelected);
	wstring shape(shapeSelected.GetBuffer());

	int index = m_LineShape.GetCurSel();

	if( shape == GlobalData::LINE_SHAPE_CIRCLE )
	{
		m_StaticDynamic_1.SetWindowTextW(L"内径(mm)");

		m_StaticDynamic_1.ShowWindow( true );
		m_EditDynamic_1.ShowWindow( true );
	}
	else if( shape == GlobalData::LINE_SHAPE_SQUARE )
	{
		m_StaticDynamic_1.SetWindowTextW(L"净宽(mm)");
		m_StaticDynamic_2.SetWindowTextW(L"净高(mm)");

		m_StaticDynamic_1.ShowWindow( true );
		m_EditDynamic_1.ShowWindow( true );

		m_StaticDynamic_2.ShowWindow( true );
		m_EditDynamic_2.ShowWindow( true );
	}
	else if( shape == GlobalData::LINE_SHAPE_GZQPD )
	{
		m_StaticDynamic_1.SetWindowTextW(L"净宽(mm)");
		m_StaticDynamic_2.SetWindowTextW(L"矢高(mm)");
		m_StaticDynamic_3.SetWindowTextW(L"墙高(mm)");

		m_StaticDynamic_1.ShowWindow( true );
		m_EditDynamic_1.ShowWindow( true );

		m_StaticDynamic_2.ShowWindow( true );
		m_EditDynamic_2.ShowWindow( true );

		m_StaticDynamic_3.ShowWindow( true );
		m_EditDynamic_3.ShowWindow( true );
	}
	else if( shape == GlobalData::LINE_SHAPE_GZQYG )
	{
		m_StaticDynamic_1.SetWindowTextW(L"净宽(mm)");
		m_StaticDynamic_2.SetWindowTextW(L"上矢高(mm)");
		m_StaticDynamic_3.SetWindowTextW(L"下矢高(mm)");
		m_StaticDynamic_4.SetWindowTextW(L"墙高(mm)");

		m_StaticDynamic_1.ShowWindow( true );
		m_EditDynamic_1.ShowWindow( true );

		m_StaticDynamic_2.ShowWindow( true );
		m_EditDynamic_2.ShowWindow( true );

		m_StaticDynamic_3.ShowWindow( true );
		m_EditDynamic_3.ShowWindow( true );

		m_StaticDynamic_4.ShowWindow( true );
		m_EditDynamic_4.ShowWindow( true );
	}
	else if( shape == GlobalData::LINE_SHAPE_QQMTX )
	{
		m_StaticDynamic_1.SetWindowTextW(L"上矢宽(mm)");
		m_StaticDynamic_2.SetWindowTextW(L"下矢宽(mm)");
		m_StaticDynamic_3.SetWindowTextW(L"上矢高(mm)");
		m_StaticDynamic_4.SetWindowTextW(L"下矢高(mm)");
		m_StaticDynamic_5.SetWindowTextW(L"墙高(mm)");

		m_StaticDynamic_1.ShowWindow( true );
		m_EditDynamic_1.ShowWindow( true );

		m_StaticDynamic_2.ShowWindow( true );
		m_EditDynamic_2.ShowWindow( true );

		m_StaticDynamic_3.ShowWindow( true );
		m_EditDynamic_3.ShowWindow( true );

		m_StaticDynamic_4.ShowWindow( true );
		m_EditDynamic_4.ShowWindow( true );

		m_StaticDynamic_5.ShowWindow( true );
		m_EditDynamic_5.ShowWindow( true );
	}

	UpdateData(false);
}

void EntryManageDialog::HideDynamicControl()
{
	m_StaticDynamic_1.ShowWindow( false );
	m_EditDynamic_1.ShowWindow( false );

	m_StaticDynamic_2.ShowWindow( false );
	m_EditDynamic_2.ShowWindow( false );

	m_StaticDynamic_3.ShowWindow( false );
	m_EditDynamic_3.ShowWindow( false );

	m_StaticDynamic_4.ShowWindow( false );
	m_EditDynamic_4.ShowWindow( false );

	m_StaticDynamic_5.ShowWindow( false );
	m_EditDynamic_5.ShowWindow( false );
}

void EntryManageDialog::CheckUIData()
{
	if( m_ButtonOK.IsWindowEnabled() )
	{
		int result = MessageBoxW(L"数据已更改，是否保存", L"提醒", MB_OKCANCEL);

		if( result == IDOK )
		{
			OnBnClickedButtonOK();
		}
	}
}

void EntryManageDialog::LinePointModified(void* dialog)
{
	acutPrintf(L"\n管线管理器回调函数被调用");
	EntryManageDialog* entryDlg(NULL);

	if( entryDlg = static_cast<EntryManageDialog*>(dialog) )
	{
		acutPrintf(L"\n默认为有值发生了变化");
		//控件发生了值变化
		entryDlg->OnControlValueChange();
	}
}

// EntryManageDialog message handlers

} // end of entry

} // end of assistant

} // end of guch

} // end of com
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               