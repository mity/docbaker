

if(GETTEXT_FOUND)
    set(ENABLE_I18N 1)
endif()

configure_file(config.h.in "config.h" @ONLY)


include_directories("." "${CMAKE_CURRENT_BINARY_DIR}")

file(GLOB DOCBAKER_SOURCES "*.c" "*.h")
add_executable(docbaker ${DOCBAKER_SOURCES})
target_link_libraries(docbaker clang)
