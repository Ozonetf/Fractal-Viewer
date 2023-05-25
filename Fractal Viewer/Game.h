#pragma once
#include "Graphics.h"
#include "Main.h"

struct PatchDesc
{
	float	unit;		//unit on cartisian plane per pixel
	float	w;			//width of render
	float	yEnd;			//height of render
	int		yStart;			//pixel height of job
	int		AAdepth;
	int		bailOut;
	int		id;			//job id
	int		frameHeight;
	int		iter;
};

class Game 
{
	std::unique_ptr<Graphics>	_graphics;
	std::vector<DirectX::SimpleMath::Color> pixelColours;
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
	void CalculateFractal();
	void CalculateFractalMT();
	void GetDepthInRange(PatchDesc d);
	void ResizeViewport();
	int	 getDepth(DirectX::SimpleMath::Vector2 c);
	pGBRA32 HSL2RGB(int n);
	DirectX::SimpleMath::Color HSL2RGBf(int n);
	int circle_x = 0;
	std::unique_ptr<StepTimer> _timer;
	std::unique_ptr<DirectX::Keyboard> _keyboard;
	std::unique_ptr<DirectX::Mouse> m_mouse;
	DirectX::Mouse::ButtonStateTracker _mTraker;
	DirectX::Keyboard::KeyboardStateTracker _kTraker;

	std::queue<DirectX::SimpleMath::Vector2> _renderQueue;
	bool reCalc = false;
	bool paused = true;
	bool useMultiThread = false;
	int _zoom = 1;
	int _scrollTemp = 0;
	int _zoomFactor = 120;
	int _zoomRatioX = 0;
	int _zoomRatioY = 0;
	float _pixelScale = 0;
	int _bailOut = 50;
	int _AADepth = 4;
	DirectX::SimpleMath::Vector2 _cameraCoord = DirectX::SimpleMath::Vector2(0, 0);
	int speed = 5;
	D2D1_RECT_F _selectBox;
	D2D1_RECT_F _targetRegin;
	std::vector<std::thread> _threadPool;
};
