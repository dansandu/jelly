#include "dansandu/ballotin/container.hpp"
#include "dansandu/glyph/token.hpp"
#include "dansandu/jelly/implementation/slice.hpp"

using dansandu::ballotin::container::operator<<;

#include "catchorg/catch/catch.hpp"

using dansandu::glyph::token::Token;
using dansandu::jelly::implementation::slice::BooleanSliceTraits;
using dansandu::jelly::implementation::slice::fallbackSlice;
using dansandu::jelly::implementation::slice::sliceNumber;
using dansandu::jelly::implementation::slice::sliceString;
using dansandu::jelly::implementation::slice::sliceUsing;
using dansandu::jelly::implementation::slice::sliceWhitespace;

TEST_CASE("Slice") {
    SECTION("whitespace") {
        constexpr auto whitespace = "whitespace";

        REQUIRE(sliceWhitespace("  \n \t f()") == Token{whitespace, 0, 6});

        REQUIRE(!sliceWhitespace("9.0  \n"));
    }

    SECTION("number") {
        constexpr auto number = "number";

        REQUIRE(!sliceNumber(" 100"));

        REQUIRE(!sliceNumber("b100"));

        REQUIRE(sliceNumber("01") == Token{number, 0, 1});

        REQUIRE(sliceNumber("0") == Token{number, 0, 1});

        REQUIRE(!sliceNumber("+"));

        REQUIRE(!sliceNumber("-"));

        REQUIRE(sliceNumber("+0") == Token{number, 0, 2});

        REQUIRE(sliceNumber("-10000") == Token{number, 0, 6});

        REQUIRE(sliceNumber("100.0") == Token{number, 0, 5});

        REQUIRE(sliceNumber("23e-2") == Token{number, 0, 5});

        REQUIRE(sliceNumber("55e2") == Token{number, 0, 4});

        REQUIRE(sliceNumber("7e+3") == Token{number, 0, 4});

        REQUIRE(sliceNumber("0x20") == Token{number, 0, 1});

        REQUIRE(!sliceNumber("0."));

        REQUIRE(!sliceNumber(".0"));

        REQUIRE(!sliceNumber("1.e10"));

        REQUIRE(!sliceNumber("1.0e"));

        REQUIRE(sliceNumber("0.0") == Token{number, 0, 3});

        REQUIRE(sliceNumber("0.0001") == Token{number, 0, 6});

        REQUIRE(sliceNumber("-10.10e0") == Token{number, 0, 8});

        REQUIRE(sliceNumber("+91.10e-10") == Token{number, 0, 10});
    }

    SECTION("string") {
        constexpr auto string = "string";

        REQUIRE(sliceString("\"some string\": 20.0") == Token{string, 0, 13});

        REQUIRE(!sliceString("h3110 w0r1d  : 1.0.2"));

        REQUIRE(sliceString("\"yada yada yada\n new paragraph\" after") == Token{string, 0, 31});
    }

    SECTION("boolean") {
        constexpr auto boolean = "boolean";

        REQUIRE(sliceUsing<BooleanSliceTraits>("true  ") == Token{boolean, 0, 4});

        REQUIRE(sliceUsing<BooleanSliceTraits>("false") == Token{boolean, 0, 5});

        REQUIRE(!sliceUsing<BooleanSliceTraits>("False"));

        REQUIRE(!sliceUsing<BooleanSliceTraits>("True"));
    }

    SECTION("fallbackSlice") {
        SECTION("empty slicers") { REQUIRE(!fallbackSlice("anything")); }

        SECTION("nonempty slicers") {
            auto fallbackSlicer = fallbackSlice<sliceNumber, sliceWhitespace, sliceUsing<BooleanSliceTraits>>;

            REQUIRE(fallbackSlicer("1.0e-10") == Token{"number", 0, 7});

            REQUIRE(fallbackSlicer("0") == Token{"number", 0, 1});

            REQUIRE(fallbackSlicer("true") == Token{"boolean", 0, 4});

            REQUIRE(fallbackSlicer("  \n ") == Token{"whitespace", 0, 4});
        }
    }
}
