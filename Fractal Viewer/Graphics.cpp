#include "pch.h"
#include "Graphics.h"
#include "Main.h"

Graphics::Graphics()
{
	_brush = nullptr;	
	// Renders only 2D, so no need for a depth buffer.
	_deviceResource = std::make_unique<DX::DeviceResources>(DXGI_FORMAT_B8G8R8A8_UNORM,
		DXGI_FORMAT_UNKNOWN);
	_deviceResource->RegisterDeviceNotify(this);
}

Graphics::~Graphics()
{	
	//_WICFactory.Reset();
	//_WICBitmap.Reset();
}

void Graphics::init(HWND windowhandle, long width, long height)
{
	SetWindow(windowhandle, width, height);
	_deviceResource->SetWindow(windowhandle, width, height);   
	_deviceResource->CreateDeviceIndependentResources();
	_deviceResource->CreateDeviceResources();

	_deviceResource->CreateWindowSizeDependentResources();
	DX::ThrowIfFailed(
		CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&_WICFactory)
		)
	);
	DX::ThrowIfFailed(
		_deviceResource->GetD2DContext()->CreateSolidColorBrush(D2D1::ColorF(0, 0, 0), &_brush)
	);
	CreateWinSizeDepedentResources();
}

void Graphics::SetWindow(HWND windowhandle, long width, long height)
{
	m_windowHandle = windowhandle;
	m_windowRect.top, m_windowRect.left = 0;
	m_windowRect.right = width;
	m_windowRect.bottom = height;
}

void Graphics::ClearScreen(float r, float g, float b)
{
	// Clear the views.
	auto context = _deviceResource->GetD3DDeviceContext();
	auto renderTarget = _deviceResource->GetRenderTargetView();

	//----------dont need zbuffer for 2d rendering---------------
	//auto depthStencil = _deviceResource->GetDepthStencilView();	
	//context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	//-----------------------------------------------------------

	context->ClearRenderTargetView(renderTarget, DirectX::XMVECTORF32{r, g, b});
	context->OMSetRenderTargets(1, &renderTarget, nullptr);

	// Set the viewport.
	auto const viewport = _deviceResource->GetScreenViewport();
	context->RSSetViewports(1, &viewport);
}

void Graphics::DrawCircle(float x, float y, float radius, float r, float g, float b, float a)
{
	_brush->SetColor(D2D1::ColorF(r, g, b, a));
	_deviceResource->GetD2DContext()->DrawEllipse(
		D2D1::Ellipse(D2D1::Point2F(x, y), radius, radius),
		_brush.Get(),
		3.0f
	);
}

void Graphics::DrawRect(D2D1_RECT_F r, D2D1::ColorF colour)
{
	_brush->SetColor(colour);
	_deviceResource->GetD2DContext()->DrawRectangle(r, _brush.Get());
}

void Graphics::FillRect(DirectX::SimpleMath::Vector2 V, float zoom)
{
	D2D1_RECT_F r;
	r.top = V.y ;
	r.left = V.x ;
	r.bottom = r.top + zoom ;
	r.right = r.left + zoom ;
	_deviceResource->GetD2DContext()->FillRectangle(r, _brush.Get());
}

void Graphics::FillRect(DirectX::SimpleMath::Vector2 V, float zoom, D2D1::ColorF colour)
{
	D2D1_RECT_F r;
	r.top = V.y;
	r.left = V.x;
	r.bottom = r.top + zoom;
	r.right = r.left + zoom;
	_brush->SetColor(colour);
	_deviceResource->GetD2DContext()->FillRectangle(r, _brush.Get());
}

void Graphics::FillRect(D2D1_RECT_F r, D2D1::ColorF colour)
{
	_brush->SetColor(colour);
	_deviceResource->GetD2DContext()->FillRectangle(r, _brush.Get());
}

void Graphics::Resize(long width, long height)
{
	if (!_deviceResource->WindowSizeChanged(width, height))	return;
	m_windowRect.left, m_windowRect.top = 0;
	m_windowRect.right = width;
	m_windowRect.bottom = height;
	CreateWinSizeDepedentResources();
}

void Graphics::CopyScreenToBitmap(std::vector<DirectX::SimpleMath::Color>* src)
{
	UINT cbBufferSize = 0, stride = 0, foo;
	BYTE* pv = NULL;
	WICRect rcLock = { 0, 0, m_windowRect.right, m_windowRect.bottom };
	_WICBitmap->Lock(&rcLock, WICBitmapLockWrite, _WICBitmapLock.GetAddressOf());
	_WICBitmapLock->GetDataPointer(&cbBufferSize, &pv);
	_WICBitmapLock->GetStride(&stride);
	int i = -1;
	DirectX::PackedVector::XMCOLOR xm;
	for (auto iter : *src)
	{
		xm = iter.BGRA();
		pv[i++]	= xm.b;		//B
		pv[i++] = xm.g;		//G
		pv[i++] = xm.r;		//R
		pv[i++] = xm.a;		//A
	}
	_WICBitmapLock.Reset();	//release the lock after using
	DX::ThrowIfFailed(
		_deviceResource->GetD2DContext()->CreateBitmapFromWicBitmap(_WICBitmap.Get(), _myBitMap.GetAddressOf())
	);
}

void Graphics::DrawSavedBitmap()
{
	_deviceResource->GetD2DContext()->DrawBitmap(_myBitMap.Get(), NULL, 1.0f, D2D1_INTERPOLATION_MODE_LINEAR);
}

void Graphics::CreateWinSizeDepedentResources()
{
	m_dpi = GetDpiForWindow(m_windowHandle);
	_deviceResource->CreateWindowSizeDependentResources();
	D2D1_SIZE_U WinSize = { static_cast<UINT32>(m_windowRect.right * m_dpi / 96.0f), static_cast<UINT32>(m_windowRect.bottom * m_dpi / 96.0f) };
	D2D1_BITMAP_PROPERTIES1 bitmapProperties;
	bitmapProperties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	bitmapProperties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
	bitmapProperties.dpiX = m_dpi;
	bitmapProperties.dpiY = m_dpi;
	bitmapProperties.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET;
	bitmapProperties.colorContext = nullptr;
	_WICBitmap.Reset();
	//create WIC bitmap the size of render target
	DX::ThrowIfFailed(
		_WICFactory->CreateBitmap(
			WinSize.width,
			WinSize.height,
			GUID_WICPixelFormat32bppPBGRA,	//direct2d uses 32bppPBGRA pixelformat
			WICBitmapCacheOnDemand,			
			_WICBitmap.GetAddressOf()
		)
	);
	DX::ThrowIfFailed(
		_deviceResource->GetD2DContext()->CreateBitmap(
			WinSize,
			nullptr,
			0,
			&bitmapProperties,
			_myBitMap.ReleaseAndGetAddressOf()
		)
	);
}

void Graphics::OnDeviceLost()
{
}

void Graphics::OnDeviceRestored()
{
}
