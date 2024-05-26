/*
 * << Haru Free PDF Library >> -- hpdf_objects.c
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

void
   HPDF_Obj_Free(
      HpdfMemMgr * const mmgr,
      void         *obj)
{
   HpdfObjHeader *header;

   HPDF_PTRACE((" HPDF_Obj_Free\n"));

   if (!obj)
      return;

   header = (HpdfObjHeader *) obj;

   if (!(header->obj_id & HPDF_OTYPE_INDIRECT))
      HPDF_Obj_ForceFree(mmgr, obj);
}


void
   HPDF_Obj_ForceFree(
      HpdfMemMgr * const mmgr,
      void              *obj)
{
   HpdfObjHeader *header;

   HPDF_PTRACE((" HPDF_Obj_ForceFree\n"));

   if (!obj)
   {
      return;
   }

   header = (HpdfObjHeader *) obj;

   HPDF_PTRACE(
      (" HPDF_Obj_ForceFree obj=0x%08X obj_id=0x%08X obj_class=0x%08X\n",
       (HpdfUInt) obj, (HpdfUInt) (header->obj_id),
       (HpdfUInt) (header->obj_class)));

   switch (header->obj_class & HPDF_OCLASS_ANY) 
   {
   case HPDF_OCLASS_STRING:
      HpdfObjStringDestroy(obj);
      break;

   case HPDF_OCLASS_BINARY:
      HpdfObjBinaryDestroy(obj);
      break;

   case HPDF_OCLASS_ARRAY:
      HpdfArrayDestroy(obj);
      break;

   case HPDF_OCLASS_DICT:
      HPDF_Dict_Free(obj);
      break;

   case HPDF_OCLASS_DIRECT:
      HPDF_Direct_Free(obj);
      break;

   default:
      HpdfMemDestroy(mmgr, obj);
   }
}

HpdfStatus
HPDF_Obj_Write(void          *obj,
   HPDF_Stream   stream,
   HPDF_Encrypt  e)
{
   HpdfObjHeader *header = (HpdfObjHeader *) obj;

   HPDF_PTRACE((" HPDF_Obj_Write\n"));

   if (header->obj_id & HPDF_OTYPE_HIDDEN) {
      HPDF_PTRACE(("#HPDF_Obj_Write obj=0x%08X skipped\n", (HpdfUInt) obj));
      return HPDF_OK;
   }

   if (header->obj_class == HPDF_OCLASS_PROXY) {
      char buf[HPDF_SHORT_BUF_SIZ];
      char *pbuf = buf;
      char *eptr = buf + HPDF_SHORT_BUF_SIZ - 1;
      HPDF_Proxy p = obj;

      header = (HpdfObjHeader*) p->obj;

      pbuf = HPDF_IToA(pbuf, header->obj_id & 0x00FFFFFF, eptr);
      *pbuf++ = ' ';
      pbuf = HPDF_IToA(pbuf, header->gen_no, eptr);
      HPDF_StrCpy(pbuf, " R", eptr);

      return HPDF_Stream_WriteStr(stream, buf);
   }

   return HPDF_Obj_WriteValue(obj, stream, e);
}

HpdfStatus
   HPDF_Obj_WriteValue(
      void           *obj,
      HPDF_Stream     stream,
      HPDF_Encrypt    e)
{
   HpdfObjHeader   *header;
   HpdfStatus         ret;

   HPDF_PTRACE((" HPDF_Obj_WriteValue\n"));

   header = (HpdfObjHeader *) obj;

   HPDF_PTRACE(
      (" HPDF_Obj_WriteValue obj=0x%08X obj_class=0x%04X\n",
       (HpdfUInt) obj, 
       (HpdfUInt) header->obj_class));

   switch (header->obj_class & HPDF_OCLASS_ANY) 
   {
   case HPDF_OCLASS_NAME:
      ret = HpdfObjNameWrite(obj, stream);
      break;
   
   case HPDF_OCLASS_NUMBER:
      ret = HpdfObjNumIntWrite(obj, stream);
      break;
   
   case HPDF_OCLASS_REAL:
      ret = HpdfObjNumRealWrite(obj, stream);
      break;
   
   case HPDF_OCLASS_STRING:
      ret = HpdfObjStringWrite(obj, stream, e);
      break;
   
   case HPDF_OCLASS_BINARY:
      ret = HpdfObjBinaryWrite(obj, stream, e);
      break;
   
   case HPDF_OCLASS_ARRAY:
      ret = HpdfArrayWrite(obj, stream, e);
      break;
   
   case HPDF_OCLASS_DICT:
      ret = HPDF_Dict_Write(obj, stream, e);
      break;
   
   case HPDF_OCLASS_BOOLEAN:
      ret = HpdfObjBoolWrite(obj, stream);
      break;

   case HPDF_OCLASS_DIRECT:
      ret = HPDF_Direct_Write(obj, stream);
      break;
   
   case HPDF_OCLASS_NULL:
      ret = HPDF_Stream_WriteStr(stream, "null");
      break;
   
   default:
      ret = HPDF_ERR_UNKNOWN_CLASS;
   }

   return ret;
}

HPDF_Proxy
   HPDF_Proxy_New(
      HpdfMemMgr * const mmgr,
      void       *obj)
{
   HPDF_Proxy p = HpdfMemCreateType(mmgr, HPDF_Proxy_Rec);

   HPDF_PTRACE((" HPDF_Proxy_New\n"));

   if (p)
   {
      HpdfMemClearType(&p->header, HpdfObjHeader);
      p->header.obj_class = HPDF_OCLASS_PROXY;
      p->obj = obj;
   }

   return p;
}

