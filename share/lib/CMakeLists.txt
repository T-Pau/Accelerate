file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.s)

set(XLR8 ${CMAKE_BINARY_DIR}/src/xlr8)

foreach(SOURCE IN LISTS SOURCES)
    get_filename_component(NAME ${SOURCE} NAME_WLE)
    set(LIB ${CMAKE_CURRENT_BINARY_DIR}}/${NAME}.lib)
    add_custom_command(
            OUTPUT ${LIB}
            MAIN_DEPENDENCY ${SOURCE}
            COMMAND ${XLR8}  --system-directory ${CMAKE_SOURCE_DIR}/share --target 6502 --create-library -o ${LIB} ${SOURCE}
    )
    list(APPEND LIBS ${LIB})
endforeach()

add_custom_target(libraries ALL DEPENDS ${LIBS})

install(FILES ${LIBS} DESTINATION share/xlr8/lib)
