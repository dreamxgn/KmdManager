//#undef UNICODE
#define UNICODE
#define _NO_CRT_STDIO_INLINE


#pragma comment(linker, "/ENTRY:main")
#pragma comment(linker, "/SUBSYSTEM:CONSOLE")
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <shlwapi.h>

#include "svc.c"
#include "res\\res.h"
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "ucrt.lib")
#pragma comment(lib, "msvcrt.lib")
#pragma comment(lib, "libvcruntime.lib")

HMODULE hInstance = 0;

HWND g_hw_edit_path = 0;
HWND g_hw_edit_control = 0;
HWND g_hw_btn_regiser = 0;
HWND g_hw_btn_run = 0;
HWND g_hw_btn_control = 0;
HWND g_hw_btn_stop = 0;
HWND g_hw_btn_unregiser = 0;
HWND g_hw_check_reg_run = 0;
HWND g_hw_check_unreg_stop = 0;
HWND g_hw_check_all = 0;
HWND g_hw_list = 0;

DWORD g_width = 0;
DWORD g_height = 0;
DWORD g_height2 = 0;

WNDPROC lpPrevWndFunc;
HMENU g_hMenu;

LRESULT  list_additem(WCHAR *sys_name, WCHAR *operation, WCHAR *status, WCHAR *last_error);

int  dlg_command(HWND hWnd, int a2, WPARAM wParam, LPARAM lParam)
{
	int result;					// eax
	int dwIoControlCode;		// [esp+0h] [ebp-268h]
	WCHAR szCode[32] = {0};		// [esp+4h] [ebp-264h] BYREF
	WCHAR szSvcName[260] = {0};	// [esp+14h] [ebp-254h] BYREF
	WCHAR szSysPath[260] = {0}; // [esp+118h] [ebp-150h] BYREF
	OPENFILENAME op = {0};		// [esp+21Ch] [ebp-4Ch] OVERLAPPED BYREF

	result = wParam;
	switch (wParam)
	{
	case WM_DESTROY: //窗口被销毁
		result = MessageBox(hWnd, L"确定要退出吗？", L"退出确认", 0x24u);
		if (result != 6)
			return result;
		return EndDialog(hWnd, 0);
		break;

	case CL_BTN_EXIT: //退出按钮
		return EndDialog(hWnd, 0);
		break;

	case CL_BTN_OPTION: //选项按钮
		MessageBox(hWnd, L"选项按钮", L"选项", 0x40u);
		break;

	case CL_BTN_ABOUT: //关于按钮
		MessageBox(hWnd, L"服务于内核驱动管理器", L"关于", 0x40u);
		break;

	case CL_BTN_SELECT: //打开对话框 选择驱动按钮 ...
		op.lStructSize = sizeof(op);
		op.hwndOwner = hWnd;
		op.hInstance = hInstance;
		op.lpstrFilter = L"SYS EXE DLL\0*.sys;*.exe;*.dll\0All Files\0*.*\0";
		op.lpstrFile = szSysPath;
		op.nMaxFile = 260*sizeof(wchar_t);
		GetCurrentDirectory(op.nMaxFile, szSvcName);
		op.lpstrInitialDir = szSvcName;
		op.lpstrTitle = L"选择驱动程序";
		op.Flags = 2627584;
		result = GetOpenFileName(&op);
		if (result)
			result = SetWindowText(g_hw_edit_path, op.lpstrFile);
		return result;
	case CL_BTN_REG: //注册按钮

		if (GetWindowText(g_hw_edit_path, szSysPath, sizeof(szSysPath) - 1))
		{
			
			result = is_sys(szSysPath, szSvcName);
		
			if (result == 1)
			{
				sys_load(szSvcName, szSysPath);
				result = SendMessage(g_hw_check_reg_run, 0xF0u, 0, 0);
				if (result == 1)
					result = sys_start(szSvcName);
			}
			return result;
		}
		return MessageBox(hWnd, L"输入完整的驱动程序路径.", L"提示", 0x10u);

	case CL_UNINSTALL: //卸载按钮
		if (!GetWindowText(g_hw_edit_path, szSysPath, 260))
			return MessageBox(hWnd, L"输入完整的驱动程序路径.", L"提示", 0x10u);
		result = is_sys(szSysPath, szSvcName);
		if (result == 1)
		{
			if (SendMessage(g_hw_check_unreg_stop, 0xF0u, 0, 0) == 1)
				sys_stop(szSvcName);
			result = sys_unload(szSvcName);
		}
		break;

	case CL_BTN_START: //运行按钮

		if (!GetWindowText(g_hw_edit_path, szSysPath, 260))
			return MessageBox(hWnd, L"输入完整的驱动程序路径", L"提示", 0x10u);
		result = is_sys(szSysPath, szSvcName);
		if (result == 1)
			result = sys_start(szSvcName);
		break;

	case CL_BTN_STOP: //停止按钮

		if (!GetWindowText(g_hw_edit_path, szSysPath, 260))
			return MessageBox(hWnd, L"输入完整的驱动程序路径", L"提示", 0x10u);
		result = is_sys(szSysPath, szSvcName);
		if (result == 1)
			result = sys_stop(szSvcName);
		break;

	case CL_BTN_IOCOL: //IO控制按钮

		if (!GetWindowText(g_hw_edit_path, szSysPath, 260))
			return MessageBox(hWnd, L"输入完整的驱动程序路径",  L"提示", 0x10u);
		result = is_sys(szSysPath, szSvcName);
		if (result == 1)
		{
			if (GetWindowText(g_hw_edit_control, szCode, 32))
			{

				wchar_t *ptr;
				dwIoControlCode = wcstol(szCode, &ptr, 16);
				if (SendMessage(g_hw_check_all, 0xF0u, 0, 0) == 1)
				{
					result = sys_load(szSvcName, szSysPath);
					if (result)
					{
						result = sys_start(szSvcName);
						if (result)
						{
							result = sys_ccode(szSvcName, dwIoControlCode);
							if (result)
							{
								result = sys_stop(szSvcName);
								if (result)
									result = sys_unload(szSvcName);
							}
						}
					}
				}
				else
				{
					result = sys_ccode(szSvcName, dwIoControlCode);
				}
			}
			else
			{
				result = MessageBox(hWnd, L"首先要输入控制代码", L"提示", 0x10u);
			}
		}
		break;

	case CL_CHK_REG_START: //检查框 注册+启动
		if (SendDlgItemMessage(hWnd, 1010, 0xF0u, 0, 0) == 1)
		{
			EnableWindow(g_hw_btn_run, 0);
			result = SetWindowText(g_hw_btn_regiser, L"注册+启动");
		}
		else
		{
			EnableWindow(g_hw_btn_run, 1);
			result = SetWindowText(g_hw_btn_regiser, L"注册");
		}
		break;

	case CL_CHK_STOP_UN: //检查框 停止+卸载
		if (SendDlgItemMessage(hWnd, 1011, 0xF0u, 0, 0) == 1)
		{
			EnableWindow(g_hw_btn_stop, 0);
			result = SetWindowText(g_hw_btn_unregiser, L"停止+卸载");
		}
		else
		{
			EnableWindow(g_hw_btn_stop, 1);
			result = SetWindowText(g_hw_btn_unregiser, L"卸载");
		}
		break;

	case CL_CHK_ALL: //检查框 所有操作
		if (SendDlgItemMessage(hWnd, 1012, 0xF0u, 0, 0) == 1)
		{
			EnableWindow(g_hw_btn_regiser, 0);
			EnableWindow(g_hw_check_reg_run, 0);
			EnableWindow(g_hw_btn_run, 0);
			EnableWindow(g_hw_btn_stop, 0);
			EnableWindow(g_hw_check_unreg_stop, 0);
			EnableWindow(g_hw_btn_unregiser, 0);
			result = SetWindowText(g_hw_btn_control, L"所有操作");
		}
		else
		{
			EnableWindow(g_hw_btn_regiser, 1);
			EnableWindow(g_hw_check_reg_run, 1);
			if (!SendMessage(g_hw_check_reg_run, 0xF0u, 0, 0))
				EnableWindow(g_hw_btn_run, 1);
			if (!SendMessage(g_hw_check_unreg_stop, 0xF0u, 0, 0))
				EnableWindow(g_hw_btn_stop, 1);
			EnableWindow(g_hw_check_unreg_stop, 1);
			EnableWindow(g_hw_btn_unregiser, 1);
			result = SetWindowText(g_hw_btn_control, L"I/O 控制");
		}
		break;
	}
	return result;
}

//通过枚举窗口查找 edit控件并设置 样式
BOOL WINAPI EnumFunc(HWND hWnd, LPARAM a2)
{
	LONG style;			 // [rsp+20h] [rbp-68h]
	int isok;				 // [rsp+24h] [rbp-64h]
	LONG exstyle;			 // [rsp+28h] [rbp-60h]
	WCHAR ClassName[32]; // [rsp+30h] [rbp-58h] BYREF

	isok = 0;
	if (GetClassName(hWnd, ClassName, 32))
	{
		if (!lstrcmpiW(ClassName, L"edit"))
		{
			style = GetWindowLong(hWnd, GWL_STYLE);
			if (style)
			{
				if ((style & 0x800) != 0)
					isok = 1;
			}
		}
	}

	if (a2 == 1 && !isok)
	{
		exstyle = GetWindowLong(hWnd, GWL_EXSTYLE);
		if (exstyle)
			SetWindowLong(hWnd, GWL_EXSTYLE, exstyle & 0xFFFDFFFF);
	}
	return 1;
}

//使用主题
void WINAPI use_theme(HWND hWndParent)
{
	HMODULE v1; // eax
	BOOL(WINAPI * IsAppThemed)
	(); // eax

	v1 = GetModuleHandle(L"UxTheme.dll");
	if (v1)
	{
		IsAppThemed = (void *)GetProcAddress(v1, "IsAppThemed");
		if (IsAppThemed)
		{
			if (IsAppThemed())
				EnumChildWindows(hWndParent, EnumFunc, 1);
			else
				EnumChildWindows(hWndParent, EnumFunc, 0);
		}
	}
}

//列表 添加项目
LRESULT  list_additem(WCHAR *sys_name, WCHAR *operation, WCHAR *status, WCHAR *last_error)
{
	LRESULT index;	 // esi
	WCHAR szText[8]; // [esp-8h] [ebp-54h] BYREF
	//int col; // [esp+0h] [ebp-4Ch]
	//int v8; // [esp+Ch] [ebp-40h]
	LVITEM lv; // [esp+24h] [ebp-28h] OVERLAPPED BYREF

	lv.mask = 1;
	lv.pszText = sys_name;
	lv.iSubItem = 0;
	index = SendMessage(g_hw_list, LVM_GETITEMCOUNT, 0, 0);
	lv.iItem = index;

	SendMessage(g_hw_list, LVM_INSERTITEM, 0, (LPARAM)&lv);
	//col = 1;
	//v8 = info;

	lv.pszText = operation;
	lv.iSubItem = 1;
	SendMessage(g_hw_list, LVM_SETITEMTEXT, index, (LPARAM)&lv);

	//col = 2;
	//v8 = a3;
	lv.pszText = status;
	lv.iSubItem = 2;
	SendMessage(g_hw_list, LVM_SETITEMTEXT, index, (LPARAM)&lv);
	//col = 3;
	//v8 = a4;
	lv.pszText = last_error;
	lv.iSubItem = 3;
	SendMessage(g_hw_list, LVM_SETITEMTEXT, index, (LPARAM)&lv);

	return SendMessage(g_hw_list, LVM_ENSUREVISIBLE, index, 0);
}

//列表 插入标题
LRESULT WINAPI list_insert(HWND hWnd)
{

	LRESULT index = SendMessage(hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, 0x21);
	LVCOLUMN lv = {0};
	lv.mask = 7;
	lv.cx = 60;
	lv.pszText = L"驱动";
	SendMessage(hWnd, LVM_INSERTCOLUMN, 0, (LPARAM)&lv);

	lv.pszText = L"操作";
	SendMessage(hWnd, LVM_INSERTCOLUMN, 1, (LPARAM)&lv);

	lv.pszText = L"状态";
	SendMessage(hWnd, LVM_INSERTCOLUMN, 2, (LPARAM)&lv);

	lv.cx = 400;
	lv.pszText = L"错误";
	return SendMessage(hWnd, LVM_INSERTCOLUMN, 3, (LPARAM)&lv);
}

//列表弹出菜单
LRESULT WINAPI list_msgproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg == WM_CONTEXTMENU)
	{
		if (SendMessage(hWnd, 0x1004u, 0, 0))
		{
			TrackPopupMenu(g_hMenu, 0, (unsigned __int16)lParam, HIWORD(lParam), 0, hWnd, 0); //弹出菜单
		}
			
	}else if (Msg == WM_COMMAND && (unsigned __int16)wParam == 5000)
	{
		SendMessage(hWnd, 0x1009u, 0, 0);
	}
	return CallWindowProc(lpPrevWndFunc, hWnd, Msg, wParam, lParam);
}

//对话框初始化
void  dlg_initdialog(HWND hWnd, int a2, WPARAM wParam, LPARAM lParam)
{
	HICON hIcon;	 // eax
	HWND btn_option; // eax
	RECT Rect;		 // [esp+0h] [ebp-10h] BYREF

	hIcon = LoadIcon(hInstance, (LPCWSTR)CL_ICON); //图标
	SendMessage(hWnd, 128, 1u, (LPARAM)hIcon);
	g_hw_edit_path = GetDlgItem(hWnd, CL_EDIT_PATH);	//获取驱动路径
	g_hw_edit_control = GetDlgItem(hWnd, CL_EDIT_CODE); //获取控制码
	//code_ctlhex(g_hw_edit_control, "0123456789abcdefABCDEF", 1);//只允许输入16进制
	SendMessage(g_hw_edit_control, 197, 8u, 0);

	g_hw_btn_regiser = GetDlgItem(hWnd, CL_BTN_REG);			//注册
	g_hw_btn_run = GetDlgItem(hWnd, CL_BTN_START);				//启动
	g_hw_btn_control = GetDlgItem(hWnd, CL_BTN_IOCOL);			//控制
	g_hw_btn_stop = GetDlgItem(hWnd, CL_BTN_STOP);				//停止
	g_hw_btn_unregiser = GetDlgItem(hWnd, CL_UNINSTALL);		//卸载
	g_hw_check_reg_run = GetDlgItem(hWnd, CL_CHK_REG_START);	//注册+运行
	g_hw_check_unreg_stop = GetDlgItem(hWnd, CL_CHK_STOP_UN); 	//停止+卸载
	g_hw_check_all = GetDlgItem(hWnd, CL_CHK_ALL);				//全选
	g_hw_list = GetDlgItem(hWnd, CL_LIST);						//SysListView32 列表框
	list_insert(g_hw_list);
	lpPrevWndFunc = (WNDPROC)SetWindowLongPtr(g_hw_list, -4, (LONG_PTR)list_msgproc);
	g_hMenu = CreatePopupMenu();
	AppendMenu(g_hMenu, 0, 0x1388u, L"清除日志"); 			//右键菜单
	btn_option = GetDlgItem(hWnd, CL_BTN_OPTION);		   //选项
	EnableWindow(btn_option, 1);
	GetWindowRect(hWnd, &Rect);
	g_width = Rect.right - Rect.left;
	g_height = Rect.bottom - Rect.top;
	g_height2 = Rect.bottom - Rect.top + 195;
	use_theme(hWnd);
}

int  dlg_getminmaxinfo(HWND hwnd, int a2, WPARAM wParam, MINMAXINFO *mi)
{
	mi->ptMinTrackSize.x = g_width;
	mi->ptMinTrackSize.y = g_height;
	mi->ptMaxTrackSize.x = g_width;
	mi->ptMaxTrackSize.y = g_height2;
	return g_height2;
}

//窗口大小被改变事件
BOOL  dlg_size(HWND hWnd, int a2, WPARAM wParam, LPARAM lParam)
{
	POINT Point; // [esp+4h] [ebp-30h] BYREF
	int nWidth;	 // [esp+Ch] [ebp-28h]
	int nHeight; // [esp+10h] [ebp-24h]
	RECT list;	 // [esp+14h] [ebp-20h] BYREF
	RECT Rect;	 // [esp+24h] [ebp-10h] BYREF

	GetWindowRect(hWnd, &Rect);
	GetWindowRect(g_hw_list, &list);
	nHeight = Rect.bottom - 6 - list.top;
	nWidth = list.right - list.left;
	Point.x = list.left;
	Point.y = list.top;
	ScreenToClient(hWnd, &Point);
	return MoveWindow(g_hw_list, Point.x, Point.y, nWidth, nHeight, 1);
}

//接收拖放文件
BOOL  dlg_dropfiles(HWND hWnd, int a2, WPARAM hDrop, LPARAM a4)
{
	WCHAR szFile[260] = {0}; 
	DragQueryFile((HDROP)hDrop, 0, szFile, sizeof(szFile));
	SetWindowText(g_hw_edit_path, szFile); //把文件路径设置到 编辑框控件
	PathRemoveFileSpecW(szFile);		   //移出文件名 只要路径
	return SetCurrentDirectory(szFile);
}


//主窗口消息处理
INT_PTR WINAPI DialogFunc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
		dlg_command(hwnd, 273, wParam, lParam);
		break;
	case WM_INITDIALOG:
		dlg_initdialog(hwnd, 272, wParam, lParam);
		break;
	case WM_GETMINMAXINFO:
		dlg_getminmaxinfo(hwnd, 36, wParam, (MINMAXINFO *)lParam);
		break;
	case WM_SIZE:
		dlg_size(hwnd, 5, wParam, lParam);
		break;
	case WM_DROPFILES:
		dlg_dropfiles(hwnd, 563, wParam, lParam);
		break;
	case WM_CLOSE:
		EndDialog(hwnd, 0);
		break;
	default:
		return 0;
	}
	return 1;
}

int main()
{

	hInstance = GetModuleHandle(0);
	INT_PTR uExitCode = DialogBoxParam(hInstance, (LPCWSTR)CL_DIALOGEX, 0, DialogFunc, 0);
	ExitProcess(uExitCode);

	return 0;
}