#pragma once


#include <fstream>


#include "core.hpp"
#include "block_allocator.hpp"





namespace AudioEngine {


    template <class CharT, size_t Alignment = 32>
    class buffer_reader {
    private:
        std::unique_ptr<CharT> m_buffer;
        std::streamsize m_size;
        std::streamoff m_pos;
        bool m_fail;

    protected:
        [[nodiscard]] CharT& get_char(size_t idx) const noexcept {
            return std::assume_aligned<Alignment>(static_cast<CharT*>(m_buffer.get()))[idx];
        }

        [[nodiscard]] CharT& curr_char() const noexcept { return get_char(m_pos); }
    
    public:
        explicit buffer_reader(CharT *buffer, size_t size) :
            m_buffer(buffer), m_size(size), m_pos(0), m_fail(false)
        {}

        buffer_reader(std::unique_ptr<CharT> buffer, size_t size) :
            m_buffer(std::move(buffer)), m_size(size), m_pos(0), m_fail(false)
        {}

        buffer_reader& operator>>(std::basic_string<CharT>& word) {
            word.clear();

            while (m_pos < m_size && (curr_char() == '\r' || std::isspace(curr_char())))
                ++m_pos;
            
            std::streamoff word_start_pos = m_pos;
            
            while (m_pos < m_size && curr_char() != '\r' && !std::isspace(curr_char()))
                ++m_pos;
            
            
            if (m_pos < word_start_pos) [[unlikely]]
                throw std::runtime_error("internal counter overflow during file read (big file?)");

            std::string_view slice( &get_char(word_start_pos), m_pos - word_start_pos);

            word += slice;

            if (word.empty())
                m_fail = true;

            return *this;
        }

        buffer_reader& operator>>(int64_t& dst) {
            while (m_pos < m_size && !std::isdigit(curr_char()) )
                ++m_pos;

            std::streamoff word_start_pos = m_pos;

            while (m_pos < m_size && std::isdigit(curr_char()) )
                ++m_pos;

            if (m_pos < word_start_pos) [[unlikely]] //overflow
                throw std::runtime_error("internal counter overflow during file read (big file?)");
            
            std::string_view slice( &get_char(word_start_pos), m_pos - word_start_pos);
            
            std::from_chars_result res = std::from_chars(slice.data(), slice.data() + slice.size(), dst);
            if (res.ec == std::errc::invalid_argument || res.ec == std::errc::result_out_of_range)
                m_fail = true;
            
            return *this;
        }

        [[nodiscard]] std::streamoff tellg() const noexcept {
            return m_pos;
        }

        void seekg(std::streamoff pos) {
            if (pos <= m_size) {
                m_pos = pos;
            } else
                throw std::out_of_range(format("Provided pos {} was outside of file size {}", pos, m_size));
        }

        void ignore(std::streamoff count, CharT ignore) {
            while (m_pos < m_size && curr_char() != ignore && count-- > 0)
                ++m_pos;
        }

        explicit operator bool() {
            return m_pos <= m_size && !m_fail;
        }
    };
    
}