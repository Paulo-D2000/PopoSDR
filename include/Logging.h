#pragma once

#include <format>
#include <iostream>

#define BLK "\x1b[38;0;" << "30m"
#define RED "\x1b[38;0;" << "31m"
#define GRN "\x1b[38;0;" << "32m"
#define YEL "\x1b[38;0;" << "33m"
#define BLU "\x1b[38;0;" << "34m"
#define MAG "\x1b[38;0;" << "35m"
#define CYN "\x1b[38;0;" << "36m"
#define WHT "\x1b[38;0;" << "37m"
#define RST "\x1b[38;0;" << "0m"

#define LOG_ERROR(...)\
{\
    std::cerr << RED << "[ERROR] " << std::format(__VA_ARGS__) << RST << std::endl;\
}\

#define LOG_INFO(...)\
{\
    std::cerr << GRN <<  "[INFO] " << std::format(__VA_ARGS__) << RST << std::endl;\
}\

#define LOG_DEBUG(...)\
{\
    std::cerr << YEL <<  "[DEBUG] " << std::format(__VA_ARGS__) << RST << std::endl;\
}\

#define LOG_TEST(...)\
{\
    std::cerr << MAG <<  "[TEST] " << std::format(__VA_ARGS__) << RST << std::endl;\
}\
