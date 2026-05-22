#pragma once

#include <cstddef>
#include <tuple>

namespace jsm {

template<std::size_t N>
struct string
{
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using value_type = char;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = pointer;
    using const_iterator = const_pointer;

    static constexpr size_type npos = static_cast<size_type>(-1);

    consteval string() = default;

    consteval string(const char (&s)[N + 1]) noexcept
    {
        for (size_type i = 0; i < N + 1; ++i) {
            storage[i] = s[i];
        }
    }

    consteval size_type size() const noexcept
    {
        return N;
    }

    consteval pointer data() noexcept
    {
        return storage;
    }

    consteval const_pointer data() const noexcept
    {
        return storage;
    }

    consteval reference operator[](size_type n) noexcept
    {
        return storage[n];
    }

    consteval const_reference operator[](size_type n) const noexcept
    {
        return storage[n];
    }

    consteval reference front() noexcept
    {
        return operator[](0);
    }

    consteval const_reference front() const noexcept
    {
        return operator[](0);
    }

    consteval reference back() noexcept
    {
        return operator[](size() - 1);
    }

    consteval const_reference back() const noexcept
    {
        return operator[](size() - 1);
    }

    consteval iterator begin() noexcept
    {
        return storage;
    }

    consteval const_iterator begin() const noexcept
    {
        return storage;
    }

    consteval const_iterator cbegin() const noexcept
    {
        return storage;
    }

    consteval iterator end() noexcept
    {
        return storage + size();
    }

    consteval const_iterator end() const noexcept
    {
        return storage + size();
    }

    consteval const_iterator cend() const noexcept
    {
        return storage + size();
    }

    template<size_t M>
    consteval string<N + M> append(string<M> s) const noexcept
    {
        string<N + M> result{};

        auto it = result.begin();

        for (auto c : *this) {
            *it = c;
            it++;
        }

        for (auto c : s) {
            *it = c;
            it++;
        }

        return result;
    }

    template<size_type M>
    consteval auto operator+(string<M> s) const noexcept
    {
        return append(s);
    }

    template<size_t M>
    consteval bool operator==(string<M> s) const noexcept
    {
        if constexpr (N != M) {
            return false;
        }

        for (size_type i = 0; i < N; ++i) {
            if (storage[i] != s[i]) {
                return false;
            }
        }

        return true;
    }

    template<size_t M>
    consteval bool operator==(const char (&s)[M]) const noexcept
    {
        if constexpr (M != (N + 1)) {
            return false;
        }

        for (size_type i = 0; i < N + 1; ++i) {
            if (storage[i] != s[i]) {
                return false;
            }
        }

        return true;
    }

    value_type storage[N + 1]{};
};

template<size_t N>
string(const char (&)[N]) -> string<N - 1>;

inline constinit string empty = "";

consteval bool operator==(string<0>, string<0>)
{
    return true;
}

template<string S>
using value_type = typename decltype(S)::value_type;

template<string S, typename F>
consteval auto map(F&& op)
{
    auto new_str = S;
    for (auto& s : new_str) {
        s = op(s);
    }
    return new_str;
}

template<string S, typename T, typename F>
consteval T fold(T&& init, F&& op)
{
    decltype(auto) result = std::forward<T>(init);
    for (auto s : S) {
        result = op(std::forward<T>(result), s);
    }
    return result;
}

template<string... S>
consteval auto concat()
{
    return (... + S);
}

template<string S, std::size_t InclusiveBegin, std::size_t ExclusiveEnd>
consteval string<ExclusiveEnd - InclusiveBegin> slice()
{
    static_assert(InclusiveBegin >= 0);
    static_assert(ExclusiveEnd <= S.size());
    static_assert(ExclusiveEnd >= InclusiveBegin);

    if constexpr (ExclusiveEnd == InclusiveBegin) {
        return string<0>{ "" };
    } else {
        char data[ExclusiveEnd - InclusiveBegin + 1] = {};
        for (size_t i = InclusiveBegin; i < ExclusiveEnd; ++i) {
            data[i - InclusiveBegin] = S[i];
        }
        return string{ data };
    }
}

template<std::size_t N>
consteval string<N>::size_type find(string<N> str, char c)
{
    for (size_t i = 0; i < N; ++i) {
        if (str[i] == c) {
            return i;
        }
    }

    return string<N>::npos;
}

template<string S, char C>
consteval auto find()
{
    return find(S, C);
}

template<string S, char C>
consteval auto count()
{
    return fold<S>(static_cast<size_t>(0), [](auto acc, char s) { return acc + (s == C); });
}

template<string S, char From, char To>
consteval auto replace()
{
    return map<S>([](char s) { return s == From ? To : s; });
}

template<string S, char C>
consteval auto split_once()
{
    constexpr auto pos = find<S, C>();

    if constexpr (pos == decltype(S)::npos) {
        return std::make_tuple(S);
    } else if constexpr (pos == S.size()) {
        return std::make_tuple(slice<0, pos>(S), string<0>{ "" });
    } else if constexpr (pos == 0) {
        return std::make_tuple(string<0>{ "" }, slice<S, 1, S.size()>());
    } else {
        return std::make_tuple(slice<S, 0, pos>(), slice<S, pos + 1, S.size()>());
    }
}

template<string S, char C>
consteval auto split()
{
    constexpr auto s = split_once<S, C>();
    if constexpr (std::tuple_size_v<decltype(s)> == 1) {
        return s;
    } else {
        constexpr auto left = std::make_tuple(std::get<0>(s));
        constexpr auto right = split<std::get<1>(s), C>();
        return std::tuple_cat(left, right);
    }
}

template<std::size_t N>
consteval string<N> reverse(string<N> s)
{
    string<N> r{};
    for (std::size_t i = 0; i < N; ++i) {
        r[i] = s[N - (i + 1)];
    }
    return r;
}

template<string S>
consteval auto reverse()
{
    return reverse(S);
}

} // namespace jsm

#ifndef JSM_STRING_DISABLE_TESTS
#define JSM_TEST(DESC, ...) static_assert((bool)[]{ __VA_ARGS__ })
#define JSM_EXPECT(...) static_assert((__VA_ARGS__))

JSM_TEST("string", {
    constexpr jsm::string example = "123456";
    JSM_EXPECT(example.size() == 6);
    JSM_EXPECT(example.front() == '1');
    JSM_EXPECT(example[0] == '1');
    JSM_EXPECT(example[1] == '2');
    JSM_EXPECT(example[2] == '3');
    JSM_EXPECT(example[3] == '4');
    JSM_EXPECT(example[4] == '5');
    JSM_EXPECT(example[5] == '6');
    JSM_EXPECT(example[6] == '\0');
    JSM_EXPECT(example.back() == '6');
});

JSM_TEST("concat", {
    constexpr jsm::string a = "123";
    constexpr jsm::string b = "456";
    constexpr jsm::string c = "789";
    constexpr jsm::string d = "0";
    JSM_EXPECT(jsm::concat<a, b>() == "123456");
    JSM_EXPECT(jsm::concat<a, b, c>() == "123456789");
    JSM_EXPECT(jsm::concat<a, b, c, d>() == "1234567890");
});

JSM_TEST("slice", {
    constexpr jsm::string ab = "123456";
    JSM_EXPECT(jsm::slice<ab, 0, 3>() == "123");
    JSM_EXPECT(jsm::slice<ab, 2, 4>() == "34");
    JSM_EXPECT(jsm::slice<ab, 3, 6>() == "456");
});

JSM_TEST("find", {
    constexpr jsm::string x = "1234:56";
    JSM_EXPECT(jsm::find<x, ':'>() == 4);
});

JSM_TEST("count", {
    constexpr jsm::string x = "1:234:5:67:9";
    JSM_EXPECT(jsm::count<x, ':'>() == 4);
});

JSM_TEST("replace", {
    constexpr jsm::string x = "123:4:5:6";
    JSM_EXPECT(jsm::replace<x, ':', '_'>() == "123_4_5_6");
});

JSM_TEST("split_once", {
    constexpr auto z1 = jsm::split_once<"123:4", ':'>();
    JSM_EXPECT(std::tuple_size_v<decltype(z1)> == 2);
    JSM_EXPECT(std::get<0>(z1) == "123");
    JSM_EXPECT(std::get<1>(z1) == "4");

    constexpr auto z2 = jsm::split_once<"123:", ':'>();
    JSM_EXPECT(std::tuple_size_v<decltype(z2)> == 2);
    JSM_EXPECT(std::get<0>(z2) == "123");
    JSM_EXPECT(std::get<1>(z2) == "");

    constexpr auto z3 = jsm::split_once<":123", ':'>();
    JSM_EXPECT(std::tuple_size_v<decltype(z3)> == 2);
    JSM_EXPECT(std::get<0>(z3) == "");
    JSM_EXPECT(std::get<1>(z3) == "123");

    constexpr auto z4 = jsm::split_once<"123", ':'>();
    JSM_EXPECT(std::tuple_size_v<decltype(z4)> == 1);
    JSM_EXPECT(std::get<0>(z4) == "123");
});

JSM_TEST("split", {
    constexpr auto z1 = jsm::split<"1:2:3:4", ':'>();
    JSM_EXPECT(std::tuple_size_v<decltype(z1)> == 4);
    JSM_EXPECT(std::get<0>(z1) == "1");
    JSM_EXPECT(std::get<1>(z1) == "2");
    JSM_EXPECT(std::get<2>(z1) == "3");
    JSM_EXPECT(std::get<3>(z1) == "4");
});

JSM_TEST("reverse", { JSM_EXPECT(jsm::reverse<"123">() == "321"); });

#undef JSM_TEST
#undef JSM_EXPECT
#endif
