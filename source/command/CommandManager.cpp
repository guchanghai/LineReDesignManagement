#include "stdafx.h"

#include <CommandManager.h>
#include <LineManageAssitant.h>

#include <LineCutPosDialog.h>
#include <EntryManageDialog.h>
#include <LineIntersectManage.h>
#include <ArxWrapper.h>
#include <GlobalDataConfig.h>
#include <LMACopyRight.h>

#include "AsdkAcUiDialogSample.h"
#include "AcExtensionModule.h"

using namespace com::guch::assistant::config;
using namespace com::guch::assistant::entry;
using namespace com::guch::assistant::data;
using namespace com::guch::assistant::Intersect;

typedef map<wstring,AcRxFunctionPtr>::const_iterator CommandIterator;

CommandManager* CommandManager::gCmdManager = NULL;

const WCHAR* CommandManager::CMD_GROUP = L"LMA_CMD_GROUP";

//管线输入
const WCHAR* CommandManager::CMD_LINE_IMPORT = L"LMA_LINE_IMP";
const WCHAR* CommandManager::CMD_LINE_INPUT = L"LMA_LINE_INPUT";
const WCHAR* CommandManager::CMD_LINE_EXPORT = L"LMA_LINE_EXP";

//阻隔体输入
const WCHAR* CommandManager::CMD_BLOCK_IMPORT = L"LMA_BLOCK_IMP";
const WCHAR* CommandManager::CMD_BLOCK_INPUT = L"LMA_BLOCK_INPUT";
const WCHAR* CommandManager::CMD_BLOCK_EXPORT = L"LMA_BLOCK_EXP";

//切剖面图
const WCHAR* CommandManager::CMD_LINE_CUT = L"LMA_CUT";
const WCHAR* CommandManager::CMD_LINE_CUT_BACK = L"LMA_CUT_BACK";

//遮挡切图
const WCHAR* CommandManager::CMD_LINE_SHADOW = L"LMA_SHADOW";
const WCHAR* CommandManager::CMD_LINE_SHADOW_BACK = L"LMA_SHADOW_BACK";

//管线相侵
const WCHAR* CommandManager::CMD_LINE_INTERACT = L"LMA_LINE_INTERACT";
const WCHAR* CommandManager::CMD_LINE_INTERACT_BACK = L"LMA_LINE_INTERACT_BACK";

//寻找路由
const WCHAR* CommandManager::CMD_LINE_ROUTE = L"LMA_LINE_ROUTE";

//功能测试
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

	//切面生成、恢复
	mSupportCommands[CMD_LINE_CUT] = GenerateCut;
	mSupportCommands[CMD_LINE_CUT_BACK] = GenerateCutBack;

	//遮挡切图
	mSupportCommands[CMD_LINE_SHADOW] = LineShadow;
	mSupportCommands[CMD_LINE_SHADOW_BACK] = LineShadowBack;

	//相侵判断、恢复
	mSupportCommands[CMD_LINE_INTERACT] = InteractCheck;
	mSupportCommands[CMD_LINE_INTERACT_BACK] = InteractCheckBack;

	//寻找路由
	mSupportCommands[CMD_LINE_ROUTE] = LineRoute;

	//测试功能
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

	LineEntityFileManager::ImportLMALineFile(GlobalData::KIND_LINE);
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

	LineEntityFileManager::ExportLMALineFile(GlobalData::KIND_LINE);
}

void CommandManager::ImportBlock()
{
#ifdef DEBUG
	acutPrintf(L"\n导入阻隔体数据");
#endif

	LineEntityFileManager::ImportLMALineFile(GlobalData::KIND_BLOCK);
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

	LineEntityFileManager::ExportLMALineFile(GlobalData::KIND_BLOCK);
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
	acutPrintf(L"\n删除临时切图，恢复3D视窗");
#endif

	LineCutPosDialog::Reset();
}

void CommandManager::LineShadow()
{
#ifdef DEBUG
	acutPrintf(L"\n遮挡切图");
#endif

	LineCutPosDialog dlg(CWnd::FromHandle(adsw_acadMainWnd()));
	INT_PTR nReturnValue = dlg.DoModal();
}

void CommandManager::LineShadowBack()
{
#ifdef DEBUG
	acutPrintf(L"\n删除临时遮挡切图，恢复3D视窗");
#endif

	LineCutPosDialog::Reset();
}

//侵限相关
void CommandManager::InteractCheck()
{
#ifdef DEBUG
	acutPrintf(L"\n侵限判断");
#endif

	LineIntersectManage::Instance()->CheckInteract();
}

void CommandManager::InteractCheckBack()
{
#ifdef DEBUG
	acutPrintf(L"\n删除侵限判断结果，恢复3D模型窗口");
#endif

	LineIntersectManage::Instance()->Reset();
}

void CommandManager::LineRoute()
{
#ifdef DEBUG
	acutPrintf(L"\n计算路由，解决侵限问题");
#endif

	LineIntersectManage::Instance()->Reset();
}

void CommandManager::TestFunction()
{
#ifdef DEBUG
	acutPrintf(L"\n显示版本信息");
#endif

	LMACopyRight dlg(CWnd::FromHandle(adsw_acadMainWnd()));
	INT_PTR nReturnValue = dlg.DoModal();

	//ArxWrapper::TestFunction();
}
