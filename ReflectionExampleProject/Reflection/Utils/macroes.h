#pragma once
#include "statuscode.h"

#define CRITICAL1(arg1) qCritical() << arg1;
#define CRITICAL2(arg1, arg2) qCritical() << arg1 << arg2;
#define CRITICAL3(arg1, arg2, arg3) qCritical() << arg1 << arg2 << arg3;
#define CHECK_(exp, errorLogger, failAct) \
    if (!(exp)) { errorLogger failAct }

#define CHECK(exp) CHECK_(exp, CRITICAL2("!(exp): ", #exp), ;)
#define CHECK_R0(exp) CHECK_(exp, CRITICAL2("!(exp): ", #exp), return;)
#define CHECK_R2(exp, ret) CHECK_(exp, CRITICAL2("!(exp): ", #exp), return ret;)
#define CHECK_D(exp, failAct) CHECK_(exp, CRITICAL2("!(exp): ", #exp), failAct)

#define CHECK_SC(exp) { reflection::StatusCode sc = (exp); CHECK_(reflection::isGood(sc), CRITICAL1(sc), ;) }
#define CHECK_SC_R(exp) { reflection::StatusCode sc = (exp); CHECK_(reflection::isGood(sc), CRITICAL1(sc), return sc;) }
#define CHECK_SC_R0(exp) { reflection::StatusCode sc = (exp); CHECK_(reflection::isGood(sc), CRITICAL1(sc), return;) }
#define CHECK_SC_R2(exp, ret) { reflection::StatusCode sc = (exp); CHECK_(reflection::isGood(sc), CRITICAL1(sc), return ret;) }
#define CHECK_SC_D(exp, failAct) { reflection::StatusCode sc = (exp); CHECK_(reflection::isGood(sc), CRITICAL1(sc), failAct) }

#define CHECK_POINTER_R(exp) CHECK_(nullptr != (exp), CRITICAL2("nullptr == ", #exp), return reflection::StatusCode::BadPointer;)
#define CHECK_POINTER_R0(exp) CHECK_(nullptr != (exp), CRITICAL2("nullptr == ", #exp), return;)
#define CHECK_POINTER_R2(exp, ret) CHECK_(nullptr != (exp), CRITICAL2("nullptr == ", #exp), return ret;)
#define CHECK_POINTER_D(exp, failAct) CHECK_(nullptr != (exp), CRITICAL2("nullptr == ", #exp), failAct)

#define CHECK_R_THROW(exp) CHECK_(exp, CRITICAL2("!(exp): ", #exp), throw reflection::CustomException(reflection::StatusCode::Bad);)
#define CHECK_R2_THROW(exp, errMsg) CHECK_(exp, CRITICAL3("!(exp): ", #exp, errMsg), throw reflection::CustomException(errMsg);)
#define CHECK_SC_THROW(exp) { reflection::StatusCode sc = (exp); CHECK_(reflection::isGood(sc), CRITICAL1(sc), throw reflection::CustomException(sc);) }
#define CHECK_POINTER_THROW(exp) CHECK_(nullptr != (exp), CRITICAL2("nullptr == ", #exp), throw reflection::CustomException(reflection::StatusCode::BadPointer);)
