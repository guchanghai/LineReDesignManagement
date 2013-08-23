#pragma once

#include <resource.h>
#include <dbsymtb.h>
#include <dbapserv.h>
#include <adslib.h>
#include <adui.h>
#include <acui.h>
#include "afxwin.h"

#include <map>
#include <list>

#include <LineEntryData.h>

using namespace std;
using namespace com::guch::assistant::data;

// LineCalRouteDialog dialog

class LineCalRouteDialog : public CAcUiDialog
{
	DECLARE_DYNAMIC(LineCalRouteDialog)

public:

	// Dialog Data
	enum { IDD = IDD_DIALOG_CAL_ROUTE };

	// Pass through direction 000000 -> 111111
	enum PASS_STATUS
		{	PASS_NONE = 0x0,
			PASS_UP = 0x1, 
			PASS_DOWN = ( 0x1 << 1 ),
			PASS_LEFT = (0x1 << 2), 
			PASS_RIGHT = (0x1 << 3), 
			PASS_FRONT = (0x1 << 4), 
			PASS_BACK = (0x1 << 5),
			PASS_ALL = 0X3F };

	// Status for one possible route line
	typedef enum _CALCULATE_STATUS
	{
		INIT = 0,
		DONE = 1
	} CAL_STATUS;

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

	void GetStartEndPoint();

	void CalculateShortestRoute();

	bool CalculateShortestRoute( const AcGePoint3d& start, const AcGePoint3d& end);

	bool SetupRouteLineEnv();

	bool CreateRouteSegment( const AcGePoint3d& start, const AcGePoint3d& end);

	static PASS_STATUS GetPassDirecion( PointEntity *lineSegment);

	double GetShortestRoute();

	void DrawFinalResult();

private:

	bool InitializeRouteLine();

	bool InitializeRouteLineInfo();

	bool InitializePossibleLines();

	bool InitializeCompareSegmentEntity( const AcGePoint3d& startPoint, const AcGePoint3d& endPoint );

	void InitializeProjectPlace();

	bool CreateCompareLineSegement(const AcGePoint3d& startPoint, const AcGePoint3d& endPoint);

	bool SaveRouteLinePoint( const AcGePoint3d& newPoint );

	void CheckIntersect(AcArray<PointEntity*>* intersectEntities);

	PointEntity* GetNearestLineSegement( AcArray<PointEntity*>* intersectEntities );

	//计算中心线与自动路由切面的交点而得到上下切点，然后直接斜线相连
	AcGePoint3d GetProjectPoint3d(PointEntity* lineSegment);

	//计算中心线与自动路由切面的交点而得到上下切点，然后先直线相连，然后转动偏离
	AcGePoint3d GetIntersectPoint3d(PointEntity* lineSegment);

	//计算自动路由线路与前置的交点，然后先直线相连，然后转动偏离
	AcGePoint3d GetIntersectPoint3d(PointEntity* lineSegment, const AcGePoint3d& throughStart, const AcGePoint3d& throughEnd);

	//计算与12条棱的相切情况，然后先直线相连，然后转动偏离
	AcGePoint3d GetIntersectArountLinePoint3d(PointEntity* lineSegment, const AcGePoint3d& throughStart, const AcGePoint3d& throughEnd);

	void SetupLineRouteResult();

	void SetupFinalResult();

	void GetHeightAndStep(PointEntity* lineSegment, double& height, double& step);

	bool GetPossibleStartPoint(AcGePoint3d& startPoint);

	bool SetCurrentPossibleLineDone();

	LineEntity* CreateNewLineEntity();

	void AppendInterSegment(const AcGePoint3d& newPoint);

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
	static LineEntityFile* m_EntryFile;

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

	//所有可能的管线路由
	static list<LineEntity*> m_AllPossibleLineEntities;

	//管线实体，代表当前进行的管线
	LineEntity* m_CurrentRouteLineEntity;

	//管线实体，代表当前管线的起始点与最终点的之间的连接管线
	LineEntity* m_CompareLineSegmentEntity;

	//所有可能的路线,每条路线都保存各个线段的坐标
	static map<AcGePoint3dArray*, CAL_STATUS> m_lPossibleRoutes;

	//所有可能的路线,每条路线都保存各个线段的坐标
	static map<LineEntity*, CAL_STATUS> m_lPossibleLineEntities;
	
	//当前进行计算的路线
	AcGePoint3dArray* m_CurrentPointVertices;

	//最短的路线
	AcGePoint3dArray* m_ShortestPointVertices;

	//起始、终止点与X轴垂直的平面
	AcGePlane m_ProjectPlane;

	//越过的线段不在重复计算
	set<LinePointID> m_CheckedEntities;

	//Draw DB entity in realtime
	bool m_DrawRealTime;
};
