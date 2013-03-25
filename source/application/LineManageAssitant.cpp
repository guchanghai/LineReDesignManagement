//
//////////////////////////////////////////////////////////////////////////////
//
//  Copyright 2012 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//
//////////////////////////////////////////////////////////////////////////////
//
// AsdkAcUiSample.cpp : Defines the initialization routines for the DLL.
//
#include "StdAfx.h"

#if defined(_DEBUG) && !defined(AC_FULL_DEBUG)
#error _DEBUG should not be defined except in internal Adesk debug builds
#endif

#include "LineManageAssitant.h"

#include "AsdkAcUiDialogSample.h"
#include "AcExtensionModule.h"

#include "MenuManager.h"

#include "CommandManager.h"

#include <ArxCustomObject.h>

wstring gLmaArxLoadPath;

using namespace com::guch::assistant::arx;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
extern "C" HWND adsw_acadMainWnd();

/////////////////////////////////////////////////////////////////////////////
// Define the sole extension module object.
AC_IMPLEMENT_EXTENSION_MODULE(theArxDLL);

static void initApp(void* appId)
{
  // 注册菜单
  MenuManager::CreateMenu(appId);

  // 注册命令
  CommandManager::instance()->RegisterCommand();

  // 注册自定义类
  LMADbObjectManager::RegisterClass();
}

static void unloadApp()
{
  // 移除默认上下文菜单
  MenuManager::unRegister();

  //删除命令
  CommandManager::instance()->UnRegisterCommand();

  // 注销自定义类
  LMADbObjectManager::RegisterClass();
}

static void dwgLoaded()
{
	//从当前DWG文件中读取管线信息
	//LineEntryFileManager::ReadFromCurrentDWG();
}

/// <summary>
/// Stores the LMA arx path.
/// </summary>
/// <param name="hInstance">The h instance.</param>
static void StoreLMAArxPath( HINSTANCE hInstance )
{
	CString arxPath,acedPath;   //存放路径

	HINSTANCE curdll;//当前DLL的句柄,可在DllMain()函数的传入参数中找到，用一个全局变量保存即可
	curdll = hInstance;

	//第一个参数为NULL时,则得到调用当前DLL文件的可执行程序的路径,为DLL句柄时,就得到DLL文件的路径
	GetModuleFileName(NULL,acedPath.GetBufferSetLength(MAX_PATH+1),MAX_PATH);
	acutPrintf(L"\nAutoCAD的路径是【%s】",acedPath.GetBuffer());

	GetModuleFileName(curdll,arxPath.GetBufferSetLength(MAX_PATH+1),MAX_PATH);
	acutPrintf(L"\n管线设计系统的路径是【%s】",arxPath.GetBuffer());

	arxPath.ReleaseBuffer();   //这里得到的是带名称的路径
	acedPath.ReleaseBuffer();

	int nPos = arxPath.ReverseFind('\\');   
	arxPath = arxPath.Left(nPos);  //获得绝对路径 

	gLmaArxLoadPath = wstring(arxPath.GetBuffer());
	acutPrintf(L"\n存储管线设计系统的路径【%s】",gLmaArxLoadPath.c_str());
}

static void dwgUnLoaded()
{
	acutPrintf(L"\nDWG文件卸载");
	if( LineEntryFileManager::openingDwg )
	{
		acutPrintf(L"\n但当前是打开文件状态，估不做任何处理");
		LineEntryFileManager::openingDwg = false;
	}
	else
	{
		acutPrintf(L"\n删除所有管线配置");
		LineEntryFileManager::RemoveEntryFileOnDWGUnLoad();
	}
}

static void dwgSaved()
{
	acutPrintf(L"\nDWG文件保存");

	LineEntryFileManager::SaveFileEntity();
}

//////////////////////////////////////////////////////////////
//
// Entry points
//
//////////////////////////////////////////////////////////////

static AFX_EXTENSION_MODULE MyAsdkMfcComSampDLL = { NULL, NULL };

/// <summary>
/// DLLs the main.
/// </summary>
/// <param name="hInstance">The h instance.</param>
/// <param name="dwReason">The dw reason.</param>
/// <param name="lpReserved">The lp reserved.</param>
/// <returns></returns>
extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    // Remove this if you use lpReserved
    UNREFERENCED_PARAMETER(lpReserved);
    
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        theArxDLL.AttachInstance(hInstance);

		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(MyAsdkMfcComSampDLL, hInstance))
			return 0;

		// Insert this DLL into the resource chain
		// NOTE: If this Extension DLL is being implicitly linked to by
		//  an MFC Regular DLL (such as an ActiveX Control)
		//  instead of an MFC application, then you will want to
		//  remove this line from DllMain and put it in a separate
		//  function exported from this Extension DLL.  The Regular DLL
		//  that uses this Extension DLL should then explicitly call that
		//  function to initialize this Extension DLL.  Otherwise,
		//  the CDynLinkLibrary object will not be attached to the
		//  Regular DLL's resource chain, and serious problems will
		//  result.

		new CDynLinkLibrary(MyAsdkMfcComSampDLL);

		StoreLMAArxPath( hInstance );
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        theArxDLL.DetachInstance();  
		
		TRACE0("MyAsdkMfcComSamp.DLL Terminating!\n");

		// Terminate the library before destructors are called
		AfxTermExtensionModule(MyAsdkMfcComSampDLL);
    }
    return 1;   // ok
}



extern "C" AcRx::AppRetCode acrxEntryPoint( AcRx::AppMsgCode msg, void* appId)
{
    switch( msg ) 
    {
    case AcRx::kInitAppMsg: 

        acrxDynamicLinker->unlockApplication(appId);
        acrxDynamicLinker->registerAppMDIAware(appId);
        
		initApp(appId); 
        break;

    case AcRx::kUnloadAppMsg: 

        unloadApp(); 
        break;

	case AcRx::kLoadDwgMsg:

		dwgLoaded();
        break;

    case AcRx::kUnloadDwgMsg:

		dwgUnLoaded();
        break;

    case AcRx::kInitDialogMsg:
        break;

	case AcRx::kSaveMsg:
		dwgSaved();
		break;

    default:
        break;
    }
    return AcRx::kRetOK;
}

