#include <windows.h>
#include <math.h>
#include <wingdi.h>
#include <gdiplus.h>
#include <fstream>

#define A_KEY 0x41
#define D_KEY 0x44
#define R_KEY 0x52
#define S_KEY 0x53
#define W_KEY 0x57

#define MENU_ID 33

#define RECT_WIDTH 42
#define RECT_HEIGHT 42
#define OFFSET 3

using namespace std;

TCHAR fileName[_MAX_PATH];

HDC dc;
HBITMAP image = NULL;
HBRUSH brush;
LOGBRUSH logBrush;
ofstream fout("messageResult.txt");


//protorypes
void KeyDownHandler(HWND hWnd, WPARAM key, LPARAM params);
void MouseWheelHandler(HWND hWnd, WPARAM params, LPARAM position);
void DrawPicture(HWND hWnd, INT16 x, INT16 y, bool delta = false);
void DrawRect(HWND hWnd, INT16 x, INT16 y, bool delta = false);
void DrawImage(HWND hWnd, INT16 x, INT16 y, bool delta = false);
void Rotate(INT16 x, INT16 y, HWND hWnd, HBITMAP hBitmap);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
	WPARAM wParam, LPARAM lParam);


//CODE



void KeyDownHandler(HWND hWnd, WPARAM key, LPARAM params) {
	switch (key) {
	case W_KEY:
		DrawPicture(hWnd, 0, -OFFSET, true);
		break;
	case S_KEY:
		DrawPicture(hWnd, 0, OFFSET, true);
		break;
	case A_KEY:
		DrawPicture(hWnd, -OFFSET, 0, true);
		break;
	case D_KEY:
		DrawPicture(hWnd, OFFSET, 0, true);
		break;
	case R_KEY:
		DrawPicture(hWnd, 0, 0, true);
		break;
	case VK_ESCAPE:
		if (MessageBox(hWnd, "Do you want to exit?", "Exit", MB_YESNO) == IDYES)
			PostMessage(hWnd, WM_DESTROY, 0, 0);
		break;
	}
}




void MouseWheelHandler(HWND hWnd, WPARAM params, LPARAM position) {
	int wheelDelta = GET_WHEEL_DELTA_WPARAM(params);
	if (params & MK_SHIFT)
		DrawPicture(hWnd, -wheelDelta, 0, true);
	else
		DrawPicture(hWnd, 0, wheelDelta, true);
}





void DrawPicture(HWND hWnd, INT16 x, INT16 y, bool delta) {
	if (image == NULL)
		DrawRect(hWnd, x, y, delta);
	else
		DrawImage(hWnd, x, y, delta);
}





void DrawRect(HWND hWnd, INT16 x, INT16 y, bool delta) {
	static RECT oldRect = { 0, 0, 0, 0 };
	InvalidateRect(hWnd, &oldRect, true);
	UpdateWindow(hWnd);
	if (delta) {
		RECT border;
		GetClientRect(hWnd, &border);
		x = oldRect.left + x;
		y = oldRect.top + y;
		if (x < 0 || y < 0 || (x + RECT_WIDTH) > (border.right - border.left) || (y + RECT_HEIGHT) > (border.bottom - border.top)) {
			x = oldRect.left;
			y = oldRect.top;
		}
	}
	oldRect = { x, y, x + RECT_WIDTH, y + RECT_HEIGHT };
	Rectangle(dc, oldRect.left, oldRect.top, oldRect.right, oldRect.bottom);
}





void DrawImage(HWND hWnd, INT16 x, INT16 y, bool delta) {
	static const int diag = (int)sqrt(pow(RECT_WIDTH, 2) + pow(RECT_HEIGHT, 2));
	static int oldx = 0, oldy = 0;
	static int dx = 0, dy = 0;
	RECT rect = { oldx - diag, oldy - diag, oldx + diag, oldy + diag };
	InvalidateRect(hWnd, &rect, true);
	UpdateWindow(hWnd);
	if (delta) {
		RECT border;
		GetClientRect(hWnd, &border);
		x = oldx + x;
		y = oldy + y;
		if (x < 0 || y < 0 || (x + RECT_WIDTH) > (border.right - border.left) || (y + RECT_HEIGHT) > (border.bottom - border.top)) {
			x = oldx;
			y = oldy;
		}
	}
	dx += x - oldx;
	dy += y - oldy;
	Rotate(x, y, hWnd, image);
	oldx = x;
	oldy = y;
}





void Rotate(INT16 x, INT16 y, HWND hWnd, HBITMAP hBitmap)
{
	static float radians = 0;
	// Create a memory DC compatible with the display
	HDC sourceDC, destDC;
	destDC = GetDC(hWnd);
	sourceDC = CreateCompatibleDC(destDC);

	// Get logical coordinates
	BITMAP bm;
	GetObject(hBitmap, sizeof(bm), &bm);

	float cosine = (float)cos(radians);
	float sine = (float)sin(radians);

	HBITMAP hbmResult = CreateCompatibleBitmap(destDC, bm.bmWidth, bm.bmHeight);

	HBITMAP hbmOldSource = (HBITMAP)SelectObject(sourceDC, hBitmap);
	HBITMAP hbmOldDest = (HBITMAP)SelectObject(destDC, hbmResult);

	SetGraphicsMode(destDC, GM_ADVANCED);
	XFORM xform;
	xform.eM11 = cosine;
	xform.eM12 = -sine;
	xform.eM21 = sine;
	xform.eM22 = cosine;
	xform.eDx = (float)x;
	xform.eDy = (float)y;

	SetWorldTransform(destDC, &xform);

	StretchBlt(destDC, 0, 0, RECT_WIDTH, RECT_HEIGHT, sourceDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

	SelectObject(sourceDC, hbmOldSource);
	SelectObject(destDC, hbmOldDest);
	DeleteDC(sourceDC);
	ReleaseDC(hWnd, destDC);

	radians += 0.0174;
}



//params                 wind handle    msg
LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
	//   additional msg info
	WPARAM wParam, LPARAM lParam)
{
	char tmp[10] = "";
	RECT messageWritingBlock = {};
	PAINTSTRUCT ps;
	HDC hdc;

	//writing to file
	_itoa_s(message, tmp, 10, 10);
	fout << message;
	fout << "\n";


	switch (message)
	{
	case WM_KEYDOWN:
		
		hdc = BeginPaint(hWnd, &ps);

		messageWritingBlock.right = 100;
		messageWritingBlock.top = 50;
		messageWritingBlock.left = 300;
		messageWritingBlock.bottom = 30;
		
		DrawText(hdc, tmp, 10, &messageWritingBlock,
			DT_WORDBREAK | DT_EDITCONTROL | DT_CENTER);

		ReleaseDC(hWnd, hdc);
		EndPaint(hWnd, &ps);


		KeyDownHandler(hWnd, wParam, lParam);

		break;
	case WM_MOUSEMOVE:
		DrawPicture(hWnd, lParam, lParam >> 16);
		break;
	case WM_MOUSEWHEEL:
		MouseWheelHandler(hWnd, wParam, lParam);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == MENU_ID) {
			OPENFILENAME openFile;
			ZeroMemory(&openFile, sizeof(openFile));

			openFile.lStructSize = sizeof(openFile);
			openFile.hwndOwner = hWnd;
			openFile.lpstrFile = (LPSTR)fileName;
			openFile.nMaxFile = sizeof(fileName);
			openFile.lpstrFilter = "Images\0*.bmp;*.png\0\0";
			openFile.nFilterIndex = 1;
			openFile.lpstrFileTitle = NULL;
			openFile.nMaxFileTitle = NULL;
			openFile.lpstrInitialDir = NULL;
			openFile.lpstrTitle = "SelectFile";
			openFile.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			openFile.hInstance = 0;
			if (GetOpenFileNameA(&openFile)) {

				int fileNameLength = MultiByteToWideChar(CP_ACP, 0, fileName, -1, NULL, 0);
				WCHAR *wideCharFileName = new WCHAR[MultiByteToWideChar(CP_ACP, 0, fileName, -1, NULL, 0)];
				MultiByteToWideChar(CP_ACP, 0, fileName, -1, wideCharFileName, fileNameLength);

				Gdiplus::GdiplusStartupInput gdiplusStartupInput;
				ULONG_PTR gdiplusToken;
				GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

				Gdiplus::Bitmap *sourceImage = Gdiplus::Bitmap::FromFile(wideCharFileName);
				HBITMAP hBitmap;
				Gdiplus::Color imageBackgroundColor;
				imageBackgroundColor.SetFromCOLORREF(COLOR_WINDOW + 1);
				Gdiplus::Status bitmapStatus = sourceImage->GetHBITMAP(imageBackgroundColor, &hBitmap);

				Gdiplus::GdiplusShutdown(gdiplusToken);
				delete wideCharFileName;
				image = hBitmap;
			}
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		exit(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}




//params             application descriptor
int APIENTRY WinMain(HINSTANCE hInstance,
	//previous app descriptor     cmd pointer    window style
	HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wcex; HWND hWnd; MSG msg;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_DBLCLKS;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = NULL;
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = 0;
	wcex.lpszClassName = "ImajeMovingClass";
	wcex.hIconSm = wcex.hIcon;
	RegisterClassEx(&wcex);

	hWnd = CreateWindow("ImajeMovingClass", "OSaSP-Lab#1",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0,
		CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	HMENU menu = CreateMenu();
	AppendMenu(menu, 0, MENU_ID, "Load Image");
	SetMenu(hWnd, menu);

	dc = GetDC(hWnd);

	logBrush.lbStyle = BS_SOLID;
	logBrush.lbHatch = NULL;
	logBrush.lbColor = RGB(255, 255, 0);
	brush = CreateBrushIndirect(&logBrush);
	SelectObject(dc, brush);

	while (GetMessage(&msg, hWnd, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	fout.close();
	return (int)msg.wParam;
}