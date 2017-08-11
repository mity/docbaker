
configure_file(version.h.in "version.h" @ONLY)
include_directories("${CMAKE_CURRENT_BINARY_DIR}" ".")

file(GLOB DOCBAKER_SOURCES "*.c" "*.h")
add_executable(docbaker ${DOCBAKER_SOURCES})
