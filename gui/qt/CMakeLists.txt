cmake_minimum_required (VERSION 3.9)

project(qtgui)

#message("${MyBCopyObjDir}")
#message("${MyBCopy_SOURCES}")
#message("${MyBCopy_EXTRALIBS}")

set(QT5_BUILDDIR "" CACHE PATH "Qt5 Dir. Example: C:\\Qt5\\msvc2015 ")
if(NOT DEFINED QT5_BUILDDIR OR QT5_BUILDDIR STREQUAL "")		
		message(FATAL_ERROR "define a varrible QT5_BUILDDIR and specify the path to qt library")
endif()

set (CMAKE_PREFIX_PATH ${QT5_BUILDDIR})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)



find_package (Qt5Widgets REQUIRED)
find_package (Qt5LinguistTools REQUIRED)
 


file(GLOB_RECURSE SOURCES *.cpp)
file(GLOB_RECURSE HEADERS *.h *.inl)
SOURCE_GROUP(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES} ${HEADERS})

file(GLOB_RECURSE UIForms *.ui)
SOURCE_GROUP(UIForms FILES ${UIForms})

file(GLOB_RECURSE QRCResources *.qrc)
file(GLOB_RECURSE RCResources *.rc)
SOURCE_GROUP(Resources FILES ${QRCResources} ${RCResources})


#foreach(file IN LISTS UIForms)		
		#get_filename_component(fileWE ${file} NAME_WE)		
		#qt5_wrap_ui(${CMAKE_CURRENT_BINARY_DIR}/ui_${fileWE}.h ${file})		
#endforeach(file)
file(GLOB_RECURSE TSFiles *.ts)
SOURCE_GROUP(TS FILES ${TSFiles})
SOURCE_GROUP(~Generated FILES ${CMAKE_CURRENT_BINARY_DIR}/qtgui_autogen)



add_executable(qtgui WIN32 ${SOURCES} ${HEADERS} ${UIForms} ${QRCResources} ${RCResources} ${TSFiles})
set_property(TARGET qtgui PROPERTY OUTPUT_NAME MyBCopy-qt5)

#translation
function(_ADD_TRANSLATION_QT5 tsFiles)	
    foreach(tsFile ${tsFiles})
		get_filename_component(fileWE ${tsFile} NAME_WE)
		get_property(ExeFileName TARGET qtgui PROPERTY OUTPUT_NAME)
		set(qmFile "${DeployDir}/$<CONFIG>/${ExeFileName}_${fileWE}.qm")				
	
		add_custom_command(TARGET qtgui POST_BUILD			
			COMMAND "${Qt5_LRELEASE_EXECUTABLE}"
			ARGS -nounfinished -removeidentical -compress ${tsFile} -qm ${qmFile}
			VERBATIM
		)		
    endforeach()    
endfunction()

_ADD_TRANSLATION_QT5(${TSFiles})

add_custom_target(qtgui_lupdate COMMAND "${Qt5_LUPDATE_EXECUTABLE}" -no-obsolete ${CMAKE_CURRENT_SOURCE_DIR} -ts "${CMAKE_CURRENT_SOURCE_DIR}/ru.ts")




include_directories(${CMAKE_SOURCE_DIR}/src)

SET_PROPERTY(TARGET qtgui PROPERTY AUTOMOC_MOC_OPTIONS -b qtgui_pch.h) 

#target_link_libraries(qtgui  ${MyBCopy_OBJECTS})
#target_link_libraries(qtgui  ${MyBCopy_EXTRALIBS})


target_link_libraries(qtgui  MyBCopy)



target_link_libraries(qtgui Qt5::Widgets)
add_precompiled_header(qtgui qtgui_pch.h FORCEINCLUDE SOURCE_CXX ${CMAKE_CURRENT_SOURCE_DIR}/qtgui_pch.cpp)


InheritanceFromMyBCopyMacro(qtgui)