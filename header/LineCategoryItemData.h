// ------------------------------------------------
//                  LineManagementAssistant
// Copyright 2012-2013, Chengyong Yang & Changhai Gu. 
//               All rights reserved.
// ------------------------------------------------
//	LineCategoryItemData.h
//	written by Changhai Gu
// ------------------------------------------------
// $File:\\LineManageAssitant\header\LineCategoryItemData.h $
// $Author: Changhai Gu $
// $DateTime: 2013/4/15 19:35:46 $
// $Revision: #1 $
// ------------------------------------------------

#pragma once

#include <string>

#pragma warning(disable:4005) 

using namespace std;

namespace com
{

namespace guch
{

namespace assistant
{

namespace config
{

//通用配置数据
struct CommonConfig
{
	wstring mCategory;
	wstring mName;
	wstring mSubName;
	wstring mReserved;
};

struct LineSizeData
{
	//针对圆
	wstring mRadius;

	//针对矩形
	wstring mWidth;
	wstring mHeight;

	//扩展长度
	wstring mReservedA;
	wstring mReservedB;

	LineSizeData();
	
	LineSizeData( const LineSizeData& rData);

	LineSizeData( const wstring& rRadius,
					const wstring& rWidth,
					const wstring& rHeight,
					const wstring& rReservedA,
					const wstring& rReservedB);

	wstring toString() const;
};

struct LineCategoryItemData
{
	//配置的种类
	wstring mCategory;
	
	//圆、矩形
	wstring mShape;

	//大小数据
	struct LineSizeData mSize;

	//壁厚与安全距离
	wstring mWallSize;
	wstring mSafeSize;

	//平面标注于剖面标注
	wstring mPlaneMark;
	wstring mCutMark;

	//穿越方向
	wstring mThroughDirection;

	LineCategoryItemData(void);
	LineCategoryItemData( const wstring& rCategory,
							const wstring& rShape,
							const LineSizeData& rSize,
							const wstring& rWallSize,
							const wstring& rSafeSize,
							const wstring& rPlaneMark,
							const wstring& rCutMark,
							const wstring& rThroughDirection);

	LineCategoryItemData( const LineCategoryItemData& rData);

	wstring toString() const;
};

} // end of data

} // end of assistant

} // end of guch

} // end of com
