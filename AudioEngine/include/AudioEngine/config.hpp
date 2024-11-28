#pragma once

#include "AudioEngine/core.hpp"
#include "AudioEngine/buffer_reader.hpp"




#define TRIVIAL_IDEN_DEFINE(alias, type) \
constexpr char const alias[] = "DspCfgTrivial<" #type ">"

namespace AudioEngine {

    template <class CharT, size_t Alignment>
    class cfg_parser_context {
        buffer_reader<CharT, Alignment>& input;
        std::vector<std::streampos> stack;

    public:
        explicit cfg_parser_context(buffer_reader<CharT, Alignment>& input) : input(input), stack({input.tellg()}) {}

        void push_pos() { stack.push_back(input.tellg()); }
        void pop_pos() { input.seekg(stack.back()); stack.pop_back(); }
        void success_pos() { stack.pop_back(); }
        buffer_reader<CharT, Alignment>& reader() const noexcept { return input; }
    };

    template <typename T, class CharT, size_t Alignment>
    concept dsp_cfg_parser_impl = requires(T parser, cfg_parser_context<CharT, Alignment>& ctx) {
        typename T::ValueType;
        std::is_trivially_constructible_v<T>;
        { parser.parse(ctx) } -> std::same_as<std::optional<typename T::ValueType>>;
        { parser.identifier() } -> std::same_as<std::string>;
        { parser.parser_type() } -> std::same_as<std::string>;
    };

    TRIVIAL_IDEN_DEFINE(DSPLITERAL_default_iden, Unknown);

    template <class T,  char const* iden = DSPLITERAL_default_iden>
    class dsp_cfg_trivial_parser_impl {
    private:
        std::string m_identifier = iden;
    public:
        using ValueType = T;

        template <class CharT, size_t Alignment>
        std::optional<ValueType> parse(cfg_parser_context<CharT, Alignment>& ctx) {
            std::string identifier;
            T value;

            if (!(ctx.reader() >> m_identifier >> value))
                return std::nullopt;
            
            return value;
        }

        std::string identifier() { return m_identifier; }
        std::string parser_type() { return iden; }
    };

    class dsp_cfg_bool_parser_impl { 
        std::string m_identifier = "DspCfgBool";
    public:
        using ValueType = bool;

        template <class CharT, size_t Alignment>
        std::optional<ValueType> parse(cfg_parser_context<CharT, Alignment>& ctx) {
            auto& input = ctx.reader();
            std::string value;

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

        std::string identifier() { return m_identifier; }
        std::string parser_type() { return "DspCfgBool"; }
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
        std::string m_identifier = "DspCfgMonitorInput";
    public:
        using ValueType = probe_input_cfg;

        template <class CharT, size_t Alignment>
        std::optional<ValueType> parse(cfg_parser_context<CharT, Alignment>& ctx) {
            auto& input = ctx.reader();
            
            std::string start_input_tok;
            probe_input_cfg res;

            dsp_cfg_string_parser_impl str_parser;

            if (!(input >> start_input_tok)) {
                return std::nullopt;
            }

            if (start_input_tok != "ProbeInput")
                return std::nullopt;

            if (std::optional<std::string> service = str_parser.parse(ctx)) {
                res.in_service = *service;
                if (str_parser.identifier() != "ProbeService") {
                    return std::nullopt;
                }
            }
            else {
                return std::nullopt;
            }

            if (std::optional<std::string> probe = str_parser.parse(ctx)) {
                res.in_probe = *probe;
                if (str_parser.identifier() != "ProbeName") {
                    return std::nullopt;
                }
            }
            else {
                return std::nullopt;
            }

            return res;
        }

        std::string identifier() { return m_identifier; }
        std::string parser_type() { return "DspCfgMonitorInput"; }
    };

    template <class CharT, size_t Alignment, dsp_cfg_parser_impl<CharT, Alignment>... Parsers>
    class dsp_cfg {
    private:
        using cfg_pair_t = std::pair<std::string, std::variant<typename Parsers::ValueType...>>;

        std::vector<cfg_pair_t> m_cfg_fields;
    
    public:
        dsp_cfg(std::vector<cfg_pair_t>&& in_fields) 
        :   m_cfg_fields(std::move(in_fields)) 
        {}

        template <class Type>
        [[nodiscard]] Type& get(std::string const& str) const {
            for (auto& e : m_cfg_fields) {
                if (e.first == str) {
                    return std::get<Type>(e.second);
                }
            }
        }

        template <class Type>
        [[nodiscard]] Type& get(std::string_view const& view) const {
            for (auto& e : m_cfg_fields) {
                if (e.first == view) {
                    return std::get<Type>(e.second);
                }
            }
        }
    };

    template <class CharT, size_t Alignment, dsp_cfg_parser_impl<CharT, Alignment>... Parsers>
    class dsp_cfg_parser {
    private:
        std::tuple<Parsers...> parsers;
        using cfg_pair_t = std::pair<std::string, std::variant<typename Parsers::ValueType...>>;

        std::ifstream m_file;
        std::vector<cfg_pair_t> m_cfg_fields;

        struct alignas(Alignment) aligned_block {
            char s[Alignment];
        };


        template <class Parser>
        bool try_parse_one(cfg_parser_context<CharT, Alignment>& ctx) {
            
            auto& parser = std::get<Parser>(parsers);

            ctx.push_pos(); //save state before parse attempt

            std::optional<typename Parser::ValueType> result = parser.parse(ctx);
            if (result.has_value()) {
                
                m_cfg_fields.emplace_back(parser.identifier(), std::move(*result));
                ctx.success_pos();
                return true;
            }
            else {
                ctx.pop_pos();
                return false;
            }
        }

        bool try_parse_all(cfg_parser_context<CharT, Alignment>& ctx) {
            return (try_parse_one<Parsers>(ctx) || ...); //OR across all parse impls to see if any succeeded, should early exit on first true
        }

    public:
        using cfg_storage_t = std::vector<cfg_pair_t>;

        

        explicit dsp_cfg_parser(std::string const& path) {

            std::ifstream bfile(path, std::ios::binary | std::ios::ate);
            if (!bfile.is_open())
                throw cfg_parse_error( format("Failed to open file {}", path));
            
            size_t buffsize = static_cast<size_t>(bfile.tellg());

            //this would not be nececssary if std::aligned_alloc would work but GCC do not want to be standards compliant here, or something... I am not happy.
            auto alloc = block_allocator<aligned_block, 4096>(new aligned_block[ 4096 ]);
            block_allocator<char, 4096> ralloc(alloc); //now we can allocate chars on a ${Alignment}byte alignment

            std::unique_ptr<char> file_buffer( ralloc.allocate(buffsize) );
            if (!file_buffer)
                throw Memory::memory_error(format("Failed to allocate {} bytes of {} byte aligned memory.", buffsize, Alignment));
            
            bfile.seekg(0);
            if (!bfile.read(file_buffer.get(), buffsize))
                throw cfg_parse_error( format("Failed to read file {} into buffer ({} bytes)", path, buffsize));



            buffer_reader<char, Alignment> reader(std::move(file_buffer), buffsize);
            cfg_parser_context context = cfg_parser_context(reader);

            while (reader) {
                if (!try_parse_all(context)) {
                    reader.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); //Skip the current line
                }
            }

            //sort the config fields alphabetically by their identifiers
            std::sort(m_cfg_fields.begin(), m_cfg_fields.end(), [](const auto& a, const auto& b) { return a.first < b.first; }); 
        }

        std::vector<cfg_pair_t> const& get_config_fields() const noexcept { 
            return m_cfg_fields;
        }

        
    };
}