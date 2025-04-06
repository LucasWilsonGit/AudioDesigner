#include "tokenizers.hpp"
#include "token_stream.hpp"

#include <tuple>

#include <gtest/gtest.h>

using namespace ArgParser;

using token_t = std::variant<string_token, identifier_token>;

class ArgParser::token_stream_tester {
public:
    template <class T>
    [[nodiscard]] static size_t get_stream_stack_size(ArgParser::token_stream<T> const& s) noexcept {
        return s.access_stack().size();
    }
};

template <class T>
bool operator==(token_t const& l, T const& r) noexcept {

    return std::visit([&r](const auto& lhs) -> bool {
        if constexpr (std::is_same_v<std::decay_t<decltype(lhs)>, std::decay_t<T>>) {
            return lhs == r;
        }
        else
            return false;
        
    }, l);
}

TEST(TokenStreamTestSuite, TestConstructor) {
    ArgParser::token_stream<token_t> tokens(
        std::vector<token_t>{{
            string_token("3524"),
            identifier_token('c'),
            string_token("Hello, world"),
            identifier_token("named_identifier")
        }}
    );

    EXPECT_FALSE(tokens.is_empty());
    EXPECT_EQ(tokens.size(), 4);
    EXPECT_EQ(std::get<string_token>(tokens.current_token()), string_token("3524"));
}

TEST(TokenStreamTestSuite, CurrentToken) {
    auto test_token = string_token("3524");
    ArgParser::token_stream<token_t> stream(std::vector<token_t>{
        test_token, identifier_token('b'), identifier_token('c')
    });

    EXPECT_EQ(std::get<string_token>(stream.current_token()), test_token);
    EXPECT_EQ(stream.size(), 3);
}

TEST(TokenStreamTestSuite, ConsumeToken) {
    auto test_token = string_token("3524");
    ArgParser::token_stream<token_t> stream(std::vector<token_t>{
        test_token, 
        identifier_token('b'), 
        identifier_token('c')
    });

    auto consumed = stream.consume();
    EXPECT_EQ(std::get<string_token>(consumed), test_token);
    EXPECT_EQ(stream.size(), 2);
    EXPECT_EQ(std::get<identifier_token>(stream.current_token()), identifier_token('b'));
}

TEST(TokenStreamTestSuite, BeginAndCompleteParse) {
    auto test_token = string_token("golden token");
    ArgParser::token_stream<token_t> stream(std::vector<token_t>{
        string_token("3524"),
        identifier_token('c'),
        test_token, //copy
        string_token("Hello, world"),
        identifier_token("named_identifier")
    });
    stream.begin_parse();

    std::ignore = stream.consume();
    std::ignore = stream.consume();
    
    size_t stacksize = token_stream_tester::get_stream_stack_size(stream);

    stream.complete_parse();
    // ensure stack shrunk but cursor unchanged

    EXPECT_EQ(token_stream_tester::get_stream_stack_size(stream), stacksize-1);
    EXPECT_EQ(std::get<string_token>(stream.current_token()), test_token);
}

TEST(TokenStreamTestSuite, FailParseRestoresCursor) {
    auto first_token = string_token("first token");
    auto third_token = string_token("third token");

    ArgParser::token_stream<token_t> stream(std::vector<token_t>({
        first_token, 
        string_token("second token"), 
        third_token
    }));

    stream.begin_parse();
    // advance cursor manually
    std::ignore = stream.consume();
    std::ignore = stream.consume();

    EXPECT_EQ(std::get<string_token>(stream.current_token()), third_token);

    stream.fail_parse();
    // ensure cursor reset

    EXPECT_EQ(std::get<string_token>(stream.current_token()), first_token);
    EXPECT_NE(std::get<string_token>(stream.current_token()), third_token); //ne check just to ensure token_t operator== overload isn't doing something silly like always returning true
}

TEST(TokenStreamTestSuite, ResetWorks) {
    auto first_token = string_token("Hello, world");
    ArgParser::token_stream<token_t> stream(std::vector<token_t>{
        first_token
    });
    stream.begin_parse();
    std::ignore = stream.consume();

    // modify cursor
    stream.reset();
    EXPECT_EQ(std::get<string_token>(stream.current_token()), first_token);
}

TEST(TokenStreamTestSuite, EmptyStream) {
    ArgParser::token_stream<token_t> stream(std::vector<token_t>{});
    EXPECT_TRUE(stream.is_empty());
    EXPECT_EQ(stream.size(), 0);
    EXPECT_THROW(std::ignore = stream.current_token(), std::out_of_range);
}