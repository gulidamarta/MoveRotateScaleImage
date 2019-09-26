#include <Windows.h>
#include <gdiplus.h>
#include <shellapi.h>
#include <math.h>

#pragma comment (lib,"Gdiplus.lib")

#define PATH_TO_IMAGE_BMP L"ewa.bmp"
#define MOVE_LENGTH 20
#define A_PARAM 1.5
#define SCALING_STEP 0.2f
#define ROTATE_STEP 5
#define MASK_TRANSPARENT RGB(255, 0, 255)

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void DrawImage(HDC, HWND);
void MoveImageOnArrowKeys(WPARAM, HDC);
void MoveImageOnMousewheel(WPARAM);

INT xPosition = 0;
INT yPosition = 0;

INT windowHight = 0;
INT windowWidth = 0;

INT imageHight = 0;
INT imageWidth = 0;

BOOL isScaling = false;
BOOL isPositiveScaling = true;

BOOL isPositiveRotating = true;
BOOL isRotating = false;

Gdiplus::REAL currentScaling = 1.0f;
Gdiplus::REAL currentAngel = 0;

// hInstance - дескриптор экземпляра приложения. Этот дескриптор содержит адрес начала кода программы в ее адресном пространстве.
// hPrevInstance - дескриптор предыдущего экземпляра приложения, почти всегда равен NULL
// lpCmdLine - указатель на начало командной строки, введенной при запуске программы
// nCmdShow - это значение содержит желаемый вид окна (например, свернутый или развернутый)
int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	// Initialize gdiplus functionality
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

	WNDCLASSEX wcex;
	// a handle to a window
	HWND hWnd;
	MSG msg;

	wcex.cbSize = sizeof(WNDCLASSEX);
	// при изменении размеров окна, функция окна может получить сообщение WM_PAINT 
	// в этом случае функция окна должна перериросовать все окно или его часть
	// CS_DBLCLKS - отслеживание двойных щелчков мыши, в функцию окна 
	// посылаются сообщения WM_LBUTTONDBLCLK и WM_RBUTTONDBLCLK
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = "HelloWorldClass";
	wcex.hIconSm = wcex.hIcon;
	RegisterClassEx(&wcex);

	hWnd = CreateWindow("HelloWorldClass", "Lab1_OSiSP",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0,
		CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	// Sets the specified window's show state
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	//MessageBox(hWnd, "Hi from Message Box", "WinApi", 0);

	RECT rect;
	if (GetWindowRect(hWnd, &rect))
	{
		windowWidth = rect.right - rect.left;
		windowHight = rect.bottom - rect.top;
	}

	// If wMsgFilterMin and wMsgFilterMax are both zero, GetMessage returns all available messages 
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	Gdiplus::GdiplusShutdown(gdiplusToken);
	return (int)msg.wParam;
}

void DrawImage(HDC hdc, HWND hWnd)
{
	RECT rcClient;
	GetClientRect(hWnd, &rcClient);
	int left = rcClient.left;
	int top = rcClient.top;
	int width = rcClient.right - rcClient.left;
	int hight = rcClient.bottom - rcClient.top;

	HDC hdcMem = CreateCompatibleDC(hdc);
	const int nMemDC = SaveDC(hdcMem);


	HBITMAP hBitmap = CreateCompatibleBitmap(hdc, width, hight);
	SelectObject(hdcMem, hBitmap);

	Gdiplus::Graphics graphics(hdcMem);
	Gdiplus::Bitmap bmp(PATH_TO_IMAGE_BMP);

	imageWidth = bmp.GetWidth();
	imageHight = bmp.GetHeight();

	Gdiplus::REAL imageCenterWidth = xPosition + imageWidth / 2;
	Gdiplus::REAL imageCenterHight = yPosition + imageHight / 2;

	RECT rect;
	if (GetWindowRect(hWnd, &rect))
	{
		windowWidth = rect.right - rect.left;
		windowHight = rect.bottom - rect.top;
	}
	FillRect(hdcMem, &rcClient, (HBRUSH)(COLOR_WINDOW + 1));

	if (isScaling && (!isPositiveScaling)) {
		graphics.TranslateTransform(xPosition, yPosition);
		graphics.ScaleTransform(currentScaling, currentScaling);
		graphics.TranslateTransform(-xPosition, -yPosition);
		//isScaling = false;
	}

	if (isScaling && isPositiveScaling) {
		graphics.TranslateTransform(xPosition, yPosition);
		graphics.ScaleTransform(currentScaling, currentScaling);
		graphics.TranslateTransform(-xPosition, -yPosition);
		//isScaling = false;
	}

	if (isRotating && (!isPositiveRotating)) {
		graphics.TranslateTransform(imageCenterWidth, imageCenterHight);
		graphics.RotateTransform(currentAngel);
		graphics.TranslateTransform(-imageCenterWidth, -imageCenterHight);
		isRotating = false;
	}

	if (isRotating && isPositiveRotating) {
		graphics.TranslateTransform(imageCenterWidth, imageCenterHight);
		graphics.RotateTransform(currentAngel);
		graphics.TranslateTransform(-imageCenterWidth, -imageCenterHight);
		isRotating = false;
	}

	graphics.DrawImage(&bmp, xPosition, yPosition);
	BitBlt(hdc, left, top, width, hight, hdcMem, left, top, SRCCOPY);
	RestoreDC(hdcMem, nMemDC);
	DeleteObject(hBitmap);
}

void ProcessDraggedFiles(HWND hWnd, WPARAM wParam) {
	LPWSTR fileName = NULL;
	//HDC hdc;
	//PAINTSTRUCT ps;
	//hdc = BeginPaint(hWnd, &ps);
	//Gdiplus::Graphics graphics(hdc);

	DragQueryFileW((HDROP)wParam, 0, fileName, 200);
	//MessageBox(hWnd, (LPCSTR)fileName, "caption", 0);
	//Gdiplus::Bitmap bmp(fileName);

	//graphics.DrawImage(&bmp, 100, 100);
	//InvalidateRect(hWnd, NULL, TRUE);
	//EndPaint(hWnd, &ps);

}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;

	switch (message)
	{
	case WM_CREATE:
		// catch drag and dropped files.
		DragAcceptFiles(hWnd, true);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		DrawImage(hdc, hWnd);
		EndPaint(hWnd, &ps);
		return 0;
	case WM_DESTROY:
		DragAcceptFiles(hWnd, false);
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		hdc = BeginPaint(hWnd, &ps);
		MoveImageOnArrowKeys(wParam, hdc);
		EndPaint(hWnd, &ps);
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	case WM_MOUSEWHEEL:
		MoveImageOnMousewheel(wParam);
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	case WM_GETMINMAXINFO:
		MINMAXINFO FAR *lpMinMaxInfo;
		// set the MINMAXINFO structure pointer 
		lpMinMaxInfo = (MINMAXINFO FAR *) lParam;
		lpMinMaxInfo->ptMinTrackSize.x = 250;
		lpMinMaxInfo->ptMinTrackSize.y = 340;
		break;
		break;
	case WM_DROPFILES:
		//DragFinish(wParam);
		ProcessDraggedFiles(hWnd, wParam);
		//MessageBox(hWnd, "Hi from dropping", "winApi", NULL);
		InvalidateRect(hWnd, NULL, TRUE);
		//PostMessage(hWnd, , wParam, lParam);
		break;
	case WM_ERASEBKGND:
		return FALSE;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

INT GetCurrentWidth() {
	return (imageWidth + (currentScaling - 1) * 250);
}

INT GetCurrentHight() {
	return (imageHight + (currentScaling - 1) * 250);
}

void MoveLeft() {
	if (xPosition >= MOVE_LENGTH)
		xPosition -= MOVE_LENGTH;
}

void MoveRight() {
	INT tempWidth = GetCurrentWidth();
	if (xPosition + tempWidth < windowWidth + 2 * MOVE_LENGTH)
		xPosition += MOVE_LENGTH;
}

void MoveUp() {
	if (yPosition >= MOVE_LENGTH)
		yPosition -= MOVE_LENGTH;
}

void MoveDown() {
	INT tempHight = GetCurrentHight();
	if (yPosition + tempHight < windowHight + 2 * MOVE_LENGTH)
		yPosition += MOVE_LENGTH;
}

void ZoomPlus() {
	if (((GetCurrentWidth() + xPosition) < windowWidth) &&
		((GetCurrentHight() + yPosition) < windowHight)) {
		currentScaling += SCALING_STEP;
	}
	isPositiveScaling = true;
	isScaling = true;
}

void ZoomMinus() {
	if (currentScaling > 2 * SCALING_STEP) {
		currentScaling -= SCALING_STEP;
	}
	isPositiveScaling = false;
	isScaling = true;
}


bool CanBeRotated() {
	INT tempWidth = GetCurrentWidth();
	INT tempHight = GetCurrentHight();
	float diameter = sqrt((tempHight*tempHight) +
		(tempWidth * tempWidth));
	float radius = diameter / 2;
	float centerX = xPosition + tempWidth / 2;
	float centerY = yPosition + tempHight / 2;
	if ((centerX >= radius) &&
		(centerY >= radius) &&
		(centerX <= (windowWidth - radius)) &&
		(centerY <= (windowHight - radius)))
		return true;
	return false;
}

void RotatePositive() {
	if (CanBeRotated()) {
		currentAngel += ROTATE_STEP;
		isRotating = true;
		isPositiveRotating = true;
	}
}

void RotateNegative() {
	if (CanBeRotated()) {
		currentAngel -= ROTATE_STEP;
		isRotating = true;
		isPositiveRotating = false;
	}
}

void MoveImageOnArrowKeys(WPARAM wParam, HDC hdc) {
	switch (wParam) {
	case VK_LEFT:
		RotateNegative();
		break;
	case VK_RIGHT:
		RotatePositive();
		break;
	case VK_UP:
		ZoomPlus();
		break;
	case VK_DOWN:
		ZoomMinus();
		break;
	}
}

void MoveImageOnMousewheel(WPARAM wParam) {
	if (GET_KEYSTATE_WPARAM(wParam) == MK_SHIFT) {
		if (GET_WHEEL_DELTA_WPARAM(wParam) > 0) {
			MoveRight();
		}
		else {
			MoveLeft();
		}
	}
	else {
		if (GET_WHEEL_DELTA_WPARAM(wParam) > 0) {
			MoveDown();
		}
		else {
			MoveUp();
		}
	}
}


