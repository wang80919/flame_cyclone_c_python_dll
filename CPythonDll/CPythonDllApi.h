#pragma once
#include <windows.h>

#ifdef CPYTHONDLL_EXPORTS
#define CPYTHONDLL_API __declspec(dllexport)
#else
#define CPYTHONDLL_API __declspec(dllimport)
#endif

extern "C"
{
    // c_bool
    CPYTHONDLL_API bool GetBoolApi();

    // c_int(c_int, c_int)
    CPYTHONDLL_API int GetIntApi(int a, int b);

    // c_char_p(c_char_p, c_char_p)
    CPYTHONDLL_API const char* GetStringApi(const char* str1, const char* str2);


    CPYTHONDLL_API const char* GetIsoFilePath(const char* path, size_t size, int depth);
}
