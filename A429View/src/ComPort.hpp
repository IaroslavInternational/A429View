#pragma once

#include <string>
#include <vector>

#include <Windows.h>
#include <fileapi.h>
#include <atlstr.h>

using buffer_t = std::vector<int8_t>;

class ComPort
{
public:
	ComPort() = default;
	~ComPort();
public:
	bool Open(const std::string& portName, uint32_t speed = CBR_9600, uint8_t byte_size = 8, uint8_t stop_bytes = ONESTOPBIT, uint8_t parity = NOPARITY);
	void Close();
	void TxData(int8_t* data);
	bool RxData(buffer_t* buffer);
public:
	bool IsOpen() const;
	void SetBufferSize(uint64_t buffer_size);
	void ClearBuffer();
	std::string GetName() const;
private:
	HANDLE COM   = nullptr;
	bool   isActive = false;

	std::string name      = "NO PORT";
	uint32_t    speed     = CBR_9600;
	uint8_t     byte_size = ONESTOPBIT;
	uint8_t     parity    = NOPARITY;

	DWORD    sentSize    = 0;
	DWORD    readSize    = 0;
	uint64_t buffer_size = 64;
};

