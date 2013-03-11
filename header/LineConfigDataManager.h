#pragma once

#include <vector>
#include <LineCategoryItemData.h>

using namespace std;

namespace com
{

namespace guch
{

namespace assistant
{

namespace config
{

typedef vector<CommonConfig*> LineCommonConfigVector;
typedef LineCommonConfigVector::iterator ConfigIterator;

class LineConfigDataManager
{
public:

	static LineConfigDataManager* Instance();
	
	static wstring CONFIG_LINE_NAME;
	static wstring CONFIG_SHAPE_NAME;
	static wstring CONFIG_BLOCK_NAME;

	LineConfigDataManager(void);
	~LineConfigDataManager(void);

	LineCommonConfigVector* FindConfig( const wstring& category ) const;
	
	LineCommonConfigVector* mLineConfigData;

	wstring FindDefaultSize( const wstring& category);

protected:

	bool initialize();

private:

	static LineConfigDataManager* instance;
	static LPCWSTR LMA_CONFIG_FILE;

};

} // end of data

} // end of assistant

} // end of guch

} // end of com
