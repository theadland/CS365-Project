#pragma once

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER              // Allow use of features specific to Windows 7 or later.
#define WINVER 0x0A00       // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT        // Allow use of features specific to Windows 7 or later.
#define _WIN32_WINNT 0x0A00 // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef UNICODE
#define UNICODE
#endif 

#define WIN32_LEAN_AND_MEAN     // Exclude rarely-used stuff from Windows headers
#include <Windows.h>
#include <stdexcept>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <wchar.h>
#include <math.h>

#include <d2d1_1.h>
#include <d2d1_1helper.h>
#include <dwrite.h>
#include <wincodec.h>
#pragma comment(lib, "d2d1")

#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")

#include "Resource.h"


/******************************************************************
*                                                                 *
*  Macros                                                         *
*                                                                 *
******************************************************************/

template<class Interface>
inline void
SafeRelease(
	Interface** ppInterfaceToRelease
)
{
	if (*ppInterfaceToRelease != NULL)
	{
		(*ppInterfaceToRelease)->Release();

		(*ppInterfaceToRelease) = NULL;
	}
}

#ifndef Assert
#if defined( DEBUG ) || defined( _DEBUG )
#define Assert(b) do {if (!(b)) {OutputDebugStringA("Assert: " #b "\n");}} while(0)
#else
#define Assert(b)
#endif //DEBUG || _DEBUG
#endif


#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

/******************************************************************
*                                                                 *
*  MainWindow                                                     *
*                                                                 *
******************************************************************/

class MainWindow
{
public:
	MainWindow();
	~MainWindow();

	// Create application window and initialize device-indedendent resources
	HRESULT Initialize();

	// Create resources not bound to any device
	HRESULT CreateDeviceIndependentResources();

	// Create resourcs bound to a particular device
	HRESULT CreateDeviceResources();

	HRESULT CreateGridPatternBrush(
		ID2D1RenderTarget* pRenderTarget,
		ID2D1BitmapBrush** ppBitmapBrush
	);

	HRESULT CreateBlankBackgroundBitmap(
		ID2D1RenderTarget* pRenderTarget,
		ID2D1BitmapBrush** ppBitmapBrush
	);

	HRESULT CreateCircleBrush(
		ID2D1RenderTarget* pRenderTarget,
		ID2D1BitmapBrush** ppBitmapBrush
	);

	void DiscardDeviceResources();

	HRESULT OnRender();

	void OnResize(
		UINT width,
		UINT height
	);

	// Create and load resource file
	HRESULT LoadResourceBitmap(
		ID2D1RenderTarget* pRenderTarget,
		IWICImagingFactory* pIWICFactory,
		PCWSTR resourceName,
		PCWSTR resourceType,
		UINT destinationWidth,
		UINT destinationHeight,
		ID2D1Bitmap** ppBitmap
	);

	HRESULT LoadBitmapFromFile(
		ID2D1RenderTarget* pRenderTarget,
		IWICImagingFactory* pIWICFactory,
		PCWSTR uri,
		UINT destinationWidth,
		UINT destinationHeight,
		ID2D1Bitmap** ppBitmap
	);


	// Main window message loop
	void RunMessageLoop();

	// the WindowProc function prototype
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


private:
	HWND m_hwnd;
	ID2D1Factory* m_pD2DFactory;
	IWICImagingFactory* m_pWICFactory;
	IDWriteFactory* m_pDWriteFactory;
	ID2D1HwndRenderTarget* m_pRenderTarget;
	IDWriteTextFormat* m_pTextFormat;
	ID2D1PathGeometry* m_pPathGeometry;
	ID2D1LinearGradientBrush* m_pLinearGradientBrush;
	ID2D1BitmapBrush* m_pBlankBackgroundBrush;
	ID2D1BitmapBrush* m_pCircleBitmapBrush;
	ID2D1SolidColorBrush* m_pBlackBrush;
	ID2D1BitmapBrush* m_pGridPatternBitmapBrush;
	ID2D1Bitmap* m_pBitmap;
	ID2D1Bitmap* m_pAnotherBitmap;

	HIMAGELIST g_hImageList = NULL;

	ID2D1Bitmap* pBackgroundBitmap = NULL;

	bool showImage;
};

