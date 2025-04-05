#pragma once

#include "AudioEngine/core.hpp"
#include "AudioEngine/buffer_reader.hpp"

#include <vector>
#include <string>
#include <ios>
#include <algorithm>
#include <type_traits>
#include <tuple>
#include <optional>
#include <variant>
#include <iostream>


#define TRIVIAL_IDEN_DEFINE(alias, type) \
constexpr char const alias[] = "DspCfgTrivial<" #type ">"

namespace AudioEngine {

    template <class CharT, size_t Alignment>
    class cfg_parser_context {
        buffer_reader<CharT, Alignment>& input;
        std::vector<size_t> stack;

    public:
        explicit cfg_parser_context(buffer_reader<CharT, Alignment>& input) : input(input), stack({input.tellg()}) {}
        cfg_parser_context(cfg_parser_context const&)  = delete;
        cfg_parser_context *operator=(cfg_parser_context const&) = delete;

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
            std::optional<T> value;

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

    TRIVIAL_IDEN_DEFINE(DSPLITERAL_CfgString, std::string);
    using dsp_cfg_string_parser_impl = dsp_cfg_trivial_parser_impl<std::string, DSPLITERAL_CfgString>;

    class integer_parser_result {
        std::variant<int64_t, uint64_t> m_value; 
        
    public:
        integer_parser_result(int64_t const& v) 
        :
            m_value(v)
        {}

        integer_parser_result(uint64_t const& v)
        :
            m_value(v)
        {}

        template <std::integral T>
        T get_as() const {
            return std::visit([](auto& v) -> T {
                using v_t = std::decay_t<decltype(v)>;
                if constexpr (std::is_signed_v<v_t>) { 
                    if constexpr (std::is_unsigned_v<T>)
                        throw cfg_parse_error("Attempt to extract signed config value as unsigned type");
                    else {
                        if (v > std::numeric_limits<T>::max()) {
                            throw cfg_parse_error("Attempt to parse signed config value into insufficient storage type");
                        }
                        else {
                            return static_cast<T>(v);
                        }
                    }
                }
                else {
                    if (v > static_cast<std::make_unsigned_t<T>>(std::numeric_limits<T>::max())) {
                        throw cfg_parse_error("Attempt to parse unsigned config value into insufficient storage type");
                    }
                    else {
                        return static_cast<T>(v);
                    }
                }
            }, m_value);
        }

        template <std::integral T>
        operator T() const {
            return get_as<T>();
        }
    };

    class dsp_cfg_integer_parser_impl {
        std::string m_identifier = "DspCfgIntegerType";
    public:
        using ValueType = integer_parser_result;

        template <class CharT, size_t Alignment>        
        std::optional<ValueType> parse(cfg_parser_context<CharT, Alignment>& ctx) {
            auto& input = ctx.reader();
            
            int64_t signed_value;
            uint64_t unsigned_value;

            if (!(input >> m_identifier))
                return std::nullopt;
            
            auto pos = input.tellg();
            if ( input >> unsigned_value ) {
                return unsigned_value;
            }
            else {
                input.clear_fail();
                input.seekg(pos);

                if (!(input >> signed_value)) {
                    return std::nullopt;
                }
                else {
                    return signed_value;
                }
            }

            //unreachable
            //throw std::runtime_error("Bad code path");
        }

        std::string identifier() { return m_identifier; }
        std::string parser_type() { return "DspCfgIntegerType"; }
    };

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
        dsp_cfg() 
        :   m_cfg_fields() 
        {
            m_cfg_fields.reserve(16);
        }

        template <class Type>
        [[nodiscard]] decltype(auto) get(std::string_view const& view) const {
            for (auto& e : m_cfg_fields) {
                if (e.first == view) {
                    if constexpr (std::is_integral_v<Type> && !std::is_same_v<Type, bool>) {
                        return std::get<integer_parser_result>(e.second).get_as<Type>();
                    }
                    else
                        return std::get<Type>(e.second);
                }
            }
            throw dsp_error(format("Failed to find field {}\n", view));
        }

        void add_field(std::string str, std::variant<typename Parsers::ValueType...> val) {
            m_cfg_fields.emplace_back(std::move(str), std::move(val));
        }

        void sort() {
            //sort the config fields alphabetically by their identifiers
            std::sort(m_cfg_fields.begin(), m_cfg_fields.end(), [](const auto& a, const auto& b) { return a.first < b.first; }); 
        }
    };

    template <class CharT, size_t Alignment, dsp_cfg_parser_impl<CharT, Alignment>... Parsers>
    class dsp_cfg_parser {
    public:
        using cfg_pair_t = std::pair<std::string, std::variant<typename Parsers::ValueType...>>;
        using dsp_cfg_t = dsp_cfg<CharT, Alignment, Parsers...>;
    private:
        std::tuple<Parsers...> parsers; //default constructs Parsers...
        std::ifstream m_file;
        dsp_cfg_t m_cfg;

        struct alignas(Alignment) aligned_block {
            char s[Alignment];
        };


        template <class Parser>
        bool try_parse_one(cfg_parser_context<CharT, Alignment>& ctx) {
            
            auto& parser = std::get<Parser>(parsers);

            ctx.push_pos(); //save state before parse attempt
            ctx.reader().clear_fail();
            

            std::optional<typename Parser::ValueType> result = parser.parse(ctx);
            if (result.has_value()) {
                std::cout << "Parsed config field " << parser.identifier() << " was " << parser.parser_type() << "\n";
                
                m_cfg.add_field(parser.identifier(), std::move(*result));
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

        dsp_cfg_parser(dsp_cfg_parser const&) = delete;
        dsp_cfg_parser* operator=(dsp_cfg_parser const&) = delete;

        dsp_cfg_parser(CharT const* buffer, size_t buff_size) {
            buffer_reader<CharT, Alignment> reader(buffer, buff_size);
            cfg_parser_context context = cfg_parser_context(reader);

            while (reader) {
                if (!try_parse_all(context)) {
                    reader.ignore(std::numeric_limits<size_t>::max(), '\n'); //Skip the current line
                }
            }

            m_cfg.sort();
        }

        dsp_cfg_parser(std::string const& path) {

            std::ifstream bfile(path, std::ios::binary | std::ios::ate);
            if (!bfile.is_open())
                throw cfg_parse_error( format("Failed to open file {}", path));
            
            size_t buffsize = static_cast<size_t>(bfile.tellg());

            //this would not be nececssary if std::aligned_alloc would work
            auto alloc = block_allocator<aligned_block, 4096>(new aligned_block[ 4096 ]);
            block_allocator<CharT, 4096> ralloc(alloc); //now we can allocate chars on a ${Alignment}byte alignment

            std::unique_ptr<CharT> file_buffer( ralloc.allocate(buffsize) );
            if (!file_buffer)
                throw Memory::memory_error(format("Failed to allocate {} bytes of {} byte aligned memory.", buffsize, Alignment));
            
            bfile.seekg(0);
            size_t unread = buffsize;
            size_t read = 0;
            constexpr size_t readmax = static_cast<size_t>(std::numeric_limits<std::streamsize>::max());
            while (unread > 0) {
                std::streamsize readsize = static_cast<std::streamsize>(std::min(unread, readmax)); //safe max read size
                auto& res = bfile.read(
                    reinterpret_cast<CharT*>(
                        reinterpret_cast<uintptr_t>(file_buffer.get()) + read
                    ),
                    readsize
                );
                std::streamsize readcount = bfile.gcount();
                if (!res || (bfile.gcount() == 0 && !bfile.eof())) 
                    throw cfg_parse_error(format("Failed to read file {}, read size {} bytes", path, readsize));
                
                #ifndef NDEBUG 
                if (readcount < 0)
                    throw std::runtime_error("WHAT!? Negative gcount()");
                #endif

                unread -= readcount;
                read += readcount;
            }

            buffer_reader<CharT, Alignment> reader(file_buffer.get(), buffsize);
            cfg_parser_context context = cfg_parser_context(reader);

            while (reader) {
                if (!try_parse_all(context)) {
                    reader.ignore(std::numeric_limits<size_t>::max(), '\n'); //Skip the current line
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

        AudioEngine::dsp_cfg_integer_parser_impl,

        AudioEngine::dsp_cfg_string_parser_impl
    >;

    using monitor_cfg_parsers = std::tuple<
        AudioEngine::dsp_cfg_monitor_input_parser_impl
    >;
    
    template <class CharT, size_t Align, typename Tuple>
    struct parser_from_tuple;

    template <class CharT, size_t Align, typename ... Types>
    struct parser_from_tuple<CharT, Align, std::tuple<Types...>> {
        using type = dsp_cfg_parser<CharT, Align, Types...>;
    };
}