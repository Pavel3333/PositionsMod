project(PositionsMod)
cmake_minimum_required (VERSION 3.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(libpython REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

#directories
set(SOURCES
    "src/API_functions.cpp"
    "src/API_functions.h"

    "src/CConfig.cpp"
    "src/CConfig.h"
    
    "src/MyLogger.cpp"
    "src/MyLogger.h"

    "src/pch.h"
    "src/py_pch.h"
    
    "src/Py_config.cpp"
    "src/Py_config.h"

    "src/Py_main.cpp"

    "src/PyModule.cpp"
    "src/PyModule.hpp"
)


add_library(PositionsMod SHARED ${SOURCES})

target_compile_definitions(PositionsMod PRIVATE "-DNOMINMAX")

target_include_directories(PositionsMod PRIVATE "./include")

target_link_libraries(PositionsMod libpython::python27)

if(CMAKE_CL_64)
    set(ARCH x64)
else()
    set(ARCH Win32)
endif()

find_library(CURL_LIBRARY
    NAMES
        "libcurl"
    HINTS
        "lib/${ARCH}/"
    PATH_SUFFIXES
        "lib"
)

target_link_libraries(PositionsMod ${CURL_LIBRARY})

set_target_properties(PositionsMod PROPERTIES LINK_FLAGS "/DELAYLOAD:dll/libcurl.dll /INCREMENTAL:NO")

if(MSVC)
  target_compile_definitions(PositionsMod PRIVATE "-D_CRT_SECURE_NO_WARNINGS")
  set_target_properties(PositionsMod PROPERTIES COMPILE_FLAGS "/wd5033") # wd4005
endif()

set_target_properties(PositionsMod PROPERTIES SUFFIX ".pyd")

#optimization
if(MSVC)
    target_compile_options(PositionsMod PRIVATE /Oi)        # /Oi enable intrinsic functions
endif()

install(TARGETS  PositionsMod
        RUNTIME DESTINATION ".")
