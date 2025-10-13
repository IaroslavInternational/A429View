#pragma once

#include <future>

namespace lib
{
	// Возвращает состояние потока
	// True  - поток жив
	// False - поток не существует
	template<typename T>
	bool IsThreadTerminated(const std::future<T>& t)
	{
		return t.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
	}
}