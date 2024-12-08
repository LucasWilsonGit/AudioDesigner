#pragma once

#include "AudioEngine/core.hpp"
#include "AudioEngine/buffer_reader.hpp"

#include <vector>
#include <string>
#include <ios>
#include <algorithm>
#include <type_traits>
#include <tuple>
#include <memory>
#include <filesystem>


#define TRIVIAL_IDEN_DEFINE(alias, type) \
constexpr char const alias[] = "DspCfgTrivial<" #type ">"

namespace AudioEngine {

    template <class CharT>
    class cfg_parser_context {
        buffer_reader<CharT>& input;
        std::vector<std::streampos> stack;

    public:
        explicit cfg_parser_context(buffer_reader<CharT>& input) : input(input), stack({input.tellg()}) {}

        void push_pos() { stack.push_back(input.tellg()); }
        void pop_pos() { input.seekg(stack.back()); stack.pop_back(); }
        void success_pos() { stack.pop_back(); }
        buffer_reader<CharT>& reader() const noexcept { return input; }
    };

    template <typename T, class CharT>
    concept dsp_cfg_parser_impl = requires(T parser, cfg_parser_context<CharT>& ctx) {
        typename T::ValueType;
        std::is_trivially_constructible_v<T>;
        { parser.parse(ctx) } -> std::same_as<std::optional<typename T::ValueType>>;
        { parser.identifier() } -> std::same_as<std::string_view>;
        { parser.parser_type() } -> std::same_as<std::string_view>;
    };

    TRIVIAL_IDEN_DEFINE(DSPLITERAL_default_iden, Unknown);

    template <class T,  char const* iden = DSPLITERAL_default_iden>
    class dsp_cfg_trivial_parser_impl {
    private:
        std::string_view m_identifier = iden;
    public:
        using ValueType = T;

        template <class CharT>
        std::optional<ValueType> parse(cfg_parser_context<CharT>& ctx) {
            std::string_view identifier;
            std::optional<T> value;

            if (!(ctx.reader() >> m_identifier >> value))
                return std::nullopt;
            
            return value;
        }

        std::string_view identifier() { return m_identifier; }
        std::string_view parser_type() { return iden; }
    };

    class dsp_cfg_bool_parser_impl { 
        std::string_view m_identifier = "DspCfgBool";
    public:
        using ValueType = bool;

        template <class CharT>
        std::optional<ValueType> parse(cfg_parser_context<CharT>& ctx) {
            auto& input = ctx.reader();
            std::string_view value;

            if (!(input >> m_identifier >> value)) {
                return std::nullopt;
            }
            
            if (value == "true") 
                return true;
            else if (value == "false")
                return false;
            else
                return std::nullopt;
        }

        std::string_view identifier() { return m_identifier; }
        std::string_view parser_type() { return "DspCfgBool"; }
    };

    TRIVIAL_IDEN_DEFINE(DSPLITERAL_CfgInt64, int64_t);
    using dsp_cfg_int64_parser_impl = dsp_cfg_trivial_parser_impl<int64_t, DSPLITERAL_CfgInt64>;

    TRIVIAL_IDEN_DEFINE(DSPLITERAL_CfgString, std::string);
    using dsp_cfg_string_parser_impl = dsp_cfg_trivial_parser_impl<std::string, DSPLITERAL_CfgString>;

    struct probe_input_cfg {
        std::string in_service;
        std::string in_probe;
    };

    class dsp_cfg_monitor_input_parser_impl {
        std::string_view m_identifier = "DspCfgMonitorInput";
    public:
        using ValueType = probe_input_cfg;

        template <class CharT>
        std::optional<ValueType> parse(cfg_parser_context<CharT>& ctx) {
            auto& input = ctx.reader();
            
            std::string_view start_input_tok;
            probe_input_cfg res;

            dsp_cfg_string_parser_impl str_parser;

            if (!(input >> start_input_tok)) {
                return std::nullopt;
            }

            if (start_input_tok != "ProbeInput")
                return std::nullopt;

            if (std::optional<std::string_view> service = str_parser.parse(ctx)) {
                res.in_service = *service;
                if (str_parser.identifier() != "ProbeService") {
                    return std::nullopt;
                }
            }
            else {
                return std::nullopt;
            }

            if (std::optional<std::string_view> probe = str_parser.parse(ctx)) {
                res.in_probe = *probe; //make a string from the string_view so we can reload a diff config with the same parser later
                if (str_parser.identifier() != "ProbeName") {
                    return std::nullopt;
                }
            }
            else {
                return std::nullopt;
            }

            return res;
        }

        std::string_view identifier() { return m_identifier; }
        std::string_view parser_type() { return "DspCfgMonitorInput"; }
    };

    template <class CharT, dsp_cfg_parser_impl<CharT>... Parsers>
    class dsp_cfg {
    private:
        using cfg_pair_t = std::pair<std::string, std::variant<typename Parsers::ValueType...>>;

        std::vector<cfg_pair_t> m_cfg_fields;
    
    public:
        dsp_cfg() 
        :   m_cfg_fields() 
        {
            m_cfg_fields.reserve(16);
        }

        template <class Type>
        [[nodiscard]] Type const& get(std::string_view const& view) const {
            for (auto& e : m_cfg_fields) {
                if (e.first == view) {
                    return std::get<Type>(e.second);
                }
            }
            throw dsp_error(format("Failed to find field {}\n", view));
        }

        void add_field(std::basic_string<CharT> str, std::variant<typename Parsers::ValueType...> val) {
            m_cfg_fields.emplace_back(std::move(str), std::move(val));
        }

        void sort() {
            //sort the config fields alphabetically by their identifiers
            std::sort(m_cfg_fields.begin(), m_cfg_fields.end(), [](const auto& a, const auto& b) { return a.first < b.first; }); 
        }
    };

    template <class CharT, class Allocator = std::allocator<CharT>, dsp_cfg_parser_impl<CharT>... Parsers>
    class dsp_cfg_parser {
    public:
        using cfg_pair_t = std::pair<std::string, std::variant<typename Parsers::ValueType...>>;
        using dsp_cfg_t = dsp_cfg<CharT, Parsers...>;
        using Allocator_t = typename std::allocator_traits<Allocator>::template rebind_alloc<CharT>;
    private:
        std::tuple<Parsers...> parsers; //default constructs Parsers...
        std::ifstream m_file;
        dsp_cfg_t m_cfg;
        Allocator_t m_buffer_alloc;


        template <class Parser>
        bool try_parse_one(cfg_parser_context<CharT>& ctx) {
            
            auto& parser = std::get<Parser>(parsers);

            ctx.push_pos(); //save state before parse attempt
            ctx.reader().clear_fail();
            

            std::optional<typename Parser::ValueType> result = parser.parse(ctx);
            if (result.has_value()) {
                
                m_cfg.add_field(std::string(parser.identifier()), std::move(*result));
                ctx.success_pos();
                return true;
            }
            else {
                ctx.pop_pos();
                return false;
            }
        }

        bool try_parse_all(cfg_parser_context<CharT>& ctx) {
            return (try_parse_one<Parsers>(ctx) || ...); //OR across all parse impls to see if any succeeded, should early exit on first true
        }



    public:
        using cfg_storage_t = std::vector<cfg_pair_t>;

        dsp_cfg_parser(std::unique_ptr<CharT> buffer, size_t buff_size, Allocator alloc = Allocator{})
        :   m_buffer_alloc(alloc)
        {
            buffer_reader<CharT> reader(std::move(buffer), buff_size);
            cfg_parser_context context = cfg_parser_context(reader);

            while (reader) {
                if (!try_parse_all(context)) {
                    reader.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); //Skip the current line
                }
            }

            m_cfg.sort();
        }

        dsp_cfg_parser(std::filesystem::path const& path, Allocator alloc = Allocator{}) 
        :   m_buffer_alloc(alloc)
        {

            std::ifstream bfile(path, std::ios::binary | std::ios::ate);
            if (!bfile.is_open())
                throw cfg_parse_error( format("Failed to open file {}", path.generic_string()));
            
            size_t buffsize = static_cast<size_t>(bfile.tellg());
            std::unique_ptr<CharT> file_buffer( m_buffer_alloc.allocate(buffsize) );
            if (!file_buffer)
                throw dsp_error(format("allocate returned nullptr"));
            
            bfile.seekg(0);
            if (!bfile.read(file_buffer.get(), buffsize))
                throw cfg_parse_error( format("Failed to read file {} into buffer ({} bytes)", path.generic_string(), buffsize));

            buffer_reader<CharT> reader(std::move(file_buffer), buffsize);
            cfg_parser_context context = cfg_parser_context(reader);

            while (reader) {
                if (!try_parse_all(context)) {
                    reader.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); //Skip the current line
                }
            }

            m_cfg.sort();
        }

        dsp_cfg_t const& get_config() const noexcept {
            return m_cfg;
        }
    };

    using base_cfg_parsers = std::tuple<
        AudioEngine::dsp_cfg_bool_parser_impl,
        AudioEngine::dsp_cfg_int64_parser_impl,
        AudioEngine::dsp_cfg_string_parser_impl
    >;

    using monitor_cfg_parsers = std::tuple<
        AudioEngine::dsp_cfg_monitor_input_parser_impl
    >;

    

    template <class CharT, class Alloc, dsp_cfg_parser_impl<CharT>... ParserTypes>
    struct dsp_cfg_parser_from_parsers {};

    template <class CharT, class Alloc, dsp_cfg_parser_impl<CharT>... ParserTypes>
    struct dsp_cfg_parser_from_parsers<CharT, Alloc, std::tuple<ParserTypes...>> {
        using dsp_cfg_parser_t = dsp_cfg_parser<CharT, Alloc, ParserTypes...>;
        using dsp_cfg_t = typename dsp_cfg_parser_t::dsp_cfg_t; 
    };
}