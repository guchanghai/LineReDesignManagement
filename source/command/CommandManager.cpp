#include "stdafx.h"

#include <CommandManager.h>
#include <LineManageAssitant.h>

#include <LineCutPosDialog.h>
#include <EntryManageDialog.h>
#include <ArxWrapper.h>
#include <GlobalDataConfig.h>

#include "AsdkAcUiDialogSample.h"
#include "AcExtensionModule.h"

using namespace com::guch::assistant::config;
using namespace com::guch::assistant::entry;
using namespace com::guch::assistant::data;

typedef map<wstring,AcRxFunctionPtr>::const_iterator CommandIterator;

CommandManager* CommandManager::gCmdManager = NULL;

const WCHAR* CommandManager::CMD_GROUP = L"LMA_CMD_GROUP";

const WCHAR* CommandManager::CMD_LINE_IMPORT = L"LMA_LINE_IMP";
const WCHAR* CommandManager::CMD_LINE_INPUT = L"LMA_LINE_INPUT";
const WCHAR* CommandManager::CMD_LINE_EXPORT = L"LMA_LINE_EXP";

const WCHAR* CommandManager::CMD_BLOCK_IMPORT = L"LMA_BLOCK_IMP";
const WCHAR* CommandManager::CMD_BLOCK_INPUT = L"LMA_BLOCK_INPUT";
const WCHAR* CommandManager::CMD_BLOCK_EXPORT = L"LMA_BLOCK_EXP";

const WCHAR* CommandManager::CMD_LIEN_CUT = L"LMA_CUT";
const WCHAR* CommandManager::CMD_LINE_CUT_BACK = L"LMA_CUT_BACK";

const WCHAR* CommandManager::CMD_LINE_TEST = L"LMA_TESTFUN";

CommandManager* CommandManager::instance()
{
	if( gCmdManager == NULL )
	{
		gCmdManager = new CommandManager();
	}

	return gCmdManager;
}

void CommandManager::Release()
{
	if( gCmdManager )
	{
		delete gCmdManager;
		gCmdManager = NULL;
	}
}

CommandManager::CommandManager(void)
{
	//管线录入、导出、导入
	mSupportCommands[CMD_LINE_IMPORT] = ImportLine;
	mSupportCommands[CMD_LINE_INPUT] = LineManage;
	mSupportCommands[CMD_LINE_EXPORT] = ExportLine;

	//阻隔体主功能
	mSupportCommands[CMD_BLOCK_IMPORT] = ImportBlock;
	mSupportCommands[CMD_BLOCK_INPUT] = BlockManage;
	mSupportCommands[CMD_BLOCK_EXPORT] = ExportBlock;

	//前面生成、恢复
	mSupportCommands[CMD_LIEN_CUT] = GenerateCut;
	mSupportCommands[CMD_LINE_CUT_BACK] = GenerateCutBack;

	mSupportCommands[CMD_LINE_TEST] = TestFunction;
}

CommandManager::~CommandManager(void)
{
}

void CommandManager::RegisterCommand() const
{
	for( CommandIterator iter = this->mSupportCommands.begin();
		iter != this->mSupportCommands.end();
		iter++)
	{
		CAcModuleResourceOverride resOverride;

		CString globalCmd;
		globalCmd.Format(L"G_%s",iter->first.c_str());

		acedRegCmds->addCommand(CMD_GROUP,globalCmd,
			iter->first.c_str(),
			ACRX_CMD_MODAL,
			iter->second);
	}
}

void CommandManager::UnRegisterCommand() const
{
	acedRegCmds->removeGroup(CMD_GROUP);

	CommandManager::Release();
}

void CommandManager::ImportLine()
{
#ifdef DEBUG
	acutPrintf(L"\n导入管线数据");
#endif

	LineEntryFileManager::ImportLMALineFile(GlobalData::KIND_LINE);
}

void CommandManager::LineManage()
{
#ifdef DEBUG
	acutPrintf(L"\n录入管线数据");
#endif

	EntryManageDialog dlg(CWnd::FromHandle(adsw_acadMainWnd()),GlobalData::KIND_LINE);
	INT_PTR nReturnValue = dlg.DoModal();
}

void CommandManager::ExportLine()
{
#ifdef DEBUG
	acutPrintf(L"\n导出管线数据");
#endif

	LineEntryFileManager::ExportLMALineFile(GlobalData::KIND_LINE);
}

void CommandManager::ImportBlock()
{
#ifdef DEBUG
	acutPrintf(L"\n导入阻隔体数据");
#endif

	LineEntryFileManager::ImportLMALineFile(GlobalData::KIND_BLOCK);
}

void CommandManager::BlockManage()
{
#ifdef DEBUG
	acutPrintf(L"\n录入阻隔体数据");
#endif

	EntryManageDialog dlg(CWnd::FromHandle(adsw_acadMainWnd()),GlobalData::KIND_BLOCK);
	INT_PTR nReturnValue = dlg.DoModal();
}

void CommandManager::ExportBlock()
{
#ifdef DEBUG
	acutPrintf(L"\n导出阻隔体数据");
#endif

	LineEntryFileManager::ExportLMALineFile(GlobalData::KIND_BLOCK);
}

void CommandManager::GenerateCut()
{
#ifdef DEBUG
	acutPrintf(L"\n生成切图");
#endif
	LineCutPosDialog dlg(CWnd::FromHandle(adsw_acadMainWnd()));
	INT_PTR nReturnValue = dlg.DoModal();
}

void CommandManager::GenerateCutBack()
{
#ifdef DEBUG
	acutPrintf(L"\n通过点击菜单恢复视窗");
#endif

	LineCutPosDialog::CutBack();
}

void CommandManager::TestFunction()
{
#ifdef DEBUG
	acutPrintf(L"\n测试AutoCAD的功能");
#endif

	ArxWrapper::TestFunction();
}
