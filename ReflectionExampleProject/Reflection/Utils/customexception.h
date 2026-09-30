#pragma once
#include <string>
#include "statuscode.h"

class CustomException : public std::exception
{
public:
	CustomException(StatusCode statusCode);
	CustomException(const std::string& message);
	CustomException(const std::string&& message);
	~CustomException() override = default;
	const char * what() const noexcept override;
private:
	std::string message;
};
