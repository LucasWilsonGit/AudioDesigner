#include "tokenizers.hpp"
#include "arg_parser.hpp"

#include <tuple>
#include <variant>

#include <gtest/gtest.h>

auto make_parser() {
    using TokenizersTup = std::tuple<ArgParser::identifier_tokenizer, ArgParser::string_tokenizer>;
    using ArgDefinitionsTup = std::tuple<>;
    using ExtraValueParsersTup = std::tuple<>;

    int num_tokens = 4;
    char const *tokens[4] = {
        "--ThisIsAStringIdentifier",
        "StringValueToken1",
        "-T", //tag identifier,
        "StringvalueToken2"
    };

    auto parser = ArgParser::arg_parser<TokenizersTup, ArgDefinitionsTup, ExtraValueParsersTup>(num_tokens, tokens);
    
    return parser;
}

TEST(ArgParserTests, TestSetup) {
    auto parser = make_parser();

    EXPECT_FALSE(parser.m_tokens.is_empty());
    EXPECT_EQ(parser.m_tokens.size(), 4);
    
    auto const first_token = std::get<ArgParser::identifier_token>(parser.m_tokens.consume());
    EXPECT_EQ(std::get<std::string_view>(first_token) , std::string_view("ThisIsAStringIdentifier"));
}

TEST(ArgParserTests, TestConsume) {
    auto parser = make_parser();    

    std::ignore = parser.m_tokens.consume();
}
