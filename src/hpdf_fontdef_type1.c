/*
 * << Haru Free PDF Library >> -- hpdf_fontdef_type1.c
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

static void
FreeWidth(HPDF_FontDef  fontdef);


static void
FreeFunc(HPDF_FontDef  fontdef);


static const char*
GetKeyword(char const *src,
   char        *keyword,
   HpdfUInt        len);

static HpdfStatus
LoadAfm(HPDF_FontDef  fontdef,
   HPDF_Stream   stream);


static HpdfStatus
LoadFontData(HPDF_FontDef  fontdef,
   HPDF_Stream   stream);


/*---------------------------------------------------------------------------*/

static void
FreeWidth(HPDF_FontDef  fontdef)
{
   HPDF_Type1FontDefAttr attr = (HPDF_Type1FontDefAttr) fontdef->attr;

   HPDF_PTRACE((" FreeWidth\n"));

   HPDF_FreeMem(fontdef->mmgr, attr->widths);
   attr->widths = NULL;

   fontdef->valid = HPDF_FALSE;
}

HPDF_FontDef
HPDF_Type1FontDef_New(HPDF_MMgr  mmgr)
{
   HPDF_FontDef fontdef;
   HPDF_Type1FontDefAttr fontdef_attr;

   HPDF_PTRACE((" HPDF_Type1FontDef_New\n"));

   if (!mmgr)
      return NULL;

   fontdef = HPDF_GetMem(mmgr, sizeof(HPDF_FontDef_Rec));
   if (!fontdef)
      return NULL;

   HPDF_MemSet(fontdef, 0, sizeof(HPDF_FontDef_Rec));
   fontdef->sig_bytes = HPDF_FONTDEF_SIG_BYTES;
   fontdef->mmgr = mmgr;
   fontdef->error = mmgr->error;
   fontdef->type = HPDF_FONTDEF_TYPE_TYPE1;
   fontdef->free_fn = FreeFunc;

   fontdef_attr = HPDF_GetMem(mmgr, sizeof(HPDF_Type1FontDefAttr_Rec));
   if (!fontdef_attr) {
      HPDF_FreeMem(fontdef->mmgr, fontdef);
      return NULL;
   }

   fontdef->attr = fontdef_attr;
   HPDF_MemSet((HpdfByte *) fontdef_attr, 0, sizeof(HPDF_Type1FontDefAttr_Rec));
   fontdef->flags = HPDF_FONT_STD_CHARSET;

   return fontdef;
}

static const char*
GetKeyword(char const *src,
   char        *keyword,
   HpdfUInt        len)
{
   HpdfUInt src_len = HPDF_StrLen(src, -1);

   HPDF_PTRACE((" GetKeyword\n"));

   if (!keyword || src_len == 0 || len == 0)
      return NULL;

   *keyword = 0;

   while (len > 1) {
      if (HPDF_IS_WHITE_SPACE(*src)) {
         *keyword = 0;

         while (HPDF_IS_WHITE_SPACE(*src))
            src++;
         return src;
      }

      *keyword++ = *src++;
      len--;
   }

   *keyword = 0;
   return NULL;
}

static HpdfStatus
LoadAfm(HPDF_FontDef  fontdef,
   HPDF_Stream   stream)
{
   HPDF_Type1FontDefAttr attr = (HPDF_Type1FontDefAttr) fontdef->attr;
   char buf[HPDF_TMP_BUF_SIZ];
   HPDF_CharData* cdata;
   HpdfStatus ret;
   HpdfUInt len;
   char keyword[HPDF_LIMIT_MAX_NAME_LEN + 1];
   HpdfUInt i;

   HPDF_PTRACE((" LoadAfm\n"));

   len = HPDF_TMP_BUF_SIZ;

   /* check AFM header */
   if ((ret = HPDF_Stream_ReadLn(stream, buf, &len)) != HPDF_OK)
      return ret;

   GetKeyword(buf, keyword, HPDF_LIMIT_MAX_NAME_LEN + 1);

   if (!HpdfStrIsEqual(keyword, "StartFontMetrics"))
   {
      return HPDF_INVALID_AFM_HEADER;
   }

   /* Global Font Information */

   for (;;) {
      const char *s;

      len = HPDF_TMP_BUF_SIZ;
      if ((ret = HPDF_Stream_ReadLn(stream, buf, &len)) != HPDF_OK)
         return ret;

      s = GetKeyword(buf, keyword, HPDF_LIMIT_MAX_NAME_LEN + 1);

      if (HpdfStrIsEqual(keyword, "FontName"))
      {
         HPDF_StrCpy(fontdef->base_font, s,
            fontdef->base_font + HPDF_LIMIT_MAX_NAME_LEN);
      }
      else
      {
         if (HpdfStrIsEqual(keyword, "Weight")) 
         {
            if (HpdfStrIsEqual(s, "Bold"))
            {
               fontdef->flags |= HPDF_FONT_FOURCE_BOLD;
            }
         }
         else
         {
            if (HpdfStrIsEqual(keyword, "IsFixedPitch")) 
            {
               if (HpdfStrIsEqual(s, "true"))
               {
                  fontdef->flags |= HPDF_FONT_FIXED_WIDTH;
               }
            }
            else
            {
               if (HpdfStrIsEqual(keyword, "ItalicAngle")) 
               {
                  fontdef->italic_angle = (HpdfInt16) HPDF_AToI(s);
                  if (fontdef->italic_angle != 0)
                  {
                     fontdef->flags |= HPDF_FONT_ITALIC;
                  }
               }
               else
               {
                  if (HpdfStrIsEqual(keyword, "CharacterSet")) 
                  {
                     HpdfUInt len1 = HPDF_StrLen(s, HPDF_LIMIT_MAX_STRING_LEN);

                     if (len1 > 0) 
                     {
                        attr->char_set = HPDF_GetMem(fontdef->mmgr, len1 + 1);
                        if (!attr->char_set)
                           return HPDF_Error_GetCode(fontdef->error);

                        HPDF_StrCpy(attr->char_set, s, attr->char_set + len1);
                     }
                  }
                  else
                  {
                     if (HpdfStrIsEqual(keyword, "FontBBox")) 
                     {
                        char buf1[HpdfInt_LEN + 1];

                        s = GetKeyword(s, buf1, HpdfInt_LEN + 1);
                        fontdef->font_bbox.left = (HpdfReal) HPDF_AToI(buf1);

                        s = GetKeyword(s, buf1, HpdfInt_LEN + 1);
                        fontdef->font_bbox.bottom = (HpdfReal) HPDF_AToI(buf1);

                        s = GetKeyword(s, buf1, HpdfInt_LEN + 1);
                        fontdef->font_bbox.right = (HpdfReal) HPDF_AToI(buf1);

                        GetKeyword(s, buf1, HpdfInt_LEN + 1);
                        fontdef->font_bbox.top = (HpdfReal) HPDF_AToI(buf1);
                     }
                     else
                     {
                        if (HpdfStrIsEqual(keyword, "EncodingScheme")) 
                        {
                           HPDF_StrCpy(attr->encoding_scheme, s,
                              attr->encoding_scheme + HPDF_LIMIT_MAX_NAME_LEN);
                        }
                        else
                        {
                           if (HpdfStrIsEqual(keyword, "CapHeight")) 
                           {
                              fontdef->cap_height = (HpdfUInt16) HPDF_AToI(s);
                           }
                           else
                           {
                              if (HpdfStrIsEqual(keyword, "Ascender")) 
                              {
                                 fontdef->ascent = (HpdfInt16) HPDF_AToI(s);
                              }
                              else 
                              {
                                 if (HpdfStrIsEqual(keyword, "Descender")) 
                                 {
                                    fontdef->descent = (HpdfInt16) HPDF_AToI(s);
                                 }
                                 else
                                 {
                                    if (HpdfStrIsEqual(keyword, "STDHW")) 
                                    {
                                       fontdef->stemh = (HpdfUInt16) HPDF_AToI(s);
                                    }
                                    else
                                    {
                                       if (HpdfStrIsEqual(keyword, "STDHV")) 
                                       {
                                          fontdef->stemv = (HpdfUInt16) HPDF_AToI(s);
                                       }
                                       else
                                       {
                                          if (HpdfStrIsEqual(keyword, "StartCharMetrics")) 
                                          {
                                             attr->widths_count = HPDF_AToI(s);
                                             break;
                                          }
                                       }
                                    }
                                 }
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }

   cdata = (HPDF_CharData*) HPDF_GetMem(fontdef->mmgr,
      sizeof(HPDF_CharData) * attr->widths_count);
   if (cdata == NULL)
      return HPDF_Error_GetCode(fontdef->error);

   HPDF_MemSet(cdata, 0, sizeof(HPDF_CharData) * attr->widths_count);
   attr->widths = cdata;

   /* load CharMetrics */
   for (i = 0; i < attr->widths_count; i++, cdata++) {
      const char *s;
      char buf2[HPDF_LIMIT_MAX_NAME_LEN + 1] = { 0 };

      len = HPDF_TMP_BUF_SIZ;
      if ((ret = HPDF_Stream_ReadLn(stream, buf, &len)) != HPDF_OK)
         return ret;

      /* C default character code. */
      s = GetKeyword(buf, buf2, HPDF_LIMIT_MAX_NAME_LEN + 1);
      if (HpdfStrIsEqual(buf2, "CX"))
      {
         /* not supported yet. */
         return HPDF_SetError(fontdef->error,
            HPDF_INVALID_CHAR_MATRICS_DATA, 0);
      }
      else
         if (HpdfStrIsEqual(buf2, "C"))
         {
            s += 2;

            s = GetKeyword(s, buf2, HPDF_LIMIT_MAX_NAME_LEN + 1);
            cdata->char_cd = (HpdfInt16) HPDF_AToI(buf2);

         }
         else
            return HPDF_SetError(fontdef->error,
               HPDF_INVALID_CHAR_MATRICS_DATA, 0);

      /* WX Character width */
      s = HPDF_StrStr(s, "WX ", 0);
      if (!s)
         return HPDF_SetError(fontdef->error, HPDF_INVALID_WX_DATA, 0);

      s += 3;

      s = GetKeyword(s, buf2, HPDF_LIMIT_MAX_NAME_LEN + 1);
      if (buf2[0] == 0)
         return HPDF_SetError(fontdef->error, HPDF_INVALID_WX_DATA, 0);

      cdata->width = (HpdfInt16) HPDF_AToI(buf2);

      /* N PostScript language character name */
      s = HPDF_StrStr(s, "N ", 0);
      if (!s)
         return HPDF_SetError(fontdef->error, HPDF_INVALID_N_DATA, 0);

      s += 2;

      GetKeyword(s, buf2, HPDF_LIMIT_MAX_NAME_LEN + 1);

      cdata->unicode = HPDF_GryphNameToUnicode(buf2);

   }

   return HPDF_OK;
}


static HpdfStatus
LoadFontData(HPDF_FontDef  fontdef,
   HPDF_Stream   stream)
{
   HPDF_Type1FontDefAttr attr = (HPDF_Type1FontDefAttr) fontdef->attr;
   char buf[HPDF_STREAM_BUF_SIZ];
   char* pbuf = buf;
   HpdfUInt len = 0;
   HpdfStatus ret;
   HpdfBool end_flg = HPDF_FALSE;

   HPDF_PTRACE((" LoadFontData\n"));

   attr->font_data = HPDF_MemStream_New(fontdef->mmgr, HPDF_STREAM_BUF_SIZ);

   if (!attr->font_data)
      return HPDF_Error_GetCode(fontdef->error);

   len = 11;
   ret = HPDF_Stream_Read(stream, (HpdfByte *) pbuf, &len);
   if (ret != HPDF_OK)
      return ret;
   pbuf += 11;

   for (;;) {
      len = HPDF_STREAM_BUF_SIZ - 11;
      ret = HPDF_Stream_Read(stream, (HpdfByte *) pbuf, &len);
      if (ret == HPDF_STREAM_EOF) {
         end_flg = HPDF_TRUE;
      }
      else if (ret != HPDF_OK)
         return ret;

      if (len > 0) {
         if (attr->length1 == 0) {
            const char *s1 = HPDF_StrStr(buf, "eexec", len + 11);

            /* length1 indicate the size of ascii-data of font-file. */
            if (s1)
               attr->length1 = (HpdfUInt) (attr->font_data->size + (s1 - buf) + 6);
         }

         if (attr->length1 > 0 && attr->length2 == 0) {
            const char *s2 = HPDF_StrStr(buf, "cleartomark",
               len + 11);

            if (s2)
               attr->length2 = (HpdfUInt) (attr->font_data->size - 520 -
                  attr->length1 + (s2 - buf));
            /*  length1 indicate the size of binary-data.
             *  in most fonts, it is all right at 520 bytes . but it need
             *  to modify because it does not fully satisfy the
             *  specification of type-1 font.
             */
         }
      }

      if (end_flg) {
         if ((ret = HPDF_Stream_Write(attr->font_data, (HpdfByte *) buf, len + 11)) !=
            HPDF_OK)
            return ret;

         break;
      }
      else {
         if ((ret = HPDF_Stream_Write(attr->font_data, (HpdfByte *) buf, len)) !=
            HPDF_OK)
            return ret;
         HPDF_MemCpy((HpdfByte *) buf, (HpdfByte *) buf + len, 11);
         pbuf = buf + 11;
      }
   }

   if (attr->length1 == 0 || attr->length2 == 0)
      return HPDF_SetError(fontdef->error, HPDF_UNSUPPORTED_TYPE1_FONT, 0);

   attr->length3 = attr->font_data->size - attr->length1 - attr->length2;

   return HPDF_OK;
}


HPDF_FontDef
HPDF_Type1FontDef_Load(HPDF_MMgr         mmgr,
   HPDF_Stream       afm,
   HPDF_Stream       font_data)
{
   HPDF_FontDef fontdef;
   HpdfStatus ret;

   HPDF_PTRACE((" HPDF_Type1FontDef_Load\n"));

   if (!afm)
      return NULL;

   fontdef = HPDF_Type1FontDef_New(mmgr);

   if (!fontdef)
      return NULL;

   ret = LoadAfm(fontdef, afm);
   if (ret != HPDF_OK) {
      HPDF_FontDef_Free(fontdef);
      return NULL;
   }

   /* if font-data is specified, the font data is embedded */
   if (font_data) {
      ret = LoadFontData(fontdef, font_data);
      if (ret != HPDF_OK) {
         HPDF_FontDef_Free(fontdef);
         return NULL;
      }
   }

   return fontdef;
}

HPDF_FontDef
HPDF_Type1FontDef_Duplicate(HPDF_MMgr     mmgr,
   HPDF_FontDef  src)
{
   HPDF_FontDef fontdef = HPDF_Type1FontDef_New(mmgr);

   HPDF_PTRACE((" HPDF_Type1FontDef_Duplicate\n"));

   fontdef->type = src->type;
   fontdef->valid = src->valid;

   /* copy data of attr,widths
    attention to charset */
   return NULL;
}

HpdfStatus
HPDF_Type1FontDef_SetWidths(HPDF_FontDef          fontdef,
   const HPDF_CharData*  widths)
{
   const HPDF_CharData* src = widths;
   HPDF_Type1FontDefAttr attr = (HPDF_Type1FontDefAttr) fontdef->attr;
   HPDF_CharData* dst;
   HpdfUInt i = 0;

   HPDF_PTRACE((" HPDF_Type1FontDef_SetWidths\n"));

   FreeWidth(fontdef);

   while (src->unicode != 0xFFFF) {
      src++;
      i++;
   }

   attr->widths_count = i;

   dst = (HPDF_CharData*) HPDF_GetMem(fontdef->mmgr, sizeof(HPDF_CharData) *
      attr->widths_count);
   if (dst == NULL)
      return HPDF_Error_GetCode(fontdef->error);

   HPDF_MemSet(dst, 0, sizeof(HPDF_CharData) * attr->widths_count);
   attr->widths = dst;

   src = widths;
   for (i = 0; i < attr->widths_count; i++) {
      dst->char_cd = src->char_cd;
      dst->unicode = src->unicode;
      dst->width = src->width;
      if (dst->unicode == 0x0020) {
         fontdef->missing_width = src->width;
      }

      src++;
      dst++;
   }

   return HPDF_OK;
}


HpdfInt16
HPDF_Type1FontDef_GetWidthByName(HPDF_FontDef      fontdef,
   const char*  gryph_name)
{
   HpdfUnicode unicode = HPDF_GryphNameToUnicode(gryph_name);

   HPDF_PTRACE((" HPDF_Type1FontDef_GetWidthByName\n"));

   return HPDF_Type1FontDef_GetWidth(fontdef, unicode);
}


HpdfInt16
HPDF_Type1FontDef_GetWidth(HPDF_FontDef  fontdef,
   HpdfUnicode  unicode)
{
   HPDF_Type1FontDefAttr attr = (HPDF_Type1FontDefAttr) fontdef->attr;
   HPDF_CharData *cdata = attr->widths;
   HpdfUInt i;

   HPDF_PTRACE((" HPDF_Type1FontDef_GetWidth\n"));

   for (i = 0; i < attr->widths_count; i++) {
      if (cdata->unicode == unicode)
         return cdata->width;
      cdata++;
   }

   return fontdef->missing_width;
}

static void
FreeFunc(HPDF_FontDef  fontdef)
{
   HPDF_Type1FontDefAttr attr = (HPDF_Type1FontDefAttr) fontdef->attr;

   HPDF_PTRACE((" FreeFunc\n"));

   if (attr->char_set)
      HPDF_FreeMem(fontdef->mmgr, attr->char_set);

   if (attr->font_data)
      HPDF_Stream_Free(attr->font_data);

   HPDF_FreeMem(fontdef->mmgr, attr->widths);
   HPDF_FreeMem(fontdef->mmgr, attr);
}

