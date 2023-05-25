#pragma once
#include "Graphics.h"
#include "Main.h"

struct PatchDesc
{
	float	unit;			//unit on cartisian plane per pixel
	float	w;				//width of render
	float	yEnd;			
	int		yStart;			
	int		AAdepth;
	int		bailOut;
	int		id;				//job id
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

	std::unique_ptr<StepTimer> m_timer;
	std::unique_ptr<DirectX::Keyboard> m_keyboard;
	std::unique_ptr<DirectX::Mouse> m_mouse;
	DirectX::Mouse::ButtonStateTracker m_mTraker;
	DirectX::Keyboard::KeyboardStateTracker m_kTraker;

	bool reCalc = false;
	bool paused = true;
	bool useMultiThread = false;
	int m_bailOut = 50;
	int m_AADepth = 4;
	D2D1_RECT_F m_selectBox;
	D2D1_RECT_F m_targetRegin;
	std::vector<std::thread> m_threadPool;
};
