#pragma once

#include <LineEntryData.h>

using namespace std;
using namespace ::com::guch::assistant::data;

// LineCutPosDialog dialog

namespace com
{

namespace guch
{

namespace assistant
{

namespace Intersect
{

class LineIntersectManage
{

public:

	static LineIntersectManage* Instance();

	//判断相侵状态
	void CheckInteract();
	
	//删除上次检查侵限结果，返回3D模型状态
	void Reset();
	
	typedef struct
	{
		PointEntity* intersetcA;
		PointEntity* intersetcB;
		AcDbObjectId intersctcId;
	} IntersectStruct;

protected:

	static LineIntersectManage* mLineIntersectInstance;

	//判断本文件里的管线相侵情况
	void CheckLineInteract();

	//判断一条折线段与其他管线的相侵情况
	void CheckLineInteract( PointEntity* point );

private:

	LineIntersectManage();

	virtual ~LineIntersectManage();

	void Clear();

private:

	AcArray<IntersectStruct*> mIntersectEntities;
	
	//已相侵比较的折线段
	set<LinePointID> m_CheckedEntities;

	LineEntityFile* m_pCheckLine;
};

} // end of Intersect

} // end of assistant

} // end of guch

} // end of com