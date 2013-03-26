#include "stdafx.h"

#include <LineConfigDataManager.h>
#include <GlobalDataConfig.h>
#include <LMAUtils.h>

#include <iostream>
#include <fstream>

#include <assert.h>
#include <acutads.h>

using namespace com::guch::assistant::data;

#pragma warning(disable : 4267)
#pragma warning(disable : 4244)

extern wstring gLmaArxLoadPath;
static wstring gConfigLineEnd = L"\r\n";
static wstring gColumnSegment = L"\t";

namespace com
{

namespace guch
{

namespace assistant
{

namespace config
{

LineConfigDataManager* LineConfigDataManager::instance = NULL;
LPCWSTR LineConfigDataManager::LMA_CONFIG_FILE = L"管线改移辅助系统配置文件.txt";

wstring LineConfigDataManager::CONFIG_LINE_NAME = L"管线种类";
wstring LineConfigDataManager::CONFIG_SHAPE_NAME = L"管线形状";
wstring LineConfigDataManager::CONFIG_BLOCK_NAME = L"阻隔体种类";

LineConfigDataManager* LineConfigDataManager::Instance()
{
	if( instance == NULL )
	{
		instance = new LineConfigDataManager();
	}

	return instance;
}

LineConfigDataManager::LineConfigDataManager(void)
{
	mLineConfigData = new LineCommonConfigVector();

#ifdef _DEMO_DATA
	const int MAX_ITEM = 10;

	for( int i = 0; i < MAX_ITEM; i++)
	{
		CString ID;
		ID.Format(L"%d",i);

		CommonConfig* item = new CommonConfig(wstring(ID.GetBuffer()), 
						L"测试管道",
						GlobalData::KIND_LINE,
						GlobalData::LINE_CATEGORY_SHANGSHUI,
						GlobalData::LINE_SHAPE_CIRCLE,
						L"15",
						L"5",
						GlobalData::LINE_UNIT_CM,
						L"测试数据");

		mLineConfigData->push_back(item);
	}
#else
	CFile archiveFile;

	try
	{
		//read data from file LMA_CONFIG_FILE
		wstring configFile = gLmaArxLoadPath + L"\\" + LMA_CONFIG_FILE;
		acutPrintf(L"\n配置文件路径是【%s】",configFile.c_str());

		BOOL result = archiveFile.Open(configFile.c_str(),CFile::modeRead);
		if( !result )
		{
			acutPrintf(L"\n打开管线类型配置文件失败.");
			return;
		}

		//得到文件内容长度
		int length = (ULONGLONG)archiveFile.GetLength()+1;

		//得到文件的窄字符内容
		char* content = new char[length];
		memset(content,0,length);
		archiveFile.Read(content,length);

		//将其转换为宽字符
		string strCnt(content,length);
		wstring wContent = StringToWString( strCnt );

		//查找回车以决定行
		size_t lineFrom = 0;
		size_t linePos = wContent.find_first_of(gConfigLineEnd,lineFrom);

		wstring category;
		while( linePos != wstring::npos )
		{
			//得到一行数据
			wstring& wLine = wContent.substr(lineFrom, linePos-lineFrom);

			//注释行
			if( wLine.substr(0,2) == L"##" )
			{
				acutPrintf(L"\n注释【%s】", wLine.substr(0,wLine.length()).c_str());
			}
			else if (wLine.substr(0,2) == L"**" )
			{
				//得到种类
				category = wLine.substr(2);
				acutPrintf(L"\n得到种类【%s】", category.c_str());
			}
			else
			{
				//将此行拆分
				size_t columnFrom = 0;
				size_t columnPos = wLine.find_first_of(gColumnSegment,columnFrom);

				CommonConfig* newItem = new CommonConfig();
				newItem->mCategory = category;

				int indexCol = 0;
				while( columnPos != wstring::npos )
				{
					//得到一个分段
					wstring& rColumn = wLine.substr(columnFrom,columnPos-columnFrom);

					//决定其属性
					if( indexCol == 0 )
					{
						newItem->mName = rColumn;
					}

					indexCol++;

					//继续下一个column
					columnFrom = columnPos + gColumnSegment.length();
					columnPos =  wLine.find_first_of(gColumnSegment,columnFrom);
				}

				wstring& name = wLine.substr(columnFrom);
				if( indexCol == 0 )
				{
					newItem->mName = name;
				}
				else if( indexCol == 1 )
				{
					newItem->mSubName = name;
				}

				mLineConfigData->push_back(newItem);
				acutPrintf(L"\n读取配置数据 - 大类【%s】种类【%s】子类【%s】", 
												newItem->mCategory.c_str(),
												newItem->mName.c_str(),
												newItem->mSubName.c_str());
			}

			//从下一个字符开始查找另外一行
			lineFrom = linePos + gConfigLineEnd.length();
			linePos = wContent.find_first_of(gConfigLineEnd,lineFrom);
		}

		//关闭文件
		archiveFile.Close();
	}
	catch(exception& e)
	{
		acutPrintf(L"\n打开管线类型配置文件异常【%s】",e.what());
	}

#endif
}

LineConfigDataManager::~LineConfigDataManager(void)
{
}

LineCommonConfigVector* LineConfigDataManager::FindConfig( const wstring& category ) const
{
	LineCommonConfigVector* configLig = new LineCommonConfigVector();

	for( ConfigIterator iter = mLineConfigData->begin();
		iter != mLineConfigData->end();
		iter++)
	{
		//不是按管线配置的种类查找，而是按用户新建管线的类型查找
		if( (*iter)->mCategory.find(category) != wstring::npos )
		{
			configLig->push_back( (*iter) );
		}
	}

	return configLig;
}

wstring LineConfigDataManager::FindDefaultSafeSize( const wstring& category)
{
	wstring lineSafeSize(L"0");

	for( ConfigIterator iter = mLineConfigData->begin();
		iter != mLineConfigData->end();
		iter++)
	{
		if( (*iter)->mName.find(category) != wstring::npos )
		{
			lineSafeSize = (*iter)->mSubName;
		}
	}

	if( lineSafeSize.length() == 0 )
		lineSafeSize = L"0";

	return lineSafeSize;
}

} // end of data

} // end of assistant

} // end of guch

} // end of com
