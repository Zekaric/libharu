/*
 * << Haru Free PDF Library >> -- hpdf_list.h
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

#ifndef _HPDF_LIST_H
#define _HPDF_LIST_H

#include "hpdf_error.h"
#include "hpdf_mmgr.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _HPDF_List_Rec  *HPDF_List;

typedef struct _HPDF_List_Rec 
{
      HpdfMemMgr        *mmgr;
      HpdfError      *error;
      HpdfUInt        block_siz;
      HpdfUInt        items_per_block;
      HpdfUInt        count;
      void          **obj;
} HPDF_List_Rec;

HPDF_List HPDF_List_New  (HpdfMemMgr * const mmgr, HpdfUInt  items_per_block);
void HPDF_List_Free  (HPDF_List  list);
HpdfStatus HPDF_List_Add  (HPDF_List  list, void       *item);
HpdfStatus HPDF_List_Insert  (HPDF_List  list, void       *target, void       *item);
HpdfStatus HPDF_List_Remove  (HPDF_List  list, void       *item);
void *HPDF_List_RemoveByIndex  (HPDF_List  list, HpdfUInt  index);
void *HPDF_List_ItemAt  (HPDF_List  list, HpdfUInt  index);
HpdfInt32 HPDF_List_Find  (HPDF_List  list, void       *item);
void HPDF_List_Clear  (HPDF_List  list);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _HPDF_LIST_H */

