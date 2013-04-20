// ------------------------------------------------
//                  LineManagementAssistant
// Copyright 2012-2013, Chengyong Yang & Changhai Gu. 
//               All rights reserved.
// ------------------------------------------------
//	ArxWrapper.h
//	written by Changhai Gu
// ------------------------------------------------
// $File:\\LineManageAssitant\main\source\ARX\ArxWrapper.h $
// $Author: Changhai Gu $
// $DateTime: 2013/1/2 01:35:46 $
// $Revision: #1 $
// ------------------------------------------------

#include "afxcmn.h"

#include <string.h>
#include <aced.h>
#include <dbents.h>
#include <dbsymtb.h>
#include <dbgroup.h>
#include <dbapserv.h>
#include "tchar.h"

#include <string>
#include <vector>

#include <LineEntryData.h>

using namespace com::guch::assistant::data;
using namespace std;

typedef vector<AcGePoint3d*> Point3dVector;
typedef Point3dVector::const_iterator Point3dIter;

class ArxWrapper
{
public:

	static const double kPi;
	static const double kHalfPi;
	static const double kTwoPi;
	static const double kRad45;
	static const double kRad90;
	static const double kRad135;
	static const double kRad180;
	static const double kRad270;
	static const double kRad360;

	static bool createNewLayer(const wstring& layerName);

	static bool ShowLayer(const wstring& theOnly);

	static bool DeleteLayer(const wstring& layerName, bool deleteChildren = true);

	//Add entity to dictionary
	static AcDbObjectId PostToModelSpace(AcDbEntity* pEnt,const wstring& layerName );

	//Remove entry from model
	static Acad::ErrorStatus RemoveFromModelSpace(AcDbEntity* pEnt,const wstring& layerName );

	//By handle to remove
	static Acad::ErrorStatus RemoveFromModelSpace(const AcDbHandle& handle,const wstring& layerName );

	//Remove whole layer
	static Acad::ErrorStatus RemoveFromModelSpace(const wstring& layerName );

	//Remove the object from Database
	static Acad::ErrorStatus RemoveDbObject(AcDbObjectId id);

	//Show/Hide the database object
	static Adesk::Boolean ShowDbObject( AcDbObjectId& objectId, AcDb::Visibility show = AcDb::kVisible );

	//Add object to name dictionary
	static AcDbObjectId PostToNameObjectsDict( AcDbObject* pNameObj,const wstring& key );

	//Read object from name dictionary
	static void PullFromNameObjectsDict();

	//Remove entity from dictionary
	static bool DeleteFromNameObjectsDict( AcDbObjectId objToRemoveId,const wstring& key );

	//move offset
	static AcDbEntity* MoveToBottom(AcDbEntity* pEntry);

	//Change view
	static void ChangeView(int viewDirection);

	//锁住当前文档
	static Acad::ErrorStatus LockCurDoc();
	
	//解锁当前文档
	static Acad::ErrorStatus UnLockCurDoc();

	//得到两个实体是否有交集
	static AcDb3dSolid* GetInterset( AcDbEntity* pEntityA, AcDbEntity* pEntityB );

	//得到实体
	static AcDbEntity* GetDbObject( const AcDbObjectId& objId, bool openWrite = false );

	//测试函数
	static void TestFunction();
};
