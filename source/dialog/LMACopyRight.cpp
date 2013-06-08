// LineCutPosDialog.cpp : implementation file
//

#include "stdafx.h"
#include "LMACopyRight.h"

IMPLEMENT_DYNAMIC(LMACopyRight, CAcUiDialog)

LMACopyRight::LMACopyRight(CWnd* pParent /*=NULL*/)
	: CAcUiDialog(LMACopyRight::IDD, pParent)
{
}

LMACopyRight::~LMACopyRight()
{
}

BOOL LMACopyRight::OnInitDialog()
{
	//和页面交互数据
	CAcUiDialog::OnInitDialog();

	return TRUE;
}

void LMACopyRight::DoDataExchange(CDataExchange* pDX)
{
	CAcUiDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(LMACopyRight, CAcUiDialog)
	ON_BN_CLICKED(IDOK, &LMACopyRight::OnBnClickedOk)
END_MESSAGE_MAP()

void LMACopyRight::OnBnClickedOk()
{
	//关闭对话框
	CAcUiDialog::OnOK();
}
