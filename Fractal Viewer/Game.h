#pragma once
#include "Graphics.h"
#include "Main.h"

class Game 
{
	std::unique_ptr<Graphics>	_graphics;
	std::vector<int> pixels;
public:
	Game();
	~Game();

	void Init(HWND windowhandle, long width, long height);

	void Tick();
	void Render();

	void OnWindowSizeChanged(long width, long height);
private:
	void Update();
	void ProcessInputs();
	void drawFractal();
	int	 getDepth(DirectX::SimpleMath::Vector2 c);
D2D1::ColorF HSL2RGB(int n);
	int circle_x = 0;
	std::unique_ptr<StepTimer> _timer;
	std::unique_ptr<DirectX::Keyboard> _keyboard;
	std::unique_ptr<DirectX::Mouse> m_mouse;
	DirectX::Mouse::ButtonStateTracker _mTraker;
	DirectX::Keyboard::KeyboardStateTracker _kTraker;

	std::queue<DirectX::SimpleMath::Vector2> _renderQueue;
	bool reCalc = false;
	bool paused = true;
	int _zoom = 1;
	int _scrollTemp = 0;
	int _zoomFactor = 120;
	int _zoomRatioX = 0;
	int _zoomRatioY = 0;
	float _pixelScale = 0;
	int _bailOut = 50;
	DirectX::SimpleMath::Vector2 _cameraCoord = DirectX::SimpleMath::Vector2(0, 0);
	int speed = 5;
	D2D1_RECT_F _selectBox;


};