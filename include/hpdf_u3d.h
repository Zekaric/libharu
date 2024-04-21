/*
 * << Haru Free PDF Library >> -- hpdf_u3d.h
 *
 * URL: http://libharu.org
 *
 * Copyright(c) 1999-2006 Takeshi Kanno <takeshi_kanno@est.hi-ho.ne.jp>
 * Copyright(c) 2007-2009 Antony Dovgal <tony@daylessday.org>
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 * It is provided "as is" without express or implied warranty.
 *
 */

#ifndef _HPDF_U3D_H
#define _HPDF_U3D_H

#include "hpdf_objects.h"

#ifdef __cplusplus
extern "C" {
#endif

HPDF_EXPORT(HPDF_JavaScript)  HPDF_CreateJavaScript(                 HpdfDoc const * const doc, const char *code);
HPDF_EXPORT(HPDF_JavaScript)  HPDF_LoadJSFromFile(                   HpdfDoc       * const doc, const char *filename);
HPDF_EXPORT(HPDF_U3D)         HPDF_LoadU3DFromFile(                  HpdfDoc       * const doc, const char *filename);
HPDF_EXPORT(HPDF_Image)       HPDF_LoadU3DFromMem(                   HpdfDoc       * const doc, HpdfByte const *const buffer, HpdfUInt size);
HPDF_EXPORT(HPDF_Dict)        HPDF_Create3DView(                     HPDF_MMgr mmgr, const char *name);
HPDF_EXPORT(HpdfStatus)       HPDF_U3D_Add3DView(                    HPDF_U3D u3d, HPDF_Dict view);
HPDF_EXPORT(HpdfStatus)       HPDF_U3D_SetDefault3DView(             HPDF_U3D u3d, const char *name);
HPDF_EXPORT(HpdfStatus)       HPDF_U3D_AddOnInstanciate(             HPDF_U3D u3d, HPDF_JavaScript javaScript);
HPDF_EXPORT(HPDF_Dict)        HPDF_3DView_CreateNode(                HPDF_Dict view, const char *name);
HPDF_EXPORT(HpdfStatus)       HPDF_3DViewNode_SetOpacity(            HPDF_Dict node, HpdfReal opacity);
HPDF_EXPORT(HpdfStatus)       HPDF_3DViewNode_SetVisibility(         HPDF_Dict node, HpdfBool visible);
HPDF_EXPORT(HpdfStatus)       HPDF_3DViewNode_SetMatrix(             HPDF_Dict node, HPDF_3DMatrix Mat3D);
HPDF_EXPORT(HpdfStatus)       HPDF_3DView_AddNode(                   HPDF_Dict view, HPDF_Dict node);
HPDF_EXPORT(HpdfStatus)       HPDF_3DView_SetLighting(               HPDF_Dict view, const char *scheme);
HPDF_EXPORT(HpdfStatus)       HPDF_3DView_SetBackgroundColor(        HPDF_Dict view, HpdfReal r, HpdfReal g, HpdfReal b);
HPDF_EXPORT(HpdfStatus)       HPDF_3DView_SetPerspectiveProjection(  HPDF_Dict view, HpdfReal fov);
HPDF_EXPORT(HpdfStatus)       HPDF_3DView_SetOrthogonalProjection(   HPDF_Dict view, HpdfReal mag);
HPDF_EXPORT(HpdfStatus)       HPDF_3DView_SetCamera(                 HPDF_Dict view, HpdfReal coox, HpdfReal cooy, HpdfReal cooz, HpdfReal c2cx, HpdfReal c2cy, HpdfReal c2cz, HpdfReal roo, HpdfReal roll);
HPDF_EXPORT(HpdfStatus)       HPDF_3DView_SetCameraByMatrix(         HPDF_Dict view, HPDF_3DMatrix Mat3D, HpdfReal co);
HPDF_EXPORT(HpdfStatus)       HPDF_3DView_SetCrossSectionOn(         HPDF_Dict view, HpdfPoint3D const center, HpdfReal const Roll, HpdfReal const Pitch, HpdfReal const opacity, HpdfBool const showintersection);
HPDF_EXPORT(HpdfStatus)       HPDF_3DView_SetCrossSectionOff(        HPDF_Dict view);
            HPDF_Dict         HPDF_3DView_New(                       HPDF_MMgr  mmgr, HPDF_Xref  xref, HPDF_U3D	u3d, const char *name);

#if defined(WIN32)
HPDF_EXPORT(HPDF_JavaScript)  HPDF_LoadJSFromFileW(                  HpdfDoc       * const doc, const wchar_t *filename);
HPDF_EXPORT(HPDF_U3D)	      HPDF_LoadU3DFromFileW(                 HpdfDoc       * const doc, const wchar_t *filename);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _HPDF_U3D_H */

