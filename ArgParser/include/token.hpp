#pragma once

#include <variant>
#include <type_traits>

namespace ArgParser {

    template <class... Ts>
    class token_wrapper_t : public std::variant<Ts...> {
        using base = std::variant<Ts...>;
    public:
        using base::base;
        using base::operator=;
    };

    template <class... Tys>
    struct _is_token_type : std::false_type {};

    template <class... Tys>
    struct _is_token_type<token_wrapper_t<Tys...>> : std::true_type {};

    template <class T>
    constexpr bool is_token_type_v = _is_token_type<T>::value;

    template <class T>
    concept TokenType = is_token_type_v<T>;

    template <class T, class U>
    struct token_type_contains : std::false_type {};

    template <class... Ts, class U>
    struct token_type_contains<token_wrapper_t<Ts...>, U> {
        static constexpr bool value = std::disjunction_v<std::is_same<Ts, U>...>;
    };

    template <class T, class U>
    constexpr bool token_type_contains_v = token_type_contains<T, U>::value;
}