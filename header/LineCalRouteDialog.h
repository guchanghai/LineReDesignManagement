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

	void CalculateShortestRoute();

	bool CalculateShortestRoute( const AcGePoint3d& start, const AcGePoint3d& end);

	bool SetupRouteLineEnv();

	bool CreateRouteSegment( const AcGePoint3d& start, const AcGePoint3d& end);

private:

	bool InitializeRouteLine();

	bool InitializeRouteLineInfo();

	bool InitializeStartEndPoints( const AcGePoint3d& startPoint, const AcGePoint3d& endPoint );

	bool AppendStartEndPoints(const AcGePoint3d& startPoint, const AcGePoint3d& endPoint);

	bool SaveRouteLinePoint( const AcGePoint3d& newPoint );

	bool HasIntersect(AcArray<PointEntity*>* intersectEntities);

	PointEntity* GetNearestLineSegement( AcArray<PointEntity*>* intersectEntities );

	AcGePoint3d GetProjectPoint3d(PointEntity* lineSegment);

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

	AcGePoint3d m_newStartPoint;

	//代表的文件名
	wstring m_fileName;

	//实体文件管理器
	LineEntityFile* m_EntryFile;

	//默认的管线种类
	static CString m_lineCategory;

	//计算的管线的名称，用于生成新的图层
	static CString m_CutLayerName;

	//默认的管线宽度
	static CString m_lineWidth;

	//默认的管线信息
	static LineCategoryItemData* m_lineInfo;

	//计算路由过程中产生的新的数据库对象
	static AcDbObjectIdArray* m_CutObjects;

	//管线实体，代表当前进行
	static LineEntity* m_RouteLineEntity;

	//最终结果,保存各个线段的坐标
	static AcGePoint3dArray* m_PointVertices;
};
