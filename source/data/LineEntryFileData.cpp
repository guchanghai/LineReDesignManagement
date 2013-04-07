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
#include <GlobalDataConfig.h>

#include <ArxWrapper.h>
#include <acdocman.h>
#include <acutmem.h>

#include <LineManageAssitant.h>

using namespace ::com::guch::assistant::config;
extern wstring gLmaArxLoadPath;

namespace com
{

namespace guch
{

namespace assistant
{

namespace data
{

///////////////////////////////////////////////////////////////////////////
// Implementation LineEntryFile

/**
 * 管线实体文件
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
			//TODO no need to do for the database??
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
		acutPrintf(L"\n打开管线实体数据文件失败!");
		return;
	}

	//得到文件内容长度
	int length = (int)archiveFile.GetLength()+1;

	//得到文件的窄字符内容
	char* content = new char[length];
	memset(content,0,length);
	archiveFile.Read(content,length);

	//将其转换为宽字符
	string strCnt(content,length);
	wstring wContent = StringToWString( strCnt );

	//查找回车以决定行
	size_t lineFrom = 0;
	size_t linePos = wContent.find_first_of(L"\n",lineFrom);

	while( linePos != wstring::npos )
	{
		//得到一行数据
		wstring& wLine = wContent.substr(lineFrom, linePos-lineFrom);

#ifdef DEBUG
		acutPrintf(L"\n得到一行管线实体数据【%s】.",wLine.c_str());
#endif

		if(wLine.length() == 0)
			break;

		LineEntry *newLine = new LineEntry(wLine);
		m_LineList->push_back( newLine );

		//保存到数据库
		newLine->m_dbId = ArxWrapper::PostToNameObjectsDict(newLine->m_pDbEntry,LineEntry::LINE_ENTRY_LAYER);
		newLine->m_pDbEntry = NULL;

		//创建对应的图层
		//ArxWrapper::createNewLayer( newLine->m_LineName );

		//创建相应的柱体
		newLine->CreateDbObjects();
		//ArxWrapper::createLMALine(*newLine );

		//从下一个字符开始查找另外一行
		lineFrom = linePos + 1;
		linePos = wContent.find_first_of(L"\n",lineFrom + 1);
	}

	//关闭文件
	archiveFile.Close();
}

void LineEntryFile::Persistent() const
{
	acutPrintf(L"\n持久化管线数据.");

	//ExportTo(this->m_FileName);
}

void LineEntryFile::ExportTo(const wstring& filename,const wstring& lineKind) const
{
	acutPrintf(L"\n导出实体数据.");

	CString exportFile;
	exportFile.Format(L"%s",filename.c_str());
	CFile archiveFile(exportFile,CFile::modeCreate|CFile::modeWrite);

	//遍历所有的类型定义
	for( ConstLineIterator iter = m_LineList->begin(); 
			iter != m_LineList->end(); 
			iter++)
	{
		LineEntry* data = *iter;

		if( data 
			&& data->m_LineKind == lineKind )
		{
			//得到消息的宽字符序列化
			wstring wData = data->toString();

			//转为窄字符
			string dataStr = WstringToString(wData);

#ifdef DEBUG
			acutPrintf(L"\n管线实体数据【%s】.",wData.c_str());
#endif
			//使用 virtual void Write( const void* lpBuf, UINT nCount ); 将窄字符写入文件
			archiveFile.Write(dataStr.c_str(),(UINT)dataStr.size());

			//使用Windows默认的回车，换行
			archiveFile.Write("\r\n",(UINT)strlen("\r\n"));
		}
	}

	acutPrintf(L"\n管线实体数据导出完成.");
	archiveFile.Close();
}

void LineEntryFile::InsertLine(LineEntry* lineEntry)
{
	if( lineEntry )
		m_LineList->push_back(lineEntry);
}

void LineEntryFile::InsertLine( LineList* lineList)
{
	if( lineList == NULL || lineList->size() == 0)
		return;

	for( LineIterator iter = lineList->begin();
			iter != lineList->end();
			iter++)
	{
		m_LineList->push_back((*iter));
	}
}

BOOL LineEntryFile::UpdateLine(LineEntry* lineEntry)
{
	LineIterator iter = FindLinePos(lineEntry->m_LineID);

	if( iter != this->m_LineList->end())
	{
		(*iter)->m_LineName = lineEntry->m_LineName;
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

wstring LineEntryFile::GetNewPipeName( const LineCategoryItemData* pipeCategoryData, const wstring& orinalName )
{
	//Find name exist, plus sequence
	int index = 1;
	const wstring& pipeCategory = pipeCategoryData->mCategory;

	while(true)
	{
		CString pipeName;
		
		//种类_形状_尺寸_序号
		CString shape;
		if( pipeCategoryData->mShape == GlobalData::LINE_SHAPE_CIRCLE )
		{
			shape.Format(L"%s_%s",pipeCategoryData->mShape.c_str(),pipeCategoryData->mRadius.c_str());
		}
		else if( pipeCategoryData->mShape == GlobalData::LINE_SHAPE_SQUARE )
		{
			shape.Format(L"%s_%sx%s",pipeCategoryData->mShape.c_str(),
				pipeCategoryData->mWidth.c_str(),pipeCategoryData->mHeight.c_str());
		}
		else
		{
			shape.Format(L"%s",pipeCategoryData->mShape.c_str());
		}

		pipeName.Format(L"%s_%s_%d",pipeCategory.c_str(),shape.GetBuffer(),index);

		//如果是更新
		if( orinalName.length() != 0 )
		{
			//如果名字无变化，直接返回
			if( wstring(pipeName.GetBuffer()) == orinalName )
			{
				return wstring(pipeName.GetBuffer());
			}
		}

		//如果没有管线是这个名字，则新增
		if( this->FindLineByName(pipeName.GetBuffer()) == NULL )
		{
			return wstring(pipeName.GetBuffer());
		}
		else
		{
			//序列号递增
			index++;
		}
	}

	return pipeCategory;
}

LineList LineEntryFile::GetList( const wstring& entityKind )
{
	LineList kindList;

	//初始化左边栏树形数据
	for( LineIterator iter = m_LineList->begin();
			iter != m_LineList->end();
			iter++)
	{
		if( (*iter)->m_LineKind == entityKind )
		{
			kindList.push_back((*iter));
		}
	}

	return kindList;
}

/////////////////////////////////////////////////////////////////////////

EntryFileList* LineEntryFileManager::pEntryFileList = NULL;
bool LineEntryFileManager::openingDwg = false;

void LineEntryFileManager::ReadFromCurrentDWG()
{
#ifdef DEBUG
	acutPrintf(L"\n从当前DWG文件读取数据。");
#endif

	//ArxWrapper::PullFromNameObjectsDict();
}

void LineEntryFileManager::RemoveEntryFileOnDWGUnLoad()
{
#ifdef DEBUG
		acutPrintf(L"\nDWG文件关闭了，删除管理数据。");

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
		acutPrintf(L"\n文件实体管理器还未创建.");
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
	acutPrintf(L"\n没有找到文件【%s】对应的管线实体.",fileName.c_str());
#endif

	return NULL;
}

LineEntryFile* LineEntryFileManager::RegisterEntryFile(const wstring& fileName)
{
	LineEntryFile* entryFile = GetLineEntryFile(fileName);
	if( entryFile == NULL )
	{
		acutPrintf(L"\n创建【%s】对应的管线实体.",fileName.c_str());

		entryFile = new LineEntryFile(fileName);
		pEntryFileList->push_back( entryFile );
	}

	return entryFile;
}

LineEntryFile* LineEntryFileManager::SaveFileEntity()
{
	wstring fileName( curDoc()->fileName() );

	acutPrintf(L"\n文件保存为[%s].",fileName.c_str());

	return GetLineEntryFile(fileName);
}

bool LineEntryFileManager::RegisterLineSegment( const wstring& fileName, UINT lineID, UINT sequence, PointEntry*& pStart, PointEntry*& pEnd )
{
	//找到文件管理类
	LineEntryFile* pFileEntry = RegisterEntryFile(fileName);
	acutPrintf(L"\n添加线段时，找到管线实体文件管理器【%s】.",fileName.c_str());

	//找到实体类
	LineEntry* lineEntry = pFileEntry->FindLine(lineID);
	PointList* pPointList = NULL;

	if( lineEntry == NULL )
	{
#ifdef DEBUG
		acutPrintf(L"\n保存到临时管线管理器中.");
#endif
		pPointList = pFileEntry->GetTempLine( lineID );
	}

	if( sequence == 1 )
	{
#ifdef DEBUG
		acutPrintf(L"\n序列号为1，这是第一个线段.");
#endif
		if( pPointList->size() < 0 )
		{
			pStart = new PointEntry();
			pEnd = new PointEntry();

			pStart->m_PointNO = 0;
			pPointList->push_back( pStart );

			pEnd->m_PointNO = 1;
			pPointList->push_back( pEnd );
		}
		else
		{
			pStart = (*pPointList)[0];
			pEnd = (*pPointList)[1];
		}
	}
	else if ( sequence > 1 )
	{
#ifdef DEBUG
		acutPrintf(L"\n普通线段.");
#endif
		pEnd = new PointEntry();
		pEnd->m_PointNO = sequence;

		pPointList->push_back( pEnd );
	}
	else if ( sequence == 0)
	{
		acutPrintf(L"\n失效的线段.");
	}

	return true;
}

LineEntryFile* LineEntryFileManager::GetCurrentLineEntryFile()
{
	//Get current filename
	wstring fileName = curDoc()->fileName();

	acutPrintf(L"\n查找【%s】对应的的管线.",fileName.c_str());

	return RegisterEntryFile(fileName);
}

BOOL LineEntryFileManager::ImportLMALineFile( const wstring& lineKind )
{
	//导入选择对话框
	CString szFilter;
	szFilter.Format(L"%s", IsLineEdit(lineKind) ? L"管线数据文件 (*.ldt)|*.ldt||" : L"阻隔体数据文件 (*.bdt)|*.bdt||");
	CFileDialog dlg(TRUE, IsLineEdit(lineKind) ? L"ldt" : L"bdt", L"", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter.GetBuffer(), 
					CWnd::FromHandle(adsw_acadMainWnd()), 0/*, TRUE*/);

	//If user hasn't export once,use the arx load path
	if( !HasUserSavedFlagData( LMA_VERSION ) )
	{
		acutPrintf(L"\n默认目录为改移设计系统的程序目录.");
		dlg.m_ofn.lpstrInitialDir = gLmaArxLoadPath.c_str();
	}

	if (dlg.DoModal() == IDOK) 
	{
		//得到当前的文件实体管理器
		LineEntryFile* currentFile = GetCurrentLineEntryFile();

		//得到导入文件
        CString impFile = dlg.GetPathName();
		acutPrintf(L"\n导入管线文件【%s】.",impFile.GetBuffer());
		LineEntryFile* importFile = new LineEntryFile(impFile.GetBuffer(),true);

		//插入其中的管线
		currentFile->InsertLine( importFile->GetList() );

		//删除临时导入的文件实体
		delete importFile;

		//Set the exported flag, then next time use the user's last save/open
		if( !HasUserSavedFlagData( LMA_VERSION ) )
		{
			acutPrintf(L"\n用户已重置导入目录.");
			PlaceUserSavedFlagData( LMA_VERSION );
		}

        return(TRUE);
    }
    else
        return(FALSE);
}

BOOL LineEntryFileManager::ExportLMALineFile( const wstring& lineKind )
{
	//去除掉文件后缀（dwg）
	wstring fileName( curDoc()->fileName() );
	fileName = fileName.substr(0, fileName.find_first_of(L"."));

	//导出选择对话框
	CString szFilter;
	szFilter.Format(L"%s", IsLineEdit(lineKind) ? L"管线数据文件 (*.ldt)|*.ldt||" : L"阻隔体数据文件 (*.bdt)|*.bdt||");
	CFileDialog dlg(FALSE, IsLineEdit(lineKind) ? L"ldt" : L"bdt", fileName.c_str(), 
					OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter.GetBuffer(), 
					CWnd::FromHandle(adsw_acadMainWnd()), 0/*, TRUE*/);

	//If user hasn't export once,use the arx load path
	if( !HasUserSavedFlagData( LMA_VERSION ) )
	{
		acutPrintf(L"\n默认目录为改移设计系统的程序目录.");
		dlg.m_ofn.lpstrInitialDir = gLmaArxLoadPath.c_str();
	}

	if (dlg.DoModal() == IDOK) 
	{
        CString expFile = dlg.GetPathName();
		acutPrintf(L"\n导出管线数据到文件【%s】.",expFile.GetBuffer());

		LineEntryFile* exportFile = GetLineEntryFile(wstring(curDoc()->fileName()));

		if( exportFile )
			exportFile->ExportTo(expFile.GetBuffer(),lineKind);

		//Set the exported flag, then next time use the user's last save/open
		if( !HasUserSavedFlagData( LMA_VERSION ) )
		{
			acutPrintf(L"\n用户已重置导出目录.");
			PlaceUserSavedFlagData( LMA_VERSION );
		}

        return(TRUE);
    }
    else
        return(FALSE);
}

} // end of data

} // end of assistant

} // end of guch

} // end of com