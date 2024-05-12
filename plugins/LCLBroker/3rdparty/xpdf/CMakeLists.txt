#========================================================================
#
# cmake-xpdf.txt
#
# CMake script for the Xpdf package.
#
# Copyright 2021 Glyph & Cog, LLC
#
#========================================================================

cmake_minimum_required(VERSION 3.1)

project(xpdf)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/goo
	${CMAKE_CURRENT_SOURCE_DIR}/fofi
	${CMAKE_CURRENT_SOURCE_DIR}/xpdf
	${CMAKE_CURRENT_SOURCE_DIR}/splash
)

add_subdirectory(goo)
add_subdirectory(fofi)
add_subdirectory(splash)
add_subdirectory(xpdf)
