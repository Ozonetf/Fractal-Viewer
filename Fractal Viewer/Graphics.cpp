#include "pch.h"
#include "Graphics.h"
#include "Main.h"

Graphics::Graphics()
{
	_factory = NULL;
	_renderTarget = NULL;
	_brush = NULL;
}

Graphics::~Graphics()
{
	if (_factory) SafeRelease(&_factory);
	if (_renderTarget) SafeRelease(&_renderTarget);
	if (_brush) SafeRelease(&_brush); 
}

bool Graphics::init(HWND windowhandle)
{
	HRESULT res = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &_factory);
	if (res != S_OK) return false;	//if factory didnt create succesfully
	RECT rect;
	GetClientRect(windowhandle, &rect);

	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties();
	//ignore the alpha chennel since we're not interested, slight optimization
	props.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;	
	res = _factory->CreateHwndRenderTarget(
		props,
		D2D1::HwndRenderTargetProperties(windowhandle, D2D1::SizeU(rect.right, rect.bottom)), 
		&_renderTarget
		);
	if (res != S_OK) return false;

	D2D1_ALPHA_MODE::D2D1_ALPHA_MODE_IGNORE;
	res = _renderTarget->CreateSolidColorBrush(D2D1::ColorF(0, 0, 0), &_brush);
	if (res != S_OK) return false;
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
	_renderTarget->Clear(D2D1::ColorF(r, g, b));
}

void Graphics::DrawCircle(float x, float y, float radius, float r, float g, float b, float a)
{
	_brush->SetColor(D2D1::ColorF(r, g, b, a));
	_renderTarget->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(x, y), radius, radius), _brush, 3.0f);
}

void Graphics::DrawRect(D2D1_RECT_F r, D2D1::ColorF colour)
{
	_brush->SetColor(colour);
	_renderTarget->DrawRectangle(r, _brush);
}

void Graphics::FillRect(DirectX::SimpleMath::Vector2 V, float zoom)
{
	D2D1_RECT_F r;
	r.top = V.y ;
	r.left = V.x ;
	r.bottom = r.top + zoom ;
	r.right = r.left + zoom ;
	_renderTarget->FillRectangle(r, _brush);
}

void Graphics::FillRect(DirectX::SimpleMath::Vector2 V, float zoom, D2D1::ColorF colour)
{
	D2D1_RECT_F r;
	r.top = V.y;
	r.left = V.x;
	r.bottom = r.top + zoom;
	r.right = r.left + zoom;
	_brush->SetColor(colour);
	_renderTarget->FillRectangle(r, _brush);
}

void Graphics::FillRect(D2D1_RECT_F r, D2D1::ColorF colour)
{
	_brush->SetColor(colour);
	_renderTarget->FillRectangle(r, _brush);
}

void Graphics::Resize(long width, long height)
{
	_windowRect.left, _windowRect.top = 0;
	_windowRect.right = width;
	_windowRect.bottom = height;
	_renderTarget->Resize(D2D1::SizeU(width, height));
}

void Graphics::CreateWinSizeDepedentResources()
{
	_renderTarget->Resize(D2D1::SizeU(_windowRect.right, _windowRect.bottom));
}
