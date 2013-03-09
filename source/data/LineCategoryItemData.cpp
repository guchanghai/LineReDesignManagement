#include "stdafx.h"
#include "LineCategoryItemData.h"
#include <LMAUtils.h>

#pragma warning (disable : 4996 )

namespace com
{

namespace guch
{

namespace assistant
{

namespace config
{

LineCategoryItemData::LineCategoryItemData(void)
:mIndex(0),
mID(0),
mKind(L""),
mCategory(L""),
mShape(L""),
mRadius(L"0"),
mWidth(L"0"),
mHeight(L"0"),
mWallSize(L"0"),
mSafeSize(L"0"),
mPlaneMark(L""),
mCutMark(L""),
mCanThrough(L""),
mThroughDirection(L"")
{
}

LineCategoryItemData::LineCategoryItemData( const int&		index,
											const UINT&		rID,
											const wstring& rKind,
											const wstring& rCategory,
											const wstring& rShape,
											const wstring& rRadius,
											const wstring& rWidth,
											const wstring& rHeight,
											const wstring& rWallSize,
											const wstring& rSafeSize,
											const wstring& rPlaneMark,
											const wstring& rCutMark,
											const wstring& rCanThrough,
											const wstring& rThroughDirection)
:mIndex(index),
mID(rID),
mKind(rKind),
mCategory(rCategory),
mShape(rShape),
mRadius(rRadius),
mWidth(rWidth),
mHeight(rHeight),
mWallSize(rWallSize),
mSafeSize(rSafeSize),
mPlaneMark(rPlaneMark),
mCutMark(rCutMark),
mCanThrough(rCanThrough),
mThroughDirection(rThroughDirection)
{}

LineCategoryItemData::LineCategoryItemData( const LineCategoryItemData& rData)
:mIndex(rData.mIndex),
mID(rData.mID),
mKind(rData.mKind),
mCategory(rData.mCategory),
mWidth(rData.mWidth),
mHeight(rData.mHeight),
mWallSize(rData.mWallSize),
mSafeSize(rData.mSafeSize),
mPlaneMark(rData.mPlaneMark),
mCutMark(rData.mCutMark),
mCanThrough(rData.mCanThrough),
mThroughDirection(rData.mThroughDirection)
{
}

wstring LineCategoryItemData::toString() const
{
	return mCategory + L"\t"
			+ mShape + L"\t"
			+ mRadius + L"\t"
			+ mWidth + L"\t"
			+ mHeight + L"\t"
			+ mWallSize + L"\t"
			+ mSafeSize + L"\t"
			+ mPlaneMark + L"\t"
			+ mCutMark + L"\t"
			+ mCanThrough + L"\t"
			+ mThroughDirection + L"\t";
}

LineCategoryItemData::~LineCategoryItemData(void){}

/*
std::ostream & operator<<(std::ostream &os, const LineCategoryItemData &itemData)
{
	return os << itemData.mID << itemData.mName << itemData.mKind;
}
*/

} // end of data

} // end of assistant

} // end of guch

} // end of com