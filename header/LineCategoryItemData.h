#pragma once

/**
 * This class contains the data for the line configuration item
 **/

#include <string>

#pragma warning(disable:4005) 
/*
#include <boost/serialization/string.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/assume_abstract.hpp>
*/
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

struct LineCategoryItemData
{
	int mIndex;

	UINT mID;

	//管线的种类
	wstring mKind;

	//配置的种类
	wstring mCategory;
	
	//圆、矩形
	wstring mShape;

	//针对圆
	wstring mRadius;

	//针对矩形
	wstring mWidth;
	wstring mHeight;

	wstring mWallSize;
	wstring mSafeSize;

	wstring mPlaneMark;
	wstring mCutMark;

	wstring mCanThrough;
	wstring mThroughDirection;

	LineCategoryItemData(void);
	LineCategoryItemData( const int& index,
							const UINT& rID,
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
							const wstring& rThroughDirection);

	LineCategoryItemData( const LineCategoryItemData& rData);

	~LineCategoryItemData(void);

	//friend class boost::serialization::access;
	//friend std::ostream & operator<<(std::ostream &os, const LineCategoryItemData &itemData);

    template<class Archive>
    void serialize(Archive & ar, const unsigned int /* file_version */){
		/*
        ar  & BOOST_SERIALIZATION_NVP(mID)
            & BOOST_SERIALIZATION_NVP(mName)
            & BOOST_SERIALIZATION_NVP(mKind);
		*/
    }

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
    {
        // note, version is always the latest when saving
        ar  & mID;
        ar  & mName;
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ar	& mID;
        ar  & mName;
    }

	wstring toString() const;
};

} // end of data

} // end of assistant

} // end of guch

} // end of com
