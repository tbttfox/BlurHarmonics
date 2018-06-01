cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/modules)
set(TARGET_DCC Maya CACHE STRING "Target Content Creation Package: Maya Max")


if (${TARGET_DCC} STREQUAL "Maya")
    project(harmonics)
    set(MAYA_VERSION "2016.5" CACHE STRING "The Maya Version")

    find_package(Maya REQUIRED)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/../output/Maya${MAYA_VERSION})

    set(SOURCE_FILES
        "lib/harmonicSolver.cpp"
        "maya/harmonics.cpp"
        "maya/mapData.cpp"
        "maya/pluginMain.cpp"
    )

    include_directories(${MAYA_INCLUDE_DIR} maya lib)
    link_directories(${MAYA_LIBRARY_DIR})
    add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
    target_link_libraries(${PROJECT_NAME} ${MAYA_LIBRARIES})

    MAYA_PLUGIN(${PROJECT_NAME})
    install(TARGETS ${PROJECT_NAME} ${MAYA_TARGET_TYPE} DESTINATION plug-ins)
elseif (${TARGET_DCC} STREQUAL "Max")
    project(harmonics)

    set(MAX_VERSION "2016" CACHE STRING "The 3dsMax version")

    find_package(3dsMax REQUIRED)

    set(RESOURCE_FILES
        "max/harmonicController.rc"
        "max/harmonicController.def"
    )

    set(SOURCE_FILES
        "lib/harmonicSolver.cpp"
        "max/harmonicController.cpp"
        "max/DllEntry.cpp"
    )

    add_definitions(-DUNICODE)

    include_directories(${MAXSDK_INCLUDE_DIR} max lib)
    add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${RESOURCE_FILES})
	set_target_properties( ${PROJECT_NAME} PROPERTIES SUFFIX ".dlu" )
    target_link_libraries(${PROJECT_NAME} ${MAXSDK_LIBRARIES})
else()
    message(FATAL_ERROR "Improper DCC Specified.  Must be either Maya or XSI")
endif()

