/*
 * << Haru Free PDF Library >> -- hpdf_doc.h
 *
 * URL: http://libharu.org
 *
 * Copyright (c) 1999-2006 Takeshi Kanno <takeshi_kanno@est.hi-ho.ne.jp>
 * Copyright (c) 2007-2009 Antony Dovgal <tony@daylessday.org>
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 * It is provided "as is" without express or implied warranty.
 *
 */


#ifndef _HPDF_DOC_H
#define _HPDF_DOC_H

#define HPDF_SIG_BYTES 0x41504446L

#include "hpdf_catalog.h"
#include "hpdf_image.h"
#include "hpdf_pages.h"
#include "hpdf_outline.h"
#include "hpdf_ext_gstate.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HPDF_VER_DEFAULT  HPDF_VER_12

typedef struct _HPDF_Doc HpdfDoc;

struct _HPDF_Doc
{
   HpdfUInt32         sig_bytes;
   HPDF_PDFVer        pdf_version;

   HpdfMemMgr        *mmgr;
   HPDF_Catalog       catalog;
   HPDF_Outline       outlines;
   HPDF_Xref          xref;
   HPDF_Pages         root_pages;
   HPDF_Pages         cur_pages;
   HPDF_Page          cur_page;
   HPDF_List          page_list;
   HpdfError          error;
   HPDF_Dict          info;
   HPDF_Dict          trailer;

   HPDF_List          font_mgr;
   HpdfByte           ttfont_tag[6];

   /* list for loaded fontdefs */
   HPDF_List          fontdef_list;

   /* list for loaded encodings */
   HPDF_List          encoder_list;

   HPDF_Encoder       cur_encoder;

   /* default compression mode */
   HpdfBool           compression_mode;

   HpdfBool           encrypt_on;
   HPDF_EncryptDict   encrypt_dict;

   HPDF_Encoder       def_encoder;

   HpdfUInt           page_per_pages;
   HpdfUInt           cur_page_num;

   /* buffer for saving into memory stream */
   HPDF_Stream        stream;
};


HPDF_Encoder   HPDF_Doc_FindEncoder(         HpdfDoc const * const pdf, char const * const encoding_name);
HPDF_FontDef   HPDF_Doc_FindFontDef(         HpdfDoc const * const pdf, char const *font_name); 
HPDF_Font      HPDF_Doc_FindFont(            HpdfDoc const * const pdf, char const *font_name, char const *encoding_name);
HpdfBool      HPDF_Doc_Validate(            HpdfDoc const * const pdf);

/*----- page handling -------------------------------------------------------*/

HPDF_Pages     HPDF_Doc_GetCurrentPages(     HpdfDoc       * const pdf);
HPDF_Pages     HPDF_Doc_AddPagesTo(          HpdfDoc       * const pdf, HPDF_Pages   parent);
HpdfStatus    HPDF_Doc_SetCurrentPages(     HpdfDoc       * const pdf, HPDF_Pages  pages);
HpdfStatus    HPDF_Doc_SetCurrentPage(      HpdfDoc       * const pdf, HPDF_Page  page);

/*----- font handling -------------------------------------------------------*/

HPDF_FontDef   HPDF_GetFontDef(              HpdfDoc       * const pdf, char const *font_name);
HpdfStatus    HPDF_Doc_RegisterFontDef(     HpdfDoc       * const pdf, HPDF_FontDef   fontdef);

/*----- encoding handling ---------------------------------------------------*/

HpdfStatus    HPDF_Doc_RegisterEncoder(     HpdfDoc       * const pdf, HPDF_Encoder   encoder);

/*----- encryption ----------------------------------------------------------*/

HpdfStatus    HPDF_Doc_SetEncryptOn(        HpdfDoc       * const pdf);
HpdfStatus    HPDF_Doc_SetEncryptOff(       HpdfDoc       * const pdf);
HpdfStatus    HPDF_Doc_PrepareEncryption(   HpdfDoc       * const pdf);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _HPDF_DOC_H */
