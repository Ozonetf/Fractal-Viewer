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
	if (_brush) SafeRelease(&_brush); 
}

bool Graphics::init(HWND windowhandle, long width, long height)
{
	SetWindow(windowhandle, width, height);
	_deviceResource->SetWindow(windowhandle, width, height);   
	_deviceResource->CreateDeviceIndependentResources();
	_deviceResource->CreateDeviceResources();

	_deviceResource->CreateWindowSizeDependentResources();
	DX::ThrowIfFailed(
		_deviceResource->GetD2DContext()->CreateSolidColorBrush(D2D1::ColorF(0, 0, 0), &_brush)
	);
	RECT rect;
	GetClientRect(windowhandle, &rect);
	return true;
}

void Graphics::SetWindow(HWND windowhandle, long width, long height)
{
	_windowHandle = windowhandle;
	_windowRect.top, _windowRect.left = 0;
	_windowRect.right = width;
	_windowRect.bottom = height;
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
	_deviceResource->GetD2DContext()->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(x, y), radius, radius), _brush, 3.0f);
	//_D2D1renderTarget->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(x, y), radius, radius), _brush, 3.0f);
}

void Graphics::DrawRect(D2D1_RECT_F r, D2D1::ColorF colour)
{
	_brush->SetColor(colour);
	_deviceResource->GetD2DContext()->DrawRectangle(r, _brush);
}

void Graphics::FillRect(DirectX::SimpleMath::Vector2 V, float zoom)
{
	D2D1_RECT_F r;
	r.top = V.y ;
	r.left = V.x ;
	r.bottom = r.top + zoom ;
	r.right = r.left + zoom ;
	_deviceResource->GetD2DContext()->FillRectangle(r, _brush);
}

void Graphics::FillRect(DirectX::SimpleMath::Vector2 V, float zoom, D2D1::ColorF colour)
{
	D2D1_RECT_F r;
	r.top = V.y;
	r.left = V.x;
	r.bottom = r.top + zoom;
	r.right = r.left + zoom;
	_brush->SetColor(colour);
	_deviceResource->GetD2DContext()->FillRectangle(r, _brush);
}

void Graphics::FillRect(D2D1_RECT_F r, D2D1::ColorF colour)
{
	_brush->SetColor(colour);
	_deviceResource->GetD2DContext()->FillRectangle(r, _brush);
}

void Graphics::Resize(long width, long height)
{
	if (!_deviceResource->WindowSizeChanged(width, height))	return;
	_windowRect.left, _windowRect.top = 0;
	_windowRect.right = width;
	_windowRect.bottom = height;
	//_D2D1renderTarget->Resize(D2D1::SizeU(width, height));
}

void Graphics::CreateWinSizeDepedentResources()
{
	_deviceResource->CreateWindowSizeDependentResources();
	//_D2D1renderTarget->Resize(D2D1::SizeU(_windowRect.right, _windowRect.bottom));
}

void Graphics::OnDeviceLost()
{
}

void Graphics::OnDeviceRestored()
{
}
