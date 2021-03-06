#include "MenuLMAMain.h"

#include "Resource.h"
#include <acdocman.h>
#include <acutads.h>

#include "StdAfx.h"

#include <CommandManager.h>

extern HINSTANCE _hdllInstance = NULL;//全局变量

MenuLMAMain::MenuLMAMain(void)
{
	acutPrintf(L"\n开始构造主菜单。");
	acDocManagerPtr()->pushResourceHandle(_hdllInstance);

	this->mpMenu = new CMenu();
	this->mpMenu->LoadMenu(IDR_MENU1);// ID_LMA_MAIN_MENU 是要调入的菜单 

	acDocManager->popResourceHandle();
	acutPrintf(L"\n主菜单构造完成。");
}

MenuLMAMain::~MenuLMAMain(void)
{
	if (mpMenu) 
		delete mpMenu;
}

void* MenuLMAMain::getMenuContext(const AcRxClass *, const AcDbObjectIdArray&)
{
	acutPrintf(L"\n得到子菜单");
	m_tempHMenu = mpMenu->GetSubMenu(0)->GetSafeHmenu();   
	return &m_tempHMenu;
}

void MenuLMAMain::onCommand(Adesk::UInt32 cmdIndex)
{
	acDocManager->pushResourceHandle(_hdllInstance);

	CString strMenuTitle,strPrompt;
	mpMenu->GetMenuString(cmdIndex,strMenuTitle,MF_BYCOMMAND);

	strPrompt.Format(L"\n您已经选取了菜单:%s\n",strMenuTitle);
	acutPrintf(strPrompt); // 我们的例子显示哪一个菜单项被选择

	onAction(strMenuTitle);

	acedPostCommandPrompt();
	acDocManager->pushResourceHandle(_hdllInstance); // 切换当前使用的资源
}

void MenuLMAMain::OnUpdateMenu()
{
	//mpMenu->EnableMenuItem(ID_LINE_CONFIG,MF_GRAYED); // 使菜单变灰
	//mpMenu->EnableMenuItem(ID_INPUT_POINT,MF_ENABLED); // 使能菜单项
	//mpMenu->CheckMenuItem(ID_GENERATE_CUT, MF_BYCOMMAND|MF_CHECKED); // 复选菜单项 
}

void MenuLMAMain::onAction(const CString& menuName)
{
	if( menuName == MAIN_MENU_LINE_INPUT )
	{
		CommandManager::LineManage();
	}
	else if( menuName == MAIN_MENU_LINE_EXPORT )
	{
		CommandManager::ExportLine();
	}
	else if( menuName == MAIN_MENU_LINE_IMPORT )
	{
		CommandManager::ImportLine();
	}
	if( menuName == MAIN_MENU_BLOCK_INPUT )
	{
		CommandManager::BlockManage();
	}
	else if( menuName == MAIN_MENU_LINE_EXPORT )
	{
		CommandManager::ExportBlock();
	}
	else if( menuName == MAIN_MENU_LINE_IMPORT )
	{
		CommandManager::ImportBlock();
	}
	else if( menuName == MAIN_MENU_LINE_CUT )
	{
		CommandManager::GenerateCut();
	}
	else if( menuName == MAIN_MENU_LINE_CUT_BACK )
	{
		CommandManager::GenerateCutBack();
	}
	else if( menuName == MAIN_MENU_LINE_SHADOW )
	{
		CommandManager::GenerateCut();
	}
	else if( menuName == MAIN_MENU_LINE_CUT_BACK )
	{
		CommandManager::LineShadow();
	}
	else if( menuName == MAIN_MENU_LINE_SHADOW_BACK )
	{
		CommandManager::LineShadowBack();
	}
	else if( menuName == MAIN_MENU_LINE_CALCULATE_ROUTE )
	{
		CommandManager::LineRoute();
	}
	else if( menuName == MAIN_MENU_LINE_DELETE_ROUTE )
	{
		CommandManager::LineRouteBack();
	}
	else
	{
		acutPrintf(L"\n未定义的菜单项【%s】",const_cast<CString&>(menuName).GetBuffer());
	}
}

