cmake_minimum_required (VERSION 3.9)

project(cli)



#Sources


RECURSIVE_GLOBBING()

ADD_EXECUTABLE(cli ${SOURCES} ${HEADERS} ${OTHERS})
set_property(TARGET cli PROPERTY OUTPUT_NAME MyBCopy)

add_precompiled_header(cli cli_pch.h FORCEINCLUDE SOURCE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/cli_pch.cpp)

include_directories(${CMAKE_SOURCE_DIR}/src)


target_link_libraries(cli  MyBCopy)


InheritanceFromMyBCopyMacro(cli)
