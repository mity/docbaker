
find_package(LibClang REQUIRED)

if(WIN32)
    # For WIN32 platform, we copy LibClang the runtime system headers
    # in the post-build step and they become part of the app.
    # Also for this platform, the path is in run-time prepended with
    # path to the executable (derived from argv[0]).
    set(LIBCLANG_SYSINCDIR "include")
else()
    set(LIBCLANG_SYSINCDIR "${LIBCLANG_RUNTIME_INCLUDE_DIR}")
endif()

configure_file(config.h.in "config.h" )

if(GETTEXT_FOUND)
    set(ENABLE_I18N 1)
endif()

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()



include_directories("." "3rd_party" "${CMAKE_CURRENT_BINARY_DIR}" "${LIBCLANG_INCLUDE_DIR}")

add_executable(docbaker
        3rd_party/buffer.c
        3rd_party/buffer.h
        3rd_party/cmdline.c
        3rd_party/cmdline.h
        3rd_party/fnv1a.c
        3rd_party/fnv1a.h
        3rd_party/json.c
        3rd_party/json.h
        3rd_party/json-dom.c
        3rd_party/json-dom.h
        3rd_party/value.c
        3rd_party/value.h
        array.c
        array.h
        gen_html.c
        gen_html.h
        gen_json.c
        gen_json.h
        main.c
        misc.c
        misc.h
        parse_cxx.c
        parse_cxx.h
        path.c
        path.h
        store.c
        store.h
        win32compat.h
)

target_link_libraries(docbaker ${LIBCLANG_LIBRARY})


if(WIN32)
    # For WIN32, copy libclang.dll and headers it needs in run-time into our
    # build directory.
    add_custom_command(TARGET docbaker POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${LIBCLANG_LIBRARY_DIR}/../bin/libclang.dll"
            "$<TARGET_FILE_DIR:docbaker>")

    if(NOT "${LIBCLANG_RUNTIME_INCLUDE_DIR}" STREQUAL "")
        add_custom_command(TARGET docbaker POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${LIBCLANG_RUNTIME_INCLUDE_DIR}"
                "$<TARGET_FILE_DIR:docbaker>/include")
    endif()
endif()
