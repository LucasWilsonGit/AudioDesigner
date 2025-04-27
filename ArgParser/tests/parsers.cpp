#include "tokenizers.hpp"
#include "token_stream.hpp"
#include "parsers.hpp"


#include <tuple>
#include <variant>

#include <gtest/gtest.h>

using namespace ArgParser;

auto make_token_stream() {
    auto stream = token_stream<token_wrapper_t<identifier_token, string_token>>();
    stream.emplace_back(identifier_token("Hello"));
    stream.emplace_back(string_token("World"));
    stream.emplace_back(identifier_token("Hello2"));
    stream.emplace_back(string_token("World2"));
    
    return stream;
}

TEST(ParserTests, TestParseSimple) {
    auto stream = make_token_stream();
    using parser = sequence_parser<identifier_token, string_token>;

    auto result_tup = parser::parse(stream);
    EXPECT_TRUE((bool)result_tup);

    {
        auto [iden, value] = *result_tup;
        EXPECT_EQ(std::get<std::string_view>(iden), "Hello");
    }

    EXPECT_EQ(stream.size(), 2);
    
    result_tup = parser::parse(stream);
    EXPECT_TRUE((bool)result_tup);

    auto [iden, value] = *result_tup;
    EXPECT_EQ(std::get<std::string_view>(iden), "Hello2");
}

TEST(ParserTests, LoopParseSimple) {
    auto stream = token_stream<token_wrapper_t<identifier_token, string_token>>();
    stream.emplace_back(identifier_token("Hello1"));
    stream.emplace_back(identifier_token("Hello2"));
    stream.emplace_back(identifier_token("Hello3"));
    stream.emplace_back(string_token("Bye"));

    using parser = loop_parser<identifier_token>;
    std::optional<typename parser::return_t> result_vec = parser::parse(stream);

    EXPECT_TRUE((bool)result_vec);
    EXPECT_EQ(result_vec->size(), 3);

    EXPECT_EQ(std::get<string_token>(stream.current_token()), "Bye");
}

TEST(ParserTests, LoopParseEmpty) {
    auto stream = token_stream<token_wrapper_t<identifier_token, string_token>>();

    using parser = loop_parser<identifier_token>;
    std::optional<typename parser::return_t> result_vec = parser::parse(stream);

    EXPECT_TRUE((bool)result_vec);
    EXPECT_EQ(result_vec->size(), 0);
}

TEST(ParserTests, LoopParseMinimumPass) {
    auto stream = token_stream<token_wrapper_t<identifier_token, string_token>>();
    stream.emplace_back(identifier_token("Hello1"));
    stream.emplace_back(identifier_token("Hello2"));
    stream.emplace_back(identifier_token("Hello3"));
    stream.emplace_back(string_token("Bye"));

    using parser = loop_parser<identifier_token, 2>;
    std::optional<typename parser::return_t> result_vec = parser::parse(stream);

    EXPECT_TRUE((bool)result_vec);
    EXPECT_EQ(result_vec->size(), 3);

    EXPECT_EQ(std::get<string_token>(stream.current_token()), "Bye");
}

TEST(ParserTests, LoopParseMinimumFail) {
    auto stream = token_stream<token_wrapper_t<identifier_token, string_token>>();
    stream.emplace_back(identifier_token("Hello1"));
    stream.emplace_back(identifier_token("Hello2"));
    stream.emplace_back(identifier_token("Hello3"));
    stream.emplace_back(string_token("Bye"));

    using parser = loop_parser<identifier_token, 4>;
    std::optional<typename parser::return_t> result_vec = parser::parse(stream);

    EXPECT_FALSE((bool)result_vec);

    EXPECT_EQ(std::get<std::string_view>(std::get<identifier_token>(stream.current_token())), "Hello1");
}

TEST(ParserTests, LoopParseSequence) {
    auto stream = make_token_stream();
    using parser = loop_parser<sequence_parser<identifier_token, string_token>>;

    std::optional<typename parser::return_t> result_vec = parser::parse(stream);

    EXPECT_TRUE((bool)result_vec);
    EXPECT_EQ(result_vec->size(), 2);

    EXPECT_TRUE(stream.is_empty());
}

struct AlternativesParserSimple_Visitor {
    using parser_one_result = typename sequence_parser<identifier_token, identifier_token, identifier_token, identifier_token>::return_t;
    using parser_two_result = typename sequence_parser<loop_parser<identifier_token>, string_token>::return_t;
    
    bool operator()(parser_one_result const&) {
        return false;
    }
    bool operator()(parser_two_result const&) {
        return true;
    }
};

TEST(ParserTests, AlternativesParserSimple) {
    auto stream = token_stream<token_wrapper_t<identifier_token, string_token>>();
    stream.emplace_back(identifier_token("Hello1"));
    stream.emplace_back(identifier_token("Hello2"));
    stream.emplace_back(identifier_token("Hello3"));
    stream.emplace_back(string_token("Bye"));

    using parser_one = sequence_parser<identifier_token, identifier_token, identifier_token, identifier_token>;
    using parser_two = sequence_parser<loop_parser<identifier_token>, string_token>;

    using parser = alternatives_parser<parser_one, parser_two>;
    std::optional<typename parser::return_t> result = parser::parse(stream);

    EXPECT_TRUE(stream.is_empty());
    EXPECT_TRUE((bool)result);
    bool res = std::visit(AlternativesParserSimple_Visitor{}, *result); //check we got the right branch
    EXPECT_TRUE(res);
    
    auto real_result = std::get<typename parser_two::return_t>(*result);
    auto& [identifiers, stringval] = real_result; 
    EXPECT_EQ(identifiers.size(), 3);
    EXPECT_EQ(stringval, "Bye");
}

