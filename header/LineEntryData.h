// ------------------------------------------------
//                  LineManagementAssistant
// Copyright 2012-2013, Chengyong Yang & Changhai Gu. 
//               All rights reserved.
// ------------------------------------------------
//	LineEntryData.h
//	written by Changhai Gu
// ------------------------------------------------
// $File:\\LineManageAssitant\main\header\LineEntryData.h $
// $Author: Changhai Gu $
// $DateTime: 2013/1/2 01:35:46 $
// $Revision: #1 $
// ------------------------------------------------
#pragma once

#include "stdafx.h"

#include <string>
#include <vector>
#include <map>
#include <set>

#include <dbsymtb.h>
#include <dbapserv.h>
#include <adslib.h>
#include <adui.h>
#include <acui.h>

#include <GlobalDataConfig.h>
#include <LineCategoryItemData.h>

using namespace std;
using namespace com::guch::assistant::config;
using namespace com::guch::assistant::data;

namespace com
{

namespace guch
{

namespace assistant
{

namespace data
{

struct LineAroundEntity
{
	bool bInitialized;

	AcGePlane mLeftPlane;
	AcGePlane mRightPlane;
	AcGePlane mTopPlane;
	AcGePlane mBottomPlane;
	AcGePlane mFrontPlane;
	AcGePlane mBackPlane;

	AcGeLineSeg3d mLineLeftFront;
	AcGeLineSeg3d mLineRightFront;
	AcGeLineSeg3d mLineLeftBack;
	AcGeLineSeg3d mLineRightBack;

	AcGeLineSeg3d mLineFrontTop;
	AcGeLineSeg3d mLineFrontBottom;
	AcGeLineSeg3d mLineBackTop;
	AcGeLineSeg3d mLineBackBottom;
	
	AcGeLineSeg3d mLineLeftTop;
	AcGeLineSeg3d mLineLeftBottom;
	AcGeLineSeg3d mLineRightTop;
	AcGeLineSeg3d mLineRightBottom;

	LineAroundEntity();
};

class PointDBEntityCollection
{
public:

	typedef enum { DB_LINE, DB_SAFELINE, DB_DIM, DB_MARK } DBEntityKind;

	PointDBEntityCollection();
	~PointDBEntityCollection(){}

	void SetLineEntity( const AcDbObjectId entityId ){ m_LineEntryId = entityId; }
	const AcDbObjectId& GetLineEntity() const { return m_LineEntryId; }

	void SetWallLineEntity( const AcDbObjectId entityId ){ m_WallLineEntryId = entityId; }
	const AcDbObjectId& GetWallLineEntity() const { return m_WallLineEntryId; }

	void SetSafeLineEntity( const AcDbObjectId entityId ){ m_SafeLineEntityId = entityId; }
	const AcDbObjectId& GetSafeLineEntity() const { return m_SafeLineEntityId; }

	void SetDimEntity( const AcDbObjectId entityId ){ m_DimEntityId = entityId; }
	const AcDbObjectId& GetDimEntity() const { return m_DimEntityId; }

	void SetMarkEntity( const AcDbObjectId entityId ){ m_MarkEntityId = entityId; }
	const AcDbObjectId& GetMarkEntity() const { return m_MarkEntityId; }

	bool HasEntity( const AcDbObjectId& entityId ) const;

	void SetLineWarning( bool warning = true );

	void TransformBy( const AcGeMatrix3d& matrix );

	//database object collection
	bool DrawEntityCollection(GlobalData::LineProirity proirity);
	void DropEntityCollection();

	//Get the plane at position
	AcGePlane& GetAroundPlane(int direction);

	//the layer to insert
	wstring mLayerName;

	//line basic info
	LineCategoryItemData* mCategoryData;

	//Store in database
	Adesk::Int32 mLineID;

	//Identify the index in the line
	Adesk::Int32 mSequenceNO;

	//the bottom
	AcGePoint3d mStartPoint;

	//the top
	AcGePoint3d mEndPoint;

	//the real line
	AcDbObjectId m_LineEntryId;

	//the wall line
	AcDbObjectId m_WallLineEntryId;

	//the line contains safe size
	AcDbObjectId m_SafeLineEntityId;

	//the dimision
	AcDbObjectId m_DimEntityId;

	//the text mark
	AcDbObjectId m_MarkEntityId;

	//the around panel
	LineAroundEntity m_LineAroundEntity;

private:

	//Calculate the around panel
	void CalculatePanel();
};

/**
 * 管线坐标实体
 */
struct PointEntity
{
	//点号
	UINT m_PointNO;
	ads_point m_Point; 

	wstring m_LevelKind;
	wstring m_Direction;

	PointEntity();
	PointEntity( const UINT& pointNO, const ads_point& point, 
		const wstring& levelKind, const wstring& direction, const AcDbObjectId& entityID);
	PointEntity( const PointEntity& );
	PointEntity( const wstring& data );

	void CreateLineFrom( const void* lineEntity, const ads_point& start );

	PointDBEntityCollection m_DbEntityCollection;

	wstring toString() const;
};

typedef PointEntity *pPointEntry;

typedef vector<pPointEntry> PointList;
typedef PointList::iterator PointIter;
typedef PointList::const_iterator ContstPointIter;

/**
 * 管线实体
 */
class LineDBEntity;

class LineEntity
{
public:

	static const wstring LINE_ENTRY_LAYER;
	static const wstring LINE_DATA_BEGIN;

	LineEntity();
	LineEntity(const wstring& rLineName, const wstring& rLineKind,
				LineCategoryItemData* itemdata, PointList* pointList);

	LineEntity(wstring& data );
	~LineEntity();

	UINT GetLineID() const { return m_LineID; }

	void SetName( const wstring& rNewName ) { m_LineName = rNewName; }
	const wstring& GetName() const { return m_LineName; }

	int InsertPoint( AcGePoint3d* newPoint, bool createDBEntity);
	int InsertPoint( PointEntity* newPoint);
	int InsertPoint( PointEntity* newPoint, bool createDBEntity);
	void UpdatePoint( const PointEntity& updatePoint );
	void DeletePoint( const UINT& PointNO );

	void SetBasicInfo( LineCategoryItemData* m_LineBasiInfo );
	LineCategoryItemData* GetBasicInfo() const { return m_LineBasiInfo; }

	void SetPoints( PointList* newPoints);

	PointIter FindPoint( const UINT& PointNO ) const;
	ContstPointIter FindConstPoint( const UINT& PointNO ) const;

	wstring toString();

	void ClearPoints();
	void ClearPoints(PointList* pPointList);

	//Create Database Line
	void CreateDbObjects();

	//Erase Database Line
	void EraseDbObjects(bool old = false);

	//delete first and draw again
	void Redraw();

protected:

	//Create the 3D database entities
	void DrawDBEntity();

public:

	//来标识唯一性
	UINT m_LineID;

	//显示的数据
	wstring m_LineName;
	wstring m_LineKind;

	//基本信息
	LineCategoryItemData* m_LineBasiInfo;

	//折线段信息
	UINT m_CurrentPointNO;

	PointList* m_PrePointList;
	PointList* m_PointList;

	//数据库代理对象(既限于新建对象时使用)
	LineDBEntity* m_pDbEntry;

	//保存其ID
	AcDbObjectId m_dbId;

	//管线显示级别
	GlobalData::LineProirity m_LinePriority;
};

/**
 * 管线数据库实体
 */
class LineDBEntity : public AcDbObject
{
public:

	ACRX_DECLARE_MEMBERS(LineDBEntity);

	LineDBEntity();
	LineDBEntity( LineEntity* implementation );

	LineEntity* pImplemention;

	virtual Acad::ErrorStatus dwgInFields (AcDbDwgFiler*);
    virtual Acad::ErrorStatus dwgOutFields(AcDbDwgFiler*)
        const;

    virtual Acad::ErrorStatus dxfInFields (AcDbDxfFiler*);
    virtual Acad::ErrorStatus dxfOutFields(AcDbDxfFiler*)
        const;
};

typedef vector<LineEntity*> LineList;
typedef LineList::iterator LineIterator;
typedef LineList::const_iterator ConstLineIterator;

typedef map<UINT,PointList*> LinePointMap;
typedef pair<Adesk::Int32, Adesk::Int32> LinePointID;

/**
 * 管线实体文件
 */
class LineEntityFile
{
public:
	LineEntityFile(const wstring& fileName, bool import = false);
	~LineEntityFile();

	void InsertLine( LineEntity* lineEntry);
	void InsertLine( LineList* lineList);

	BOOL UpdateLine( LineEntity* lineEntry);
	BOOL DeleteLine( const UINT& lineID );

	LineIterator FindLinePos( const UINT& lineID ) const;
	LineIterator FindLinePosByNO( const wstring& lineNO ) const;
	LineIterator FindLinePosByName( const wstring& lineName ) const;

	LineEntity* FindLine( const UINT& lineID ) const;
	LineEntity* FindLineByNO( const wstring& lineNO  ) const;
	LineEntity* FindLineByName( const wstring& lineName  ) const;

	LineEntity* HasAnotherLineByNO( const UINT& lineID, const wstring& lineNO  ) const;
	LineEntity* HasAnotherLineByByName( const UINT& lineID, const wstring& lineName  ) const;

	PointList* GetTempLine( const UINT& lineID );
	PointList* TransferTempLine( const UINT& lineID );

	wstring GetNewPipeName( const LineCategoryItemData* pipeCategoryData, const wstring& orinalName );

	void Import();
	void Persistent() const;
	void ExportTo(const wstring& filename,const wstring& lineKind) const;

	LineList* GetList() const {return m_LineList;}
	LineList GetList( const wstring& entityKind ) const;

	wstring m_FileName;

private:

	LineList* m_LineList;

	//临时实体管理器
	LinePointMap* m_LinePoint;
};

/**
 * 多文件管线实体管理对象
 */

typedef vector<LineEntityFile*> EntryFileList;
typedef EntryFileList::iterator EntryFileIter;

class LineEntityFileManager
{
public:

	static void ReadFromCurrentDWG();

	static void RemoveEntryFileOnDWGUnLoad();

	static BOOL ImportLMALineFile( const wstring& lineKind );

	static BOOL ExportLMALineFile( const wstring& lineKind );

	static LineEntityFile* GetCurrentLineEntryFile();

	static LineEntityFile* GetLineEntryFile( const wstring& fileName );

	static LineEntityFile* RegisterEntryFile(const wstring& fileName);

	static LineEntityFile* SaveFileEntity();

	static bool RegisterLineSegment( const wstring& fileName, UINT lineID, UINT sequence, 
		LineEntity*& pLineEntity, PointEntity*& pStart, PointEntity*& pEnd );

public:
	
	static bool openingDwg;

private:

	static EntryFileList* pEntryFileList;

};

} // end of data

} // end of assistant

} // end of guch

} // end of com
