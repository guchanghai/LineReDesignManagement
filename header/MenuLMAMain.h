#pragma once

#include "afxwin.h"

#include "aced.h"

/*
* 主菜单
*/

#define MAIN_MENU_NAME L"管线改移设计"

/*
* 管线菜单
*/

#define MAIN_MENU_LINE_MANAGE L"管线输入"

#define MAIN_MENU_LINE_IMPORT L"管线数据导入"

#define MAIN_MENU_LINE_INPUT L"管线数据录入"

#define MAIN_MENU_LINE_EXPORT L"管线数据导出"

/*
* 阻隔体菜单
*/

#define MAIN_MENU_BLOCK_MANAGE L"阻隔体输入"

#define MAIN_MENU_BLOCK_IMPORT L"阻隔体数据导入"

#define MAIN_MENU_BLOCK_INPUT L"阻隔体数据录入"

#define MAIN_MENU_BLOCK_EXPORT L"阻隔体数据导出"

/*
* 切图菜单
*/

#define MAIN_MENU_LINE_CUT_MANAGE L"切剖面图"

#define MAIN_MENU_LINE_CUT L"生成切图"

#define MAIN_MENU_LINE_CUT_BACK L"恢复视窗"

/*
* 测试
*/

#define MAIN_MENU_LINE_TEST L"功能测试"

class MenuLMAMain : public AcEdUIContext
{

public:

	MenuLMAMain(void);

	~MenuLMAMain(void);

    virtual void* getMenuContext(const AcRxClass *pClass, const AcDbObjectIdArray& ids) ;
    virtual void  onCommand(Adesk::UInt32 cmdIndex);
    virtual void  OnUpdateMenu();

	void onAction(const CString& menuName);

private:

	HMENU m_tempHMenu;
    CMenu *mpMenu;
};

