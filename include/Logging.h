#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <stdexcept>

#define BLK "\x1b[" << "30m"
#define RED "\x1b[" << "31m"
#define GRN "\x1b[" << "32m"
#define YEL "\x1b[" << "33m"
#define BLU "\x1b[" << "34m"
#define MAG "\x1b[" << "35m"
#define CYN "\x1b[" << "36m"
#define WHT "\x1b[" << "37m"
#define RST "\x1b[" << "0m"

// Format + args
template<typename ... Args>
static std::string string_format(const std::string& format, Args ... args)
{
    int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
    auto size = static_cast<size_t>( size_s );
    std::unique_ptr<char[]> buf( new char[ size ] );
    std::snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}

// "Fake" format with no args
static std::string string_format(const std::string& format)
{
   return format;
}

template<typename ... Args>
static void LOG_ERROR(const std::string& format, Args ... args) 
{ 
    std::cerr << RED << "[ERROR] " << string_format(format, args ...) << RST << std::endl; 
} 

template<typename ... Args>
static void LOG_INFO(const std::string& format, Args ... args) 
{ 
    std::cerr << GRN <<  "[INFO] " << string_format(format, args ...) << RST << std::endl; 
} 

template<typename ... Args>
static void LOG_DEBUG(const std::string& format, Args ... args) 
{ 
    std::cerr << YEL <<  "[DEBUG] " << string_format(format, args ...) << RST << std::endl; 
} 

template<typename ... Args>
static void LOG_TEST(const std::string& format, Args ... args) 
{ 
    std::cerr << MAG <<  "[TEST] " << string_format(format, args ...)<< RST << std::endl;
}