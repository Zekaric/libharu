/*
 * << Haru Free PDF Library >> -- hpdf_namedict.h
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

#ifndef _HPDF_NAMEDICT_H
#define _HPDF_NAMEDICT_H

#include "hpdf_objects.h"

#ifdef __cplusplus
extern "C" {
#endif


HpdfValueNameDict HpdfValueNameDict_New  (HpdfMemMgr * const mmgr, HPDF_Xref  xref);
HpdfValueNameTree HpdfValueNameDict_GetNameTree  (HpdfValueNameDict     namedict, HpdfValueNameDictKey  key); 
HpdfStatus HpdfValueNameDict_SetNameTree  (HpdfValueNameDict     namedict, HpdfValueNameDictKey  key, HpdfValueNameTree     tree);
HpdfBool HpdfValueNameDict_Validate  (HpdfValueNameDict  namedict);

/*------- NameTree -------*/

HpdfValueNameTree HpdfValueNameTree_New  (HpdfMemMgr * const mmgr, HPDF_Xref  xref);
HpdfStatus HpdfValueNameTree_Add  (HpdfValueNameTree  tree, HpdfValueString * const name, void          *obj);
HpdfBool HpdfValueNameTree_Validate  (HpdfValueNameTree  tree);

/*------- EmbeddedFile -------*/

HPDF_EmbeddedFile HPDF_EmbeddedFile_New  (HpdfMemMgr * const mmgr, HPDF_Xref  xref, char const *file);

#if defined(WIN32)
HPDF_EmbeddedFile HPDF_EmbeddedFile_NewW (HpdfMemMgr * const mmgr, HPDF_Xref  xref, wchar_t const *file);
#endif

HpdfBool HPDF_EmbeddedFile_Validate  (HPDF_EmbeddedFile  emfile);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _HPDF_NAMEDICT_H */

