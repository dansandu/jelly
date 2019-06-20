#pragma once

#include "dansandu/glyph/token.hpp"

#include <algorithm>
#include <optional>
#include <string_view>

namespace dansandu::jelly::implementation::slice {

std::optional<dansandu::glyph::token::Token> sliceWhitespaceAt(std::string_view::const_iterator begin,
                                                               std::string_view string);

std::optional<dansandu::glyph::token::Token> sliceNumberAt(std::string_view::const_iterator begin,
                                                           std::string_view string);

std::optional<dansandu::glyph::token::Token> sliceStringAt(std::string_view::const_iterator begin,
                                                           std::string_view string);

template<typename SliceTraits>
auto sliceAtUsing(std::string_view::const_iterator begin, std::string_view string) {
    using dansandu::glyph::token::Token;
    for (auto match : SliceTraits::matches)
        if (std::equal(begin,
                       begin + std::min(static_cast<std::string_view::size_type>(string.cend() - begin), match.size()),
                       match.cbegin(), match.cend())) {
            auto beginIndex = static_cast<int>(begin - string.cbegin());
            return std::optional<Token>{{std::string{SliceTraits::identifier.cbegin(), SliceTraits::identifier.cend()},
                                         beginIndex, beginIndex + static_cast<int>(match.size())}};
        }
    return std::optional<Token>{};
}

inline auto sliceWhitespace(std::string_view string) { return sliceWhitespaceAt(string.cbegin(), string); }

inline auto sliceNumber(std::string_view string) { return sliceNumberAt(string.cbegin(), string); }

inline auto sliceString(std::string_view string) { return sliceStringAt(string.cbegin(), string); }

template<typename SliceTraits>
auto sliceUsing(std::string_view string) {
    return sliceAtUsing<SliceTraits>(string.cbegin(), string);
}

template<std::optional<dansandu::glyph::token::Token> (*... slicers)(std::string_view::const_iterator,
                                                                     std::string_view)>
auto fallbackSliceAt([[maybe_unused]] std::string_view::const_iterator begin,
                     [[maybe_unused]] std::string_view string) {
    using dansandu::glyph::token::Token;
    auto result = std::optional<Token>{};
    [[maybe_unused]] auto unused = (... || (result = slicers(begin, string)));
    return result;
}

template<std::optional<dansandu::glyph::token::Token> (*... slicers)(std::string_view)>
auto fallbackSlice([[maybe_unused]] std::string_view string) {
    using dansandu::glyph::token::Token;
    auto result = std::optional<Token>{};
    [[maybe_unused]] auto unused = (... || (result = slicers(string)));
    return result;
}

struct BooleanSliceTraits {
    static constexpr std::string_view identifier = "boolean";
    static constexpr std::string_view matches[] = {"true", "false"};
};

struct NullSliceTraits {
    static constexpr std::string_view identifier = "null";
    static constexpr std::string_view matches[] = {"null"};
};

struct ArrayBeginSliceTraits {
    static constexpr std::string_view identifier = "arrayBegin";
    static constexpr std::string_view matches[] = {"["};
};

struct ArrayEndSliceTraits {
    static constexpr std::string_view identifier = "arrayEnd";
    static constexpr std::string_view matches[] = {"]"};
};

struct ObjectBeginSliceTraits {
    static constexpr std::string_view identifier = "objectBegin";
    static constexpr std::string_view matches[] = {"{"};
};

struct ObjectEndSliceTraits {
    static constexpr std::string_view identifier = "objectEnd";
    static constexpr std::string_view matches[] = {"}"};
};

struct ColonSliceTraits {
    static constexpr std::string_view identifier = "colon";
    static constexpr std::string_view matches[] = {":"};
};

struct CommaSliceTraits {
    static constexpr std::string_view identifier = "comma";
    static constexpr std::string_view matches[] = {","};
};

}
