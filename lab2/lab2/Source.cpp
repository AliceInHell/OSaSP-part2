#include <windows.h>
#include <math.h>
#include <string>
#include <time.h>
#include <fstream>


#define DELTA 3
#define BOLD_MENU_ID 42
#define COURSIVE_MENU_ID 43
#define UNDERLINE_MENU_ID 44

using namespace std;

int N;
int M;
string **stringMatrix;
ifstream fin("source.txt");

bool isBold = false;
bool isCoursive = false;
bool isUnderline = false;



string generateRandomString(int length)
{
	string resultString;

	for (int i = 0; i < length; i++)
		resultString += char(rand() % (127 - 32) + 32);

	return resultString;
}


HFONT randomFont()
{
	int fnWeight = 400;
	DWORD fdwItalic = FALSE;
	DWORD fdwUnderline = FALSE;

	if (isBold)
		fnWeight += 500;

	if (isCoursive)
		fdwItalic = TRUE;

	if (isUnderline)
		fdwUnderline = TRUE;

	DWORD fontFamily;
	switch (rand() % 6 + 1)
	{
		case 1:
			fontFamily = FF_DECORATIVE;
			break;
		case 2:
			fontFamily = FF_DONTCARE;
			break;
		case 3:
			fontFamily = FF_MODERN;
			break;
		case 4:
			fontFamily = FF_ROMAN;
			break;
		case 5:
			fontFamily = FF_SCRIPT;
			break;
		case 6:
			fontFamily = FF_SWISS;
			break;
	}

	return CreateFont(0, 0, 0, 0, fnWeight, fdwItalic, fdwUnderline, FALSE, FALSE,
		FALSE, FALSE, FALSE, DEFAULT_PITCH | fontFamily, NULL);
}


void Initialization(HDC hdc)
{
	//matrix Initialization
	srand(time(NULL));
	N = rand() % 13 + 1;
	M = rand() % 13 + 1;
	stringMatrix = new string*[N];
	for (int i = 0; i < N; i++)
	{
		stringMatrix[i] = new string[M];

		for (int j = 0; j < M; j++)
		{
			//stringMatrix[i][j] = generateRandomString(rand() % 30 + 1);
			getline(fin, stringMatrix[i][j]);
		}
	}
}



string getLongestString(int currentLine)
{
	string longestSrting = stringMatrix[currentLine][0];
	for (int j = 1; j < M; j++)
	{
		if (stringMatrix[currentLine][j].length() > longestSrting.length())
			longestSrting = stringMatrix[currentLine][j];
	}

	return longestSrting;
}



int GetBlockHeight(HDC hdc, int currentLine, int width)
{
	//get the longest string
	string longestString = getLongestString(currentLine);

	RECT nonDrawableBlock;
	nonDrawableBlock.left = 0;
	nonDrawableBlock.top = 0;
	nonDrawableBlock.bottom = 1;
	nonDrawableBlock.right = width;

	//draw longest string to deside block height
	return (int) DrawText(hdc, longestString.c_str(), longestString.length(), &nonDrawableBlock, 
		DT_WORDBREAK | DT_CENTER | DT_CALCRECT | DT_EDITCONTROL) + DELTA;
}



void DrawTextBlock(HDC hdc, int left, int top, int width, int height, int raw, int column)
{
	RECT drawableBlock;
	drawableBlock.left = left;
	drawableBlock.top = top + DELTA;
	drawableBlock.bottom = top + height;
	drawableBlock.right = left + width - DELTA;

	DrawText(hdc, stringMatrix[raw][column].c_str(), stringMatrix[raw][column].length(), &drawableBlock, 
		DT_WORDBREAK | DT_EDITCONTROL | DT_CENTER);
}



void DrawLine(HDC hdc, int x1, int y1, int x2, int y2)
{
	MoveToEx(hdc, x1, y1, NULL);
	LineTo(hdc, x2, y2);
}



void DrawTable(HDC hdc, int width, int height)
{
	int blockWidth = (int) width / M;
	int currentTop = 0;
	HFONT newFont;

	for (int i = 0; i < N; i++)
	{
		int blockHeigth = (int) GetBlockHeight(hdc, i, blockWidth + DELTA);

		for (int j = 0; j < M; j++)
		{
			newFont = randomFont();
			SelectObject(hdc, newFont);

			//draw one stringBlock

			DrawTextBlock(hdc, (int) j * blockWidth, currentTop, 
				blockWidth, blockHeigth, i, j);

			//draw vertical lines
			DrawLine(hdc, (j + 1) * blockWidth - 2, currentTop, 
				(j + 1) * blockWidth - 2, blockHeigth + currentTop);

			DeleteObject(newFont);
		}

		//moving down
		currentTop += blockHeigth;

		//draw horizontal lines
		DrawLine(hdc, 0, currentTop, blockWidth * M, currentTop);
	}
}



void KeyDownHandler(HWND hWnd, WPARAM key, LPARAM params) {
	switch (key) {
	case VK_ESCAPE:
		if (MessageBox(hWnd, "Do you want to exit?", "Exit", MB_YESNO) == IDYES)
			PostMessage(hWnd, WM_DESTROY, 0, 0);
		break;
	}
}



//params                 wind handle    msg
LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
	//   additional msg info
	WPARAM wParam, LPARAM lParam)
{

	//controll window size
	static int width = 0;
	static int height = 0;

	//struct for repainting
	HDC hdc;
	PAINTSTRUCT ps;
	RECT window = {};


	switch (message)
	{
		case WM_KEYDOWN:
			KeyDownHandler(hWnd, wParam, lParam);
			break;
		case WM_SIZE:
			width = LOWORD(lParam);
			height = HIWORD(lParam);
			UpdateWindow(hWnd);
			InvalidateRect(hWnd, NULL, TRUE);
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			DrawTable(hdc, width, height);
			ReleaseDC(hWnd, hdc);
			EndPaint(hWnd, &ps);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			exit(0);
			break;
		case WM_COMMAND:
			GetWindowRect(hWnd, &window);

			switch (LOWORD(wParam))
			{
				case BOLD_MENU_ID:
					isBold = !isBold;
					PostMessage(hWnd, WM_SIZE, 0, LOWORD(window.right - window.left) + 
						HIWORD(window.bottom - window.top));
					PostMessage(hWnd, WM_PAINT, 0, LOWORD(window.right - window.left) +
						HIWORD(window.bottom - window.top));
					break;
				case COURSIVE_MENU_ID:
					isCoursive = !isCoursive;
					PostMessage(hWnd, WM_SIZE, 0, LOWORD(window.right - window.left) +
						HIWORD(window.bottom - window.top));
					PostMessage(hWnd, WM_PAINT, 0, LOWORD(window.right - window.left) +
						HIWORD(window.bottom - window.top));
					break;
				case UNDERLINE_MENU_ID:
					isUnderline = !isUnderline;
					PostMessage(hWnd, WM_SIZE, 0, LOWORD(window.right - window.left) +
						HIWORD(window.bottom - window.top));
					PostMessage(hWnd, WM_PAINT, 0, LOWORD(window.right - window.left) +
						HIWORD(window.bottom - window.top));
					break;
			}
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
	wcex.lpszClassName = "TableBuilderClass";
	wcex.hIconSm = wcex.hIcon;
	RegisterClassEx(&wcex);

	hWnd = CreateWindow("TableBuilderClass", "OSaSP-Lab#2",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0,
		CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	Initialization(GetDC(hWnd));

	HMENU menu = CreateMenu();
	AppendMenu(menu, 0, BOLD_MENU_ID, "BOLT_STYLE");
	SetMenu(hWnd, menu);

	AppendMenu(menu, 0, COURSIVE_MENU_ID, "COURSIVE_STYLE");
	SetMenu(hWnd, menu);

	AppendMenu(menu, 0, UNDERLINE_MENU_ID, "UNDERLINE_STYLE");
	SetMenu(hWnd, menu);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&msg, hWnd, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}