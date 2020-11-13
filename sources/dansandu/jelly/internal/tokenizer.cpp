#include "dansandu/jelly/internal/tokenizer.hpp"
#include "dansandu/ballotin/exception.hpp"
#include "dansandu/glyph/error.hpp"
#include "dansandu/glyph/token.hpp"
#include "dansandu/jelly/internal/matcher.hpp"

using dansandu::glyph::error::TokenizationError;
using dansandu::glyph::symbol::Symbol;
using dansandu::glyph::token::Token;
using dansandu::jelly::internal::matcher::ExactMatcher;
using dansandu::jelly::internal::matcher::makeFallbackMatcher;
using dansandu::jelly::internal::matcher::NumberMatcher;
using dansandu::jelly::internal::matcher::StringMatcher;
using dansandu::jelly::internal::matcher::WhitespaceMatcher;

namespace dansandu::jelly::internal::tokenizer
{

std::vector<Token> tokenize(std::string_view string, const SymbolPack& symbols)
{
    auto tokens = std::vector<Token>{};
    auto matcher =
        makeFallbackMatcher(ExactMatcher{symbols.arrayBegin, "["}, ExactMatcher{symbols.arrayEnd, "]"},
                            ExactMatcher{symbols.objectBegin, "{"}, ExactMatcher{symbols.objectEnd, "}"},
                            ExactMatcher{symbols.comma, ","}, ExactMatcher{symbols.colon, ":"},
                            ExactMatcher{symbols.trueBoolean, "true"}, ExactMatcher{symbols.falseBoolean, "false"},
                            ExactMatcher{symbols.null, "null"}, NumberMatcher{symbols.integer, symbols.floatingPoint},
                            StringMatcher{symbols.string}, WhitespaceMatcher{symbols.whitespace});
    auto position = 0;
    while (position < static_cast<int>(string.size()))
    {
        if (auto match = matcher(string.substr(position)); match.second > 0)
        {
            tokens.push_back(Token{match.first, position, position + match.second});
            position += match.second;
        }
        else
        {
            THROW(TokenizationError, "unrecognized symbol at position ", position + 1, " in input string:\n", string,
                  "\n", std::string(position, ' '), "^");
        }
    }
    return tokens;
}

}
