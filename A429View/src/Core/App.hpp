#pragma once

#include "Window.hpp"
#include "../UI.hpp"

struct FrameTimer
{
	float Mark()
	{
		const auto old = counter;
		counter = std::chrono::steady_clock::now();

		return std::chrono::duration<float>(counter - old).count();
	}

	std::chrono::steady_clock::time_point counter;
};

class App
{
public:
	App(const std::wstring& name, int x, int y);
public:
	void Go();
private:
	Window wnd;
	UI ui;
	FrameTimer timer;
};

