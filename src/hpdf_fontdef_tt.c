/*
 * << Haru Free PDF Library >> -- hpdf_fontdef_tt.c
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
 * 2006.08.24 fixed for composite glyph description.
 */

#include "hpdf_conf.h"
#include "hpdf_utils.h"
#include "hpdf_fontdef.h"


#define HPDF_TTF_MAX_MEM_SIZ    10000

#define HPDF_REQUIRED_TAGS_COUNT  13

static const char * const REQUIRED_TAGS[HPDF_REQUIRED_TAGS_COUNT] = {
    "OS/2",
    "cmap",
    "cvt ",
    "fpgm",
    "glyf",
    "head",
    "hhea",
    "hmtx",
    "loca",
    "maxp",
    "name",
    "post",
    "prep"
};


static void
FreeFunc(HPDF_FontDef  fontdef);


static HpdfStatus
LoadFontData(HPDF_FontDef  fontdef,
   HPDF_Stream   stream,
   HpdfBool     embedding,
   HpdfUInt     offset);


static HpdfStatus
LoadFontData2(HPDF_FontDef  fontdef,
   HPDF_Stream   stream,
   HpdfUInt     index,
   HpdfBool     embedding);


static void
InitAttr(HPDF_FontDef  fontdef);


static HpdfStatus
GetUINT32(HPDF_Stream   stream,
   HpdfUInt32  *value);


static HpdfStatus
GetUINT16(HPDF_Stream   stream,
   HpdfUInt16  *value);


static HpdfStatus
GetINT16(HPDF_Stream   stream,
   HpdfInt16    *value);


static HpdfStatus
WriteUINT32(HPDF_Stream   stream,
   HpdfUInt32   value);


static HpdfStatus
WriteUINT16(HPDF_Stream   stream,
   HpdfUInt16   value);


static HpdfStatus
WriteINT16(HPDF_Stream   stream,
   HpdfInt16    value);


static void
UINT32Swap(HpdfUInt32  *value);


static void
UINT16Swap(HpdfUInt16  *value);


static void
INT16Swap(HpdfInt16  *value);


static HpdfStatus
LoadTTFTable(HPDF_FontDef  fontdef);


static HpdfStatus
ParseHead(HPDF_FontDef  fontdef);


static HpdfStatus
ParseMaxp(HPDF_FontDef  fontdef);


static HpdfStatus
ParseHhea(HPDF_FontDef  fontdef);


static HpdfStatus
ParseCMap(HPDF_FontDef  fontdef);


static HpdfStatus
ParseCMAP_format0(HPDF_FontDef  fontdef,
   HpdfUInt32   offset);


static HpdfStatus
ParseCMAP_format4(HPDF_FontDef  fontdef,
   HpdfUInt32   offset);


static HpdfStatus
ParseHmtx(HPDF_FontDef  fontdef);


static HpdfStatus
ParseLoca(HPDF_FontDef  fontdef);


static HpdfStatus
LoadUnicodeName(HPDF_Stream   stream,
   HpdfUInt     offset,
   HpdfUInt     len,
   char          *buf);

static HpdfStatus
ParseName(HPDF_FontDef  fontdef);


static HpdfStatus
ParseOS2(HPDF_FontDef  fontdef);


static HPDF_TTFTable*
FindTable(HPDF_FontDef   fontdef,
   char const   *tag);


static void
CleanFunc(HPDF_FontDef   fontdef);


static HpdfStatus
CheckCompositGryph(HPDF_FontDef   fontdef,
   HpdfUInt16    gid);


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

static void
FreeFunc(HPDF_FontDef  fontdef)
{
   HPDF_TTFontDefAttr attr = (HPDF_TTFontDefAttr) fontdef->attr;

   HPDF_PTRACE((" HPDF_TTFontDef_FreeFunc\n"));

   if (attr) {
      InitAttr(fontdef);

      HpdfMemDestroy(fontdef->mmgr, attr);
   }
}


static void
CleanFunc(HPDF_FontDef   fontdef)
{
   HPDF_TTFontDefAttr attr = (HPDF_TTFontDefAttr) fontdef->attr;
   HpdfMemClearTypeArray(attr->glyph_tbl.flgs, HpdfByte, attr->num_glyphs);
   attr->glyph_tbl.flgs[0] = 1;
}


static void
InitAttr(HPDF_FontDef  fontdef)
{
   HPDF_TTFontDefAttr attr = (HPDF_TTFontDefAttr) fontdef->attr;

   if (attr) {
      if (attr->char_set)
         HpdfMemDestroy(fontdef->mmgr, attr->char_set);

      if (attr->h_metric)
         HpdfMemDestroy(fontdef->mmgr, attr->h_metric);

      if (attr->name_tbl.name_records)
         HpdfMemDestroy(fontdef->mmgr, attr->name_tbl.name_records);

      if (attr->cmap.end_count)
         HpdfMemDestroy(fontdef->mmgr, attr->cmap.end_count);

      if (attr->cmap.start_count)
         HpdfMemDestroy(fontdef->mmgr, attr->cmap.start_count);

      if (attr->cmap.id_delta)
         HpdfMemDestroy(fontdef->mmgr, attr->cmap.id_delta);

      if (attr->cmap.id_range_offset)
         HpdfMemDestroy(fontdef->mmgr, attr->cmap.id_range_offset);

      if (attr->cmap.glyph_id_array)
         HpdfMemDestroy(fontdef->mmgr, attr->cmap.glyph_id_array);

      if (attr->offset_tbl.table)
         HpdfMemDestroy(fontdef->mmgr, attr->offset_tbl.table);

      if (attr->glyph_tbl.flgs)
         HpdfMemDestroy(fontdef->mmgr, attr->glyph_tbl.flgs);

      if (attr->glyph_tbl.offsets)
         HpdfMemDestroy(fontdef->mmgr, attr->glyph_tbl.offsets);

      if (attr->stream)
         HPDF_Stream_Free(attr->stream);
   }
}


HPDF_FontDef
   HPDF_TTFontDef_New(
      HpdfMemMgr * const mmgr)
{
   HPDF_FontDef fontdef;
   HPDF_TTFontDefAttr fontdef_attr;

   HPDF_PTRACE((" HPDF_TTFontDef_New\n"));

   if (!mmgr)
      return NULL;

   fontdef = HpdfMemCreateType(mmgr, HPDF_FontDef_Rec);
   if (!fontdef)
   {
      return NULL;
   }

   HpdfMemClearType(fontdef, HPDF_FontDef_Rec);
   fontdef->sig_bytes = HPDF_FONTDEF_SIG_BYTES;
   fontdef->mmgr = mmgr;
   fontdef->error = mmgr->error;
   fontdef->type = HPDF_FONTDEF_TYPE_TRUETYPE;
   fontdef->clean_fn = CleanFunc;
   fontdef->free_fn = FreeFunc;

   fontdef_attr = HpdfMemCreateType(mmgr, HPDF_TTFontDefAttr_Rec);
   if (!fontdef_attr) 
   {
      HpdfMemDestroy(fontdef->mmgr, fontdef);
      return NULL;
   }

   fontdef->attr = fontdef_attr;
   HpdfMemClearType(fontdef_attr, HPDF_TTFontDefAttr_Rec);
   fontdef->flags = HPDF_FONT_STD_CHARSET;

   return fontdef;
}


HPDF_FontDef
   HPDF_TTFontDef_Load(
      HpdfMemMgr * const mmgr,
      HPDF_Stream   stream,
      HpdfBool     embedding)
{
   HpdfStatus ret;
   HPDF_FontDef fontdef;

   HPDF_PTRACE((" HPDF_TTFontDef_Load\n"));

   fontdef = HPDF_TTFontDef_New(mmgr);

   if (!fontdef) {
      HPDF_Stream_Free(stream);
      return NULL;
   }

   ret = LoadFontData(fontdef, stream, embedding, 0);
   if (ret != HPDF_OK) {
      HPDF_FontDef_Free(fontdef);
      return NULL;
   }

   return fontdef;
}


HPDF_FontDef
   HPDF_TTFontDef_Load2(
      HpdfMemMgr * const mmgr,
      HPDF_Stream   stream,
      HpdfUInt     index,
      HpdfBool     embedding)
{
   HpdfStatus ret;
   HPDF_FontDef fontdef;

   HPDF_PTRACE((" HPDF_TTFontDef_Load\n"));

   fontdef = HPDF_TTFontDef_New(mmgr);

   if (!fontdef) {
      HPDF_Stream_Free(stream);
      return NULL;
   }

   ret = LoadFontData2(fontdef, stream, index, embedding);
   if (ret != HPDF_OK) {
      HPDF_FontDef_Free(fontdef);
      return NULL;
   }

   return fontdef;
}


#ifdef HPDF_TTF_DEBUG
static void
DumpTable(HPDF_FontDef   fontdef)
{
   HPDF_TTFontDefAttr attr = (HPDF_TTFontDefAttr) fontdef->attr;
   HPDF_Stream  stream;
   HpdfUInt i;

   for (i = 0; i < HPDF_REQUIRED_TAGS_COUNT; i++) {
      char fname[9];
      HPDF_TTFTable *tbl = FindTable(fontdef, REQUIRED_TAGS[i]);

      if (!tbl) {
         HPDF_PTRACE((" ERR: cannot seek %s\n", fname));
         return;
      }

      HpdfMemClear(fname, 9);
      HPDF_MemCpy(fname, REQUIRED_TAGS[i], 4);
      HPDF_MemCpy(fname + 4, ".dat", 4);
      HPDF_PTRACE((" %s open\n", fname));

      if (HpdfMemIsEqual(fname, "OS/2", 4))
      {
         fname[2] = '_';
      }

      stream = HPDF_FileWriter_New(fontdef->mmgr, fname);

      if (!stream) {
         HPDF_PTRACE((" ERR: cannot open %s\n", fname));
      }
      else {
         HpdfStatus ret;
         HpdfUInt tbl_len = tbl->length;

         ret = HPDF_Stream_Seek(attr->stream, tbl->offset, HPDF_SEEK_SET);
         if (ret != HPDF_OK) {
            HPDF_PTRACE((" ERR: cannot seek \n"));
            HPDF_Stream_Free(stream);
            return;
         }

         for (;;) {
            HpdfByte buf[HPDF_STREAM_BUF_SIZ];
            HpdfUInt len = HPDF_STREAM_BUF_SIZ;

            if (len > tbl_len)
               len = tbl_len;

            HPDF_Stream_Read(attr->stream, buf, &len);
            if (len <= 0)
               break;

            ret = HPDF_Stream_Write(stream, buf, len);
            if (ret != HPDF_OK) {
               HPDF_PTRACE((" ERR: cannot write\n"));
               break;
            }

            tbl_len -= len;
            if (tbl_len == 0)
               break;
         }

         HPDF_Stream_Free(stream);
      }
   }
}
#endif

static HpdfStatus
LoadFontData(HPDF_FontDef  fontdef,
   HPDF_Stream   stream,
   HpdfBool     embedding,
   HpdfUInt     offset)
{
   HPDF_TTFontDefAttr attr = (HPDF_TTFontDefAttr) fontdef->attr;
   HpdfStatus ret;
   HPDF_TTFTable *tbl;

   HPDF_PTRACE((" HPDF_TTFontDef_LoadFontData\n"));

   attr->stream = stream;
   attr->embedding = embedding;

   if ((ret = HPDF_Stream_Seek(stream, offset, HPDF_SEEK_SET)) != HPDF_OK)
      return ret;

   if ((ret = LoadTTFTable(fontdef)) != HPDF_OK)
      return ret;

#ifdef HPDF_DUMP_FONTDATA
   DumpTable(fontdef);
#endif /* HPDF_DUMP_FONTDATA */

   if ((ret = ParseHead(fontdef)) != HPDF_OK)
      return ret;

   if ((ret = ParseMaxp(fontdef)) != HPDF_OK)
      return ret;

   if ((ret = ParseHhea(fontdef)) != HPDF_OK)
      return ret;

   if ((ret = ParseCMap(fontdef)) != HPDF_OK)
      return ret;

   if ((ret = ParseHmtx(fontdef)) != HPDF_OK)
      return ret;

   if ((ret = ParseLoca(fontdef)) != HPDF_OK)
      return ret;

   if ((ret = ParseName(fontdef)) != HPDF_OK)
      return ret;

   if ((ret = ParseOS2(fontdef)) != HPDF_OK)
      return ret;

   tbl = FindTable(fontdef, "glyf");
   if (!tbl)
      return HPDF_SetError(fontdef->error, HPDF_TTF_MISSING_TABLE, 4);

   attr->glyph_tbl.base_offset = tbl->offset;
   fontdef->cap_height =
      (HpdfUInt16) HPDF_TTFontDef_GetCharBBox(fontdef, (HpdfUInt16) 'H').top;
   fontdef->x_height =
      (HpdfUInt16) HPDF_TTFontDef_GetCharBBox(fontdef, (HpdfUInt16) 'x').top;
   fontdef->missing_width = (HpdfInt16) ((HpdfUInt32) attr->h_metric[0].advance_width * 1000 /
      attr->header.units_per_em);

   HPDF_PTRACE((" fontdef->cap_height=%d\n", fontdef->cap_height));
   HPDF_PTRACE((" fontdef->x_height=%d\n", fontdef->x_height));
   HPDF_PTRACE((" fontdef->missing_width=%d\n", fontdef->missing_width));

   if (!embedding) {
      HPDF_Stream_Free(attr->stream);
      attr->stream = NULL;
   }

   return HPDF_OK;
}

static HpdfStatus
LoadFontData2(HPDF_FontDef  fontdef,
   HPDF_Stream   stream,
   HpdfUInt     index,
   HpdfBool     embedding)
{
   HPDF_TTFontDefAttr attr = (HPDF_TTFontDefAttr) fontdef->attr;
   HpdfStatus ret;
   HpdfByte tag[4];
   HpdfUInt32 num_fonts;
   HpdfUInt32 offset;
   HpdfUInt size;

   HPDF_PTRACE((" HPDF_TTFontDef_LoadFontData2\n"));

   attr->stream = stream;
   attr->embedding = embedding;

   ret = HPDF_Stream_Seek(stream, 0, HPDF_SEEK_SET);
   if (ret != HPDF_OK)
      return ret;

   size = 4;
   if ((ret = HPDF_Stream_Read(stream, tag, &size)) != HPDF_OK)
      return ret;

   if (!HpdfMemIsEqual(tag, (HpdfByte *) "ttcf", 4))
   {
      return HPDF_SetError(fontdef->error, HPDF_INVALID_TTC_FILE, 0);
   }

   if ((ret = HPDF_Stream_Seek(stream, 8, HPDF_SEEK_SET)) != HPDF_OK)
      return ret;

   if ((ret = GetUINT32(stream, &num_fonts)) != HPDF_OK)
      return ret;

   HPDF_PTRACE((" HPDF_TTFontDef_LoadFontData2 num_fonts=%u\n",
      (HpdfUInt) num_fonts));

   if (index >= num_fonts)
      return HPDF_SetError(fontdef->error, HPDF_INVALID_TTC_INDEX, 0);

   /* read offset table for target font and set stream positioning to offset
    * value.
    */
   if ((ret = HPDF_Stream_Seek(stream, 12 + index * 4, HPDF_SEEK_SET)) !=
      HPDF_OK)
      return ret;

   if ((ret = GetUINT32(stream, &offset)) != HPDF_OK)
      return ret;

   return LoadFontData(fontdef, stream, embedding, offset);
}

HPDF_Box
HPDF_TTFontDef_GetCharBBox(HPDF_FontDef   fontdef,
   HpdfUInt16    unicode)
{
   HPDF_TTFontDefAttr attr = (HPDF_TTFontDefAttr) fontdef->attr;
   HpdfUInt16 gid = HPDF_TTFontDef_GetGlyphid(fontdef, unicode);
   HpdfStatus ret;
   HPDF_Box bbox = HPDF_ToBox(0, 0, 0, 0);
   HpdfInt16 i;
   HpdfInt m;

   if (gid == 0) {
      HPDF_PTRACE((" GetCharHeight cannot get gid char=0x%04x\n", unicode));
      return bbox;
   }

   if (attr->header.index_to_loc_format == 0)
      m = 2;
   else
      m = 1;

   ret = HPDF_Stream_Seek(attr->stream, attr->glyph_tbl.base_offset +
      attr->glyph_tbl.offsets[gid] * m + 2, HPDF_SEEK_SET);

   if (ret != HPDF_OK)
      return bbox;

   ret += GetINT16(attr->stream, &i);
   bbox.left = (HpdfReal) ((HpdfInt32) i * 1000 / attr->header.units_per_em);

   ret += GetINT16(attr->stream, &i);
   bbox.bottom = (HpdfReal) ((HpdfInt32) i * 1000 / attr->header.units_per_em);

   ret += GetINT16(attr->stream, &i);
   bbox.right = (HpdfReal) ((HpdfInt32) i * 1000 / attr->header.units_per_em);

   ret += GetINT16(attr->stream, &i);
   bbox.top = (HpdfReal) ((HpdfInt32) i * 1000 / attr->header.units_per_em);

   if (ret != HPDF_OK)
      return HPDF_ToBox(0, 0, 0, 0);

   HPDF_PTRACE((" PdfTTFontDef_GetCharBBox char=0x%04X, "
      "box=[%f,%f,%f,%f]\n", unicode, bbox.left, bbox.bottom, bbox.right,
      bbox.top));

   return bbox;
}


static HpdfStatus
GetUINT32(HPDF_Stream         stream,
   HpdfUInt32         *value)
{
   HpdfStatus ret;
   HpdfUInt size = sizeof(HpdfUInt32);

   ret = HPDF_Stream_Read(stream, (HpdfByte *) value, &size);
   if (ret != HPDF_OK) {
      *value = 0;
      return ret;
   }

   UINT32Swap(value);

   return HPDF_OK;
}


static HpdfStatus
GetUINT16(HPDF_Stream         stream,
   HpdfUInt16         *value)
{
   HpdfStatus ret;
   HpdfUInt size = sizeof(HpdfUInt16);

   ret = HPDF_Stream_Read(stream, (HpdfByte *) value, &size);
   if (ret != HPDF_OK) {
      *value = 0;
      return ret;
   }

   UINT16Swap(value);

   return HPDF_OK;
}


static HpdfStatus
GetINT16(HPDF_Stream    stream,
   HpdfInt16    *value)
{
   HpdfStatus ret;
   HpdfUInt size = sizeof(HpdfInt16);

   ret = HPDF_Stream_Read(stream, (HpdfByte *) value, &size);
   if (ret != HPDF_OK) {
      *value = 0;
      return ret;
   }

   INT16Swap(value);

   return HPDF_OK;
}

static HpdfStatus
WriteUINT32(HPDF_Stream   stream,
   HpdfUInt32   value)
{
   HpdfStatus ret;
   HpdfUInt32 tmp = value;

   UINT32Swap(&tmp);

   ret = HPDF_Stream_Write(stream, (HpdfByte *) &tmp, sizeof(tmp));
   if (ret != HPDF_OK)
      return ret;

   return HPDF_OK;
}


static HpdfStatus
WriteUINT16(HPDF_Stream   stream,
   HpdfUInt16   value)
{
   HpdfStatus ret;
   HpdfUInt16 tmp = value;

   UINT16Swap(&tmp);

   ret = HPDF_Stream_Write(stream, (HpdfByte *) &tmp, sizeof(tmp));
   if (ret != HPDF_OK)
      return ret;

   return HPDF_OK;
}


static HpdfStatus
WriteINT16(HPDF_Stream   stream,
   HpdfInt16    value)
{
   HpdfStatus ret;
   HpdfInt16 tmp = value;

   INT16Swap(&tmp);

   ret = HPDF_Stream_Write(stream, (HpdfByte *) &tmp, sizeof(tmp));
   if (ret != HPDF_OK)
      return ret;

   return HPDF_OK;
}


HpdfStatus
LoadTTFTable(HPDF_FontDef  fontdef)
{
   HPDF_TTFontDefAttr attr = (HPDF_TTFontDefAttr) fontdef->attr;
   HpdfStatus ret = HPDF_OK;
   HpdfInt i;
   HPDF_TTFTable *tbl;

   HPDF_PTRACE((" HPDF_TTFontDef_LoadTTFTable\n"));

   ret += GetUINT32(attr->stream, &attr->offset_tbl.sfnt_version);
   ret += GetUINT16(attr->stream, &attr->offset_tbl.num_tables);
   ret += GetUINT16(attr->stream, &attr->offset_tbl.search_range);
   ret += GetUINT16(attr->stream, &attr->offset_tbl.entry_selector);
   ret += GetUINT16(attr->stream, &attr->offset_tbl.range_shift);

   if (ret != HPDF_OK)
      return HPDF_Error_GetCode(fontdef->error);

   if (attr->offset_tbl.num_tables * sizeof(HPDF_TTFTable) >
      HPDF_TTF_MAX_MEM_SIZ)
      return HPDF_SetError(fontdef->error, HPDF_TTF_INVALID_FOMAT, 0);

   attr->offset_tbl.table = HpdfMemCreateTypeArray(
      fontdef->mmgr, 
      HPDF_TTFTable, 
      attr->offset_tbl.num_tables);
   if (!attr->offset_tbl.table)
   {
      return HPDF_Error_GetCode(fontdef->error);
   }

   tbl = attr->offset_tbl.table;
   for (i = 0; i < attr->offset_tbl.num_tables; i++) {
      HpdfUInt siz = 4;

      ret += HPDF_Stream_Read(attr->stream, (HpdfByte *) tbl->tag, &siz);
      ret += GetUINT32(attr->stream, &tbl->check_sum);
      ret += GetUINT32(attr->stream, &tbl->offset);
      ret += GetUINT32(attr->stream, &tbl->length);

      HPDF_PTRACE((" [%d] tag=[%c%c%c%c] check_sum=%u offset=%u length=%u\n",
         i, tbl->tag[0], tbl->tag[1], tbl->tag[2], tbl->tag[3],
         (HpdfUInt) tbl->check_sum, (HpdfUInt) tbl->offset,
         (HpdfUInt) tbl->length));

      if (ret != HPDF_OK)
         return HPDF_Error_GetCode(fontdef->error);;

      tbl++;
   }

   return HPDF_OK;
}


static HpdfStatus
ParseHead(HPDF_FontDef  fontdef)
{
   HPDF_TTFontDefAttr attr = (HPDF_TTFontDefAttr) fontdef->attr;
   HPDF_TTFTable *tbl = FindTable(fontdef, "head");
   HpdfStatus ret;
   HpdfUInt siz;

   HPDF_PTRACE((" HPDF_TTFontDef_ParseHead\n"));

   if (!tbl)
      return HPDF_SetError(fontdef->error, HPDF_TTF_MISSING_TABLE, 5);

   ret = HPDF_Stream_Seek(attr->stream, tbl->offset, HPDF_SEEK_SET);
   if (ret != HPDF_OK)
      return ret;

   siz = 4;
   ret += HPDF_Stream_Read(attr->stream,
      (HpdfByte *) &attr->header.version_number, &siz);
   ret += GetUINT32(attr->stream, &attr->header.font_revision);
   ret += GetUINT32(attr->stream, &attr->header.check_sum_adjustment);
   ret += GetUINT32(attr->stream, &attr->header.magic_number);
   ret += GetUINT16(attr->stream, &attr->header.flags);
   ret += GetUINT16(attr->stream, &attr->header.units_per_em);

   siz = 8;
   ret += HPDF_Stream_Read(attr->stream, attr->header.created, &siz);
   siz = 8;
   ret += HPDF_Stream_Read(attr->stream, attr->header.modified, &siz);

   ret += GetINT16(attr->stream, &attr->header.x_min);
   ret += GetINT16(attr->stream, &attr->header.y_min);
   ret += GetINT16(attr->stream, &attr->header.x_max);
   ret += GetINT16(attr->stream, &attr->header.y_max);
   ret += GetUINT16(attr->stream, &attr->header.mac_style);
   ret += GetUINT16(attr->stream, &attr->header.lowest_rec_ppem);
   ret += GetINT16(attr->stream, &attr->header.font_direction_hint);
   ret += GetINT16(attr->stream, &attr->header.index_to_loc_format);
   ret += GetINT16(attr->stream, &attr->header.glyph_data_format);

   if (ret != HPDF_OK)
      return HPDF_Error_GetCode(fontdef->error);

   fontdef->font_bbox.left = (HpdfReal) ((HpdfInt32) attr->header.x_min * 1000 /
      attr->header.units_per_em);
   fontdef->font_bbox.bottom = (HpdfReal) ((HpdfInt32) attr->header.y_min * 1000 /
      attr->header.units_per_em);
   fontdef->font_bbox.right = (HpdfReal) ((HpdfInt32) attr->header.x_max * 1000 /
      attr->header.units_per_em);
   fontdef->font_bbox.top = (HpdfReal) ((HpdfInt32) attr->header.y_max * 1000 /
      attr->header.units_per_em);

   return HPDF_OK;
}


static HpdfStatus
ParseMaxp(HPDF_FontDef  fontdef)
{
   HPDF_TTFontDefAttr attr = (HPDF_TTFontDefAttr) fontdef->attr;
   HPDF_TTFTable *tbl = FindTable(fontdef, "maxp");
   HpdfStatus ret;

   HPDF_PTRACE((" HPDF_TTFontDef_ParseMaxp\n"));

   if (!tbl)
      return HPDF_SetError(fontdef->error, HPDF_TTF_MISSING_TABLE, 9);

   ret = HPDF_Stream_Seek(attr->stream, tbl->offset + 4, HPDF_SEEK_SET);
   if (ret != HPDF_OK)
      return ret;

   ret = GetUINT16(attr->stream, &attr->num_glyphs);

   HPDF_PTRACE((" HPDF_TTFontDef_ParseMaxp num_glyphs=%u\n",
      attr->num_glyphs));

   return ret;
}


static HpdfStatus
ParseHhea(HPDF_FontDef  fontdef)
{
   HPDF_TTFontDefAttr attr = (HPDF_TTFontDefAttr) fontdef->attr;
   HPDF_TTFTable *tbl = FindTable(fontdef, "hhea");
   HpdfStatus ret;

   HPDF_PTRACE((" HPDF_TTFontDef_ParseHhea\n"));

   if (!tbl)
      return HPDF_SetError(fontdef->error, HPDF_TTF_MISSING_TABLE, 6);

   ret = HPDF_Stream_Seek(attr->stream, tbl->offset + 4, HPDF_SEEK_SET);
   if (ret != HPDF_OK)
      return ret;

   ret += GetINT16(attr->stream, &fontdef->ascent);
   fontdef->ascent = (HpdfInt16) ((HpdfInt32) fontdef->ascent * 1000 /
      attr->header.units_per_em);
   ret += GetINT16(attr->stream, &fontdef->descent);
   fontdef->descent = (HpdfInt16) ((HpdfInt32) fontdef->descent * 1000 /
      attr->header.units_per_em);

   if (ret != HPDF_OK)
      return HPDF_Error_GetCode(fontdef->error);

   ret = HPDF_Stream_Seek(attr->stream, tbl->offset + 34, HPDF_SEEK_SET);
   if (ret != HPDF_OK)
      return ret;

   ret = GetUINT16(attr->stream, &attr->num_h_metric);
   if (ret != HPDF_OK)
      return HPDF_Error_GetCode(fontdef->error);

   HPDF_PTRACE((" HPDF_TTFontDef_ParseHhea num_h_metric=%u\n",
      attr->num_h_metric));

   return ret;
}


static HpdfStatus
ParseCMap(HPDF_FontDef  fontdef)
{
   HPDF_TTFontDefAttr attr = (HPDF_TTFontDefAttr) fontdef->attr;
   HPDF_TTFTable *tbl = FindTable(fontdef, "cmap");
   HpdfStatus ret;
   HpdfUInt16 version;
   HpdfUInt16 num_cmap;
   HpdfUInt i;
   HpdfUInt32 ms_unicode_encoding_offset = 0;
   HpdfUInt32 byte_encoding_offset = 0;

   HPDF_PTRACE((" HPDF_TTFontDef_ParseCMap\n"));

   if (!tbl)
      return HPDF_SetError(fontdef->error, HPDF_TTF_MISSING_TABLE, 1);

   ret = HPDF_Stream_Seek(attr->stream, tbl->offset, HPDF_SEEK_SET);
   if (ret != HPDF_OK)
      return ret;

   ret += GetUINT16(attr->stream, &version);
   if (ret != HPDF_OK)
      return HPDF_Error_GetCode(fontdef->error);

   if (version != 0)
      return HPDF_SetError(fontdef->error, HPDF_TTF_INVALID_FOMAT, 0);

   ret += GetUINT16(attr->stream, &num_cmap);
   if (ret != HPDF_OK)
      return HPDF_Error_GetCode(fontdef->error);

   for (i = 0; i < num_cmap; i++) {
      HpdfUInt16 platformID;
      HpdfUInt16 encodingID;
      HpdfUInt16 format;
      HpdfUInt32 offset;
      HpdfInt32 save_offset;

      ret += GetUINT16(attr->stream, &platformID);
      ret += GetUINT16(attr->stream, &encodingID);
      ret += GetUINT32(attr->stream, &offset);
      if (ret != HPDF_OK)
         return HPDF_Error_GetCode(fontdef->error);

      save_offset = HPDF_Stream_Tell(attr->stream);
      if (save_offset < 0)
         return HPDF_Error_GetCode(fontdef->error);

      ret = HPDF_Stream_Seek(attr->stream, tbl->offset + offset,
         HPDF_SEEK_SET);
      if (ret != HPDF_OK)
         return ret;

      ret = GetUINT16(attr->stream, &format);
      if (ret != HPDF_OK)
         return ret;

      HPDF_PTRACE((" HPDF_TTFontDef_ParseCMap tables[%d] platformID=%u "
         "encodingID=%u format=%u offset=%u\n", i, platformID,
         encodingID, format, (HpdfUInt) offset));

      /* MS-Unicode-CMAP is used for priority */
      if (platformID == 3 && encodingID == 1 && format == 4) {
         ms_unicode_encoding_offset = offset;
         break;
      }

      /* Byte-Encoding-CMAP will be used if MS-Unicode-CMAP is not found */
      if (platformID == 1 && encodingID ==0 && format == 1)
         byte_encoding_offset = offset;

      ret = HPDF_Stream_Seek(attr->stream, save_offset, HPDF_SEEK_SET);
      if (ret != HPDF_OK)
         return ret;
   }

   if (ms_unicode_encoding_offset != 0) {
      HPDF_PTRACE((" found microsoft unicode cmap.\n"));
      ret = ParseCMAP_format4(fontdef, ms_unicode_encoding_offset +
         tbl->offset);
   }
   else if (byte_encoding_offset != 0) {
      HPDF_PTRACE((" found byte encoding cmap.\n"));
      ret = ParseCMAP_format0(fontdef, byte_encoding_offset + tbl->offset);
   }
   else {
      HPDF_PTRACE((" cannot found target cmap.\n"));
      return HPDF_SetError(fontdef->error, HPDF_TTF_INVALID_FOMAT, 0);
   }

   return ret;
}


static HpdfStatus
ParseCMAP_format0(HPDF_FontDef  fontdef,
   HpdfUInt32   offset)
{
   HPDF_TTFontDefAttr attr = (HPDF_TTFontDefAttr) fontdef->attr;
   HpdfStatus ret;
   HpdfByte array[256];
   HpdfUInt size;
   HpdfUInt16 *parray;
   HpdfUInt i;

   HPDF_PTRACE((" ParseCMAP_format0\n"));

   ret = HPDF_Stream_Seek(attr->stream, offset, HPDF_SEEK_SET);
   if (ret != HPDF_OK)
      return ret;

   ret += GetUINT16(attr->stream, &attr->cmap.format);
   ret += GetUINT16(attr->stream, &attr->cmap.length);
   ret += GetUINT16(attr->stream, &attr->cmap.language);

   if (ret != HPDF_OK)
      return HPDF_Error_GetCode(fontdef->error);

   if (attr->cmap.format != 0)
      return HPDF_SetError(fontdef->error, HPDF_TTF_INVALID_FOMAT, 0);

   size = 256;
   ret = HPDF_Stream_Read(attr->stream, array, &size);
   if (ret != HPDF_OK)
      return ret;

   attr->cmap.glyph_id_array_count = 256;
   attr->cmap.glyph_id_array = HpdfMemCreateTypeArray(fontdef->mmgr, HpdfUInt16, 256);
   if (!attr->cmap.glyph_id_array)
   {
      return HPDF_Error_GetCode(fontdef->error);
   }

   parray = attr->cmap.glyph_id_array;
   for (i = 0; i < 256; i++) {
      *parray = attr->cmap.glyph_id_array[i];
      HPDF_PTRACE((" ParseCMAP_format0 glyph_id_array[%d]=%u\n",
         i, *parray));
      parray++;
   }

   return HPDF_OK;
}


static HpdfStatus
ParseCMAP_format4(HPDF_FontDef  fontdef,
   HpdfUInt32   offset)
{
   HPDF_TTFontDefAttr attr = (HPDF_TTFontDefAttr) fontdef->attr;
   HpdfStatus ret;
   HpdfUInt i;
   HpdfUInt16 *pend_count;
   HpdfUInt16 *pstart_count;
   HpdfInt16 *pid_delta;
   HpdfUInt16 *pid_range_offset;
   HpdfUInt16 *pglyph_id_array;
   HpdfInt32 num_read;

   HPDF_PTRACE((" ParseCMAP_format4\n"));

   if ((ret = HPDF_Stream_Seek(attr->stream, offset, HPDF_SEEK_SET)) !=
      HPDF_OK)
      return ret;

   ret += GetUINT16(attr->stream, &attr->cmap.format);
   ret += GetUINT16(attr->stream, &attr->cmap.length);
   ret += GetUINT16(attr->stream, &attr->cmap.language);

   if (ret != HPDF_OK)
      return HPDF_Error_GetCode(fontdef->error);

   if (attr->cmap.format != 4)
      return HPDF_SetError(fontdef->error, HPDF_TTF_INVALID_FOMAT, 0);

   ret += GetUINT16(attr->stream, &attr->cmap.seg_count_x2);
   ret += GetUINT16(attr->stream, &attr->cmap.search_range);
   ret += GetUINT16(attr->stream, &attr->cmap.entry_selector);
   ret += GetUINT16(attr->stream, &attr->cmap.range_shift);

   if (ret != HPDF_OK)
      return HPDF_Error_GetCode(fontdef->error);

   /* end_count */
   attr->cmap.end_count = HpdfMemCreateTypeArray(
      fontdef->mmgr, 
      HpdfUInt16, 
      attr->cmap.seg_count_x2 / 2);
   if (!attr->cmap.end_count)
   {
      return HPDF_Error_GetCode(fontdef->error);
   }

   pend_count = attr->cmap.end_count;
   for (i = 0; i < (HpdfUInt) attr->cmap.seg_count_x2 / 2; i++)
      if ((ret = GetUINT16(attr->stream, pend_count++)) != HPDF_OK)
         return ret;

   if ((ret = GetUINT16(attr->stream, &attr->cmap.reserved_pad)) != HPDF_OK)
      return ret;

   /* start_count */
   attr->cmap.start_count = HpdfMemCreateTypeArray(
      fontdef->mmgr, 
      HpdfUInt16, 
      attr->cmap.seg_count_x2 / 2);
   if (!attr->cmap.start_count)
   {
      return HPDF_Error_GetCode(fontdef->error);
   }

   pstart_count = attr->cmap.start_count;
   for (i = 0; i < (HpdfUInt) attr->cmap.seg_count_x2 / 2; i++)
      if ((ret = GetUINT16(attr->stream, pstart_count++)) != HPDF_OK)
         return ret;

   /* id_delta */
   attr->cmap.id_delta = HpdfMemCreateTypeArray(
      fontdef->mmgr,
      HpdfInt16,
      attr->cmap.seg_count_x2 / 2);
   if (!attr->cmap.id_delta)
   {
      return HPDF_Error_GetCode(fontdef->error);
   }

   pid_delta = attr->cmap.id_delta;
   for (i = 0; i < (HpdfUInt) attr->cmap.seg_count_x2 / 2; i++)
      if ((ret = GetINT16(attr->stream, pid_delta++)) != HPDF_OK)
         return ret;

   /* id_range_offset */
   attr->cmap.id_range_offset = HpdfMemCreateTypeArray(
      fontdef->mmgr,
      HpdfUInt16,
      attr->cmap.seg_count_x2 / 2);
   if (!attr->cmap.id_range_offset)
   {
      return HPDF_Error_GetCode(fontdef->error);
   }

   pid_range_offset = attr->cmap.id_range_offset;
   for (i = 0; i < (HpdfUInt) attr->cmap.seg_count_x2 / 2; i++)
      if ((ret = GetUINT16(attr->stream, pid_range_offset++)) != HPDF_OK)
         return ret;

   num_read = HPDF_Stream_Tell(attr->stream) - offset;
   if (num_read < 0)
      return HPDF_Error_GetCode(fontdef->error);

   attr->cmap.glyph_id_array_count = (attr->cmap.length - num_read) / 2;

   if (attr->cmap.glyph_id_array_count > 0) {
      /* glyph_id_array */
      attr->cmap.glyph_id_array = HpdfMemCreateTypeArray(
         fontdef->mmgr,
         HpdfUInt16,
         attr->cmap.glyph_id_array_count);
      if (!attr->cmap.glyph_id_array)
      {
         return HPDF_Error_GetCode(fontdef->error);
      }

      pglyph_id_array = attr->cmap.glyph_id_array;
      for (i = 0; i < attr->cmap.glyph_id_array_count; i++)
         if ((ret = GetUINT16(attr->stream, pglyph_id_array++)) != HPDF_OK)
            return ret;
   }
   else
      attr->cmap.glyph_id_array = NULL;

#ifdef LIBHPDF_DEBUG
   /* print all elements of cmap table */
   for (i = 0; i < (HpdfUInt) attr->cmap.seg_count_x2 / 2; i++) {
      HPDF_PTRACE((" ParseCMAP_format4[%d] start_count=0x%04X, "
         "end_count=0x%04X, id_delta=%d, id_range_offset=%u\n", i,
         attr->cmap.start_count[i], attr->cmap.end_count[i],
         attr->cmap.id_delta[i], attr->cmap.id_range_offset[i]));
   }
#endif

   return HPDF_OK;
}


HpdfUInt16
HPDF_TTFontDef_GetGlyphid(HPDF_FontDef   fontdef,
   HpdfUInt16    unicode)
{
   HPDF_TTFontDefAttr attr = (HPDF_TTFontDefAttr) fontdef->attr;
   HpdfUInt16 *pend_count = attr->cmap.end_count;
   HpdfUInt seg_count = attr->cmap.seg_count_x2 / 2;
   HpdfUInt i;

   HPDF_PTRACE((" HPDF_TTFontDef_GetGlyphid\n"));

   /* format 0 */
   if (attr->cmap.format == 0) {
      unicode &= 0xFF;
      return attr->cmap.glyph_id_array[unicode];
   }

   /* format 4 */
   if (attr->cmap.seg_count_x2 == 0) {
      HPDF_SetError(fontdef->error, HPDF_TTF_INVALID_CMAP, 0);
      return 0;
   }

   for (i = 0; i < seg_count; i++) {
      if (unicode <= *pend_count)
         break;
      pend_count++;
   }

   if (attr->cmap.start_count[i] > unicode) {
      HPDF_PTRACE((" HPDF_TTFontDef_GetGlyphid undefined char(0x%04X)\n",
         unicode));
      return 0;
   }

   if (attr->cmap.id_range_offset[i] == 0) {
      HPDF_PTRACE((" HPDF_TTFontDef_GetGlyphid idx=%u code=%u "
         " ret=%u\n", i, unicode,
         unicode + attr->cmap.id_delta[i]));

      return (HpdfUInt16) (unicode + attr->cmap.id_delta[i]);
   }
   else {
      HpdfUInt idx = attr->cmap.id_range_offset[i] / 2 +
         (unicode - attr->cmap.start_count[i]) - (seg_count - i);

      if (idx > attr->cmap.glyph_id_array_count) {
         HPDF_PTRACE((" HPDF_TTFontDef_GetGlyphid[%u] %u > %u\n",
            i, idx, (HpdfUInt) attr->cmap.glyph_id_array_count));
         return 0;
      }
      else {
         HpdfUInt16 gid = (HpdfUInt16) (attr->cmap.glyph_id_array[idx] +
            attr->cmap.id_delta[i]);
         HPDF_PTRACE((" HPDF_TTFontDef_GetGlyphid idx=%u unicode=0x%04X "
            "id=%u\n", idx, unicode, gid));
         return gid;
      }
   }
}


HpdfInt16
HPDF_TTFontDef_GetCharWidth(HPDF_FontDef   fontdef,
   HpdfUInt16    unicode)
{
   HpdfUInt16 advance_width;
   HPDF_TTF_LongHorMetric hmetrics;
   HPDF_TTFontDefAttr attr = (HPDF_TTFontDefAttr) fontdef->attr;
   HpdfUInt16 gid = HPDF_TTFontDef_GetGlyphid(fontdef, unicode);

   HPDF_PTRACE((" HPDF_TTFontDef_GetCharWidth\n"));

   if (gid >= attr->num_glyphs) {
      HPDF_PTRACE((" HPDF_TTFontDef_GetCharWidth WARNING gid > "
         "num_glyphs %u > %u\n", gid, attr->num_glyphs));
      return fontdef->missing_width;
   }

   hmetrics = attr->h_metric[gid];

   if (!attr->glyph_tbl.flgs[gid]) {
      attr->glyph_tbl.flgs[gid] = 1;

      if (attr->embedding)
         CheckCompositGryph(fontdef, gid);
   }

   advance_width = (HpdfUInt16) ((HpdfUInt) hmetrics.advance_width * 1000 /
      attr->header.units_per_em);

   return (HpdfInt16) advance_width;
}


static HpdfStatus
CheckCompositGryph(HPDF_FontDef   fontdef,
   HpdfUInt16    gid)
{
   HPDF_TTFontDefAttr attr = (HPDF_TTFontDefAttr) fontdef->attr;
   HpdfUInt offset = attr->glyph_tbl.offsets[gid];
   /* HpdfUInt len = attr->glyph_tbl.offsets[gid + 1] - offset; */
   HpdfStatus ret;

   HPDF_PTRACE((" CheckCompositGryph\n"));

   if (attr->header.index_to_loc_format == 0)
      offset *= 2;

   offset += attr->glyph_tbl.base_offset;

   if ((ret = HPDF_Stream_Seek(attr->stream, offset, HPDF_SEEK_SET))
      != HPDF_OK) {
      return ret;
   }
   else {
      HpdfInt16 num_of_contours;
      HpdfInt16 flags;
      HpdfUInt16 glyph_index;
      const HpdfUInt16 ARG_1_AND_2_ARE_WORDS = 1;
      const HpdfUInt16 WE_HAVE_A_SCALE  = 8;
      const HpdfUInt16 MORE_COMPONENTS = 32;
      const HpdfUInt16 WE_HAVE_AN_X_AND_Y_SCALE = 64;
      const HpdfUInt16 WE_HAVE_A_TWO_BY_TWO = 128;

      if ((ret = GetINT16(attr->stream, &num_of_contours)) != HPDF_OK)
         return ret;

      if (num_of_contours != -1)
         return HPDF_OK;

      HPDF_PTRACE((" CheckCompositGryph composite font gid=%u\n", gid));

      if ((ret = HPDF_Stream_Seek(attr->stream, 8, HPDF_SEEK_CUR))
         != HPDF_OK)
         return ret;

      do {
         if ((ret = GetINT16(attr->stream, &flags)) != HPDF_OK)
            return ret;

         if ((ret = GetUINT16(attr->stream, &glyph_index)) != HPDF_OK)
            return ret;

         if (flags & ARG_1_AND_2_ARE_WORDS) {
            if ((ret = HPDF_Stream_Seek(attr->stream, 4, HPDF_SEEK_CUR))
               != HPDF_OK)
               return ret;
         }
         else {
            if ((ret = HPDF_Stream_Seek(attr->stream, 2, HPDF_SEEK_CUR))
               != HPDF_OK)
               return ret;
         }

         if (flags & WE_HAVE_A_SCALE) {
            if ((ret = HPDF_Stream_Seek(attr->stream, 2, HPDF_SEEK_CUR))
               != HPDF_OK)
               return ret;
         }
         else if (flags & WE_HAVE_AN_X_AND_Y_SCALE) {
            if ((ret = HPDF_Stream_Seek(attr->stream, 4, HPDF_SEEK_CUR))
               != HPDF_OK)
               return ret;
         }
         else if (flags & WE_HAVE_A_TWO_BY_TWO) {
            if ((ret = HPDF_Stream_Seek(attr->stream, 8, HPDF_SEEK_CUR))
               != HPDF_OK)
               return ret;
         }

         if (glyph_index < attr->num_glyphs &&
            !attr->glyph_tbl.flgs[glyph_index]) {
            HpdfInt32 next_glyph;

            attr->glyph_tbl.flgs[glyph_index] = 1;
            next_glyph = HPDF_Stream_Tell(attr->stream);
            CheckCompositGryph(fontdef, glyph_index);
            HPDF_Stream_Seek(attr->stream, next_glyph, HPDF_SEEK_SET);
         }

         HPDF_PTRACE((" gid=%d, num_of_contours=%d, flags=%d, "
            "glyph_index=%d\n", gid, num_of_contours, flags,
            glyph_index));

      } while (flags & MORE_COMPONENTS);
   }

   return HPDF_OK;
}


HpdfInt16
HPDF_TTFontDef_GetGidWidth(HPDF_FontDef   fontdef,
   HpdfUInt16    gid)
{
   HpdfUInt16 advance_width;
   HPDF_TTF_LongHorMetric hmetrics;
   HPDF_TTFontDefAttr attr = (HPDF_TTFontDefAttr) fontdef->attr;

   HPDF_PTRACE((" HPDF_TTFontDef_GetGidWidth\n"));

   if (gid >= attr->num_glyphs) {
      HPDF_PTRACE((" HPDF_TTFontDef_GetGidWidth WARNING gid > "
         "num_glyphs %u > %u\n", gid, attr->num_glyphs));
      return fontdef->missing_width;
   }

   hmetrics = attr->h_metric[gid];

   advance_width = (HpdfUInt16) ((HpdfUInt) hmetrics.advance_width * 1000 /
      attr->header.units_per_em);

   HPDF_PTRACE((" HPDF_TTFontDef_GetGidWidth gid=%u, width=%u\n",
      gid, advance_width));

   return (HpdfInt16) advance_width;
}



static HpdfStatus
ParseHmtx(HPDF_FontDef  fontdef)
{
   HPDF_TTFontDefAttr attr = (HPDF_TTFontDefAttr) fontdef->attr;
   HPDF_TTFTable *tbl = FindTable(fontdef, "hmtx");
   HpdfStatus ret;
   HpdfUInt i;
   HpdfUInt16 save_aw = 0;
   HPDF_TTF_LongHorMetric *pmetric;

   HPDF_PTRACE((" HPDF_TTFontDef_ParseHtmx\n"));

   if (!tbl)
      return HPDF_SetError(fontdef->error, HPDF_TTF_MISSING_TABLE, 7);

   ret = HPDF_Stream_Seek(attr->stream, tbl->offset, HPDF_SEEK_SET);
   if (ret != HPDF_OK)
      return ret;

   /* allocate memory for a table of holizontal matrix.
    * the count of metric records is same as the number of glyphs
    */
   attr->h_metric = HpdfMemCreateTypeArray(fontdef->mmgr, HPDF_TTF_LongHorMetric, attr->num_glyphs);
   if (!attr->h_metric)
   {
      return HPDF_Error_GetCode(fontdef->error);
   }

   pmetric = attr->h_metric;
   for (i = 0; i < attr->num_h_metric; i++) {
      if ((ret = GetUINT16(attr->stream, &pmetric->advance_width)) !=
         HPDF_OK)
         return ret;

      if ((ret = GetINT16(attr->stream, &pmetric->lsb)) != HPDF_OK)
         return ret;

      HPDF_PTRACE((" ParseHmtx metric[%u] aw=%u lsb=%d\n", i,
         pmetric->advance_width, pmetric->lsb));

      save_aw = pmetric->advance_width;
      pmetric++;
   }

   /* pad the advance_width of remaining metrics with the value of last metric */
   while (i < attr->num_glyphs) {
      pmetric->advance_width = save_aw;

      if ((ret = GetINT16(attr->stream, &pmetric->lsb)) != HPDF_OK)
         return ret;

      pmetric++;
      i++;
   }

   return HPDF_OK;
}

static HpdfStatus
ParseLoca(HPDF_FontDef  fontdef)
{
   HPDF_TTFontDefAttr attr = (HPDF_TTFontDefAttr) fontdef->attr;
   HPDF_TTFTable *tbl = FindTable(fontdef, "loca");
   HpdfStatus ret;
   HpdfUInt i;
   HpdfUInt32 *poffset;

   HPDF_PTRACE((" HPDF_TTFontDef_ParseLoca\n"));

   if (!tbl)
      return HPDF_SetError(fontdef->error, HPDF_TTF_MISSING_TABLE, 8);

   ret = HPDF_Stream_Seek(attr->stream, tbl->offset, HPDF_SEEK_SET);
   if (ret != HPDF_OK)
      return ret;

   /* allocate glyph-offset-table. */
   attr->glyph_tbl.offsets = HpdfMemCreateTypeArray(fontdef->mmgr, HpdfUInt32, attr->num_glyphs + 1);
   if (!attr->glyph_tbl.offsets)
   {
      return HPDF_Error_GetCode(fontdef->error);
   }

   HpdfMemClearTypeArray(attr->glyph_tbl.offsets, HpdfUInt32, attr->num_glyphs + 1);

   /* allocate glyph-flg-table.
    * this flgs are used to judge whether glyphs should be embedded.
    */
   attr->glyph_tbl.flgs = HpdfMemCreateTypeArray(fontdef->mmgr, HpdfByte, attr->num_glyphs);
   if (!attr->glyph_tbl.flgs)
   {
      return HPDF_Error_GetCode(fontdef->error);
   }

   HpdfMemClearTypeArray(attr->glyph_tbl.flgs, HpdfByte, attr->num_glyphs);
   attr->glyph_tbl.flgs[0] = 1;

   poffset = attr->glyph_tbl.offsets;
   if (attr->header.index_to_loc_format == 0) {
      /* short version */
      for (i = 0; i <= attr->num_glyphs; i++) {
         HpdfUInt16 tmp = 0;

         if ((ret = GetUINT16(attr->stream, &tmp)) != HPDF_OK)
            return ret;

         *poffset = tmp;
         poffset++;
      }
   }
   else {
      /* long version */
      for (i = 0; i <= attr->num_glyphs; i++) {
         if ((ret = GetUINT32(attr->stream, poffset)) != HPDF_OK)
            return ret;

         poffset++;
      }
   }


#ifdef LIBHPDF_DEBUG
   poffset = attr->glyph_tbl.offsets;
   for (i = 0; i <= attr->num_glyphs; i++) {
      HPDF_PTRACE((" ParseLOCA offset[%u]=%u\n", i, (HpdfUInt) *poffset));
      poffset++;
   }
#endif


   return HPDF_OK;
}


static HpdfStatus
LoadUnicodeName(HPDF_Stream   stream,
   HpdfUInt     offset,
   HpdfUInt     len,
   char    *buf)
{
   HpdfByte tmp[HPDF_LIMIT_MAX_NAME_LEN * 2 + 1];
   HpdfUInt i = 0;
   HpdfUInt j = 0;
   HpdfStatus ret;

   HpdfMemClear(buf, HPDF_LIMIT_MAX_NAME_LEN + 1);

   if ((ret = HPDF_Stream_Seek(stream, offset, HPDF_SEEK_SET)) !=
      HPDF_OK)
      return ret;

   if ((ret = HPDF_Stream_Read(stream, tmp, &len))
      != HPDF_OK)
      return ret;

   while (i < len) {
      i++;
      buf[j] = tmp[i];
      j++;
      i++;
   }

   return HPDF_OK;
}

static HpdfStatus
ParseName(HPDF_FontDef  fontdef)
{
   HPDF_TTFontDefAttr attr = (HPDF_TTFontDefAttr) fontdef->attr;
   HPDF_TTFTable *tbl = FindTable(fontdef, "name");
   HpdfStatus ret;
   HpdfUInt i;
   HPDF_TTF_NameRecord *name_rec;
   HpdfUInt offset_id1 = 0;
   HpdfUInt offset_id2 = 0;
   HpdfUInt offset_id1u = 0;
   HpdfUInt offset_id2u = 0;
   HpdfUInt len_id1 = 0;
   HpdfUInt len_id2 = 0;
   HpdfUInt len_id1u = 0;
   HpdfUInt len_id2u = 0;
   char tmp[HPDF_LIMIT_MAX_NAME_LEN + 1];

   HPDF_PTRACE((" HPDF_TTFontDef_ParseMaxp\n"));

   if (!tbl)
      return HPDF_SetError(fontdef->error, HPDF_TTF_MISSING_TABLE, 10);

   ret = HPDF_Stream_Seek(attr->stream, tbl->offset, HPDF_SEEK_SET);
   if (ret != HPDF_OK)
      return ret;

   ret += GetUINT16(attr->stream, &attr->name_tbl.format);
   ret += GetUINT16(attr->stream, &attr->name_tbl.count);
   ret += GetUINT16(attr->stream, &attr->name_tbl.string_offset);
   if (ret != HPDF_OK)
      return HPDF_Error_GetCode(fontdef->error);

   HPDF_PTRACE((" ParseName() format=%u, count=%u, string_offset=%u\n",
      attr->name_tbl.format, attr->name_tbl.count,
      attr->name_tbl.string_offset));

   attr->name_tbl.name_records = HpdfMemCreateTypeArray(
      fontdef->mmgr,
      HPDF_TTF_NameRecord,
      attr->name_tbl.count);
   if (!attr->name_tbl.name_records)
   {
      return HPDF_Error_GetCode(fontdef->error);
   }

   name_rec = attr->name_tbl.name_records;

   for (i = 0; i < attr->name_tbl.count; i++) {
      ret += GetUINT16(attr->stream, &name_rec->platform_id);
      ret += GetUINT16(attr->stream, &name_rec->encoding_id);
      ret += GetUINT16(attr->stream, &name_rec->language_id);
      ret += GetUINT16(attr->stream, &name_rec->name_id);
      ret += GetUINT16(attr->stream, &name_rec->length);
      ret += GetUINT16(attr->stream, &name_rec->offset);

      if (ret != HPDF_OK)
         return HPDF_Error_GetCode(fontdef->error);

      HPDF_PTRACE((" ParseName() platformID=%u, encodingID=%d, nameID=%d\n",
         name_rec->platform_id, name_rec->encoding_id,
         name_rec->name_id));

      if (name_rec->platform_id == 1 && name_rec->encoding_id == 0 &&
         name_rec->name_id == 6) {
         offset_id1 = tbl->offset + name_rec->offset +
            attr->name_tbl.string_offset;
         len_id1 = name_rec->length;
      }

      if (name_rec->platform_id == 1 && name_rec->encoding_id == 0 &&
         name_rec->name_id == 2) {
         offset_id2 = tbl->offset + name_rec->offset +
            attr->name_tbl.string_offset;
         len_id2 = name_rec->length;
      }

      if (name_rec->platform_id == 3 && name_rec->encoding_id == 1 &&
         name_rec->name_id == 6 && name_rec->language_id == 0x0409) {
         offset_id1u = tbl->offset + name_rec->offset +
            attr->name_tbl.string_offset;
         len_id1u = name_rec->length;
      }

      if (name_rec->platform_id == 3 && name_rec->encoding_id == 1 &&
         name_rec->name_id == 2 && name_rec->language_id == 0x0409) {
         offset_id2u = tbl->offset + name_rec->offset +
            attr->name_tbl.string_offset;
         len_id2u = name_rec->length;
      }

      name_rec++;
   }

   if ((!offset_id1 && !offset_id1u) ||
      (!offset_id2 && !offset_id2u))
      return HPDF_SetError(fontdef->error, HPDF_TTF_INVALID_FOMAT, 0);

   if (len_id1 == 0 && len_id1u > 0)
      len_id1 = len_id1u / 2 + len_id1u % 2;

   if (len_id2 == 0 && len_id2u > 0)
      len_id2 = len_id2u / 2 + len_id2u % 2;

   if (len_id1 + len_id2 + 8 > 127)
      return HPDF_SetError(fontdef->error, HPDF_TTF_INVALID_FOMAT, 0);

   HpdfMemClear(attr->base_font, HPDF_LIMIT_MAX_NAME_LEN + 1);

   if (offset_id1) {
      if ((ret = HPDF_Stream_Seek(attr->stream, offset_id1,
         HPDF_SEEK_SET)) != HPDF_OK)
         return ret;

      if ((ret = HPDF_Stream_Read(attr->stream, (HpdfByte *) attr->base_font, &len_id1))
         != HPDF_OK)
         return ret;
   }
   else {
      if ((ret = LoadUnicodeName(attr->stream, offset_id1u, len_id1u,
         attr->base_font)) != HPDF_OK)
         return ret;
   }

   HpdfMemClear(tmp, HPDF_LIMIT_MAX_NAME_LEN + 1);

   if (offset_id2) {
      if ((ret = HPDF_Stream_Seek(attr->stream, offset_id2, HPDF_SEEK_SET))
         != HPDF_OK)
         return ret;

      if ((ret = HPDF_Stream_Read(attr->stream, (HpdfByte *) tmp, &len_id2)) != HPDF_OK)
         return ret;
   }
   else {
      if ((ret = LoadUnicodeName(attr->stream, offset_id2u, len_id2u,
         tmp)) != HPDF_OK)
         return ret;
   }

   /*
    * get "postscript name" of from a "name" table as BaseName.
    * if subfamily name is not "Regular", add subfamily name to BaseName.
    * if subfamily name includes the blank character, remove it.
    * if subfamily name is "Bold" or "Italic" or "BoldItalic", set flags
    * attribute.
    */
   if (!HpdfMemIsEqual((HpdfByte *) tmp, (HpdfByte *) "Regular", 7)) 
   {
      char *dst = attr->base_font + len_id1;
      char *src = tmp;
      HpdfUInt j;

      *dst++ = ',';

      for (j = 0; j < len_id2; j++) {
         if (*src != ' ')
            *dst++ = *src++;

         if (dst >= attr->base_font + HPDF_LIMIT_MAX_NAME_LEN)
            break;
      }

      *dst = 0;

      if (HPDF_StrStr(tmp, "Bold", len_id2))
         fontdef->flags |= HPDF_FONT_FOURCE_BOLD;
      if (HPDF_StrStr(tmp, "Italic", len_id2))
         fontdef->flags |= HPDF_FONT_ITALIC;
   }

   HPDF_MemCpy((HpdfByte *) fontdef->base_font, (HpdfByte *) attr->base_font, HPDF_LIMIT_MAX_NAME_LEN + 1);

   HPDF_PTRACE(("  ParseName() base_font=%s\n", attr->base_font));

   return HPDF_OK;
}

static HpdfStatus
ParseOS2(HPDF_FontDef  fontdef)
{
   HPDF_TTFontDefAttr attr = (HPDF_TTFontDefAttr) fontdef->attr;
   HPDF_TTFTable *tbl = FindTable(fontdef, "OS/2");
   HpdfStatus ret;
   HpdfUInt16 version;
   HpdfUInt len;

   HPDF_PTRACE((" ParseOS2\n"));

   if (!tbl)
      return HPDF_SetError(fontdef->error, HPDF_TTF_MISSING_TABLE, 0);

   /* get the number version. */
   ret = HPDF_Stream_Seek(attr->stream, tbl->offset, HPDF_SEEK_SET);
   if (ret != HPDF_OK)
      return ret;

   if ((ret = GetUINT16(attr->stream, &version)) != HPDF_OK)
      return ret;

   /* check whether the font is allowed to be embedded. */
   ret = HPDF_Stream_Seek(attr->stream, tbl->offset + 8, HPDF_SEEK_SET);
   if (ret != HPDF_OK)
      return ret;

   if ((ret = GetUINT16(attr->stream, &attr->fs_type)) != HPDF_OK)
      return ret;

   if (attr->fs_type  & (0x0002 | 0x0100 | 0x0200) && attr->embedding)
      return HPDF_SetError(fontdef->error, HPDF_TTF_CANNOT_EMBEDDING_FONT,
         0);

   /* get fields sfamilyclass and panose. */
   if ((ret = HPDF_Stream_Seek(attr->stream, tbl->offset + 30, HPDF_SEEK_SET))
      != HPDF_OK)
      return ret;

   len = 2;
   if ((ret = HPDF_Stream_Read(attr->stream, attr->sfamilyclass, &len)) != HPDF_OK)
      return ret;

   len = 10;
   if ((ret = HPDF_Stream_Read(attr->stream, attr->panose, &len)) != HPDF_OK)
      return ret;

   HPDF_PTRACE((" ParseOS2 sFamilyClass=%d-%d "
      "Panose=%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X\n",
      attr->sfamilyclass[0], attr->sfamilyclass[1],
      attr->panose[0], attr->panose[1], attr->panose[2], attr->panose[3],
      attr->panose[4], attr->panose[5], attr->panose[6], attr->panose[7],
      attr->panose[8], attr->panose[9]));

   /* Class ID = 1   Oldstyle Serifs
      Class ID = 2   Transitional Serifs
      Class ID = 3   Modern Serifs
      Class ID = 4   Clarendon Serifs
      Class ID = 5   Slab Serifs
      Class ID = 6   (reserved for future use)
      Class ID = 7   Freeform Serifs
      Class ID = 8   Sans Serif
      Class ID = 9   Ornamentals
      Class ID = 10  Scripts
      Class ID = 11  (reserved for future use)
      Class ID = 12  Symbolic */
   if ((attr->sfamilyclass[0] > 0 && attr->sfamilyclass[0] < 6)
      || (attr->sfamilyclass[0] == 7))
      fontdef->flags = fontdef->flags | HPDF_FONT_SERIF;

   if (attr->sfamilyclass[0] == 10)
      fontdef->flags = fontdef->flags | HPDF_FONT_SCRIPT;

   if (attr->sfamilyclass[0] == 12)
      fontdef->flags = fontdef->flags | HPDF_FONT_SYMBOLIC;

   /* get fields ulCodePageRange1 and ulCodePageRange2 */
   if (version > 0) {
      if ((ret = HPDF_Stream_Seek(attr->stream, 36, HPDF_SEEK_CUR)) != HPDF_OK)
         return ret;

      if ((ret = GetUINT32(attr->stream, &attr->code_page_range1)) != HPDF_OK)
         return ret;

      if ((ret = GetUINT32(attr->stream, &attr->code_page_range2)) != HPDF_OK)
         return ret;
   }

   HPDF_PTRACE(("  ParseOS2 CodePageRange1=%08X CodePageRange2=%08X\n",
      (HpdfUInt) attr->code_page_range1,
      (HpdfUInt) attr->code_page_range2));

   return HPDF_OK;
}


static HpdfStatus
RecreateGLYF(HPDF_FontDef   fontdef,
   HpdfUInt32   *new_offsets,
   HPDF_Stream    stream)
{
   HpdfUInt32 save_offset = 0;
   HpdfUInt32 start_offset = stream->size;
   HPDF_TTFontDefAttr attr = (HPDF_TTFontDefAttr) fontdef->attr;
   HpdfStatus ret;
   HpdfInt i;

   HPDF_PTRACE((" RecreateGLYF\n"));

   for (i = 0; i < attr->num_glyphs; i++) {
      HpdfByte buf[HPDF_STREAM_BUF_SIZ];

      if (attr->glyph_tbl.flgs[i] == 1) {
         HpdfUInt offset = attr->glyph_tbl.offsets[i];
         HpdfUInt len = attr->glyph_tbl.offsets[i + 1] - offset;

         new_offsets[i] = stream->size - start_offset;
         if (attr->header.index_to_loc_format == 0) {
            new_offsets[i] /= 2;
            len *= 2;
         }

         HPDF_PTRACE((" RecreateGLYF[%u] move from [%u] to [%u]\n", i,
            (HpdfUInt) attr->glyph_tbl.base_offset + offset,
            (HpdfUInt) new_offsets[i]));

         if (attr->header.index_to_loc_format == 0)
            offset *= 2;

         offset += attr->glyph_tbl.base_offset;

         if ((ret = HPDF_Stream_Seek(attr->stream, offset, HPDF_SEEK_SET))
            != HPDF_OK)
            return ret;

         while (len > 0) {
            HpdfUInt tmp_len =
               (len > HPDF_STREAM_BUF_SIZ) ? HPDF_STREAM_BUF_SIZ : len;

            HpdfMemClear(buf, tmp_len);

            if ((ret = HPDF_Stream_Read(attr->stream, buf, &tmp_len))
               != HPDF_OK)
               return ret;

            if ((ret = HPDF_Stream_Write(stream, buf, tmp_len)) !=
               HPDF_OK)
               return ret;

            len -= tmp_len;
         }

         save_offset = stream->size - start_offset;
         if (attr->header.index_to_loc_format == 0)
            save_offset /= 2;
      }
      else {
         new_offsets[i] = save_offset;
      }
   }

   new_offsets[attr->num_glyphs] = save_offset;

#ifdef DEBUG
   for (i = 0; i <= attr->num_glyphs; i++) {
      HPDF_PTRACE((" RecreateGLYF[%u] offset=%u\n", i, new_offsets[i]));
   }
#endif

   return HPDF_OK;
}

static HpdfStatus
RecreateName(HPDF_FontDef   fontdef,
   HPDF_Stream    stream)
{
   HPDF_TTFontDefAttr attr = (HPDF_TTFontDefAttr) fontdef->attr;
   HPDF_TTFTable *tbl = FindTable(fontdef, "name");
   HpdfStatus ret = HPDF_OK;
   HpdfUInt i;
   HPDF_TTF_NameRecord *name_rec;
   HPDF_Stream tmp_stream = HPDF_MemStream_New(fontdef->mmgr,
      HPDF_STREAM_BUF_SIZ);

   HPDF_PTRACE((" RecreateName\n"));

   if (!tmp_stream)
      return HPDF_Error_GetCode(fontdef->error);

   ret += WriteUINT16(stream, attr->name_tbl.format);
   ret += WriteUINT16(stream, attr->name_tbl.count);
   ret += WriteUINT16(stream, attr->name_tbl.string_offset);

   if (ret != HPDF_OK) {
      HPDF_Stream_Free(tmp_stream);
      return HPDF_Error_GetCode(fontdef->error);
   }

   name_rec = attr->name_tbl.name_records;
   for (i = 0; i < attr->name_tbl.count; i++) {
      HpdfUInt name_len = name_rec->length;
      HpdfByte buf[HPDF_STREAM_BUF_SIZ];
      HpdfUInt tmp_len = name_len;
      HpdfUInt offset = tbl->offset + attr->name_tbl.string_offset +
         name_rec->offset;
      HpdfUInt rec_offset = tmp_stream->size;

      /* add suffix to font-name. */
      if (name_rec->name_id == 1 || name_rec->name_id == 4) {
         if (name_rec->platform_id == 0 || name_rec->platform_id == 3) {
            ret += HPDF_Stream_Write(tmp_stream, (HpdfByte *) attr->tag_name2,
               sizeof(attr->tag_name2));
            name_len += sizeof(attr->tag_name2);
         }
         else {
            ret += HPDF_Stream_Write(tmp_stream, (HpdfByte *) attr->tag_name,
               sizeof(attr->tag_name));
            name_len += sizeof(attr->tag_name);
         }
      }

      ret += WriteUINT16(stream, name_rec->platform_id);
      ret += WriteUINT16(stream, name_rec->encoding_id);
      ret += WriteUINT16(stream, name_rec->language_id);
      ret += WriteUINT16(stream, name_rec->name_id);
      ret += WriteUINT16(stream, (HpdfUInt16) name_len);
      ret += WriteUINT16(stream, (HpdfUInt16) rec_offset);

      ret += HPDF_Stream_Seek(attr->stream, offset, HPDF_SEEK_SET);

      if (ret != HPDF_OK) {
         HPDF_Stream_Free(tmp_stream);
         return HPDF_Error_GetCode(fontdef->error);
      }

      while (tmp_len > 0) {
         HpdfUInt len = (tmp_len > HPDF_STREAM_BUF_SIZ) ?
            HPDF_STREAM_BUF_SIZ : tmp_len;

         if ((ret = HPDF_Stream_Read(attr->stream, buf, &len)) != HPDF_OK) {
            HPDF_Stream_Free(tmp_stream);
            return ret;
         }

         if ((ret = HPDF_Stream_Write(tmp_stream, buf, len)) != HPDF_OK) {
            HPDF_Stream_Free(tmp_stream);
            return ret;
         }

         tmp_len -= len;
      }

      HPDF_PTRACE((" RecreateNAME name_rec[%u] platform_id=%u "
         "encoding_id=%u language_id=%u name_rec->name_id=%u "
         "length=%u offset=%u\n", i, name_rec->platform_id,
         name_rec->encoding_id, name_rec->language_id,
         name_rec->name_id, name_len, rec_offset));

      name_rec++;
   }

   ret = HPDF_Stream_WriteToStream(tmp_stream, stream,
      HPDF_STREAM_FILTER_NONE, NULL);

   HPDF_Stream_Free(tmp_stream);

   return ret;
}


static HpdfStatus
_WriteHeader(HPDF_FontDef   fontdef,
   HPDF_Stream    stream,
   HpdfUInt32   *check_sum_ptr)
{
   HPDF_TTFontDefAttr attr = (HPDF_TTFontDefAttr) fontdef->attr;
   HpdfStatus ret = HPDF_OK;

   HPDF_PTRACE((" _WriteHeader\n"));

   ret += HPDF_Stream_Write(stream, attr->header.version_number, 4);
   ret += WriteUINT32(stream, attr->header.font_revision);

   /* save the address of checkSumAdjustment.
    * the value is rewrite to computed value after new check-sum-value is
    * generated.
    */
   *check_sum_ptr = stream->size;

   ret += WriteUINT32(stream, 0);
   ret += WriteUINT32(stream, attr->header.magic_number);
   ret += WriteUINT16(stream, attr->header.flags);
   ret += WriteUINT16(stream, attr->header.units_per_em);
   ret += HPDF_Stream_Write(stream, attr->header.created, 8);
   ret += HPDF_Stream_Write(stream, attr->header.modified, 8);
   ret += WriteINT16(stream, attr->header.x_min);
   ret += WriteINT16(stream, attr->header.y_min);
   ret += WriteINT16(stream, attr->header.x_max);
   ret += WriteINT16(stream, attr->header.y_max);
   ret += WriteUINT16(stream, attr->header.mac_style);
   ret += WriteUINT16(stream, attr->header.lowest_rec_ppem);
   ret += WriteINT16(stream, attr->header.font_direction_hint);
   ret += WriteINT16(stream, attr->header.index_to_loc_format);
   ret += WriteINT16(stream, attr->header.glyph_data_format);

   if (ret != HPDF_OK)
      return HPDF_Error_GetCode(fontdef->error);

   return HPDF_OK;
}


HpdfStatus
HPDF_TTFontDef_SaveFontData(HPDF_FontDef   fontdef,
   HPDF_Stream    stream)
{
   HPDF_TTFontDefAttr attr = (HPDF_TTFontDefAttr) fontdef->attr;
   HPDF_TTFTable tmp_tbl[HPDF_REQUIRED_TAGS_COUNT];
   HPDF_Stream tmp_stream;
   HpdfUInt32 *new_offsets;
   HpdfUInt i;
   HpdfUInt32 check_sum_ptr = 0;
   HpdfStatus ret;
   HpdfUInt32 offset_base;
   HpdfUInt32 tmp_check_sum = 0xB1B0AFBA;
   HPDF_TTFTable emptyTable;
   emptyTable.length = 0;
   emptyTable.offset = 0;

   HPDF_PTRACE((" SaveFontData\n"));

   ret = WriteUINT32(stream, attr->offset_tbl.sfnt_version);
   ret += WriteUINT16(stream, HPDF_REQUIRED_TAGS_COUNT);
   ret += WriteUINT16(stream, attr->offset_tbl.search_range);
   ret += WriteUINT16(stream, attr->offset_tbl.entry_selector);
   ret += WriteUINT16(stream, attr->offset_tbl.range_shift);

   if (ret != HPDF_OK)
      return HPDF_Error_GetCode(fontdef->error);

   tmp_stream = HPDF_MemStream_New(fontdef->mmgr, HPDF_STREAM_BUF_SIZ);
   if (!tmp_stream)
      return HPDF_Error_GetCode(fontdef->error);

   offset_base = 12 + 16 * HPDF_REQUIRED_TAGS_COUNT;

   new_offsets = HpdfMemCreateTypeArray(fontdef->mmgr, HpdfUInt32, attr->num_glyphs + 1);
   if (!new_offsets) 
   {
      HPDF_Stream_Free(tmp_stream);
      return HPDF_Error_GetCode(fontdef->error);
   }

   for (i = 0; i < HPDF_REQUIRED_TAGS_COUNT; i++) {
      HPDF_TTFTable *tbl = FindTable(fontdef, REQUIRED_TAGS[i]);
      HpdfUInt32 length;
      HpdfUInt new_offset;
      HpdfUInt32 *poffset;
      HpdfUInt32 value;

      if (!tbl) {
         tbl = &emptyTable;
         HPDF_MemCpy((HpdfByte *) tbl->tag,
            (HpdfByte const *) REQUIRED_TAGS[i], 4);
      }

      if (!tbl) {
         ret = HPDF_SetError(fontdef->error, HPDF_TTF_MISSING_TABLE, i);
         goto Exit;
      }

      ret = HPDF_Stream_Seek(attr->stream, tbl->offset, HPDF_SEEK_SET);
      if (ret != HPDF_OK)
         goto Exit;

      length = tbl->length;
      new_offset = tmp_stream->size;

      if      (HpdfMemIsEqual((HpdfByte *) tbl->tag, (HpdfByte *) "head", 4))
      {
         ret = _WriteHeader(fontdef, tmp_stream, &check_sum_ptr);
      }
      else if (HpdfMemIsEqual((HpdfByte *) tbl->tag, (HpdfByte *) "glyf", 4))
      {
         ret = RecreateGLYF(fontdef, new_offsets, tmp_stream);
      }
      else if (HpdfMemIsEqual((HpdfByte *) tbl->tag, (HpdfByte *) "hmtx", 4)) 
      {
         HpdfUInt j;
         HPDF_TTF_LongHorMetric *pmetric;

         HpdfMemClear(&value, 4);
         pmetric=attr->h_metric;
         for (j = 0; j < attr->num_h_metric; j++) {
            // write all the used glyphs and write the last metric in the hMetrics array
            if (attr->glyph_tbl.flgs[j] == 1 || j == (HpdfUInt) (attr->num_h_metric - 1)) {
               ret += WriteUINT16(tmp_stream, pmetric->advance_width);
               ret += WriteINT16(tmp_stream, pmetric->lsb);
            }
            else
            {
               ret += WriteUINT16(tmp_stream, (HpdfUInt16) value);
               ret += WriteINT16(tmp_stream, (HpdfInt16) value);
            }
            pmetric++;
         }

         while (j < attr->num_glyphs) {
            if (attr->glyph_tbl.flgs[j] == 1) {
               ret += WriteINT16(tmp_stream, pmetric->lsb);
            }
            else
               ret += WriteINT16(tmp_stream, (HpdfInt16) value);
            pmetric++;
            j++;
         }
      }
      else if (HpdfMemIsEqual((HpdfByte *) tbl->tag, (HpdfByte *) "loca", 4)) 
      {
         HpdfUInt j;

         HpdfMemClear(&value, 4);
         poffset = new_offsets;

         if (attr->header.index_to_loc_format == 0) {
            for (j = 0; j <= attr->num_glyphs; j++) {
               ret += WriteUINT16(tmp_stream, (HpdfUInt16) *poffset);
               poffset++;
            }
         }
         else {
            for (j = 0; j <= attr->num_glyphs; j++) {
               ret += WriteUINT32(tmp_stream, *poffset);
               poffset++;
            }
         }
      }
      else if (HpdfMemIsEqual((HpdfByte *) tbl->tag, (HpdfByte *) "name", 4)) 
      {
         ret = RecreateName(fontdef, tmp_stream);
      }
      else if (HpdfMemIsEqual((HpdfByte *) tbl->tag, (HpdfByte *) "post", 4)) 
      {
         value=0x00030000;
         ret += HPDF_Stream_Write(tmp_stream, (HpdfByte *) &value, 4);
         HpdfMemClear(&value, 4);
         ret  = HPDF_Stream_Write(tmp_stream, (HpdfByte *) &value, 4); // italicAngle
         ret += HPDF_Stream_Write(tmp_stream, (HpdfByte *) &value, 4); // underlinePosition + underlineThickness
         ret += HPDF_Stream_Write(tmp_stream, (HpdfByte *) &value, 4); // isFixedPitch
         ret += HPDF_Stream_Write(tmp_stream, (HpdfByte *) &value, 4); // minMemType42 
         ret += HPDF_Stream_Write(tmp_stream, (HpdfByte *) &value, 4); // maxMemType42 
         ret += HPDF_Stream_Write(tmp_stream, (HpdfByte *) &value, 4); // minMemType1
         ret += HPDF_Stream_Write(tmp_stream, (HpdfByte *) &value, 4); // maxMemType1 
      }
      else {
         HpdfUInt size = 4;

         while (length > 4) {
            value = 0;
            size = 4;
            ret = HPDF_Stream_Read(attr->stream, (HpdfByte *) &value, &size);
            ret += HPDF_Stream_Write(tmp_stream, (HpdfByte *) &value, size);
            length -= 4;
         }

         value = 0;
         size = length;
         ret += HPDF_Stream_Read(attr->stream, (HpdfByte *) &value, &size);
         ret += HPDF_Stream_Write(tmp_stream, (HpdfByte *) &value, size);
      }

      tmp_tbl[i].offset = new_offset;
      tmp_tbl[i].length = tmp_stream->size - new_offset;

      /* pad at 4 bytes */
      {
         HpdfUInt size=tmp_tbl[i].length % 4;

         HpdfMemClear(&value, 4);

         if (size != 0)
            ret += HPDF_Stream_Write(tmp_stream, (HpdfByte *) &value, 4-size);
      }


      if (ret != HPDF_OK)
         goto Exit;
   }

   /* recalcurate checksum */
   for (i = 0; i < HPDF_REQUIRED_TAGS_COUNT; i++) {
      HPDF_TTFTable tbl = tmp_tbl[i];
      HpdfUInt32 buf;
      HpdfUInt length = tbl.length;

      HPDF_PTRACE((" SaveFontData() tag[%s] length=%u\n",
         REQUIRED_TAGS[i], length));

      if ((ret = HPDF_Stream_Seek(tmp_stream, tbl.offset, HPDF_SEEK_SET))
         != HPDF_OK)
         break;

      tbl.check_sum = 0;
      while (length > 0) {
         HpdfUInt rlen = (length > 4) ? 4 : length;
         buf = 0;
         if ((ret = HPDF_Stream_Read(tmp_stream, (HpdfByte *) &buf, &rlen))
            != HPDF_OK)
            break;

         UINT32Swap(&buf);
         tbl.check_sum += buf;
         length -= rlen;
      }

      if (ret != HPDF_OK)
         break;

      HPDF_PTRACE((" SaveFontData tag[%s] check-sum=%u offset=%u\n",
         REQUIRED_TAGS[i], (HpdfUInt) tbl.check_sum,
         (HpdfUInt) tbl.offset));

      ret += HPDF_Stream_Write(stream, (HpdfByte *) REQUIRED_TAGS[i], 4);
      ret += WriteUINT32(stream, tbl.check_sum);
      tbl.offset += offset_base;
      ret += WriteUINT32(stream, tbl.offset);
      ret += WriteUINT32(stream, tbl.length);

      if (ret != HPDF_OK)
         break;
   }

   if (ret != HPDF_OK)
      goto Exit;

   /* calucurate checkSumAdjustment.*/
   ret = HPDF_Stream_Seek(tmp_stream, 0, HPDF_SEEK_SET);
   if (ret != HPDF_OK)
      goto Exit;

   for (;;) {
      HpdfUInt32 buf;
      HpdfUInt siz = sizeof(buf);

      ret = HPDF_Stream_Read(tmp_stream, (HpdfByte *) &buf, &siz);
      if (ret != HPDF_OK || siz <= 0) {
         if (ret == HPDF_STREAM_EOF)
            ret = HPDF_OK;
         break;
      }

      UINT32Swap(&buf);
      tmp_check_sum -= buf;
   }

   if (ret != HPDF_OK)
      goto Exit;

   HPDF_PTRACE((" SaveFontData new checkSumAdjustment=%u\n",
      (HpdfUInt) tmp_check_sum));

   UINT32Swap(&tmp_check_sum);

   ret = HPDF_Stream_Seek(tmp_stream, check_sum_ptr, HPDF_SEEK_SET);
   if (ret == HPDF_OK) {
      ret = HPDF_MemStream_Rewrite(tmp_stream, (HpdfByte *) &tmp_check_sum,
         4);
   }

   if (ret != HPDF_OK)
      goto Exit;

   attr->length1 = tmp_stream->size + offset_base;
   ret = HPDF_Stream_WriteToStream(tmp_stream, stream, 0, NULL);

   goto Exit;

Exit:
   HpdfMemDestroy(fontdef->mmgr, new_offsets);
   HPDF_Stream_Free(tmp_stream);
   return ret;
}

void
HPDF_TTFontDef_SetTagName(HPDF_FontDef   fontdef,
   char     *tag)
{
   HPDF_TTFontDefAttr attr = (HPDF_TTFontDefAttr) fontdef->attr;
   char buf[HPDF_LIMIT_MAX_NAME_LEN + 1];
   HpdfUInt i;

   HPDF_PTRACE((" HPDF_TTFontDef_SetTagName\n"));

   if (HPDF_StrLen(tag, HPDF_LIMIT_MAX_NAME_LEN) != HPDF_TTF_FONT_TAG_LEN)
      return;

   HPDF_MemCpy((HpdfByte *) attr->tag_name, (HpdfByte *) tag, HPDF_TTF_FONT_TAG_LEN);
   attr->tag_name[HPDF_TTF_FONT_TAG_LEN] = '+';

   for (i = 0; i < HPDF_TTF_FONT_TAG_LEN + 1; i++) {
      attr->tag_name2[i * 2] = 0x00;
      attr->tag_name2[i * 2 + 1] = attr->tag_name[i];
   }

   HpdfMemClear(buf, HPDF_LIMIT_MAX_NAME_LEN + 1);
   HPDF_MemCpy((HpdfByte *) buf, (HpdfByte *) attr->tag_name, HPDF_TTF_FONT_TAG_LEN + 1);
   HPDF_MemCpy((HpdfByte *) buf + HPDF_TTF_FONT_TAG_LEN + 1, (HpdfByte *) fontdef->base_font, HPDF_LIMIT_MAX_NAME_LEN - HPDF_TTF_FONT_TAG_LEN - 1);

   HPDF_MemCpy((HpdfByte *) attr->base_font, (HpdfByte *) buf, HPDF_LIMIT_MAX_NAME_LEN + 1);
}

/*
int
PdfTTFontDef::GetNameAttr(unsigned char* buf, HpdfUInt name_id,
        HpdfUInt platform_id, HpdfUInt lang_id, HpdfUInt max_len)


*/

static HPDF_TTFTable*
FindTable(HPDF_FontDef      fontdef,
   char const *tag)
{
   HPDF_TTFontDefAttr attr = (HPDF_TTFontDefAttr) fontdef->attr;
   HPDF_TTFTable* tbl = attr->offset_tbl.table;
   HpdfUInt i;

   for (i = 0; i < attr->offset_tbl.num_tables; i++, tbl++) 
   {
      if (HpdfMemIsEqual((HpdfByte *) tbl->tag, (HpdfByte *) tag, 4))
      {
         HPDF_PTRACE((" FindTable find table[%c%c%c%c]\n",
            tbl->tag[0], tbl->tag[1], tbl->tag[2], tbl->tag[3]));
         return tbl;
      }
   }

   return NULL;
}

static void
UINT32Swap(HpdfUInt32  *value)
{
   HpdfByte b[4];

   HPDF_MemCpy(b, (HpdfByte *) value, 4);
   *value = (HpdfUInt32) ((HpdfUInt32) b[0] << 24 |
      (HpdfUInt32) b[1] << 16 |
      (HpdfUInt32) b[2] << 8 |
      (HpdfUInt32) b[3]);
}

static void
UINT16Swap(HpdfUInt16  *value)
{
   HpdfByte b[2];

   HPDF_MemCpy(b, (HpdfByte *) value, 2);
   *value = (HpdfUInt16) ((HpdfUInt16) b[0] << 8 | (HpdfUInt16) b[1]);
}

static void
INT16Swap(HpdfInt16  *value)
{
   HpdfByte b[2];

   HPDF_MemCpy(b, (HpdfByte *) value, 2);
   *value = (HpdfInt16) ((HpdfInt16) b[0] << 8 | (HpdfInt16) b[1]);
}
