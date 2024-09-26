# SetupChowdspLib.cmake

# Debug messages
message("PROJECT_SOURCE_DIR is ${PROJECT_SOURCE_DIR}")
message("CMAKE_CURRENT_SOURCE_DIR is ${CMAKE_CURRENT_SOURCE_DIR}")
message("CMAKE_CURRENT_LIST_DIR is ${CMAKE_CURRENT_LIST_DIR}")

# Set CHOWDSP_MODULES_DIR using CMAKE_CURRENT_LIST_DIR
set(CHOWDSP_MODULES_DIR "${CMAKE_CURRENT_LIST_DIR}/../modules" CACHE INTERNAL "Source directory for ChowDSP modules")
message("CHOWDSP_MODULES_DIR is ${CHOWDSP_MODULES_DIR}")

function(_chowdsp_find_module_dependencies module_header module_deps)
    file(STRINGS "${module_header}" dependencies_raw REGEX "[ \t]*dependencies:")
    string(STRIP ${dependencies_raw} dependencies_raw)
    string(SUBSTRING ${dependencies_raw} 13 -1 dependencies_raw)

    if("${dependencies_raw}" STREQUAL "")
        # No dependencies here! Let's move on...
        set (${module_deps} "" PARENT_SCOPE)
        return()
    endif()

    string(STRIP ${dependencies_raw} dependencies_raw)
    string(REPLACE ", " ";" dependencies_list ${dependencies_raw})
    set (${module_deps} ${dependencies_list} PARENT_SCOPE)
endfunction(_chowdsp_find_module_dependencies)

function(_chowdsp_load_module lib_name module)
    # Debug messages
    message(STATUS "Loading module: ${module}")
    message(STATUS "CHOWDSP_MODULES_DIR: ${CHOWDSP_MODULES_DIR}")

    # Paths to search
    set(MODULE_SEARCH_PATHS
        "${CHOWDSP_MODULES_DIR}/common/${module}"
        "${CHOWDSP_MODULES_DIR}/dsp/${module}"
        "${CHOWDSP_MODULES_DIR}/music/${module}"
    )

    message(STATUS "Searching for ${module}.h in:")
    foreach(path IN LISTS MODULE_SEARCH_PATHS)
        message(STATUS " - ${path}")
    endforeach()

    find_path(chowdsp_module_path
        NAMES ${module}.h
        PATHS ${MODULE_SEARCH_PATHS}
        NO_DEFAULT_PATH
        NO_CMAKE_FIND_ROOT_PATH
        REQUIRED
    )

    if(NOT chowdsp_module_path)
        message(FATAL_ERROR "Could not find ${module}.h in specified paths.")
    else()
        message(STATUS "Found ${module}.h at: ${chowdsp_module_path}")
    endif()

    get_filename_component(module_parent_path ${chowdsp_module_path} DIRECTORY)
    target_include_directories(${lib_name} PUBLIC ${module_parent_path})
    target_compile_definitions(${lib_name} PUBLIC JUCE_MODULE_AVAILABLE_${module})

    if(EXISTS "${chowdsp_module_path}/${module}.cpp")
        target_sources(${lib_name} PRIVATE "${chowdsp_module_path}/${module}.cpp")
    endif()

    # Load any modules that this one depends on:
    _chowdsp_find_module_dependencies(${chowdsp_module_path}/${module}.h module_dependencies)
    unset(chowdsp_module_path CACHE)
    foreach(module_dep IN LISTS module_dependencies)
        get_target_property(_lib_compile_defs ${lib_name} COMPILE_DEFINITIONS)
        if("JUCE_MODULE_AVAILABLE_${module_dep}" IN_LIST _lib_compile_defs)
            continue() # we've already loaded this module, so continue...
        endif()
        _chowdsp_load_module(${lib_name} ${module_dep})
    endforeach()
endfunction(_chowdsp_load_module)

function(setup_chowdsp_lib lib_name)
    set(multiValueArgs MODULES)
    cmake_parse_arguments(CHOWDSPLIB "" "" "${multiValueArgs}" ${ARGN})

    message(STATUS "Setting up ChowDSP Static Lib: ${lib_name}, with modules: ${CHOWDSPLIB_MODULES}")
    add_library(${lib_name} STATIC)

    foreach(module IN LISTS CHOWDSPLIB_MODULES)
        _chowdsp_load_module(${lib_name} ${module})
    endforeach()

    target_compile_definitions(${lib_name}
        PUBLIC
            $<IF:$<CONFIG:DEBUG>,DEBUG=1 _DEBUG=1,NDEBUG=1 _NDEBUG=1>
    )

    if(APPLE)
        target_link_libraries(${lib_name} PUBLIC "-framework Accelerate")
    elseif(UNIX)
        # We need to link to pthread explicitly on Linux/GCC
        set(THREADS_PREFER_PTHREAD_FLAG ON)
        find_package(Threads REQUIRED)
        target_link_libraries(${lib_name} PUBLIC Threads::Threads)
    endif()

    install(
        TARGETS ${lib_name}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endfunction(setup_chowdsp_lib)
