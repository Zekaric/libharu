/*
 * << Haru Free PDF Library >> -- hpdf_null.c
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

#include "hpdf_utils.h"
#include "hpdf_objects.h"

HpdfObjNull *
   HpdfObjNullCreate(
      HpdfMemMgr * const mmgr)
{
   HpdfObjNull *obj = HpdfMemCreateType(mmgr, HpdfObjNull);
   if (obj)
   {
      HpdfMemClearType(&obj->header, HpdfObjHeader);
      obj->header.obj_class = HPDF_OCLASS_NULL;
   }

   return obj;
}
