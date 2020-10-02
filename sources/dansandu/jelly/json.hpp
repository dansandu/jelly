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
public:
    using null_type = std::nullptr_t;
    using list_type = std::vector<Json>;
    using object_type = std::map<std::string, Json>;

private:
    using held_types =
        dansandu::ballotin::type_traits::type_pack<null_type, bool, int, double, std::string, list_type, object_type>;

    using value_type = typename held_types::as_variant_type;

public:
    static Json deserialize(const std::string_view json);

    static Json object(object_type map)
    {
        return Json{std::move(map)};
    }

    static Json list(list_type vector)
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

    const Json& operator[](const int index) const
    {
        return get<list_type>().at(index);
    }

    Json& operator[](const int index)
    {
        return get<list_type>().at(index);
    }

    const Json& operator[](const std::string& key) const
    {
        return get<object_type>().at(key);
    }

    Json& operator[](const std::string& key)
    {
        return get<object_type>().at(key);
    }

    std::string serialize() const;

private:
    value_type value_;
};

PRALINE_EXPORT std::ostream& operator<<(std::ostream& stream, const Json& json);

}
