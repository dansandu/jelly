#include "dansandu/jelly/implementation/slice.hpp"
#include "dansandu/glyph/token.hpp"

#include <cctype>
#include <optional>
#include <string_view>

using dansandu::glyph::token::Token;

namespace dansandu::jelly::implementation::slice {

static bool isSign(char c) { return (c == '-') | (c == '+'); }

static bool isDigit(char c) { return ('0' <= c) & (c <= '9'); }

static bool isExponential(char c) { return (c == 'e') | (c == 'E'); }

static bool isFraction(char c) { return c == '.'; }

std::optional<Token> sliceWhitespaceAt(std::string_view::const_iterator begin, std::string_view string) {
    auto sliceEnd = begin;
    while (sliceEnd != string.cend() && std::isspace(*sliceEnd))
        ++sliceEnd;
    if (sliceEnd != begin)
        return Token{"whitespace", static_cast<int>(begin - string.cbegin()),
                     static_cast<int>(sliceEnd - string.cbegin())};
    return {};
}

std::optional<Token> sliceNumberAt(std::string_view::const_iterator begin, std::string_view string) {
    auto sliceEnd = begin;
    auto valid = false;
    auto isInteger = true;
    if (sliceEnd != string.cend() && isSign(*sliceEnd))
        ++sliceEnd;
    if (sliceEnd != string.cend()) {
        if (*sliceEnd == '0') {
            ++sliceEnd;
            valid = true;
        } else {
            while (sliceEnd != string.cend() && isDigit(*sliceEnd)) {
                ++sliceEnd;
                valid = true;
            }
        }
    }

    if (valid && sliceEnd != string.cend() && isFraction(*sliceEnd)) {
        ++sliceEnd;
        valid = false;
        isInteger = false;
        while (sliceEnd != string.cend() && isDigit(*sliceEnd)) {
            ++sliceEnd;
            valid = true;
        }
    }

    if (valid && sliceEnd != string.cend() && isExponential(*sliceEnd)) {
        ++sliceEnd;
        valid = false;
        isInteger = false;
        if (sliceEnd != string.cend() && isSign(*sliceEnd))
            ++sliceEnd;
        while (sliceEnd != string.cend() && isDigit(*sliceEnd)) {
            ++sliceEnd;
            valid = true;
        }
    }

    if (valid)
        return Token{isInteger ? "integer" : "floatingPoint", static_cast<int>(begin - string.cbegin()),
                     static_cast<int>(sliceEnd - string.cbegin())};
    return {};
}

std::optional<Token> sliceStringAt(std::string_view::const_iterator begin, std::string_view string) {
    auto sliceEnd = begin;
    auto complete = false;
    if (sliceEnd != string.cend() && *sliceEnd == '"') {
        ++sliceEnd;
        while (sliceEnd != string.cend() && *sliceEnd != '"')
            ++sliceEnd;
        if (sliceEnd != string.cend()) {
            ++sliceEnd;
            complete = true;
        }
    }
    if (complete)
        return Token{"string", static_cast<int>(begin - string.cbegin()), static_cast<int>(sliceEnd - string.cbegin())};
    return {};
}

}
