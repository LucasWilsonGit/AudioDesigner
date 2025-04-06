#pragma once

#include <string_view>
#include <optional>

namespace ArgParser {

    template <class Parser>
    struct arg_definition {
        using value_parser_t = Parser;

        arg_definition() = delete;
        arg_definition(char c, std::string_view name, std::string_view description)
        :   m_tag(c),
            m_name(name),
            m_description(description)
        {}

        arg_definition(std::string_view name, std::string_view description)
        :   m_tag(std::nullopt),
            m_name(name),
            m_description(description)
        {}

        std::optional<char> const m_tag;
        std::string_view const m_name;
        std::string_view const m_description;
    };

    template <class T>
    concept is_arg_definition = requires(T inst, char c, std::string_view n, std::string_view d) {
        {T::arg_definition(c, n, d)};
        {T::parser_t};
        { inst.m_tag } -> std::same_as<std::optional<char>>;
        { inst.m_name } -> std::same_as<std::string_view>;
        { inst.m_description } -> std::same_as<std::string_view>;
    };
}