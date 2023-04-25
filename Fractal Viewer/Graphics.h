#pragma once
class Graphics
{
	ID2D1Factory* _factory;
	ID2D1HwndRenderTarget* _renderTarget;
	ID2D1SolidColorBrush* _brush;
	
public:
	Graphics();
	~Graphics();

	bool init(HWND windowhandle);
	void BeginDraw() { _renderTarget->BeginDraw(); }
	void EndDraw() { if(_renderTarget->EndDraw()== D2DERR_RECREATE_TARGET) throw 20; }

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
	long GetWinHeight() { return (_windowRect.bottom*DPI/96); }
	long GetWinWidth() { return (_windowRect.right*DPI/96); }


private:
	int DPI;
	void CreateWinSizeDepedentResources();
	HWND _windowHandle;
	RECT _windowRect;
};