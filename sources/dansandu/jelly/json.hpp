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
        json.data_ = std::move(value);
        return json;
    }

    template<typename Type>
    static std::enable_if_t<held_types::contains<Type>, Json> from()
    {
        auto json = Json{};
        json.data_ = Type{};
        return json;
    }

    Json() : data_{nullptr}
    {
    }

    Json(const Json&) = default;
    Json(Json&&) noexcept = default;
    Json& operator=(const Json&) = default;
    Json& operator=(Json&&) noexcept = default;

    template<typename Type>
    std::enable_if_t<held_types::contains<Type>> set(Type value)
    {
        data_ = std::move(value);
    }

    template<typename Type>
    std::enable_if_t<held_types::contains<Type>> set()
    {
        data_ = Type{};
    }

    template<typename Type, typename = std::enable_if_t<held_types::contains<Type>>>
    const Type& get() const
    {
        try
        {
            return std::get<Type>(data_);
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
            return std::get<Type>(data_);
        }
        catch (const std::bad_variant_access&)
        {
            THROW(std::invalid_argument, "invalid type requested in json getter -- json holds a different type");
        }
    }

    template<typename Type, typename = std::enable_if_t<held_types::contains<Type>>>
    bool is() const
    {
        return std::holds_alternative<Type>(data_);
    }

    std::string toString() const;

private:
    value_type data_;
};

std::ostream& operator<<(std::ostream& stream, const Json& json);

}
