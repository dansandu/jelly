#include "dansandu/jelly/implementation/matcher.hpp"

#include <string>
#include <string_view>

using dansandu::glyph::symbol::Symbol;

namespace dansandu::jelly::implementation::matcher
{

static bool isSign(char c)
{
    return (c == '+') | (c == '-');
}

static bool isDigit(char c)
{
    return ('0' <= c) & (c <= '9');
}

static bool isExponential(char c)
{
    return (c == 'e') | (c == 'E');
}

static bool isFraction(char c)
{
    return c == '.';
}

std::pair<Symbol, int> ExactMatcher::operator()(std::string_view string) const
{
    if (string.substr(0, std::min(string.size(), string_.size())) == string_)
    {
        return {symbol_, static_cast<int>(string_.size())};
    }
    return {Symbol{}, 0};
}

std::pair<Symbol, int> WhitespaceMatcher::operator()(std::string_view string) const
{
    auto position = string.cbegin();
    while (position != string.cend() && std::isspace(*position))
    {
        ++position;
    }
    auto size = static_cast<int>(position - string.cbegin());
    return {size > 0 ? symbol_ : Symbol{}, size};
}

std::pair<Symbol, int> NumberMatcher::operator()(std::string_view string) const
{
    auto position = string.cbegin();
    auto valid = false;
    auto isInteger = true;
    if (position != string.cend() && isSign(*position))
    {
        ++position;
    }
    if (position != string.cend())
    {
        if (*position == '0')
        {
            ++position;
            valid = true;
        }
        else
        {
            while (position != string.cend() && isDigit(*position))
            {
                ++position;
                valid = true;
            }
        }
    }

    if (valid && position != string.cend() && isFraction(*position))
    {
        ++position;
        valid = false;
        isInteger = false;
        while (position != string.cend() && isDigit(*position))
        {
            ++position;
            valid = true;
        }
    }

    if (valid && position != string.cend() && isExponential(*position))
    {
        ++position;
        valid = false;
        isInteger = false;
        if (position != string.cend() && isSign(*position))
        {
            ++position;
        }
        while (position != string.cend() && isDigit(*position))
        {
            ++position;
            valid = true;
        }
    }

    if (valid)
    {
        return {isInteger ? integer_ : floatingPoint_, static_cast<int>(position - string.cbegin())};
    }
    return {Symbol{}, 0};
}

std::pair<Symbol, int> StringMatcher::operator()(std::string_view string) const
{
    auto position = string.cbegin();
    if (position != string.cend() && *position == '"')
    {
        ++position;
        while (position != string.cend() && *position != '"')
        {
            ++position;
        }
        if (position != string.cend())
        {
            ++position;
            return {symbol_, static_cast<int>(position - string.cbegin())};
        }
    }
    return {Symbol{}, 0};
}

}
