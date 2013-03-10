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
	//�����ؼ���Ҫ�ص�
	m_LineDetailList.m_Callback = EntryManageDialog::LinePointModified;
	m_LineDetailList.m_ParentDialog = (void*)this;

	//�õ���ǰ�������ĵ�
	m_fileName = curDoc()->fileName();
	acutPrintf(L"\n�����Ի��������%s��������.",m_fileName.c_str());

	//�õ�ʵ�������ļ��е�����
	m_EntryFile = LineEntryFileManager::RegisterEntryFile(m_fileName);
	acutPrintf(L"\n��ǰ�ļ��С�%d��������.",m_EntryFile->GetList() ? m_EntryFile->GetList()->size() : 0 );
}

BOOL EntryManageDialog::OnInitDialog()
{
	//��ҳ�潻������
	CDialog::OnInitDialog();

	//��ʼ����߹����б�
	InitEntryListControl();

	//��ʼ��������ϸ�ؼ�
	InitEntryDetailControl();

	//��ʼ�����߶���Ϣ
	InitEntryPointsControl();

	//Ĭ�Ͽؼ�������
	EnableDetailControl(false);

	//ɾ����ȷ�ϰ�ť������
	m_ButtonDel.EnableWindow(false);
	m_ButtonOK.EnableWindow(false);

	//��ʾ��̬Ԫ��
	ShowControlDynamic();

	return TRUE;
}

BOOL EntryManageDialog::InitEntryListControl()
{
	acutPrintf(L"\n��ʼ������ʵ������.");

#ifdef _DEMO_DATA
	HTREEITEM hKindItem ,hCatogreyItem,kLineItem;

	//�ڸ����������"����"
	hKindItem = m_LinesTree.InsertItem(L"����",TVI_ROOT);

	//�ڡ����ߡ���������������
	hCatogreyItem = m_LinesTree.InsertItem(L"ˮ��",hKindItem);

	//����������
	kLineItem = m_LinesTree.InsertItem(L"ˮ��#1",hCatogreyItem);
	kLineItem = m_LinesTree.InsertItem(L"ˮ��#2",hCatogreyItem,kLineItem);

	//������������
	hCatogreyItem = m_LinesTree.InsertItem(L"ů��",hKindItem,hCatogreyItem);//��Parent1������һ���ӽ�㣬����Child1_1����
	hCatogreyItem = m_LinesTree.InsertItem(L"����",hKindItem,hCatogreyItem);

	hKindItem = m_LinesTree.InsertItem(L"�����",TVI_ROOT,hKindItem);   

	//�ڡ�����塱��������������
	hCatogreyItem = m_LinesTree.InsertItem(L"���",hKindItem);

	//����������
	kLineItem = m_LinesTree.InsertItem(L"���#1",hCatogreyItem);
	kLineItem = m_LinesTree.InsertItem(L"���#2",hCatogreyItem,kLineItem);

#else
	LineList* lineList = m_EntryFile->GetList();

	//��ʼ���������������
	for( LineIterator iter = lineList->begin();
			iter != lineList->end();
			iter++)
	{
		InsertLine((*iter),TRUE);
	}

	//Ĭ��չ�����ڵ�
	m_LinesTree.Expand(TVI_ROOT, TVE_EXPAND);
#endif
	return TRUE;
}

BOOL EntryManageDialog::InitEntryDetailControl()
{
	//��ʼ�����������б�
	{
		int index = 0;
		LineCommonConfigVector* lineKindConfig = LineConfigDataManager::Instance()->FindConfig( GlobalData::CONFIG_LINE_KIND);

		for( ConfigIterator iter = lineKindConfig->begin();
			iter != lineKindConfig->end();
			iter++)
		{
			m_LineCategory.InsertString(index++,(*iter)->mName.c_str());
		}

		m_LineCategory.SetCurSel(0);
	}

	//��ʼ���������������б�
	{
		int index = 0;
		LineCommonConfigVector* lineShapeKind = LineConfigDataManager::Instance()->FindConfig( GlobalData::CONFIG_SHAPE_KIND);

		for( ConfigIterator iter = lineShapeKind->begin();
			iter != lineShapeKind->end();
			iter++)
		{
			m_LineShape.InsertString(index++,(*iter)->mName.c_str());
		}

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
		   acutPrintf(L"\n�������ҵ����������͡�%s��.",category);
#endif
		  //m_LinesTree.DeleteItem(item.hItem);
		  return item.hItem;
	   }
	}

	if( createOnDemand )
	{
		//not find the kind, creat a new kind
#ifdef DEBUG
		acutPrintf(L"\n������û���ҵ����������͡�%s�����ô���.",category.c_str());
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
		   acutPrintf(L"\n�������ҵ�������ID��%d��.",lineID);
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
	//�ж���Ϸ���
	try
	{
		if( !lineEntry )
		{
			CString errorMsg;
			errorMsg.Format(L"����Ϊ��");

			throw ErrorException(errorMsg.GetBuffer());
		}

		LineEntry* pLine = NULL;
		if( !bInitialize )
		{
			if( pLine = m_EntryFile->FindLineByName( lineEntry->m_LineName ) )
			{
				CString errorMsg;
				errorMsg.Format(L"���й��ߡ�%s��Ҳ��������֣���һ����",lineEntry->m_LineName.c_str());

				throw ErrorException(errorMsg.GetBuffer());
			}
		}
	}
	catch(const ErrorException& e)
	{
		MessageBoxW(e.errMsg.c_str(), GlobalData::ERROR_DIALOG_CAPTION.c_str(), MB_OK);
		return FALSE;
	}

	//�õ������Ľڵ�
	HTREEITEM parentNode = GetKindNode(lineEntry->m_LineBasiInfo->mCategory,true);
	
	//�����ʵ������
	HTREEITEM newItem = m_LinesTree.InsertItem(lineEntry->m_LineName.c_str(),parentNode);
		
	//Ĭ�ϲ���֮��Ĭ�ϴ򿪸ýڵ�ĸ��ڵ�
	m_LinesTree.Expand(parentNode, TVE_EXPAND);

	//���ֱ��ڵ�ɼ�
	m_LinesTree.EnsureVisible(newItem);

	//������Ǵ������ļ���ʼ���������û��ֹ�����
	if( !bInitialize )
	{		
		//�������ݵ�������
		m_EntryFile->InsertLine(lineEntry);

		//���浽���ݿ�
		ArxWrapper::PostToNameObjectsDict(lineEntry->pDbEntry,LineEntry::LINE_ENTRY_LAYER);

		//���ø���Ϊѡ��
		m_LinesTree.Select(newItem, TVGN_CARET);
	}

	//���ø����ID
	m_LinesTree.SetItemData(newItem,(DWORD_PTR)lineEntry->m_LineID);

	return TRUE;
}

BOOL EntryManageDialog::UpdateLine( LineEntry* lineEntry )
{
	//�ж���Ϸ���
	try
	{
		if( !lineEntry )
		{
			CString errorMsg;
			errorMsg.Format(L"����Ϊ��");

			throw ErrorException(errorMsg.GetBuffer());
		}

		LineEntry* pAnotherLine = NULL;

		if( pAnotherLine = m_EntryFile->HasAnotherLineByByName( lineEntry->m_LineID, lineEntry->m_LineName ) )
		{
			CString errorMsg;
			errorMsg.Format(L"���й��߽�������֣���һ����");

			throw ErrorException(errorMsg.GetBuffer());
		}
	}
	catch(const ErrorException& e)
	{
		MessageBoxW(e.errMsg.c_str(), GlobalData::ERROR_DIALOG_CAPTION.c_str(), MB_OK);
		return FALSE;
	}

	//�õ���ڵ�
	HTREEITEM hItem = FindKindNode(lineEntry->m_LineID);

	//�򿪸��ڵ�
	HTREEITEM hParent = m_LinesTree.GetParentItem(hItem);
	if (hParent != NULL)
		m_LinesTree.Expand(hParent, TVE_EXPAND);

	//���ֱ��ڵ�ɼ�
	m_LinesTree.EnsureVisible(hItem);

	//����������
	if( hItem )
	{
		//TODO ������
		m_LinesTree.SetItemText(hItem, lineEntry->m_LineName.c_str());

		//��������
		m_EntryFile->UpdateLine(lineEntry);

		//���浽���ݿ�
		ArxWrapper::PostToNameObjectsDict(lineEntry->pDbEntry,lineEntry->LINE_ENTRY_LAYER);

		//���浽�����ļ�
		m_EntryFile->Persistent();

		//������ϸ��Ϣ
		InitEntryPointsControl();
	}

	return TRUE;
}

BOOL EntryManageDialog::InitEntryPointsControl()
{
#ifdef DEBUG
	acutPrintf(L"\n��ʼ�������е����������.");
#endif

	int index = 0;

	//�������
	m_LineDetailList.InsertColumn(0, L"���", LVCFMT_LEFT, 80);
	m_LineDetailList.InsertColumn(1, L"X����", LVCFMT_LEFT, 100);
	m_LineDetailList.InsertColumn(2, L"Y����", LVCFMT_LEFT, 100);
	m_LineDetailList.InsertColumn(3, L"�߳�", LVCFMT_LEFT, 100);

	//�����ֻ��
	m_LineDetailList.SetColumnReadOnly(0);

	//Ĭ�ϱ༭��༭
	m_LineDetailList.SetDefaultEditor(NULL, NULL, &m_PointEdit);

	m_LineDetailList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );

	return TRUE;
}

BOOL EntryManageDialog::InitEntryPointsData(LineEntry* lineEntry)
{
	m_LineDetailList.DeleteAllItems();

	acutPrintf(L"\n��ʼ��������Ϣ.");

	if( lineEntry && lineEntry->m_PointList )
	{
		acutPrintf(L"\n�������߶Ρ�%d����.",lineEntry->m_PointList->size()-1);
		int index = 0;
		for( PointIter iter = lineEntry->m_PointList->begin();
			iter != lineEntry->m_PointList->end();
			iter++)
		{
			if(*iter)
			{
				CString temp;

				//���
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
		acutPrintf(L"\n�����ļ���%s����ʵ������Ի���ر���.",m_EntryFile->m_FileName.c_str());
#endif
}

void EntryManageDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	//�б�
	DDX_Control(pDX, IDC_TREE_LINES, m_LinesTree);

	//��ϸ��Ϣ
	DDX_Control(pDX, IDC_COMBO_CATEGORY, m_LineCategory);
	DDX_Control(pDX, IDC_COMBO_SHAPE, m_LineShape);

	DDX_Control(pDX, IDC_STATIC_RADIUS, m_StaticRadius);
	DDX_Control(pDX, IDC_EDIT_LINE_RADIUS, m_LineRadius);

	DDX_Control(pDX, IDC_STATIC_WIDTH, m_StaticWidth);
	DDX_Control(pDX, IDC_EDIT_WIDTH, m_LineWidth);

	DDX_Control(pDX, IDC_STATIC_HEIGHT, m_StaticHeight);
	DDX_Control(pDX, IDC_EDIT_HEIGHT, m_LineHeight);

	DDX_Control(pDX, IDC_EDIT_WALL_SIZE,m_LineWallSize);
	DDX_Control(pDX, IDC_EDIT_SAFESIZE,m_LineSafeSize);

	DDX_Control(pDX, IDC_EDIT_PLANE_MARK,m_LinePlaneDesc);
	DDX_Control(pDX, IDC_EDIT_CUT_MARK,m_LineCutDesc);

	//���߶�
	DDX_Control(pDX, IDC_EDIT_POINT, m_PointEdit);
	DDX_Control(pDX, IDC_LIST_LINE_DETAIL, m_LineDetailList);

	//��ť
	DDX_Control(pDX, IDC_BUTTON_ADD, m_ButtonAdd);
	DDX_Control(pDX, IDC_BUTTON_DEL, m_ButtonDel);

	DDX_Control(pDX, IDOK, m_ButtonOK);
}

LineEntry* EntryManageDialog::GetLineEntry( const UINT& ID )
{
	return m_EntryFile->FindLine(ID);
}

BEGIN_MESSAGE_MAP(EntryManageDialog, CDialog)

	//��ť��Ӧ
	ON_BN_CLICKED(IDOK, OnBnClickedButtonOK)

	ON_BN_CLICKED(IDC_BUTTON_ADD, OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_DEL, OnBnClickedButtonDel)
	
	//����ѡ��
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_LINES, OnTreeSelChanged)

	//��������ѡ��
	ON_CBN_SELCHANGE(IDC_COMBO_SHAPE,		OnCbnShapeChange)

	ON_CBN_SELCHANGE(IDC_COMBO_CATEGORY,	OnControlValueChange)
	ON_CBN_SELCHANGE(IDC_COMBO_SHAPE,		OnControlValueChange)

	ON_EN_CHANGE(IDC_EDIT_LINE_RADIUS,		OnControlValueChange)
	ON_EN_CHANGE(IDC_EDIT_WIDTH,			OnControlValueChange)
	ON_EN_CHANGE(IDC_EDIT_HEIGHT,			OnControlValueChange)
	
	ON_EN_CHANGE(IDC_EDIT_WALL_SIZE,		OnControlValueChange)
	ON_EN_CHANGE(IDC_EDIT_SAFESIZE,			OnControlValueChange)
	
	ON_EN_CHANGE(IDC_EDIT_PLANE_MARK,		OnControlValueChange)
	ON_EN_CHANGE(IDC_EDIT_CUT_MARK,			OnControlValueChange)

	ON_BN_DOUBLECLICKED(IDC_LIST_LINE_DETAIL ,		OnControlValueChange)

END_MESSAGE_MAP()

void EntryManageDialog::OnBnClickedButtonOK()
{
	//ȷ�ϰ�ť������
	m_ButtonOK.EnableWindow(false);

	//�õ�������ϸ��Ϣ
	LineCategoryItemData* detailInfo = CreateEntryDetailInfo();

	//�õ����߶�����
	PointList* pointList = CreateEntryPointList();

	if( m_OperType == OPER_ADD )
	{
		//�õ�ʵ������
		wstring pipeName = m_EntryFile->GetNewPipeName(detailInfo->mCategory);
		acutPrintf(L"\n�������ߡ�%s��,���߶Ρ�%d����.",pipeName.c_str(),pointList->size());

		//�����µĹ���
		LineEntry* newLine = new LineEntry(pipeName,GlobalData::KIND_LINE,
											detailInfo,NULL);

		//���ɸ����ID
		newLine->m_LineID = (UINT)GetTickCount();

		//�������߶�(�ڴ˽����ػ�)
		newLine->SetPoints( pointList );

		//�����µĹ��ߣ�ͬʱ���ӵ����ݿ⣩
		InsertLine(newLine);

		//���ò�������Ϊ����
		SetOperType( OPER_UPDATE );
	}
	else /*if( m_OperType == OPER_UPDATE )*/
	{
		//�õ���ǰ�༭��ֱ��
		LineEntry* selectLine = GetSelectLine();

		//�����µ�����
		if( selectLine )
		{
			acutPrintf(L"\n���¹��ߡ�%s��,���߶Ρ�%d����.",
				selectLine->m_LineName.c_str(),
				pointList->size());

			//���û�����Ϣ
			selectLine->SetBasicInfo( detailInfo );

			//�������߶���Ϣ
			selectLine->SetPoints( pointList );

			//���¹���
			UpdateLine(selectLine);
		}
	}

	//Ĭ�Ͻ���XY��ͼ
	acedCommand(RTSTR, _T("._-VIEW"), RTSTR, L"TOP", 0);

	//���浽��ʱ�ļ�
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
	//�жϵ�ǰ�����Ƿ���Ҫ����
	CheckUIData();

	//���пռ����
	EnableDetailControl(true);

	//������е�ҳ������
	ClearLineData();

	//���ò�������
	SetOperType(OPER_ADD);
}

void EntryManageDialog::OnBnClickedButtonDel()
{
	//ѡ�����
	HTREEITEM selectedItem = m_LinesTree.GetSelectedItem();

	if( selectedItem == NULL)
		 return;

	UINT selectedID = (UINT)m_LinesTree.GetItemData(selectedItem);

	//�õ�ѡ�������
	LineEntry* pEntry = m_EntryFile->FindLine(selectedID);

	if( pEntry )
	{
		// Initializes the variables to pass to the MessageBox::Show method.
		CString message;
		message.Format(L"ȷʵҪɾ��[%s]��?",pEntry->m_LineName.c_str());

		LPCTSTR caption = L"ɾ��ʵ��";

		// Displays the MessageBox.
		int result = MessageBoxW(message, caption, MB_OKCANCEL);
		if ( result == IDOK )
		{
			//�����ݿ�ɾ�����߱���
			ArxWrapper::PostToNameObjectsDict(pEntry->pDbEntry,pEntry->LINE_ENTRY_LAYER,true);

			//�����ݿ�ɾ���������е��߶�
			ArxWrapper::eraseLMALine(*pEntry);

			//ɾ�����е��ڴ�ڵ�
			pEntry->ClearPoints();

			//Delete from the list
			m_EntryFile->DeleteLine(selectedID);

			// delete the selected item. 
			m_LinesTree.DeleteItem(selectedItem);

			//���浽�����ļ�
			m_EntryFile->Persistent();
		}
	}
}

BOOL EntryManageDialog::InitEntryData()
{
	//ѡ�����
	HTREEITEM selectedItem = m_LinesTree.GetSelectedItem();

	UINT selectedID = (UINT)m_LinesTree.GetItemData(selectedItem);

	//�õ�ѡ�������
	LineEntry* pEntry = m_EntryFile->FindLine(selectedID);

	FillLineData(pEntry);

	return TRUE;
}

void EntryManageDialog::OnTreeSelChanged(LPNMHDR pnmhdr, LRESULT *pLResult)
{
	//�����ĳһ�ѡ��
	if( pnmhdr->code == TVN_SELCHANGED )
	{
		//ѡ�������ڵ�ʱ�����ж�
		CheckUIData();

		//���ѡ�е�����
		InitEntryData();

		//��ʾ��̬���
		ShowControlDynamic();

		//����ȷ�ϰ�ť������
		m_ButtonOK.EnableWindow(false);

		//���ؼ�����
		EnableDetailControl(true);
	}
}

void EntryManageDialog::FillLineData( LineEntry* lineEntry )
{
	if( lineEntry )
	{
		//������ϸ����
		m_LineCategory.SetWindowText(lineEntry->m_LineBasiInfo->mCategory.c_str());
		m_LineShape.SetWindowText(lineEntry->m_LineBasiInfo->mShape.c_str());

		m_LineRadius.SetWindowText(lineEntry->m_LineBasiInfo->mRadius.c_str());
		m_LineWidth.SetWindowText(lineEntry->m_LineBasiInfo->mWidth.c_str());
		m_LineHeight.SetWindowText(lineEntry->m_LineBasiInfo->mHeight.c_str());

		m_LineWallSize.SetWindowText(lineEntry->m_LineBasiInfo->mWallSize.c_str());
		m_LineSafeSize.SetWindowText(lineEntry->m_LineBasiInfo->mSafeSize.c_str());

		m_LinePlaneDesc.SetWindowText(lineEntry->m_LineBasiInfo->mPlaneMark.c_str());
		m_LineCutDesc.SetWindowText(lineEntry->m_LineBasiInfo->mCutMark.c_str());

		//����������Ϣ
		InitEntryPointsData(lineEntry);
	}

	UpdateData(FALSE);
}

LineCategoryItemData* EntryManageDialog::CreateEntryDetailInfo()
{
	UpdateData(TRUE);

	CString lineCategory,lineShape,lineRadius,lineWidth,lineHeight,
			lineSafeSize,lineWallSize,linePlaneDesc,lineCutDesc;

	//�õ�������������Ϣ
	m_LineCategory.GetWindowTextW(lineCategory);
	m_LineShape.GetWindowTextW(lineShape);

	m_LineRadius.GetWindowTextW(lineRadius);
	m_LineWidth.GetWindowTextW(lineWidth);
	m_LineHeight.GetWindowTextW(lineHeight);

	m_LineWallSize.GetWindowTextW(lineWallSize);
	m_LineSafeSize.GetWindowTextW(lineSafeSize);

	m_LinePlaneDesc.GetWindowTextW(linePlaneDesc);
	m_LineCutDesc.GetWindowTextW(lineCutDesc);

	//׼���������ݽṹ��
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

		//�õ���ǰ��ţ��������б��е����кţ�
		point->m_PointNO = (UINT)i;

		temp = m_LineDetailList.GetItemText(i,1);
		acdbDisToF(temp.GetBuffer(), -1, &((point->m_Point)[X]));

		temp = m_LineDetailList.GetItemText(i,2);
		acdbDisToF(temp.GetBuffer(), -1, &((point->m_Point)[Y]));

		temp = m_LineDetailList.GetItemText(i,3);
		acdbDisToF(temp.GetBuffer(), -1, &((point->m_Point)[Z]));

		//���뵽������
		newPoints->push_back(point);
	}

	return newPoints;
}

void EntryManageDialog::ClearLineData()
{
	//�����ϸ����
	m_LineCategory.SetCurSel(0);
	m_LineShape.SetCurSel(0);

	m_LineRadius.SetWindowText(L"");
	m_LineWidth.SetWindowText(L"");
	m_LineHeight.SetWindowText(L"");

	m_LineWallSize.SetWindowText(L"");
	m_LineSafeSize.SetWindowText(L"");

	m_LinePlaneDesc.SetWindowText(L"");
	m_LineCutDesc.SetWindowText(L"");

	//������ߵ�����
	m_LineDetailList.DeleteAllItems();

	//����ɾ��������
	m_ButtonDel.EnableWindow(false);
	m_ButtonOK.EnableWindow(false);

	UpdateData(FALSE);
}

void EntryManageDialog::EnableDetailControl(bool enable)
{
	m_LineCategory.EnableWindow(enable);
	m_LineShape.EnableWindow(enable);

	m_LineRadius.EnableWindow(enable);
	m_LineWidth.EnableWindow(enable);
	m_LineHeight.EnableWindow(enable);

	m_LineWallSize.EnableWindow(enable);
	m_LineSafeSize.EnableWindow(enable);

	m_LinePlaneDesc.EnableWindow(enable);
	m_LineCutDesc.EnableWindow(enable);

	m_LineDetailList.EnableWindow(enable);
}

void EntryManageDialog::OnCbnShapeChange()
{
	ShowControlDynamic();
}

void EntryManageDialog::OnControlValueChange()
{
	//ȷ�ϰ�ť����
	m_ButtonOK.EnableWindow(true);

	UpdateData(false);
}

void EntryManageDialog::ShowControlDynamic()
{
	if( m_LineShape.GetCurSel() == 0 )
	{
		m_StaticRadius.EnableWindow( true );
		m_LineRadius.EnableWindow( true );

		m_StaticWidth.EnableWindow( false );
		m_LineWidth.EnableWindow( false );

		m_StaticHeight.EnableWindow( false );
		m_LineHeight.EnableWindow( false );
	}
	else if( m_LineShape.GetCurSel() == 1 )
	{
		m_StaticRadius.EnableWindow( false );
		m_LineRadius.EnableWindow( false );

		m_StaticWidth.EnableWindow( true );
		m_LineWidth.EnableWindow( true );

		m_StaticHeight.EnableWindow( true );
		m_LineHeight.EnableWindow( true );
	}
}

void EntryManageDialog::CheckUIData()
{
	if( m_ButtonOK.IsWindowEnabled() )
	{
		int result = MessageBoxW(L"����", L"�����Ѹ��ģ��Ƿ񱣴�", MB_OKCANCEL);

		if( result == IDOK )
		{
			OnBnClickedButtonOK();
		}
	}
}

void EntryManageDialog::LinePointModified(void* dialog)
{
	acutPrintf(L"\n���߹������ص�����������");
	EntryManageDialog* entryDlg(NULL);

	if( entryDlg = static_cast<EntryManageDialog*>(dialog) )
	{
		acutPrintf(L"\nĬ��Ϊ��ֵ�����˱仯");
		//�ؼ�������ֵ�仯
		entryDlg->OnControlValueChange();
	}
}

// EntryManageDialog message handlers

} // end of entry

} // end of assistant

} // end of guch

} // end of com
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               