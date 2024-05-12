#========================================================================
#
# cmake-xpdf.txt
#
# CMake script for the Xpdf package.
#
# Copyright 2021 Glyph & Cog, LLC
#
#========================================================================

cmake_minimum_required(VERSION 2.8.12)

project(xpdf)

include(cmake-config.txt)

add_subdirectory(goo)
add_subdirectory(fofi)
add_subdirectory(splash)
add_subdirectory(xpdf)
