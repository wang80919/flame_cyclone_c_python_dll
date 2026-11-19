#pragma once
#include <vector>
#include <string>

typedef struct _ISO_PATH_INFO
{
    std::string     name;
    std::vector<_ISO_PATH_INFO> list;
    uint32_t        offset;
    uint32_t        data_offset;
    size_t          size;
    uint8_t         attr;

    _ISO_PATH_INFO():
        offset(0),
        data_offset(0),
        size(0),
        attr(0)
    {

    }

}ISO_PATH_INFO;

typedef std::vector<_ISO_PATH_INFO> ISO_PATH_LIST;


class iso_format
{
public:

    static ISO_PATH_INFO get_iso_path_info(const std::string& path, size_t size = -1, int depth = 260);
    static void print_iso_path_info(const ISO_PATH_INFO& path_list);
    static std::string dump_iso_path_info_to_json(const ISO_PATH_INFO& path_list);

private:
    static void _print_iso_path_info(const ISO_PATH_INFO& path_list, int tab);
    static std::string _dump_iso_path_info_to_json(const ISO_PATH_INFO& path_list);
};

