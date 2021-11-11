
#include <Windows.h>
#include <shellapi.h>
#include "ImageEditor.h"


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

	
	if (SUCCEEDED(CoInitialize(NULL))) // Initialize Windows COM library
	{
		{
			ImageEditor mainWindow;

			if (SUCCEEDED(mainWindow.Initialize()))
			{
				mainWindow.RunMessageLoop();
				//mainWindow.SaveBitmapToFile(L"testString", GUID_ContainerFormatJpeg);
			}
		}
		CoUninitialize(); // Unitialize Windows COM library
	}
	
	return 0;
}
