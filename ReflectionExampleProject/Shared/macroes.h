#pragma once
#include <memory>
#include <QDebug>

#ifndef FORCEINLINE
#define FORCEINLINE __forceinline
#endif // #ifndef FORCEINLINE

using real = double;

#define DECL_SHARED(x) using x ## Ptr = std::shared_ptr<class x>; \
	using x ## CPtr = std::shared_ptr<const class x>;
#define DECL_UNIQUE(x) using x ## UPtr = std::unique_ptr<class x>; \
	using x ## UCPtr = std::unique_ptr<const class x>;
#define DECL_WEAK(x) using x ## WPtr = std::weak_ptr<class x>; \
	using x ## WCPtr = std::weak_ptr<const class x>;

#define DECL_SHARED_KNOWN(x) using x ## Ptr = std::shared_ptr<x>; \
	using x ## CPtr = std::shared_ptr<const x>;
#define DECL_UNIQUE_KNOWN(x) using x ## UPtr = std::unique_ptr<x>; \
	using x ## UCPtr = std::unique_ptr<const x>;
#define DECL_WEAK_KNOWN(x) using x ## WPtr = std::weak_ptr<x>; \
	using x ## WCPtr = std::weak_ptr<const x>;

#define PROTECTED(x, errMsg) \
	try \
	{ \
		x \
	} \
	catch (const std::exception& ex) \
	{ \
		qCritical() << errMsg << ex.what(); \
	} \
	catch (...) \
	{ \
		qCritical() << errMsg << "..."; \
	}

#define PROTECTED_SYNC(x, errMsg) \
	PROTECTED(std::lock_guard{ mutex }; x, errMsg)

#define TRY try {

#define CATCH(errMsg) \
	} \
	catch (const std::exception& ex) \
	{ \
		qCritical() << errMsg << ex.what(); \
	} \
	catch (...) \
	{ \
		qCritical() << errMsg << "..."; \
	}

#define CATCH_R2(errMsg, RET) \
	} \
	catch (const std::exception& ex) \
	{ \
		qCritical() << errMsg << ex.what(); \
		return RET; \
	} \
	catch (...) \
	{ \
		qCritical() << errMsg << "..."; \
		return RET; \
	}

#define CATCH_D(errMsg, failAct) \
	} \
	catch (const std::exception& ex) \
	{ \
		qCritical() << errMsg << ex.what(); \
		failAct \
	} \
	catch (...) \
	{ \
		qCritical() << errMsg << "..."; \
		failAct \
	}
