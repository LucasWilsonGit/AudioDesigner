#pragma once

#include <concepts>
#include <string_view>
#include <variant>
#include <tuple>
#include <vector>
#include <iostream>

#include "tokenizers.hpp"
#include "token_stream.hpp"
#include "argdefs.hpp"

#include "template_magic.hpp"

namespace ArgParser {
    

    
    template <class Tokenizer, class Tuple>
    concept is_valid_tokenizer = requires(std::string_view s) {
        { Tokenizer::tokenize(s) } -> std::same_as<std::optional<typename Tokenizer::return_type>>;
        tuple_contains_v<typename Tokenizer::return_type, Tuple>;
    };

    //tokens

    

    //tokenizers

    
    //token stream

    


    //default arg definitions
    


    //arg parser

    template <class TokenizersTuple, class ArgDefinitionsTuple, class ExtraValueParsersTuple>
    struct arg_parser;

    template <class... Tokenizers, is_arg_definition... ArgDefinitions, class... ExtraValueParsers> 
    struct arg_parser<std::tuple<Tokenizers...>, std::tuple<ArgDefinitions...>, std::tuple<ExtraValueParsers...>> {
        using token_types_tuple_t = std::tuple<typename Tokenizers::return_type...>;
        using token_t = std::variant<typename Tokenizers::return_type...>;

        using tokenizers_pack = std::tuple<Tokenizers...>;
        static constexpr size_t tokenizers_count = sizeof...(Tokenizers);

        using parsers_pack = tuple_combine_t< std::tuple<ExtraValueParsers...>, std::tuple<typename ArgDefinitions::value_parser_t...>>;
        using arg_defs = std::tuple<ArgDefinitions...>;

        token_stream<token_t> m_tokens;

        arg_parser(int argc, char const** argv) 
        :   m_tokens()
        {
            if (argc < 0)
                throw std::runtime_error("NEGATIVE NUM ARGUMENTS?!");
            m_tokens.reserve(static_cast<size_t>(argc));
            parse(argc, argv);
        }

        void parse(int argc, char const**argv) {
            for (int i = 0; i < argc; i++) {
                std::string_view raw = std::string_view(argv[i]);
                std::cerr << "Trying to parse raw string " << raw << "\n";

                if (!try_tokenizers<Tokenizers...>(raw))
                    throw std::runtime_error("Failed to tokenize token");
            }
        }

    private:
        template <class Tokenizer, class... Rest>
        bool try_tokenizers(std::string_view s) {
            using return_type = Tokenizer::return_type;

            if (std::optional<return_type> result = Tokenizer::tokenize(s)) {
                m_tokens.emplace_back(*result);
                return true;
            }

            if constexpr (sizeof...(Rest) != 0) {
                return try_tokenizers<Rest...>(s);
            }
            else
                return false;
        }
    };
}
