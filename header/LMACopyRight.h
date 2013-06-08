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

// LineCutPosDialog dialog

class LMACopyRight : public CAcUiDialog
{
	DECLARE_DYNAMIC(LMACopyRight)

public:

	// Dialog Data
	enum { IDD = IDD_COPYRIGHT };

	LMACopyRight(CWnd* pParent = NULL);   // standard constructor
	virtual ~LMACopyRight();

	virtual BOOL OnInitDialog();

protected:

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	afx_msg void OnBnClickedOk();

	DECLARE_MESSAGE_MAP()
};
