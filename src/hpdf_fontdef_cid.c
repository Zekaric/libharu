/*
 * << Haru Free PDF Library >> -- hpdf_fontdef_cid.c
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

#include "hpdf_conf.h"
#include "hpdf_utils.h"
#include "hpdf_fontdef.h"

void
HPDF_CIDFontDef_FreeWidth(HPDF_FontDef  fontdef);


void
HPDF_CIDFontDef_FreeFunc(HPDF_FontDef  fontdef);


/*----------------------------------------------------------------------*/
/*----- HPDF_CIDFontDef ------------------------------------------------*/

void
HPDF_CIDFontDef_FreeWidth(HPDF_FontDef  fontdef)
{
   HPDF_CIDFontDefAttr attr = (HPDF_CIDFontDefAttr) fontdef->attr;
   HpdfUInt i;

   HPDF_PTRACE((" HPDF_FontDef_Validate\n"));

   for (i = 0; i < attr->widths->count; i++) {
      HPDF_CID_Width *w =
         (HPDF_CID_Width *) HPDF_List_ItemAt(attr->widths, i);

      HpdfMemDestroy(fontdef->mmgr, w);
   }

   HPDF_List_Free(attr->widths);
   attr->widths = NULL;

   fontdef->valid = HPDF_FALSE;
}


HPDF_FontDef
   HPDF_CIDFontDef_New(
      HpdfMemMgr * const mmgr,
      char              *name,
      HPDF_FontDef_InitFunc   init_fn)
{
   HPDF_FontDef fontdef;
   HPDF_CIDFontDefAttr fontdef_attr;

   HPDF_PTRACE((" HPDF_CIDFontDef_New\n"));

   if (!mmgr)
      return NULL;

   fontdef = HpdfMemCreateType(mmgr, HPDF_FontDef_Rec);
   if (!fontdef)
   {
      return NULL;
   }

   HpdfMemClearType(fontdef, HPDF_FontDef_Rec);
   fontdef->sig_bytes = HPDF_FONTDEF_SIG_BYTES;
   HPDF_StrCpy(fontdef->base_font, name, fontdef->base_font +
      HPDF_LIMIT_MAX_NAME_LEN);
   fontdef->mmgr = mmgr;
   fontdef->error = mmgr->error;
   fontdef->type = HPDF_FONTDEF_TYPE_UNINITIALIZED;
   fontdef->free_fn = HPDF_CIDFontDef_FreeFunc;
   fontdef->init_fn = init_fn;
   fontdef->valid = HPDF_FALSE;
   fontdef_attr = HpdfMemCreateType(mmgr, HPDF_CIDFontDefAttr_Rec);
   if (!fontdef_attr) 
   {
      HpdfMemDestroy(fontdef->mmgr, fontdef);
      return NULL;
   }

   fontdef->attr = fontdef_attr;
   HpdfMemClearType(fontdef_attr, HPDF_CIDFontDefAttr_Rec);

   fontdef_attr->widths = HPDF_List_New(mmgr, HPDF_DEF_CHAR_WIDTHS_NUM);
   if (!fontdef_attr->widths) {
      HpdfMemDestroy(fontdef->mmgr, fontdef);
      HpdfMemDestroy(fontdef->mmgr, fontdef_attr);
      return NULL;
   }

   fontdef->missing_width = 500;
   fontdef_attr->DW = 1000;
   fontdef_attr->DW2[0] = 880;
   fontdef_attr->DW2[1] = -1000;

   return fontdef;
}


HpdfInt16
HPDF_CIDFontDef_GetCIDWidth(HPDF_FontDef  fontdef,
   HpdfUInt16    cid)
{
   HPDF_CIDFontDefAttr attr = (HPDF_CIDFontDefAttr) fontdef->attr;
   HpdfUInt i;

   HPDF_PTRACE((" HPDF_CIDFontDef_GetCIDWidth\n"));

   for (i = 0; i < attr->widths->count; i++) {
      HPDF_CID_Width *w = (HPDF_CID_Width *) HPDF_List_ItemAt(attr->widths,
         i);

      if (w->cid == cid)
         return w->width;
   }

   /* Not found in pdf_cid_width array. */
   return attr->DW;
}

void
HPDF_CIDFontDef_FreeFunc(HPDF_FontDef  fontdef)
{
   HPDF_CIDFontDefAttr attr = (HPDF_CIDFontDefAttr) fontdef->attr;

   HPDF_PTRACE((" HPDF_CIDFontDef_FreeFunc\n"));

   HPDF_CIDFontDef_FreeWidth(fontdef);
   HpdfMemDestroy(fontdef->mmgr, attr);
}


HpdfStatus
HPDF_CIDFontDef_AddWidth(HPDF_FontDef            fontdef,
   const HPDF_CID_Width   *widths)
{
   HPDF_CIDFontDefAttr attr = (HPDF_CIDFontDefAttr) fontdef->attr;

   HPDF_PTRACE((" HPDF_CIDFontDef_AddWidth\n"));

   while (widths->cid != 0xFFFF) 
   {
      HPDF_CID_Width *w = HpdfMemCreateType(fontdef->mmgr, HPDF_CID_Width);
      HpdfStatus ret;

      if (!w)
         return fontdef->error->error_no;

      w->cid = widths->cid;
      w->width = widths->width;

      if ((ret = HPDF_List_Add(attr->widths, w)) != HPDF_OK) {
         HpdfMemDestroy(fontdef->mmgr, w);

         return ret;
      }

      widths++;
   }

   return HPDF_OK;
}


HpdfStatus
HPDF_CIDFontDef_ChangeStyle(HPDF_FontDef   fontdef,
   HpdfBool      bold,
   HpdfBool      italic)
{
   HPDF_PTRACE((" HPDF_CIDFontDef_ChangeStyle\n"));

   if (!fontdef || !fontdef->attr)
      return HPDF_INVALID_FONTDEF_DATA;

   if (bold) {
      fontdef->stemv *= 2;
      fontdef->flags |= HPDF_FONT_FOURCE_BOLD;
   }

   if (italic) {
      fontdef->italic_angle -= 11;
      fontdef->flags |= HPDF_FONT_ITALIC;
   }

   return HPDF_OK;
}



