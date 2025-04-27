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
    
    
    
    

    //tokens

    

    //tokenizers

    
    //token stream

    


    //default arg definitions

   


    //arg parser

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
            return std::visit([this](auto const& val) -> bool {
                if constexpr (std::is_same_v<char, decltype(val)>) {
                    return this->iden.tag.has_value() && *(this->iden.tag) == val; 
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
    struct variant_from_pack {
        using type = std::variant<Ts...>;
    };

    template <class... Ts>
    using variant_from_pack_t = typename variant_from_pack<Ts...>::type;

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
        using value_parsers_pack = tuple_combine_t< arg_descriptors_parsers, extra_value_parsers>;
        using option_value_parser = alternatives_parser_from_parsers_tuple_t<value_parsers_pack>;
        using value_t = typename option_value_parser::return_t;
        using option_assignment_parser = sequence_parser<identifier_token, option_value_parser>;
        using all_options_assignment_parser = loop_parser<alternatives_parser<option_assignment_parser, identifier_token>>;
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

        template <size_t Idx>
        bool find_arg_impl(identifier_token const& iden, auto& out_res) {
            std::tuple_element_t<Idx, ArgDescriptorsTuple>& elem = std::get<Idx>(m_descriptors); 
            if (elem.matches_identifier(iden)) {
                out_res = elem;
                return true;
            }
            return false;
        }
        template <size_t... sizes>
        auto find_arg_helper(identifier_token const& iden, std::index_sequence<sizes...>) { //gives back some kind of reference_wrapper
            auto res = std::optional<variant_from_pack_t<std::reference_wrapper<std::tuple_element_t<sizes, ArgDescriptorsTuple>>...>>();
            (find_arg_impl<sizes>(iden, res) || ...); //keep going until we find one

            return res;
        }
        auto find_arg(identifier_token const& iden) {
            return find_arg_helper(iden, std::make_index_sequence<std::tuple_size_v<ArgDescriptorsTuple>>());
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

        void assign_option_value(identifier_token const& iden, auto value) {
            if (auto arg = find_arg(iden)) { //arg is optional<variant<reference_wrapper<T>...>>
                std::visit([rv = std::ref(value)]<class Q>(std::reference_wrapper<Q> ref_wrap)  -> void {
                    if (!std::is_same_v<decltype(ref_wrap.get().value), std::optional<decltype(rv.get())>>) {
                        throw std::runtime_error("Assertation failed");
                    }
                    using value_t = typename Q::value_t;

                    ref_wrap.get().value = *(value_t*)&rv.get();
                }, *arg);
                
            }
        }

        void parse(token_stream<TokTy>& stream) {
            
            auto res = qual_traits::parser_impl_t::parse(stream);
            if (res) {
                typename qual_traits::all_options_assignment_parser::return_t& option_assignment_vector = std::get<0>(*res);
                typename qual_traits::all_positional_parser::return_t& positional_values_vector = std::get<1>(*res);

                (volatile void*)&positional_values_vector;

                for (auto const& assignment : option_assignment_vector) {
                    std::visit(overloads{
                        [this](typename qual_traits::option_assignment_parser::return_t const& inst) {
                            auto const& [iden, value] = inst;
                            std::cout << "Assigned " << std::remove_reference_t<std::decay_t<decltype(*this)>>::get_identifier_repr(iden) << " with a value\n";

                            this->assign_option_value(iden, value);
                        },
                        [this](identifier_token const& tok) {

                            std::cout << "Found standlone flag " <<  std::remove_reference_t<std::decay_t<decltype(*this)>>::get_identifier_repr(tok) << "\n";

                            this->assign_option_value(tok, true);
                        }
                    }, assignment);
                }
            }
        }
    private:
        static std::string get_identifier_repr(identifier_token const& tok) {
            return std::visit(overloads{
                [](string_token const& name) {
                    return std::string(name);
                },
                [](char const& c) {
                    return std::string(1, c);
                }
            }, tok);
        }


    };
}
