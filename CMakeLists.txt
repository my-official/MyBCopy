cmake_minimum_required (VERSION 3.9)
project(MyBCopy)

include(${CMAKE_SOURCE_DIR}/CMake/PrecompiledHeader.cmake)
include(${CMAKE_SOURCE_DIR}/CMake/Utils.cmake)

SET(DeployDir ${CMAKE_BINARY_DIR}/Output)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Output)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Output)

set (CMAKE_CXX_STANDARD 17)


if(MSVC)	
    add_definitions(-DUNICODE -D_UNICODE)
	add_compile_options(/MP)	

	option (Build_STATIC_RUNTIME "Linkage with static runtime library" OFF)	
	if (Build_STATIC_RUNTIME)
		add_compile_options(/MT)
	endif ()	
endif()

set_property( GLOBAL PROPERTY USE_FOLDERS ON)



SET(Targets )

option (Build_STATIC_MyBCopy "Build MyBCopy as static library" OFF)

if (NOT Build_STATIC_MyBCopy)
	add_definitions(-DMyBCopy_SHARED_LIB)
endif ()



add_subdirectory(src)
LIST(APPEND Targets MyBCopy)

option (Build_CLI "Build MyBCopy command line interface" ON) 

if (Build_CLI)
	add_subdirectory(cli)
	add_dependencies(cli MyBCopy)
	LIST(APPEND Targets cli)
endif (Build_CLI)
 

option (Build_QTGUI "Build MyBCopy QT5 gui" ON) 

if (Build_QTGUI)
	add_subdirectory(gui/qt)
	add_dependencies(qtgui MyBCopy)
	LIST(APPEND Targets qtgui)
endif (Build_QTGUI)


if (NOT Build_STATIC_MyBCopy)
	remove_definitions(-DMyBCopy_SHARED_LIB)
endif ()

option (Build_TESTS "Build MyBCopy tests" OFF)

if (Build_TESTS)
	add_subdirectory(tests/BlackBox)
	set_property(TARGET BlackBox PROPERTY FOLDER "Tests") 
	add_dependencies(BlackBox MyBCopy)

	LIST(APPEND Targets BlackBox)
endif (Build_TESTS)


if(MSVC)	
	foreach(tg IN LISTS Targets)		
		SET_TARGET_PROPERTIES(${tg} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/Output/$(Configuration))	
	endforeach(tg)	
endif()
	

