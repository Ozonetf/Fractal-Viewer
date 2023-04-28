#pragma once
#include "DeviceResources.h"
class Graphics : public DX::IDeviceNotify
{
	ID2D1SolidColorBrush* _brush;
	std::unique_ptr<DX::DeviceResources> _deviceResource;

public:
	Graphics();
	~Graphics();

	bool init(HWND windowhandle, long width, long height);
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

	RECT GetWinRect() { return _windowRect; };
	long GetWinHeight() { return _windowRect.bottom - _windowRect.top; }
	long GetWinWidth() { return _windowRect.right - _windowRect.left; }


private:
	void CreateWinSizeDepedentResources();
	HWND _windowHandle;
	RECT _windowRect;
	// Inherited via IDeviceNotify
	virtual void OnDeviceLost() override;
	virtual void OnDeviceRestored() override;
};