#pragma once

#include "dansandu/glyph/symbol.hpp"
#include "dansandu/glyph/token.hpp"

#include <string_view>
#include <vector>

namespace dansandu::jelly::internal::tokenizer
{

struct SymbolPack
{
    dansandu::glyph::symbol::Symbol null, trueBoolean, falseBoolean, integer, floatingPoint, string, objectBegin,
        objectEnd, arrayBegin, arrayEnd, comma, colon, whitespace;
};

std::vector<dansandu::glyph::token::Token> tokenize(std::string_view string, const SymbolPack& symbols);

}
