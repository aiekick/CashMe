#========================================================================
#
# xpdf/cmake-xpdf.txt
#
# CMake script for the Xpdf tools.
#
# Copyright 2015 Glyph & Cog, LLC
#
#========================================================================

add_library(xpdf STATIC
  AcroForm.cc
  Annot.cc
  Array.cc
  BuiltinFont.cc
  BuiltinFontTables.cc
  Catalog.cc
  CharCodeToUnicode.cc
  CMap.cc
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

