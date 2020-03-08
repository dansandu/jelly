#include "dansandu/jelly/json.hpp"
#include "dansandu/ballotin/exception.hpp"
#include "dansandu/ballotin/type_traits.hpp"
#include "dansandu/glyph/node.hpp"
#include "dansandu/glyph/parser.hpp"
#include "dansandu/glyph/token.hpp"
#include "dansandu/jelly/implementation/tokenizer.hpp"

#include <algorithm>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <vector>

using dansandu::ballotin::type_traits::type_pack;
using dansandu::glyph::node::Node;
using dansandu::glyph::parser::Parser;
using dansandu::jelly::implementation::tokenizer::tokenize;

namespace dansandu::jelly::json
{

// clang-format off
static constexpr auto grammar = /* 0*/"Start    -> Value                         \n"
                                /* 1*/"Object   -> objectBegin Members objectEnd \n"
                                /* 2*/"Object   -> objectBegin objectEnd         \n"
                                /* 3*/"Array    -> arrayBegin Elements arrayEnd  \n"
                                /* 4*/"Array    -> arrayBegin arrayEnd           \n"
                                /* 5*/"Members  -> Members comma Member          \n"
                                /* 6*/"Members  -> Member                        \n"
                                /* 7*/"Member   -> string colon Value            \n"
                                /* 8*/"Elements -> Elements comma Element        \n"
                                /* 9*/"Elements -> Element                       \n"
                                /*10*/"Element  -> Value                         \n"
                                /*11*/"Value    -> Object                        \n"
                                /*12*/"Value    -> Array                         \n"
                                /*13*/"Value    -> string                        \n"
                                /*14*/"Value    -> integer                       \n"
                                /*15*/"Value    -> floatingPoint                 \n"
                                /*16*/"Value    -> true                          \n"
                                /*17*/"Value    -> false                         \n"
                                /*18*/"Value    -> null                          ";
// clang-format on

static void deserializeWork(std::string_view json, const Node& node, Json& object)
{
    if (node.isToken())
        THROW(std::runtime_error, "ill-formed syntax tree -- node should always be a production rule");
    auto ruleIndex = node.getRuleIndex();
    constexpr int fallThrough[] = {0, 6, 9, 11, 12};
    if (std::find(std::begin(fallThrough), std::end(fallThrough), ruleIndex) != std::end(fallThrough))
    {
        return deserializeWork(json, node.getChild(0), object);
    }
    else if (ruleIndex == 1)
    {
        object = std::map<std::string, Json>{};
        deserializeWork(json, node.getChild(1), object);
    }
    else if (ruleIndex == 2)
    {
        object = std::map<std::string, Json>{};
    }
    else if (ruleIndex == 3)
    {
        object = std::vector<Json>();
        deserializeWork(json, node.getChild(1), object);
    }
    else if (ruleIndex == 4)
    {
        object = std::vector<Json>();
    }
    else if (ruleIndex == 5 || ruleIndex == 8)
    {
        deserializeWork(json, node.getChild(0), object);
        deserializeWork(json, node.getChild(2), object);
    }
    else if (ruleIndex == 7)
    {
        auto value = Json{};
        deserializeWork(json, node.getChild(2), value);
        const auto& token = node.getChild(0).getToken();
        auto key = std::string{json.cbegin() + token.begin() + 1, json.cbegin() + token.end() - 1};
        auto& map = object.get<std::map<std::string, Json>>();
        if (map.find(key) == map.end())
        {
            map.insert({std::move(key), std::move(value)});
        }
        else
        {
            THROW(JsonDeserializationError, "duplicate key '", key, "' found in Json object");
        }
    }
    else if (ruleIndex == 10)
    {
        auto value = Json{};
        deserializeWork(json, node.getChild(0), value);
        auto& vector = object.get<std::vector<Json>>();
        vector.push_back(std::move(value));
    }
    else if (ruleIndex == 13)
    {
        const auto& token = node.getChild(0).getToken();
        object = std::string{json.cbegin() + token.begin() + 1, json.cbegin() + token.end() - 1};
    }
    else if (ruleIndex == 14)
    {
        const auto& token = node.getChild(0).getToken();
        auto value = std::string{json.cbegin() + token.begin(), json.cbegin() + token.end()};
        object = std::stoi(value);
    }
    else if (ruleIndex == 15)
    {
        const auto& token = node.getChild(0).getToken();
        auto value = std::string{json.cbegin() + token.begin(), json.cbegin() + token.end()};
        object = std::stod(value);
    }
    else if (ruleIndex == 16)
    {
        object = true;
    }
    else if (ruleIndex == 17)
    {
        object = false;
    }
    else if (ruleIndex == 18)
    {
        object = nullptr;
    }
    else
    {
        THROW(std::runtime_error, "unrecognized production rule index");
    }
}

Json Json::deserialize(std::string_view json)
{
    static auto parser = Parser{grammar};
    auto object = Json{};
    auto node = parser.parse(json, tokenize);
    deserializeWork(json, node, object);
    return object;
}

std::string Json::toString() const
{
    std::ostringstream os;
    os << *this;
    return os.str();
}

std::ostream& operator<<(std::ostream& stream, const Json& json)
{
    static constexpr const char* boolean[] = {"false", "true"};
    std::visit(
        [&stream](auto&& value) {
            using value_type = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<value_type, std::vector<Json>>)
            {
                auto first = true;
                stream << "[";
                for (const auto& element : value)
                {
                    if (!first)
                        stream << ',';
                    stream << element;
                    first = false;
                }
                stream << "]";
            }
            else if constexpr (std::is_same_v<value_type, std::map<std::string, Json>>)
            {
                auto first = true;
                stream << "{";
                for (const auto& entry : value)
                {
                    if (!first)
                        stream << ',';
                    stream << '"' << entry.first << "\":" << entry.second;
                    first = false;
                }
                stream << "}";
            }
            else if constexpr (std::is_same_v<value_type, bool>)
            {
                stream << boolean[value];
            }
            else if constexpr (type_pack<int, double>::contains<value_type>)
            {
                stream << value;
            }
            else if constexpr (std::is_same_v<value_type, std::string>)
            {
                stream << '"' << value << '"';
            }
            else if constexpr (std::is_same_v<value_type, std::nullptr_t>)
            {
                stream << "null";
            }
        },
        json.value_);
    return stream;
}

}
