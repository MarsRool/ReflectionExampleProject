#pragma once
#include <cstdint>

enum class [[nodiscard]] StatusCode : std::uint32_t
{
	Good = 0U,
	GoodNotInitialized,
	GoodAlreadyInitialized,
	GoodAlreadyExists,
	GoodNothingTodo,
	Bad = 256U,
	InvalidArgument,
	BadPointer,
	AccessDenied,
	NotFound,
	NotValid,
	NotImplemented,
	NotSupported,
	Unexpected
};

inline const char* scToCString(StatusCode code)
{
	switch (code)
	{
	case StatusCode::Good: return "Good";
	case StatusCode::GoodNotInitialized: return "GoodNotInitialized";
	case StatusCode::GoodAlreadyInitialized: return "GoodAlreadyInitialized";
	case StatusCode::GoodAlreadyExists: return "GoodAlreadyExists";
	case StatusCode::GoodNothingTodo: return "GoodNothingTodo";
	case StatusCode::Bad: return "Bad";
	case StatusCode::InvalidArgument: return "InvalidArgument";
	case StatusCode::BadPointer: return "BadPointer";
	case StatusCode::AccessDenied: return "AccessDenied";
	case StatusCode::NotImplemented: return "NotImplemented";
	case StatusCode::NotFound: return "NotFound";
	case StatusCode::NotValid: return "NotValid";
	case StatusCode::NotSupported: return "NotSupported";
	case StatusCode::Unexpected: return "Unexpected";
	default: return "";
	}
}

template <typename Logger>
inline Logger& operator<<(Logger logger, StatusCode statusCode)
{
	return logger << scToCString(statusCode);
}

inline bool isBad(StatusCode sc)
{
	return sc >= StatusCode::Bad;
}

inline bool isGood(StatusCode sc)
{
	return sc < StatusCode::Bad;
}
