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

class LineCutPosDialog : public CAcUiDialog
{
	DECLARE_DYNAMIC(LineCutPosDialog)

public:

	// Dialog Data
	enum { IDD = IDD_DIALOG_CUT_POS };

	LineCutPosDialog(int dialogId, CWnd* pParent = NULL);   // standard constructor
	virtual ~LineCutPosDialog();

	virtual BOOL OnInitDialog();

	static afx_msg void Reset();

protected:

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

protected:

	afx_msg void OnBnClickedX();

	afx_msg void OnBnClickedY();

	afx_msg void OnBnClickedZ();

	virtual afx_msg void OnBnClickedOk();

	afx_msg void OnBnPickCutPos();

protected:

	//对当前文件中的所有管线进行切图
	void GenerateCutRegion();

	//对一根管线进行切图
	virtual void GenerateCutRegion(LineEntity* lineEntry);

	//对一个折线段进行切图
	virtual void GenerateCutRegion(PointEntity* pointEntity, double markOffset);

	//生成切面图所在的图层
	virtual void GenerateCutPlane();

	//生成转换矩阵
	void GenerateTransform();

	//显示切面图所在的图层，也既显示所有的切面
	void ShowCutRegion();

	//设置填充
	AcDbObjectId CreateHatch(AcDbObjectId entityId);

	//添加注释
	AcDbObjectId CreateMLeader(const AcGePoint3d& center, const wstring& content, double markOffset);

	//删除上次切图中产生的结果对象
	static void CutBack();

	//进行矩阵转换，将切面图放置XY平面
	void TransformToXY( AcDbObjectIdArray entityIds );

protected:

	static int m_DialogID;

	//选取X、Y、Z三个方向
	CButton m_DirectionX;
	CButton m_DirectionY;
	CButton m_DirectionZ;

	//选取的偏移量
	CAcUiNumericEdit m_EditOffset;

	//选取偏移量的按钮
	CAcUiPickButton m_PickCutPosButton;

	//切面的偏移量
	int m_strOffset;

	//切面的方向
	int m_Direction;

	//生成的切面
	AcGePlane m_CutPlane;

	//切面的名称，用于生成新的图层
	static CString m_CutLayerName;

	//切图过程中产生的新的数据库对象
	static AcDbObjectIdArray m_CutObjects;

	//切面的填充样式
	static wstring m_CutHatchStyle;

	//进行转换的矩阵
	AcGeMatrix3d m_MoveMatrix;

	AcGeMatrix3d m_RotateMatrixFirst;
	AcGeMatrix3d m_RotateMatrixSecond;
};
