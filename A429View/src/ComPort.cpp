#include "ComPort.hpp"

#include <cassert>

#define CHECK_ERROR(expr, status, comment) \
if(expr) \
{					  \
	status = false;   \
} \
else \
{	 \
	status = true;	\
}

#define S2WS(str) std::wstring(str.begin(), str.end())

ComPort::~ComPort()
{
	Close();
}

bool ComPort::Open(const std::string& portName, uint32_t speed, uint8_t byte_size, uint8_t stop_bytes, uint8_t parity)
{	
	if (isActive)
	{
		Close();
	}

	name = portName;
	this->speed = speed;
	this->byte_size = byte_size;
	this->parity = parity;

	// Open port
	COM = CreateFile(S2WS(portName).c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, 0);
	
	CHECK_ERROR(COM == INVALID_HANDLE_VALUE,         isActive, "Unknown handle error");
	CHECK_ERROR(GetLastError() == ERROR_FILE_NOT_FOUND, isActive, "Serial Port not found");

	DCB dcbSerialParam = { 0 };
	dcbSerialParam.DCBlength = sizeof(dcbSerialParam);

	CHECK_ERROR(!GetCommState(COM, &dcbSerialParam), isActive, "Get params failed");

	dcbSerialParam.BaudRate = speed;
	dcbSerialParam.ByteSize = byte_size;
	dcbSerialParam.StopBits = stop_bytes;
	dcbSerialParam.Parity   = parity;

	CHECK_ERROR(!SetCommState(COM, &dcbSerialParam), isActive, "Set params failed");

	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout         = 10;
	timeouts.ReadTotalTimeoutConstant    = 50;
	timeouts.ReadTotalTimeoutMultiplier  = 10;
	timeouts.WriteTotalTimeoutConstant   = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;

	CHECK_ERROR(!SetCommTimeouts(COM, &timeouts), isActive, "Set timeouts failed");

	SetBufferSize(10000);

	// Clear internal buffer
	if (isActive)
	{
		PurgeComm(COM, PURGE_RXCLEAR | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_TXABORT);
	}

	return isActive;
}

void ComPort::Close()
{
	if (!CloseHandle(COM))
	{
		
	}
	else
	{
		isActive = false;
		ClearBuffer();
	}
}

void ComPort::TxData(int8_t* data)
{
	assert(data != nullptr && sizeof(data) > 0);

	if (!WriteFile(COM, data, sizeof(data) / sizeof(data[0]), NULL, NULL))
	{
		
	}
}

bool ComPort::RxData(buffer_t* buffer)
{
	// Проверяем доступные данные
	DWORD available = 0;
	DWORD errors;
	COMSTAT status;

	if (!ClearCommError(COM, &errors, &status)) 
	{
		return false;
	}

	available = status.cbInQue;
	if (available == 0) 
	{
		return false;  // Нет данных
	}

	// Ограничиваем размер чтения
	DWORD to_read = (available > buffer_size) ? buffer_size : available;

	buffer->resize(to_read);

	if (!ReadFile(COM, buffer->data(), to_read, &readSize, NULL)) 
	{
		return false;
	}

	if (readSize > 0) 
	{
		buffer->resize(readSize);
		return true;
	}

	return false;
}

bool ComPort::IsOpen() const
{
	return isActive;
}

void ComPort::SetBufferSize(uint64_t buffer_size)
{
	this->buffer_size = buffer_size;
	//buffer.resize(buffer_size);
}

void ComPort::ClearBuffer()
{
	if (COM && COM != INVALID_HANDLE_VALUE) 
	{
		PurgeComm(COM, PURGE_RXCLEAR | PURGE_RXABORT);
	}
}

std::string ComPort::GetName() const
{
	return name;
}
