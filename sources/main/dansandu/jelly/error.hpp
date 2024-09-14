#pragma once

#include <exception>
#include <string>

namespace dansandu::jelly::error
{

class JsonDeserializationError : public std::exception
{
public:
    explicit JsonDeserializationError(std::string message) : message_{std::move(message)}
    {
    }

    const char* what() const noexcept override
    {
        return message_.c_str();
    }

private:
    std::string message_;
};

}
