cmake_minimum_required(VERSION 3.19)
project(cpp-stub CXX)

# Header-only interface library. Downstream usage:
#
#   add_subdirectory(third_party/cpp-stub)
#   add_executable(my_test test.cpp)
#   target_link_libraries(my_test PRIVATE cpp-stub)
#
# On macOS the build system automatically patches the produced binary
# after link so cpp-stub can rewrite __TEXT at runtime; no manual step
# is required. On Linux/Windows the post-build hook is a no-op.

add_library(cpp-stub INTERFACE)
add_library(cpp-stub::cpp-stub ALIAS cpp-stub)

target_include_directories(cpp-stub INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/src")

if(APPLE)
    target_include_directories(cpp-stub INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/src_darwin")
endif()

if(UNIX AND NOT APPLE)
    target_include_directories(cpp-stub INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/src_linux")
endif()

if(WIN32)
    target_include_directories(cpp-stub INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/src_win")
endif()

# -----------------------------------------------------------------------------
# macOS post-build automation
# -----------------------------------------------------------------------------
if(APPLE)
    set(_CPP_STUB_ENABLE_TOOL
        "${CMAKE_CURRENT_SOURCE_DIR}/tool/macos_enable_stub.sh"
        CACHE INTERNAL "")

    # Explicit opt-in helper for cases where the auto-detect below cannot
    # see a target (e.g. the executable only links cpp-stub transitively
    # through an intermediate static library).
    function(cpp_stub_enable target)
        get_target_property(_already "${target}" _CPP_STUB_POSTBUILD_ADDED)
        if(_already)
            return()
        endif()
        add_custom_command(TARGET "${target}" POST_BUILD
            COMMAND "${_CPP_STUB_ENABLE_TOOL}" "$<TARGET_FILE:${target}>"
            COMMENT "cpp-stub: patching __TEXT maxprot and re-signing ${target}"
            VERBATIM)
        set_target_properties("${target}" PROPERTIES
            _CPP_STUB_POSTBUILD_ADDED TRUE)
    endfunction()

    # Recursively collect all targets in the directory tree rooted at `dir`.
    function(_cpp_stub_collect_targets dir out_var)
        set(_acc "")
        get_property(_here DIRECTORY "${dir}" PROPERTY BUILDSYSTEM_TARGETS)
        if(_here)
            list(APPEND _acc ${_here})
        endif()
        get_property(_subs DIRECTORY "${dir}" PROPERTY SUBDIRECTORIES)
        foreach(_s IN LISTS _subs)
            _cpp_stub_collect_targets("${_s}" _sub)
            if(_sub)
                list(APPEND _acc ${_sub})
            endif()
        endforeach()
        set(${out_var} "${_acc}" PARENT_SCOPE)
    endfunction()

    # Deferred callback: invoked at end-of-config of the top-level directory,
    # by which time every target has been declared. For each executable that
    # (directly) links cpp-stub, attach the post-build patch step.
    function(_cpp_stub_auto_enable_all)
        _cpp_stub_collect_targets("${CMAKE_SOURCE_DIR}" _targets)
        foreach(_t IN LISTS _targets)
            if(NOT TARGET "${_t}")
                continue()
            endif()
            get_target_property(_type "${_t}" TYPE)
            if(NOT _type STREQUAL "EXECUTABLE")
                continue()
            endif()
            set(_links "")
            get_target_property(_l "${_t}" LINK_LIBRARIES)
            if(_l)
                list(APPEND _links ${_l})
            endif()
            get_target_property(_il "${_t}" INTERFACE_LINK_LIBRARIES)
            if(_il)
                list(APPEND _links ${_il})
            endif()
            if("cpp-stub" IN_LIST _links OR "cpp-stub::cpp-stub" IN_LIST _links)
                cpp_stub_enable("${_t}")
            endif()
        endforeach()
    endfunction()

    cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}"
        CALL _cpp_stub_auto_enable_all)
endif()
