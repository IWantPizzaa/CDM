CMAKE_MINIMUM_REQUIRED(VERSION 3.15)
IF (NOT DEFINED TEST_BINARY_DIR)
    MESSAGE(FATAL_ERROR "TEST_BINARY_DIR is required")
ENDIF ()
GET_FILENAME_COMPONENT(configure_script "${CMAKE_CURRENT_LIST_DIR}/../cmake/ConfigureSecrets.cmake" ABSOLUTE)

FUNCTION(check_case name environment_key expected)
    SET(case_dir "${TEST_BINARY_DIR}/${name}")
    FILE(MAKE_DIRECTORY "${case_dir}")
    EXECUTE_PROCESS(
        COMMAND "${CMAKE_COMMAND}" -E env "CDM_API_KEY=${environment_key}"
            "${CMAKE_COMMAND}" ${ARGN} -P "${configure_script}"
        WORKING_DIRECTORY "${case_dir}"
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors)
    IF (NOT result EQUAL 0)
        MESSAGE(FATAL_ERROR "${name}: configuration unexpectedly failed")
    ENDIF ()
    FILE(READ "${case_dir}/Secrets.h" header)
    STRING(FIND "${header}" "#define CDM_API_KEY \"${expected}\"" found)
    IF (found EQUAL -1)
        MESSAGE(FATAL_ERROR "${name}: generated literal mismatch")
    ENDIF ()
    STRING(FIND "${output}${errors}" "${expected}" leaked)
    IF (NOT expected STREQUAL "" AND NOT leaked EQUAL -1)
        MESSAGE(FATAL_ERROR "${name}: configuration printed its key")
    ENDIF ()
ENDFUNCTION()

check_case(environment_only "test-env-key" "test-env-key")
check_case(explicit_precedence "test-env-key" "test-explicit-key" -DCDM_API_KEY=test-explicit-key)
check_case(empty_override "test-env-key" "" -DCDM_API_KEY=)
check_case(missing_optional "" "")
check_case(required_present "test-required-key" "test-required-key" -DCDM_REQUIRE_API_KEY=ON)
check_case(escaping [=[quote"slash\literal${UNCHANGED}@value@]=] [=[quote\"slash\\literal${UNCHANGED}@value@]=])

FOREACH(name required_missing invalid_newline)
    SET(case_dir "${TEST_BINARY_DIR}/${name}")
    FILE(MAKE_DIRECTORY "${case_dir}")
    SET(key "")
    IF (name STREQUAL "invalid_newline")
        SET(key "invalid\nkey")
    ENDIF ()
    EXECUTE_PROCESS(
        COMMAND "${CMAKE_COMMAND}" -E env "CDM_API_KEY=${key}"
            "${CMAKE_COMMAND}" -DCDM_REQUIRE_API_KEY=ON -P "${configure_script}"
        WORKING_DIRECTORY "${case_dir}"
        RESULT_VARIABLE result OUTPUT_QUIET ERROR_QUIET)
    IF (result EQUAL 0)
        MESSAGE(FATAL_ERROR "${name}: invalid configuration unexpectedly succeeded")
    ENDIF ()
ENDFOREACH()
MESSAGE(STATUS "API key configuration: all 8 cases passed (test credentials only).")
