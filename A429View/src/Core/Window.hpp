#pragma once

#define _WIN32_WINNT 0x0601
#include <sdkddkver.h>
#include <dwmapi.h>

#include <imgui.h>
#include <implot.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

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
private:
	WNDCLASSEXW wc;
	HWND hwnd;
};

