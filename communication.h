#pragma once
#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#define ComSelectIndex 120
#define ComAmount 50

int SerialBegin(int baud, int port)
{
	CloseHandle(ConnectedPort);

	ConnectedPort = CreateFileA(
		("\\\\.\\COM" + std::to_string(port)).c_str(),
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (ConnectedPort == INVALID_HANDLE_VALUE) return -4;

	DCB ComDCM;
	memset(&ComDCM, 0, sizeof(ComDCM));
	ComDCM.DCBlength = sizeof(ComDCM);

	if (!GetCommState(ConnectedPort, &ComDCM)) return -3;

	ComDCM.BaudRate = baud;
	ComDCM.ByteSize = 8;
	ComDCM.Parity = NOPARITY;
	ComDCM.StopBits = ONESTOPBIT;
	ComDCM.fAbortOnError = TRUE;
	ComDCM.fDtrControl = DTR_CONTROL_DISABLE;
	ComDCM.fRtsControl = RTS_CONTROL_DISABLE;
	ComDCM.fBinary = TRUE;
	ComDCM.fParity = FALSE;
	ComDCM.fInX = FALSE;
	ComDCM.fOutX = FALSE;
	ComDCM.XonChar = 0;
	ComDCM.XoffChar = (unsigned char)0xFF;
	ComDCM.fErrorChar = FALSE;
	ComDCM.fNull = FALSE;
	ComDCM.fOutxCtsFlow = FALSE;
	ComDCM.fOutxDsrFlow = FALSE;
	ComDCM.XonLim = 128;
	ComDCM.XoffLim = 128;

	if (!SetCommState(ConnectedPort, &ComDCM)) return -2;

	COMMTIMEOUTS SerialTimeouts;
	SerialTimeouts.ReadIntervalTimeout = 1;
	SerialTimeouts.ReadTotalTimeoutConstant = 1;
	SerialTimeouts.ReadTotalTimeoutMultiplier = 1;
	SerialTimeouts.WriteTotalTimeoutConstant = 1;
	SerialTimeouts.WriteTotalTimeoutMultiplier = 1;

	if (!SetCommTimeouts(ConnectedPort, &SerialTimeouts)) return -1;

	SetCommMask(ConnectedPort, EV_RXCHAR);
	PurgeComm(ConnectedPort, PURGE_TXCLEAR | PURGE_RXCLEAR);
	SetupComm(ConnectedPort, 256, 256);

	return 0;
}

void ConnectRequest(void)
{
	if (isConnected)
	{
		CloseHandle(ConnectedPort);
		SetWindowStatus("Устройство отключено");
		isConnected = false;
		return;
	}

	switch (SerialBegin(targetBaudRate, selectedPort)) {
	case -4: SetWindowStatus("Устройство не подключено"); break;
	case -3: SetWindowStatus("Ошибка получения параметров"); break;
	case -2: SetWindowStatus("Ошибка установки параметров"); break;
	case -1: SetWindowStatus("Ошибка установки таймаутов"); break;
	case 0:
		SetWindowStatus("Подключено к: СОМ" + std::to_string(selectedPort));
		isConnected = true;
		return;
	}

}

DWORD WINAPI SerialRead(LPVOID lpParameter)
{
	DWORD BytesIterated;

	while (isThreading) {
		if (!isConnected) continue;
		if (!SetCommMask(ConnectedPort, EV_RXCHAR))
		{
			ConnectRequest();
			continue;
		}
		if (ReadFile(ConnectedPort, COMBUF, sizeof(COMBUF), &BytesIterated, NULL))
		{
			SetWindowTextA(hEditControl, COMBUF);
		}
	}
	return 0;
}


void SerialWrite(char* buf, int length) {
	if (!isConnected) return;
	DWORD BytesIterated;
	WriteFile(ConnectedPort, buf, length, &BytesIterated, NULL);
}

void SerialUpdate(void)
{
	while (RemoveMenu(ComListMenu, 0, MF_BYPOSITION));
	int radioLast = 0;
	int radioCurrent = -1;

	for (int i = 1; i < 50; ++i) 
	{
		HANDLE port = CreateFileA(
			("\\\\.\\COM" + std::to_string(i)).c_str(),
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		if (port != INVALID_HANDLE_VALUE) {
			AppendMenuA(ComListMenu, MF_STRING, ComSelectIndex + i, ("COM" + std::to_string(i)).c_str());
			if (i == selectedPort)
			{
				radioCurrent = radioLast;
			}
			++radioLast;
		}
		CloseHandle(port);
	}
	if (radioLast)
	{
		--radioLast;
	}
	if (radioCurrent != -1)
	{
		CheckMenuItem(ComListMenu, radioCurrent, MF_BYPOSITION | MF_CHECKED);
	}
}

#endif