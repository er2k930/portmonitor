#pragma once
#ifndef DESCRIPTIONS_H
#define DESCRIPTIONS_H

#define OnClearField 1
#define OnReadField 2
#define OnExit 3
#define OnSaveFile 4
#define OnLoadFile 5
#define OnConnectRequest 6
#define OnSerialRefresh 7
#define ComAmount 50;
#define ComSelectIndex 120;
#define BUFFER 255

char buffer[BUFFER];
char buffer2[BUFFER];
char buffer3[BUFFER];
char COMBUF[32];

int CharsRead;

HWND hEditControl;
HWND hGetControl;
HWND hStaticControl;
HWND hParseControl;

HMENU ComSubMenu;
HMENU ComListMenu;

volatile bool isConnected = false;
volatile bool isThreading = true;
int selectedPort = 1;
int targetBaudRate = 9600;

HANDLE ConnectedPort;
HANDLE ReadThread;

char filename[255];
OPENFILENAMEA ofn;

LRESULT CALLBACK MainProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);

WNDCLASS NewWindowClass(HBRUSH BGColor, HCURSOR cursor, HINSTANCE hInst, HICON icon, LPCWSTR name, WNDPROC procedure);

void AddMenus(HWND hWnd);
void AddWidgets(HWND hWnd);
void SaveData(LPCSTR path);
void LoadData(LPCSTR path);
void SetOpenFileParams(HWND hWnd);
void SetWindowStatus(std::string status);
void ExitApp(void);

#endif 
