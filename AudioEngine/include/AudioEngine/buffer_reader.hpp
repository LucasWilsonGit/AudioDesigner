#pragma once


#include <fstream>
#include <optional>

#include "core.hpp"
#include "block_allocator.hpp"





namespace AudioEngine {


    template <class CharT, size_t Alignment = 32>
    class buffer_reader {
    private:
        nonowning_ptr<CharT const> m_buffer;
        size_t m_size;
        size_t m_pos;
        bool m_fail;

    protected:
        [[nodiscard]] CharT const& get_char(size_t idx) const noexcept {
            return std::assume_aligned<Alignment>(m_buffer)[idx];
        }

        //Peeks the current character in the stream
        [[nodiscard]] CharT const& curr_char() const noexcept { return get_char(m_pos); }
    
    public:
        using string_view_t = std::basic_string_view<CharT>;
        explicit buffer_reader(CharT const *buffer, size_t size) :
            m_buffer(buffer), m_size(size), m_pos(0), m_fail(false)
        {}

        explicit buffer_reader(std::basic_string_view<CharT> buffer) :
            m_buffer(buffer.data()), m_size(buffer.length()), m_pos(0), m_fail(false)
        {}

        buffer_reader(buffer_reader const&) = delete;
        buffer_reader* operator=(buffer_reader const&) = delete;

        buffer_reader& operator>>(std::basic_string<CharT>& word) {
            word.clear();

            while (m_pos < m_size && (curr_char() == '\r' || std::isspace(curr_char())))
                ++m_pos;
            
            size_t word_start_pos = m_pos;
            
            while (m_pos < m_size && curr_char() != '\r' && !std::isspace(curr_char()))
                ++m_pos;
            
            
            if (m_pos < word_start_pos) [[unlikely]]
                throw std::runtime_error("internal counter overflow during file read (big file?)");

            string_view_t slice( &get_char(word_start_pos), m_pos - word_start_pos);

            word += slice;

            if (word.empty())
                m_fail = true;

            return *this;
        }

        buffer_reader& operator>>(std::optional<std::basic_string<CharT>>& oword) {
            std::basic_string<CharT> s;
            auto& res = (*this >> s);
            oword = s;

            return res;
        }

        template <std::integral T>
        buffer_reader& operator>>(T& dst) {
            while (m_pos < m_size && std::isspace(curr_char()))
                ++m_pos;
            
            size_t word_start_pos = m_pos;

            if (curr_char() == '-' && m_pos < m_size)
                ++m_pos;

            while (m_pos < m_size && std::isdigit(curr_char()) )
                ++m_pos;

            if (m_pos < word_start_pos) [[unlikely]] 
                throw std::runtime_error("internal counter overflow during file read (big file?)");
            
            string_view_t slice( &get_char(word_start_pos), m_pos - word_start_pos);
            
            static_assert(std::is_same_v<CharT, wchar_t> || std::is_same_v<CharT, char>, "Cannot parse with std::from_chars for CharT that is not char/wchar_t");
            std::from_chars_result res = std::from_chars(slice.data(), slice.data() + slice.size(), dst);
            
            if (res.ec == std::errc::invalid_argument || res.ec == std::errc::result_out_of_range)
                m_fail = true;
            
            return *this;
        }

        template <std::integral T>
        buffer_reader& operator>>(std::optional<T>& dst) {
            T v = 0;
            auto& res = (*this >> v);
            dst = v;
            return res;
        }

        [[nodiscard]] size_t tellg() const noexcept {
            return m_pos;
        }

        void seekg(size_t pos) {
            if (pos <= m_size) {
                m_pos = pos;
            } else
                throw std::out_of_range(format("Provided pos {} was outside of file size {}", pos, m_size));
        }

        void ignore(size_t count, CharT ignore) {
            while (m_pos < m_size && curr_char() != ignore && count-- > 0) 
                ++m_pos;
        }

        void clear_fail() noexcept {
            m_fail = false;
        }

        explicit operator bool() {
            return m_pos <= m_size && !m_fail;
        }
    };
    
}