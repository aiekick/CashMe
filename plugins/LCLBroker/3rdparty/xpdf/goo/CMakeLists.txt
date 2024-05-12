#========================================================================
#
# goo/CMakeLists.txt
#
# CMake script for the goo library.
#
# Copyright 2015 Glyph & Cog, LLC
#
#========================================================================

add_library(goo STATIC
  FixedPoint.cc
  GHash.cc
  GList.cc
  GString.cc
  gfile.cc
  gmem.cc
  gmempp.cc
  parseargs.c
  Trace.cc
)

