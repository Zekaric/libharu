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


HPDF_NameDict HPDF_NameDict_New  (HpdfMemMgr * const mmgr, HPDF_Xref  xref);
HPDF_NameTree HPDF_NameDict_GetNameTree  (HPDF_NameDict     namedict, HPDF_NameDictKey  key); 
HpdfStatus HPDF_NameDict_SetNameTree  (HPDF_NameDict     namedict, HPDF_NameDictKey  key, HPDF_NameTree     tree);
HpdfBool HPDF_NameDict_Validate  (HPDF_NameDict  namedict);

/*------- NameTree -------*/

HPDF_NameTree HPDF_NameTree_New  (HpdfMemMgr * const mmgr, HPDF_Xref  xref);
HpdfStatus HPDF_NameTree_Add  (HPDF_NameTree  tree, HPDF_String    name, void          *obj);
HpdfBool HPDF_NameTree_Validate  (HPDF_NameTree  tree);

/*------- EmbeddedFile -------*/

HPDF_EmbeddedFile HPDF_EmbeddedFile_New  (HpdfMemMgr * const mmgr, HPDF_Xref  xref, const char *file);

#if defined(WIN32)
HPDF_EmbeddedFile HPDF_EmbeddedFile_NewW (HpdfMemMgr * const mmgr, HPDF_Xref  xref, const wchar_t *file);
#endif

HpdfBool HPDF_EmbeddedFile_Validate  (HPDF_EmbeddedFile  emfile);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _HPDF_NAMEDICT_H */

