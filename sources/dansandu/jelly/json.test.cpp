#include "catchorg/catch/catch.hpp"
#include "dansandu/jelly/error.hpp"
#include "dansandu/jelly/json.hpp"

#include <map>
#include <string>
#include <vector>

using Catch::Detail::Approx;
using dansandu::jelly::error::JsonDeserializationError;
using dansandu::jelly::json::Json;

TEST_CASE("Json")
{
    SECTION("primitives")
    {
        SECTION("null")
        {
            auto jsonAsString = "null";

            auto json = Json::deserialize(jsonAsString);

            REQUIRE(json.serialize() == jsonAsString);

            REQUIRE(json.is<Json::null_type>());
        }

        SECTION("boolean")
        {
            auto jsonAsString = "false";

            auto json = Json::deserialize(jsonAsString);

            REQUIRE(json.serialize() == jsonAsString);

            REQUIRE(json.is<bool>());

            REQUIRE(!json.get<bool>());
        }

        SECTION("integer")
        {
            auto jsonAsString = "73";

            auto json = Json::deserialize(jsonAsString);

            REQUIRE(json.serialize() == jsonAsString);

            REQUIRE(json.is<int>());

            REQUIRE(json.get<int>() == 73);
        }

        SECTION("double")
        {
            auto jsonAsString = "0.125";

            auto json = Json::deserialize(jsonAsString);

            REQUIRE(json.serialize() == jsonAsString);

            REQUIRE(json.is<double>());

            REQUIRE(json.get<double>() == Approx(0.125));
        }

        SECTION("string")
        {
            auto jsonAsString = R"("some_value")";

            auto json = Json::deserialize(jsonAsString);

            REQUIRE(json.serialize() == jsonAsString);

            REQUIRE(json.is<std::string>());

            REQUIRE(json.get<std::string>() == "some_value");
        }

        SECTION("list")
        {
            auto jsonAsString = "[79,2485,93]";

            auto json = Json::deserialize(jsonAsString);

            REQUIRE(json.serialize() == jsonAsString);

            REQUIRE(json.is<Json::list_type>());

            REQUIRE(json[0].get<int>() == 79);

            REQUIRE(json[1].get<int>() == 2485);

            REQUIRE(json[2].get<int>() == 93);
        }

        SECTION("empty list")
        {
            auto jsonAsString = "[]";

            auto json = Json::deserialize(jsonAsString);

            REQUIRE(json.get<Json::list_type>().empty());
        }

        SECTION("object")
        {
            auto jsonAsString = R"({"a":1,"b":2,"c":3})";

            auto json = Json::deserialize(jsonAsString);

            REQUIRE(json.serialize() == jsonAsString);

            REQUIRE(json.is<Json::object_type>());

            REQUIRE(json["a"].get<int>() == 1);

            REQUIRE(json["b"].get<int>() == 2);

            REQUIRE(json["c"].get<int>() == 3);
        }

        SECTION("empty object")
        {
            auto jsonAsString = "{}";

            auto json = Json::deserialize(jsonAsString);

            REQUIRE(json.get<Json::object_type>().empty());
        }
    }

    SECTION("medium json")
    {
        auto jsonAsString =
            R"({"array":[1,2,3],"boolean":true,"floatingPoint":0.25,"integer":7,"null":null,"string":"myString"})";

        auto json = Json::deserialize(jsonAsString);

        REQUIRE(json.serialize() == jsonAsString);

        REQUIRE(json.is<Json::object_type>());

        SECTION("retrieval with getters")
        {
            auto map = json.get<Json::object_type>();

            SECTION("throws on wrong value")
            {
                REQUIRE_THROWS_AS(json.get<std::string>(), std::logic_error);
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

            SECTION("floating point value")
            {
                auto value = map.at("floatingPoint");

                REQUIRE(value.is<double>());

                REQUIRE(value.get<double>() == 0.25);
            }

            SECTION("null value")
            {
                auto value = map.at("null");

                REQUIRE(value.is<Json::null_type>());

                REQUIRE(value.get<Json::null_type>() == nullptr);
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

                REQUIRE(value.is<Json::list_type>());

                auto vector = value.get<Json::list_type>();

                REQUIRE(vector.size() == 3);

                REQUIRE(vector[0].is<int>());

                REQUIRE(vector[0].get<int>() == 1);

                REQUIRE(vector[1].is<int>());

                REQUIRE(vector[1].get<int>() == 2);

                REQUIRE(vector[2].is<int>());

                REQUIRE(vector[2].get<int>() == 3);
            }
        }

        SECTION("sugar")
        {
            REQUIRE(json["string"].get<std::string>() == "myString");

            REQUIRE(json["boolean"].get<bool>());

            REQUIRE(json["floatingPoint"].get<double>() == Approx(0.25));

            REQUIRE(json["integer"].get<int>() == 7);

            REQUIRE(json["null"].get<Json::null_type>() == nullptr);

            REQUIRE(json["array"][0].get<int>() == 1);

            REQUIRE(json["array"][1].get<int>() == 2);

            REQUIRE(json["array"][2].get<int>() == 3);

            REQUIRE_THROWS_AS(json[0], std::logic_error);

            REQUIRE_THROWS_AS(json["array"]["first"], std::logic_error);

            REQUIRE_THROWS_AS(static_cast<std::string>(json["boolean"]), std::logic_error);
        }

        SECTION("add new object member")
        {
            SECTION("initialization")
            {
                json["newNull"];

                REQUIRE(json["newNull"].get<Json::null_type>() == nullptr);
            }

            SECTION("assignment")
            {
                json["newInteger"] = 999;

                REQUIRE(json["newInteger"].get<int>() == 999);
            }
        }
    }

    SECTION("big json")
    {
        auto jsonAsString = R"([{"battery":88,"identifier":"f2c4deb09cc1558","lastCharge":1597779221,"list":[],)"
                            R"("location":[50,10],"samples":{"CO":2,"O2":19},"timestamp":1597780427},)"
                            R"({"battery":41,"identifier":"1eb6731c7132367","lastCharge":null,"location":[26,32],)"
                            R"("map":{},"samples":{"CO":18,"O2":10},"timestamp":1597780001}])";

        auto json = Json::deserialize(jsonAsString);

        SECTION("serialized")
        {
            REQUIRE(json.serialize() == jsonAsString);
        }

        SECTION("check values")
        {
            REQUIRE(json[0]["identifier"].get<std::string>() == "f2c4deb09cc1558");

            REQUIRE(json[0]["list"].get<Json::list_type>().empty());

            REQUIRE(json[0]["location"][0].get<int>() == 50);

            REQUIRE(json[0]["location"][1].get<int>() == 10);

            REQUIRE(json[0]["timestamp"].get<int>() == 1597780427);

            REQUIRE(json[0]["samples"]["CO"].get<int>() == 2);

            REQUIRE(json[0]["samples"]["O2"].get<int>() == 19);

            REQUIRE(json[0]["battery"].get<int>() == 88);

            REQUIRE(json[0]["lastCharge"].get<int>() == 1597779221);

            REQUIRE(json[1]["identifier"].get<std::string>() == "1eb6731c7132367");

            REQUIRE(json[1]["map"].get<Json::object_type>().empty());

            REQUIRE(json[1]["location"][0].get<int>() == 26);

            REQUIRE(json[1]["location"][1].get<int>() == 32);

            REQUIRE(json[1]["timestamp"].get<int>() == 1597780001);

            REQUIRE(json[1]["samples"]["CO"].get<int>() == 18);

            REQUIRE(json[1]["samples"]["O2"].get<int>() == 10);

            REQUIRE(json[1]["battery"].get<int>() == 41);

            REQUIRE(json[1]["lastCharge"].get<Json::null_type>() == nullptr);
        }

        SECTION("change values")
        {
            json[1]["location"][0] = 30.0;

            REQUIRE(json[1]["location"][0].get<double>() == Approx(30.0));

            json[1]["timestamp"].get<int>() += 20;

            REQUIRE(json[1]["timestamp"].get<int>() == 1597780021);

            json[1]["samples"]["CO"].get<int>() += 20;

            REQUIRE(json[1]["samples"]["CO"].get<int>() == 38);
        }
    }

    SECTION("bad json")
    {
        REQUIRE_THROWS_AS(Json::deserialize(R"({"badColonMember"; [1, 2, 3]})"), JsonDeserializationError);

        REQUIRE_THROWS_AS(Json::deserialize(R"({"goodString": missingQuoteString")"), JsonDeserializationError);

        REQUIRE_THROWS_AS(Json::deserialize(R"({"duplicateKey": false, "duplicateKey": "value"})"),
                          JsonDeserializationError);
    }

    SECTION("README example")
    {
        auto order = Json::deserialize(R"({
            "orderId": "471fc736-56e9-4a78-a256-4b6f641b7d13",
            "items": [
                {
                    "itemId": "e01d9d5d-2994-4eeb-b24d-0c80fcfe7be0",
                    "count": 5
                },
                {
                    "itemId": "52cace0f-e482-4cf0-a5e5-0d8728e3015d",
                    "count": 2
                }
            ],
            "hasPromotion": true,
            "vat": 0.20,
            "previousOrderId": null
        })");

        const std::string orderId = order["orderId"];
        const int firstItemCount = order["items"][0]["count"];
        const auto secondItemCount = order["items"][1]["count"].get<int>();
        const double vat = order["vat"];
        const bool hasPreviousOrder = !order["previousOrderId"].is<std::nullptr_t>();

        // const std::nullptr_t    badCast1 = order["previousOrderId"]; // won't compile
        // const int*              badCast2 = order["previousOrderId"];  // won't compile
        // const unsigned          badCast3 = order["vat"];              // won't compile
        // const int               badCast4 = order["vat"];              // compiles but throws exception
        // const Json::list_type   badCast5 = order["items"];            // won't compile
        // const Json::object_type badCast6 = order;                     // won't compile

        auto total = (1.0 + vat) * (firstItemCount * 10.99 + secondItemCount * 55.99);

        if (order["hasPromotion"])
        {
            total *= 0.90;
        }

        auto orderReceipt = Json::object();
        orderReceipt["orderId"] = order["orderId"];
        orderReceipt["total"] = total;

        REQUIRE(!hasPreviousOrder);

        REQUIRE(static_cast<std::string>(orderReceipt["orderId"]) == "471fc736-56e9-4a78-a256-4b6f641b7d13");

        REQUIRE(static_cast<double>(orderReceipt["total"]) == 180.2844);
    }
}
