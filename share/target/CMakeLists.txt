file(GLOB TARGETS ${CMAKE_CURRENT_SOURCE_DIR}/*.target)
install(FILES ${TARGETS} DESTINATION share/xlr8/target)
