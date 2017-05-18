#pragma once
//error handler that doesnt require including CSApplication
namespace cs {
    void globalError(const char* fmt, ...) __noreturn __nonull_args(1) __printf_type(1);
    void globalLog(const char* fmt, ...) __nonull_args(1) __printf_type(1);
}