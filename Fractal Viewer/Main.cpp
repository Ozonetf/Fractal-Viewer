#include "pch.h"
#include "Main.h"
#include "Game.h"


namespace
{
    std::unique_ptr<Game> _game;
}

LPCWSTR g_szAppName = L"Device Resource UWP example";
LPCWSTR WINDOW_NAME = L"Fractal";

void RunMessageLoop()
{

    MSG message;
    message.message = WM_NULL;
    while (message.message != WM_QUIT)
    {
        //handle messages if there are any
        if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message); //used to translate user input to character msg so it can be processed
            DispatchMessage(&message);  //sends msg to the WinddowProc function to be processed
        }
        //normal game loop
        else
        {
            _game->Tick();
        }
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static bool s_in_sizemove = false;
    static bool s_in_suspend = false;
    static bool s_minimized = false;
    static bool s_fullscreen = false;
    // TODO: Set s_fullscreen to true if defaulting to fullscreen.

    switch (uMsg)
    {
    case WM_PAINT:  //draws when app requests to, also called by DispatchMessage function
        if (s_in_sizemove && _game)
        {
            _game->Tick();
        }
        else
        {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
        }
        break;

    case WM_MOVE:   //when the user drags the window around
        if (_game)
        {
            OutputDebugString(L"Window Moved.\n");
            //game->OnWindowMoved();
        }
        break;

    case WM_SIZE:   //Sent to a window after its size has changed.
        if (wParam == SIZE_MINIMIZED)   //window minimized
        {
            if (!s_minimized)
            {
                OutputDebugString(L"Minimized\n");
                s_minimized = true;
                if (!s_in_suspend && _game)
                    //game->OnSuspending();
                    s_in_suspend = true;
            }
        }
        else if (s_minimized)   //window un-minimized
        {
            OutputDebugString(L"Un-Minimized\n");
            s_minimized = false;
            if (s_in_suspend && _game)
                //game->OnResuming();
                s_in_suspend = false;
        }
        else if (!s_in_sizemove && _game)//window resized
        {
            OutputDebugString(L"Window Resized??.\n");
            _game->OnWindowSizeChanged(LOWORD(lParam), HIWORD(lParam));
        }
        break;

    case WM_ENTERSIZEMOVE:  //when user starts to resize window by dragging the edges
        s_in_sizemove = true;
        break;

    case WM_EXITSIZEMOVE:   //when user finished resizing window
        s_in_sizemove = false;
        if (_game)
        {
            OutputDebugString(L"Window Resized.\n");
            RECT rc;
            GetWindowRect(hwnd, &rc);
            _game->OnWindowSizeChanged(rc.right - rc.left, rc.bottom - rc.top);
        }
        break;

        //case WM_GETMINMAXINFO:
        //    if (lParam)
        //    {
        //        auto info = reinterpret_cast<MINMAXINFO*>(lParam);
        //        info->ptMinTrackSize.x = 320;
        //        info->ptMinTrackSize.y = 200;
        //    }
        //    break;

    case WM_ACTIVATEAPP:
        OutputDebugString(L"Window Active.\n");

        DirectX::Keyboard::ProcessMessage(uMsg, wParam, lParam);
        DirectX::Mouse::ProcessMessage(uMsg, wParam, lParam);

        //if (_game)
        //{
        //    if (wParam)
        //    {
        //        //game->OnActivated();
        //    }
        //    else
        //    {
        //        //game->OnDeactivated();
        //    }
        //}
        break;

        //case WM_POWERBROADCAST:
        //    switch (wParam)
        //    {
        //    case PBT_APMQUERYSUSPEND:
        //        if (!s_in_suspend && game)
        //            //game->OnSuspending();
        //        s_in_suspend = true;
        //        return TRUE;

        //    case PBT_APMRESUMESUSPEND:
        //        if (!s_minimized)
        //        {
        //            if (s_in_suspend && game)
        //                //game->OnResuming();
        //            s_in_suspend = false;
        //        }
        //        return TRUE;
        //    }
        //    break;

    case WM_DESTROY:    //close the program when users quits
        PostQuitMessage(0);
        break;

    case WM_ACTIVATE:
    case WM_INPUT:
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MOUSEWHEEL:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
    case WM_MOUSEHOVER:
        DirectX::Mouse::ProcessMessage(uMsg, wParam, lParam);
        break;

    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
        DirectX::Keyboard::ProcessMessage(uMsg, wParam, lParam);
        break;

    case WM_SYSKEYDOWN: //when system key is pressed (Alt)
        DirectX::Keyboard::ProcessMessage(uMsg, wParam, lParam);
        if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
        {
            // Implements the classic ALT+ENTER fullscreen toggle
            if (s_fullscreen)
            {
                SetWindowLongPtr(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
                SetWindowLongPtr(hwnd, GWL_EXSTYLE, 0);

                int width = 800;
                int height = 600;
                if (_game)
                    //game->GetDefaultSize(width, height);

                    ShowWindow(hwnd, SW_SHOWNORMAL);
                float dpi = GetDpiForWindow(hwnd);
                SetWindowPos(hwnd, HWND_TOP, 0, 0, 
                    static_cast<int>(ceil(640.f * dpi / 96.f)),
                    static_cast<int>(ceil(480.f * dpi / 96.f)), 
                    SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
            }
            else
            {
                SetWindowLongPtr(hwnd, GWL_STYLE, WS_POPUP);
                SetWindowLongPtr(hwnd, GWL_EXSTYLE, WS_EX_TOPMOST);

                SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

                ShowWindow(hwnd, SW_SHOWMAXIMIZED);
                //BUG when enter full screen the winsize dont get properly updated causing problem in 
                //render queue cullings
                RECT rc;
                GetClientRect(hwnd, &rc);
                _game->OnWindowSizeChanged(rc.right - rc.left, rc.bottom - rc.top);
            }

            s_fullscreen = !s_fullscreen;
        }
        break;

    case WM_MOUSEACTIVATE:
        // When you click activate the window, we want Mouse to ignore it.
        return MA_ACTIVATEANDEAT;

        //case WM_MENUCHAR:
        //    // A menu is active and the user presses a key that does not correspond
        //    // to any mnemonic or accelerator key. Ignore so we don't produce an error beep.
        //    return MAKELRESULT(0, MNC_CLOSE);
        //}
    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
    //return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

HRESULT Initialize()
{
    OutputDebugString(L"This will go to the output.\n");
    HRESULT res;
    LPCWSTR CLASS_NAME = L"MainWindow";

    WNDCLASSEX windowclass;
    ZeroMemory(&windowclass, sizeof(WNDCLASSEX));
    windowclass.cbSize = sizeof(WNDCLASSEX);
    windowclass.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
    windowclass.hInstance = HINST_THISCOMPONENT;
    windowclass.lpfnWndProc = WindowProc;
    windowclass.lpszClassName = CLASS_NAME;
    windowclass.style = CS_HREDRAW | CS_VREDRAW;

    RegisterClassEx(&windowclass);

    //rectangle size for the window client without border
    RECT rect = { 0, 0, 1920, 1080 };
    AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, false, WS_EX_OVERLAPPEDWINDOW);
    HWND windowHandle = CreateWindowExW(
        WS_EX_OVERLAPPEDWINDOW,
        CLASS_NAME,				//window class name
        WINDOW_NAME,			//window name
        WS_OVERLAPPEDWINDOW,	//style	
        0,					//x coord
        0,					//y coord
        rect.right - rect.left,	//window width
        rect.bottom - rect.top,	//window height
        NULL,					//window parent
        NULL,					//window menu
        HINST_THISCOMPONENT,	//hinstance	
        0);
    _game = std::make_unique<Game>();
    _game->Init(windowHandle, rect.right, rect.bottom);

    res = windowHandle ? S_OK : E_FAIL;
    if (SUCCEEDED(res))
    {
        ShowWindow(windowHandle, SW_SHOWNORMAL);
        UpdateWindow(windowHandle);
    }

    return res;
}


int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
    if (SUCCEEDED(CoInitialize(NULL)))
    {
        {
            if (SUCCEEDED(Initialize()))
            {
                RunMessageLoop();
            }
        }
        CoUninitialize();
    }
    return 0;
}
