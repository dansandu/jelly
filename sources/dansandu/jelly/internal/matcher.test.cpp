#include "dansandu/jelly/internal/matcher.hpp"
#include "catchorg/catch/catch.hpp"
#include "dansandu/glyph/symbol.hpp"

using dansandu::glyph::symbol::Symbol;
using dansandu::jelly::internal::matcher::ExactMatcher;
using dansandu::jelly::internal::matcher::makeFallbackMatcher;
using dansandu::jelly::internal::matcher::NumberMatcher;
using dansandu::jelly::internal::matcher::StringMatcher;
using dansandu::jelly::internal::matcher::WhitespaceMatcher;

using Match = std::pair<Symbol, int>;

TEST_CASE("Matcher")
{
    auto noMatch = Match{Symbol{}, 0};

    SECTION("ExactMatcher")
    {
        auto symbol = Symbol{2};
        auto matcher = ExactMatcher{Symbol{2}, "str"};

        REQUIRE(matcher("string") == Match{symbol, 3});

        REQUIRE(matcher("str") == Match{symbol, 3});

        REQUIRE(matcher("st") == noMatch);

        REQUIRE(matcher("") == noMatch);
    }

    SECTION("FallbackMatcher")
    {
        auto first = Symbol{1};
        auto second = Symbol{2};
        auto third = Symbol{3};
        auto matcher =
            makeFallbackMatcher(ExactMatcher{first, "+++"}, ExactMatcher{second, "++"}, ExactMatcher{third, "+"});

        REQUIRE(matcher("") == noMatch);

        REQUIRE(matcher("+") == Match{third, 1});

        REQUIRE(matcher("++") == Match{second, 2});

        REQUIRE(matcher("+++") == Match{first, 3});

        REQUIRE(matcher("++++") == Match{first, 3});
    }

    SECTION("WhitespaceMatcher")
    {
        auto symbol = Symbol{1};
        auto matcher = WhitespaceMatcher{symbol};

        REQUIRE(matcher("  \n \t f()") == Match{symbol, 6});

        REQUIRE(matcher("") == noMatch);

        REQUIRE(matcher("9.0   ") == noMatch);
    }

    SECTION("NumberMatcher")
    {
        auto integer = Symbol{1};
        auto floatingPoint = Symbol{2};

        auto matcher = NumberMatcher{integer, floatingPoint};

        REQUIRE(matcher(" 100") == noMatch);

        REQUIRE(matcher("b100") == noMatch);

        REQUIRE(matcher("01") == Match{integer, 1});

        REQUIRE(matcher("0") == Match{integer, 1});

        REQUIRE(matcher("+") == noMatch);

        REQUIRE(matcher("-") == noMatch);

        REQUIRE(matcher("+0") == Match{integer, 2});

        REQUIRE(matcher("-10000") == Match{integer, 6});

        REQUIRE(matcher("100.0") == Match{floatingPoint, 5});

        REQUIRE(matcher("23e-2") == Match{floatingPoint, 5});

        REQUIRE(matcher("55e2") == Match{floatingPoint, 4});

        REQUIRE(matcher("7e+3") == Match{floatingPoint, 4});

        REQUIRE(matcher("0x20") == Match{integer, 1});

        REQUIRE(matcher("0.") == noMatch);

        REQUIRE(matcher(".") == noMatch);

        REQUIRE(matcher(".0") == noMatch);

        REQUIRE(matcher("+.") == noMatch);

        REQUIRE(matcher("-.") == noMatch);

        REQUIRE(matcher("+0.") == noMatch);

        REQUIRE(matcher("-0.") == noMatch);

        REQUIRE(matcher("+.0") == noMatch);

        REQUIRE(matcher("-.0") == noMatch);

        REQUIRE(matcher("1.e10") == noMatch);

        REQUIRE(matcher("1.0e") == noMatch);

        REQUIRE(matcher("0.0") == Match{floatingPoint, 3});

        REQUIRE(matcher("0.0001") == Match{floatingPoint, 6});

        REQUIRE(matcher("-10.10e0") == Match{floatingPoint, 8});

        REQUIRE(matcher("+91.10e-10") == Match{floatingPoint, 10});
    }

    SECTION("StringMatcher")
    {
        const auto symbol = Symbol{1};
        const auto matcher = StringMatcher{symbol};

        REQUIRE(matcher("\"some string\": 20.0") == Match{symbol, 13});

        REQUIRE(matcher("h3110 w0r1d  : 1.0.2") == noMatch);

        REQUIRE(matcher("hello\"world") == noMatch);

        REQUIRE(matcher("hello \"") == noMatch);

        REQUIRE(matcher("\"hello world!\"") == Match{symbol, 14});

        REQUIRE(matcher("\"yada yada yada\n new paragraph\" after") == Match{symbol, 31});
    }
}
