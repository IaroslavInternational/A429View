#pragma once

#define S2WS(str) std::wstring(str.begin(), str.end())

#include <vector>
#include <string>

using buffer_t    = std::vector<int8_t>;

// Структура отказов
struct fault_t
{
	uint8_t value;

	union
	{
		uint8_t time_out : 1;
	} bits;
};

typedef union
{
	uint32_t value;
	struct
	{
		uint32_t label  : 8;  // Бит 0-7: Значение метки 
		uint32_t sdi    : 2;  // Бит 8-9: ID источника/получателя
		uint32_t data   : 19; // Бит 10-28: Данные
		uint32_t ssm    : 2;  // Бит 29-30: Матрица SSM
		uint32_t parity : 1;  // Бит 31: Бит чётности
	} bits;
} a429_word_t;