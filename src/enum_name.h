#pragma once

#ifndef __ENUM_NAME_H__
#define __ENUM_NAME_H__

#include <vector>
#include <string>


// Add the definition of this method into a cpp file. (only the declaration in the header)
const std::vector<std::string> &get_enum_names(
    const std::string &en_key,
    const std::string &en_str);

#define DECLARE_ENUM(ENUM_NAME, ENUM_TYPE, ...)                                    \
    enum class ENUM_NAME : ENUM_TYPE                                               \
    {                                                                              \
        __VA_ARGS__                                                                \
    };                                                                             \
    inline std::string get_enum_name(ENUM_NAME en)                                 \
    {                                                                              \
        const auto names = get_enum_names(#ENUM_NAME #__VA_ARGS__, #__VA_ARGS__); \
        return names[static_cast<std::size_t>(en)];                                \
    }

#endif // __ENUM_NAME_H__