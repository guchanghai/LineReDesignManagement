#include "stdafx.h"
#include "LineCategoryItemData.h"

#pragma warning (disable : 4996 )

namespace com
{

namespace guch
{

namespace assistant
{

namespace config
{

LineSizeData::LineSizeData():
	mRadius(L"0"),
	mWidth(L"0"),
	mHeight(L"0"),
	mReservedA(L"0"),
	mReservedB(L"0")
{
}

LineSizeData::LineSizeData( const wstring& rRadius,
							const wstring& rWidth,
							const wstring& rHeight,
							const wstring& rReservedA,
							const wstring& rReservedB):
	mRadius(rRadius),
	mWidth(rWidth),
	mHeight(rHeight),
	mReservedA(rReservedA),
	mReservedB(rReservedB)
{
}

LineSizeData::LineSizeData( const LineSizeData& rData ):
	mRadius(rData.mRadius),
	mWidth(rData.mWidth),
	mHeight(rData.mHeight),
	mReservedA(rData.mReservedA),
	mReservedB(rData.mReservedB)
{
}

wstring LineSizeData::toString() const
{
	return mRadius + L"\t"
		+ mWidth + L"\t"
		+ mHeight + L"\t"
		+ mReservedA + L"\t"
		+ mReservedB;
}

LineCategoryItemData::LineCategoryItemData(void)
:mCategory(L""),
mShape(L""),
mSize(),
mWallSize(L"0"),
mSafeSize(L"0"),
mPlaneMark(L""),
mCutMark(L""),
mThroughDirection(L"")
{
}

LineCategoryItemData::LineCategoryItemData( const wstring& rCategory,
											const wstring& rShape,
											const LineSizeData& rSize,
											const wstring& rWallSize,
											const wstring& rSafeSize,
											const wstring& rPlaneMark,
											const wstring& rCutMark,
											const wstring& rThroughDirection)
:mCategory(rCategory),
mShape(rShape),
mSize(rSize),
mWallSize(rWallSize),
mSafeSize(rSafeSize),
mPlaneMark(rPlaneMark),
mCutMark(rCutMark),
mThroughDirection(rThroughDirection)
{}

LineCategoryItemData::LineCategoryItemData( const LineCategoryItemData& rData)
:mCategory(rData.mCategory),
mSize(rData.mSize),
mWallSize(rData.mWallSize),
mSafeSize(rData.mSafeSize),
mPlaneMark(rData.mPlaneMark),
mCutMark(rData.mCutMark),
mThroughDirection(rData.mThroughDirection)
{
}

wstring LineCategoryItemData::toString() const
{
	return mCategory + L"\t"
			+ mShape + L"\t"
			+ mSize.toString() + L"\t"
			+ mWallSize + L"\t"
			+ mSafeSize + L"\t"
			+ mPlaneMark + L"\t"
			+ mCutMark + L"\t"
			+ mThroughDirection;
}

} // end of data

} // end of assistant

} // end of guch

} // end of com