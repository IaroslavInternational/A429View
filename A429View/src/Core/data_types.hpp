#pragma once

#define S2WS(str) std::wstring(str.begin(), str.end())

#include <vector>
#include <string>
#include <queue>

using buffer_t    = std::vector<int8_t>;
using task_list_t = std::queue<std::string>;

// Структура отказов
struct fault_t
{
	union
	{
		uint8_t time_out : 1;
	} bits;

	uint8_t value;
};

// Структура лога
struct log_t
{
	log_t()
	{
		name.reserve(20);
		period = 100.0f;
	}

	std::string name;
	float       period;
};

typedef union
{
	uint32_t value;
	struct
	{
		uint32_t label : 8;  // Бит 0-7: Значение метки 
		uint32_t sdi : 2;  // Бит 8-9: ID источника/получателя
		uint32_t data : 19; // Бит 10-28: Данные
		uint32_t ssm : 2;  // Бит 29-30: Матрица SSM
		uint32_t parity : 1;  // Бит 31: Бит чётности
	} bits;
} a429_word_t;