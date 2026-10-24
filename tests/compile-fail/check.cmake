# A compile-fail test passes only if compilation fails with the expected error.

foreach(_required IN ITEMS BUILD_DIR BUILD_CONFIG TARGET_NAME EXPECTED_DIAGNOSTIC)
    if(NOT DEFINED ${_required})
        message(FATAL_ERROR "Missing ${_required}")
    endif()
endforeach()

execute_process(
    COMMAND "${CMAKE_COMMAND}" --build "${BUILD_DIR}"
        --config "${BUILD_CONFIG}" --target "${TARGET_NAME}"
    RESULT_VARIABLE _result
    OUTPUT_VARIABLE _stdout
    ERROR_VARIABLE _stderr
)

set(_diagnostics "${_stdout}\n${_stderr}")

if("${_result}" STREQUAL "0")
    message(FATAL_ERROR
        "Compilation unexpectedly succeeded for ${TARGET_NAME}.\n${_diagnostics}")
endif()

string(FIND "${_diagnostics}" "${EXPECTED_DIAGNOSTIC}" _position)
if(_position EQUAL -1)
    message(FATAL_ERROR
        "Compilation of ${TARGET_NAME} failed without the expected diagnostic:\n"
        "  ${EXPECTED_DIAGNOSTIC}\n\n"
        "Actual build output:\n${_diagnostics}")
endif()

message(STATUS "PASS: ${TARGET_NAME} (expected diagnostic found)")
