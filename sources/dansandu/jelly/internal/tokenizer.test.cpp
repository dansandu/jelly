#include "catchorg/catch/catch.hpp"
#include "dansandu/glyph/token.hpp"
#include "dansandu/jelly/internal/tokenizer.hpp"

#include <vector>

using dansandu::glyph::symbol::Symbol;
using dansandu::glyph::token::Token;
using dansandu::jelly::internal::tokenizer::SymbolPack;
using dansandu::jelly::internal::tokenizer::tokenize;

// clang-format off
TEST_CASE("Tokenizer")
{
    const auto json = "\"someString\": 12.0,\n"
                      "\"otherString\": [1,2,3],\n"
                      "\"anotherString\": {\"yetAnotherString\":false,\"lastString\":null}";

    const auto symbols = SymbolPack{Symbol{0}, Symbol{1}, Symbol{2}, Symbol{3}, Symbol{4}, Symbol{5}, Symbol{6},
                                    Symbol{7}, Symbol{8}, Symbol{9}, Symbol{10}, Symbol{11}, Symbol{12}};

    REQUIRE(tokenize(json, symbols) == std::vector<Token>{
        {symbols.string, 0, 12},
        {symbols.colon, 12, 13},
        {symbols.whitespace, 13, 14},
        {symbols.floatingPoint, 14, 18},
        {symbols.comma, 18, 19},
        {symbols.whitespace, 19, 20},
        {symbols.string, 20, 33},
        {symbols.colon, 33, 34},
        {symbols.whitespace, 34, 35},
        {symbols.arrayBegin, 35, 36},
        {symbols.integer, 36, 37},
        {symbols.comma, 37, 38},
        {symbols.integer, 38, 39},
        {symbols.comma, 39, 40},
        {symbols.integer, 40, 41},
        {symbols.arrayEnd, 41, 42},
        {symbols.comma, 42, 43},
        {symbols.whitespace, 43, 44},
        {symbols.string, 44, 59},
        {symbols.colon, 59, 60},
        {symbols.whitespace, 60, 61},
        {symbols.objectBegin, 61, 62},
        {symbols.string, 62, 80},
        {symbols.colon, 80, 81},
        {symbols.falseBoolean, 81, 86},
        {symbols.comma, 86, 87},
        {symbols.string, 87, 99},
        {symbols.colon, 99, 100},
        {symbols.null, 100, 104},
        {symbols.objectEnd, 104, 105}
    });
}
// clang-format on
