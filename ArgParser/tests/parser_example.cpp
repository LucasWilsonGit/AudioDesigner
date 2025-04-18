
#include "arg_parser.hpp"

using token_t = ArgParser::token_wrapper_t<ArgParser::string_token, ArgParser::int_token, ArgParser::path_token, ArgParser::identifier_token>;

int main() {
    ArgParser::token_stream<token_t> tokens(
        std::vector<token_t>{{
            ArgParser::identifier_token('c'),
            ArgParser::path_token("./conf.cfg"),
            ArgParser::identifier_token("use_cache"),
            ArgParser::identifier_token("page_size"),
            ArgParser::string_token("2mb"),
            ArgParser::identifier_token("page_count"),
            ArgParser::int_token(16)
        }}
    );

    ArgParser::argument_descriptor<ArgParser::path_parser> arg1 {
        .iden = {
            .name = "config_path",
            .tag = 'c'
        },
        .value = std::nullopt
    };

    using tokenizers_t = std::tuple<ArgParser::path_tokenizer, ArgParser::integer_tokenizer, ArgParser::identifier_tokenizer, ArgParser::string_tokenizer>;
    using arg_descriptors_t = std::tuple<decltype(arg1)>;

    auto parser = ArgParser::arg_parser<token_t, tokenizers_t, arg_descriptors_t>(std::make_tuple(arg1));

    parser.parse(tokens);

    std::cout << std::endl; //flush
}