#include "pch.h"
#include "Game.h"

Game::Game()
{

}

Game::~Game()
{

}

void Game::Init(HWND windowhandle, long width, long height)
{
	_graphics = std::make_unique<Graphics>();
	_timer = std::make_unique<StepTimer>();
	_keyboard = std::make_unique<DirectX::Keyboard>();
	m_mouse = std::make_unique<DirectX::Mouse>();
	m_mouse->SetWindow(windowhandle);
	//sets the window handle as ther rendertarget for d2d
	if (!_graphics->init(windowhandle, width, height))
	{
		throw 20;
	};
	//_cameraCoord.x = width / 2;
	//_cameraCoord.y = height / 2;	
	//_zoomRatioX = width / 48;
	//_zoomRatioY = height / 48;
	//circle_x = 0;
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
	//if (circle_x>_graphics->GetWinWidth())
	//{
	//	circle_x = 0;
	//}
	//else if (circle_x < 0)
	//{
	//	circle_x = _graphics->GetWinWidth();
	//}
	//else
	//{
	//	circle_x += speed;
	//}


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
	_graphics->ClearScreen(0, 0, 0);

	//rect's right and bottom are not insclusive in fillrect, workaround to avoid gap
	//in cells when zoomed in
	float zoom = (_pixelScale == 1 ? 1 : _pixelScale+1);
	if (reCalc) 
	{
		drawFractal();
	}
	_graphics->DrawRect(_selectBox, D2D1::ColorF::White);
	//_graphics->DrawCircle(circle_x, _graphics->GetWinHeight()/2, 30, 1, 1, 1, 1);
	_graphics->EndDraw();
	if (reCalc)
	{
		_graphics->CopyScreenToBitmap();
		reCalc = false;
	}
	_graphics->Present();

}

void Game::ProcessInputs()
{
	using ButtonState = DirectX::Mouse::ButtonStateTracker::ButtonState;
	using namespace DirectX::SimpleMath;
	//KEYBOARD
	auto kb = _keyboard->GetState();
	_kTraker.Update(kb);
	if (_kTraker.pressed.A) speed = -speed;
	if (_kTraker.pressed.P) reCalc = true;
	if (_kTraker.pressed.Right)
	{
		_bailOut += 10; 
		drawFractal();
	}
	if (_kTraker.pressed.Left)
	{
		_bailOut -= 10; 
		drawFractal();
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
	if (_mTraker.leftButton == ButtonState::PRESSED)
	{
		_selectBox.left = _selectBox.right = mouse.x;
		_selectBox.top = _selectBox.bottom = mouse.y;
	}
	else if(_mTraker.leftButton == ButtonState::HELD)
	{
		_selectBox.right = mouse.x;
		_selectBox.bottom = mouse.y;
	}
	else if(_mTraker.leftButton == ButtonState::RELEASED)
	{
		_selectBox = { 0, 0, 0, 0 };
	}
	_zoom -= (_scrollTemp - mouse.scrollWheelValue) / _zoomFactor;
	_scrollTemp = mouse.scrollWheelValue;
	if (_zoom < 1) _zoom = 1;
}

void Game::OnWindowSizeChanged(long width, long height)
{
	PRINT_DEBUG("this resized");
	_graphics->Resize(width, height);
	pixels.resize(width * height);
	//keep camera centered when resizing window
	_cameraCoord.x = width / 2;
	_cameraCoord.y = height / 2;
	_zoomRatioX = width / 48;
	_zoomRatioY = height / 48;
	PRINT_DEBUG("x ratio: %d, y ratio: %d\n", _zoomRatioX, _zoomRatioY);
}

int Game::getDepth(DirectX::SimpleMath::Vector2 c)
{
	DirectX::SimpleMath::Vector2 v1{ 0, 0 };
	int ret = 0;
	float temp, BBB = (v1.x * v1.x + v1.y * v1.y);
	while (ret < _bailOut && BBB <= 4.0f)
	{
		temp = v1.x * v1.x - v1.y * v1.y + c.x;
		v1.y = (v1.x + v1.x) * v1.y + c.y;
		v1.x = temp;
		ret++;
		BBB = (v1.x * v1.x + v1.y * v1.y);
	}
	if (ret>25)
	{
		return ret;
	}
	return ret;
}

D2D1::ColorF Game::HSL2RGB(int n)
{
	float C, X, S, L, Hdot, H;
	const float Lightnes  = .5f;
	L = (1 - std::abs(2 * Lightnes - 1));
	S = ((float)n / (float)_bailOut);
	C = S * L;
	H = (360.0f / (float)_bailOut) * n;
	Hdot = (float)std::abs(std::fmod(H, 2) - 1);
	X = C * (1 - Hdot);
	switch (int(H)/60)
	{
	case 0: return D2D1::ColorF{ C, X, 0 }; break;
	case 1: return D2D1::ColorF{ X, C, 0 }; break;
	case 2: return D2D1::ColorF{ 0, C, X }; break;
	case 3: return D2D1::ColorF{ 0, X, C }; break;
	case 4: return D2D1::ColorF{ X, 0, C }; break;
	case 5: return D2D1::ColorF{ C, 0, X }; break;
	}
}

void Game::drawFractal()
{
	auto start = std::chrono::steady_clock::now();
	float hue;
	int iter = 0, w, h;
	w = _graphics->GetWinWidth();
	h = _graphics->GetWinHeight();
	float halfWidth = (float)w / 2.0f;
	float halfHeight = (float)h / 2.0f;
	DirectX::SimpleMath::Vector2 v;
	for (size_t i = 0; i < h; i++)
	{
		for (size_t j= 0; j < w; j++)
		{
			v.x = (float(j) - halfWidth) / (halfHeight /2.0f);
			v.y = (float(i) - halfHeight) / (halfHeight /2.0f);
			pixels.at(iter) = getDepth(v);
			iter++;
		}
	}
	auto end = std::chrono::steady_clock::now();
	auto total = end - start;
	PRINT_DEBUG("%d	milli\n", std::chrono::duration_cast<std::chrono::milliseconds>(total).count());
	start = std::chrono::steady_clock::now();
	iter = 0;
	//_graphics->BeginDraw();
	for (size_t i = 0; i < h; i++)
	{
		for (size_t j = 0; j < w; j++)
		{
			hue = (float)std::abs(pixels.at(iter) - _bailOut);
			_graphics->FillRect(DirectX::SimpleMath::Vector2{ (float)j, (float)i }, 1, HSL2RGB(hue));
			iter++;
		}
	}
	//reCalc = false;

	end = std::chrono::steady_clock::now();
	total = end - start;
	PRINT_DEBUG("render:	%d	milli\n", std::chrono::duration_cast<std::chrono::milliseconds>(total).count());
	//_graphics->EndDraw();
}

