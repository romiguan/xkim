#ifndef __INI_FILE_READER_H__
#define __INI_FILE_READER_H__

#include <stdint.h>

#include <string>
#include <vector>
#include <map>

namespace util
{

class IniFileReader
{
    public:
        IniFileReader(const char *ini_file);
        ~IniFileReader();

        struct SecKeyValue
        {
            std::string key;
            std::string value;
        };

        /* 
         * 获取ini配置文件中, section下对应item的字符串值. 如果找不到, 或找到值为空, 返回default_value 
         */
        std::string IniGetStrValue(const std::string& section_name, const std::string& item_key, const std::string& default_value);

        /* 
         * 获取ini配置文件中, section下对应item的整形值. 如果找不到, 或找到值为空, 返回default_value 
         */
        int IniGetIntValue(const std::string& section_name, const std::string& item_key, const int default_value);

        /* 
         * 获取ini配置文件中, section下对应item的64位整形值. 如果找不到, 或找到值为空, 返回default_value 
         */
        int64_t IniGetInt64Value(const std::string& section_name, const std::string& item_key, const int64_t default_value);

        /* 
         * 获取ini配置文件中, section下对应item的布尔值. 如果找不到, 或找到值为空, 返回default_value 
         */
        bool IniGetBoolValue(const std::string& section_name, const std::string& item_key, const bool default_value);

        /* 
         * 获取ini配置文件中, section下对应item的浮点数值. 如果找不到, 或找到值为空, 返回default_value 
         */
        double IniGetDoubleValue(const std::string& section_name, const std::string& item_key, const double default_value);

        /* 
         * 获取ini配置文件中, section下所有item key和value的字符串值. 如果section找不到, 或section下item为空, 返回-1
         */
        int IniGetSecValues(const std::string& section_name, std::vector<IniFileReader::SecKeyValue>& sec_kv);

        bool HasSection(const std::string& section_name) const;
        bool SectionIsUnique(const std::string& section_name) const;
        bool HasKeyValue(const std::string& key, const std::string& value) const;
        bool KeyValueIsUnique(const std::string& key, const std::string& value) const;

    private:
        std::string IniGetValue(const std::string& section_name, const std::string& item_key);
        std::map<std::string, int32_t> section_count;
        std::map<std::string, std::string> ini_kv_map;
        std::map<std::string, std::map<std::string, int32_t> > ini_kv_status_map;
};

}

#endif

