set(QATERIAL_VERSION_MAJOR 1)
set(QATERIAL_VERSION_MINOR 4)
set(QATERIAL_VERSION_PATCH 6)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
  execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE QATERIAL_VERSION_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
endif()
if(NOT QATERIAL_VERSION_TAG)
  set(QATERIAL_VERSION_TAG 00000000)
endif()
set(QATERIAL_VERSION_TAG_HEX 0x${QATERIAL_VERSION_TAG})
set(QATERIAL_VERSION_TAG ${QATERIAL_VERSION_TAG} CACHE STRING "Git Tag of Qaterial")
set(QATERIAL_VERSION ${QATERIAL_VERSION_MAJOR}.${QATERIAL_VERSION_MINOR}.${QATERIAL_VERSION_PATCH} CACHE STRING "Version of Qaterial")
