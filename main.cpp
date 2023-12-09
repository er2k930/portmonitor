#include <Windows.h>
#include <string>
#include "resource.h"
#include "descriptions.h"
#include "communication.h"

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR args, int ncmdshow)
{
	WNDCLASS MainClass = NewWindowClass((HBRUSH)COLOR_WINDOW, LoadCursor(NULL, IDC_ARROW), hInst, LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1)), L"MainWindClass", MainProcedure);

	if (!RegisterClass(&MainClass)) return -1;

	MSG MainMessage = { 0 };
	
	CreateWindow(L"MainWindClass", L"Port Monitor", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, 500, 300, NULL, NULL, NULL, NULL);

	while (GetMessage(&MainMessage, NULL, NULL, NULL)) {
		TranslateMessage(&MainMessage);
		DispatchMessage(&MainMessage);
	}

	return 0;
}

WNDCLASS NewWindowClass(HBRUSH BGColor, HCURSOR cursor, HINSTANCE hInst, HICON icon, LPCWSTR name, WNDPROC procedure)
{
	WNDCLASS NWC = { 0 };
	NWC.hCursor = cursor;
	NWC.hIcon = icon;
	NWC.hInstance = hInst;
	NWC.hbrBackground = BGColor;
	NWC.lpszClassName = name;
	NWC.lpfnWndProc = procedure;

	return NWC;
}

LRESULT CALLBACK MainProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) 
	{
	case WM_COMMAND:

		if ((wp >= ComSelectIndex) && (wp < ComSelectIndex + ComAmount))
		{
			selectedPort = wp - ComSelectIndex;
			SetWindowStatus("Выбран порт: " + std::to_string(selectedPort));
			SerialUpdate();
			break;
		}

		switch (wp) 
		{
		case OnClearField:
			SetWindowTextA(hEditControl, "");
			break;
		case OnReadField:
			CharsRead = GetWindowTextA(hParseControl, buffer2, BUFFER);
			SerialWrite(buffer2, CharsRead);
			break;
		case OnSaveFile:
			if (GetSaveFileNameA(&ofn))
			{
				SaveData(filename);
			}
			break;
		case OnLoadFile:
			if (GetSaveFileNameA(&ofn))
			{
				LoadData(filename);
			}
			break;
		case OnConnectRequest:
			ConnectRequest();
			break;
		case OnSerialRefresh:
			SerialUpdate();
			break;
		case OnExit:
			ExitApp();
			break;
		default: break;
		}
		break;

	case WM_CREATE:
		AddMenus(hWnd);
		AddWidgets(hWnd);
		SetOpenFileParams(hWnd);
		SerialUpdate();
		ReadThread = CreateThread(NULL, 0, SerialRead, NULL, 0, NULL);
		break;

	case WM_DESTROY:
		ExitApp();
		break;

	default: return DefWindowProc(hWnd, msg, wp, lp);
	}
}

void AddMenus(HWND hWnd)
{
	HMENU RootMenu = CreateMenu();
	HMENU SubMenu = CreateMenu();

	ComSubMenu = CreateMenu();
	ComListMenu = CreateMenu();

	AppendMenu(RootMenu, MF_POPUP, (UINT_PTR)SubMenu, L"Файл");
	AppendMenu(RootMenu, MF_POPUP, (UINT_PTR)ComSubMenu, L"Подключение");
	AppendMenu(RootMenu, MF_STRING, (UINT_PTR)SubMenu, L"Помощь");

	AppendMenu(SubMenu, MF_STRING, OnSaveFile, L"Сохранить в файл");
	AppendMenu(SubMenu, MF_STRING, OnLoadFile, L"Загрузить из файла");
	
	AppendMenu(ComSubMenu, MF_STRING, OnConnectRequest, L"Подключиться");
	AppendMenu(ComSubMenu, MF_STRING, OnSerialRefresh, L"Обновить порты");
	AppendMenu(ComSubMenu, MF_POPUP, (UINT_PTR)ComListMenu, L"Выбранный порт");

	SetMenu(hWnd, RootMenu);
}

void AddWidgets(HWND hWnd)
{
	hStaticControl = CreateWindowA("static", "Монитор СОМ-порта", WS_VISIBLE | WS_CHILD, 250, 5, 150, 35, hWnd, NULL, NULL, NULL);
	hEditControl = CreateWindowA("edit", "Вывод с МК / ввод для сохранения в файл", WS_VISIBLE | WS_CHILD | ES_MULTILINE | WS_VSCROLL, 5, 40, 475, 120, hWnd, NULL, NULL, NULL);
	hParseControl = CreateWindowA("edit", "Ввод для отправки на МК", WS_VISIBLE | WS_CHILD, 5, 170, 475, 50, hWnd, NULL, NULL, NULL);
	CreateWindowA("button", "Очистить", WS_VISIBLE | WS_CHILD | ES_CENTER, 5, 5, 80, 30, hWnd, (HMENU)OnClearField, NULL, NULL);
	CreateWindowA("button", "Отправить", WS_VISIBLE | WS_CHILD | ES_CENTER, 90, 5, 80, 30, hWnd, (HMENU)OnReadField, NULL, NULL);
}

void SaveData(LPCSTR path)
{
	HANDLE FileToSave = CreateFileA(
		path, 
		GENERIC_WRITE, 
		0, 
		NULL, 
		CREATE_ALWAYS, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL);

	int length = GetWindowTextLength(hEditControl) + 1;
	char* data = new char[length];

	length = GetWindowTextA(hEditControl, data, length);

	DWORD bytesWritten;
	WriteFile(FileToSave, data, length, &bytesWritten, NULL);

	CloseHandle(FileToSave);
	delete[] data;
}

void LoadData(LPCSTR path)
{
	HANDLE FileToLoad = CreateFileA(
		path, 
		GENERIC_READ, 
		0, 
		NULL, 
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL);

	DWORD bytesRead;
	ReadFile(FileToLoad, buffer, BUFFER, &bytesRead, NULL);

	SetWindowTextA(hEditControl, buffer);
	CloseHandle(FileToLoad);
}

void SetOpenFileParams(HWND hWnd)
{
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = sizeof(filename);
	ofn.lpstrFilter = "*.txt";
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
}

void SetWindowStatus(std::string status)
{
	SetWindowTextA(hStaticControl, status.c_str());
}

void ExitApp(void) 
{
	isConnected = false;
	isThreading = false;
	CloseHandle(ConnectedPort);
	ExitThread(0);
	CloseHandle(ReadThread);
	PostQuitMessage(0);
}