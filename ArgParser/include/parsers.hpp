#pragma once

#include <iostream>
#include <optional>

#include "token.hpp"
#include "token_stream.hpp"

namespace ArgParser {
#ifndef NDEBUG
    constexpr bool __debugmode = true;
#else
    constexpr bool __debugmode = false;
#endif
        
    template <class Parser>
    concept ParserType = requires(token_stream<token_wrapper_t<int>> s) {
        { Parser::parse(s) };
    };

    

    template <class ExpectedTokenType>
    struct expected_token_extractor {
        constexpr std::optional<ExpectedTokenType> operator()(ExpectedTokenType const& t) {
            return std::optional(t);
        }
        constexpr std::optional<ExpectedTokenType> operator()(auto const&) {
            return std::nullopt;
        }
    };

    /**
     * @brief: Writing various compositional parser types like sequence etc is going to become complex to maintain if each type has to duplicate the implementation of dealing with token types or parser types
     * for each recipe step. Having a single_token_parser that tokens in a composition recipe can be wrapped in a parser type so only parser step handling needs to be implemented  
     * 
     * @param: T The type to attempt to extract from the first token_t in the token_stream
     */
    template <class T>
    struct token_unwrap_parser {
        using return_t = T;

        template <TokenType TokT>
        static std::optional<return_t> parse(token_stream<TokT>& stream) {
            if (stream.is_empty())
                return std::nullopt;

            stream.begin_parse();
            std::optional<T> res;
            if (res = std::visit(expected_token_extractor<T>{}, stream.consume()); res.has_value()) {
                stream.complete_parse();
            }
            else {
                stream.fail_parse();
            }
            return res;
        }
    };

    template <class T>
    struct select_step_parser {
        using type = token_unwrap_parser<T>;
    };

    template <ParserType T>
    struct select_step_parser<T> {
        using type = T;
    };

    template <class T>
    using select_step_parser_t = typename select_step_parser<T>::type;

    template <class... Steps>
    class sequence_parser {
        using parser_ts = std::tuple<select_step_parser_t<Steps>...>;

        using intermediate_t = std::tuple<std::optional<typename select_step_parser_t<Steps>::return_t>...>;
    public:
        static constexpr size_t step_count = sizeof...(Steps);
        using return_t = std::tuple<typename select_step_parser_t<Steps>::return_t...>;

        template <TokenType T>
        static std::optional<return_t> parse(token_stream<T>& stream) {
            intermediate_t intermediate;

            if (stream.is_empty())
                return std::nullopt;
            stream.begin_parse();

            if (!try_all_parsers(stream, intermediate, std::make_index_sequence<step_count>{})) {
                stream.fail_parse();
                return std::nullopt;
            }
            
            stream.complete_parse();
            //convert the elems in intermediate by unpacking them from the optional wrapping them into a new tuple and return it
            return build_result_t(intermediate, std::make_index_sequence<step_count>{});
        } 
    private:
        template <TokenType T, size_t... Is>
        static bool try_all_parsers(token_stream<T> &stream, intermediate_t &resultstore, std::index_sequence<Is...>) {
            return (try_parser<Is>(stream, resultstore) && ...);
        }

        template <size_t I, class TokenType>
        static bool try_parser(token_stream<TokenType>& stream, intermediate_t& result_holder) {

            if (auto parsed = std::tuple_element_t<I, parser_ts>::parse(stream); parsed.has_value()) {
                std::get<I>(result_holder) = *std::move(parsed);
                return true;
            }

            if constexpr (__debugmode) {
                std::cerr << "seequence_parser step " << I << " failed.\n";
            }

            return false;

        }

        template <size_t... Is>
        static return_t build_result_t(intermediate_t& intermediate, std::index_sequence<Is...>) {
            return std::make_tuple( *std::move(  std::get<Is>(intermediate)  ) ... );
        }
    };

    template <class Step>
    class loop_parser {
        using step_parser_t = select_step_parser_t<Step>;
    public:
        using return_t = std::vector<typename select_step_parser_t<Step>::return_t>;
        

        template <TokenType T>
        static std::optional<return_t> parse(token_stream<T>& stream) {
            if (stream.is_empty())
                return std::nullopt;
            stream.begin_parse();
            return_t accum;

            std::optional<typename step_parser_t::return_t> res;
            while ( ( res = step_parser_t::parse(stream) ).has_value() ) {
                accum.push_back(*std::move(res));
            }

            if (accum.size() == 0) {
                stream.fail_parse();
                return std::nullopt;
            }
            stream.complete_parse();
            return accum;
        }
    };

    template <class... Steps>
    class alternatives_parser {
        using parser_ts = std::tuple<select_step_parser_t<Steps>...>;
    public:
        using return_t = std::variant<typename select_step_parser_t<Steps>::return_t...>;
        static constexpr size_t alternatives_count = sizeof...(Steps);

        template <TokenType T>
        static std::optional<return_t> parse(token_stream<T>& stream) {
            stream.begin_parse();

            if ( auto res = first_successful_parse(stream, std::make_index_sequence<alternatives_count>{}); res.has_value() ) {
                stream.complete_parse();
                return res;
            }

            stream.fail_parse();
            return std::nullopt;
        }
    
    private:

        template <size_t... Is>
        static std::optional<return_t> first_successful_parse(auto& stream, std::index_sequence<Is...>) {
            std::optional<return_t> res = std::nullopt;
            ( ( res = std::tuple_element_t<Is, parser_ts>::parse(stream) ) || ...);
            return res;
        }
    };

}