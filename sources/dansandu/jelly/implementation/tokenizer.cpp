#include "dansandu/jelly/implementation/tokenizer.hpp"
#include "dansandu/ballotin/exception.hpp"
#include "dansandu/glyph/token.hpp"
#include "dansandu/jelly/implementation/slice.hpp"

using dansandu::glyph::token::Token;
using dansandu::glyph::token::TokenizationError;
using dansandu::jelly::implementation::slice::ArrayBeginSliceTraits;
using dansandu::jelly::implementation::slice::ArrayEndSliceTraits;
using dansandu::jelly::implementation::slice::ColonSliceTraits;
using dansandu::jelly::implementation::slice::CommaSliceTraits;
using dansandu::jelly::implementation::slice::fallbackSliceAt;
using dansandu::jelly::implementation::slice::FalseBooleanSliceTraits;
using dansandu::jelly::implementation::slice::NullSliceTraits;
using dansandu::jelly::implementation::slice::ObjectBeginSliceTraits;
using dansandu::jelly::implementation::slice::ObjectEndSliceTraits;
using dansandu::jelly::implementation::slice::sliceAtUsing;
using dansandu::jelly::implementation::slice::sliceNumberAt;
using dansandu::jelly::implementation::slice::sliceStringAt;
using dansandu::jelly::implementation::slice::sliceWhitespaceAt;
using dansandu::jelly::implementation::slice::TrueBooleanSliceTraits;

namespace dansandu::jelly::implementation::tokenizer
{

std::vector<Token> tokenize(std::string_view string)
{
    auto tokens = std::vector<Token>{};
    auto slicer = fallbackSliceAt<sliceAtUsing<ArrayBeginSliceTraits>, sliceAtUsing<ArrayEndSliceTraits>,
                                  sliceAtUsing<TrueBooleanSliceTraits>, sliceAtUsing<FalseBooleanSliceTraits>,
                                  sliceAtUsing<ColonSliceTraits>, sliceAtUsing<CommaSliceTraits>,
                                  sliceAtUsing<NullSliceTraits>, sliceAtUsing<ObjectBeginSliceTraits>,
                                  sliceAtUsing<ObjectEndSliceTraits>, sliceNumberAt, sliceStringAt, sliceWhitespaceAt>;
    auto position = string.cbegin();
    while (position != string.cend())
        if (auto slice = slicer(position, string); slice)
        {
            position = string.cbegin() + slice->end();
            if (slice->getIdentifier() != "whitespace")
                tokens.push_back(std::move(*slice));
        }
        else
        {
            auto index = position - string.cbegin();
            THROW(TokenizationError, "unrecognized symbol at position ", index, " in input string:\n", string,
                  std::string(index, ' '), "^");
        }
    return tokens;
}

}
