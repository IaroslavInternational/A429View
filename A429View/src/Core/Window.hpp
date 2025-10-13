#pragma once

#define _WIN32_WINNT 0x0601
#include <sdkddkver.h>
#include <dwmapi.h>

#include "../../imgui/imgui.h"
#include "../../imgui/implot.h"
#include "../../imgui/imgui_impl_win32.h"
#include "../../imgui/imgui_impl_dx11.h"

#include <d3d11.h>

#include <string>

class Window
{
public:
	Window(const std::wstring& name, int x, int y);
	~Window();
public:
	void BeginFrame();
	void EndFrame();
private:
	bool CreateDeviceD3D();
	void CleanupDeviceD3D();
	void CreateRenderTarget();
	void CleanupRenderTarget();
private:
	WNDCLASSEXW wc;
	HWND hwnd;

	ID3D11Device*		    g_pd3dDevice = nullptr;
	ID3D11DeviceContext*    g_pd3dDeviceContext = nullptr;
	IDXGISwapChain*		    g_pSwapChain = nullptr;
	bool                    g_SwapChainOccluded = false;
	ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
};

