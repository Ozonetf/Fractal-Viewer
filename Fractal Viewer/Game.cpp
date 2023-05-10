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
	_graphics->init(windowhandle, width, height);
	//_cameraCoord.x = width / 2;
	//_cameraCoord.y = height / 2;	
	//_zoomRatioX = width / 48;
	//_zoomRatioY = height / 48;
	//circle_x = 0;
	pixelColours = std::vector<DirectX::SimpleMath::Color>(width * height);
	_targetRegin = 	{ 
		-2 * ((float)width / (float)height), 
		-2, 
		2 * ((float)width / (float)height),
		2 
	};
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
		CalculateFractal();
	}
	_graphics->DrawRect(_selectBox, D2D1::ColorF::White);
	//_graphics->DrawCircle(circle_x, _graphics->GetWinHeight()/2, 30, 1, 1, 1, 1);
	_graphics->DrawSavedBitmap();
	_graphics->EndDraw();
	if (reCalc)
	{
		_graphics->CopyScreenToBitmap(&pixelColours);
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
	if (_kTraker.pressed.P)
	{
		_targetRegin = {
		-2 * ((float)_graphics->GetWinWidth() / (float)_graphics->GetWinHeight()),
		-2,
		2 * ((float)_graphics->GetWinWidth() / (float)_graphics->GetWinHeight()),
		2
		};
		reCalc = true;
	}
	if (_kTraker.pressed.Right)
	{
		_bailOut += 10; 
		CalculateFractal();
	}
	if (_kTraker.pressed.Left)
	{
		_bailOut -= 10; 
		CalculateFractal();
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
		ResizeViewport();
		CalculateFractal();
		_selectBox = { 0, 0, 0, 0 };
	}
	_zoom -= (_scrollTemp - mouse.scrollWheelValue) / _zoomFactor;
	_scrollTemp = mouse.scrollWheelValue;
	if (_zoom < 1) _zoom = 1;
}

void Game::OnWindowSizeChanged(long width, long height)
{
	_graphics->Resize(width, height);
	pixelColours.resize(width * height);
	//keep camera centered when resizing window
	_cameraCoord.x = width / 2;
	_cameraCoord.y = height / 2;
	_zoomRatioX = width / 48;
	_zoomRatioY = height / 48;
	_targetRegin = {
	-2 * ((float)width / (float)height),
	-2,
	2 * ((float)width / (float)height),
	2
	};
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
	return ret;
}

pGBRA32 Game::HSL2RGB(int n)
{
	float C, X, S, L, Hdot, H;
	const float Lightnes  = .5f;
	L = (1 - std::abs(2 * Lightnes - 1));
	S = ((float)n / (float)_bailOut);
	C = S * L;
	H = (360.0f / (float)_bailOut) * n;
	Hdot = (float)std::abs(std::fmod(H, 2) - 1);
	X = C * (1 - Hdot);
	uint8_t iC = static_cast<uint8_t>(C*255), iX = static_cast<uint8_t>(X * 255);
	switch (int(H)/60)
	{
	default:
	case 0: return pGBRA32{ iC, iX, 0, 0 }; break;
	case 1: return pGBRA32{ iX, iC, 0, 0 }; break;
	case 2: return pGBRA32{ 0, iC, iX, 0 }; break;
	case 3: return pGBRA32{ 0, iX, iC, 0 }; break;
	case 4: return pGBRA32{ iX, 0, iC, 0 }; break;
	case 5: return pGBRA32{ iC, 0, iX, 0 }; break;
	}
}

DirectX::SimpleMath::Color Game::HSL2RGBf(int n)
{
	if (n == _bailOut)
	{
		return DirectX::SimpleMath::Color{0, 0, 0, 0};
	}
	float C, X, S, L, Hdot, H;
	const float Lightnes = .5f;
	L = (1 - std::abs(2 * Lightnes - 1));
	S = ((float)n / (float)_bailOut);
	C = S * L;
	H = (360.0f / (float)_bailOut) * n;
	Hdot = (float)std::abs(std::fmod(H, 2) - 1);
	X = C * (1 - Hdot);
	switch (int(H) / 60)
	{
	case 0: return DirectX::SimpleMath::Color{ C, X, 0, 1 }; break;
	case 1: return DirectX::SimpleMath::Color{ X, C, 0, 1 }; break;
	case 2: return DirectX::SimpleMath::Color{ 0, C, X, 1 }; break;
	case 3: return DirectX::SimpleMath::Color{ 0, X, C, 1 }; break;
	case 4: return DirectX::SimpleMath::Color{ X, 0, C, 1 }; break;
	case 5: return DirectX::SimpleMath::Color{ C, 0, X, 1 }; break;
	case 6: return DirectX::SimpleMath::Color{}; break;
	}
}

void Game::CalculateFractal()
{
	float w, h;
	int iter = 0;
	w = (float)_graphics->GetWinWidth();
	h = (float)_graphics->GetWinHeight();
	float unit = (_targetRegin.right - _targetRegin.left) / (float)w;
	float halfWidth = w / 2.0f;
	float halfHeight = h / 2.0f;
	DirectX::SimpleMath::Vector2 v;
	auto start = std::chrono::steady_clock::now();

	for (size_t i = 0; i < h; i++)
	{
		v.y = _targetRegin.top + i * unit;
		for (size_t j = 0; j < w; j++)
		{
			v.x = _targetRegin.left + unit * j;
			pixelColours.at(iter) = HSL2RGBf(getDepth(v));
			iter++;
		}
	}
	auto end = std::chrono::steady_clock::now();
	auto total = end - start;
	PRINT_DEBUG("%d	milli\n", std::chrono::duration_cast<std::chrono::milliseconds>(total).count());
	start = std::chrono::steady_clock::now();
	_graphics->CopyScreenToBitmap(&pixelColours);
	end = std::chrono::steady_clock::now();
	total = end - start;
	PRINT_DEBUG("copy:	%d	milli\n", std::chrono::duration_cast<std::chrono::milliseconds>(total).count());
}

void Game::ResizeViewport()
{
	if (_selectBox.bottom < _selectBox.top)
	{
		FLOAT temp = _selectBox.top;
		_selectBox.top = _selectBox.bottom;
		_selectBox.bottom = temp;
	}
	if (_selectBox.left > _selectBox.right)
	{
		FLOAT temp = _selectBox.left;
		_selectBox.left = _selectBox.right;
		_selectBox.right = temp;
	}
	float w = (float)_graphics->GetWinWidth();
	float h = (float)_graphics->GetWinHeight();
	float unit = (_targetRegin.bottom - _targetRegin.top) / h;
	//we want to keep the render in correct ratio to the current window, so we calculate
	//the new target regin that contain the user's selection while maintaining ratio
	
	//if select box is taller in ratio than current window
	if (std::abs(_selectBox.bottom - _selectBox.top) > std::abs(_selectBox.left - _selectBox.right))
	{
		float ratio = w / h, offset, center;
		offset = (_selectBox.bottom - _selectBox.top) * .5f * ratio;
		center = (_selectBox.left + _selectBox.right) / 2.f;
		_targetRegin.top += _selectBox.top * unit;
		_targetRegin.bottom -= (h - _selectBox.bottom) * unit;
		_targetRegin.left += (center-offset) * unit;
		_targetRegin.right -= (w - (center + offset)) * unit;
	}
	//if select box is wider in ratio than current window
	else
	{
		float ratio = h / w, offset, center;
		offset = (_selectBox.right - _selectBox.left) * .5f * ratio;
		center = (_selectBox.bottom + _selectBox.top) * .5f;
		_targetRegin.left += _selectBox.left * unit;
		_targetRegin.right -= (w - _selectBox.right) * unit;
		_targetRegin.top += (center - offset) * unit;
		_targetRegin.bottom -= (h - (center + offset)) * unit;
	}
}

