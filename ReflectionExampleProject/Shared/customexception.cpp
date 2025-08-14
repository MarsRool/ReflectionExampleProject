#include "Shared/customexception.h"

CustomException::CustomException(StatusCode statusCode)
	: CustomException(scToCString(statusCode))
{}

CustomException::CustomException(const std::string& message)
	: message(message)
{}

CustomException::CustomException(const std::string&& message)
	: message(std::move(message))
{}

const char* CustomException::what() const noexcept
{
	return message.c_str();
}


