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
	m_timer = std::make_unique<StepTimer>();
	m_keyboard = std::make_unique<DirectX::Keyboard>();
	m_mouse = std::make_unique<DirectX::Mouse>();
	m_mouse->SetWindow(windowhandle);
	_graphics->init(windowhandle, width, height);
	pixelColours = std::vector<DirectX::SimpleMath::Color>(width * height);
	m_targetRegin = 	{ 
		-2 * ((float)width / (float)height), 
		-2, 
		2 * ((float)width / (float)height),
		2 
	};
	m_threadPool.resize(std::thread::hardware_concurrency());
}

void Game::Tick()
{
	m_timer->Tick([&]()
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
	if (reCalc)
	{
		if (useMultiThread)
		{
			CalculateFractalMT();
		}
		else
		{
			CalculateFractal();
		}
	}
}

void Game::Render()
{
	_graphics->BeginDraw();
	_graphics->ClearScreen(0, 0, 0);
	_graphics->DrawRect(m_selectBox, D2D1::ColorF::White);
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
	auto kb = m_keyboard->GetState();
	m_kTraker.Update(kb);
	if (m_kTraker.pressed.P)
	{
		m_targetRegin = {
		-2 * ((float)_graphics->GetWinWidth() / (float)_graphics->GetWinHeight()),
		-2,
		2 * ((float)_graphics->GetWinWidth() / (float)_graphics->GetWinHeight()),
		2
		};
		reCalc = true;
	}
	if (m_kTraker.pressed.M)
	{
		useMultiThread = !useMultiThread;
	}
	if (m_kTraker.pressed.Right)
	{
		m_bailOut += 10; 		
		reCalc = true;
	}
	if (m_kTraker.pressed.Left)
	{
		m_bailOut -= 10; 
		reCalc = true;
	}
	if (kb.Up)
	{
		m_AADepth *= 2;
		reCalc = true;
	}
	if (kb.Down)
	{
		m_AADepth /= 2 < 1 ? 1 : m_AADepth;
		reCalc = true;
	}	
	//MOUSE
	auto mouse = m_mouse->GetState();
	m_mTraker.Update(mouse);
	if (m_mTraker.leftButton == ButtonState::PRESSED)
	{
		m_selectBox.left = m_selectBox.right = mouse.x;
		m_selectBox.top = m_selectBox.bottom = mouse.y;
	}
	else if(m_mTraker.leftButton == ButtonState::HELD)
	{
		m_selectBox.right = mouse.x;
		m_selectBox.bottom = mouse.y;
	}
	else if(m_mTraker.leftButton == ButtonState::RELEASED)
	{
		ResizeViewport();
		reCalc = true;
	}
}

void Game::OnWindowSizeChanged(long width, long height)
{
	_graphics->Resize(width, height);
	pixelColours.resize(width * height);
	//keep camera centered when resizing window
	m_targetRegin = {
	-2 * ((float)width / (float)height),
	-2,
	2 * ((float)width / (float)height),
	2
	};
}

int Game::getDepth(DirectX::SimpleMath::Vector2 c)
{
	DirectX::SimpleMath::Vector2 v1{ 0, 0 };
	int ret = 0;
	float temp, BBB = (v1.x * v1.x + v1.y * v1.y);
	while (ret < m_bailOut && BBB <= 4.0f)
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
	S = ((float)n / (float)m_bailOut);
	C = S * L;
	H = (360.0f / (float)m_bailOut) * n;
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
	if (n == m_bailOut)
	{
		return DirectX::SimpleMath::Color{0, 0, 0, 0};
	}
	float C, X, S, L, Hdot, H;
	const float Lightnes = .5f;
	L = (1 - std::abs(2 * Lightnes - 1));
	S = ((float)n / (float)m_bailOut);
	C = S * L;
	H = (360.0f / (float)m_bailOut) * n;
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
	float w, h, curX, curY, patchsize = (float)m_AADepth * (float)m_AADepth;
	int iter = 0;
	w = (float)_graphics->GetWinWidth();
	h = (float)_graphics->GetWinHeight();
	float unit = (m_targetRegin.right - m_targetRegin.left) / (float)w;
	float AAunit = unit / (float)m_AADepth;
	DirectX::SimpleMath::Vector2 v;
	auto start = std::chrono::steady_clock::now();
	DirectX::SimpleMath::Color c, aafactor;
	aafactor = { patchsize, patchsize , patchsize };
	c = { 0, 0, 0, 0};
	for (size_t i = 0; i < h; i++)
	{
		curY = m_targetRegin.top + i * unit;
		for (size_t j = 0; j < w; j++)
		{
			curX = m_targetRegin.left + unit * j;
			for (size_t AAi = 0; AAi < m_AADepth; AAi++)
			{
				v.y += AAi * AAunit;
				v.x = curX;
				for (size_t AAj = 0; AAj < m_AADepth; AAj++)
				{
					v.x += AAj * AAunit;
					c += HSL2RGBf(getDepth(v));
				}
			}
			c /= aafactor;
			pixelColours.at(iter) = c;
			iter++;
			v.y = curY;
			c = { 0, 0, 0, 0};
		}
	}
	auto end = std::chrono::steady_clock::now();
	auto total = end - start;
	PRINT_DEBUG("singlethread calc time:	%d	milli\n", std::chrono::duration_cast<std::chrono::milliseconds>(total).count());
	start = std::chrono::steady_clock::now();
	_graphics->CopyScreenToBitmap(&pixelColours);
	end = std::chrono::steady_clock::now();
	total = end - start;
	PRINT_DEBUG("copy:	%d	milli\n", std::chrono::duration_cast<std::chrono::milliseconds>(total).count());
}

void Game::CalculateFractalMT()
{
	auto start = std::chrono::steady_clock::now();
	PatchDesc patchDesc = {};
	patchDesc.w = (float)_graphics->GetWinWidth();
	patchDesc.unit = (m_targetRegin.right - m_targetRegin.left) / patchDesc.w;
	patchDesc.AAdepth = m_AADepth;
	patchDesc.bailOut = m_bailOut;
	int threadCount = m_threadPool.size();
	int patchHeight = _graphics->GetWinHeight() / threadCount;
	patchDesc.yEnd = 0;
	for (size_t i = 0; i < threadCount; i++)
	{
		patchDesc.iter = i * patchDesc.w * patchHeight;
		patchDesc.id = i;
		patchDesc.yStart = patchHeight * i;
		patchDesc.yEnd += patchHeight;
		if (i == threadCount-1)	//last job handles remainding pixels
		{
			patchDesc.yEnd += _graphics->GetWinHeight() % threadCount;
		}
		m_threadPool[i] = std::thread(&Game::GetDepthInRange, this, patchDesc);

	}
	for (auto& t : m_threadPool)
	{
		t.join();
	}
	auto end = std::chrono::steady_clock::now();
	auto total = end - start;
	PRINT_DEBUG("multithreaded calc time:	%d	milli\n", std::chrono::duration_cast<std::chrono::milliseconds>(total).count());
	_graphics->CopyScreenToBitmap(&pixelColours);
}

void Game::ResizeViewport()
{
	if (m_selectBox.bottom < m_selectBox.top)
	{
		FLOAT temp = m_selectBox.top;
		m_selectBox.top = m_selectBox.bottom;
		m_selectBox.bottom = temp;
	}
	if (m_selectBox.left > m_selectBox.right)
	{
		FLOAT temp = m_selectBox.left;
		m_selectBox.left = m_selectBox.right;
		m_selectBox.right = temp;
	}
	float w = (float)_graphics->GetWinWidth();
	float h = (float)_graphics->GetWinHeight();
	float unit = (m_targetRegin.bottom - m_targetRegin.top) / h;
	//we want to keep the render in correct ratio to the current window, so we calculate
	//the new target regin that contain the user's selection while maintaining ratio
	
	//if select box is taller in ratio than current window
	if (std::abs(m_selectBox.bottom - m_selectBox.top) > std::abs(m_selectBox.left - m_selectBox.right))
	{
		float ratio = w / h, offset, center;
		offset = (m_selectBox.bottom - m_selectBox.top) * .5f * ratio;
		center = (m_selectBox.left + m_selectBox.right) / 2.f;
		m_targetRegin.top += m_selectBox.top * unit;
		m_targetRegin.bottom -= (h - m_selectBox.bottom) * unit;
		m_targetRegin.left += (center-offset) * unit;
		m_targetRegin.right -= (w - (center + offset)) * unit;
	}
	//if select box is wider in ratio than current window
	else
	{
		float ratio = h / w, offset, center;
		offset = (m_selectBox.right - m_selectBox.left) * .5f * ratio;
		center = (m_selectBox.bottom + m_selectBox.top) * .5f;
		m_targetRegin.left += m_selectBox.left * unit;
		m_targetRegin.right -= (w - m_selectBox.right) * unit;
		m_targetRegin.top += (center - offset) * unit;
		m_targetRegin.bottom -= (h - (center + offset)) * unit;
	}
	m_selectBox = { 0, 0, 0, 0 };
}

void Game::GetDepthInRange(PatchDesc d)
{
	int iter = d.iter;
	int bcount = d.w * d.yEnd;
	DirectX::SimpleMath::Vector2 v;
	float curY, curX, patchsize = (float)d.AAdepth * (float)d.AAdepth;	
	float unit = (m_targetRegin.right - m_targetRegin.left) / (float)d.w;
	float AAunit = unit / (float)d.AAdepth;
	DirectX::SimpleMath::Color c, aafactor;
	aafactor = { patchsize, patchsize , patchsize };
	for (size_t y = d.yStart; y < d.yEnd; y++)
	{
		v.y = curY = m_targetRegin.top + y * unit;
		for (size_t x = 0; x < d.w; x++)
		{
			curX = m_targetRegin.left + unit * x;
			for (size_t AAi = 0; AAi < d.AAdepth; AAi++)
			{
				v.y += AAi * AAunit;
				v.x = curX;
				for (size_t AAj = 0; AAj < d.AAdepth; AAj++)
				{
					v.x += AAj * AAunit;
					c += HSL2RGBf(getDepth(v));
				}
			}
			c /= aafactor;
			pixelColours.at(iter) = c;
			iter++;
			v.y = curY;
			c = { 0, 0, 0, 0 };
		}
	}
}
