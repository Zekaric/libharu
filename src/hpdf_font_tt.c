/*
 * << Haru Free PDF Library >> -- hpdf_font_tt.c
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
#include "hpdf_font.h"

static HpdfStatus
OnWrite(HPDF_Dict    obj,
   HPDF_Stream  stream);

static HpdfStatus
BeforeWrite(HPDF_Dict   obj);


static void
OnFree(HPDF_Dict  obj);


static HpdfInt
CharWidth(HPDF_Font  font,
   HpdfByte  code);

static HPDF_TextWidth
TextWidth(HPDF_Font         font,
   HpdfByte  const * const text,
   HpdfUInt         len);


static HpdfStatus
CreateDescriptor(HPDF_Font  font);


static HpdfUInt
MeasureText(HPDF_Font          font,
   HpdfByte   const * const text,
   HpdfUInt          len,
   HpdfReal          width,
   HpdfReal          font_size,
   HpdfReal          char_space,
   HpdfReal          word_space,
   HpdfBool          wordwrap,
   HpdfReal         *real_width);


HPDF_Font
   HPDF_TTFont_New(
      HpdfMemMgr * const mmgr,
      HPDF_FontDef     fontdef,
      HpdfEncoder * const encoder,
      HPDF_Xref        xref)
{
   HPDF_Dict font;
   HPDF_FontAttr attr;
   HPDF_TTFontDefAttr fontdef_attr;
   HPDF_BasicEncoderAttr encoder_attr;
   HpdfStatus ret = 0;

   HPDF_PTRACE((" HPDF_TTFont_New\n"));

   font = HPDF_Dict_New(mmgr);
   if (!font)
      return NULL;

   font->header.obj_class |= HPDF_OSUBCLASS_FONT;

   /* check whether the fontdef object and the encoder object is valid. */
   if (encoder->type != HPDF_ENCODER_TYPE_SINGLE_BYTE) {
      HPDF_SetError(font->error, HPDF_INVALID_ENCODER_TYPE, 0);
      return NULL;
   }

   if (fontdef->type != HPDF_FONTDEF_TYPE_TRUETYPE) {
      HPDF_SetError(font->error, HPDF_INVALID_FONTDEF_TYPE, 0);
      return NULL;
   }

   attr = HpdfMemCreateType(mmgr, HPDF_FontAttr_Rec);
   if (!attr) 
   {
      HPDF_Dict_Free(font);
      return NULL;
   }

   HpdfMemClearType(attr, HPDF_FontAttr_Rec);

   font->header.obj_class |= HPDF_OSUBCLASS_FONT;
   font->write_fn = OnWrite;
   font->before_write_fn = BeforeWrite;
   font->free_fn = OnFree;
   font->attr = attr;

   attr->type = HPDF_FONT_TRUETYPE;
   attr->writing_mode = HPDF_WMODE_HORIZONTAL;
   attr->text_width_fn = TextWidth;
   attr->measure_text_fn = MeasureText;
   attr->fontdef = fontdef;
   attr->encoder = encoder;
   attr->xref = xref;

   /* singlebyte-font has a widths-array which is an array of 256 signed
    * short integer.
    * in the case of type1-font, widths-array for all letters is made in
    * constructor. but in the case of true-type-font, the array is
    * initialized at 0, and set when the corresponding character was used
    * for the first time.
    */
   attr->widths = HpdfMemCreateTypeArray(mmgr, HpdfInt16, 256);
   if (!attr->widths) 
   {
      HPDF_Dict_Free(font);
      return NULL;
   }

   HpdfMemClearTypeArray(attr->widths, HpdfInt16, 256);

   attr->used = HpdfMemCreateTypeArray(mmgr, HpdfByte, 256);
   if (!attr->used) 
   {
      HPDF_Dict_Free(font);
      return NULL;
   }

   HpdfMemClearTypeArray(attr->used, HpdfByte, 256);

   fontdef_attr = (HPDF_TTFontDefAttr) fontdef->attr;

   ret += HPDF_Dict_AddName(font, "Type", "Font");
   ret += HPDF_Dict_AddName(font, "BaseFont", fontdef_attr->base_font);
   ret += HPDF_Dict_AddName(font, "Subtype", "TrueType");

   encoder_attr = (HPDF_BasicEncoderAttr) encoder->attr;

   ret += HPDF_Dict_AddNumber(font, "FirstChar", encoder_attr->first_char);
   ret += HPDF_Dict_AddNumber(font, "LastChar", encoder_attr->last_char);
   if (fontdef->missing_width != 0)
      ret += HPDF_Dict_AddNumber(font, "MissingWidth",
         fontdef->missing_width);

   if (ret != HPDF_OK) {
      HPDF_Dict_Free(font);
      return NULL;
   }

   if (HPDF_Xref_Add(xref, font) != HPDF_OK)
      return NULL;

   return font;
}


static HpdfStatus
CreateDescriptor(HPDF_Font  font)
{
   HPDF_FontAttr font_attr = (HPDF_FontAttr) font->attr;
   HPDF_FontDef def = font_attr->fontdef;
   HPDF_TTFontDefAttr def_attr = (HPDF_TTFontDefAttr) def->attr;

   HPDF_PTRACE((" HPDF_TTFont_CreateDescriptor\n"));

   if (!font_attr->fontdef->descriptor) {
      HPDF_Dict descriptor = HPDF_Dict_New(font->mmgr);
      HpdfStatus ret = 0;
      HpdfArray *array;

      if (!descriptor)
         return HPDF_Error_GetCode(font->error);

      ret += HPDF_Xref_Add(font_attr->xref, descriptor);
      ret += HPDF_Dict_AddName(descriptor, "Type", "FontDescriptor");
      ret += HPDF_Dict_AddNumber(descriptor, "Ascent", def->ascent);
      ret += HPDF_Dict_AddNumber(descriptor, "Descent", def->descent);
      ret += HPDF_Dict_AddNumber(descriptor, "CapHeight", def->cap_height);
      ret += HPDF_Dict_AddNumber(descriptor, "Flags", def->flags);

      array = HPDF_Box_Array_New(font->mmgr, def->font_bbox);
      ret += HPDF_Dict_Add(descriptor, "FontBBox", array);

      ret += HPDF_Dict_AddName(descriptor, "FontName", def_attr->base_font);
      ret += HPDF_Dict_AddNumber(descriptor, "ItalicAngle",
         def->italic_angle);
      ret += HPDF_Dict_AddNumber(descriptor, "StemV", def->stemv);
      ret += HPDF_Dict_AddNumber(descriptor, "XHeight", def->x_height);

      if (def_attr->char_set)
         ret += HPDF_Dict_AddName(descriptor, "CharSet",
            def_attr->char_set);

      if (ret != HPDF_OK)
         return HPDF_Error_GetCode(font->error);

      if (def_attr->embedding) {
         HPDF_Dict font_data = HPDF_DictStream_New(font->mmgr,
            font_attr->xref);

         if (!font_data)
            return HPDF_Error_GetCode(font->error);

         if (HPDF_TTFontDef_SaveFontData(font_attr->fontdef,
            font_data->stream) != HPDF_OK)
            return HPDF_Error_GetCode(font->error);

         ret += HPDF_Dict_Add(descriptor, "FontFile2", font_data);
         ret += HPDF_Dict_AddNumber(font_data, "Length1",
            def_attr->length1);
         ret += HPDF_Dict_AddNumber(font_data, "Length2", 0);
         ret += HPDF_Dict_AddNumber(font_data, "Length3", 0);

         font_data->filter = font->filter;
      }

      if (ret != HPDF_OK)
         return HPDF_Error_GetCode(font->error);

      font_attr->fontdef->descriptor = descriptor;
   }

   return HPDF_Dict_Add(font, "FontDescriptor",
      font_attr->fontdef->descriptor);
}


static HpdfInt
CharWidth(HPDF_Font  font,
   HpdfByte  code)
{
   HPDF_FontAttr attr = (HPDF_FontAttr) font->attr;

   if (attr->used[code] == 0) {
      HpdfUnicode unicode = HPDF_Encoder_ToUnicode(attr->encoder, code);

      attr->used[code] = 1;
      attr->widths[code] = HPDF_TTFontDef_GetCharWidth(attr->fontdef,
         unicode);
   }

   return attr->widths[code];
}


static HPDF_TextWidth
TextWidth(HPDF_Font         font,
   HpdfByte  const * const text,
   HpdfUInt         len)
{
   HPDF_FontAttr attr = (HPDF_FontAttr) font->attr;
   HPDF_TextWidth ret = { 0, 0, 0, 0 };
   HpdfUInt i;
   HpdfByte b = 0;

   HPDF_PTRACE((" HPDF_TTFont_TextWidth\n"));

   if (attr->widths) {
      for (i = 0; i < len; i++) {
         b = text[i];
         ret.numchars++;
         ret.width += CharWidth(font, b);

         if (HPDF_IS_WHITE_SPACE(b)) {
            ret.numspace++;
            ret.numwords++;
         }
      }
   }
   else
      HPDF_SetError(font->error, HPDF_FONT_INVALID_WIDTHS_TABLE, 0);

   /* 2006.08.19 add. */
   if (HPDF_IS_WHITE_SPACE(b))
      ; /* do nothing. */
   else
      ret.numwords++;

   return ret;
}


static HpdfUInt
MeasureText(HPDF_Font          font,
   HpdfByte   const * const text,
   HpdfUInt          len,
   HpdfReal          width,
   HpdfReal          font_size,
   HpdfReal          char_space,
   HpdfReal          word_space,
   HpdfBool          wordwrap,
   HpdfReal         *real_width)
{
   HpdfDouble w = 0;
   HpdfUInt tmp_len = 0;
   HpdfUInt i;

   HPDF_PTRACE((" HPDF_TTFont_MeasureText\n"));

   for (i = 0; i < len; i++) {
      HpdfByte b = text[i];

      if (HPDF_IS_WHITE_SPACE(b)) {
         tmp_len = i + 1;

         if (real_width)
            *real_width = (HpdfReal) w;

         w += word_space;
      }
      else if (!wordwrap) {
         tmp_len = i;

         if (real_width)
            *real_width = (HpdfReal) w;
      }

      w += (HpdfDouble) CharWidth(font, b) * font_size / 1000;

      /* 2006.08.04 break when it encountered  line feed */
      if (w > width || b == 0x0A)
         return tmp_len;

      if (i > 0)
         w += char_space;
   }

   /* all of text can be put in the specified width */
   if (real_width)
      *real_width = (HpdfReal) w;
   return len;
}


static HpdfStatus
OnWrite(HPDF_Dict    obj,
   HPDF_Stream  stream)
{
   HPDF_FontAttr attr = (HPDF_FontAttr) obj->attr;
   HPDF_BasicEncoderAttr encoder_attr =
      (HPDF_BasicEncoderAttr) attr->encoder->attr;
   HpdfUInt i;
   HpdfStatus ret;
   char buf[128];
   char *pbuf = buf;
   char *eptr = buf + 127;

   HPDF_PTRACE((" HPDF_Font_OnWrite\n"));

   /* Widths entry */
   if ((ret = HPDF_Stream_WriteEscapeName(stream, "Widths")) != HPDF_OK)
      return ret;

   if ((ret = HPDF_Stream_WriteStr(stream, " [\012")) != HPDF_OK)
      return ret;

   for (i = encoder_attr->first_char; i <= encoder_attr->last_char; i++) {

      pbuf = HPDF_IToA(pbuf, attr->widths[i], eptr);
      *pbuf++ = ' ';

      if ((i + 1) % 16 == 0) {
         HPDF_StrCpy(pbuf, "\012", eptr);
         if ((ret = HPDF_Stream_WriteStr(stream, buf)) != HPDF_OK)
            return ret;
         pbuf = buf;
      }

   }

   HPDF_StrCpy(pbuf, "]\012", eptr);

   if ((ret = HPDF_Stream_WriteStr(stream, buf)) != HPDF_OK)
      return ret;

   return attr->encoder->write_fn(attr->encoder, stream);
}

static HpdfStatus
BeforeWrite(HPDF_Dict   obj)
{
   HPDF_PTRACE((" HPDF_TTFont_BeforeWrite\n"));

   return CreateDescriptor(obj);
}

static void
OnFree(HPDF_Dict  obj)
{
   HPDF_FontAttr attr = (HPDF_FontAttr) obj->attr;

   HPDF_PTRACE((" HPDF_TTFont_OnFree\n"));

   if (attr) {
      if (attr->widths) {
         HpdfMemDestroy(obj->mmgr, attr->widths);
      }

      if (attr->used) {
         HpdfMemDestroy(obj->mmgr, attr->used);
      }

      HpdfMemDestroy(obj->mmgr, attr);
   }
}
