CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

PROJECT(Accelerate
        VERSION 0.1
        LANGUAGES C CXX)
set(PACKAGE_BUGREPORT "accelerate@tpau.group")

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS OFF)

INCLUDE(GNUInstallDirs)

set(ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION})
IF(NOT TARGET dist)
ADD_CUSTOM_TARGET(dist
  COMMAND git config tar.tar.xz.command "xz -c"
  COMMAND git archive --prefix=${ARCHIVE_NAME}/ -o ${ARCHIVE_NAME}.tar.gz HEAD
  COMMAND git archive --prefix=${ARCHIVE_NAME}/ -o ${ARCHIVE_NAME}.tar.xz HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  )
ADD_CUSTOM_TARGET(distcheck
  COMMAND chmod -R u+w ${ARCHIVE_NAME} ${ARCHIVE_NAME}-build ${ARCHIVE_NAME}-dest 2>/dev/null || true
  COMMAND rm -rf ${ARCHIVE_NAME} ${ARCHIVE_NAME}-build ${ARCHIVE_NAME}-dest
  COMMAND cmake -E tar xf ${ARCHIVE_NAME}.tar.gz
  COMMAND chmod -R u-w ${ARCHIVE_NAME}
  COMMAND mkdir ${ARCHIVE_NAME}-build
  COMMAND mkdir ${ARCHIVE_NAME}-dest
  COMMAND cd ${ARCHIVE_NAME}-build && cmake -DCMAKE_INSTALL_PREFIX=../${ARCHIVE_NAME}-dest ../${ARCHIVE_NAME}
  COMMAND cd ${ARCHIVE_NAME}-build && make -j4
  COMMAND cd ${ARCHIVE_NAME}-build && make test
  COMMAND cd ${ARCHIVE_NAME}-build && make install
#  COMMAND cd ${ARCHIVE_NAME}-build && make uninstall
#  COMMAND if [ `find ${ARCHIVE_NAME}-dest ! -type d | wc -l` -ne 0 ]; then echo leftover files in ${ARCHIVE_NAME}-dest; false; fi
  COMMAND cd ${ARCHIVE_NAME}-build && make clean
  COMMAND chmod -R u+w ${ARCHIVE_NAME} ${ARCHIVE_NAME}-build ${ARCHIVE_NAME}-dest
  COMMAND rm -rf ${ARCHIVE_NAME} ${ARCHIVE_NAME}-build ${ARCHIVE_NAME}-dest
  COMMAND echo "${ARCHIVE_NAME}.tar.gz is ready for distribution."
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  )
ADD_DEPENDENCIES(distcheck dist)
ENDIF(NOT TARGET dist)

if(CMAKE_GENERATOR STREQUAL "Visual Studio 17 2022")
    if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME OR CMAKE_INSTALL_CONFIG_NAME STREQUAL "")
        set(BIN_SUBDIRECTORY "/Release")
    else()
        set(BIN_SUBDIRECTORY "/${CMAKE_INSTALL_CONFIG_NAME}")
    endif()
else()
    set(BIN_SUBDIRECTORY "")
endif()

# Checks

ADD_DEFINITIONS("-DHAVE_CONFIG_H")

# Testing
enable_testing()

# Targets
add_subdirectory(src)
add_subdirectory(share)
add_subdirectory(tests)

# write out config file
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
