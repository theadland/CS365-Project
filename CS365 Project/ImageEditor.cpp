#include "ImageEditor.h"

ImageEditor::ImageEditor() :
	// Initialize members

	m_hwnd(NULL),

	m_pD2DFactory(NULL),
	m_pWICFactory(NULL),
	m_pWICFactory2(NULL),
	m_pDWriteFactory(NULL),

	m_pRenderTarget(NULL),
	m_pDCRenderTarget(NULL),
	m_pWICRenderTarget(NULL),

	m_pTextFormat(NULL),
	m_pPathGeometry(NULL),
	m_pLinearGradientBrush(NULL),
	m_pBlackBrush(NULL),
	m_pGridPatternBitmapBrush(NULL),
	m_pBlankBackgroundBrush(NULL),
	m_pCircleBitmapBrush(NULL),

	m_pDisplayBitmap(NULL),
	m_pBitmap(NULL),
	m_pAnotherBitmap(NULL),
	m_pWICBitmap(NULL)

{
	showImage = false;
	showWindow = true;
}

ImageEditor::~ImageEditor()
	// Release resources
{
	SafeRelease(&m_pD2DFactory);
	SafeRelease(&m_pWICFactory);
	SafeRelease(&m_pDWriteFactory);
	SafeRelease(&m_pRenderTarget);
	SafeRelease(&m_pTextFormat);
	SafeRelease(&m_pPathGeometry);
	SafeRelease(&m_pLinearGradientBrush);
	SafeRelease(&m_pBlackBrush);
	SafeRelease(&m_pGridPatternBitmapBrush);
	SafeRelease(&m_pBlankBackgroundBrush);
	SafeRelease(&m_pCircleBitmapBrush);
	SafeRelease(&m_pBitmap);
	SafeRelease(&m_pAnotherBitmap);
}

HRESULT ImageEditor::Initialize()
{
	HRESULT hr;

	hr = ImageEditor::CreateDeviceIndependentResources();

	

	if (SUCCEEDED(hr))
	{
		// Register the window class
		WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = ImageEditor::WindowProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = sizeof(LONG_PTR);
		wcex.hInstance = HINST_THISCOMPONENT;
		wcex.hbrBackground = NULL;
		wcex.lpszMenuName = MAKEINTRESOURCEW(TOOLBAR_DROPDOWN_MENU);
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.lpszClassName = L"CLassApp";

		RegisterClassEx(&wcex);


		// Because the CreateWindow function takes its size in pixels, we
		// obtain the system DPI and use it to scale the window size.
		UINT dpi = GetDpiForSystem();

		// Create the application window.
		m_hwnd = CreateWindow(
			L"CLassApp",
			L"CS362 Course Project",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			static_cast<INT>(ceil(640.f * dpi / 96.f)),
			static_cast<INT>(ceil(480.f * dpi / 96.f)),
			NULL,
			NULL,
			HINST_THISCOMPONENT,
			this
		);

		// Currently, resource parameters based on client size
		hr = ImageEditor::CreateDeviceResources();

		ImageEditor::DrawInitialBlankCanvas(m_pWICRenderTarget);

		// Show window if created successfully
		hr = m_hwnd ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			ShowWindow(m_hwnd, SW_SHOWNORMAL);
			UpdateWindow(m_hwnd);
		}
	}

	return hr;
}

// Create resources which are not bound
// to any device. Their lifetime effectively extends for the
// duration of the app. These resources include the Direct2D,
// DirectWrite, and WIC factories; and a DirectWrite Text Format object
// (used for identifying particular font characteristics) and
// a Direct2D geometry.
HRESULT ImageEditor::CreateDeviceIndependentResources()
{
	static const WCHAR msc_fontName[] = L"Verdana";
	static const FLOAT msc_fontSize = 50;
	HRESULT hr;
	ID2D1GeometrySink* pSink = NULL;

	// Create a Direct2D factory.
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
	if (SUCCEEDED(hr))
	{
		// Create WIC factory.
		hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IWICImagingFactory,
			reinterpret_cast<void**>(&m_pWICFactory)
		);
	}
	if (SUCCEEDED(hr))
	{
		// Create a DirectWrite factory.
		hr = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(m_pDWriteFactory),
			reinterpret_cast<IUnknown**>(&m_pDWriteFactory)
		);
	}
	if (SUCCEEDED(hr))
	{
		// Create a DirectWrite text format object.
		hr = m_pDWriteFactory->CreateTextFormat(
			msc_fontName,
			NULL,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			msc_fontSize,
			L"", //locale
			&m_pTextFormat
		);
	}
	if (SUCCEEDED(hr))
	{
		// Center the text horizontally and vertically.
		m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

		m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	}


	return hr;
}

HRESULT ImageEditor::CreateDeviceResources()
{
	HRESULT hr = S_OK;

	if (!m_pRenderTarget)
	{
		RECT rc;
		GetClientRect(m_hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(
			static_cast<UINT>(rc.right - rc.left),
			static_cast<UINT>(rc.bottom - rc.top)
		);

		// Create a pixel format and initial its format and alphaMode fields.
		//  - compatible with GUID_WICPixelFormat32bppPBGRA
		D2D1_PIXEL_FORMAT pixelFormat = D2D1::PixelFormat(
			DXGI_FORMAT_B8G8R8A8_UNORM,
			D2D1_ALPHA_MODE_PREMULTIPLIED
		);

		UINT dpi = GetDpiForSystem();

		D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties();
		props.usage = D2D1_RENDER_TARGET_USAGE_FORCE_BITMAP_REMOTING;
		props.type = D2D1_RENDER_TARGET_TYPE_SOFTWARE;							// This type required for sharing resources between different render targets
		props.pixelFormat = pixelFormat;
		props.dpiX = dpi;
		props.dpiY = dpi;

		// Create a Direct2D Hwnd render target.
		hr = m_pD2DFactory->CreateHwndRenderTarget(
			props,
			D2D1::HwndRenderTargetProperties(m_hwnd, size),
			&m_pRenderTarget
		);

		// Create WIC bitmap to be used as render target
		hr = m_pWICFactory->CreateBitmap(
			size.width,
			size.height,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapCacheOnLoad,
			&m_pWICBitmap
		);

		// Create WIC rendertarget
		hr = m_pD2DFactory->CreateWicBitmapRenderTarget(
			m_pWICBitmap,
			props,
			&m_pWICRenderTarget
		);

		if (SUCCEEDED(hr))
		{
			// Create a black brush.
			hr = m_pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::Black),
				&m_pBlackBrush
			);
		}
		if (SUCCEEDED(hr))
		{
			ID2D1GradientStopCollection* pGradientStops = NULL;
			// Create a linear gradient.
			static const D2D1_GRADIENT_STOP stops[] =
			{
				{   0.f,  { 0.f, 1.f, 1.f, 0.25f }  },
				{   1.f,  { 0.f, 0.f, 1.f, 1.f }  },
			};

			hr = m_pRenderTarget->CreateGradientStopCollection(
				stops,
				ARRAYSIZE(stops),
				&pGradientStops
			);
			if (SUCCEEDED(hr))
			{
				hr = m_pRenderTarget->CreateLinearGradientBrush(
					D2D1::LinearGradientBrushProperties(
						D2D1::Point2F(100, 0),
						D2D1::Point2F(100, 200)),
					D2D1::BrushProperties(),
					pGradientStops,
					&m_pLinearGradientBrush
				);
				pGradientStops->Release();
			}
		}

		// Create a bitmap from an application resource.
		hr = LoadResourceBitmap(
			m_pRenderTarget,
			m_pWICFactory,
			L"SampleImage",
			L"Image",
			400,
			0,
			&m_pBitmap
		);

		if (SUCCEEDED(hr))
		{
			// Create a bitmap by loading it from a file.
			hr = LoadBitmapFromFile(
				m_pRenderTarget,
				m_pWICFactory,
				L".\\sampleImage.jpg",
				100,
				0,
				&m_pAnotherBitmap
			);
		}
		if (SUCCEEDED(hr))
		{
			hr = CreateGridPatternBrush(m_pRenderTarget, &m_pGridPatternBitmapBrush);
			hr = CreateCircleBrush(m_pRenderTarget, &m_pCircleBitmapBrush);
		}
	}

	return hr;
}

HRESULT ImageEditor::CreateGridPatternBrush(ID2D1RenderTarget* pRenderTarget, ID2D1BitmapBrush** ppBitmapBrush)
{
	HRESULT hr = S_OK;

	// Create a compatible render target.
	ID2D1BitmapRenderTarget* pCompatibleRenderTarget = NULL;
	hr = pRenderTarget->CreateCompatibleRenderTarget(
		D2D1::SizeF(10.0f, 10.0f),
		&pCompatibleRenderTarget
	);
	if (SUCCEEDED(hr))
	{
		// Draw a pattern.
		ID2D1SolidColorBrush* pGridBrush = NULL;
		hr = pCompatibleRenderTarget->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF(0.93f, 0.94f, 0.96f, 1.0f)),
			&pGridBrush
		);
		if (SUCCEEDED(hr))
		{
			pCompatibleRenderTarget->BeginDraw();
			pCompatibleRenderTarget->FillRectangle(D2D1::RectF(0.0f, 0.0f, 10.0f, 1.0f), pGridBrush);
			pCompatibleRenderTarget->FillRectangle(D2D1::RectF(0.0f, 0.1f, 1.0f, 10.0f), pGridBrush);
			pCompatibleRenderTarget->EndDraw();

			// Retrieve the bitmap from the render target.
			ID2D1Bitmap* pGridBitmap = NULL;
			hr = pCompatibleRenderTarget->GetBitmap(&pGridBitmap);
			if (SUCCEEDED(hr))
			{
				// Choose the tiling mode for the bitmap brush.
				D2D1_BITMAP_BRUSH_PROPERTIES brushProperties =
					D2D1::BitmapBrushProperties(D2D1_EXTEND_MODE_WRAP, D2D1_EXTEND_MODE_WRAP);

				// Create the bitmap brush.
				hr = m_pRenderTarget->CreateBitmapBrush(pGridBitmap, brushProperties, ppBitmapBrush);

				pGridBitmap->Release();
			}

			pGridBrush->Release();
		}

		pCompatibleRenderTarget->Release();
	}

	return hr;
}

HRESULT ImageEditor::DrawInitialBlankCanvas(ID2D1RenderTarget* pRenderTarget)
{
	ID2D1Bitmap* pTempBitmap = NULL;

	HRESULT hr = S_OK;

	// Create a compatible render target.
	ID2D1BitmapRenderTarget* pCompatibleRenderTarget = NULL;
	hr = pRenderTarget->CreateCompatibleRenderTarget(
		D2D1::SizeF(400.0f, 400.0f),
		&pCompatibleRenderTarget);

	// Draw a pattern.
	ID2D1SolidColorBrush* pSolidBrush = NULL;
	hr = pCompatibleRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::AntiqueWhite),
		&pSolidBrush
	);

	// File rendertarget with solid color
	pCompatibleRenderTarget->BeginDraw();
	pCompatibleRenderTarget->FillRectangle(D2D1::RectF(0.0f, 0.0f, 400.0f, 400.0f), pSolidBrush);
	pCompatibleRenderTarget->EndDraw();

	// Retrieve the bitmap from the render target.
	hr = pCompatibleRenderTarget->GetBitmap(&pTempBitmap);

	// Draw blank canvas bitmap to WIC bitmap
	m_pWICRenderTarget->BeginDraw();
	m_pWICRenderTarget->DrawBitmap(pTempBitmap);
	hr = m_pWICRenderTarget->EndDraw();

	hr = m_pWICRenderTarget->CreateBitmapFromWicBitmap(m_pWICBitmap, &m_pDisplayBitmap);

	return hr;
}

HRESULT ImageEditor::CreateCircleBrush(ID2D1RenderTarget* pRenderTarget, ID2D1BitmapBrush** ppBitmapBrush)
{
	HRESULT hr = S_OK;

	// Create a compatible render target.
	ID2D1BitmapRenderTarget* pCompatibleRenderTarget = NULL;
	hr = pRenderTarget->CreateCompatibleRenderTarget(
		D2D1::SizeF(10.0f, 10.0f),
		&pCompatibleRenderTarget
	);
	if (SUCCEEDED(hr))
	{
		// Draw a pattern.
		ID2D1SolidColorBrush* pCricleBrush = NULL;
		hr = pCompatibleRenderTarget->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF(D2D1::ColorF::White)),
			&pCricleBrush
		);

		// Define a rounded rectangle.
		D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(
			D2D1::RectF(20.f, 20.f, 150.f, 100.f),
			10.f,
			10.f
		);

		// Draw the rectangle.
		pCompatibleRenderTarget->DrawRoundedRectangle(roundedRect, m_pBlackBrush, 10.f);

		// Retrieve the bitmap from the render target.
		ID2D1Bitmap* pCircleBrushBitmap = NULL;
		hr = pCompatibleRenderTarget->GetBitmap(&pCircleBrushBitmap);
		if (SUCCEEDED(hr))
		{
			// Choose the tiling mode for the bitmap brush.
			D2D1_BITMAP_BRUSH_PROPERTIES brushProperties =
				D2D1::BitmapBrushProperties(D2D1_EXTEND_MODE_CLAMP, D2D1_EXTEND_MODE_CLAMP);

			// Create the bitmap brush.
			hr = m_pRenderTarget->CreateBitmapBrush(pCircleBrushBitmap, brushProperties, ppBitmapBrush);

			pCircleBrushBitmap->Release();
		}

		pCricleBrush->Release();
	}

	pCompatibleRenderTarget->Release();

	return hr;
}

//  Discard device-specific resources which need to be recreated
//  when a Direct3D device is lost
void ImageEditor::DiscardDeviceResources()
{
	SafeRelease(&m_pRenderTarget);
	SafeRelease(&m_pBitmap);
	SafeRelease(&m_pBlackBrush);
	SafeRelease(&m_pLinearGradientBrush);
	SafeRelease(&m_pAnotherBitmap);
	SafeRelease(&m_pGridPatternBitmapBrush);
}

//  Called whenever the application needs to display the client
//  window. This method draws a bitmap a couple times, draws some
//  geometries, and writes "Hello, World"
//
//  Note that this function will not render anything if the window
//  is occluded (e.g. when the screen is locked).
//  Also, this function will automatically discard device-specific
//  resources if the Direct3D device disappears during function
//  invocation, and will recreate the resources the next time it's
//  invoked.
HRESULT ImageEditor::OnRender()
{
	HRESULT hr = S_OK;

	if (SUCCEEDED(hr) && !(m_pRenderTarget->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
	{
		static const WCHAR sc_helloWorld[] = L"Example Label!";
		// Retrieve the size of the render target.
		D2D1_SIZE_F renderTargetSize = m_pRenderTarget->GetSize();

		m_pRenderTarget->BeginDraw();

		m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

		m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

		// Paint a grid background.
		m_pRenderTarget->FillRectangle(
			D2D1::RectF(0.0f, 0.0f, renderTargetSize.width, renderTargetSize.height),
			m_pGridPatternBitmapBrush
		);

		// Paint the initial blank image background.
		m_pRenderTarget->DrawBitmap(
			m_pDisplayBitmap
		);

		if (showImage)
		{
			D2D1_SIZE_F size = m_pBitmap->GetSize();

			// Draw a bitmap in the center of the window.
			m_pRenderTarget->DrawBitmap(
				m_pBitmap,
				D2D1::RectF(
					ceil(renderTargetSize.width / 2) - ceil(size.width / 2),
					ceil(renderTargetSize.height / 2) - ceil(size.height / 2),
					ceil(renderTargetSize.width / 2) + ceil(size.width / 2),
					ceil(renderTargetSize.height / 2) + ceil(size.height / 2))
			);
		}

		m_pRenderTarget->SetTransform(
			D2D1::Matrix3x2F::Translation(0, renderTargetSize.height / 4)
		);

		m_pRenderTarget->DrawText(
			sc_helloWorld,
			ARRAYSIZE(sc_helloWorld) - 1,
			m_pTextFormat,
			D2D1::RectF(0, 0, renderTargetSize.width, renderTargetSize.height),
			m_pBlackBrush
		);

		//
		// Reset back to the identity transform

		

		hr = m_pRenderTarget->EndDraw();

		if (hr == D2DERR_RECREATE_TARGET)
		{
			hr = S_OK;
			DiscardDeviceResources();
		}
	}

	return hr;
}

//  If the application receives a WM_SIZE message, this method
//  resize the render target appropriately.
void ImageEditor::OnResize(UINT width, UINT height)
{
	if (m_pRenderTarget)
	{
		D2D1_SIZE_U size;
		size.width = width;
		size.height = height;

		// Note: This method can fail, but it's okay to ignore the
		// error here -- it will be repeated on the next call to
		// EndDraw.
		m_pRenderTarget->Resize(size);
	}
}

HRESULT ImageEditor::LoadResourceBitmap(
	ID2D1RenderTarget* pRenderTarget, 
	IWICImagingFactory* pIWICFactory, 
	PCWSTR resourceName, 
	PCWSTR resourceType, 
	UINT destinationWidth, 
	UINT destinationHeight, 
	ID2D1Bitmap** ppBitmap)
{
	HRESULT hr = S_OK;
	IWICBitmapDecoder* pDecoder = NULL;
	IWICBitmapFrameDecode* pSource = NULL;
	IWICStream* pStream = NULL;
	IWICFormatConverter* pConverter = NULL;
	IWICBitmapScaler* pScaler = NULL;

	HRSRC imageResHandle = NULL;
	HGLOBAL imageResDataHandle = NULL;
	void* pImageFile = NULL;
	DWORD imageFileSize = 0;

	// Locate the resource.
	imageResHandle = FindResourceW(HINST_THISCOMPONENT, resourceName, resourceType);

	hr = imageResHandle ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		// Load the resource.
		imageResDataHandle = LoadResource(HINST_THISCOMPONENT, imageResHandle);

		hr = imageResDataHandle ? S_OK : E_FAIL;
	}
	if (SUCCEEDED(hr))
	{
		// Lock it to get a system memory pointer.
		pImageFile = LockResource(imageResDataHandle);

		hr = pImageFile ? S_OK : E_FAIL;
	}
	if (SUCCEEDED(hr))
	{
		// Calculate the size.
		imageFileSize = SizeofResource(HINST_THISCOMPONENT, imageResHandle);

		hr = imageFileSize ? S_OK : E_FAIL;
	}
	if (SUCCEEDED(hr))
	{
		// Create a WIC stream to map onto the memory.
		hr = pIWICFactory->CreateStream(&pStream);
	}
	if (SUCCEEDED(hr))
	{
		// Initialize the stream with the memory pointer and size.
		hr = pStream->InitializeFromMemory(
			reinterpret_cast<BYTE*>(pImageFile),
			imageFileSize
		);
	}
	if (SUCCEEDED(hr))
	{
		// Create a decoder for the stream.
		hr = pIWICFactory->CreateDecoderFromStream(
			pStream,
			NULL,
			WICDecodeMetadataCacheOnLoad,
			&pDecoder
		);
	}
	if (SUCCEEDED(hr))
	{
		// Create the initial frame.
		hr = pDecoder->GetFrame(0, &pSource);
	}
	if (SUCCEEDED(hr))
	{
		// Convert the image format to 32bppPBGRA
		// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
		hr = pIWICFactory->CreateFormatConverter(&pConverter);
	}
	if (SUCCEEDED(hr))
	{
		// If a new width or height was specified, create an
		// IWICBitmapScaler and use it to resize the image.
		if (destinationWidth != 0 || destinationHeight != 0)
		{
			UINT originalWidth, originalHeight;
			hr = pSource->GetSize(&originalWidth, &originalHeight);
			if (SUCCEEDED(hr))
			{
				if (destinationWidth == 0)
				{
					FLOAT scalar = static_cast<FLOAT>(destinationHeight) / static_cast<FLOAT>(originalHeight);
					destinationWidth = static_cast<UINT>(scalar * static_cast<FLOAT>(originalWidth));
				}
				else if (destinationHeight == 0)
				{
					FLOAT scalar = static_cast<FLOAT>(destinationWidth) / static_cast<FLOAT>(originalWidth);
					destinationHeight = static_cast<UINT>(scalar * static_cast<FLOAT>(originalHeight));
				}

				hr = pIWICFactory->CreateBitmapScaler(&pScaler);
				if (SUCCEEDED(hr))
				{
					hr = pScaler->Initialize(
						pSource,
						destinationWidth,
						destinationHeight,
						WICBitmapInterpolationModeCubic
					);
					if (SUCCEEDED(hr))
					{
						hr = pConverter->Initialize(
							pScaler,
							GUID_WICPixelFormat32bppPBGRA,
							WICBitmapDitherTypeNone,
							NULL,
							0.f,
							WICBitmapPaletteTypeMedianCut
						);
					}
				}
			}
		}
		else
		{
			hr = pConverter->Initialize(
				pSource,
				GUID_WICPixelFormat32bppPBGRA,
				WICBitmapDitherTypeNone,
				NULL,
				0.f,
				WICBitmapPaletteTypeMedianCut
			);
		}
	}
	if (SUCCEEDED(hr))
	{
		//create a Direct2D bitmap from the WIC bitmap.
		hr = pRenderTarget->CreateBitmapFromWicBitmap(
			pConverter,
			NULL,
			ppBitmap
		);
	}

	SafeRelease(&pDecoder);
	SafeRelease(&pSource);
	SafeRelease(&pStream);
	SafeRelease(&pConverter);
	SafeRelease(&pScaler);

	return hr;
}

HRESULT ImageEditor::LoadBitmapFromFile(ID2D1RenderTarget* pRenderTarget, IWICImagingFactory* pIWICFactory, PCWSTR uri, UINT destinationWidth, UINT destinationHeight, ID2D1Bitmap** ppBitmap)
{
	HRESULT hr = S_OK;

	IWICBitmapDecoder* pDecoder = NULL;
	IWICBitmapFrameDecode* pSource = NULL;
	IWICStream* pStream = NULL;
	IWICFormatConverter* pConverter = NULL;
	IWICBitmapScaler* pScaler = NULL;

	hr = pIWICFactory->CreateDecoderFromFilename(
		uri,
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&pDecoder
	);
	if (SUCCEEDED(hr))
	{

		// Create the initial frame.
		hr = pDecoder->GetFrame(0, &pSource);
	}

	if (SUCCEEDED(hr))
	{
		// Convert the image format to 32bppPBGRA
		// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
		hr = pIWICFactory->CreateFormatConverter(&pConverter);
	}
	if (SUCCEEDED(hr))
	{
		// If a new width or height was specified, create an
		// IWICBitmapScaler and use it to resize the image.
		if (destinationWidth != 0 || destinationHeight != 0)
		{
			UINT originalWidth, originalHeight;
			hr = pSource->GetSize(&originalWidth, &originalHeight);
			if (SUCCEEDED(hr))
			{
				if (destinationWidth == 0)
				{
					FLOAT scalar = static_cast<FLOAT>(destinationHeight) / static_cast<FLOAT>(originalHeight);
					destinationWidth = static_cast<UINT>(scalar * static_cast<FLOAT>(originalWidth));
				}
				else if (destinationHeight == 0)
				{
					FLOAT scalar = static_cast<FLOAT>(destinationWidth) / static_cast<FLOAT>(originalWidth);
					destinationHeight = static_cast<UINT>(scalar * static_cast<FLOAT>(originalHeight));
				}

				hr = pIWICFactory->CreateBitmapScaler(&pScaler);
				if (SUCCEEDED(hr))
				{
					hr = pScaler->Initialize(
						pSource,
						destinationWidth,
						destinationHeight,
						WICBitmapInterpolationModeCubic
					);
				}
				if (SUCCEEDED(hr))
				{
					hr = pConverter->Initialize(
						pScaler,
						GUID_WICPixelFormat32bppPBGRA,
						WICBitmapDitherTypeNone,
						NULL,
						0.f,
						WICBitmapPaletteTypeMedianCut
					);
				}
			}
		}
		else // Don't scale the image.
		{
			hr = pConverter->Initialize(
				pSource,
				GUID_WICPixelFormat32bppPBGRA,
				WICBitmapDitherTypeNone,
				NULL,
				0.f,
				WICBitmapPaletteTypeMedianCut
			);
		}
	}
	if (SUCCEEDED(hr))
	{
		// Create a Direct2D bitmap from the WIC bitmap.
		hr = pRenderTarget->CreateBitmapFromWicBitmap(
			pConverter,
			NULL,
			ppBitmap
		);
		bool success;
		success = UpdateWindow(m_hwnd);
	}

	SafeRelease(&pDecoder);
	SafeRelease(&pSource);
	SafeRelease(&pStream);
	SafeRelease(&pConverter);
	SafeRelease(&pScaler);

	return hr;
}

HRESULT ImageEditor::SaveBitmapToFile(PCWSTR uri, REFGUID wicFormat)
{
	HRESULT hr = S_OK;

	IWICBitmapEncoder* pBitmapEncoder = NULL;
	IWICBitmapFrameEncode* pFrameEncode = NULL;
	IWICImageEncoder* pImageEncoder = NULL;

	IWICStream* pStream = NULL;

	PCWSTR imageFilename = L"testImage.bmp";

	if (SUCCEEDED(hr))
	{
		m_pWICRenderTarget->BeginDraw();
		m_pWICRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Blue));
		m_pWICRenderTarget->DrawBitmap(m_pBitmap);
	}
	hr = m_pWICRenderTarget->EndDraw();

	 
	if (SUCCEEDED(hr))
	{
		hr = m_pWICFactory->CreateStream(&pStream);
	}

	hr = m_pWICFactory->CreateStream(&pStream);

	// Create WIC bitmap encoder, frame encoder
// ----------------TODO: add logic that stores initial file type so can re-encode to that type-------------
	
	hr = pStream->InitializeFromFilename(imageFilename, GENERIC_WRITE);

	hr = m_pWICFactory->CreateEncoder(GUID_ContainerFormatBmp, nullptr, &pBitmapEncoder); // -----first param is file format to be user selected
	hr = pBitmapEncoder->Initialize(pStream, WICBitmapEncoderNoCache);

	hr = pBitmapEncoder->CreateNewFrame(&pFrameEncode, nullptr);
	hr = pFrameEncode->Initialize(nullptr);

	WICPixelFormatGUID format = GUID_WICPixelFormatDontCare;
	hr = pFrameEncode->SetPixelFormat(&format);

	hr = pFrameEncode->WriteSource(m_pWICBitmap, NULL);

	hr = pFrameEncode->Commit();

	hr = pBitmapEncoder->Commit();

	return hr;
}

PWSTR ImageEditor::GetFilePathFromOpenWindow()
{
	IFileOpenDialog* pFileOpen;

	// Create the FileOpenDialog object.
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
		IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));


	// Show the Open dialog box.
	hr = pFileOpen->Show(NULL);

	// Get the file name from the dialog box.
	IShellItem* pItem;
	hr = pFileOpen->GetResult(&pItem);

	PWSTR pszFilePath = nullptr;
	if (SUCCEEDED(hr))
	{
		hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
	}

	pItem->Release();

	pFileOpen->Release();

	return pszFilePath;
}

void ImageEditor::RunMessageLoop()
{
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0))
	{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
	}
}

LRESULT ImageEditor::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	if (message == WM_CREATE)
	{
		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
		ImageEditor* pImageEditor = (ImageEditor*)pcs->lpCreateParams;

		::SetWindowLongPtrW(
			hwnd,
			GWLP_USERDATA,
			reinterpret_cast<LONG_PTR>(pImageEditor)
		);

		result = 1;
	}
	else
	{
		ImageEditor* pImageEditor = reinterpret_cast<ImageEditor*>(
			::GetWindowLongPtrW(
				hwnd,
				GWLP_USERDATA
			));

		bool wasHandled = false;

		if (pImageEditor)
		{
			switch (message)
			{
			case WM_COMMAND:
			{
				int wmId = LOWORD(wParam);
				// Parse the menu selections:
				switch (wmId)
				{
				case OPEN:
				{
					// Get file path of image to open from user
					PWSTR filePath = pImageEditor->GetFilePathFromOpenWindow();

					// ---------------------------------TODO-------------------------------!!!
					// --create class to hold multiple bitmaps loaded from file
					// Load bitmap to output bitmap
					pImageEditor->LoadBitmapFromFile(
						pImageEditor->m_pWICRenderTarget,
						pImageEditor->m_pWICFactory,
						filePath,
						0,
						0,
						&pImageEditor->m_pDisplayBitmap
					);

					bool success;
					success = UpdateWindow(hwnd);
					PostMessage(hwnd, WM_COMMAND, 78, 78);
				}
				case WM_PAINT:
				case WM_DISPLAYCHANGE:
				{
					PAINTSTRUCT ps;
					BeginPaint(hwnd, &ps);
					pImageEditor->OnRender();
					EndPaint(hwnd, &ps);
				}
				result = 0;
				wasHandled = true;
				break;
				case SAVE:
				{
					IFileSaveDialog* pFileSave;

					// Create the FileOpenDialog object.
					HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL,
						IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileSave));

					if (SUCCEEDED(hr))
					{
						// Show the Open dialog box.
						hr = pFileSave->Show(NULL);

						// TODO: implement save feature
						pImageEditor->SaveBitmapToFile(
							L"C:/Users/thead/Desktop", // ----------------Fix
							GUID_ContainerFormatJpeg);  // -------------Fix

						pFileSave->Release();
					}

					return 0;
				}
				case RESIZE:
				{
					pImageEditor->callSubProcess();

					return 0;
				}
				case ABOUT:
				case EXIT:
					DestroyWindow(hwnd);
					break;
				default:
					return DefWindowProc(hwnd, message, wParam, lParam);
				}
			}
			break;
			case WM_SIZE:
			{
				UINT width = LOWORD(lParam);
				UINT height = HIWORD(lParam);
				pImageEditor->OnResize(width, height);
			}
			result = 0;
			wasHandled = true;
			break;

			case WM_PAINT:
			case WM_DISPLAYCHANGE:
			{
				PAINTSTRUCT ps;
				BeginPaint(hwnd, &ps);
				pImageEditor->OnRender();
				EndPaint(hwnd, &ps);
			}
			result = 0;
			wasHandled = true;
			break;

			case WM_DESTROY:
			{
				PostQuitMessage(0);
			}
			result = 1;
			wasHandled = true;
			break;
			}
		}

		if (!wasHandled)
		{
			result = DefWindowProc(hwnd, message, wParam, lParam);
		}
	}

	return result;
}

void ImageEditor::addImageLabel(LPWSTR* pArgList)
{
	HRESULT hr = S_OK;

	


}

void ImageEditor::callSubProcess()
{
	wchar_t path[] = L"C:/My_Files/School Work/OSU/CS 361 - Software Engineering I/Module 4/Image Manipulator.exe";

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// Start the child process. 
	if (!CreateProcess(NULL,   // No module name (use command line)
		path,        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi)           // Pointer to PROCESS_INFORMATION structure
		)
	{
		DWORD lastError = GetLastError();
		printf("CreateProcess failed (%d).\n", lastError);
		return;
	}

	// Wait until child process exits.
	WaitForSingleObject(pi.hProcess, INFINITE);

	// Close process and thread handles. 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}



