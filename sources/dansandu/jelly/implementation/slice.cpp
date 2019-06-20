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
    auto sufficient = false;
    if (sliceEnd != string.cend() && isSign(*sliceEnd))
        ++sliceEnd;
    if (sliceEnd != string.cend()) {
        if (*sliceEnd == '0') {
            ++sliceEnd;
            sufficient = true;
        } else {
            while (sliceEnd != string.cend() && isDigit(*sliceEnd)) {
                ++sliceEnd;
                sufficient = true;
            }
        }
    }

    if (sufficient && sliceEnd != string.cend() && isFraction(*sliceEnd)) {
        ++sliceEnd;
        sufficient = false;
        while (sliceEnd != string.cend() && isDigit(*sliceEnd)) {
            ++sliceEnd;
            sufficient = true;
        }
    }

    if (sufficient && sliceEnd != string.cend() && isExponential(*sliceEnd)) {
        ++sliceEnd;
        sufficient = false;
        if (sliceEnd != string.cend() && isSign(*sliceEnd))
            ++sliceEnd;
        while (sliceEnd != string.cend() && isDigit(*sliceEnd)) {
            ++sliceEnd;
            sufficient = true;
        }
    }

    if (sufficient)
        return Token{"number", static_cast<int>(begin - string.cbegin()), static_cast<int>(sliceEnd - string.cbegin())};
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
