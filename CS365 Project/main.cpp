

#include "MainWindow.h"

// the entry point for any Windows program
int WINAPI wWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	PWSTR pCmdLine,
	int nCmdShow)
{
	
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
