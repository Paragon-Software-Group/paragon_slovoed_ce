# Сборочный файл для Compiler

cmake_minimum_required( VERSION 2.6 )

set( BIN Compiler )
project( ${BIN} )

set( SOURCE_ROOT Compiler )
aux_source_directory( ${SOURCE_ROOT} SOURCES )
aux_source_directory( Compiler/PugiXML PUGI_SOURCES )
include_directories( "${SOURCE_ROOT}" )
include_directories( "${PUGI_SOURCES}" )

set( CMAKE_CXX_FLAGS "-Wall -O2 -std=c++0x" )
add_definitions( 
	-DGCC_BUILD 
	-DNDEBUG 
	-D_CONSOLE 
	-D_CRT_SECURE_NO_WARNINGS 
	-D_CRT_NON_CONFORMING_SWPRINTFS 
	-D_LINUX )

set( REQUIRED_LIBRARIES
	pthread )

add_executable( ${BIN} ${SOURCES} ${PUGI_SOURCES} )
target_link_libraries( ${BIN} ${REQUIRED_LIBRARIES} )
