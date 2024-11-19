#pragma once

#include <array>
#include <tuple>
#include <fstream>

#include "core.hpp"
#include "address.hpp"
#include "sockapi.hpp"
#include "shm.hpp"
#include "monitoring.hpp"

#define TRIVIAL_IDEN_DEFINE(alias, type) \
constexpr char const alias[] = "DspCfgTrivial<" #type ">"

namespace AudioEngine {

    class cfg_parser_context {
        std::istream& input;
        std::vector<std::streampos> stack;

    public:
        explicit cfg_parser_context(std::istream& input) : input(input), stack({input.tellg()}) {}

        void push_pos() { stack.push_back(input.tellg()); }
        void pop_pos() { input.seekg(stack.back()); stack.pop_back(); }
        std::istream& stream() const noexcept { return input; }
    };

    template <typename T>
    concept dsp_cfg_parser_impl = requires(T parser, cfg_parser_context& ctx) {
        typename T::ValueType;
        std::is_trivially_constructible_v<T>;
        { parser.parse(ctx) } -> std::same_as<std::optional<typename T::ValueType>>;
        { parser.identifier() } -> std::same_as<std::string>;
    };

    TRIVIAL_IDEN_DEFINE(DSPLITERAL_default_iden, Unknown);

    template <class T,  char const* iden = DSPLITERAL_default_iden>
    class dsp_cfg_trivial_parser_impl {
    private:
        std::string m_identifier = iden;
    public:
        using ValueType = T;

        std::optional<ValueType> parse(cfg_parser_context& ctx) {
            std::string identifier;
            T value;

            if (!(ctx.stream() >> identifier >> value))
                return std::nullopt;
            
            return value;
        }

        static constexpr std::string identifier() { return iden; }
    };

    class dsp_cfg_bool_parser_impl { 
        std::string m_identifier = "DspCfgBool";
    public:
        using ValueType = bool;

        std::optional<ValueType> parse(cfg_parser_context& ctx) {
            auto& input = ctx.stream();
            std::string value;

            if (!(input >> m_identifier >> value))
                return std::nullopt;
            
            if (value == "true") 
                return true;
            else if (value == "false")
                return false;
            else
                return std::nullopt;
        }

        std::string identifier() { return m_identifier; }
    };

    TRIVIAL_IDEN_DEFINE(DSPLITERAL_CfgInt64, int64_t);
    using dsp_cfg_int64_parser_impl = dsp_cfg_trivial_parser_impl<int64_t, DSPLITERAL_CfgInt64>;

    TRIVIAL_IDEN_DEFINE(DSPLITERAL_CfgString, std::string);
    using dsp_cfg_string_parser_impl = dsp_cfg_trivial_parser_impl<int64_t, DSPLITERAL_CfgString>;













    template <dsp_cfg_parser_impl... Parsers>
    class dsp_cfg_parser {
    private:
        std::tuple<Parsers...> parsers;
        using cfg_pair_t = std::pair<std::string, std::variant<typename Parsers::ValueType...>>;

        std::ifstream m_file;
        std::vector<cfg_pair_t> m_cfg_fields;


        template <class Parser>
        bool try_parse_one(cfg_parser_context& ctx) {
            auto& parser = std::get<Parser>(parsers);

            ctx.push_pos(); //save state before parse attempt
            if (std::optional<typename Parser::ValueType> result = parser.parse(ctx)) {
                m_cfg_fields.emplace_back(parser.identifier(), std::move(*result));
                return true;
            }
            else {
                ctx.pop_pos();
                return false;
            }
        }

        bool try_parse_all(cfg_parser_context& ctx) {
            return (try_parse_one<Parsers>(ctx) || ...); //OR across all parse impls to see if any succeeded, should early exit on first true
        }

    public:
        using cfg_storage_t = std::vector<cfg_pair_t>;

        explicit dsp_cfg_parser(std::string const& path) {
            std::ifstream file(path);
            if (!file.is_open())
                throw cfg_parse_error( format("Failed to open file {}", path) );
            
            cfg_parser_context context = cfg_parser_context(file);

            while (file) {
                if (!try_parse_all(context)) {
                    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); //Skip the current line
                }
            }

            //sort the config fields alphabetically by their identifiers
            std::sort(m_cfg_fields.begin(), m_cfg_fields.end(), [](const auto& a, const auto& b) { return a.first < b.first; }); 
        }

        std::vector<cfg_pair_t> const& get_config_fields() const noexcept { 
            return m_cfg_fields;
        }

        
    };




















    template <class T>
    concept dsp_plugin = requires (T t) {
        {t.init()};
        {t.idle()};
    };

    template <dsp_plugin plugin>    
    class dsp {
    protected:
        friend plugin;

        struct dsp_state {
            //probe_service service;
        };

        dsp_state m_state;
    private:
        


    public:
        dsp() {

        }
        ~dsp() = default;

        
    protected:
    private:
    };
}