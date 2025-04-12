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

    template <class ValueParserT = void>
    struct argument_descriptor {

        using parser_t = select_step_parser_t<ValueParserT>;

        sd::optional<char> tag;
        std::string_view name;
        std::optional<typename parser_t::return_t> value;

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

    template <TupleType T>
    using alternatives_parser_from_tuple_t = typename alternatives_parser_from_tuple<T>::type;



    template <class T>
    struct parsers_from_arg_descriptors_tuple;

    template <ArgumentDescriptor... Ts>
    struct parsers_from_arg_descriptors_tuple<std::tuple<Ts...>> {
    private:
        template <ArgumentDescriptor T>    
        using parser_if_valid = std::conditional_t<
            std::is_void_v<typename (typename T::parser_t)::return_t>,
            std::tuple<>,
            std::tuple<typename T::parser_t>;
    public:
        using type = decltype(std::tuple_cat(declval(parser_if_valid<Ts>)...));
    };

    template <class... Ts>
    struct parsers_from_extra_values_tuple;

    template <class... Ts>
    struct parsers_from_extra_values_tuple<std::tuple<Ts...>> {
        using type = std::tuple<select_step_parser_t<Ts>...>;
    };


    template <
        TokenType TokTy,
        TupleType TokenizersTuple, 
        TupleType ArgDescriptorsTuple,
        TupleType ExtraValueParsersTuple 
    >
    struct arg_parser_traits {
        using token_stream_t = token_stream<TokTy>;
        using descriptors_t = std::tuple<ArgTys...>;
        using arg_descriptors_parsers = typename parsers_from_arg_descriptors_tuple<ArgDescriptorsTuple>::type;
        using extra_value_parsers = typename parsers_from_extra_values_tuple<ExtraValueParsersTuple>::type;
        using value_parsers_pack = tuple_combine_t< arg_descriptors_parsers, extra_value_parsers >;
        
        using option_value_parser = alternatives_parser< value_parsers_pack >;
        using value_t = typename option_value_parser::return_t;

        using option_assignment_parser = sequence_parser<identifier_token, option_value_parser>;
        using all_options_assignment_parser = loop_parser<option_assignment_parser>;

        using all_positional_parser = loop_parser<option_value_parser>;

        using arg_parser = sequence_parser< all_options_assignment_parser, all_positional_parser >; //loop of options assignments, then positional arguments
    };

    template <
        TokenType TokTy,
        TupleType TokenizersTuple = std::tuple<identifier_tokenizer, path_tokenizer, integer_tokenizer, string_tokenizer>, 
        TupleType ArgDescriptorsTuple = std::tuple<>,
        TupleType ExtraValueParsersTuple = std::tuple<>
    >
    class arg_parser : arg_parser_traits {

        descriptors_t descriptors;
        std::map< identifier_token, std::optional<value_t> > options;
        
        
    };
}
