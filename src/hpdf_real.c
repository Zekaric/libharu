/*
 * << Haru Free PDF Library >> -- hpdf_real.c
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

#include "hpdf_conf.h"
#include "hpdf_utils.h"
#include "hpdf_objects.h"


HPDF_Real
   HPDF_Real_New(
      HpdfMemMgr * const mmgr,
      HpdfReal  value)
{
   HPDF_Real obj = HpdfMemCreateType(mmgr, HPDF_Real_Rec);
   if (obj)
   {
      HpdfMemClearType(&obj->header, HPDF_Obj_Header);
      obj->header.obj_class = HPDF_OCLASS_REAL;
      obj->error = mmgr->error;
      HPDF_Real_SetValue(obj, value);
   }

   return obj;
}


HpdfStatus
HPDF_Real_Write(HPDF_Real    obj,
   HPDF_Stream  stream)
{
   return HPDF_Stream_WriteReal(stream, obj->value);
}


HpdfStatus
HPDF_Real_SetValue(HPDF_Real  obj,
   HpdfReal  value)
{
   HpdfStatus ret = HPDF_OK;

   if (value > HPDF_LIMIT_MAX_REAL)
      return HPDF_SetError(obj->error, HpdfReal_OUT_OF_RANGE, 0);

   if (value < HPDF_LIMIT_MIN_REAL)
      return HPDF_SetError(obj->error, HpdfReal_OUT_OF_RANGE, 0);

   obj->value =value;

   return ret;
}

