#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include <stdexcept>

const wchar_t app_title[] = L"CS365 Project";

struct StateInfo {
	// not yet in use
	// https://docs.microsoft.com/en-us/windows/win32/learnwin32/managing-application-state-
};

// the WindowProc function prototype
LRESULT CALLBACK WindowProc(HWND hwnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam);

// the entry point for any Windows program
int WINAPI wWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	PWSTR pCmdLine,
	int nCmdShow)
{
	// Register the window class
	const wchar_t CLASS_NAME[] = L"Sample Window Class";

	// State initialization info to pass to main window
	StateInfo* pState = new (std::nothrow) StateInfo;

	// this struct holds information for the window class
	WNDCLASS wc = { };

	// fill in the struct with the needed information
	wc.lpfnWndProc = WindowProc;				// pointer to WindowProc callback
	wc.hInstance = hInstance;					// application instance handle
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = CLASS_NAME;				// window class name

	// register the window class with the operating system
	RegisterClass(&wc);

	// adjust window size based on client size
	RECT wr = { 0, 0, 500, 400 };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, false);

	// create the window and use the result as the handle
	HWND hwnd = CreateWindowEx(0,
		CLASS_NAME,				// name of the window class
		app_title,				// title of the window
		WS_OVERLAPPEDWINDOW,	// window style
		300,					// x-position of window
		300,					// y-position of window
		wr.right - wr.left,		// width of the window
		wr.bottom - wr.top,		// height of the window
		NULL,					// we have no parent window, so null
		NULL,					// we aren't using menus, so null
		hInstance,				// application handle
		pState);				// pointer to struct passing state infomation

	if (hwnd == NULL)
	{
		return 0;
	}

	//display window on the screen
	ShowWindow(hwnd, nCmdShow);

	// -- MAIN LOOP --

	// this struct holds Windows event messages
	MSG msg;
	// wait for the next message in the queue, store the result in 'msg'
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg); // translate keystroke messages into the right format
		DispatchMessage(&msg);  // send the message tot hte WindowProc function
	}

	// return this part of the WM_QUIT message to Windows
	return msg.wParam;
}

// this is the main message handler for the program and where application
// behaviour is defined
LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// sort through and find what code to run for the message given
	switch (message)
	{
		//this message is read when the window is clased
	case WM_DESTROY:
	{
		// close the application entirely
		PostQuitMessage(0); // message causes GetMessage to return 0, ends while loop above
		return 0;			 // tells windows message was handled
	}
	case WM_CLOSE:
	{
		if (MessageBox(hwnd, L"Are you sure you want to Exit?", app_title, MB_OKCANCEL) == IDOK)
		{
			DestroyWindow(hwnd);
		}
		// Else: User canceled. Do nothing.
		return 0;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		// All painting occurs here, between BeginPaint and EndPaint.

		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

		EndPaint(hwnd, &ps);
	}
	return 0;

	}
	// Handle any messages the switch statement didn't
	return DefWindowProc(hwnd, message, wParam, lParam);
}