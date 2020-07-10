#include "dansandu/ballotin/container.hpp"
#include "dansandu/glyph/token.hpp"
#include "dansandu/jelly/implementation/tokenizer.hpp"

using dansandu::ballotin::container::operator<<;

#include "catchorg/catch/catch.hpp"

#include <vector>

using dansandu::glyph::token::Token;
using dansandu::jelly::implementation::tokenizer::tokenize;

// clang-format off
TEST_CASE("Tokenizer") {
    [[maybe_unused]]
    constexpr auto json = "\"someString\": 12.0,\n"
                          "\"otherString\": [1,2,3],\n"
                          "\"anotherString\": {\"yetAnotherString\":false,\"lastString\":null}";

    REQUIRE(tokenize(json) == std::vector<Token>{{
        {"string", 0, 12},
        {"colon", 12, 13},
        {"floatingPoint", 14, 18},
        {"comma", 18, 19},
        {"string", 20, 33},
        {"colon", 33, 34},
        {"arrayBegin", 35, 36},
        {"integer", 36, 37},
        {"comma", 37, 38},
        {"integer", 38, 39},
        {"comma", 39, 40},
        {"integer", 40, 41},
        {"arrayEnd", 41, 42},
        {"comma", 42, 43},
        {"string", 44, 59},
        {"colon", 59, 60},
        {"objectBegin", 61, 62},
        {"string", 62, 80},
        {"colon", 80, 81},
        {"false", 81, 86},
        {"comma", 86, 87},
        {"string", 87, 99},
        {"colon", 99, 100},
        {"null", 100, 104},
        {"objectEnd", 104, 105}
    }});
}
// clang-format on
