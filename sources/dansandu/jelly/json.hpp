#pragma once

#include "dansandu/ballotin/exception.hpp"
#include "dansandu/ballotin/type_traits.hpp"

#include <map>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace dansandu::jelly::json
{

class JsonDeserializationError : public std::runtime_error
{
public:
    using runtime_error::runtime_error;
};

class Json
{
    friend std::ostream& operator<<(std::ostream& stream, const Json& json);

    using held_types = dansandu::ballotin::type_traits::type_pack<std::nullptr_t, bool, int, double, std::string,
                                                                  std::vector<Json>, std::map<std::string, Json>>;

public:
    using value_type = typename held_types::as_variant_type;

    static Json deserialize(std::string_view json);

    template<typename Type>
    static std::enable_if_t<held_types::contains<Type>, Json> from(Type value)
    {
        auto json = Json{};
        json.value_ = std::move(value);
        return json;
    }

    template<typename Type>
    static std::enable_if_t<held_types::contains<Type>, Json> from()
    {
        auto json = Json{};
        json.set<Type>();
        return json;
    }

    Json() : value_{nullptr}
    {
    }

    Json(const Json&) = default;
    Json(Json&&) noexcept = default;
    Json& operator=(const Json&) = default;
    Json& operator=(Json&&) noexcept = default;

    template<typename Type>
    std::enable_if_t<held_types::contains<Type>> set(Type value)
    {
        value_ = std::move(value);
    }

    template<typename Type>
    std::enable_if_t<held_types::contains<Type>> set()
    {
        value_ = Type{};
    }

    template<typename Type, typename = std::enable_if_t<held_types::contains<Type>>>
    const Type& get() const
    {
        try
        {
            return std::get<Type>(value_);
        }
        catch (const std::bad_variant_access&)
        {
            THROW(std::invalid_argument, "invalid type requested in json getter -- json holds a different type");
        }
    }

    template<typename Type, typename = std::enable_if_t<held_types::contains<Type>>>
    Type& get()
    {
        try
        {
            return std::get<Type>(value_);
        }
        catch (const std::bad_variant_access&)
        {
            THROW(std::invalid_argument, "invalid type requested in json getter -- json holds a different type");
        }
    }

    template<typename Type, typename = std::enable_if_t<held_types::contains<Type>>>
    bool is() const
    {
        return std::holds_alternative<Type>(value_);
    }

    const Json& operator[](int n) const
    {
        return get<std::vector<Json>>().at(n);
    }

    Json& operator[](int n)
    {
        return get<std::vector<Json>>().at(n);
    }

    const Json& operator[](const std::string& key) const
    {
        return get<std::map<std::string, Json>>().at(key);
    }

    Json& operator[](const std::string& key)
    {
        return get<std::map<std::string, Json>>().at(key);
    }

    std::string toString() const;

private:
    value_type value_;
};

std::ostream& operator<<(std::ostream& stream, const Json& json);

}
