#pragma once

#include <variant>
#include <string_view>
#include <optional>
#include <stdexcept>
#include <iostream>

namespace ArgParser {

    using string_token = std::string_view;
    using identifier_token = std::variant<char, std::string_view>;

    //example of a trivial tokenizer
    struct string_tokenizer {
        using return_type = std::string_view;

        static std::optional<std::string_view> tokenize(std::string_view s) {
            std::cerr << "string_tokenizer returning " << s << "\n";
            return s;
        }
    };

    //identifier tokenizer
    struct identifier_tokenizer {
        using return_type = identifier_token;

        static std::optional<identifier_token> tokenize(std::string_view s) {
            size_t identifier_start = 0;

            //check it starts with - or --, fail for extra consecutive occurrences
            switch ( (identifier_start = s.find_first_not_of("-")) ) {
                case 0:
                    return std::nullopt;
                case 1: 
                    return tokenize_tag(s);
                case 2:
                    return tokenize_name(s);
                default:
                    throw std::runtime_error("Bad argument"); //TODO: Not useless exception
            }

            //Unreachable code
        }
    private:
        static std::optional<identifier_token> tokenize_tag(std::string_view s) {
            if (s.length() != 2)
                throw std::runtime_error("Bad argument");
            
            return s.at(1);
        }

        static std::optional<identifier_token> tokenize_name(std::string_view s) {
            if (s.length() < 3)
                throw std::runtime_error("Bad argument");
            
            return s.substr(2);
        }
    };
}