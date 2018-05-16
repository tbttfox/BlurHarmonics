cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/modules)
set(TARGET_DCC Maya CACHE STRING "Target Content Creation Package: Maya XSI Python")

include_directories(lib)

if (${TARGET_DCC} STREQUAL "Maya")
    project(harmonics)
    find_package(Maya REQUIRED)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/../output/Maya${MAYA_VERSION})

    set(SOURCE_FILES
        "lib/harmonicSolver.cpp"
        "maya/harmonics.cpp"
        "maya/mapData.cpp"
        "maya/pluginMain.cpp"
    )

    include_directories(${MAYA_INCLUDE_DIR} maya)
    link_directories(${MAYA_LIBRARY_DIR})
    add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
    target_link_libraries(${PROJECT_NAME} ${MAYA_LIBRARIES})

    MAYA_PLUGIN(${PROJECT_NAME})
    install(TARGETS ${PROJECT_NAME} ${MAYA_TARGET_TYPE} DESTINATION plug-ins)


else()
    message(FATAL_ERROR "Improper DCC Specified.  Must be either Maya or XSI")
endif()



