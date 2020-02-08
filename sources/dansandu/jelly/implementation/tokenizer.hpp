#pragma once

#include "dansandu/glyph/token.hpp"

#include <string_view>
#include <vector>

namespace dansandu::jelly::implementation::tokenizer
{

std::vector<dansandu::glyph::token::Token> tokenize(std::string_view string);

}
