#pragma once

#ifndef __ENUM_NAME_H__
#define __ENUM_NAME_H__

#include "for_each.h"
#include <vector>
#include <string>
#include "property.hpp"
#include <map>

#include <format>
#include <iostream>
#include <cstdint>

template <typename TUnderlineType, class TEnum>
struct EnumBase
{
protected:
    TUnderlineType value;
    constexpr EnumBase(TUnderlineType value) : value(value) {}
    constexpr EnumBase() : value(TUnderlineType(0)) {}
    constexpr EnumBase(const TEnum &other) : value(other.value) {}

public:
    virtual std::string get_name() = 0;

    inline const char *get_c_name()
    {
        return get_name().c_str();
    }

    constexpr inline TUnderlineType get_value() const { return value; }
    constexpr TEnum &operator=(TUnderlineType other)
    {
        value = other;
        return *this;
    }

    constexpr operator TUnderlineType() const { return value; }

    constexpr bool operator==(const TEnum &rhs)
    {
        return value == rhs.value;
    }

    constexpr bool operator!=(const TEnum &rhs)
    {
        return value != rhs.value;
    }
    char *to_string() const
    {
        return get_name();
    }

    friend std::ostream &operator<<(std::ostream &os, const TEnum &obj)
    {
        os << obj.get_name();
        return os;
    }
};

template <typename TUnderlineType, class TEnum>
struct FlagEnum : public EnumBase<TUnderlineType, TEnum>
{
protected:
    constexpr FlagEnum(TUnderlineType value) : EnumBase<TUnderlineType, TEnum>(value) {}
    constexpr FlagEnum() : EnumBase<TUnderlineType, TEnum>() {}
    constexpr FlagEnum(const TEnum &other) : EnumBase<TUnderlineType, TEnum>(other) {}

public:
    constexpr TEnum operator|(const TEnum &rhs) const
    {
        return TEnum(static_cast<TUnderlineType>(static_cast<TUnderlineType>(*this) | static_cast<TUnderlineType>(rhs)));
    }

    constexpr TEnum &operator&(const TEnum &rhs) const
    {
        return TEnum(static_cast<TUnderlineType>(static_cast<TUnderlineType>(*this) & static_cast<TUnderlineType>(rhs)));
    }

    constexpr TEnum &operator~() const
    {
        return TEnum(static_cast<TUnderlineType>(~static_cast<TUnderlineType>(*this)));
    }

    constexpr bool operator<(const TEnum &rhs)
    {
        return static_cast<TUnderlineType>(*this) < rhs.value;
    }

    constexpr bool operator>(const TEnum &rhs)
    {
        return static_cast<TUnderlineType>(*this) > rhs.value;
    }

    constexpr bool operator<=(const TEnum &rhs)
    {
        return static_cast<TUnderlineType>(*this) <= rhs.value;
    }
};

template <typename TUnderlineType, class TEnum>
struct std::formatter<EnumBase<TUnderlineType, TEnum>>
{
    bool _is_string{false};
    constexpr auto parse(std::format_parse_context &ctx)
    {
        auto pos = ctx.begin();
        while (pos != ctx.end() && *pos != '}')
        {
            if (*pos == 's' || *pos == 'S')
                _is_string = true;
            ++pos;
        }
        return pos; // expect `}` at this position, otherwise it's error! exception!
    }

    auto format(const EnumBase<TUnderlineType, TEnum> &_enum, std::format_context &ctx)
    {
        if (_is_string)
            return std::format_to(ctx.out(), "({})", _enum.get_name());
        return std::format_to(ctx.out(), "({})", _enum.get_value());
    }
};

#define NOOP_OPERATORS(ENUM_NAME, ENUM_TYPE, ...) // No custom operators
#define DEFAULT_INHERITANCE(ENUM_NAME, ENUM_TYPE, ...)  EnumBase

#define _DECLARE_ENUM_BASE(ENUM_NAME, ENUM_TYPE, OPTIONS_DECLARATION, OPTIONS_INITIALIZATION, NAMES_INITIALIZATION, INHERITANCE, OPERATORS_DEFINITION, ...) \
    struct ENUM_NAME : public EXPAND(INHERITANCE(ENUM_NAME, ENUM_TYPE, __VA_ARGS__))<ENUM_TYPE, ENUM_NAME>                                                         \
    {                                                                                                                                                       \
        constexpr ENUM_NAME(ENUM_TYPE value) : EXPAND(INHERITANCE(ENUM_NAME, ENUM_TYPE, __VA_ARGS__))(value) {}                                             \
        constexpr ENUM_NAME() : EXPAND(INHERITANCE(ENUM_NAME, ENUM_TYPE, __VA_ARGS__))() {}                                                                 \
        constexpr ENUM_NAME(const ENUM_NAME &other) : EXPAND(INHERITANCE(ENUM_NAME, ENUM_TYPE, __VA_ARGS__))(other) {}                                      \
        EXPAND(OPERATORS_DEFINITION(ENUM_NAME, ENUM_TYPE, __VA_ARGS__))                                                                                     \
                                                                                                                                                            \
        EXPAND(OPTIONS_DECLARATION(ENUM_NAME, ENUM_TYPE, __VA_ARGS__))                                                                                      \
        inline std::string get_name() override                                                                                                              \
        {                                                                                                                                                   \
            return names.at(*this);                                                                                                                         \
        }                                                                                                                                                   \
                                                                                                                                                            \
    private:                                                                                                                                                \
        inline static const std::map<ENUM_TYPE, std::string, std::equal_to<ENUM_NAME>> names = {                                                            \
            EXPAND(NAMES_INITIALIZATION(ENUM_NAME, ENUM_TYPE, __VA_ARGS__))};                                                                               \
    };                                                                                                                                                      \
    EXPAND(OPTIONS_INITIALIZATION(ENUM_NAME, ENUM_TYPE, __VA_ARGS__))

#define _AUTO_ENUM_INITIALIZE_NAME(ENUM_NAME, OPTION, INDEX) {INDEX, #OPTION},
#define _AUTO_ENUM_DECLARE_OPTION(ENUM_NAME, OPTION, INDEX) static const ENUM_NAME OPTION;
#define _AUTO_ENUM_INITIALIZE_OPTION(ENUM_NAME, OPTION, INDEX) constexpr const ENUM_NAME ENUM_NAME::OPTION{INDEX};

#define _AUTO_ENUM_NAMES_INITIALIZATION(ENUM_NAME, ENUM_TYPE, ...) FOR_EACH_PIVOT_1ST_ARG(_AUTO_ENUM_INITIALIZE_NAME, ENUM_NAME, __VA_ARGS__)

#define _AUTO_ENUM_OPTIONS_DECLARATION(ENUM_NAME, ENUM_TYPE, ...) FOR_EACH_PIVOT_1ST_ARG(_AUTO_ENUM_DECLARE_OPTION, ENUM_NAME, __VA_ARGS__)
#define _AUTO_ENUM_OPTIONS_INITIALIZATION(ENUM_NAME, ENUM_TYPE, ...) FOR_EACH_PIVOT_1ST_ARG(_AUTO_ENUM_INITIALIZE_OPTION, ENUM_NAME, __VA_ARGS__)

#define DECLARE_ENUM(ENUM_NAME, ENUM_TYPE, ...)           \
    _DECLARE_ENUM_BASE(ENUM_NAME, ENUM_TYPE,              \
                       _AUTO_ENUM_OPTIONS_DECLARATION,    \
                       _AUTO_ENUM_OPTIONS_INITIALIZATION, \
                       _AUTO_ENUM_NAMES_INITIALIZATION,   \
                       DEFAULT_INHERITANCE,               \
                       NOOP_OPERATORS,                    \
                       __VA_ARGS__)

#define DECLARE_VALUE_ENUM_PROPERTY(ENUM_NAME, NAME, VALUE) static const ENUM_NAME NAME;
#define DECLARE_VALUE_ENUM_NAMES(ENUM_NAME, NAME, VALUE) {VALUE, #NAME},
#define DEFINE_VALUE_ENUM_OPTION(ENUM_NAME, NAME, VALUE) constexpr const ENUM_NAME ENUM_NAME::NAME{VALUE};

#define _PAIR_ENUM_OPTIONS_DECLARATION(ENUM_NAME, ENUM_TYPE, ...) FOR_EACH_PAIR_PIVOT_1ST_ARG(DECLARE_VALUE_ENUM_PROPERTY, ENUM_NAME, __VA_ARGS__)
#define _PAIR_ENUM_OPTIONS_INITIALIZATION(ENUM_NAME, ENUM_TYPE, ...) FOR_EACH_PAIR_PIVOT_1ST_ARG(DEFINE_VALUE_ENUM_OPTION, ENUM_NAME, __VA_ARGS__)
#define _PAIR_ENUM_NAMES_INITIALIZATION(ENUM_NAME, ENUM_TYPE, ...) FOR_EACH_PAIR_PIVOT_1ST_ARG(DECLARE_VALUE_ENUM_NAMES, ENUM_NAME, __VA_ARGS__)

#define DECLARE_VALUE_ENUM(ENUM_NAME, ENUM_TYPE, ...)     \
    _DECLARE_ENUM_BASE(ENUM_NAME, ENUM_TYPE,              \
                       _PAIR_ENUM_OPTIONS_DECLARATION,    \
                       _PAIR_ENUM_OPTIONS_INITIALIZATION, \
                       _PAIR_ENUM_NAMES_INITIALIZATION,   \
                       DEFAULT_INHERITANCE,               \
                       NOOP_OPERATORS,                    \
                       __VA_ARGS__)

#define _FLAG_ENUM_INHERITANCE(ENUM_NAME, ENUM_TYPE, ...) FlagEnum

#define DECLARE_FLAG_ENUM(ENUM_NAME, ENUM_TYPE, ...)      \
    _DECLARE_ENUM_BASE(ENUM_NAME, ENUM_TYPE,              \
                       _PAIR_ENUM_OPTIONS_DECLARATION,    \
                       _PAIR_ENUM_OPTIONS_INITIALIZATION, \
                       _PAIR_ENUM_NAMES_INITIALIZATION,   \
                       _FLAG_ENUM_INHERITANCE,            \
                       NOOP_OPERATORS,                    \
                       __VA_ARGS__)

#endif //__ENUM_NAME_H__