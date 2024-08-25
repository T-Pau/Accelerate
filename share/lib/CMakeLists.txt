# TODO: add dependencies (c128.s and mega65.s include c64.s)

if(${CMAKE_GENERATOR} EQUAL "Visual Studio 17 2022")
    message(STATUS "Visual Studio detected")
    if(${CMAKE_BUILD_TYPE} EQUAL "")
        message(STATUS "no build type, defaulting to Release")
        set(BIN_SUBDIRECTORY, "/Release")
    else()
        message(STATUS "using build type ${CMAKE_BUILD_TYPE}")
        set(BIN_SUBDIRECTORY, "/${CMAKE_BUILD_TYPE}")
    endif()
else()
    message(STATUS "not using Visual Studio")
    set(BIN_SUBDIRECTORY, "")
endif()
set(XLR8 ${CMAKE_BINARY_DIR}/src${BIN_SUBDIRECTORY}/xlr8${CMAKE_EXECUTABLE_SUFFIX})
message(STATUS "CMAKE_BUILD_TYPE='${CMAKE_BUILD_TYPE}', CMAKE_GENERATOR='${CMAKE_GENERATOR}', BIN_SUBDIRECTORY='${BIN_SUBDIRECTORY}', XLR8='${XLR8}'")

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.s)

cmake_policy(SET CMP0116 NEW)

foreach(SOURCE IN LISTS SOURCES)
    get_filename_component(NAME ${SOURCE} NAME_WLE)
    set(LIB ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.lib)
    set(depfile ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.d)
    add_custom_command(
            OUTPUT ${LIB}
            MAIN_DEPENDENCY ${SOURCE}
            DEPENDS ${XLR8}
            DEPFILE ${depfile}
            COMMAND ${XLR8} --system-directory ${CMAKE_SOURCE_DIR}/share --depfile ${depfile} --create-library -o ${LIB} ${SOURCE}
    )
    list(APPEND LIBS ${LIB})
endforeach()

add_custom_target(libraries ALL DEPENDS ${LIBS})

install(FILES ${LIBS} DESTINATION share/xlr8/lib)
