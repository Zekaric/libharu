/*
 * << Haru Free PDF Library >> -- hpdf_gstate.h
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

#ifndef _HPDF_GSTATE_H
#define _HPDF_GSTATE_H

#include "hpdf_font.h"

#ifdef __cplusplus
extern "C" {
#endif


/*----------------------------------------------------------------------------*/
/*------ graphic state stack -------------------------------------------------*/

typedef struct _HPDF_GState_Rec  *HPDF_GState;

typedef struct _HPDF_GState_Rec {
    HPDF_TransMatrix        trans_matrix;
    HpdfReal               line_width;
    HPDF_LineCap            line_cap;
    HPDF_LineJoin           line_join;
    HpdfReal               miter_limit;
    HPDF_DashMode           dash_mode;
    HpdfReal               flatness;

    HpdfReal               char_space;
    HpdfReal               word_space;
    HpdfReal               h_scalling;
    HpdfReal               text_leading;
    HPDF_TextRenderingMode  rendering_mode;
    HpdfReal               text_rise;

    HPDF_ColorSpace         cs_fill;
    HPDF_ColorSpace         cs_stroke;
    HPDF_RGBColor           rgb_fill;
    HPDF_RGBColor           rgb_stroke;
    HPDF_CMYKColor          cmyk_fill;
    HPDF_CMYKColor          cmyk_stroke;
    HpdfReal               gray_fill;
    HpdfReal               gray_stroke;

    HPDF_Font               font;
    HpdfReal               font_size;
    HPDF_WritingMode        writing_mode;

    HPDF_GState             prev;
    HpdfUInt               depth;
} HPDF_GState_Rec;

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

HPDF_GState
HPDF_GState_New  (HPDF_MMgr    mmgr,
                  HPDF_GState  current);


HPDF_GState
HPDF_GState_Free  (HPDF_MMgr    mmgr,
                   HPDF_GState  gstate);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _HPDF_GSTATE_H */

