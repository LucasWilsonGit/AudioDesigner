#pragma once

#include <concepts>
#include <string_view>
#include <variant>
#include <tuple>
#include <vector>
#include <iostream>
#include <optional>
#include <map>

#include "tokenizers.hpp"
#include "token_stream.hpp"
#include "argdefs.hpp"
#include "parsers.hpp"

#include "template_magic.hpp"

namespace ArgParser {
    

    
    template <class Tokenizer, class Tuple>
    concept ValidTokenizer = requires(std::string_view s) {
        { Tokenizer::tokenize(s) } -> std::same_as<std::optional<typename Tokenizer::return_t>>;
        tuple_contains_v<typename Tokenizer::return_t, Tuple>;
    };

    //tokens

    

    //tokenizers

    
    //token stream

    


    //default arg definitions

   


    //arg parser
    template <class T>
    struct is_tuple : std::false_type {};

    template <class... Ts>
    struct is_tuple<std::tuple<Ts...>> : std::true_type {};

    template <class T>
    constexpr bool is_tuple_v = is_tuple<T>::value;

    template <class T>
    concept TupleType = is_tuple<T>::value;

    template <class T>
    struct parser_value_type_resolver {
        using type = void
    };

    template <ParserType T>
    struct parser_value_type_resolver {
        using type = typename T::return_t;
    };

    template <>
    struct parser_value_type_resolver<void> {
        using type = bool;
    };

    template <class T>
    using parser_value_type_resolver_t = typename parser_value_type_resolver<T>::type;


    template <class ValueT = void, class ParserT = void>
    struct argument_descriptor {

        using parser_t = parser_value_type_resolver_t<ParserT>;

        sd::optional<char> tag;
        std::string_view name;
        std::optional<ValueT> value;

        bool matches_identifier(identifier_token const& tok) {
            return std::visit([](auto const& val) -> bool {
                if constexpr (std::is_same_v<char, decltype(val)>) {
                    return tag.has_value() && *tag == val; 
                }
                else if constexpr (std::is_same_v<std::string_view, decltype(val)>) {
                    return val == name;
                }
                else {
                    return false;
                }
            }, tok);
        }

        void operator=(ValueT const& val) {
            value = val;
        }
    };

    template <class T>
    struct is_argument_descriptor : std::false_type {};

    template <class T>
    struct is_argument_descriptor<argument_descriptor<T>> : std::true_type {};

    template <class T>
    constexpr bool is_argument_descriptor_v = is_argument_descriptor<T>::value;

    template <class T>
    concept ArgumentDescriptor = is_argument_descriptor_v<T>;

    template <class T>
    struct alternatives_parser_from_tuple;

    template <class... Ts>
    struct alternatives_parser_from_tuple<std::tuple<Ts...>> {
        using type = alternatives_parser<select_step_parser_t<Ts>...>;
    };



    template <class T>
    struct parsers_from_arg_descriptors_tuple;

    template <ArgumentDescriptor... Ts>
    struct parsers_from_arg_descriptors_tuple<std::tuple<Ts...>> {
    private:
        template <ArgumentDescriptor T>    
        using parser_if_valid = std::conditional_t<
            std::is_void_v<typename T::ParserType
    }

    template <TupleType T>
    using alternatives_parser_from_tuple_t = typename alternatives_parser_from_tuple<T>::type;

    template <
        TokenType TokTy,
        TupleType TokenizersTuple = std::tuple<identifier_tokenizer, path_tokenizer, integer_tokenizer, string_tokenizer>, 
        TupleType ArgDescriptorsTuple = std::tuple<>,
        TupleType ExtraValueParsersTuple = std::tuple<>
    >
    struct arg_parser_traits {
        using arg_descriptors_parsers

        using option_value_parser = alternatives_parser_from_tuple_t<TokenizersTuple>
        using option_assignment_parser = sequence_parser<identifier_token, option_value_parser>;

    };

    template <
        TokenType TokTy,
        TupleType TokenizersTuple = std::tuple<identifier_tokenizer, path_tokenizer, integer_tokenizer, string_tokenizer>, 
        TupleType ArgDescriptorsTuple = std::tuple<>,
        TupleType ExtraValueParsersTuple = std::tuple<>
    >
    class arg_parser {
        using traits = arg_parser_traits;
        using token_stream_t = token_stream<TokTy>;
        using descriptors_t = std::tuple<ArgTys...>;
        using value_t = std::variant<typename ArgTys::ValueType...>

        descriptors_t descriptors;
        std::map<identifier_token, std::optional<std::variant<typename ArgTys::ValueType...>>> options;
        std::vector<std::
    };
}
