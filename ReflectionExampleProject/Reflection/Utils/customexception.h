#pragma once
#include <string>
#include "statuscode.h"

namespace reflection
{

class CustomException : public std::exception
{
public:
    CustomException(StatusCode statusCode)
        : CustomException(scToCString(statusCode))
    {}
    CustomException(const std::string& message)
        : message(message)
    {}
    CustomException(const std::string&& message)
        : message(std::move(message))
    {}
	~CustomException() override = default;

    const char * what() const noexcept override
    {
        return message.c_str();
    }

private:
	std::string message;
};

} // namespace reflection
