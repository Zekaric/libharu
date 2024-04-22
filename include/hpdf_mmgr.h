/*
 * << Haru Free PDF Library >> -- hpdf_mmgr.h
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

#ifndef _HPDF_MMGR_H
#define _HPDF_MMGR_H

#include "hpdf_types.h"
#include "hpdf_error.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _HpdfMemPoolNode HpdfMemPoolNode;

struct _HpdfMemPoolNode
{
   HpdfByte          *buf;
   HpdfUInt           size;
   HpdfUInt           used_size;
   HpdfMemPoolNode   *next_node;
};


typedef struct _HpdfMemMgr HpdfMemMgr;

struct _HpdfMemMgr 
{
   HpdfError         *error;
   HPDF_Alloc_Func    alloc_fn;
   HPDF_Free_Func     free_fn;
   HpdfMemPoolNode   *mpool;
   HpdfUInt           buf_size;

#ifdef HPDF_MEM_DEBUG
   HpdfUInt           alloc_cnt;
   HpdfUInt           free_cnt;
#endif
};

/*  HPDF_mpool_new
**
**  create new HPDF_mpool object. when memory allocation goes wrong,
**  it returns NULL and error handling function will be called.
**  if buf_size is non-zero, mmgr is configured to be using memory-pool */
HpdfMemMgr  *HpdfMemMgrCreate(   HpdfError * const error, HpdfUInt buf_size, HPDF_Alloc_Func  alloc_fn, HPDF_Free_Func free_fn);
void         HpdfMemMgrDestroy(  HpdfMemMgr * const mmgr);
void        *HpdfMemCreate(      HpdfMemMgr * const mmgr, HpdfUInt size);
void         HpdfMemDestroy(     HpdfMemMgr * const mmgr, void *aptr);

#define HpdfMemCreateType(     MEMMGR, TYPE)          (TYPE *) HpdfMemCreate(MEMMGR, sizeof(TYPE))
#define HpdfMemCreateTypeArray(MEMMGR, TYPE, COUNT)   (TYPE *) HpdfMemCreate(MEMMGR, sizeof(TYPE) * (COUNT))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _HPDF_MMGR_H */

