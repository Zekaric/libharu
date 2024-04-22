/*
 * << Haru Free PDF Library >> -- hpdf_name.c
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

HpdfValueName *
   HpdfValueNameCreate(
      HpdfMemMgr * const mmgr,
      char const        *value)
{
   HpdfValueName *obj;

   obj  = HpdfMemCreateType(mmgr, HpdfValueName);
   if (obj)
   {
      HpdfMemClearType(&obj->header, HPDF_Obj_Header);
      obj->header.obj_class = HPDF_OCLASS_NAME;
      obj->error            = mmgr->error;

      if (HpdfValueNameSet(obj, value) == HPDF_NAME_INVALID_VALUE) 
      {
         HpdfMemDestroy(mmgr, obj);
         return NULL;
      }
   }

   return obj;
}

HpdfStatus
   HpdfValueNameWrite(
      HpdfValueName const * const obj,
      HPDF_Stream                 stream)
{
   return HPDF_Stream_WriteEscapeName(stream, obj->value);
}

HpdfStatus
   HpdfValueNameSet(
      HpdfValueName * const obj,
      char const           *value)
{
   if (!value || 
       value[0] == 0)
   {
      return HPDF_SetError(obj->error, HPDF_NAME_INVALID_VALUE, 0);
   }

   if (HPDF_StrLen(value, HPDF_LIMIT_MAX_NAME_LEN + 1) > HPDF_LIMIT_MAX_NAME_LEN)
   {
      return HPDF_SetError(obj->error, HPDF_NAME_OUT_OF_RANGE, 0);
   }

   HPDF_StrCpy(obj->value, value, obj->value + HPDF_LIMIT_MAX_NAME_LEN);

   return HPDF_OK;
}

char const *
   HpdfValueNameGet(
      HpdfValueName const * const obj)
{
   return (char const *) obj->value;
}
