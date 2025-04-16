#include "tokenizers.hpp"
#include "parsers.hpp"
#include "token_stream.hpp"
#include "arg_parser.hpp"

#include <tuple>
#include <map>
#include <variant>

#include <gtest/gtest.h>

using namespace ArgParser;
using option_value_parser = alternatives_parser<int_token, path_token, string_token>;
using option_assignment_parser = sequence_parser<identifier_token, option_value_parser>;
using option_flag_parser = token_unwrap_parser<identifier_token>;

using _test_invalid_token = std::tuple<>;
using token_t = token_wrapper_t<string_token, int_token, path_token, identifier_token, _test_invalid_token>;

auto make_stream() {
    ArgParser::token_stream<token_t> tokens(
        std::vector<token_t>{{
            identifier_token('c'),
            path_token("./conf.cfg"),
            identifier_token("use_cache"),
            identifier_token("page_size"),
            string_token("2mb"),
            identifier_token("page_count"),
            int_token(16)
        }}
    );

    return tokens;
}

TEST(ArgParserTests, TestSetup) {
    auto stream = make_stream();

    EXPECT_FALSE(stream.is_empty());
    EXPECT_EQ(stream.size(), 7);
    
    auto const& first_token = std::get<ArgParser::identifier_token>(stream.current_token()); 
    EXPECT_EQ(std::get<char>(first_token) , 'c');
}

struct value_repackager {
    identifier_token iden;
    std::map<identifier_token, std::variant<int_token, path_token, string_token, bool>>& results;

    value_repackager() = delete;
    value_repackager(identifier_token i, decltype(results)& res) 
    :   iden(i),
        results(res)
    {}

    void operator()(int_token const& v) {
        results.emplace(iden, v);
    }
    void operator()(path_token const& v) {
        results.emplace(iden, v);
    }
    void operator()(string_token const& v) {
        results.emplace(iden, v);
    }
};

struct argsparse_visitor {
    std::map<identifier_token, std::variant<int_token, path_token, string_token, bool>> results;
    std::vector<typename option_value_parser::return_t> positional_results;

    void operator()(typename option_assignment_parser::return_t const& v) {
        auto [identifier, value] = v;

        std::visit(value_repackager(identifier, results), value);
    }
    void operator()(typename option_flag_parser::return_t const& v) {   
        results.emplace(v, true);
    }
    void operator()(typename option_value_parser::return_t const& v) {
        positional_results.emplace_back(std::move(v));
    }
};

struct stringify_value {
    std::string operator()(int_token tok) {
        if (std::holds_alternative<uint64_t>(tok)) {
            return std::to_string(std::get<uint64_t>(tok));
        }
        else
            return std::to_string(std::get<int64_t>(tok));
    }
    std::string operator()(path_token tok) {
        return tok.string();
    }
    std::string operator()(string_token tok) {
        return std::string(tok);
    }
    std::string operator()(bool tok) {
        if (tok) {
            return "true";
        }
        else
            return "false";
    }
};

TEST(ArgParserTests, TestParseValidStream) {
    auto stream = make_stream();

    using parser_t = loop_parser<alternatives_parser<option_assignment_parser, option_flag_parser, option_value_parser>>; //gonna give us a vector of variant of our specific parsers outs
    auto result = parser_t::parse(stream);
    EXPECT_TRUE((bool)result);

    EXPECT_TRUE(stream.is_empty()); //There should no be leftover tokens after parsing a valid token stream

    argsparse_visitor v;
    for (auto const& alternate : *result ) {
        std::visit(v, alternate);
    }

    auto stringify_identifier = [](auto tok) {
        if constexpr (std::is_same_v<decltype(tok), char>)
            return std::string(1, tok);
        else
            return std::string(tok);
    };

    for (auto [key, value] : v.results) {
        std::cerr << std::visit(stringify_identifier, key) << " " << std::visit(stringify_value(), value) << "\n";
    }

    int i =0;
    for (auto res : v.positional_results) {
        std::cerr << i++ << " " << std::visit(stringify_value{}, res) << "\n"; 
    }
}


