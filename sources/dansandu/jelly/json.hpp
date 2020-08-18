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

class PRALINE_EXPORT Json
{
    using held_types = dansandu::ballotin::type_traits::type_pack<std::nullptr_t, bool, int, double, std::string,
                                                                  std::vector<Json>, std::map<std::string, Json>>;

public:
    using value_type = typename held_types::as_variant_type;

    static Json deserialize(std::string_view json);

    static Json object(std::map<std::string, Json> map)
    {
        return Json{std::move(map)};
    }

    static Json list(std::vector<Json> vector)
    {
        return Json{std::move(vector)};
    }

    static Json string(std::string str)
    {
        return Json{std::move(str)};
    }

    Json() : value_{nullptr}
    {
    }

    template<typename Type, typename = std::enable_if_t<held_types::contains<std::decay_t<Type>>>>
    explicit Json(Type&& value) : value_{std::forward<Type>(value)}
    {
    }

    Json(const Json&) = default;

    Json(Json&&) noexcept = default;

    template<typename Type, typename = std::enable_if_t<held_types::contains<std::decay_t<Type>>>>
    Json& operator=(Type&& value)
    {
        value_ = std::forward<Type>(value);
        return *this;
    }

    Json& operator=(const Json&) = default;

    Json& operator=(Json&&) noexcept = default;

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

    void serialize(std::ostream& stream) const;

private:
    value_type value_;
};

PRALINE_EXPORT std::ostream& operator<<(std::ostream& stream, const Json& json);

}
