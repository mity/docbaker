
find_package(LibClang REQUIRED)

if(WIN32)
    if(NOT "${LIBCLANG_RUNTIME_INCLUDE_DIR}" STREQUAL "")
        # On Windows, we copy LibClang the runtime system headers
        # so it becomes part of the package.
        file(COPY "${LIBCLANG_RUNTIME_INCLUDE_DIR}" DESTINATION "${CMAKE_BINARY_DIR}")
    endif()
    set(LIBCLANG_SYSINCDIR "include")
    file(COPY "${LIBCLANG_LIBRARY_DIR}/../bin/libclang.dll" DESTINATION "${CMAKE_BINARY_DIR}")
else()
    set(LIBCLANG_SYSINCDIR "${LIBCLANG_RUNTIME_INCLUDE_DIR}")
endif()


if(GETTEXT_FOUND)
    set(ENABLE_I18N 1)
endif()

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()


configure_file(config.h.in "config.h" )

include_directories("." "${CMAKE_CURRENT_BINARY_DIR}" "${LIBCLANG_INCLUDE_DIR}")

file(GLOB DOCBAKER_SOURCES "*.c" "*.h")
add_executable(docbaker ${DOCBAKER_SOURCES})
target_link_libraries(docbaker ${LIBCLANG_LIBRARY})
