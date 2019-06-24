cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/modules)

project(harmonics)
set(MAYA_VERSION "2018" CACHE STRING "The Maya Version")

find_package(Maya REQUIRED)
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/../output/Maya${MAYA_VERSION})

set(SOURCE_FILES
    "lib/harmonicSolver.cpp"
    "lib/harmonicSolver.h"
    "maya/harmonics.cpp"
    "maya/harmonics.h"
    "maya/mapData.cpp"
    "maya/mapData.h"
    "maya/pluginMain.cpp"
)

include_directories(${MAYA_INCLUDE_DIR} maya lib)
link_directories(${MAYA_LIBRARY_DIR})
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${MAYA_LIBRARIES})

MAYA_PLUGIN(${PROJECT_NAME})
install(TARGETS ${PROJECT_NAME} ${MAYA_TARGET_TYPE} DESTINATION plug-ins)

