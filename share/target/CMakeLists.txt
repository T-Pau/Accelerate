file(GLOB TARGETS ${CMAKE_CURRENT_SOURCE_DIR}/*.target)
file(GLOB INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/*.inc)
install(FILES ${TARGETS} DESTINATION share/xlr8/target)
install(FILES ${INCLUDES} DESTINATION share/xlr8/target)
