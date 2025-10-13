#include "App.hpp"

App::App(const std::wstring& name, int x, int y)
	:
	wnd(name, x, y)
{
}

void App::Go()
{
	const auto dt = timer.Mark();
	
	wnd.BeginFrame();
	ui.Render(dt);
	wnd.EndFrame();
}
