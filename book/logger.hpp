#pragma once

#include <iostream>

#ifdef BENCHMARK_ENABLE
#define LOG_DEBUG(TXT)
#define LOG_INFO(TXT)
#define LOG_ERROR(TXT)
#else
//#define LOG_DEBUG(TXT) std::cout << TXT << '\n'
#define LOG_DEBUG(TXT)
#define LOG_INFO(TXT) std::cout << TXT << '\n'
#define LOG_ERROR(TXT) std::cout << TXT << '\n'
#endif