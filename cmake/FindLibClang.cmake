# FindLibClang
#
# This modules searches libclang and related stuff.
#
# It defines the following variables:
# 
# LIBCLANG_FOUND:               Set if libclang has been found.
# LIBCLANG_INCLUDE_DIR:         Include directory (containing clang-c/Index.h et al.)
# LIBCLANG_RUNTIME_INCLUDE_DIR: (Optional) run-time include directory (containing <stdarg.h> et al.)
# LIBCLANG_LIBRARY_DIR:         Directory where libclang is located
# LIBCLANG_LIBRARY:             The libclang library


set(LIBCLANG_LLVM_VERSION_LIST
        5.0.0
        5.0
        4.0.1
        4.0.0
        3.9.1
        3.9.0
        3.9
        3.8.1
        3.8.0
        3.8
)

set(libclang_llvm_header_search_paths)
set(libclang_llvm_lib_search_paths)

list(APPEND libclang_llvm_lib_search_paths "/usr/lib/llvm")


if (NOT "${LLVM_PREFIX}" STREQUAL "")
    list(APPEND libclang_llvm_header_search_paths
            "${LLVM_PREFIX}/include")

    list(APPEND libclang_llvm_lib_search_paths
            "${LLVM_PREFIX}/lib")
endif()


foreach(version ${LIBCLANG_LLVM_VERSION_LIST})
    list(APPEND libclang_llvm_header_search_paths
            "/usr/lib/llvm-${version}/include"
            "/usr/lib/llvm/${version}/include"
            "/opt/local/llvm-${version}/include")

    list(APPEND libclang_llvm_lib_search_paths
            "/usr/lib/llvm-${version}/lib"
            "/usr/lib/llvm/${version}/lib"
            "/opt/local/llvm-${version}/lib")
endforeach()


find_path(LIBCLANG_INCLUDE_DIR "clang-c/Index.h"
        PATHS ${libclang_llvm_header_search_paths}
        PATH_SUFFIXES "LLVM/include"    # for Windows
        DOC "The path to libclang include dir (must contain clang-c/Index.h et al.)")

find_library(LIBCLANG_LIBRARY NAMES libclang.imp libclang clang
        PATHS ${libclang_llvm_lib_search_paths}
        PATH_SUFFIXES "LLVM/lib"    # for Windows
        DOC "The file path of libclang library")
get_filename_component(LIBCLANG_LIBRARY_DIR ${LIBCLANG_LIBRARY} PATH)


find_path(LIBCLANG_RUNTIME_INCLUDE_DIR_HELPER "include/stdarg.h"
        PATHS "${LIBCLANG_LIBRARY_DIR}/clang"
        PATH_SUFFIXES ${LIBCLANG_LLVM_VERSION_LIST}
        DOC "The path to libclang run-time include dir (should contain stdarg.h et al.")
if ("${LIBCLANG_RUNTIME_INCLUDE_DIR_HELPER}" STREQUAL "LIBCLANG_RUNTIME_INCLUDE_DIR-NOTFOUND")
    message(WARNING "LibClang runtime include dir not found.")
    message(WARNING "Re-try and set LIBCLANG_RUNTIME_INCLUDE_DIR manually.")
    message(WARNING "Otherwise user would have to use something like 'docbaker -isystem ${LLVM_ROOT}/lib/clang/4.0.0/include' to prevent run-time errors.")
else()
    set(LIBCLANG_RUNTIME_INCLUDE_DIR "${LIBCLANG_RUNTIME_INCLUDE_DIR_HELPER}/include")
endif()

set(LIBCLANG_LIBRARIES ${LIBCLANG_LIBRARY})
set(LIBCLANG_INCLUDE_DIRS ${LIBCLANG_INCLUDE_DIR})



include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBCLANG_FOUND to TRUE if
# all listed variables are TRUE
find_package_handle_standard_args(LibClang DEFAULT_MSG
        LIBCLANG_LIBRARY LIBCLANG_INCLUDE_DIR)

