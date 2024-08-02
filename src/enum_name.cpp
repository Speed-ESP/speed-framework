#include "enum_name.h"
#include <iostream>
#include <algorithm>
#include <unordered_map>
const std::vector<std::string> &get_enum_names(const std::string &en_key, const std::string &en_str)
{
    static std::unordered_map<std::string, std::vector<std::string>> en_names_map;
    const auto it = en_names_map.find(en_key);
    if (it != en_names_map.end())
        return it->second;

    constexpr auto delim(',');
    std::vector<std::string> en_names;
    std::size_t start{};
    auto end = en_str.find(delim);
    while (end != std::string::npos)
    {
        while (en_str[start] == ' ')
            ++start;
        en_names.push_back(en_str.substr(start, end - start));
        start = end + 1;
        end = en_str.find(delim, start);
    }
    while (en_str[start] == ' ')
        ++start;
    en_names.push_back(en_str.substr(start));
    return en_names_map.emplace(en_key, std::move(en_names)).first->second;
}