#include "ini_file_reader.h"
#include "string_algorithms.h"

#include <unistd.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <cstdio>

#include <boost/regex.hpp>
#include <glog/logging.h>

/************************************************************************
 * PRIVATE DEFINITIONS
 ************************************************************************/

#define SEC_KEY_DELIM		'@'
#define SEC_KEYS_DELIM		','

#define SEC_KEY_DELIM_STR	"@"
#define SEC_KEYS_DELIM_STR	","

/************************************************************************
 * IMPLEMENTATION OF PUBLIC FUNCTIONS
 ************************************************************************/

namespace util
{

IniFileReader::IniFileReader(const char *ini_file)
{
    FILE *fp = NULL;
    char *line_read = NULL;
    size_t line_read_len = 0;
    ssize_t read_size = -1;

    if (!ini_file) {
        LOG(ERROR)<<"invalid config file name\n";
        exit(1);
    }

    if (access(ini_file, R_OK) < 0) {
        LOG(ERROR)<< "access file \""<<ini_file<<"\" fail\n";
        exit(1);
    }

    if (!(fp = fopen(ini_file, "r"))) {
        LOG(ERROR)<< "can not open config file \""<<ini_file<<"\"\n";
        exit(1);
    }

    boost::regex pattern_section("\\s*\\[(.*?)\\]\\s*.*");
    boost::regex pattern_item("\\s*([^\\s\\r\\=]*?)\\s*\\=\\s*([^\\s\\r\\n]*?)([\\r\\n\\s]*)$");
    boost::cmatch match;

    ini_kv_map.clear();

    std::string section_name, item_key, item_value;
    const char *start;
    while ((read_size = getline(&line_read, &line_read_len, fp)) != -1) {
        start = line_read;	
        while (*start == ' ' || *start == '\n' || *start == '\t' || *start == '\r') start++;

        /* 略去空行和注释 */
        if (*start == '#' || *start == '\0') {
            continue;
        }

        if (boost::regex_search(line_read, match, pattern_section)) {
            section_name.assign(match[1].first, match[1].second);
            if (section_count.find(section_name) == section_count.end())
                section_count[section_name] = 1;
            else
                section_count[section_name] += 1;
        } else if (boost::regex_search(line_read, match, pattern_item)) {

            if (section_name == "") {
                LOG(ERROR)<< "格式错误, 当前行不属于任何section! ("<<line_read<<")\n";
                exit(1);	
            }

            item_key.assign(match[1].first, match[1].second);
            item_value.assign(match[2].first, match[2].second);

            ini_kv_map[item_key + SEC_KEY_DELIM_STR + section_name] = item_value;

            /* 记录属于某section的所有key名称 */
            if (ini_kv_map.find(section_name) == ini_kv_map.end()) {
                ini_kv_map[section_name] += item_key;
            } else {
                ini_kv_map[section_name] += std::string(SEC_KEYS_DELIM_STR) + item_key;
            }

            if (ini_kv_status_map.find(item_key) == ini_kv_status_map.end()) {
                ini_kv_status_map[item_key] = std::map<std::string, int32_t>();
                ini_kv_status_map[item_key][item_value] = 1;
            } else {
                std::map<std::string, int32_t>& kv = ini_kv_status_map[item_key];
                if (kv.find(item_value) == kv.end()) {
                    kv[item_value] = 1;
                } else {
                    kv[item_value] += 1;
                }
            }
        } else {
            LOG(ERROR)<< "配置文件格式错误! ("<<line_read<<")\n";
            exit(1);	
        }
    }


    if (line_read) {
        free(line_read);
        line_read = NULL;
    }

    if (fp) {
        fclose(fp);
    }
}

IniFileReader::~IniFileReader()
{

}

std::string IniFileReader::IniGetStrValue(const std::string& section_name, const std::string& item_key, const std::string& default_value)
{
    if (section_name == "" || item_key == "")
        return "";

    std::string item_value = IniGetValue(section_name, item_key);

    if (item_value == "")
        return default_value;

    return item_value;
}

int IniFileReader::IniGetIntValue(const std::string& section_name, const std::string& item_key, const int default_value)
{
    if (section_name == "" || item_key == "")
        return default_value;

    std::string item_value = IniGetValue(section_name, item_key);

    if (item_value == "")
        return default_value;

    return atoi(item_value.c_str());
}

int64_t IniFileReader::IniGetInt64Value(const std::string& section_name, const std::string& item_key, const int64_t default_value)
{
    if (section_name == "" || item_key == "")
        return default_value;

    std::string item_value = IniGetValue(section_name, item_key);

    if (item_value == "")
        return default_value;

    return strtoll(item_value.c_str(), NULL, 10);
}

bool IniFileReader::IniGetBoolValue(const std::string& section_name, const std::string& item_key, const bool default_value)
{
    const char *pValue;

    if (section_name == "" || item_key == "")
        return default_value;

    std::string item_value = IniGetValue(section_name, item_key);

    if (item_value == "")
        return default_value;

    pValue = item_value.c_str();

    if (strcasecmp(pValue, "true") == 0 || strcasecmp(pValue, "yes") == 0 || strcasecmp(pValue, "on") == 0 || strcmp(pValue, "1") == 0) {
        return true;
    } else {
        return false;
    }
}

double IniFileReader::IniGetDoubleValue(const std::string& section_name, const std::string& item_key, const double default_value)
{
    if (section_name == "" || item_key == "")
        return default_value;

    std::string item_value = IniGetValue(section_name, item_key);

    if (item_value == "")
        return default_value;

    return strtod(item_value.c_str(), NULL);
}

int IniFileReader::IniGetSecValues(const std::string& section_name, std::vector<IniFileReader::SecKeyValue>& sec_kv)
{
    if (section_name == "")
        return -1;

    sec_kv.clear();

    if (ini_kv_map.find(section_name) == ini_kv_map.end())
        return -1;

    std::vector<std::string> sec_keys;
    if (StringAlgorithms::Split(ini_kv_map[section_name], SEC_KEYS_DELIM, sec_keys) == 0)
        return -1;

    for (unsigned int i = 0; i < sec_keys.size(); i++)
    {
        SecKeyValue kv = { sec_keys[i], IniGetValue(section_name, sec_keys[i]) };
        sec_kv.push_back(kv);
    }

    return 0;
}

bool IniFileReader::HasSection(const std::string& section_name) const
{
    return (ini_kv_map.find(section_name) != ini_kv_map.end());
}

bool IniFileReader::SectionIsUnique(const std::string& section_name) const
{
    std::map<std::string, int32_t>::const_iterator find_it = section_count.find(section_name);
    if (find_it == section_count.end())
        return false;
    if (find_it->second != 1)
        return false;
    return true;
}

bool IniFileReader::HasKeyValue(const std::string& key, const std::string& value) const
{
    std::map<std::string, std::map<std::string, int32_t> >::const_iterator key_find_it =
        ini_kv_status_map.find(key);
    if (key_find_it == ini_kv_status_map.end())
        return false;

    const std::map<std::string, int32_t>& value_map = key_find_it->second;
    std::map<std::string, int32_t>::const_iterator value_find_it = value_map.find(value);
    if (value_find_it == value_map.end())
        return false;

    return true;
}

bool IniFileReader::KeyValueIsUnique(const std::string& key, const std::string& value) const
{
    std::map<std::string, std::map<std::string, int32_t> >::const_iterator key_find_it =
        ini_kv_status_map.find(key);
    if (key_find_it == ini_kv_status_map.end())
        return false;

    const std::map<std::string, int32_t>& value_map = key_find_it->second;
    std::map<std::string, int32_t>::const_iterator value_find_it = value_map.find(value);
    if (value_find_it == value_map.end())
        return false;

    if (value_find_it->second != 1)
        return false;

    return true;
}

/************************************************************************
 * PRIVATE FUNCTIONS IMPLEMENTATION
 ************************************************************************/

std::string IniFileReader::IniGetValue(const std::string& section_name, const std::string& item_key)
{
    if (ini_kv_map.find(item_key + SEC_KEY_DELIM_STR + section_name) == ini_kv_map.end())
        return "";

    return ini_kv_map[item_key + SEC_KEY_DELIM_STR + section_name];
}

}

