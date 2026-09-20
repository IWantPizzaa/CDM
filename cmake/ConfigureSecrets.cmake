# An explicit -DCDM_API_KEY takes precedence. Environment-only keys stay out of
# CMakeCache.txt, though the generated header and DLL necessarily contain them.
IF (NOT DEFINED CDM_API_KEY)
    SET(CDM_API_KEY "$ENV{CDM_API_KEY}")
ENDIF ()

IF (CDM_API_KEY STREQUAL "")
    IF (CDM_REQUIRE_API_KEY)
        MESSAGE(FATAL_ERROR "CDM_API_KEY is required for an authenticated CDM build.")
    ENDIF ()
    MESSAGE(WARNING "CDM_API_KEY is empty; CDM API authentication will not work.")
ENDIF ()
IF (CDM_API_KEY MATCHES "[\r\n]")
    MESSAGE(FATAL_ERROR "CDM_API_KEY must not contain line breaks.")
ENDIF ()

# Escape a C++ string literal; @ONLY keeps any ${...} in a key literal.
STRING(REPLACE "\\" "\\\\" CDM_API_KEY_ESCAPED "${CDM_API_KEY}")
STRING(REPLACE "\"" "\\\"" CDM_API_KEY_ESCAPED "${CDM_API_KEY_ESCAPED}")
CONFIGURE_FILE(
    "${CMAKE_CURRENT_LIST_DIR}/../src/Secrets.h.in"
    "${CMAKE_BINARY_DIR}/Secrets.h"
    @ONLY
)
