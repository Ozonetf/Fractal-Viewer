#include "pch.h"
#include "Game.h"
#include <exception>

Game::Game()
{
}

Game::~Game()
{
}

void Game::Init(HWND windowhandle, int width, int height)
{
	_graphics = std::make_unique<Graphics>();
	_timer = std::make_unique<StepTimer>();
	_keyboard = std::make_unique<DirectX::Keyboard>();
	m_mouse = std::make_unique<DirectX::Mouse>();
	m_mouse->SetWindow(windowhandle);

	//sets the window handle as ther rendertarget for d2d
	if (!_graphics->init(windowhandle))
	{
		throw 20;
	};
	_cameraCoord.x = width / 2;
	_cameraCoord.y = height / 2;	
	_zoomRatioX = width / 48;
	_zoomRatioY = height / 48;
	circle_x = 0;
	getDepth(DirectX::SimpleMath::Vector2{ -0.4f, 0.3f });
	pixels = std::vector<int>(width * height, 0);
}

void Game::Tick()
{
	_timer->Tick([&]()
	{
		Update();
	});
	//PRINT_DEBUG("frame time: %d\n", _timer->GetFramesPerSecond());
	Render();
}

void Game::Update()
{
	using namespace DirectX::SimpleMath;
	ProcessInputs();
	if (circle_x>_graphics->GetWinWidth())
	{
		circle_x = 0;
	}
	else if (circle_x < 0)
	{
		circle_x = _graphics->GetWinWidth();
	}
	else
	{
		circle_x += speed;
	}


	//figures out which pixels are in camera's view, get their relative postion and push to render queue
	//cam coord is center of the screen
	int left, right, top, bottom;
	int height = _graphics->GetWinHeight();
	int width = _graphics->GetWinWidth();
	left = _cameraCoord.x - (width / 2) + ((_zoom - 1) * _zoomRatioX);
	right = _cameraCoord.x + (width / 2) - ((_zoom - 1) * _zoomRatioX);
	top = _cameraCoord.y - (height / 2) + ((_zoom - 1) * _zoomRatioY);
	bottom = _cameraCoord.y + (height / 2) - ((_zoom - 1) * _zoomRatioY);
	float xx, yy;
	xx = float(width) / (right - left);
	yy = float(height) / (bottom - top);

	_pixelScale = float(width) / float(right - left);
}

void Game::Render()
{
	_graphics->BeginDraw();
	_graphics->ClearScreen(0.0f, 0.0f, 0.0f);

	//rect's right and bottom are not insclusive in fillrect, workaround to avoid gap
	//in cells when zoomed in
	float zoom = (_pixelScale == 1 ? 1 : _pixelScale+1);
	drawFractal();
	_graphics->DrawCircle(circle_x, _graphics->GetWinHeight()/2, 30, 1, 1, 1, 1);
	_graphics->EndDraw();
}

void Game::ProcessInputs()
{
	using ButtonState = DirectX::Mouse::ButtonStateTracker::ButtonState;
	using namespace DirectX::SimpleMath;
	//KEYBOARD
	auto kb = _keyboard->GetState();
	_kTraker.Update(kb);
	if (_kTraker.pressed.A) speed = -speed;
	if (kb.Right)
	{
		_cameraCoord.x += 10;
	}
	if (kb.Left)
	{
		_cameraCoord.x -= 10;
	}
	if (kb.Up)
	{
		_cameraCoord.y -= 10;
	}
	if (kb.Down)
	{
		_cameraCoord.y += 10;
	}	
	if (_kTraker.pressed.LeftControl) _zoomFactor = 30; 
	//one "scroll" on MWheel typically 120 in value
	if (_kTraker.released.LeftControl) _zoomFactor = 120;

	//MOUSE
	auto mouse = m_mouse->GetState();
	_mTraker.Update(mouse);
	if (_mTraker.rightButton == ButtonState::PRESSED)
	{
		m_mouse->SetMode(DirectX::Mouse::MODE_RELATIVE);
	}
	else if (_mTraker.rightButton == ButtonState::RELEASED)
	{
		m_mouse->SetMode(DirectX::Mouse::MODE_ABSOLUTE);
	}
	else if (_mTraker.rightButton == ButtonState::HELD)
	{
		if (mouse.positionMode == DirectX::Mouse::MODE_RELATIVE)
		{
			_cameraCoord.x -= mouse.x;
			_cameraCoord.y -= mouse.y;
		}

	}
	if (mouse.leftButton)
	{
		int left, right, top;
		int height = _graphics->GetWinHeight();
		int width = _graphics->GetWinWidth();
		left = _cameraCoord.x - (width / 2) + ((_zoom - 1) * _zoomRatioX);
		right = _cameraCoord.x + (width / 2) - ((_zoom - 1) * _zoomRatioX);
		top = _cameraCoord.y - (height / 2) + ((_zoom - 1) * _zoomRatioY);
		_pixelScale = float(width) / float(right - left);
		float x = left + (mouse.x) / _pixelScale;
		float y = top + (mouse.y) / _pixelScale;
		activePixels.push_back(DirectX::SimpleMath::Vector2(int(x), int(y)));
		//PRINT_DEBUG("mouse x: %d, y: %d		x: %f y: %f\n", mouse.x, mouse.y, x, y);
	}
	_zoom -= (_scrollTemp - mouse.scrollWheelValue) / _zoomFactor;
	_scrollTemp = mouse.scrollWheelValue;
	if (_zoom < 1) _zoom = 1;
}

void Game::OnWindowSizeChanged(long width, long height)
{
	PRINT_DEBUG("this resized");
	_graphics->Resize(width, height);
	//keep camera centered when resizing window
	_cameraCoord.x = width / 2;
	_cameraCoord.y = height / 2;
	_zoomRatioX = width / 48;
	_zoomRatioY = height / 48;
	PRINT_DEBUG("x ratio: %d, y ratio: %d\n", _zoomRatioX, _zoomRatioY);
}

void Game::OnResize()
{
	//_graphics->Resize();
}

int getDepth(DirectX::SimpleMath::Vector2 c)
{
	DirectX::SimpleMath::Vector2 v1{ 0, 0 };
	int ret = 0;
	float temp, BBB = (v1.x * v1.x + v1.y * v1.y);
	while (ret < 1000 && BBB <= 4.0f)
	{
		temp = v1.x * v1.x - v1.y * v1.y + c.x;
		v1.y = (v1.x + v1.x) * v1.y + c.y;
		v1.x = temp;
		ret++;
		BBB = (v1.x * v1.x + v1.y * v1.y);
	}
	return ret;
}

void Game::drawFractal()
{
	auto start = std::chrono::steady_clock::now();
	int iter = 0;
	float d;
	float halfWidth = (float)_graphics->GetWinWidth() / 2.0f;
	float halfHeight = (float)_graphics->GetWinHeight() / 2.0f;
	DirectX::SimpleMath::Vector2 v;
	for (size_t i = 0; i < _graphics->GetWinHeight(); i++)
	{
		for (size_t j= 0; j < _graphics->GetWinWidth(); j++)
		{
			v.x = (float(j) - halfWidth) / (halfHeight/2);
			v.y = (float(i) - halfHeight) / (halfHeight/2);
			d = getDepth(v)/1000.0f;
			_graphics->FillRect(DirectX::SimpleMath::Vector2{(float)j, (float)i}, 1, D2D1::ColorF(d, 0.0f, 0.0f));
			iter++;
		}
	}
	auto end = std::chrono::steady_clock::now();
	auto total = end - start;
	PRINT_DEBUG("%d	milli\n", std::chrono::duration_cast<std::chrono::milliseconds>(total).count());
}

