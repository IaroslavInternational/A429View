#include "Core/App.hpp"

#if _DEBUG
int main()
#else
int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR pArgs, INT)
#endif
{
	App app(L"A429", 1300, 1100);

	while (1)
	{
		app.Go();
	}
}
