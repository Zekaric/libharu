/*
 * << Haru Free PDF Library >> -- hpdf_fontdef.h
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

#ifndef _HPDF_FONTDEF_H
#define _HPDF_FONTDEF_H

#include "hpdf_objects.h"

#ifdef __cplusplus
extern "C" {
#endif


#define HPDF_FONTDEF_SIG_BYTES 0x464F4E54L

/*------ collection of flags for defining characteristics. ---*/

#define HPDF_FONT_FIXED_WIDTH    1
#define HPDF_FONT_SERIF          2
#define HPDF_FONT_SYMBOLIC       4
#define HPDF_FONT_SCRIPT         8
  /* Reserved                    16 */
#define HPDF_FONT_STD_CHARSET    32
#define HPDF_FONT_ITALIC         64
  /* Reserved                    128
     Reserved                    256
     Reserved                    512
     Reserved                    1024
     Reserved                    2048
     Reserved                    4096
     Reserved                    8192
     Reserved                    16384
     Reserved                    32768 */
#define HPDF_FONT_ALL_CAP        65536
#define HPDF_FONT_SMALL_CAP      131072
#define HPDF_FONT_FOURCE_BOLD    262144

#define HPDF_CID_W_TYPE_FROM_TO     0
#define HPDF_CID_W_TYPE_FROM_ARRAY  1

/*-- HPDF_FontDef ---------------------------------------*/

typedef struct _HPDF_CharData {
    HpdfInt16     char_cd;
    HpdfUnicode   unicode;
    HpdfInt16     width;
} HPDF_CharData;

typedef enum  _HPDF_FontDefType {
    HPDF_FONTDEF_TYPE_TYPE1,
    HPDF_FONTDEF_TYPE_TRUETYPE,
    HPDF_FONTDEF_TYPE_CID,
    HPDF_FONTDEF_TYPE_UNINITIALIZED,
    HPDF_FONTDEF_TYPE_EOF
} HPDF_FontDefType;

typedef struct _HPDF_CID_Width {
    HpdfUInt16   cid;
    HpdfInt16    width;
}  HPDF_CID_Width;

/*----------------------------------------------------------------------------*/
/*----- HPDF_FontDef ---------------------------------------------------------*/

typedef struct _HPDF_FontDef_Rec  *HPDF_FontDef;

typedef void  (*HPDF_FontDef_FreeFunc)  (HPDF_FontDef  fontdef);

typedef void  (*HPDF_FontDef_CleanFunc)  (HPDF_FontDef  fontdef);

typedef HpdfStatus  (*HPDF_FontDef_InitFunc)  (HPDF_FontDef  fontdef);

typedef struct _HPDF_FontDef_Rec {
    HpdfUInt32              sig_bytes;
    char                base_font[HPDF_LIMIT_MAX_NAME_LEN + 1];
    HPDF_MMgr                mmgr;
    HpdfError               *error;
    HPDF_FontDefType         type;
    HPDF_FontDef_CleanFunc   clean_fn;
    HPDF_FontDef_FreeFunc    free_fn;
    HPDF_FontDef_InitFunc    init_fn;

    HpdfInt16    ascent;
    HpdfInt16    descent;
    HpdfUInt     flags;
    HPDF_Box      font_bbox;
    HpdfInt16    italic_angle;
    HpdfUInt16   stemv;
    HpdfInt16    avg_width;
    HpdfInt16    max_width;
    HpdfInt16    missing_width;
    HpdfUInt16   stemh;
    HpdfUInt16   x_height;
    HpdfUInt16   cap_height;

    /*  the initial value of descriptor entry is NULL.
     *  when first font-object based on the fontdef object is created,
     *  the font-descriptor object is created and descriptor entry is set.
     */
    HPDF_Dict                descriptor;
    HPDF_Stream              data;

    HpdfBool                valid;
    void                    *attr;
} HPDF_FontDef_Rec;


void
HPDF_FontDef_Free  (HPDF_FontDef  fontdef);


void
HPDF_FontDef_Cleanup  (HPDF_FontDef  fontdef);


HpdfBool
HPDF_FontDef_Validate  (HPDF_FontDef  fontdef);


/*----------------------------------------------------------------------------*/
/*----- HPDF_Type1FontDef  ---------------------------------------------------*/

typedef struct _HPDF_Type1FontDefAttrRec   *HPDF_Type1FontDefAttr;

typedef struct _HPDF_Type1FontDefAttrRec {
    HpdfByte        first_char;                               /* Required */
    HpdfByte        last_char;                                /* Required */
    HPDF_CharData  *widths;                                   /* Required */
    HpdfUInt       widths_count;

    HpdfInt16      leading;
    char      *char_set;
    char       encoding_scheme[HPDF_LIMIT_MAX_NAME_LEN + 1];
    HpdfUInt       length1;
    HpdfUInt       length2;
    HpdfUInt       length3;
    HpdfBool       is_base14font;
    HpdfBool       is_fixed_pitch;

    HPDF_Stream     font_data;
} HPDF_Type1FontDefAttr_Rec;



HPDF_FontDef
HPDF_Type1FontDef_New  (HPDF_MMgr  mmgr);


HPDF_FontDef
HPDF_Type1FontDef_Load  (HPDF_MMgr         mmgr,
                         HPDF_Stream       afm,
                         HPDF_Stream       font_data);


HPDF_FontDef
HPDF_Type1FontDef_Duplicate  (HPDF_MMgr     mmgr,
                              HPDF_FontDef  src);


HpdfStatus
HPDF_Type1FontDef_SetWidths  (HPDF_FontDef         fontdef,
                              const HPDF_CharData  *widths);


HpdfInt16
HPDF_Type1FontDef_GetWidthByName  (HPDF_FontDef     fontdef,
                                   char const *gryph_name);


HpdfInt16
HPDF_Type1FontDef_GetWidth  (HPDF_FontDef  fontdef,
                             HpdfUnicode  unicode);


HPDF_FontDef
HPDF_Base14FontDef_New  (HPDF_MMgr        mmgr,
                         char const *font_name);



/*----------------------------------------------------------------------------*/
/*----- HPDF_TTFontDef  ------------------------------------------------------*/

#define HPDF_TTF_FONT_TAG_LEN  6

typedef struct _HPDF_TTF_Table {
        char     tag[4];
        HpdfUInt32   check_sum;
        HpdfUInt32   offset;
        HpdfUInt32   length;
} HPDF_TTFTable;


typedef struct _HPDF_TTF_OffsetTbl {
        HpdfUInt32     sfnt_version;
        HpdfUInt16     num_tables;
        HpdfUInt16     search_range;
        HpdfUInt16     entry_selector;
        HpdfUInt16     range_shift;
        HPDF_TTFTable  *table;
} HPDF_TTF_OffsetTbl;


typedef struct _HPDF_TTF_CmapRange {
        HpdfUInt16   format;
        HpdfUInt16   length;
        HpdfUInt16   language;
        HpdfUInt16   seg_count_x2;
        HpdfUInt16   search_range;
        HpdfUInt16   entry_selector;
        HpdfUInt16   range_shift;
        HpdfUInt16  *end_count;
        HpdfUInt16   reserved_pad;
        HpdfUInt16  *start_count;
        HpdfInt16   *id_delta;
        HpdfUInt16  *id_range_offset;
        HpdfUInt16  *glyph_id_array;
        HpdfUInt     glyph_id_array_count;
} HPDF_TTF_CmapRange;


typedef struct _HPDF_TTF_GryphOffsets 
{
        HpdfUInt32    base_offset;
        HpdfUInt32   *offsets;
        HpdfByte     *flgs;   /* 0: unused, 1: used */
} HPDF_TTF_GryphOffsets;


typedef struct _HPDF_TTF_LongHorMetric {
        HpdfUInt16  advance_width;
        HpdfInt16   lsb;
} HPDF_TTF_LongHorMetric;


typedef struct _HPDF_TTF_FontHeader {
    HpdfByte      version_number[4];
    HpdfUInt32   font_revision;
    HpdfUInt32   check_sum_adjustment;
    HpdfUInt32   magic_number;
    HpdfUInt16   flags;
    HpdfUInt16   units_per_em;
    HpdfByte      created[8];
    HpdfByte      modified[8];
    HpdfInt16    x_min;
    HpdfInt16    y_min;
    HpdfInt16    x_max;
    HpdfInt16    y_max;
    HpdfUInt16   mac_style;
    HpdfUInt16   lowest_rec_ppem;
    HpdfInt16    font_direction_hint;
    HpdfInt16    index_to_loc_format;
    HpdfInt16    glyph_data_format;
} HPDF_TTF_FontHeader;


typedef struct _HPDF_TTF_NameRecord {
    HpdfUInt16   platform_id;
    HpdfUInt16   encoding_id;
    HpdfUInt16   language_id;
    HpdfUInt16   name_id;
    HpdfUInt16   length;
    HpdfUInt16   offset;
}  HPDF_TTF_NameRecord;


typedef struct _HPDF_TTF_NamingTable {
    HpdfUInt16           format;
    HpdfUInt16           count;
    HpdfUInt16           string_offset;
    HPDF_TTF_NameRecord  *name_records;
}  HPDF_TTF_NamingTable;


typedef struct _HPDF_TTFontDefAttr_Rec   *HPDF_TTFontDefAttr;

typedef struct _HPDF_TTFontDefAttr_Rec {
    char                base_font[HPDF_LIMIT_MAX_NAME_LEN + 1];
    HpdfByte                 first_char;
    HpdfByte                 last_char;
    char               *char_set;
    char                tag_name[HPDF_TTF_FONT_TAG_LEN + 1];
    char                tag_name2[(HPDF_TTF_FONT_TAG_LEN + 1) * 2];
    HPDF_TTF_FontHeader      header;
    HPDF_TTF_GryphOffsets    glyph_tbl;
    HpdfUInt16              num_glyphs;
    HPDF_TTF_NamingTable     name_tbl;
    HPDF_TTF_LongHorMetric  *h_metric;
    HpdfUInt16              num_h_metric;
    HPDF_TTF_OffsetTbl       offset_tbl;
    HPDF_TTF_CmapRange       cmap;
    HpdfUInt16              fs_type;
    HpdfByte                 sfamilyclass[2];
    HpdfByte                 panose[10];
    HpdfUInt32              code_page_range1;
    HpdfUInt32              code_page_range2;

    HpdfUInt                length1;

    HpdfBool                embedding;
    HpdfBool                is_cidfont;

    HPDF_Stream              stream;
} HPDF_TTFontDefAttr_Rec;



HPDF_FontDef
HPDF_TTFontDef_New (HPDF_MMgr   mmgr);


HPDF_FontDef
HPDF_TTFontDef_Load  (HPDF_MMgr     mmgr,
                      HPDF_Stream   stream,
                      HpdfBool     embedding);


HPDF_FontDef
HPDF_TTFontDef_Load2  (HPDF_MMgr     mmgr,
                       HPDF_Stream   stream,
                       HpdfUInt     index,
                       HpdfBool     embedding);


HpdfUInt16
HPDF_TTFontDef_GetGlyphid  (HPDF_FontDef   fontdef,
                            HpdfUInt16    unicode);


HpdfInt16
HPDF_TTFontDef_GetCharWidth  (HPDF_FontDef   fontdef,
                              HpdfUInt16    unicode);


HpdfInt16
HPDF_TTFontDef_GetGidWidth  (HPDF_FontDef   fontdef,
                             HpdfUInt16    gid);


HpdfStatus
HPDF_TTFontDef_SaveFontData  (HPDF_FontDef   fontdef,
                              HPDF_Stream    stream);


HPDF_Box
HPDF_TTFontDef_GetCharBBox  (HPDF_FontDef   fontdef,
                             HpdfUInt16    unicode);


void
HPDF_TTFontDef_SetTagName  (HPDF_FontDef   fontdef,
                            char     *tag);


/*----------------------------------------------------------------------------*/
/*----- HPDF_CIDFontDef  -----------------------------------------------------*/

typedef struct _HPDF_CIDFontDefAttrRec   *HPDF_CIDFontDefAttr;

typedef struct _HPDF_CIDFontDefAttrRec {
    HPDF_List     widths;
    HpdfInt16    DW;
    HpdfInt16    DW2[2];
} HPDF_CIDFontDefAttr_Rec;


HPDF_FontDef
HPDF_CIDFontDef_New  (HPDF_MMgr               mmgr,
                      char              *name,
                      HPDF_FontDef_InitFunc   init_fn);


HpdfStatus
HPDF_CIDFontDef_AddWidth  (HPDF_FontDef            fontdef,
                           const HPDF_CID_Width   *widths);


HpdfInt16
HPDF_CIDFontDef_GetCIDWidth  (HPDF_FontDef  fontdef,
                              HpdfUInt16   cid);



HpdfStatus
HPDF_CIDFontDef_ChangeStyle   (HPDF_FontDef    fontdef,
                               HpdfBool       bold,
                               HpdfBool       italic);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _HPDF_FONTDEF_H */
