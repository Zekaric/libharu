/*
 * << Haru Free PDF Library >> -- hpdf_binary.c
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


HpdfObjBinary *
   HpdfObjBinaryCreate(
      HpdfMemMgr * const mmgr,
      HpdfByte  * const value,
      HpdfUInt  len)
{
   HpdfObjBinary *obj;

   obj  = HpdfMemCreateType(mmgr, HpdfObjBinary);
   if (obj)
   {
      HpdfMemClearType(&obj->header, HpdfObjHeader);

      obj->header.obj_class = HPDF_OCLASS_BINARY;
      obj->mmgr             = mmgr;
      obj->error            = mmgr->error;
      obj->value            = NULL;
      obj->len              = 0;

      if (HpdfObjBinarySet(obj, value, len) != HPDF_OK) 
      {
         HpdfMemDestroy(mmgr, obj);
         return NULL;
      }
   }

   return obj;
}

HpdfStatus
   HpdfObjBinaryWrite(
      HpdfObjBinary const * const obj,
      HPDF_Stream   stream,
      HPDF_Encrypt  e)
{
   HpdfStatus ret;

   if (obj->len == 0)
   {
      return HPDF_Stream_WriteStr(stream, "<>");
   }

   ret = HPDF_Stream_WriteChar(stream, '<');
   if (ret != HPDF_OK)
   {
      return ret;
   }

   if (e) 
   {
      HPDF_Encrypt_Reset(e);
   }

   ret = HPDF_Stream_WriteBinary(stream, obj->value, obj->len, e);
   if (ret != HPDF_OK)
   {
      return ret;
   }

   return HPDF_Stream_WriteChar(stream, '>');
}

HpdfStatus
   HpdfObjBinarySet(
      HpdfObjBinary * const obj,
      HpdfByte    * const value,
      HpdfUInt    len)
{
   if (len > HPDF_LIMIT_MAX_STRING_LEN)
   {
      return HPDF_SetError(obj->error, HPDF_BINARY_LENGTH_ERR, 0);
   }

   if (obj->value) 
   {
      HpdfMemDestroy(obj->mmgr, obj->value);
      obj->len = 0;
   }

   obj->value = HpdfMemCreateTypeArray(obj->mmgr, HpdfByte, len);
   if (!obj->value)
   {
      return HPDF_Error_GetCode(obj->error);
   }

   HPDF_MemCpy(obj->value, value, len);
   obj->len = len;

   return HPDF_OK;
}

void
   HpdfObjBinaryDestroy(
      HpdfObjBinary * const obj)
{
   if (!obj)
   {
      return;
   }

   if (obj->value)
   {
      HpdfMemDestroy(obj->mmgr, obj->value);
   }

   HpdfMemDestroy(obj->mmgr, obj);
}

HpdfUInt
   HpdfObjBinaryGetLength(
      HpdfObjBinary const * const obj)
{
   return obj->len;
}

HpdfByte *
   HpdfObjBinaryGet(
      HpdfObjBinary const * const obj)
{
   return obj->value;
}
