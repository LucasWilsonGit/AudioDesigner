#pragma once

#include <variant>
#include <string_view>
#include <optional>
#include <stdexcept>
#include <iostream>
#include <numeric>
#include <charconv>
#include <filesystem>

#include "template_magic.hpp"

namespace ArgParser {

    using string_token = std::string_view;
    using identifier_token = std::variant<char, std::string_view>;
    using int_token = std::variant<uint64_t, int64_t>;
    using path_token = std::filesystem::path;
    
    template <class Tokenizer, class Tuple>
    concept ValidTokenizer = requires(std::string_view s) {
        { Tokenizer::tokenize(s) } -> std::same_as<std::optional<typename Tokenizer::return_t>>;
        tuple_contains_v<typename Tokenizer::return_t, Tuple>;
    };

    //example of a trivial tokenizer
    struct string_tokenizer {
        using return_t = std::string_view;

        static std::optional<std::string_view> tokenize(std::string_view s) {
            return s;
        }
    };

    struct unsigned_integer_tokenizer {
        using return_t = uint64_t;

        static std::optional<return_t> tokenize(std::string_view s) {
            if (s.empty()) return std::nullopt;
            if (s.front() == '-') [[unlikely]] return std::nullopt;
            
            uint8_t base = 10;
            if (s.starts_with("0x")) base = 16;

            uint64_t result;
            auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.length(), result, base);
            if (ec == std::errc::result_out_of_range || ec == std::errc::invalid_argument || ptr != s.data() + s.length()) {
                return std::nullopt;
            }
            return result;
        }
    };

    struct signed_integer_tokenizer {
        using return_t = int64_t;

        static std::optional<return_t> tokenize(std::string_view s) {
            if (s.empty()) return std::nullopt;

            s = s.substr(1); //skip the negative sign

            std::optional<uint64_t> result = unsigned_integer_tokenizer::tokenize(s);
            if (result) {
                uint64_t dat = *result;
                if (dat > static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
                    return std::nullopt;
                }

                return -1 * static_cast<int64_t>(dat);
            }

            return std::nullopt;
        }
    };

    struct integer_tokenizer {
        using return_t = int_token;

        static std::optional<return_t> tokenize(std::string_view s) {
            if (auto res = unsigned_integer_tokenizer::tokenize(s); res.has_value() ) {
                return *res;
            }
            else
                return signed_integer_tokenizer::tokenize(s); 

        }
    };

    //identifier tokenizer
    struct identifier_tokenizer {
        using return_t = identifier_token;

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

    struct path_tokenizer {
        using return_t = path_token;

        static std::optional<return_t> tokenize(std::string_view word) {
            if (word.empty()) return std::nullopt;
            return_t res;
            try { res = return_t(word); } catch ( ... ) { return std::nullopt; }
            return res;
        }
    };





    
}