#========================================================================
#
# fofi/CMakeLists.txt
#
# CMake script for the fofi library.
#
# Copyright 2015 Glyph & Cog, LLC
#
#========================================================================

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/goo
	${CMAKE_CURRENT_SOURCE_DIR}/xpdf
)

add_library(fofi STATIC
  FoFiBase.cc
  FoFiEncodings.cc
  FoFiIdentifier.cc
  FoFiTrueType.cc
  FoFiType1.cc
  FoFiType1C.cc
)

target_link_libraries(fofi PRIVATE goo)
