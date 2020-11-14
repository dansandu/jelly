#include "dansandu/jelly/json.hpp"
#include "dansandu/ballotin/container.hpp"
#include "dansandu/ballotin/exception.hpp"
#include "dansandu/ballotin/type_traits.hpp"
#include "dansandu/glyph/error.hpp"
#include "dansandu/glyph/node.hpp"
#include "dansandu/glyph/parser.hpp"
#include "dansandu/glyph/token.hpp"
#include "dansandu/jelly/error.hpp"
#include "dansandu/jelly/internal/tokenizer.hpp"

#include <fstream>
#include <iterator>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

using dansandu::ballotin::container::contains;
using dansandu::ballotin::container::pop;
using dansandu::ballotin::container::uniquePushBack;
using dansandu::ballotin::type_traits::type_pack;
using dansandu::glyph::error::SyntaxError;
using dansandu::glyph::error::TokenizationError;
using dansandu::glyph::node::Node;
using dansandu::glyph::parser::Parser;
using dansandu::glyph::symbol::Symbol;
using dansandu::glyph::token::Token;
using dansandu::jelly::error::JsonDeserializationError;
using dansandu::jelly::internal::tokenizer::SymbolPack;
using dansandu::jelly::internal::tokenizer::tokenize;

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
                                /*18*/"Value    -> null                            ";
// clang-format on

Json Json::deserialize(const std::string_view json)
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
                const auto string = string_type{json.cbegin() + token.begin(), json.cbegin() + token.end()};
                stack.emplace_back(std::stoi(string));
            }
            else if (symbol == symbols.floatingPoint)
            {
                const auto string = string_type{json.cbegin() + token.begin(), json.cbegin() + token.end()};
                stack.emplace_back(std::stod(string));
            }
            else if (symbol == symbols.string)
            {
                auto string = string_type{json.cbegin() + token.begin() + 1, json.cbegin() + token.end() - 1};
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

                auto map = std::map<string_type, Json>{};
                for (auto position = begin; position != end; ++position)
                {
                    auto key = position->get<string_type>();
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
                const auto begin = stack.begin() + listBeginStack.back();
                const auto end = stack.end();
                listBeginStack.pop_back();

                auto list = std::vector<Json>{std::make_move_iterator(begin), std::make_move_iterator(end)};
                stack.erase(begin, end);
                stack.emplace_back(std::move(list));
            }
        }
    };

    try
    {
        const auto tokens = tokenize(json, symbols);
        parser.parse(tokens, visitor);
    }
    catch (const SyntaxError& error)
    {
        THROW(JsonDeserializationError, error.what());
    }

    return std::move(stack.back());
}

std::string Json::serialize() const
{
    static constexpr const char* boolean[] = {"false", "true"};
    static constexpr const char* separator[] = {",", ""};

    auto stack = std::vector<const value_type*>{&value_};
    auto visited = std::vector<const value_type*>{};
    auto serializedStack = std::vector<std::string>{};
    auto serializedBeginStack = std::vector<int>{};
    while (!stack.empty())
    {
        const auto visitor = [&](auto&& value) {
            using type = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<type, bool>)
            {
                serializedStack.push_back(boolean[value]);
                stack.pop_back();
            }
            else if constexpr (type_pack<int, double>::contains<type>)
            {
                auto stream = std::stringstream{};
                stream << value;
                serializedStack.push_back(stream.str());
                stack.pop_back();
            }
            else if constexpr (std::is_same_v<type, string_type>)
            {
                serializedStack.push_back("\"" + value + "\"");
                stack.pop_back();
            }
            else if constexpr (std::is_same_v<type, null_type>)
            {
                serializedStack.push_back("null");
                stack.pop_back();
            }
            else if constexpr (std::is_same_v<type, list_type>)
            {
                if (!contains(visited, stack.back()))
                {
                    visited.push_back(stack.back());
                    serializedBeginStack.push_back(static_cast<int>(serializedStack.size()));
                    for (const auto& element : value)
                    {
                        stack.push_back(&element.value_);
                    }
                }
                else
                {
                    const auto serializedBegin = pop(serializedBeginStack);
                    const auto reversedSerializedEnd = static_cast<int>(serializedStack.size()) - serializedBegin;
                    auto first = true;
                    auto stream = std::stringstream{};
                    stream << '[';
                    for (auto position = serializedStack.crbegin();
                         position != serializedStack.crbegin() + reversedSerializedEnd; ++position)
                    {
                        stream << separator[first] << *position;
                        first = false;
                    }
                    stream << ']';
                    serializedStack.erase(serializedStack.cbegin() + serializedBegin, serializedStack.cend());
                    serializedStack.push_back(stream.str());
                    stack.pop_back();
                }
            }
            else if constexpr (std::is_same_v<type, object_type>)
            {
                if (!contains(visited, stack.back()))
                {
                    visited.push_back(stack.back());
                    serializedBeginStack.push_back(static_cast<int>(serializedStack.size()));
                    for (const auto& entry : value)
                    {
                        stack.push_back(&entry.second.value_);
                        serializedStack.push_back(entry.first);
                    }
                }
                else
                {
                    const auto serializedKeysBegin = pop(serializedBeginStack);
                    const auto serializedValuesOffset =
                        (static_cast<int>(serializedStack.size()) - serializedKeysBegin) / 2;
                    auto first = true;
                    auto stream = std::stringstream{};
                    stream << '{';
                    for (auto i = 0; i < serializedValuesOffset; ++i)
                    {
                        stream << separator[first] << "\"" << *(serializedStack.cbegin() + serializedKeysBegin + i)
                               << "\":" << *(serializedStack.cend() - i - 1);
                        first = false;
                    }
                    stream << '}';
                    serializedStack.erase(serializedStack.cbegin() + serializedKeysBegin, serializedStack.cend());
                    serializedStack.push_back(stream.str());
                    stack.pop_back();
                }
            }
        };
        std::visit(visitor, *stack.back());
    }
    return std::move(serializedStack.back());
}

std::ostream& operator<<(std::ostream& stream, const Json& json)
{
    return stream << json.serialize();
}

}
