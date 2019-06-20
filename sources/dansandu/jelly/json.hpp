#include "dansandu/ballotin/exception.hpp"
#include "dansandu/ballotin/type_traits.hpp"

#include <map>
#include <string_view>
#include <vector>

namespace dansandu::jelly::json {

class Json {
    using held_types = dansandu::ballotin::type_traits::type_pack<std::nullptr_t, bool, int, double, std::string,
                                                                  std::vector<Json>, std::map<std::string, Json>>;

public:
    using value_type = typename held_types::as_variant_type;

    Json() noexcept : data_{nullptr} {}

    explicit Json(std::string_view json);

    Json(const Json&) = default;
    Json(Json&&) noexcept = default;
    Json& operator=(const Json&) = default;
    Json& operator=(Json&&) noexcept = default;

    template<typename Type, typename = std::enable_if_t<held_types::contains<Type>>>
    void set(Type value) {
        data_ = std::move(value);
    }

    template<typename Type, typename = std::enable_if_t<held_types::contains<Type>>>
    const Type& get() const {
        try {
            return std::get<Type>(data_);
        } catch (const std::bad_variant_access&) {
            THROW(std::invalid_argument, "invalid type requested in json getter -- json holds a different type");
        }
    }

private:
    value_type data_;
};

}
