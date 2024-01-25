#include "stdafx.h"
#include "xr_3da/resource.h"

#include "Include/editor/ide.hpp"
#include "engine_impl.hpp"
#include "GameFont.h"
#include "PerformanceAlert.hpp"
#include "xrCore/ModuleLookup.hpp"

extern LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void CRenderDevice::initialize_weather_editor()
{
    GEnv.isEditor = true;
}

void CRenderDevice::Initialize()
{
    Log("Initializing Engine...");
    TimerGlobal.Start();
    TimerMM.Start();

    if (strstr(Core.Params, "-weather"))
        initialize_weather_editor();

    // Unless a substitute hWnd has been specified, create a window to render into
    if (m_hWnd == NULL)
    {
        const char* wndclass = "_XRAY_1.6";
        // Register the windows class
        HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(0);
        WNDCLASS wndClass = {0, WndProc, 0, 0, hInstance, LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1)), LoadCursor(NULL, IDC_ARROW), (HBRUSH)GetStockObject(BLACK_BRUSH), NULL, wndclass};
        RegisterClass(&wndClass);
        // Set the window's initial style
        m_dwWindowStyle = WS_BORDER | WS_DLGFRAME;
        // Set the window's initial width
        u32 screen_width = GetSystemMetrics(SM_CXSCREEN);
        u32 screen_height = GetSystemMetrics(SM_CYSCREEN);

        DEVMODE screen_settings;
        memset(&screen_settings, 0, sizeof(screen_settings));
        screen_settings.dmSize = sizeof(screen_settings);
        screen_settings.dmPelsWidth = (unsigned long)screen_width;
        screen_settings.dmPelsHeight = (unsigned long)screen_height;
        screen_settings.dmBitsPerPel = 32;
        screen_settings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

        ChangeDisplaySettings(&screen_settings, CDS_FULLSCREEN);

        // Create the render window
        m_hWnd = CreateWindowEx(WS_EX_TOPMOST, wndclass, "S.T.A.L.K.E.R.: Call of Chernobyl", m_dwWindowStyle, 0, 0, screen_width, screen_height, 0L, 0, hInstance, 0L );
    }
    // Save window properties
    m_dwWindowStyle = GetWindowLong(m_hWnd, GWL_STYLE);
    GetWindowRect(m_hWnd, &m_rcWindowBounds);
    GetClientRect(m_hWnd, &m_rcWindowClient);
}

void CRenderDevice::DumpStatistics(IGameFont& font, IPerformanceAlert* alert)
{
    font.OutNext("*** ENGINE:   %2.2fms", stats.EngineTotal.result);
    font.OutNext("FPS/RFPS:     %3.1f/%3.1f", stats.fFPS, stats.fRFPS);
    font.OutNext("TPS:          %2.2f M", stats.fTPS);
    if (alert && stats.fFPS < 30)
        alert->Print(font, "FPS       < 30:   %3.1f", stats.fFPS);
}
