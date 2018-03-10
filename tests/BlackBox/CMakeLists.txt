cmake_minimum_required (VERSION 3.9)

project(BlackBox)


#message("${MyBCopy_SOURCES}")
#message("${MyBCopy_EXTRALIBS}")


RECURSIVE_GLOBBING()


add_executable(BlackBox ${SOURCES} ${HEADERS} ${OTHERS})
	

include_directories(${CMAKE_SOURCE_DIR}/src)


target_link_libraries(BlackBox  ${MyBCopy_OBJECTS})

InheritanceFromMyBCopyMacro(BlackBox)
add_precompiled_header(BlackBox BlackBox_pch.h FORCEINCLUDE SOURCE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/BlackBox_pch.cpp)

set(BlackBoxTest_TESTFILES_DIR "" CACHE PATH "Path to directory for generated files")
set(BlackBoxTest_OVERRIDE_RAR_EXE "" CACHE FILEPATH "Path to non-system rar executable")
set(BlackBoxTest_OVERRIDE_7Z_EXE "" CACHE FILEPATH "Path to non-system 7z executable")
set(BlackBoxTest_TEMPORARY_DIR "" CACHE PATH "Path to temporary directory")

configure_file (
  ${PROJECT_SOURCE_DIR}/BlackBox_config.in.h
  ${PROJECT_BINARY_DIR}/BlackBox_config.h
  )

include_directories("${PROJECT_BINARY_DIR}")




macro(CopyXML)    
	configure_file (
		  ${PROJECT_SOURCE_DIR}/BlackBoxTest_in.mybcopy
		  ${DeployDir}/${config}/BlackBoxTest_in.mybcopy
		  )
endmacro()


if(DEFINED CMAKE_CONFIGURATION_TYPES  AND (NOT CMAKE_CONFIGURATION_TYPES  STREQUAL "") )		
	foreach(config IN LISTS CMAKE_CONFIGURATION_TYPES)	
		CopyXML()
	endforeach(config)
ELSE()
	SET(config )
	CopyXML()
endif()


RETURN()
#############################
#Config deploy


set(BlackBoxTest_OVERRIDE_RAR_DIR "" CACHE PATH "Path to non-system rar directory")

if(DEFINED BlackBoxTest_OVERRIDE_RAR_DIR AND (NOT BlackBoxTest_OVERRIDE_RAR_DIR STREQUAL "") )		
		set(BlackBoxTest_OVERRIDE_RAR_DIR path="${BlackBoxTest_OVERRIDE_RAR_DIR}")
endif()

set(BlackBoxTest_OVERRIDE_7Z_DIR "" CACHE PATH "Path to non-system rar directory")

if(DEFINED BlackBoxTest_OVERRIDE_7Z_DIR  AND (NOT BlackBoxTest_OVERRIDE_7Z_DIR  STREQUAL "") )		
		set(BlackBoxTest_OVERRIDE_7Z_DIR  "${BlackBoxTest_OVERRIDE_RAR_DIR}")
endif()


set(Backup1Source_DIR  ${BlackBoxTest_DirPath}/Source/)



#if(DEFINED CMAKE_CONFIGURATION_TYPES  AND (NOT CMAKE_CONFIGURATION_TYPES  STREQUAL "") )
#	foreach(config IN LISTS CMAKE_CONFIGURATION_TYPES)	
#		configure_file (
#		  ${PROJECT_SOURCE_DIR}/BlackBoxTest.mybcopy.in
#		  ${DeployDir}/${config}/BlackBoxTest.mybcopy
#		  )
#	endforeach(config)
#ELSE()
#	configure_file (
#		  ${PROJECT_SOURCE_DIR}/BlackBoxTest.mybcopy.in
#		  ${DeployDir}/BlackBoxTest.mybcopy
#		  )
#endif()




macro(ConfigurateXML)    
	configure_file (
		  ${PROJECT_SOURCE_DIR}/BlackBoxTest.in.mybcopy
		  ${DeployDir}/${config}/BlackBoxTest.mybcopy
		  )
endmacro()


if(DEFINED CMAKE_CONFIGURATION_TYPES  AND (NOT CMAKE_CONFIGURATION_TYPES  STREQUAL "") )		
	foreach(config IN LISTS CMAKE_CONFIGURATION_TYPES)	
		ConfigurateXML()
	endforeach(config)
ELSE()
	SET(config )
	ConfigurateXML()
endif()