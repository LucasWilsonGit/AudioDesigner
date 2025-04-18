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
    
    template <class... Ts> struct overload : Ts... { using Ts::operator()...; };
    template <class... Ts> overload(Ts...) -> overload<Ts...>;
    
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

    struct argument_identifier {
        std::string_view name;
        std::optional<char> tag;
        uint8_t _padding[6];
    };

    bool operator<(argument_identifier const& l, argument_identifier const& r) {
        if (l.tag && r.tag) {
            return (l.tag < r.tag);
        }
        return l.name < r.name;
    }

    template <class ValueParserT = void>
    struct argument_descriptor {

        using parser_t = select_step_parser_t<ValueParserT>;
        using _parser_return_t = typename parser_t::return_t;
        using value_t = std::conditional_t<!std::is_void_v<_parser_return_t>, std::optional<_parser_return_t>, std::optional<bool>>;

        argument_identifier iden;
        value_t value;

        bool matches_identifier(identifier_token const& tok) {
            return std::visit([](auto const& val) -> bool {
                if constexpr (std::is_same_v<char, decltype(val)>) {
                    return iden.tag.has_value() && *(iden.tag) == val; 
                }
                else if constexpr (std::is_same_v<std::string_view, decltype(val)>) {
                    return val == iden.name;
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
            std::is_void_v<typename T::parser_t::return_t>,
            std::tuple<>,
            std::tuple<typename T::parser_t>
        >;
    public:
        using type = tuple_combine_multi_t<parser_if_valid<Ts>...>;
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
        using arg_descriptors_parsers = typename parsers_from_arg_descriptors_tuple<ArgDescriptorsTuple>::type;
        using extra_value_parsers = typename parsers_from_extra_values_tuple<ExtraValueParsersTuple>::type;
        using value_parsers_pack = tuple_combine_t< arg_descriptors_parsers, extra_value_parsers >;
        using option_value_parser = alternatives_parser_from_parsers_tuple_t<value_parsers_pack>;
        using value_t = typename option_value_parser::return_t;
        using option_assignment_parser = sequence_parser<identifier_token, option_value_parser>;
        using all_options_assignment_parser = loop_parser<option_assignment_parser>;
        using all_positional_parser = loop_parser<option_value_parser>;
        using parser_impl_t = sequence_parser< all_options_assignment_parser, all_positional_parser >; //loop of options assignments, then positional arguments
    };
    template <
        TokenType TokTy,
        TupleType TokenizersTuple = std::tuple<identifier_tokenizer, path_tokenizer, integer_tokenizer, string_tokenizer>, 
        TupleType ArgDescriptorsTuple = std::tuple<>,
        TupleType ExtraValueParsersTuple = std::tuple<>
    >
    class arg_parser {
        using qual_traits = arg_parser_traits<TokTy, TokenizersTuple, ArgDescriptorsTuple, ExtraValueParsersTuple>;

        ArgDescriptorsTuple m_descriptors;
        typename qual_traits::all_positional_parser::return_t positional_arguments; //vector< variant< value_ts... > >;

        template <size_t I>
        void init_arg() {
            using arg_I_t = std::tuple_element_t<I, ArgDescriptorsTuple>;
            auto& arg_desc = std::get<I>(m_descriptors);
            if constexpr (std::is_void_v<typename arg_I_t::_parser_return_t>) {
                arg_desc.value = false;
            }
        }

        template <size_t... Is>
        void init_args(std::index_sequence<Is...>) {
            (init_arg<Is>(), ...);
        }
    public:
        explicit arg_parser(ArgDescriptorsTuple&& arg_descriptors, ExtraValueParsersTuple extra_value_parsers) 
        :   m_descriptors(std::forward<ArgDescriptorsTuple>(arg_descriptors)) 
        {
            init_args(std::make_index_sequence<std::tuple_size_v<ArgDescriptorsTuple>>{});
        }

        explicit arg_parser(ArgDescriptorsTuple&& arg_descriptors) 
        :   m_descriptors(std::forward<ArgDescriptorsTuple>(arg_descriptors)) 
        {
            init_args(std::make_index_sequence<std::tuple_size_v<ArgDescriptorsTuple>>{});
        }

        void parse(token_stream<TokTy>& stream) {
            using option_assignment_sequence_parse_t = typename qual_traits::option_assignment_parser::return_t;
            
            auto res = qual_traits::parser_impl_t::parse(stream);
            if (res) {
                typename qual_traits::all_options_assignment_parser::return_t& option_assignment_vector = std::get<0>(*res);
                typename qual_traits::all_positional_parser::return_t& positional_values_vector = std::get<1>(*res);

                (volatile void*)&positional_values_vector;

                for (option_assignment_sequence_parse_t const& assignment : option_assignment_vector) {
                    auto const& [iden, value] = assignment;
                    
                    std::visit(overload{
                        [](std::string_view const& name) {
                            std::cout << "Found name identifier: " << name << "\n";
                        },
                        [](char const& tag) {
                            std::cout << "Found tag identifier: " << tag << "\n";
                        }
                    }, iden);
                }
            }
        }
        
    };
}
