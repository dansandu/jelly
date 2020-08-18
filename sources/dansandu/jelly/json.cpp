#include "dansandu/jelly/json.hpp"
#include "dansandu/ballotin/exception.hpp"
#include "dansandu/ballotin/type_traits.hpp"
#include "dansandu/glyph/node.hpp"
#include "dansandu/glyph/parser.hpp"
#include "dansandu/glyph/token.hpp"
#include "dansandu/jelly/implementation/tokenizer.hpp"

#include <iterator>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

using dansandu::ballotin::type_traits::type_pack;
using dansandu::glyph::node::Node;
using dansandu::glyph::parser::Parser;
using dansandu::glyph::symbol::Symbol;
using dansandu::glyph::token::Token;
using dansandu::jelly::implementation::tokenizer::SymbolPack;
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

Json Json::deserialize(std::string_view json)
{
    static const auto parser = Parser{grammar};
    static const auto symbols =
        SymbolPack{parser.getTerminalSymbol("null"),          parser.getTerminalSymbol("true"),
                   parser.getTerminalSymbol("false"),         parser.getTerminalSymbol("integer"),
                   parser.getTerminalSymbol("floatingPoint"), parser.getTerminalSymbol("string"),
                   parser.getTerminalSymbol("objectBegin"),   parser.getTerminalSymbol("objectEnd"),
                   parser.getTerminalSymbol("arrayBegin"),    parser.getTerminalSymbol("arrayEnd"),
                   parser.getTerminalSymbol("comma"),         parser.getTerminalSymbol("colon"),
                   parser.getDiscardedSymbolPlaceholder()};

    auto stack = std::vector<Json>{};
    auto listBeginStack = std::vector<int>{};

    const auto visitor = [&](const Node& node) {
        if (node.isToken())
        {
            const auto& token = node.getToken();
            const auto symbol = token.getSymbol();
            if (symbol == symbols.null)
            {
                stack.emplace_back(nullptr);
            }
            else if (symbol == symbols.trueBoolean)
            {
                stack.emplace_back(true);
            }
            else if (symbol == symbols.falseBoolean)
            {
                stack.emplace_back(false);
            }
            else if (symbol == symbols.integer)
            {
                auto string = std::string{json.cbegin() + token.begin(), json.cbegin() + token.end()};
                stack.emplace_back(std::stoi(string));
            }
            else if (symbol == symbols.floatingPoint)
            {
                auto string = std::string{json.cbegin() + token.begin(), json.cbegin() + token.end()};
                stack.emplace_back(std::stod(string));
            }
            else if (symbol == symbols.string)
            {
                auto string = std::string{json.cbegin() + token.begin() + 1, json.cbegin() + token.end() - 1};
                stack.emplace_back(std::move(string));
            }
            else if (symbol == symbols.objectBegin)
            {
                listBeginStack.push_back(static_cast<int>(stack.size()));
            }
            else if (symbol == symbols.arrayBegin)
            {
                listBeginStack.push_back(static_cast<int>(stack.size()));
            }
        }
        else
        {
            const auto ruleIndex = node.getRuleIndex();
            if (ruleIndex == 1 || ruleIndex == 2)
            {
                const auto begin = stack.begin() + listBeginStack.back();
                const auto end = stack.end();
                listBeginStack.pop_back();

                auto map = std::map<std::string, Json>{};
                for (auto position = begin; position != end; ++position)
                {
                    auto key = position->get<std::string>();
                    if (map.find(key) == map.cend())
                    {
                        ++position;
                        map.emplace(std::move(key), std::move(*position));
                    }
                    else
                    {
                        THROW(JsonDeserializationError, "duplicate key '", key, "' found in Json object");
                    }
                }
                stack.erase(begin, end);
                stack.emplace_back(std::move(map));
            }
            else if (ruleIndex == 3 || ruleIndex == 4)
            {
                if (listBeginStack.empty())
                {
                    throw std::runtime_error{"corrupted list begin stack"};
                }

                const auto begin = stack.begin() + listBeginStack.back();
                const auto end = stack.end();
                listBeginStack.pop_back();

                auto list = std::vector<Json>{std::make_move_iterator(begin), std::make_move_iterator(end)};
                stack.erase(begin, end);
                stack.emplace_back(std::move(list));
            }
        }
    };

    const auto tokens = tokenize(json, symbols);
    parser.parse(tokens, visitor);

    if (stack.size() != 1)
    {
        THROW(std::runtime_error, "stack must only contain the final json");
    }

    return std::move(stack.back());
}

static std::ostream& print(std::ostream& stream, const Json::value_type& value)
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
        value);
    return stream;
}

std::string Json::toString() const
{
    std::ostringstream stream;
    print(stream, value_);
    return stream.str();
}

void Json::serialize(std::ostream& stream) const
{
    print(stream, value_);
}

std::ostream& operator<<(std::ostream& stream, const Json& json)
{
    json.serialize(stream);
    return stream;
}

}
