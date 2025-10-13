#pragma once

#include "Core/Window.hpp"
#include "Core/data_types.hpp"
#include "ComPort.hpp"

#include <vector>
#include <map>
#include <future>
#include <mutex>
#include <functional>

typedef struct
{
	std::vector<long long> delta_buf;
	long long delta;
	std::chrono::steady_clock::time_point stamp;
	a429_word_t word;
} a429_flow_t;

class UI
{
public:
	UI();
public:
	void Render(float dt); // Рендер
private:
	void ShowLeftPanel();  // Рендер левой панели
	void ShowTable();	   // Рендер таблицы
	void SetPanelSizeAndPosition(int corner, float width, float height, float x_offset, float y_offset);
private:
	std::list<int> getAvailablePorts();					    // Получить доступные порты
	void		   TryConnection(const std::string& name);  // Функция установки подключения (15 попыток) -- в отдельном потоке ConnectionThread
	void		   CloseConnection();						// Функция отключения порта -- в отдельном потоке ConnectionThread
	void		   ReceiveData();						    // Функция получения данных в бесконечном цикле -- в отдельном потоке RxThread
	void		   DataProc(buffer_t* pData);				// Функция обработки данных (из ReceiveData)
private:
	ComPort             port;		      // Порт
	std::mutex          mtx;		      // Mutex для управления tasks
	std::future<void>   ConnectionThread; // Асинхронный поток подключения к порту
	std::future<void>   RxThread;         // Асинхронный поток приёма данных
	task_list_t	        tasks;            // Список команд
private:
	fault_t			  fault;				  // Структура состояния подключения к порту
	std::atomic<bool> ThreadsAllowed = false; // Флаг прерывания потоков RxThread и CmdThread
private:
	std::map<uint32_t, std::map<uint32_t, a429_flow_t>> labels;
};
