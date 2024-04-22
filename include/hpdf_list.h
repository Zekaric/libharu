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

typedef struct _HpdfList HpdfList;

struct _HpdfList
{
   HpdfMemMgr  *mmgr;
   HpdfError   *error;
   HpdfUInt     block_siz;
   HpdfUInt     items_per_block;
   HpdfUInt     count;
   void       **obj;
};

HpdfList    *HPDF_List_New(            HpdfMemMgr * const mmgr, HpdfUInt  items_per_block);
void         HPDF_List_Free(           HpdfList       * const  list);
HpdfStatus   HPDF_List_Add(            HpdfList       * const  list, void       *item);
HpdfStatus   HPDF_List_Insert(         HpdfList       * const  list, void       *target, void       *item);
HpdfStatus   HPDF_List_Remove(         HpdfList       * const  list, void       *item);
void        *HPDF_List_RemoveByIndex(  HpdfList       * const  list, HpdfUInt  index);
void        *HPDF_List_ItemAt(         HpdfList const * const  list, HpdfUInt  index);
HpdfInt32    HPDF_List_Find(           HpdfList const * const  list, void       *item);
void         HPDF_List_Clear(          HpdfList       * const  list);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _HPDF_LIST_H */

