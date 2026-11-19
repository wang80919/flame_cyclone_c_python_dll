import ctypes
import os
import json
import re
from ctypes import *

# DLL 路径配置
dll_name = "CPythonDll.dll"
file_dir = os.path.split(os.path.realpath(__file__))[0]
dll_path = file_dir + "\\" + dll_name

# 接口 原型
#c_bool
#CPYTHONDLL_API bool GetBoolApi();

#c_int(c_int, c_int)
#CPYTHONDLL_API int GetIntApi(int a, int b);

#c_char_p(c_char_p, c_char_p)
#CPYTHONDLL_API const char* GetStringApi(const char* str1, const char* str2);

# 加载 DLL
dll = ctypes.cdll.LoadLibrary(dll_path)

# 返回值类型设置
# https://docs.python.org/3.14/library/ctypes.html
dll.GetBoolApi.restype = c_bool
dll.GetIntApi.restype = c_int
dll.GetIntApi.argtype = (c_int,c_int)
dll.GetStringApi.restype = c_char_p
dll.GetStringApi.argtype = (c_char_p,c_char_p)

dll.GetIsoFilePath.restype = c_char_p

# path, size, depth
dll.GetIsoFilePath.argtype = (c_char_p,c_int,c_int)

# 调用接口

# 打印结果
print("GetStringApi: ", dll.GetBoolApi())
print("GetStringApi: ", dll.GetIntApi(256,512))

str1 = b"Flame"
str2 = b"Cyclone"
print("GetStringApi: ", dll.GetStringApi(str1, str2).decode())

json_str = dll.GetIsoFilePath(b"C:\\Users\\FlameCyclone\\Downloads\\CHDMAN\\1.iso", 1048576, 0).decode()

info = json.loads(json_str)

# print(info)

for item in info['items']:
    search = re.search('.+_\d{3}\.\d{2}$', item['name'], flags=0)
    if search:
        print(item['name'])
    