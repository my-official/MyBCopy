cmake_minimum_required (VERSION 3.9)

project(MyBCopy)


set(QT5_BUILDDIR "" CACHE PATH "Qt5 Dir. Example: C:\\Qt5\\msvc2015 ")
set(WINREG_INCLUDE_DIR "" CACHE PATH "WinReg directory path (https://github.com/GiovanniDicanio/WinReg)")
set(PUGIXML_INCLUDE_DIR "" CACHE PATH "Pugixml include path")
set(PUGIXML_LIBRARY_DEBUG "" CACHE FILEPATH "Pugixml lib")
set(PUGIXML_LIBRARY_RELEASE "" CACHE FILEPATH "Pugixml lib")


if(NOT DEFINED QT5_BUILDDIR OR QT5_BUILDDIR STREQUAL "")		
		message(FATAL_ERROR "define a varrible QT5_BUILDDIR and specify the path to qt library")
endif()

set (CMAKE_PREFIX_PATH ${QT5_BUILDDIR})

#Sources


FILE(GLOB_RECURSE SOURCES *.cpp)
SET(MyBCopy_SOURCES ${SOURCES} PARENT_SCOPE)
FILE(GLOB_RECURSE HEADERS *.h *.inl)
SET(MyBCopy_HEADERS ${HEADERS} PARENT_SCOPE)

FILE(GLOB_RECURSE OTHERS *)
FILE(GLOB OTHERS_PARENT LIST_DIRECTORIES false ../*)
LIST(APPEND OTHERS ${OTHERS_PARENT})
FILE(GLOB_RECURSE UserSpecificLocalData ../UserSpecificLocalData/*)
LIST(APPEND OTHERS ${UserSpecificLocalData})





SET(MyBCopyObjDir ${MyBCopy_BINARY_DIR}/MyBCopy.dir/${CMAKE_CFG_INTDIR})

foreach(objFile IN LISTS SOURCES)		
	get_filename_component(objFileWithoutExt ${objFile} NAME_WE)	
	if(objFileWithoutExt STREQUAL "MyBCopy")
		continue()
	endif() 
		
	LIST(APPEND OBJECTS ${MyBCopyObjDir}/${objFileWithoutExt}.obj)	
endforeach(objFile)

SET(MyBCopy_OBJECTS ${OBJECTS} PARENT_SCOPE)

if (Build_STATIC_MyBCopy)
	ADD_LIBRARY(MyBCopy STATIC ${SOURCES} ${HEADERS} ${OTHERS})	
else()
	ADD_LIBRARY(MyBCopy SHARED ${SOURCES} ${HEADERS} ${OTHERS})
	add_definitions(-DMyBCopy_EXPORT)
endif ()


#############
#Dependencies
#############



#WinReg
if(NOT DEFINED WINREG_INCLUDE_DIR OR WINREG_INCLUDE_DIR STREQUAL "")		
		message(FATAL_ERROR "define a varrible WINREG_INCLUDE_DIR and specify the path")
endif()

LIST(APPEND INCLUDEDIRS ${WINREG_INCLUDE_DIR})



#Windows
target_link_libraries(MyBCopy Mpr.lib)



#pugixml

if(NOT DEFINED PUGIXML_INCLUDE_DIR OR PUGIXML_INCLUDE_DIR STREQUAL "")		
		message(FATAL_ERROR "define a varrible PUGIXML_INCLUDE_DIR and specify the path")
endif()
LIST(APPEND INCLUDEDIRS ${PUGIXML_INCLUDE_DIR})

if(NOT DEFINED PUGIXML_LIBRARY_DEBUG OR PUGIXML_LIBRARY_DEBUG STREQUAL "")		
		message(FATAL_ERROR "define a varrible PUGIXML_LIBRARY_DEBUG and specify the path")
endif()

if(NOT DEFINED PUGIXML_LIBRARY_RELEASE OR PUGIXML_LIBRARY_RELEASE STREQUAL "")		
		message(FATAL_ERROR "define a varrible PUGIXML_LIBRARY_RELEASE and specify the path")
endif()

#if (${CMAKE_CL_64})
	#set(PugiXml ${DepDir}/pugixml/x64)
#else()
	#set(PugiXml ${DepDir}/pugixml/x86)
#endif()


target_link_libraries(MyBCopy debug ${PUGIXML_LIBRARY_DEBUG}
							optimized ${PUGIXML_LIBRARY_RELEASE} )


#Boost

FIND_PACKAGE( Boost REQUIRED )
LIST(APPEND EXTRA_CMAKE_PACKAGES Boost)

INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )
LIST(APPEND INCLUDEDIRS ${Boost_INCLUDE_DIR})



#Qt5

find_package (Qt5Core REQUIRED)
LIST(APPEND EXTRA_CMAKE_PACKAGES Qt5Core)

target_link_libraries(MyBCopy Qt5::Core)


add_precompiled_header(MyBCopy MyBCopy_pch.h FORCEINCLUDE SOURCE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/MyBCopy_pch.cpp)


include_directories(${INCLUDEDIRS})





SET(MyBCopy_INCLUDEDIRS ${INCLUDEDIRS} PARENT_SCOPE)




SET(MyBCopy_PACKAGES_REQUIRED ${EXTRA_CMAKE_PACKAGES} PARENT_SCOPE)

get_target_property(EXTRA_LIBS MyBCopy LINK_LIBRARIES)
SET(MyBCopy_EXTRALIBS ${EXTRA_LIBS} PARENT_SCOPE)