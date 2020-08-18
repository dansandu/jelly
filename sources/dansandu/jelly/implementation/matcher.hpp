#pragma once

#include "dansandu/glyph/symbol.hpp"

#include <string>
#include <string_view>
#include <type_traits>

namespace dansandu::jelly::implementation::matcher
{

class ExactMatcher
{
public:
    ExactMatcher(dansandu::glyph::symbol::Symbol symbol, std::string string)
        : symbol_{symbol}, string_{std::move(string)}
    {
    }

    std::pair<dansandu::glyph::symbol::Symbol, int> operator()(std::string_view string) const;

private:
    dansandu::glyph::symbol::Symbol symbol_;
    std::string string_;
};

class WhitespaceMatcher
{
public:
    explicit WhitespaceMatcher(dansandu::glyph::symbol::Symbol symbol) : symbol_{symbol}
    {
    }

    std::pair<dansandu::glyph::symbol::Symbol, int> operator()(std::string_view string) const;

private:
    dansandu::glyph::symbol::Symbol symbol_;
};

class NumberMatcher
{
public:
    NumberMatcher(dansandu::glyph::symbol::Symbol integer, dansandu::glyph::symbol::Symbol floatingPoint)
        : integer_{integer}, floatingPoint_{floatingPoint}
    {
    }

    std::pair<dansandu::glyph::symbol::Symbol, int> operator()(std::string_view string) const;

private:
    dansandu::glyph::symbol::Symbol integer_;
    dansandu::glyph::symbol::Symbol floatingPoint_;
};

class StringMatcher
{
public:
    explicit StringMatcher(dansandu::glyph::symbol::Symbol symbol) : symbol_{symbol}
    {
    }

    std::pair<dansandu::glyph::symbol::Symbol, int> operator()(std::string_view string) const;

private:
    dansandu::glyph::symbol::Symbol symbol_;
};

template<typename Matcher, typename... Matchers>
class FallbackMatcher;

template<typename Matcher>
class FallbackMatcher<Matcher>
{
public:
    template<typename M>
    FallbackMatcher(M&& matcher) : matcher_{std::move(matcher)}
    {
    }

    std::pair<dansandu::glyph::symbol::Symbol, int> operator()(std::string_view string) const
    {
        if (auto match = matcher_(string); match.second > 0)
        {
            return match;
        }
        else
        {
            return {dansandu::glyph::symbol::Symbol{}, 0};
        }
    }

private:
    Matcher matcher_;
};

template<typename Matcher, typename... Matchers>
class FallbackMatcher : private FallbackMatcher<Matchers...>
{
public:
    template<typename M, typename... MM>
    FallbackMatcher(M&& matcher, MM&&... matchers)
        : FallbackMatcher<Matchers...>{std::forward<MM>(matchers)...}, matcher_{std::move(matcher)}
    {
    }

    std::pair<dansandu::glyph::symbol::Symbol, int> operator()(std::string_view string) const
    {
        if (auto match = matcher_(string); match.second > 0)
        {
            return match;
        }
        else
        {
            return FallbackMatcher<Matchers...>::operator()(string);
        }
    }

private:
    Matcher matcher_;
};

template<typename... Matchers>
auto makeFallbackMatcher(Matchers&&... matchers)
{
    return FallbackMatcher<std::decay_t<Matchers>...>(std::forward<Matchers>(matchers)...);
}

}
