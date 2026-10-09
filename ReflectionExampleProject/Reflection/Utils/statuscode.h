#pragma once
#include <cstdint>
#include <QDebug>

namespace reflection
{

enum class [[nodiscard]] StatusCode : std::uint32_t
{
    Good = 0U,
    GoodNotInitialized = 1U,
    GoodAlreadyInitialized = 2U,
    GoodAlreadyExists = 3U,
    GoodNothingTodo = 4U,

    Bad = 256U,
    InvalidArgument = 257U,
    BadPointer = 258U,
    AccessDenied = 259U,
    NotFound = 260U,
    NotValid = 261U,
    NotImplemented = 262U,
    NotSupported = 263U,
    Unexpected = 264U
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

inline QDebug operator<<(QDebug logger, StatusCode statusCode)
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

} // namespace reflection
