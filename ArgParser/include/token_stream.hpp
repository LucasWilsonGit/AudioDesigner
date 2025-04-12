#pragma once

#include "token.hpp"

#include <vector>
#include <stack>

namespace ArgParser {


    class token_stream_tester;

    template <TokenType T>
    class token_stream { 
    public:
        using token_t = T;

        token_stream()
        :   m_tokens(),
            m_cursor(0),
            m_stack({{0}})
        {}

        explicit token_stream(std::vector<token_t>&& tokens)
        :   m_tokens(std::move(tokens)),
            m_cursor(0),
            m_stack({{0}})
        {}

        void begin_parse() {
            m_stack.push(m_cursor);
        }

        void complete_parse() noexcept {
            m_stack.pop();
        }

        void fail_parse() noexcept {
            m_cursor = m_stack.top();
            m_stack.pop();
        }

        void reset() {
            m_cursor = 0;
            m_stack = std::stack<size_t>();
        }

        [[nodiscard]] bool is_empty() const noexcept {
            return m_tokens.empty() || m_tokens.size() <= m_cursor;
        }

        [[nodiscard]] size_t size() const noexcept {
            return m_tokens.size() - m_cursor;
        }

        [[nodiscard]] token_t const& current_token() const {
            return m_tokens.at(m_cursor);
        }

        [[nodiscard]] token_t const& consume() {
            return m_tokens.at(m_cursor++);
        }

        void push_back(token_t&& tok) {
            m_tokens.push_back(tok);
        }

        template <class... Args>
        void emplace_back(Args... c) {
            m_tokens.emplace_back(c...);
        }

        void reserve(size_t c) {
            m_tokens.reserve(c);
        }

    protected:
        friend class token_stream_tester;

        [[nodiscard]] std::stack<size_t> const& access_stack() const noexcept {
            return m_stack;
        }

    private:
        std::vector<token_t> m_tokens;
        size_t m_cursor;
        std::stack<size_t> m_stack;
    };
}