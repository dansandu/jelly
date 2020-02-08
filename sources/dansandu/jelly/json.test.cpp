#include "catchorg/catch/catch.hpp"
#include "dansandu/ballotin/container.hpp"
#include "dansandu/jelly/json.hpp"

using dansandu::ballotin::container::operator<<;

#include "catchorg/catch/catch.hpp"

#include <iostream>
#include <map>
#include <string>
#include <vector>

using dansandu::jelly::json::Json;
using dansandu::jelly::json::JsonDeserializationError;

TEST_CASE("Json")
{
    constexpr auto jsonAsString = "{\"array\":[1,2,3],"
                                  "\"boolean\":true,"
                                  "\"integer\":7,"
                                  "\"null\":null,"
                                  "\"string\":\"myString\"}";

    SECTION("deserialization")
    {
        auto json = Json::deserialize(jsonAsString);

        REQUIRE(json.is<std::map<std::string, Json>>());

        auto map = json.get<std::map<std::string, Json>>();

        SECTION("throws on wrong value")
        {
            REQUIRE_THROWS_AS(json.get<std::string>(), std::invalid_argument);
        }

        SECTION("string value")
        {
            auto value = map.at("string");

            REQUIRE(value.is<std::string>());

            REQUIRE(value.get<std::string>() == "myString");
        }

        SECTION("integer value")
        {
            auto value = map.at("integer");

            REQUIRE(value.is<int>());

            REQUIRE(value.get<int>() == 7);
        }

        SECTION("null value")
        {
            auto value = map.at("null");

            REQUIRE(value.is<std::nullptr_t>());

            REQUIRE(value.get<std::nullptr_t>() == nullptr);
        }

        SECTION("boolean value")
        {
            auto value = map.at("boolean");

            REQUIRE(value.is<bool>());

            REQUIRE(value.get<bool>());
        }

        SECTION("array value")
        {
            auto value = map.at("array");

            REQUIRE(value.is<std::vector<Json>>());

            auto vector = value.get<std::vector<Json>>();

            REQUIRE(vector.size() == 3);

            REQUIRE(vector[0].is<int>());

            REQUIRE(vector[0].get<int>() == 1);

            REQUIRE(vector[1].is<int>());

            REQUIRE(vector[1].get<int>() == 2);

            REQUIRE(vector[2].is<int>());

            REQUIRE(vector[2].get<int>() == 3);
        }

        REQUIRE(json.toString() == jsonAsString);
    }

    SECTION("serialization")
    {
        auto json = Json::from<std::vector<Json>>(
            {Json::from<std::map<std::string, Json>>(
                 {{"name", Json::from<std::string>("Jon")}, {"age", Json::from<int>(24)}}),
             Json::from<std::map<std::string, Json>>(
                 {{"name", Json::from<std::string>("Bill")}, {"age", Json::from<int>(20)}})});

        REQUIRE(json.toString() == "[{\"age\":24,\"name\":\"Jon\"},{\"age\":20,\"name\":\"Bill\"}]");
    }

    SECTION("duplicate map keys")
    {
        REQUIRE_THROWS_AS(Json::deserialize("{\"key\": false,\"key\": \"value\"}"), JsonDeserializationError);
    }
}
