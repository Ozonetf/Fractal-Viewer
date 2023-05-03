#pragma once
#include "DeviceResources.h"

struct pGBRA32
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
	pGBRA32(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a)
	{
		r = _r;
		g = _g;
		b = _b;
		a = _a;
	}
	pGBRA32()
	{
		r = g = b = a = 0;
	}
}; 

class Graphics : public DX::IDeviceNotify
{
	//ID2D1SolidColorBrush* _brush;
	std::unique_ptr<DX::DeviceResources> _deviceResource;
	Microsoft::WRL::ComPtr<ID2D1Bitmap1> _myBitMap;
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> _brush;
	Microsoft::WRL::ComPtr<IWICImagingFactory> _WICFactory;
	Microsoft::WRL::ComPtr<IWICBitmap > _WICBitmap;
	Microsoft::WRL::ComPtr<IWICBitmapLock> _WICBitmapLock;
public:
	Graphics();
	~Graphics();

	void init(HWND windowhandle, long width, long height);
	void BeginDraw() { _deviceResource->GetD2DContext()->BeginDraw(); }
	void EndDraw() { if(_deviceResource->GetD2DContext()->EndDraw()== D2DERR_RECREATE_TARGET) throw 20; }
	void Present() { _deviceResource->Present(); };

	void SetWindow(HWND windowhandle, long width, long height);
	void ClearScreen(float r, float g, float b);
	void DrawCircle(float x, float y, float radius, float r, float g, float b, float a);
	//draws an pixel with proper scale using a zoom factor 
	void DrawRect(D2D1_RECT_F r, D2D1::ColorF colour);
	void FillRect(DirectX::SimpleMath::Vector2 V, float zoom);
	void FillRect(DirectX::SimpleMath::Vector2 V, float zoom, D2D1::ColorF colour);
	void FillRect(D2D1_RECT_F r, D2D1::ColorF colour);
	void Resize(long width, long height);

	void CopyScreenToBitmap(std::vector<DirectX::SimpleMath::Color>* src);
	void DrawSavedBitmap();

	RECT GetWinRect() { return _windowRect; };
	long GetWinHeight() { return (_windowRect.bottom * m_dpi / 96); }
	long GetWinWidth() { return (_windowRect.right * m_dpi / 96); }


private:
	int m_dpi;
	void CreateWinSizeDepedentResources();
	HWND _windowHandle;
	RECT _windowRect;
	// Inherited via IDeviceNotify
	virtual void OnDeviceLost() override;
	virtual void OnDeviceRestored() override;
};