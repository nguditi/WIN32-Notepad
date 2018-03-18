// Notepad.cpp : Defines the entry point for the application.
//
#include "stdafx.h"
#include "Notepad.h"
#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
POINT GetStartPoint(int , int );
BOOL OpenRead(HWND , WCHAR[260], WCHAR[260]);
BOOL OpenWrite(HWND , WCHAR[260], WCHAR[260]);
BOOL MyWriteFile(HWND , LPWSTR);
BOOL MyReadFile(HWND , LPWSTR);
BOOL FindBox(HWND ,WCHAR[260]);
HFONT MyCreateFont(HWND hWnd, LOGFONT &lf);
HWND g_hwnd;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_NOTEPAD, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_NOTEPAD));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
			TranslateMessage(&msg);
			if (msg.message == WM_KEYDOWN) {
				SendMessage(g_hwnd, WM_KEYDOWN, msg.wParam, msg.lParam);
			}
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_NOTEPAD);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   static POINT center = GetStartPoint(800, 450);
   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME | WS_SIZEBOX ,
      center.x, center.y, 800, 450, nullptr, nullptr, hInstance, nullptr);

   g_hwnd = hWnd;

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

int AskForNew(HWND hWnd, LPWSTR name)
{
	WCHAR buffer[255];
	int res;
	wsprintf(buffer,L"%s có thay đổi, bạn định không lưu ?",name);
	res = MessageBox(hWnd, buffer, L"Chú ý", MB_OKCANCEL | MB_ICONSTOP);
	return res;
}

void ChangeTitle(HWND hWnd, LPWSTR name)
{
	WCHAR buffer[255];
	wsprintf(buffer,L"%s - %s", szTitle, name);
	SetWindowText(hWnd, buffer);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND lblTextZone;
	static bool textHasChange = 0;
	static HFONT font;
	static LOGFONT lf;
	static int isSelect;
	static WCHAR findword[260] = { 0 };
	static WCHAR filepath[260] = { 0 };
	static WCHAR filename[260] = L"New Text Document.txt";


	switch (message)
	{
	case WM_INITMENUPOPUP:
		if (lParam == 1)
		{
			EnableMenuItem((HMENU)wParam, ID_EDIT_PASTE, IsClipboardFormatAvailable((CF_TEXT) ? MF_ENABLED : MF_GRAYED));
			EnableMenuItem((HMENU)wParam, ID_EDIT_UNDO, SendMessage(lblTextZone, EM_CANUNDO, 0, 0) ? MF_ENABLED : MF_GRAYED);
			isSelect = SendMessage(lblTextZone, EM_GETSEL, 0, 0);
			if ((HIWORD(isSelect) != LOWORD(isSelect)))
			{
				EnableMenuItem((HMENU)wParam, ID_EDIT_COPY, MF_ENABLED);
				EnableMenuItem((HMENU)wParam, ID_EDIT_CUT, MF_ENABLED);
			}
			else
			{
				EnableMenuItem((HMENU)wParam, ID_EDIT_COPY, MF_GRAYED);
				EnableMenuItem((HMENU)wParam, ID_EDIT_CUT, MF_GRAYED);
			}
		}
		break;
	case WM_KEYDOWN:
		if(wParam == 'S' && GetAsyncKeyState(VK_CONTROL))
		{
			if (filepath[0] == 0)
				OpenWrite(hWnd, filepath, filename);
			if (MyWriteFile(lblTextZone, filepath))
			{
				MessageBox(hWnd, L"Đã lưu thành công", L"Thông báo", 0);
				textHasChange = 0;
			}
		}
		else if (wParam == 'A' && GetAsyncKeyState(VK_CONTROL))
		{
			SendMessage(lblTextZone, EM_SETSEL, 0, -1);
		}
		else if (wParam == 'O' && GetAsyncKeyState(VK_CONTROL))
		{
			if (textHasChange && IDCANCEL == AskForNew(hWnd, filename))
			{
				return 0;
			}
			else
			{
				if (OpenRead(hWnd, filepath, filename))
				{
					if (!MyReadFile(lblTextZone, filepath))
					{
						MessageBox(hWnd, L"Xảy ra lỗi không xác định", L"Không thể đọc", 0);
						memset(filepath, 0, 260);
						wcscpy(filename, L"New Text Document.txt");
						return 0;
					}
				}
				textHasChange = 0;
				ChangeTitle(hWnd, filename);
			}
		}
		break;
	case WM_SIZE:
		MoveWindow(lblTextZone, 2, 0, LOWORD(lParam)-3, HIWORD(lParam),1);
		break;
	case WM_CREATE:
		//Set font
		wcscpy_s(lf.lfFaceName, L"Consolas");
		lf.lfHeight = 20;
		font = CreateFontIndirect(&lf);
		//Text Zone
		lblTextZone = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE |WS_HSCROLL|WS_VSCROLL|ES_MULTILINE, 0, 0 , 0, 0, hWnd, 0, hInst, 0);
		SendMessage(lblTextZone, WM_SETFONT, WPARAM(font), TRUE);
		//Tiêu đề
		ChangeTitle(hWnd, filename);
		break;
	case WM_SETFOCUS:
		SetFocus(lblTextZone);
		break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
			if (lParam)
			{
				if(HIWORD(wParam) == EN_UPDATE)
					textHasChange = 1;
			}
            // Parse the menu selections:
            switch (wmId)
            {
			case ID_FILE_NEW:
				if (textHasChange && IDCANCEL == AskForNew(hWnd, filename))
				{
					return 0;
				}
				else
				{
					SetWindowText(lblTextZone, L"\0");
					memset(filepath,0,260);
					wcscpy(filename, L"New Text Document.txt");
					textHasChange = 0;
					ChangeTitle(hWnd, filename);
					return 0;
				}
			case ID_FILE_SAVE:
				if (filepath[0] == 0)
					OpenWrite(hWnd, filepath, filename);
				if (MyWriteFile(lblTextZone, filepath))
				{
					MessageBox(hWnd, L"Đã lưu thành công", L"Thông báo", 0);
					textHasChange = 0;
				}
				break;
			case ID_FILE_SAVEAS:
				if (OpenWrite(hWnd, filepath, filename))
				{
					ChangeTitle(hWnd, filename);
					if (MyWriteFile(lblTextZone, filepath))
					{
						MessageBox(hWnd, L"Đã lưu thành công", L"Thông báo", 0);
						textHasChange = 0;
					}
				}
				break;
			case ID_FILE_OPEN:
				if (textHasChange && IDCANCEL == AskForNew(hWnd, filename))
				{
					return 0;
				}
				else
				{
					if (OpenRead(hWnd, filepath, filename))
					{
						if (!MyReadFile(lblTextZone, filepath))
						{
							MessageBox(hWnd, L"Xảy ra lỗi không xác định", L"Không thể đọc", 0);
							memset(filepath, 0, 260);
							wcscpy(filename, L"New Text Document.txt");
							return 0;
						}
					}
					textHasChange = 0;
					ChangeTitle(hWnd, filename);
				}
				break;
			case ID_FORMAT_FONT:
				font = MyCreateFont(hWnd,lf);
				SendMessage(lblTextZone, WM_SETFONT, WPARAM(font), 1);
				break;
			case ID_EDIT_SELECTALL:
				SendMessage(lblTextZone, EM_SETSEL, 0, -1);
				break;
			case ID_EDIT_COPY:
				SendMessage(lblTextZone, WM_COPY, 0, 0);
				break;
			case ID_EDIT_CUT:
				SendMessage(lblTextZone, WM_CUT, 0, 0);
				break;
			case ID_EDIT_PASTE:
				SendMessage(lblTextZone, WM_PASTE, 0, 0);
				break;
			case ID_EDIT_UNDO:
				SendMessage(lblTextZone, WM_UNDO, 0, 0);
				break;
			case ID_ABOUT_AUTHOR:
				MessageBox(hWnd, L"Nguyễn Đình Tiến", L"Info", MB_OK|MB_ICONINFORMATION);
				break;
            case IDM_EXIT:
				SendMessage(hWnd, WM_CLOSE, 0, 0);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
	case WM_CLOSE:
		if (textHasChange && IDCANCEL == AskForNew(hWnd, filename))
		{
			return 0;
		}
		DestroyWindow(hWnd);
		break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


POINT GetStartPoint(int x,int y)
{
	POINT center;
	RECT rect;
	HWND desktop = GetDesktopWindow();
	GetWindowRect(desktop, &rect);
	center.x = (rect.right - x) / 2;
	center.y = (rect.bottom - y) / 2;
	return center;
}

BOOL OpenRead(HWND hWnd, WCHAR filepath[260], WCHAR filename[260])
{
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.nMaxFile = 260;
	ofn.lpstrFilter = L"Text document (*.txt)\0*.txt\0All files (*.*)\0*.*\0";
	ofn.nFilterIndex = 2;
	ofn.lpstrInitialDir = NULL;
	ofn.hInstance = NULL;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nMaxFileTitle = _MAX_FNAME + _MAX_EXT;
	ofn.lpstrTitle = NULL;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = L"txt";
	ofn.lCustData = 0L;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;
	ofn.lpstrFile = filepath;
	ofn.lpstrFileTitle = filename;
	ofn.Flags = OFN_HIDEREADONLY | OFN_CREATEPROMPT;
	return (GetOpenFileName(&ofn));
}

BOOL OpenWrite(HWND hWnd, WCHAR filepath[260], WCHAR filename[260])
{
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.nMaxFile = 260;
	ofn.lpstrFilter = L"Text document (*.txt)\0*.txt\0All files (*.*)\0*.*\0";
	ofn.nFilterIndex = 2;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
	ofn.hInstance = NULL;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.lpstrFile = filepath;
	ofn.lpstrFileTitle = filename;
	ofn.nMaxFileTitle = _MAX_FNAME + _MAX_EXT;
	ofn.lpstrTitle = NULL;         
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = L"txt";
	ofn.lCustData = 0L;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;
	return(GetSaveFileName(&ofn));
}

BOOL MyWriteFile(HWND lbltext, LPWSTR filepath)
{
	HANDLE hfile = CreateFile(filepath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);
	if (hfile != INVALID_HANDLE_VALUE)
	{
		DWORD size;
		size = GetWindowTextLength(lbltext)+1;
		if (IDYES == MessageBox(0, L"Ghi dạng UNICODE ?", L"Định dạng ghi", MB_YESNO))
		{
			WCHAR* lpBuffer = new WCHAR[size];
			GetWindowText(lbltext, lpBuffer, size);
			DWORD dwBytesWrite = 0;
			WriteFile(hfile, lpBuffer, 2 * size, &dwBytesWrite, NULL);
			CloseHandle(hfile);
			DeleteObject(hfile);
			delete lpBuffer;
		}
		else
		{
			CHAR* lpBuffer = new CHAR[size];
			GetWindowTextA(lbltext, lpBuffer, size);
			DWORD dwBytesWrite = 0;
			WriteFile(hfile, lpBuffer, size, &dwBytesWrite, NULL);
			CloseHandle(hfile);
			DeleteObject(hfile);
			delete lpBuffer;
		}
		return 1;
	}
	return 0;
}
BOOL MyReadFile(HWND lbltext, LPWSTR filepath)
{
	HANDLE hfile = CreateFile(filepath,GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hfile != INVALID_HANDLE_VALUE)
	{
		DWORD size;
		size = GetFileSize(hfile, 0);
		if (IDYES == MessageBox(0, L"Đọc dạng UNICODE ?", L"Định dạng đọc", MB_YESNO))
		{
			WCHAR* lpBuffer = new WCHAR[size / 2 + 1];
			memset(lpBuffer, 0, size / 2 + 1);
			DWORD dwBytesRead = 0;
			ReadFile(hfile, lpBuffer, size, &dwBytesRead, NULL);
			CloseHandle(hfile);;
			SetWindowText(lbltext, lpBuffer);
			delete lpBuffer;
		}
		else
		{	
			CHAR* lpBuffer = new CHAR[size+1];
			memset(lpBuffer, 0, size + 1);
			DWORD dwBytesRead = 0;
			ReadFile(hfile, lpBuffer, size, &dwBytesRead, NULL);
			CloseHandle(hfile);;
			SetWindowTextA(lbltext, lpBuffer);
			delete lpBuffer;
		}
		return 1;
	}
	return 0;
}

HFONT MyCreateFont(HWND hWnd, LOGFONT &lf)
{
	CHOOSEFONT cf;
	ZeroMemory(&cf, sizeof(cf));
	cf.lStructSize = sizeof(cf);
	cf.hwndOwner = hWnd;
	cf.lpLogFont = &lf;
	cf.rgbColors = RGB(0, 0, 0);
	cf.Flags = CF_SCREENFONTS | CF_EFFECTS| CF_INITTOLOGFONTSTRUCT;
	ChooseFont(&cf);
	return  CreateFontIndirect(cf.lpLogFont);
}