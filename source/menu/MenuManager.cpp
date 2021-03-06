
#include "MenuLMAMain.h"

#include "MenuManager.h"
#include "aced.h"

#include "StdAfx.h"

#include <afxdllx.h>
#include <rxregsvc.h>
#include <rxmfcapi.h>

#include "CAcadApplication.h"
#include "CAcadDocument.h"
#include "CAcadModelSpace.h"
#include "CAcadMenuBar.h"
#include "CAcadMenuGroup.h"
#include "CAcadMenuGroups.h"
#include "CAcadPopupMenu.h"
#include "CAcadPopupMenus.h"

#include <CommandManager.h>

MenuManager* MenuManager::gMenuManager = NULL;

void MenuManager::CreateMenu(void* appId)
{
	if( gMenuManager == NULL )
	{
		gMenuManager = new MenuManager(appId);
	}
}

MenuManager* MenuManager::instance()
{
	assert(gMenuManager);

	return gMenuManager;
}

void MenuManager::unRegister()
{
	if( gMenuManager )
	{
		delete gMenuManager;
	}
}

MenuManager::MenuManager(const void* appId)
	:mAppId(appId)
{
	RegisterMenu();
}

MenuManager::~MenuManager(void)
{
	UnRegisterMenu();
}

void MenuManager::RegisterMenu()
{
	mpMainMenu = new MenuLMAMain();
	
	//注册上下文菜单
	acedAddDefaultContextMenu(mpMainMenu, mAppId, MAIN_MENU_NAME);

	//注册下拉菜单
	AddDropdownMenu();

	acutPrintf(L"\n菜单加载成功.");
}

void MenuManager::UnRegisterMenu()
{
	//删除上下文菜单
	if( mpMainMenu )
	{
		acutPrintf(L"\n辅助系统菜单开始卸载.");
		acedRemoveDefaultContextMenu(mpMainMenu ); // 移除默认上下文菜单
	}

	//删除主下拉菜单
	RemoveDropdownMenu();

	acutPrintf(L"\n菜单卸载成功.");
}


void MenuManager::AddDropdownMenu()
{
    TRY
    {
		int menuIndex = 0;

        CAcadApplication IAcad(acedGetAcadWinApp()->GetIDispatch(TRUE));

        CAcadMenuBar IMenuBar(IAcad.get_MenuBar());

        long numberOfMenus;
        numberOfMenus = IMenuBar.get_Count();

        CAcadMenuGroups IMenuGroups(IAcad.get_MenuGroups());

        VARIANT index;
        VariantInit(&index);
        V_VT(&index) = VT_I4;
        V_I4(&index) = menuIndex++;

        CAcadMenuGroup IMenuGroup(IMenuGroups.Item(index));

        CAcadPopupMenus IPopUpMenus(IMenuGroup.get_Menus());

        CString cstrMenuName = MAIN_MENU_NAME;

        VariantInit(&index);
        V_VT(&index) = VT_BSTR;
        V_BSTR(&index) = cstrMenuName.AllocSysString();

        IDispatch* pDisp=NULL;

		//see if the menu is already there
        TRY{pDisp = IPopUpMenus.Item(index); pDisp->AddRef();} CATCH(COleDispatchException,e){}END_CATCH;

		CString cmdMenuName;

        if (pDisp==NULL) {
            //create it
            CAcadPopupMenu IPopUpMenu(IPopUpMenus.Add(cstrMenuName));

            VariantInit(&index);
            V_VT(&index) = VT_I4;
            V_I4(&index) = 0;

			//管线菜单
			cmdMenuName.Format(L"%s\n", MAIN_MENU_LINE_MANAGE );
            CAcadPopupMenu ILineConfigManagePopUpMenu(IPopUpMenu.AddSubMenu(index, cmdMenuName));

			{
				IDispatch* pLineConfigManageDisp=NULL;

				VariantInit(&index);
				V_VT(&index) = VT_I4;
				V_I4(&index) = 0;

				cmdMenuName.Format(L"%s\n",CommandManager::CMD_LINE_IMPORT);
				ILineConfigManagePopUpMenu.AddMenuItem(index, MAIN_MENU_LINE_IMPORT, cmdMenuName);
			
				VariantInit(&index);
				V_VT(&index) = VT_I4;
				V_I4(&index) = 1;

				cmdMenuName.Format(L"%s\n",CommandManager::CMD_LINE_INPUT);
				ILineConfigManagePopUpMenu.AddMenuItem(index, MAIN_MENU_LINE_INPUT, cmdMenuName);
			
				VariantInit(&index);
				V_VT(&index) = VT_I4;
				V_I4(&index) = 2;

				cmdMenuName.Format(L"%s\n",CommandManager::CMD_LINE_EXPORT);
				ILineConfigManagePopUpMenu.AddMenuItem(index, MAIN_MENU_LINE_EXPORT, cmdMenuName);

				pLineConfigManageDisp = ILineConfigManagePopUpMenu.m_lpDispatch;
				pLineConfigManageDisp->AddRef();
			}

            VariantInit(&index);
            V_VT(&index) = VT_I4;
            V_I4(&index) = menuIndex++;
            IPopUpMenu.AddSeparator(index);

            VariantInit(&index);
            V_VT(&index) = VT_I4;
            V_I4(&index) = menuIndex++;

			//阻隔体菜单
			cmdMenuName.Format(L"%s\n", MAIN_MENU_BLOCK_MANAGE );
            CAcadPopupMenu ILineManagePopUpMenu(IPopUpMenu.AddSubMenu(index, cmdMenuName));
			
			{
				IDispatch* pLineManageDisp=NULL;

				VariantInit(&index);
				V_VT(&index) = VT_I4;
				V_I4(&index) = 0;

				cmdMenuName.Format(L"%s\n",CommandManager::CMD_BLOCK_IMPORT);
				ILineManagePopUpMenu.AddMenuItem(index, MAIN_MENU_BLOCK_IMPORT, cmdMenuName);
			
				VariantInit(&index);
				V_VT(&index) = VT_I4;
				V_I4(&index) = 1;

				cmdMenuName.Format(L"%s\n",CommandManager::CMD_BLOCK_INPUT);
				ILineManagePopUpMenu.AddMenuItem(index, MAIN_MENU_BLOCK_INPUT, cmdMenuName);
			
				VariantInit(&index);
				V_VT(&index) = VT_I4;
				V_I4(&index) = 2;

				cmdMenuName.Format(L"%s\n",CommandManager::CMD_BLOCK_EXPORT);
				ILineManagePopUpMenu.AddMenuItem(index, MAIN_MENU_BLOCK_EXPORT, cmdMenuName);

				pLineManageDisp = ILineManagePopUpMenu.m_lpDispatch;
				pLineManageDisp->AddRef();
			}

			VariantInit(&index);
            V_VT(&index) = VT_I4;
            V_I4(&index) = menuIndex++;
            IPopUpMenu.AddSeparator(index);

            VariantInit(&index);
            V_VT(&index) = VT_I4;
            V_I4(&index) = menuIndex++;

			//切图菜单
			cmdMenuName.Format(L"%s\n", MAIN_MENU_LINE_CUT_MANAGE );
            CAcadPopupMenu ICutManagePopUpMenu(IPopUpMenu.AddSubMenu(index, cmdMenuName));

			{
				IDispatch* pCutManageDisp=NULL;

				VariantInit(&index);
				V_VT(&index) = VT_I4;
				V_I4(&index) = 0;

				cmdMenuName.Format(L"%s\n",CommandManager::CMD_LINE_CUT);
				ICutManagePopUpMenu.AddMenuItem(index, MAIN_MENU_LINE_CUT, cmdMenuName);
			
				VariantInit(&index);
				V_VT(&index) = VT_I4;
				V_I4(&index) = 1;

				cmdMenuName.Format(L"%s\n",CommandManager::CMD_LINE_CUT_BACK);
				ICutManagePopUpMenu.AddMenuItem(index, MAIN_MENU_LINE_CUT_BACK, cmdMenuName);

				pCutManageDisp = ICutManagePopUpMenu.m_lpDispatch;
				pCutManageDisp->AddRef();
			}

			VariantInit(&index);
            V_VT(&index) = VT_I4;
            V_I4(&index) = menuIndex++;
            IPopUpMenu.AddSeparator(index);

			VariantInit(&index);
            V_VT(&index) = VT_I4;
            V_I4(&index) = menuIndex++;

			//侵限菜单
			cmdMenuName.Format(L"%s\n", MAIN_MENU_LINE_INTERACT_MANAGE );
            CAcadPopupMenu ILineInteractManagePopUpMenu(IPopUpMenu.AddSubMenu(index, cmdMenuName));

			{
				IDispatch* pLineInteractManageDisp=NULL;

				VariantInit(&index);
				V_VT(&index) = VT_I4;
				V_I4(&index) = 0;

				cmdMenuName.Format(L"%s\n",CommandManager::CMD_LINE_INTERACT);
				ILineInteractManagePopUpMenu.AddMenuItem(index, MAIN_MENU_LINE_INTERACT, cmdMenuName);
			
				VariantInit(&index);
				V_VT(&index) = VT_I4;
				V_I4(&index) = 1;

				cmdMenuName.Format(L"%s\n",CommandManager::CMD_LINE_INTERACT_BACK);
				ILineInteractManagePopUpMenu.AddMenuItem(index, MAIN_MENU_LINE_INTERACT_BACK, cmdMenuName);

				pLineInteractManageDisp = ILineInteractManagePopUpMenu.m_lpDispatch;
				pLineInteractManageDisp->AddRef();
			}

			VariantInit(&index);
            V_VT(&index) = VT_I4;
            V_I4(&index) = menuIndex++;
            IPopUpMenu.AddSeparator(index);

			VariantInit(&index);
            V_VT(&index) = VT_I4;
            V_I4(&index) = menuIndex++;

			//遮挡菜单
			cmdMenuName.Format(L"%s\n", MAIN_MENU_LINE_SHADOW_MANAGE );
            CAcadPopupMenu ILineShadowManagePopUpMenu(IPopUpMenu.AddSubMenu(index, cmdMenuName));

			{
				IDispatch* pSubMenuDisp=NULL;

				VariantInit(&index);
				V_VT(&index) = VT_I4;
				V_I4(&index) = 0;

				cmdMenuName.Format(L"%s\n",CommandManager::CMD_LINE_SHADOW);
				ILineShadowManagePopUpMenu.AddMenuItem(index, MAIN_MENU_LINE_SHADOW, cmdMenuName);
			
				VariantInit(&index);
				V_VT(&index) = VT_I4;
				V_I4(&index) = 1;

				cmdMenuName.Format(L"%s\n",CommandManager::CMD_LINE_SHADOW_BACK);
				ILineShadowManagePopUpMenu.AddMenuItem(index, MAIN_MENU_LINE_SHADOW_BACK, cmdMenuName);

				pSubMenuDisp = ILineShadowManagePopUpMenu.m_lpDispatch;
				pSubMenuDisp->AddRef();
			}

			VariantInit(&index);
            V_VT(&index) = VT_I4;
            V_I4(&index) = menuIndex++;
            IPopUpMenu.AddSeparator(index);

			VariantInit(&index);
            V_VT(&index) = VT_I4;
            V_I4(&index) = menuIndex++;

			//计算路由
			cmdMenuName.Format(L"%s\n", MAIN_MENU_LINE_ROUTE );
            CAcadPopupMenu ILineCalRoutePopUpMenu(IPopUpMenu.AddSubMenu(index, cmdMenuName));

			{
				IDispatch* pSubMenuDisp=NULL;

				VariantInit(&index);
				V_VT(&index) = VT_I4;
				V_I4(&index) = 0;

				cmdMenuName.Format(L"%s\n",CommandManager::CMD_LINE_ROUTE);
				ILineCalRoutePopUpMenu.AddMenuItem(index, MAIN_MENU_LINE_CALCULATE_ROUTE, cmdMenuName);
			
				VariantInit(&index);
				V_VT(&index) = VT_I4;
				V_I4(&index) = 1;

				cmdMenuName.Format(L"%s\n",CommandManager::CMD_LINE_ROUTE_BACK);
				ILineCalRoutePopUpMenu.AddMenuItem(index, MAIN_MENU_LINE_DELETE_ROUTE, cmdMenuName);

				pSubMenuDisp = ILineCalRoutePopUpMenu.m_lpDispatch;
				pSubMenuDisp->AddRef();
			}

			VariantInit(&index);
            V_VT(&index) = VT_I4;
            V_I4(&index) = menuIndex++;
            IPopUpMenu.AddSeparator(index);

			VariantInit(&index);
            V_VT(&index) = VT_I4;
            V_I4(&index) = menuIndex++;

			//测试菜单
			cmdMenuName.Format(L"%s\n",CommandManager::CMD_LINE_TEST);
            IPopUpMenu.AddMenuItem(index, MAIN_MENU_LINE_TEST, cmdMenuName);
      
			pDisp = IPopUpMenu.m_lpDispatch;
            pDisp->AddRef();
        }

        CAcadPopupMenu IPopUpMenu(pDisp);
        if (!IPopUpMenu.get_OnMenuBar())
        {
            VariantInit(&index);
            V_VT(&index) = VT_I4;
            V_I4(&index) = numberOfMenus - 1;
            IPopUpMenu.InsertInMenuBar(index);
        }

        pDisp->Release();
    }
    CATCH(COleDispatchException,e)
    {
        e->ReportError();
        e->Delete();
    }

    END_CATCH;
}

void MenuManager::RemoveDropdownMenu()
{
	TRY
    {
        CAcadApplication IAcad(acedGetAcadWinApp()->GetIDispatch(TRUE));

        CAcadMenuBar IMenuBar(IAcad.get_MenuBar());

        CAcadMenuGroups IMenuGroups(IAcad.get_MenuGroups());

        VARIANT index;
        VariantInit(&index);
        V_VT(&index) = VT_I4;
        V_I4(&index) = 0;

        CAcadMenuGroup IMenuGroup(IMenuGroups.Item(index));

        CAcadPopupMenus IPopUpMenus(IMenuGroup.get_Menus());

        CString cstrMenuName = MAIN_MENU_NAME;

        VariantInit(&index);
        V_VT(&index) = VT_BSTR;
        V_BSTR(&index) = cstrMenuName.AllocSysString();

        IDispatch* pDisp=NULL;

		//see if the menu is already there
        TRY{pDisp = IPopUpMenus.Item(index); pDisp->AddRef();} CATCH(COleDispatchException,e){}END_CATCH;

        if (pDisp==NULL) {
            return;
        }

        CAcadPopupMenu IPopUpMenu(pDisp);
        if (IPopUpMenu.get_OnMenuBar())
		{
            VariantInit(&index);
            V_VT(&index) = VT_BSTR;
            V_BSTR(&index) = cstrMenuName.AllocSysString();
            IPopUpMenus.RemoveMenuFromMenuBar(index);
			VariantClear(&index);
        }

        pDisp->Release();
    }
    CATCH(COleDispatchException,e)
    {
		acutPrintf(L"\n卸载下拉菜单有异常.");
        //e->ReportError();
        //e->Delete();
    }

    END_CATCH;
}