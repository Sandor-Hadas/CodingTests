#include <Windows.h>
#include <cstdio>

#include "BmpReader.h"
#include "ImageThread.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

HWND CreateAndShowWindow(int width, int height);
LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BmpReader readers[4];
ImageThread* threads[4];
HDC backDC;
HBITMAP backBitmap;
RECT viewports[4];
HWND hwndMain;
std::vector<BmpReader> reader;

const std::string file0 = "v0.dat";
const std::string file1 = "v1.dat";
const std::string file2 = "v2.dat";
const std::string file3 = "v3.dat";

void InitDrawing();

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
	hwndMain = CreateAndShowWindow(WINDOW_WIDTH, WINDOW_HEIGHT);
	MessageBox(hwndMain, L"by Sandor Hadas\n\nPlease refer enable the console if you have any issue running this application to see debug output.\nPut the executable where .dat files are located.\nEmail me if you encountered any problem. ;)", L"Honeywell test code", MB_OK | MB_ICONINFORMATION);

	InitDrawing();

	MSG msg = { 0 };

	while (msg.message != WM_QUIT) 
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	DestroyWindow(hwndMain);

	return EXIT_SUCCESS;
}

#pragma region Boilerplate Window code

// Debug
void attachConsole() {
	return;

	AllocConsole();

	FILE* fpOut;
	FILE* fpErr;
	FILE* fpIn;

	freopen_s(&fpOut, "CONOUT$", "w", stdout);
	freopen_s(&fpErr, "CONOUT$", "w", stderr);
	freopen_s(&fpIn, "CONIN$", "r", stdin);

	printf("Console attached successfully.\n");
}


LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) 
	{
		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			EndPaint(hwnd, &ps);
			return 0;
		}
		//case WM_ERASEBKGND:
		//	return 1;
		case WM_KEYDOWN:
			if (wParam == VK_F5)
			{
				for (int i = 0; i < 4; i++)
					threads[i]->signalRestart();
			}
			return 0;
		case WM_CREATE:
			attachConsole(); // For debug output
			return 0;

		case WM_CLOSE:
			for (int i = 0; i < 4; i++)
				threads[i]->stop();
			PostQuitMessage(0);
			return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

HWND CreateAndShowWindow(int width, int height)
{
	double r = max(width / WINDOW_WIDTH, height / WINDOW_HEIGHT);
	if (r > 1.0) {
		width = (int)(width / r);
		height = (int)(height / r);
	}

	LPCWSTR windowFrameName = L"Sample Application";
	WNDCLASS wndclass;

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = (HINSTANCE)GetModuleHandle(NULL);
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = NULL;// (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = windowFrameName;

	RegisterClass(&wndclass);

	RECT rc{
		(GetSystemMetrics(SM_CXSCREEN) - width) / 2,
		(GetSystemMetrics(SM_CYSCREEN) - height) / 2,
		(GetSystemMetrics(SM_CXSCREEN) + width) / 2,
		(GetSystemMetrics(SM_CYSCREEN) + height) / 2
	};

	DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

	AdjustWindowRect(&rc, dwStyle, FALSE);

	HWND hwndMain = CreateWindow(windowFrameName, windowFrameName, dwStyle,
		rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
		NULL, NULL, wndclass.hInstance, NULL);

	if (hwndMain == NULL) {
		printf("CreateWindow failed\n");
		exit(-1);
	}

	ShowWindow(hwndMain, SW_SHOW);
	UpdateWindow(hwndMain);

	return hwndMain;
}

void InitDrawing() {
	while (!IsWindow(hwndMain)) {
		Sleep(10);
	}

	HDC hdc = GetDC(hwndMain);
	backDC = CreateCompatibleDC(hdc);
	backBitmap = CreateCompatibleBitmap(hdc, WINDOW_WIDTH, WINDOW_HEIGHT);
	SelectObject(backDC, backBitmap);
	ReleaseDC(hwndMain, hdc);

	if (!IsWindow(hwndMain)) {
		printf("hwndMain is not a valid window!\n");
	}

	RECT client;
	GetClientRect(hwndMain, &client);
	printf("Client: %ld x %ld\n", client.right, client.bottom);
	//exit(0);

	// Define viewports
	viewports[0] = { 0, 0, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 };
	viewports[1] = { WINDOW_WIDTH / 2, 0, WINDOW_WIDTH, WINDOW_HEIGHT / 2 };
	viewports[2] = { 0, WINDOW_HEIGHT / 2, WINDOW_WIDTH / 2, WINDOW_HEIGHT };
	viewports[3] = { WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, WINDOW_WIDTH, WINDOW_HEIGHT };

	// Init readers and threads
	readers[0].init(file0);
	readers[1].init(file1);
	readers[2].init(file2);
	readers[3].init(file3);

	for (int i = 0; i < 4; i++)
		threads[i] = new ImageThread(&readers[i], viewports[i], backDC, hwndMain);

	for (int i = 0; i < 4; i++)
		threads[i]->start();
}

#pragma endregion