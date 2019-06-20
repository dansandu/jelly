#include "dansandu/jelly/json.hpp"
#include "dansandu/glyph/node.hpp"
#include "dansandu/glyph/parser.hpp"
#include "dansandu/glyph/token.hpp"
#include "dansandu/jelly/implementation/tokenizer.hpp"

#include <algorithm>

using dansandu::glyph::node::Node;
using dansandu::glyph::parser::Parser;
using dansandu::jelly::implementation::tokenizer::tokenize;

namespace dansandu::jelly::json {

// clang-format off
static constexpr auto grammar = /* 0*/"Start   -> Value                            \n"
                                /* 1*/"Object  -> objectBegin Members objectEnd    \n"
                                /* 2*/"Object  -> objectBegin objectEnd            \n"
                                /* 3*/"Array   -> arrayBegin Values arrayEnd       \n"
                                /* 4*/"Array   -> arrayBegin arrayEnd              \n"
                                /* 5*/"Members -> Members comma string colon Value \n"
                                /* 6*/"Members -> string colon Value               \n"
                                /* 8*/"Values  -> Values comma Value               \n"
                                /* 9*/"Values  -> Value                            \n"
                                /*10*/"Value   -> Object                           \n"
                                /*11*/"Value   -> Array                            \n"
                                /*12*/"Value   -> string                           \n"
                                /*13*/"Value   -> number                           \n"
                                /*14*/"Value   -> boolean                          \n"
                                /*15*/"Value   -> null                             ";
// clang-format on

static void serialize(std::string_view json, const Node& node, Json& jsonObject) {
    if (node.isToken())
        throw std::runtime_error{"ill-formed syntax tree -- node should always be a production rule"};
    auto ruleIndex = node.getRuleIndex();
    constexpr int fallThrough[] = {0, 9, 10, 11};
    if (std::find(std::begin(fallThrough), std::end(fallThrough), ruleIndex) != std::end(fallThrough)) {
        return serialize(json, node.getChild(0), jsonObject);
    } else {
        throw std::runtime_error{"unrecognized production rule index"};
    }
}

Json::Json(std::string_view json) {
    static auto parser = Parser{grammar};
    auto node = parser.parse(json, tokenize);
    auto jsonObject = Json{};
    serialize(json, node, jsonObject);
    data_ = std::move(jsonObject.data_);
}

}
