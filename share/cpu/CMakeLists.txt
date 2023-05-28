file(GLOB CPUS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpu)
install(FILES ${CPUS} DESTINATION share/xlr8/cpu)
