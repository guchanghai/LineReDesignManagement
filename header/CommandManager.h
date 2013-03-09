#pragma once

#include <map>
#include <string>

#include <accmd.h>

using namespace std;

class CommandManager
{
public:

	static CommandManager* gCmdManager;

	static CommandManager* instance();

	static void Release();

	void RegisterCommand() const;

	void UnRegisterCommand() const;

	//管线
	static void ImportLine();
	
	static void LineManage();

	static void ExportLine();

	//阻隔体
	static void ImportBlock();

	static void BlockManage();

	static void ExportBlock();

	//切割相关
	static void GenerateCut();

	static void GenerateCutBack();

	//测试
	static void TestFunction();

	static const WCHAR* CMD_GROUP;

	static const WCHAR* CMD_LINE_IMPORT;
	static const WCHAR* CMD_LINE_INPUT;
	static const WCHAR* CMD_LINE_EXPORT;

	static const WCHAR* CMD_BLOCK_IMPORT; 
	static const WCHAR* CMD_BLOCK_INPUT; 
	static const WCHAR* CMD_BLOCK_EXPORT; 

	static const WCHAR* CMD_LIEN_CUT;
	static const WCHAR* CMD_LINE_CUT_BACK;

	static const WCHAR* CMD_LINE_TEST;

private:

	CommandManager(void);
	~CommandManager(void);

	//list to contains the commands supported
	map<wstring,AcRxFunctionPtr> mSupportCommands;
};
