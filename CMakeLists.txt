cmake_minimum_required(VERSION 3.10)

project(10baset1s)

option(BUILD_EXAMPLES "Build examples" ON)

set(PUBLIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(PRIVATE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB_RECURSE INCLUDE_FILES ${PUBLIC_INCLUDE_DIR}/*.h)
file(GLOB_RECURSE SOURCE_FILES ${PRIVATE_INCLUDE_DIR}/*.c ${PRIVATE_INCLUDE_DIR}/*.h)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES} ${INCLUDE_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ${PUBLIC_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${PRIVATE_INCLUDE_DIR})
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror -pedantic)

if (BUILD_EXAMPLES)
  add_subdirectory(examples)
endif	(BUILD_EXAMPLES)

install(
  TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  COMPONENT library
  )

install(
  DIRECTORY ${PUBLIC_INCLUDE_DIR}
  DESTINATION include
  )
