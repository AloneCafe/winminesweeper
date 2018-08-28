#include <windows.h>
#include <tchar.h>

LPCTSTR lpszProgName = TEXT("全自动扫雷器");
TCHAR szBuff[500];

void Move(HWND hwnd, DWORD i, DWORD j) {
	RECT rectWin;
	DWORD cxPos, cyPos;
	GetWindowRect(hwnd, &rectWin);
	cxPos = rectWin.left + 22 + j * 16;
	cyPos = rectWin.top + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYMENU) + 63 + i * 16;
	SetCursorPos(cxPos, cyPos);
}

void Assert(DWORD nRet) {
	if (!nRet) {
		SwitchToThisWindow(0, TRUE);
		MessageBox(NULL, TEXT("读取宿主内存出错，程序被迫终止"), lpszProgName, MB_OK | MB_ICONERROR);;
		exit(1);
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	HWND hwnd;
	DWORD pID;
	HANDLE hProc;
	DWORD nMines = 0, nSweepedMines = 0, nWidth = 0, nHeight = 0, i, j;
	DWORD lpAddr = 0x1005361;
	BYTE bData;
	POINT ptOld;

	if ((hwnd = FindWindow(NULL, TEXT("扫雷"))) == NULL) {
		MessageBox(NULL, TEXT("获取窗口句柄失败，请检查扫雷程序是否已运行"), lpszProgName, MB_ICONERROR | MB_OK);
		return 0;
	}

	GetWindowThreadProcessId(hwnd, &pID);
	if ((hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID)) == NULL) {
		MessageBox(NULL, TEXT("扫雷程序没有运行，访问进程失败"), lpszProgName, MB_ICONERROR | MB_OK);
		return 0;
	}

	Assert(ReadProcessMemory(hProc, (LPCVOID)0x1005330, &nMines, sizeof(nMines), NULL));
	Assert(ReadProcessMemory(hProc, (LPCVOID)0x1005334, &nWidth, sizeof(nWidth), NULL));
	Assert(ReadProcessMemory(hProc, (LPCVOID)0x1005338, &nHeight, sizeof(nHeight), NULL));

	//StringCchPrintf(szBuff, sizeof(szBuff) / sizeof(TCHAR), TEXT("地雷个数：%d，宽：%d，高：%d\n\n点击“确定”按钮开始自动扫雷！"), nMines, nWidth, nHeight);
	//MessageBox(NULL, szBuff, lpszProgName, MB_OK | MB_ICONINFORMATION);

	GetCursorPos(&ptOld);

	for (i = 0; i < nHeight; i++) {
		for (j = 0; j < nWidth; j++) {
			Assert(ReadProcessMemory(hProc, (LPCVOID)0x1005330, &nMines, sizeof(nMines), NULL));
			Assert(ReadProcessMemory(hProc, (LPVOID)(lpAddr + (i * 0x20) + j), &bData, sizeof(bData), NULL));
			//StringCchPrintf(szBuff, sizeof(szBuff) / sizeof(TCHAR), TEXT("扫雷（全自动扫雷中 - 剩余地雷个数：%d）"), nMines - nSweepedMines);
			_stprintf(szBuff, TEXT("扫雷（全自动扫雷中 - 剩余地雷个数：%d）"), nMines - nSweepedMines);
			SetWindowText(hwnd, szBuff);
			SwitchToThisWindow(hwnd, TRUE);

			if (bData == 0x8F) {
				// promise that mouse_event() is already executed.
				do {
					Move(hwnd, i, j);
					mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, NULL);
					Sleep(1);
					mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, NULL);
					Assert(ReadProcessMemory(hProc, (LPVOID)(lpAddr + (i * 0x20) + j), &bData, sizeof(bData), NULL));
				} while (bData == 0x8F);
				nSweepedMines += 1;

			} else if ((bData & 0xF0) == 0x40 || (bData & 0xF0) == 0x80) {
				continue;
			} else {
				// promise that mouse_event() is already executed.
				do {
					Move(hwnd, i, j);
					mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, NULL);
					Sleep(1);
					mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, NULL);
					Assert(ReadProcessMemory(hProc, (LPVOID)(lpAddr + (i * 0x20) + j), &bData, sizeof(bData), NULL));
				} while (bData == 0x0F);
			}
		}
	}

	MessageBox(hwnd, TEXT("全自动扫描完毕！\n\n点击“确定”按钮关闭"), lpszProgName, MB_OK | MB_ICONINFORMATION);
	SetWindowText(hwnd, TEXT("扫雷"));
	SetCursorPos(ptOld.x, ptOld.y);
	InvalidateRect(hwnd, NULL, TRUE);
	UpdateWindow(hwnd);
	return 0;
}
