#========================================================================
#
# xpdf/cmake-xpdf.txt
#
# CMake script for the Xpdf tools.
#
# Copyright 2015 Glyph & Cog, LLC
#
#========================================================================

include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/goo")
include_directories("${PROJECT_SOURCE_DIR}/fofi")
include_directories("${PROJECT_SOURCE_DIR}/splash")
if (PNG_FOUND)
  include_directories("${PNG_INCLUDE_DIRS}")
  add_definitions("${PNG_DEFINITIONS}")
endif ()
if (HAVE_LCMS)
  include_directories("${LCMS_INCLUDE_DIR}")
  set(COLOR_MANAGER_SOURCE "ColorManager.cc")
else ()
  set(COLOR_MANAGER_SOURCE "")
endif ()
if (WIN32)
  set(MANIFEST "winLongPath.exe.manifest")
else ()
  set(MANIFEST "")
endif ()

add_library(xpdf_objs OBJECT
  AcroForm.cc
  Annot.cc
  Array.cc
  BuiltinFont.cc
  BuiltinFontTables.cc
  Catalog.cc
  CharCodeToUnicode.cc
  CMap.cc
  ${COLOR_MANAGER_SOURCE}
  Decrypt.cc
  Dict.cc
  Error.cc
  FontEncodingTables.cc
  Function.cc
  Gfx.cc
  GfxFont.cc
  GfxState.cc
  GlobalParams.cc
  JArithmeticDecoder.cc
  JBIG2Stream.cc
  JPXStream.cc
  Lexer.cc
  Link.cc
  NameToCharCode.cc
  Object.cc
  OptionalContent.cc
  Outline.cc
  OutputDev.cc
  Page.cc
  Parser.cc
  PDF417Barcode.cc
  PDFDoc.cc
  PDFDocEncoding.cc
  PSTokenizer.cc
  SecurityHandler.cc
  Stream.cc
  TextString.cc
  UnicodeMap.cc
  UnicodeRemapping.cc
  UnicodeTypeTable.cc
  UTF8.cc
  XFAScanner.cc
  XRef.cc
  Zoox.cc
)
set_property(TARGET xpdf_objs
             PROPERTY POSITION_INDEPENDENT_CODE True)

#--- pdftotext

add_executable(pdftotext
  $<TARGET_OBJECTS:xpdf_objs>
  TextOutputDev.cc
  pdftotext.cc
  ${MANIFEST}
)
target_link_libraries(pdftotext goo fofi
                      ${PAPER_LIBRARY}
                      ${LCMS_LIBRARY}
                      ${FONTCONFIG_LIBRARY}
                      ${CMAKE_THREAD_LIBS_INIT})
install(TARGETS pdftotext RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${PROJECT_SOURCE_DIR}/doc/pdftotext.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
