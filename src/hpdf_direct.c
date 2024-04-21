#include "hpdf_conf.h"
#include "hpdf_utils.h"
#include "hpdf_objects.h"


HpdfDirect *
   HPDF_Direct_New(
      HpdfMemMgr * const mmgr,
      HpdfByte  * const value,
      HpdfUInt  len)
{
   HpdfDirect *obj;

   obj = HpdfMemCreateType(mmgr, HpdfDirect);
   if (obj)
   {
      HpdfMemClearType(&obj->header, HPDF_Obj_Header);
      
      obj->header.obj_class = HPDF_OCLASS_DIRECT;
      obj->mmgr             = mmgr;
      obj->error            = mmgr->error;
      obj->value            = NULL;
      obj->len              = 0;
      
      if (HPDF_Direct_SetValue(obj, value, len) != HPDF_OK) 
      {
         HpdfMemDestroy(mmgr, obj);
         return NULL;
      }
   }

   return obj;
}

HpdfStatus
   HPDF_Direct_Write(
      HpdfDirect const * const obj,
      HPDF_Stream   stream)
{
   return HPDF_Stream_Write(stream, obj->value, obj->len);
}

HpdfStatus
   HPDF_Direct_SetValue(
      HpdfDirect * const obj,
      HpdfByte    * const value,
      HpdfUInt    len)
{
   if (len > HPDF_LIMIT_MAX_STRING_LEN)
      return HPDF_SetError(obj->error, HPDF_BINARY_LENGTH_ERR, 0);

   if (obj->value) {
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
   HPDF_Direct_Free(
      HpdfDirect * const obj)
{
   if (!obj)
      return;

   if (obj->value)
      HpdfMemDestroy(obj->mmgr, obj->value);

   HpdfMemDestroy(obj->mmgr, obj);
}
