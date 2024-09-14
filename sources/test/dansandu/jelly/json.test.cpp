#include "dansandu/jelly/json.hpp"
#include "catchorg/catch/catch.hpp"
#include "dansandu/jelly/error.hpp"

using Catch::Detail::Approx;
using dansandu::jelly::error::JsonDeserializationError;
using dansandu::jelly::json::Json;

TEST_CASE("Json")
{
    SECTION("primitive")
    {
        SECTION("null")
        {
            const auto string = "null";

            const auto json = Json::deserialize(string);

            REQUIRE(json.serialize() == string);

            REQUIRE(json.is<Json::null_type>());

            REQUIRE_THROWS_AS(static_cast<Json::string_type>(json), std::logic_error);
        }

        SECTION("boolean")
        {
            const auto string = "true";

            const auto json = Json::deserialize(string);

            REQUIRE(json.serialize() == string);

            REQUIRE(json.is<bool>());

            REQUIRE(json.get<bool>());

            REQUIRE_THROWS_AS(static_cast<int>(json), std::logic_error);
        }

        SECTION("integer")
        {
            const auto string = "73";

            const auto json = Json::deserialize(string);

            REQUIRE(json.serialize() == string);

            REQUIRE(json.is<int>());

            REQUIRE(json.get<int>() == 73);

            REQUIRE_THROWS_AS(static_cast<double>(json), std::logic_error);
        }

        SECTION("double")
        {
            const auto string = "0.125";

            const auto json = Json::deserialize(string);

            REQUIRE(json.serialize() == string);

            REQUIRE(json.is<double>());

            REQUIRE(json.get<double>() == Approx(0.125));
        }

        SECTION("string")
        {
            const auto string = R"("some_value")";

            const auto json = Json::deserialize(string);

            REQUIRE(json.serialize() == string);

            REQUIRE(json.is<Json::string_type>());

            REQUIRE(json.get<Json::string_type>() == "some_value");
        }

        SECTION("list")
        {
            const auto string = R"([79,false,"str"])";

            const auto json = Json::deserialize(string);

            REQUIRE(json.serialize() == string);

            REQUIRE(json.is<Json::list_type>());

            REQUIRE(json[0].get<int>() == 79);

            REQUIRE(!json[1].get<bool>());

            REQUIRE(json[2].get<Json::string_type>() == "str");

            REQUIRE_THROWS_AS(json[3], std::logic_error);
        }

        SECTION("empty list")
        {
            const auto string = "[]";

            const auto json = Json::deserialize(string);

            REQUIRE(json.get<Json::list_type>().empty());
        }

        SECTION("object")
        {
            const auto string = R"({"a":null,"b":[-10,-0.5],"c":{"d":"value"}})";

            const auto json = Json::deserialize(string);

            REQUIRE(json.serialize() == string);

            REQUIRE(json.is<Json::object_type>());

            REQUIRE(json["a"].is<Json::null_type>());

            REQUIRE(json["b"][0].get<int>() == -10);

            REQUIRE(json["b"][1].get<double>() == -0.5);

            REQUIRE(json["c"]["d"].get<Json::string_type>() == "value");

            REQUIRE_THROWS_AS(json["e"], std::logic_error);
        }

        SECTION("empty object")
        {
            const auto string = "{}";

            const auto json = Json::deserialize(string);

            REQUIRE(json.get<Json::object_type>().empty());
        }
    }

    SECTION("deserialize then mutate")
    {
        auto json = Json::deserialize(R"({"array":[0,1],"bool":true,"real":0.25})");

        SECTION("null")
        {
            json["null"];

            REQUIRE(json["null"].get<Json::null_type>() == nullptr);
        }

        SECTION("integer")
        {
            json["int"] = 999;

            REQUIRE(json["int"].get<int>() == 999);
        }

        SECTION("string")
        {
            json["str"] = Json::string_type{"abc"};

            REQUIRE(json["str"].get<Json::string_type>() == "abc");
        }

        SECTION("array")
        {
            json["array"] = Json::deserialize("[11, 12]");

            REQUIRE(json["array"][0].get<int>() == 11);

            REQUIRE(json["array"][1].get<int>() == 12);
        }

        SECTION("boolean")
        {
            json["bool"] = false;

            REQUIRE(!json["bool"].get<bool>());
        }
    }

    SECTION("construct then mutate")
    {
        SECTION("non-empty list")
        {
            auto json = Json::list(3);
            json[0] = std::string{"myString"};
            json[2] = 100;

            REQUIRE(json.serialize() == R"(["myString",null,100])");
        }
    }

    SECTION("big json")
    {
        const auto string = R"([{"battery":88,"identifier":"f2c4deb09cc1558","lastCharge":1597779221,"list":[],)"
                            R"("location":[50,10],"samples":{"CO":2,"O2":19},"timestamp":1597780427},)"
                            R"({"battery":41,"identifier":"1eb6731c7132367","lastCharge":null,"location":[26,32],)"
                            R"("map":{},"samples":{"CO":18,"O2":10},"timestamp":1597780001}])";

        auto json = Json::deserialize(string);

        SECTION("serialized")
        {
            REQUIRE(json.serialize() == string);
        }

        SECTION("check values")
        {
            REQUIRE(json[0]["identifier"].get<Json::string_type>() == "f2c4deb09cc1558");

            REQUIRE(json[0]["list"].get<Json::list_type>().empty());

            REQUIRE(json[0]["location"][0].get<int>() == 50);

            REQUIRE(json[0]["location"][1].get<int>() == 10);

            REQUIRE(json[0]["timestamp"].get<int>() == 1597780427);

            REQUIRE(json[0]["samples"]["CO"].get<int>() == 2);

            REQUIRE(json[0]["samples"]["O2"].get<int>() == 19);

            REQUIRE(json[0]["battery"].get<int>() == 88);

            REQUIRE(json[0]["lastCharge"].get<int>() == 1597779221);

            REQUIRE(json[1]["identifier"].get<Json::string_type>() == "1eb6731c7132367");

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
        const auto order = Json::deserialize(R"({
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

        auto total = (1.0 + vat) * (firstItemCount * 10.99 + secondItemCount * 55.99);

        if (order["hasPromotion"])
        {
            total *= 0.90;
        }

        auto orderReceipt = Json::object();
        orderReceipt["orderId"] = order["orderId"];
        orderReceipt["total"] = total;

        REQUIRE(!hasPreviousOrder);

        REQUIRE(static_cast<Json::string_type>(orderReceipt["orderId"]) == "471fc736-56e9-4a78-a256-4b6f641b7d13");

        REQUIRE(static_cast<double>(orderReceipt["total"]) == 180.2844);
    }
}
