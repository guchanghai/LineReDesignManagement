// ------------------------------------------------
//                  LineManagementAssistant
// Copyright 2012-2013, Chengyong Yang & Changhai Gu. 
//               All rights reserved.
// ------------------------------------------------
//	LineEntryData.cpp
//	written by Changhai Gu
// ------------------------------------------------
// $File:\\LineManageAssitant\main\source\data\LineEntryData.h $
// $Author: Changhai Gu $
// $DateTime: 2013/1/2 01:35:46 $
// $Revision: #1 $
// ------------------------------------------------

#include <LineEntryData.h>
#include <LMAUtils.h>

#include <ArxWrapper.h>
#include <acdocman.h>
#include <acutmem.h>

namespace com
{

namespace guch
{

namespace assistant
{

namespace data
{

#define POINT_START L"PS"
#define POINT_END L"PE"
#define POINTS_SEP L"&&"

///////////////////////////////////////////////////////////////////////////
// Implementation PointEntry

/**
 * ��������ʵ��
 */

PointEntry::PointEntry()
:m_PointNO(0),
m_LevelKind(L""),
m_Direction(L""),
m_pEntry(NULL)
{
	m_Point[X] = 0;
	m_Point[Y] = 0;
	m_Point[Z] = 0;
}

PointEntry::PointEntry( const UINT& pointNO, const ads_point& point, const wstring& levelKind, const wstring& direction)
:m_PointNO(pointNO),
m_LevelKind(levelKind),
m_Direction(direction),
m_pEntry(NULL)	
{
	m_Point[X] = point[X];
	m_Point[Y] = point[Y];
	m_Point[Z] = point[Z];
}

PointEntry::PointEntry( const PointEntry& pointEntry)
{
	this->m_PointNO = pointEntry.m_PointNO;
	this->m_LevelKind = pointEntry.m_LevelKind;
	this->m_Direction = pointEntry.m_Direction;

	this->m_Point[X] = pointEntry.m_Point[X];
	this->m_Point[Y] = pointEntry.m_Point[Y];
	this->m_Point[Z] = pointEntry.m_Point[Z];

	this->m_pEntry = pointEntry.m_pEntry;
}

PointEntry::PointEntry( const wstring& data )
{
	double temp;

	const static size_t start = wcslen(POINT_START);
	size_t end = data.find_first_of(POINT_END);

	int index = 0;

	wstrVector* dataColumn = vectorContructor(data,POINTS_SEP,start,end);
	wstring& column = (*dataColumn)[index++];

	acdbDisToF(column.c_str(), -1, &temp);
	this->m_PointNO = (UINT)temp;

	column = (*dataColumn)[index++];
	acdbDisToF(column.c_str(), -1, &m_Point[X]);

	column = (*dataColumn)[index++];
	acdbDisToF(column.c_str(), -1, &m_Point[Y]);

	column = (*dataColumn)[index++];
	acdbDisToF(column.c_str(), -1, &m_Point[Z]);

	m_LevelKind = (*dataColumn)[index++];

	m_Direction = (*dataColumn)[index++];

	delete dataColumn;
}

wstring PointEntry::toString() const
{
	CString temp;
	temp.Format(L"%s%d%s%0.2f%s%0.2f%s%0.2f%s%s%s%s%s%s",
				POINT_START,
				m_PointNO, POINTS_SEP,
				m_Point[X], POINTS_SEP,
				m_Point[Y], POINTS_SEP,
				m_Point[Z], POINTS_SEP,
				m_LevelKind,POINTS_SEP,
				m_Direction,POINTS_SEP,
				POINT_END);

	return temp.GetBuffer();
}

///////////////////////////////////////////////////////////////////////////
// Implementation LineEntry

const wstring LineEntry::LINE_ENTRY_LAYER = L"����ʵ���";

/**
 * ����ʵ��
 */

LineEntry::LineEntry()
	:m_LineID(0),
	m_LineName(L""),
	m_LineKind(L""),
	m_LineBasiInfo(new LineCategoryItemData()),
	m_CurrentPointNO(0),
	m_PrePointList(NULL),
	m_PointList(new PointList())
{}

LineEntry::LineEntry(const wstring& rLineName, const wstring& rLineKind,
						LineCategoryItemData* lineInfo, PointList* pointList)
	:m_LineID(0),
	m_LineName(rLineName),
	m_LineKind(rLineKind),
	m_LineBasiInfo(lineInfo),
	m_PointList(pointList),
	m_CurrentPointNO(0),
	m_PrePointList(NULL)
{
	//�������ݿ��������
	pDbEntry = new LineDBEntry( this );
}

LineEntry::LineEntry( const wstring& data)
{
	m_PointList = new PointList();

	double temp;
	int index = 0;

	wstrVector* dataColumn = vectorContructor(data,L"\t");

	//�õ��ߵ��������
	wstring& column = (*dataColumn)[index++];
	acdbDisToF(column.c_str(), -1, &temp);
	this->m_LineID = (UINT)temp;

	m_LineName = (*dataColumn)[index++];
	m_LineKind = (*dataColumn)[index++];

	//�õ���ϸ��Ϣ
	m_LineBasiInfo = new LineCategoryItemData();
	m_LineBasiInfo->mCategory = (*dataColumn)[index++];
	m_LineBasiInfo->mShape = (*dataColumn)[index++];
	m_LineBasiInfo->mRadius = (*dataColumn)[index++];
	m_LineBasiInfo->mWidth = (*dataColumn)[index++];
	m_LineBasiInfo->mHeight = (*dataColumn)[index++];
	m_LineBasiInfo->mWallSize = (*dataColumn)[index++];
	m_LineBasiInfo->mSafeSize = (*dataColumn)[index++];
	m_LineBasiInfo->mPlaneMark = (*dataColumn)[index++];
	m_LineBasiInfo->mCutMark = (*dataColumn)[index++];
	m_LineBasiInfo->mCanThrough = (*dataColumn)[index++];
	m_LineBasiInfo->mThroughDirection = (*dataColumn)[index++];

	//����ڵ����ǰ��
	column = (*dataColumn)[index++];
	acdbDisToF(column.c_str(), -1, &temp);
	m_CurrentPointNO = (UINT)temp;

	//�õ�ÿ���������
	int size = (int)dataColumn->size();

	while( index < size )
	{
		column = (*dataColumn)[index++];
		m_PointList->push_back(new PointEntry(column));
	}

	delete dataColumn;
}

LineEntry::~LineEntry()
{
	//ClearPoints();
}

void LineEntry::ClearPoints()
{
	ClearPoints(this->m_PrePointList);
	ClearPoints(this->m_PointList);
}

void LineEntry::ClearPoints( PointList* pPointList)
{
	if( pPointList )
	{
		for( PointIter iter = pPointList->begin();
				iter != pPointList->end();
				iter++ )
		{
			if(*iter)
				delete *iter;
		}

		delete pPointList;
		pPointList = NULL;
	}
}

PointIter LineEntry::FindPoint( const UINT& PointNO ) const
{
	for( PointIter iter = this->m_PointList->begin();
			iter != this->m_PointList->end();
			iter++)
	{
		if( (*iter)->m_PointNO == PointNO )
			return iter;
	}

	return m_PointList->end();
}

ContstPointIter LineEntry::FindConstPoint( const UINT& PointNO ) const
{
	for( ContstPointIter iter = this->m_PointList->begin();
			iter != this->m_PointList->end();
			iter++)
	{
		if( (*iter)->m_PointNO == PointNO )
			return iter;
	}

	return m_PointList->end();
}

int LineEntry::InsertPoint( const PointEntry& newPoint )
{
	pPointEntry point = new PointEntry(newPoint);

	point->m_PointNO = m_CurrentPointNO;

	m_PointList->push_back(point);

	m_CurrentPointNO++;

	return (int)m_PointList->size();
}

void LineEntry::UpdatePoint( const PointEntry& updatePoint )
{
	PointIter findPoint = this->FindPoint(updatePoint.m_PointNO);

	if( findPoint != this->m_PointList->end() )
	{
		delete *findPoint;
		*findPoint = new PointEntry(updatePoint);
	}
}

void LineEntry::DeletePoint( const UINT& PointNO )
{
	PointIter findPoint = this->FindPoint(PointNO);

	if( findPoint != this->m_PointList->end() )
	{
		m_PointList->erase(findPoint);
	}
}

void LineEntry::SetBasicInfo( LineCategoryItemData* lineBasiInfo )
{
	if( m_LineBasiInfo )
		delete m_LineBasiInfo;

	m_LineBasiInfo = lineBasiInfo;
}

void LineEntry::SetPoints( PointList* newPoints)
{
	//���浱ǰ�Ľڵ��б���������ɾ����ǰ�Ķ���
	m_PrePointList = m_PointList;

	//�µ��б������ڴ����µ��߶�
	m_PointList = newPoints;

	Redraw();
}

wstring LineEntry::toString()
{
	wstring lineData;

	CString temp;
	temp.Format(L"%d\t%s\t%s\t%s\t%d",m_LineID,m_LineName,m_LineKind,
					m_LineBasiInfo->toString().c_str(),
					m_CurrentPointNO);

#ifdef DEBUG
	//acutPrintf(L"\n����ʵ�����л�Ϊ��%s��",temp.GetBuffer());
#endif

	lineData = temp;

	if( this->m_PointList )
	{
		for( ContstPointIter iter = this->m_PointList->begin();
				iter != this->m_PointList->end();
				iter++)
		{
			lineData += L"\t";
			lineData += (*iter)->toString();
		}
	}

	return lineData;
}

void LineEntry::Redraw()
{
	/*
	//ɾ����ǰ���߶�(�����ݿ���)
	ArxWrapper::eraseLMALine(*this,true);

	//ɾ����ǰ���߶�(���ڴ���)
	ClearPoints(this->m_PrePointList);

	//�����µ��߶�
	ArxWrapper::createLMALine(*this);
	*/
}

///////////////////////////////////////////////////////////////////////////
// Implementation LineDBEntry

ACRX_DXF_DEFINE_MEMBERS(LineDBEntry, AcDbObject, AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent, 0, LineDBEntry, LMA);

LineDBEntry::LineDBEntry()
{
	pImplemention = new LineEntry();

	/*
	//�ڴ�DWG�ļ�ʱ���Զ����������
	LineEntryFile* curEntryFile = LineEntryFileManager::GetCurrentLineEntryFile();
	if( curEntryFile )
	{
		curEntryFile->InsertLine(pImplemention);
	}
	else
	{
		acutPrintf(L"\n�����ˡ����ļ�ʱ���Ҳ������߹�������");
	}
	*/
}

LineDBEntry::LineDBEntry( LineEntry* implementation )
{
	pImplemention = implementation;
}

// Files data in from a DWG file.
//
Acad::ErrorStatus
LineDBEntry::dwgInFields(AcDbDwgFiler* pFiler)
{
    assertWriteEnabled();

    AcDbObject::dwgInFields(pFiler);
    // For wblock filing we wrote out our owner as a hard
    // pointer ID so now we need to read it in to keep things
    // in sync.
    //
    if (pFiler->filerType() == AcDb::kWblockCloneFiler) {
        AcDbHardPointerId id;
        pFiler->readItem(&id);
    }

	if( !this->isErased() )
	{
		Adesk::UInt32 lineID;
		pFiler->readItem(&lineID);
		pImplemention->m_LineID = (UINT)lineID;

		TCHAR* tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!

		tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
		pFiler->readItem(&tmpStr);
		pImplemention->m_LineName = wstring(tmpStr);
		acutDelString(tmpStr);

		tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
		pFiler->readItem(&tmpStr);
		pImplemention->m_LineKind = wstring(tmpStr);
		acutDelString(tmpStr);

		{
			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mCategory = wstring(tmpStr);
			acutDelString(tmpStr);

			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mShape = wstring(tmpStr);
			acutDelString(tmpStr);

			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mRadius = wstring(tmpStr);
			acutDelString(tmpStr);
		
			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mWidth = wstring(tmpStr);
			acutDelString(tmpStr);

			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mHeight = wstring(tmpStr);
			acutDelString(tmpStr);

			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mWallSize = wstring(tmpStr);
			acutDelString(tmpStr);

			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mSafeSize = wstring(tmpStr);
			acutDelString(tmpStr);
		
			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mPlaneMark = wstring(tmpStr);
			acutDelString(tmpStr);

			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mCutMark = wstring(tmpStr);
			acutDelString(tmpStr);

			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mCanThrough = wstring(tmpStr);
			acutDelString(tmpStr);

			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mThroughDirection = wstring(tmpStr);
			acutDelString(tmpStr);
		}

		CString filename;
		dbToStr(this->database(),filename);

#ifdef DEBUG
		acutPrintf(L"\n���ļ���%s����������ʵ�� ID��%d�����ơ�%s�����͡�%s��.",
					filename.GetBuffer(),
					pImplemention->m_LineID,
					pImplemention->m_LineName,
					pImplemention->m_LineKind);
#endif

		wstring fileName(filename.GetBuffer());
		LineEntryFile* entryFile = LineEntryFileManager::RegisterEntryFile(fileName);

		if( pImplemention->m_PointList )
			delete pImplemention->m_PointList;

		this->pImplemention->m_PointList = entryFile->TransferTempLine(pImplemention->m_LineID);

#ifdef DEBUG
		acutPrintf(L"\n����ʱ���߹������еõ��߶����ݣ�����Ϊ��%d��", ( pImplemention->m_PointList ? pImplemention->m_PointList->size() : 0 ) );
#endif

		//������ļ��м������line
		entryFile->InsertLine(pImplemention);
	}

    return pFiler->filerStatus();
}

// Files data out to a DWG file.
//
Acad::ErrorStatus
LineDBEntry::dwgOutFields(AcDbDwgFiler* pFiler) const
{
    assertReadEnabled();

    AcDbObject::dwgOutFields(pFiler);
    // Since objects of this class will be in the Named
    // Objects Dictionary tree and may be hard referenced
    // by some other object, to support wblock we need to
    // file out our owner as a hard pointer ID so that it
    // will be added to the list of objects to be wblocked.
    //

    if (pFiler->filerType() == AcDb::kWblockCloneFiler)
        pFiler->writeHardPointerId((AcDbHardPointerId)ownerId());

#ifdef DEBUG
	acutPrintf(L"�������ʵ�嵽���ݿ� ID��%d�����ơ�%s�����͡�%s��\n",
				pImplemention->m_LineID,
				pImplemention->m_LineName.c_str(),
				pImplemention->m_LineKind.c_str());
#endif

    pFiler->writeItem(Adesk::UInt32(pImplemention->m_LineID));
	pFiler->writeItem(pImplemention->m_LineName.c_str());
	pFiler->writeItem(pImplemention->m_LineKind.c_str());

	pFiler->writeItem(pImplemention->m_LineBasiInfo->mCategory.c_str());
	pFiler->writeItem(pImplemention->m_LineBasiInfo->mShape.c_str());

	pFiler->writeItem(pImplemention->m_LineBasiInfo->mRadius.c_str());
	pFiler->writeItem(pImplemention->m_LineBasiInfo->mWidth.c_str());
	pFiler->writeItem(pImplemention->m_LineBasiInfo->mHeight.c_str());

	pFiler->writeItem(pImplemention->m_LineBasiInfo->mWallSize.c_str());
	pFiler->writeItem(pImplemention->m_LineBasiInfo->mSafeSize.c_str());

	pFiler->writeItem(pImplemention->m_LineBasiInfo->mPlaneMark.c_str());
	pFiler->writeItem(pImplemention->m_LineBasiInfo->mCutMark.c_str());

	pFiler->writeItem(pImplemention->m_LineBasiInfo->mCanThrough.c_str());
	pFiler->writeItem(pImplemention->m_LineBasiInfo->mThroughDirection.c_str());

    return pFiler->filerStatus();
}

// Files data in from a DXF file.
//
Acad::ErrorStatus
LineDBEntry::dxfInFields(AcDbDxfFiler* pFiler)
{
    assertWriteEnabled();

    Acad::ErrorStatus es;
    if ((es = AcDbObject::dxfInFields(pFiler))
        != Acad::eOk)
    {
        return es;
    }

    // Check if we're at the right subclass getLineID marker.
    //
    if (!pFiler->atSubclassData(_T("LineEntryData"))) {
        return Acad::eBadDxfSequence;
    }

    struct resbuf inbuf;
    while (es == Acad::eOk) {
        if ((es = pFiler->readItem(&inbuf)) == Acad::eOk) {

			/*
			switch ( inbuf.restype )
			{
				//case AcDb::kDxfInt16:
				//	m_LineID = inbuf.resval.rint;
				//case AcDb::kDxfInt16 + 1:
					//mSequenceNO = inbuf.resval.rint;
			}
			*/
        }
    }

    return pFiler->filerStatus();
}

// Files data out to a DXF file.
//
Acad::ErrorStatus
LineDBEntry::dxfOutFields(AcDbDxfFiler* pFiler) const
{
    assertReadEnabled();

    AcDbObject::dxfOutFields(pFiler);
    pFiler->writeItem(AcDb::kDxfSubclass, _T("LineEntryData"));
    //pFiler->writeItem(AcDb::kDxfInt16, mLineID);
	//pFiler->writeItem(AcDb::kDxfInt16 + 1, mSequenceNO);

    return pFiler->filerStatus();
}


///////////////////////////////////////////////////////////////////////////
// Implementation LineEntryFile

/**
 * ����ʵ���ļ�
 */
LineEntryFile::LineEntryFile(const wstring& fileName, bool import)
	:m_FileName(fileName)
{
	m_LineList = new LineList();
	m_LinePoint = new LinePointMap();

	if( import )
		Import();
}

LineEntryFile::~LineEntryFile()
{
	if( m_LineList )
	{
		for( LineIterator iter = m_LineList->begin();
				iter != m_LineList->end();
				iter++ )
		{
			//if( (*iter) && (*iter)->pDbEntry )
			//	(*iter)->pDbEntry->close();
		}

		delete m_LineList;
	}

	if( m_LinePoint )
		delete m_LinePoint;
}

void LineEntryFile::Import()
{
	CFile archiveFile;

	//read data from file LMA_CONFIG_FILE
	BOOL result = archiveFile.Open(this->m_FileName.c_str(),CFile::modeRead);
	if( !result )
	{
		acutPrintf(L"\n�򿪹���ʵ�������ļ�ʧ��!");
		return;
	}

	//�õ��ļ����ݳ���
	int length = (int)archiveFile.GetLength()+1;

	//�õ��ļ���խ�ַ�����
	char* content = new char[length];
	memset(content,0,length);
	archiveFile.Read(content,length);

	//����ת��Ϊ���ַ�
	string strCnt(content,length);
	wstring wContent = StringToWString( strCnt );

	//���һس��Ծ�����
	size_t lineFrom = 0;
	size_t linePos = wContent.find_first_of(L"\n",lineFrom);

	while( linePos != wstring::npos )
	{
		//�õ�һ������
		wstring& wLine = wContent.substr(lineFrom, linePos-lineFrom);

#ifdef DEBUG
		acutPrintf(L"\n�õ�һ�й���ʵ�����ݡ�%s��.",wLine.c_str());
#endif

		if(wLine.length() == 0)
			break;

		LineEntry *newLine = new LineEntry(wLine);
		m_LineList->push_back( newLine );

		//������Ӧ��ͼ��
		ArxWrapper::createNewLayer( newLine->m_LineName );

		//������Ӧ������
		ArxWrapper::createLMALine(*newLine );

		//����һ���ַ���ʼ��������һ��
		lineFrom = linePos + 1;
		linePos = wContent.find_first_of(L"\n",lineFrom + 1);
	}

	//�ر��ļ�
	archiveFile.Close();
}

void LineEntryFile::Persistent() const
{
	acutPrintf(L"\n��ʼ��������ʵ������.");
	ExportTo(this->m_FileName);
}

void LineEntryFile::ExportTo(const wstring& filename) const
{
	acutPrintf(L"\n��ʼ��������ʵ������.");

	CString exportFile;
	exportFile.Format(L"%s",filename.c_str());
	CFile archiveFile(exportFile,CFile::modeCreate|CFile::modeWrite);

	//�������е����Ͷ���
	for( ConstLineIterator iter = m_LineList->begin(); 
			iter != m_LineList->end(); 
			iter++)
	{
		LineEntry* data = *iter;

		if( data )
		{
			//�õ���Ϣ�Ŀ��ַ����л�
			wstring wData = data->toString();

			//תΪխ�ַ�
			string dataStr = WstringToString(wData);

#ifdef DEBUG
			acutPrintf(L"\n����ʵ�����ݡ�%s��.",wData.c_str());
#endif
			//ʹ�� virtual void Write( const void* lpBuf, UINT nCount ); ��խ�ַ�д���ļ�
			archiveFile.Write(dataStr.c_str(),(UINT)dataStr.size());
			archiveFile.Write("\n",(UINT)strlen("\n"));
		}
	}

	acutPrintf(L"\n����ʵ�����ݵ������.");
	archiveFile.Close();
}

void LineEntryFile::InsertLine(LineEntry* lineEntry)
{
	if( lineEntry )
		m_LineList->push_back(lineEntry);
}

BOOL LineEntryFile::UpdateLine(LineEntry* lineEntry)
{
	LineIterator iter = FindLinePos(lineEntry->m_LineID);

	if( iter != this->m_LineList->end())
	{
		(*iter)->m_LineName = lineEntry->m_LineName;
		//(*iter)->m_LineNO = lineEntry->m_LineNO;

		return TRUE;
	}

	return FALSE;
}

BOOL LineEntryFile::DeleteLine( const UINT& lineID )
{
	LineIterator iter = FindLinePos(lineID);

	if( iter != this->m_LineList->end())
	{
		m_LineList->erase(iter);
		return TRUE;
	}
	else
		return FALSE;
}

LineIterator LineEntryFile::FindLinePos( const UINT& lineID ) const
{
	for( LineIterator iter = this->m_LineList->begin();
			iter != this->m_LineList->end();
			iter++)
	{
		if( (*iter)->m_LineID == lineID )
			return iter;
	}

	return m_LineList->end();
}

LineIterator LineEntryFile::FindLinePosByNO( const wstring& lineNO ) const
{
	for( LineIterator iter = this->m_LineList->begin();
			iter != this->m_LineList->end();
			iter++)
	{
		//if( (*iter)->m_LineNO == lineNO )
		//	return iter;
	}

	return m_LineList->end();
}

LineIterator LineEntryFile::FindLinePosByName( const wstring& lineName ) const
{
	for( LineIterator iter = this->m_LineList->begin();
			iter != this->m_LineList->end();
			iter++)
	{
		if( (*iter)->m_LineName == lineName )
			return iter;
	}

	return m_LineList->end();
}

LineEntry* LineEntryFile::FindLine( const UINT& lineID ) const
{
	LineIterator iter = FindLinePos(lineID);

	if( iter != m_LineList->end())
		return (*iter);
	else
		return NULL;
}

LineEntry* LineEntryFile::FindLineByName( const wstring& lineName ) const
{
	LineIterator iter = FindLinePosByName(lineName);

	if( iter != m_LineList->end())
		return (*iter);
	else
		return NULL;
}

LineEntry* LineEntryFile::FindLineByNO( const wstring& lineNO ) const
{
	LineIterator iter = FindLinePosByNO(lineNO);

	if( iter != m_LineList->end())
		return (*iter);
	else
		return NULL;
}

LineEntry* LineEntryFile::HasAnotherLineByNO( const UINT& lineID, const wstring& lineNO  ) const
{
	for( LineIterator iter = this->m_LineList->begin();
			iter != this->m_LineList->end();
			iter++)
	{
		//if( (*iter)->m_LineNO == lineNO && (*iter)->m_LineID != lineID)
		//	return (*iter);
	}

	return NULL;
}

LineEntry* LineEntryFile::HasAnotherLineByByName( const UINT& lineID, const wstring& lineName  ) const
{
	for( LineIterator iter = this->m_LineList->begin();
			iter != this->m_LineList->end();
			iter++)
	{
		if( (*iter)->m_LineName == lineName && (*iter)->m_LineID != lineID)
			return (*iter);
	}

	return NULL;
}

PointList* LineEntryFile::GetTempLine( const UINT& lineID )
{
	LinePointMap::iterator iter = m_LinePoint->find(lineID);

	if( iter == m_LinePoint->end() )
	{
		PointList* newList = new PointList();
		m_LinePoint->insert( std::pair<UINT,PointList*>(lineID,newList));

		return newList;
	}
	else
	{
		return iter->second;
	}
}

PointList* LineEntryFile::TransferTempLine( const UINT& lineID )
{
	LinePointMap::iterator iter = m_LinePoint->find(lineID);

	if( iter == m_LinePoint->end() )
	{
		return NULL;
	}
	else
	{
		PointList* findList = iter->second;
		m_LinePoint->erase(iter);

		return findList;
	}
}

wstring LineEntryFile::GetNewPipeName( const wstring& pipeCategory )
{
	//TODO Find name exist, plus sequence
	int index = 1;

	while(true)
	{
		CString pipeName;
		pipeName.Format(L"%s_%d",pipeCategory.c_str(),index);

		if( this->FindLineByName(pipeName.GetBuffer()) == NULL )
		{
			return wstring(pipeName.GetBuffer());
		}
		else
		{
			index++;
		}
	}

	return pipeCategory;
}

/////////////////////////////////////////////////////////////////////////

EntryFileList* LineEntryFileManager::pEntryFileList = NULL;

void LineEntryFileManager::ReadFromCurrentDWG()
{
#ifdef DEBUG
	acutPrintf(L"�ӵ�ǰDWG�ļ���ȡ���ݡ�");
#endif

	ArxWrapper::PullFromNameObjectsDict();
}

void LineEntryFileManager::RemoveEntryFileOnDWGUnLoad()
{
#ifdef DEBUG
		acutPrintf(L"DWG�ļ��ر��ˣ�ɾ���������ݡ�");

		if( pEntryFileList )
		{
			for( EntryFileIter iter = pEntryFileList->begin();
					iter != pEntryFileList->end();
					iter++)
			{
				delete (*iter);
			}

			pEntryFileList->clear();
		}
#endif
}

LineEntryFile* LineEntryFileManager::GetLineEntryFile( const wstring& fileName )
{
	if( pEntryFileList == NULL )
	{
		pEntryFileList = new EntryFileList();
#ifdef DEBUG
		acutPrintf(L"\n�ļ�ʵ���������δ����.");
#endif
		return NULL;
	}

	for( EntryFileIter iter = pEntryFileList->begin();
			iter != pEntryFileList->end();
			iter++)
	{
		if( (*iter)->m_FileName == fileName )
			return (*iter);
	}

#ifdef DEBUG
	acutPrintf(L"\nû���ҵ��ļ���%s����Ӧ�Ĺ���ʵ��.",fileName.c_str());
#endif

	return NULL;
}

LineEntryFile* LineEntryFileManager::RegisterEntryFile(const wstring& fileName)
{
	LineEntryFile* entryFile = GetLineEntryFile(fileName);
	if( entryFile == NULL )
	{
		acutPrintf(L"\n������%s����Ӧ�Ĺ���ʵ��.",fileName.c_str());

		entryFile = new LineEntryFile(fileName);
		pEntryFileList->push_back( entryFile );
	}

	return entryFile;
}

bool LineEntryFileManager::RegisterLineSegment( const wstring& fileName, AcDbEntity* pEntry, UINT lineID, UINT sequence, 
										const AcGePoint3d& start, const AcGePoint3d& end )
{
	//�ҵ��ļ�������
	LineEntryFile* pFileEntry = RegisterEntryFile(fileName);
	acutPrintf(L"\n�����߶�ʱ���ҵ�����ʵ���ļ���������%s��.",fileName.c_str());

	//�ҵ�ʵ����
	LineEntry* lineEntry = pFileEntry->FindLine(lineID);
	PointList* pPointList = NULL;

	if( lineEntry == NULL )
	{
#ifdef DEBUG
		acutPrintf(L"\n���浽��ʱ���߹�������.");
#endif
		pPointList = pFileEntry->GetTempLine( lineID );
	}

	if( sequence == 1 )
	{
#ifdef DEBUG
		acutPrintf(L"\n���к�Ϊ1�����ǵ�һ���߶�.");
#endif
		ads_point startPoint;
		startPoint[X] = start.x;
		startPoint[Y] = start.y;
		startPoint[Z] = start.z;

		PointEntry* tempPoint = new PointEntry(0,startPoint,L"",L"");
		tempPoint->m_pEntry = pEntry;

		if( lineEntry )
		{
			lineEntry->InsertPoint( *tempPoint );
			delete tempPoint;
		}
		else
		{
			pPointList->push_back( tempPoint );
		}

		ads_point endPoint;
		endPoint[X] = end.x;
		endPoint[Y] = end.y;
		endPoint[Z] = end.z;

		tempPoint = new PointEntry(0,endPoint,L"",L"");
		tempPoint->m_pEntry = pEntry;

		if( lineEntry )
		{
			lineEntry->InsertPoint( *tempPoint );
			delete tempPoint;
		}
		else
		{
			pPointList->push_back( tempPoint );
		}
	}
	else if (  sequence > 1 )
	{
#ifdef DEBUG
		acutPrintf(L"\n��ͨ�߶�.");
#endif
		ads_point endPoint;
		endPoint[X] = end.x;
		endPoint[Y] = end.y;
		endPoint[Z] = end.z;

		PointEntry* tempPoint = new PointEntry(0,endPoint,L"",L"");
		tempPoint->m_pEntry = pEntry;

		if( lineEntry )
		{
			lineEntry->InsertPoint( *tempPoint );
			delete tempPoint;
		}
		else
		{
			pPointList->push_back( tempPoint );
		}
	}
	else if ( sequence == 0)
	{
		acutPrintf(L"\n��Ч���߶�.");
	}

	return true;
}

LineEntryFile* LineEntryFileManager::GetCurrentLineEntryFile()
{
	//Get current filename
	wstring fileName = curDoc()->fileName();

	acutPrintf(L"\n���ҡ�%s����Ӧ�ĵĹ���.",fileName.c_str());

	return GetLineEntryFile(fileName);
}

BOOL LineEntryFileManager::ImportLMALineFile()
{
	//����ѡ��Ի���
	static wchar_t BASED_CODE szFilter[] = L"���������ļ� (*.dat)|*.dat||";
	CFileDialog dlg(TRUE, L"dat", L"", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, CWnd::FromHandle(adsw_acadMainWnd()), 0/*, TRUE*/);

	if (dlg.DoModal() == IDOK) 
	{
		//��仰ȷ����pEntryFileList�ǿ�
		GetCurrentLineEntryFile();

		//�õ������ļ�
        CString impFile = dlg.GetPathName();

		acutPrintf(L"\n��������ļ���%s��.",impFile.GetBuffer());
		LineEntryFile* importFile = new LineEntryFile(impFile.GetBuffer(),true);

		acutPrintf(L"\n����Ϊ��ǰ�ļ�����%s��.",curDoc()->fileName());
		importFile->m_FileName = wstring(curDoc()->fileName());

		//���뵽���߹�������
		pEntryFileList->push_back(importFile);

        return(TRUE);
    }
    else
        return(FALSE);
}

BOOL LineEntryFileManager::ExportLMALineFile()
{
	//����ѡ��Ի���
	
	//ȥ�����ļ���׺��dwg��
	wstring fileName( curDoc()->fileName() );
	fileName = fileName.substr(0, fileName.find_first_of(L"."));

	static wchar_t BASED_CODE szFilter[] = L"���������ļ� (*.dat)|*.dat||";
	CFileDialog dlg(FALSE, L"dat", fileName.c_str(), 
					OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, 
					CWnd::FromHandle(adsw_acadMainWnd()), 0/*, TRUE*/);

	if (dlg.DoModal() == IDOK) 
	{
        CString expFile = dlg.GetPathName();
		acutPrintf(L"\n�����������ݵ��ļ���%s��.",expFile.GetBuffer());

		LineEntryFile* exportFile = GetLineEntryFile(wstring(curDoc()->fileName()));

		if( exportFile )
			exportFile->ExportTo(expFile.GetBuffer());

        return(TRUE);
    }
    else
        return(FALSE);
}

} // end of data

} // end of assistant

} // end of guch

} // end of com