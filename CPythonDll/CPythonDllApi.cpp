#include "pch.h"
#include "CPythonDllApi.h"
#include <string>
#include "win32utils/iso_format.h"

CPYTHONDLL_API bool GetBoolApi()
{
    return true;
}

CPYTHONDLL_API int GetIntApi(int a, int b)
{
    return a + b;
}

CPYTHONDLL_API const char* GetStringApi(const char* str1, const char* str2)
{
    static std::string strResult;

    strResult += str1;
    strResult += str2;

    return strResult.c_str();
}

CPYTHONDLL_API const char* GetIsoFilePath(const char* path, size_t size, int depth)
{
    static std::string result_string;

    ISO_PATH_INFO path_list = iso_format::get_iso_path_info(path, size, depth);

    result_string = iso_format::dump_iso_path_info_to_json(path_list);

    return result_string.c_str();
}