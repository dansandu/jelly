#include "dansandu/ballotin/container.hpp"
#include "dansandu/glyph/token.hpp"
#include "dansandu/jelly/implementation/slice.hpp"

using dansandu::ballotin::container::operator<<;

#include "catchorg/catch/catch.hpp"

using dansandu::glyph::token::Token;
using dansandu::jelly::implementation::slice::fallbackSlice;
using dansandu::jelly::implementation::slice::FalseBooleanSliceTraits;
using dansandu::jelly::implementation::slice::sliceNumber;
using dansandu::jelly::implementation::slice::sliceString;
using dansandu::jelly::implementation::slice::sliceUsing;
using dansandu::jelly::implementation::slice::sliceWhitespace;
using dansandu::jelly::implementation::slice::TrueBooleanSliceTraits;

TEST_CASE("Slice")
{
    SECTION("whitespace")
    {
        constexpr auto whitespace = "whitespace";

        REQUIRE(sliceWhitespace("  \n \t f()") == Token{whitespace, 0, 6});

        REQUIRE(!sliceWhitespace("9.0  \n"));
    }

    SECTION("number")
    {
        constexpr auto integer = "integer";
        constexpr auto floatingPoint = "floatingPoint";

        REQUIRE(!sliceNumber(" 100"));

        REQUIRE(!sliceNumber("b100"));

        REQUIRE(sliceNumber("01") == Token{integer, 0, 1});

        REQUIRE(sliceNumber("0") == Token{integer, 0, 1});

        REQUIRE(!sliceNumber("+"));

        REQUIRE(!sliceNumber("-"));

        REQUIRE(sliceNumber("+0") == Token{integer, 0, 2});

        REQUIRE(sliceNumber("-10000") == Token{integer, 0, 6});

        REQUIRE(sliceNumber("100.0") == Token{floatingPoint, 0, 5});

        REQUIRE(sliceNumber("23e-2") == Token{floatingPoint, 0, 5});

        REQUIRE(sliceNumber("55e2") == Token{floatingPoint, 0, 4});

        REQUIRE(sliceNumber("7e+3") == Token{floatingPoint, 0, 4});

        REQUIRE(sliceNumber("0x20") == Token{integer, 0, 1});

        REQUIRE(!sliceNumber("0."));

        REQUIRE(!sliceNumber(".0"));

        REQUIRE(!sliceNumber("1.e10"));

        REQUIRE(!sliceNumber("1.0e"));

        REQUIRE(sliceNumber("0.0") == Token{floatingPoint, 0, 3});

        REQUIRE(sliceNumber("0.0001") == Token{floatingPoint, 0, 6});

        REQUIRE(sliceNumber("-10.10e0") == Token{floatingPoint, 0, 8});

        REQUIRE(sliceNumber("+91.10e-10") == Token{floatingPoint, 0, 10});
    }

    SECTION("string")
    {
        constexpr auto string = "string";

        REQUIRE(sliceString("\"some string\": 20.0") == Token{string, 0, 13});

        REQUIRE(!sliceString("h3110 w0r1d  : 1.0.2"));

        REQUIRE(sliceString("\"yada yada yada\n new paragraph\" after") == Token{string, 0, 31});
    }

    SECTION("boolean")
    {
        REQUIRE(sliceUsing<TrueBooleanSliceTraits>("true  ") == Token{"true", 0, 4});

        REQUIRE(sliceUsing<FalseBooleanSliceTraits>("false") == Token{"false", 0, 5});

        REQUIRE(!sliceUsing<FalseBooleanSliceTraits>("False"));

        REQUIRE(!sliceUsing<TrueBooleanSliceTraits>("True"));
    }

    SECTION("fallbackSlice")
    {
        SECTION("empty slicers")
        {
            REQUIRE(!fallbackSlice("anything"));
        }

        SECTION("nonempty slicers")
        {
            auto fallbackSlicer = fallbackSlice<sliceNumber, sliceWhitespace, sliceUsing<TrueBooleanSliceTraits>>;

            REQUIRE(fallbackSlicer("1.0e-10") == Token{"floatingPoint", 0, 7});

            REQUIRE(fallbackSlicer("0") == Token{"integer", 0, 1});

            REQUIRE(fallbackSlicer("true") == Token{"true", 0, 4});

            REQUIRE(fallbackSlicer("  \n ") == Token{"whitespace", 0, 4});
        }
    }
}
