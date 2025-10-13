#pragma once

#ifdef _DEBUG

#include <iostream>

#define LOG(data)            std::cout << data
#define LOG_T(header, data)  std::cout << "[" << header << "]: " << data << "\n"
#define LOG_TM(header, data) std::cout << "[" << header << "]:\n" << data << "\n"
#define LOG_ERR(data) LOG_T("Error", data)
#define LOG_H(header) LOG(">>"); LOG(header); LOG(" module\n")
#define LOG_END() LOG("\n\n")

#else
#define LOG(data) 
#define LOG_T(header, data)
#define LOG_TM(header, data)
#define LOG_ERR(data)
#define LOG_H(header) 
#define LOG_END()
#endif