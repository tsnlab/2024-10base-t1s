cmake_minimum_required(VERSION 3.10)

file(GLOB subdir *)
foreach(subdir ${subdir})
  if(IS_DIRECTORY ${subdir})
    add_subdirectory(${subdir})
  endif()
endforeach()
