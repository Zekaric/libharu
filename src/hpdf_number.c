/*
 * << Haru Free PDF Library >> -- hpdf_number.c
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

HpdfObjNumInt *
   HpdfObjNumIntCreate(
      HpdfMemMgr * const mmgr,
      HpdfInt32  value)
{
   HpdfObjNumInt *obj = HpdfMemCreateType(mmgr, HpdfObjNumInt);
   if (obj)
   {
      HpdfMemClearType(&obj->header, HpdfObjHeader);
      obj->header.obj_class = HPDF_OCLASS_NUMBER;
      obj->value            = value;
   }

   return obj;
}

HpdfStatus
   HpdfObjNumIntWrite(
      HpdfObjNumInt const * const  obj,
      HPDF_Stream  stream)
{
   return HPDF_Stream_WriteInt(stream, obj->value);
}

void 
   HpdfObjNumIntSet(
      HpdfObjNumInt * const obj,
      HpdfInt32               value)
{
   obj->value = value;
}
