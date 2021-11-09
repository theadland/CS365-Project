
#include <Windows.h>
#include <shellapi.h>
#include "MainWindow.h"


// the entry point for any Windows program
int WINAPI wWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	PWSTR pCmdLine,
	int nCmdShow)
{
	// First check to see if labeling service is being called and get the command line args
	LPWSTR* pArglist;
	int nArgs;
	pArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);

	if (nArgs > 1)
	{
		// TODO: add micro service logic
		MainWindow mainWindow;

		mainWindow.addImageLabel(pArglist);

		return 0;
	}
	
	if (SUCCEEDED(CoInitialize(NULL))) // Initialize Windows COM library
	{
		{
			MainWindow mainWindow;

			if (SUCCEEDED(mainWindow.Initialize()))
			{
				mainWindow.RunMessageLoop();
			}
		}
		CoUninitialize(); // Unitialize Windows COM library
	}
	
	return 0;
}
