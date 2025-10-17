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

    float GetParam_BNR_float(uint32_t fWord, uint8_t fMSB, uint8_t fLSB, uint8_t fSignBit, float fWeight)
    {
        // маска для знакового бита
        uint32_t signBitMask = (fSignBit == 0U) ? 0U : 1U << (fSignBit - 1U);
        // значение цены MSB
        uint32_t codeWeight = 1U << (fMSB - fLSB);
        // маска для значащего бита
        uint32_t mask = codeWeight + (codeWeight - 1U);
        // получаем значение
        float value = (fWord >> (fLSB - 1U)) & mask;

        // проверка, если слово содержит отрицательное значение
        if ((fWord & signBitMask) != 0U)
        {
            // получаем отрицательное число
            value = -1.f * (2.f * codeWeight - value);
        }

        // вычисление реального значения
        return (value * fWeight) / (float)codeWeight;
    }
}