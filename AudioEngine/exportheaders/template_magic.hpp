#include <tuple>


// Default: false
template <class U, template <class...> class T>
struct is_specialization_of : std::false_type {};

// Specialization: if U is T<...>
template <template <class...> class T, class... Args>
struct is_specialization_of<T<Args...>, T> : std::true_type {};

static_assert(is_specialization_of<std::tuple<int, float>, std::tuple>::value);

template <class T>
concept is_tuple = requires() {
    is_specialization_of<T, std::tuple>::value;
};

template <class T, class U>
struct tuple_contains;

template <class T, class... Us>
struct tuple_contains<std::tuple<Us...>, T> 
:   std::disjunction<std::is_same<T, Us>...>
{};

template <class Tuple, class T>
constexpr bool tuple_contains_v = tuple_contains<T, Tuple>::value;




//SFINAE check if a variadic contains a type, compile-time shortcircuit out with std::disjunction

template <class T, class... Variadic>
using variadic_contains = std::disjunction<std::is_same<T, Variadic>...>;

template <class T, class... Variadic>
inline constexpr bool variadic_contains_v = variadic_contains<T, Variadic...>::value;



//SFINAE insert a type into a variadic (out as tuple) if not already contained in the variadic

template <class T, class... Variadic>
struct insert_if_not_found {
    using type = std::conditional_t<
        variadic_contains<T, Variadic...>::value,
        std::tuple<Variadic...>,
        std::tuple<Variadic..., T>
    >;
};



//SFINAE merge two variadics without any duplicates, requires that the first variadic has no duplicates

template <class... Types>
struct variadic_merge_impl;

template <class... MergedTypes>
struct variadic_merge_impl<std::tuple<MergedTypes...>> {
    using type = std::tuple<MergedTypes...>;
};

template <class... MergedTypes, class Head>
struct variadic_merge_impl<std::tuple<MergedTypes...>, Head> {
    using type = typename insert_if_not_found<Head, MergedTypes...>::type;
};

template <class... MergedTypes, class Head, class... Tails>
struct variadic_merge_impl<std::tuple<MergedTypes...>, Head, Tails...> {
    using type = variadic_merge_impl<
        typename insert_if_not_found<Head, MergedTypes...>::type,
        Tails...
    >::type;
};






//SFINAE combine two tuples without duplicate contained types (requires the first tuple contains no duplicates)

template <class... ArgTypes>
struct tuple_combine {};

template <class... ArgTypes1, class... ArgTypes2>
struct tuple_combine<std::tuple<ArgTypes1...>, std::tuple<ArgTypes2...>> {
    using type = typename variadic_merge_impl<std::tuple<ArgTypes1...>, ArgTypes2...>::type;
};

template <class TupleFirst, class TupleSecond>
using tuple_combine_t = typename tuple_combine<TupleFirst, TupleSecond>::type;




//combine N tuples
template <is_tuple... Tups>
struct tuple_combine_multi;

template <is_tuple First>
struct tuple_combine_multi<First> : std::type_identity<First> {};

template <is_tuple First, is_tuple Second, is_tuple... Rest>
struct tuple_combine_multi<First, Second, Rest...> {
    using type = typename tuple_combine_multi<tuple_combine_t<First, Second>, Rest...>::type;
};

template <is_tuple... Types>
using tuple_combine_multi_t = typename tuple_combine_multi<Types...>::type;
