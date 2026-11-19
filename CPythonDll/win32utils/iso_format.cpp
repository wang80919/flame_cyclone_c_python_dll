#include "pch.h"
#include "ISO_Format.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <stdint.h>

// https://wiki.osdev.org/ISO_9660
#pragma pack(1)

// 日期时间
typedef struct _ISO_DATA_TIME
{
    uint8_t         Year;
    uint8_t         Month;
    uint8_t         Day;
    uint8_t         Hour;
    uint8_t         Minute;
    uint8_t         Second;
    uint8_t         TimeZoneOffset;
}ISO_DATA_TIME, * PISO_DATA_TIME;

// 文件标志
typedef struct _ISO_FILE_FLAG
{
    uint8_t         Hidden:1;
    uint8_t         Directory :1;
    uint8_t         Associated:1;
    uint8_t         Record:1;
    uint8_t         Protection:1;
    uint8_t         Reserved:2;
    uint8_t         MultiExtent:1;
}ISO_FILE_FLAG, * PISO_FILE_FLAG;

typedef struct _DATA32
{
    uint32_t LE;
    uint32_t BE;
}DATA32;

typedef struct _DATA16
{
    uint16_t LE;
    uint16_t BE;
}DATA16;

// 路径表
typedef struct _ISO_DIRECTORY
{
    uint8_t         DirectoryRecordLength;
    uint8_t         ExtendedAttributeRecordLength;
    DATA32          LocationOfExtent;
    DATA32          DataLength;
    ISO_DATA_TIME   RecordingDateAndTime;
    ISO_FILE_FLAG   FileFlags;
    uint8_t         FileUnitSize;
    uint8_t         InterleaveGapSize;

    DATA16          VolumeSequenceNumber;

    uint8_t         FileIdentifierLength;
    char            FileIdentifier[1];
}ISO_DIRECTORY, *PISO_DIRECTORY;

// 卷描述符类型代码
// 卷描述符类型字段指定卷描述符的类型
typedef struct _ISO_VOLUME_DESC
{
    uint8_t         BootRecord;                         // 引导记录
    uint8_t         PrimaryVolumeDescriptor;            // 主卷描述符
    uint8_t         SupplementaryVolumeDescriptor;      // 补充卷描述符
    uint8_t         VolumePartitionDescriptor;          // 卷分区描述符
    uint8_t         Reserved[251];                      // 保留
    uint8_t         VolumeDescriptorSetTerminator;      // 卷描述符集终止符
}ISO_VOLUME_DESC, *PISO_VOLUME_DESC;

// 引导记录
// 第一种卷描述符是“引导记录”。描述符的格式如下
typedef struct _ISO_BOOL_RECORD
{
    uint8_t         Type;
    char            Identifier[5];
    uint8_t         Version;
    char            BootSystemIdentifier[32];
    char            BootIdentifier[32];
    uint8_t         BootSystemUse[1977];
}ISO_BOOL_RECORD, *PISO_BOOL_RECORD;

typedef struct _ISO_HEADER
{
    uint8_t         VolDescType;
    uint8_t         StandardIdentifier[5];
    uint8_t         VolumeDescriptorVersion;
}ISO_HEADER;

// 主卷描述符
// 这是一个较长的描述符，但它包含了一些用于读取文件系统其余部分非常有用的信息
typedef struct _ISO_PRIMARY_VOLUME_DESC
{
    ISO_HEADER      Header;
    uint8_t         Unused1;
    char            SystemId[32];
    char            VolumeId[32];
    uint8_t         Unused8[8];
    DATA32          VolumeSpaceSize;
    uint8_t         UnusedField2[32];
    DATA16          VolumeSetSize;
    DATA16          VolumeSequenceNumber;
    DATA16          LogicalBlockSize;
    DATA32          PathTableSize;
    uint32_t        LocationOfPathTableLE;
    uint32_t        LocationOfOptionalPathTableLE;
    uint32_t        LocationOfPathTableBE;
    uint32_t        LocationOfOptionalPathTableBE;
    ISO_DIRECTORY   Directory;
    char            VolumeSetIdentifier[128];
    char            PublisherIdentifier[128];
    char            DataPreparerIdentifier[128];
    char            ApplicationIdentifier[128];
    char            CopyrightFileIdentifier[37];
    char            AbstractFileIdentifier[37];
    char            BibliographicFileIdentifier[37];
    uint8_t         VolumeCreationDateTime[17];
    uint8_t         VolumeModificationDateTime[17];
    uint8_t         VolumeExpirationDateTime[17];
    uint8_t         VolumeEffectiveDateTime[17];
    uint8_t         FileStructureVersion;
    uint8_t         Unused2;
    uint8_t         ApplicationUsed[512];
    uint8_t         Reserved[653];
}ISO_PRIMARY_VOLUME_DESC, *PISO_PRIMARY_VOLUME_DESC;

// 卷描述符集终止符
typedef struct _ISO_VOLUME_DESC_SET_TERMINATOR
{
    uint8_t         Type;               // 0xFF 表示卷描述符集终止符
    char            Identifier[5];      // 总是 "CD001"
    uint8_t         Version;            // 卷描述符版本（0x01）
    uint8_t         Reserved[2041];
}ISO_VOLUME_DESC_SET_TERMINATOR, *PISO_VOLUME_DESC_SET_TERMINATOR;

// 路径表
typedef struct _ISO_PATH_TABLE
{
    uint8_t         DirectoryIdentifierLength;
    uint8_t         ExtendedAttributeRecordLength;
    uint32_t        ExtentLocation;
    uint16_t        DirectoryNumber;
    char            DirectoryIdentifier[0];
}ISO_PATH_TABLE, *PISO_PATH_TABLE;

#pragma pack()

std::vector<uint8_t> read_file_data(const std::string& path, size_t read_size = -1);
void parse_iso_path(ISO_PATH_INFO& path_list, const uint8_t* pData, size_t size, PISO_PRIMARY_VOLUME_DESC pPrimary, PISO_DIRECTORY pTab, int depth, int tab);


ISO_PATH_INFO iso_format::get_iso_path_info(const std::string& path, size_t size/* = -1*/, int depth/* = 260*/)
{
    ISO_PATH_INFO info;
    std::vector<uint8_t> data = read_file_data(path, size);

    if (data.empty())
    {
        return info;
    }

    uint8_t* data_ptr = data.data();

    PISO_PRIMARY_VOLUME_DESC primary_ptr = (PISO_PRIMARY_VOLUME_DESC)&data[0x8000];

    size_t offset = (size_t)primary_ptr->Directory.LocationOfExtent.LE * (size_t)primary_ptr->LogicalBlockSize.LE;
    if (offset < size)
    {
        info.name = primary_ptr->VolumeId;
        info.attr = 0x02;
        info.size = 0;// (size_t)primary_ptr->LogicalBlockSize.LE* (size_t)primary_ptr->VolumeSpaceSize.LE;
        info.offset = 0x8000;
        info.data_offset = offset;

        size_t count = info.name.size();
        for (auto it = info.name.rbegin(); it != info.name.rend(); it++)
        {
            if (' ' != *it)
            {
                break;
            }

            count--;
        }

        info.name.resize(count);

        PISO_DIRECTORY dir_ptr = (PISO_DIRECTORY)(data_ptr + primary_ptr->Directory.LocationOfExtent.LE * primary_ptr->LogicalBlockSize.LE);
        parse_iso_path(info, data.data(), size, primary_ptr, dir_ptr, depth, 0);
        info.size = (size_t)primary_ptr->LogicalBlockSize.LE* (size_t)primary_ptr->VolumeSpaceSize.LE;
    }

    return info;
}

void iso_format::print_iso_path_info(const ISO_PATH_INFO& path_list)
{
    _print_iso_path_info(path_list, 0);
}

std::string iso_format::dump_iso_path_info_to_json(const ISO_PATH_INFO& path_info)
{
    return _dump_iso_path_info_to_json(path_info);
}

std::string iso_format::_dump_iso_path_info_to_json(const ISO_PATH_INFO& path_info)
{
    std::string result;

    if (path_info.attr & 0x02)
    {
        result += '{';

        result += "\"name\":";

        result += "\"";
        result += path_info.name;
        result += "\"";
        result += ',';

        result += "\"size\":";
        result += std::to_string(path_info.size);
        result += ',';

        result += "\"offset\":";
        result += std::to_string(path_info.offset);
        result += ',';

        result += "\"data_offset\":";
        result += std::to_string(path_info.data_offset);
        result += ',';

        result += "\"attr\":";
        result += std::to_string(path_info.attr);
        result += ',';

        result += "\"items\":";

        result += '[';
        size_t count = path_info.list.size();
        for (const auto& item : path_info.list)
        {
            result += _dump_iso_path_info_to_json(item);
            count--;
            if (count > 0)
            {
                result += ',';
            }
        }

        result += ']';

        result += '}';
    }
    else
    {
        result += '{';

        result += "\"name\":";

        result += "\"";
        result += path_info.name;
        result += "\"";
        result += ',';

        result += "\"size\":";
        result += std::to_string(path_info.size);
        result += ',';

        result += "\"offset\":";
        result += std::to_string(path_info.offset);
        result += ',';

        result += "\"data_offset\":";
        result += std::to_string(path_info.data_offset);
        result += ',';

        result += "\"attr\":";
        result += std::to_string(path_info.attr);

        result += '}';
    }

    return result;
}

void iso_format::_print_iso_path_info(const ISO_PATH_INFO& path_info, int tab)
{
    for (int i = 0; i < tab; i++)
    {
        printf("    ");
    }

    printf("%s", path_info.name.c_str());

    if (path_info.attr & 0x02)
    {
        if (path_info.size < 1024)
        {
            printf(" %.8X %zu Bytes", path_info.offset, path_info.size);
        }
        else if(path_info.size < 1024 * 1024)
        {
            printf(" %.8X %.2lf KB", path_info.offset, (double)path_info.size / (1024.0f));
        }
        else
        {
            printf(" %.8X %.2lfMB", path_info.offset, (double)path_info.size / (1024.0f * 1024.0f));
        }
    }
    else
    {
        if (path_info.size < 1024)
        {
            printf(" %.8X %zu Bytes", path_info.offset, path_info.size);
        }
        else if(path_info.size < 1024 * 1024)
        {
            printf(" %.8X %.2lf KB", path_info.offset, (double)path_info.size / (1024.0f));
        }
        else
        {
            printf(" %.8X %.2lfMB", path_info.offset, (double)path_info.size / (1024.0f * 1024.0f));
        }
    }

    printf("\n");

    if (!path_info.list.empty())
    {
        for (const auto& item : path_info.list)
        {
            _print_iso_path_info(item, tab + 1);
        }
    }
}

void parse_iso_path(ISO_PATH_INFO& path_info, const uint8_t* data_ptr, size_t size, PISO_PRIMARY_VOLUME_DESC primary_ptr, PISO_DIRECTORY dir_ptr, int depth, int tab/* = 0*/)
{
    int index = 0;

    if (depth < 0)
    {
        return;
    }

    while (dir_ptr->DirectoryRecordLength)
    {
        size_t offset = (size_t)dir_ptr - (size_t)data_ptr;

        if (offset >= size)
        {
            break;
        }

        if (0 == index) // 自身
        {
        }
        else if (1 == index) // 父路径
        {
        }
        else
        {
            char szName[260] = { 0 };
            memcpy_s(szName, sizeof(szName), dir_ptr->FileIdentifier, dir_ptr->FileIdentifierLength);

            if (0 == dir_ptr->FileFlags.Directory)
            {
                if (';' == szName[dir_ptr->FileIdentifierLength - 2])
                {
                    szName[dir_ptr->FileIdentifierLength - 2] = 0;
                }
            }

            ISO_PATH_INFO info;
            info.attr = *(uint8_t*)&dir_ptr->FileFlags;
            info.name = szName;
            info.size = dir_ptr->DataLength.LE;
            info.offset = offset;
            info.data_offset = dir_ptr->LocationOfExtent.LE * primary_ptr->LogicalBlockSize.LE;

            if (dir_ptr->FileFlags.Directory)
            {
                parse_iso_path(info, data_ptr, size, primary_ptr, (PISO_DIRECTORY)(data_ptr + dir_ptr->LocationOfExtent.LE * primary_ptr->LogicalBlockSize.LE), depth - 1, tab + 1);
            }

            path_info.list.push_back(info);
            path_info.size += info.size;
        }

        dir_ptr = (PISO_DIRECTORY)((uint8_t*)dir_ptr + dir_ptr->DirectoryRecordLength);
        index++;
    }

    return;
}

std::vector<uint8_t> read_file_data(const std::string& path, size_t read_size/* = -1*/)
{
    std::ifstream in_file(path, std::ios::in | std::ios::binary);
    std::vector<uint8_t> data;

    try
    {
        if (in_file.is_open())
        {

            in_file.seekg(0, std::ios::end);
            std::streampos nSize = in_file.tellg();
            in_file.seekg(0, std::ios::beg);

            if (read_size == -1)
            {
                read_size = nSize;
            }

            data.resize(read_size);
            in_file.read(reinterpret_cast<char*>(&data[0]), read_size);
            std::streamsize nByteSize = in_file.gcount();
        }
    }catch(...)
    {

    }

    return data;
}