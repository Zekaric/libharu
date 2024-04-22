/*
 * << Haru Free PDF Library >> -- hpdf_array.c
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

HpdfArray *
   HPDF_Array_New(
      HpdfMemMgr * const mmgr)
{
   HpdfArray *obj;

   HPDF_PTRACE((" HPDF_Array_New\n"));

   obj = HpdfMemCreateType(mmgr, HpdfArray);
   if (obj)
   {
      HpdfMemClearType(obj, HpdfArray);

      obj->header.obj_class = HPDF_OCLASS_ARRAY;
      obj->mmgr             = mmgr;
      obj->error            = mmgr->error;
      obj->list             = HPDF_List_New(mmgr, HPDF_DEF_ITEMS_PER_BLOCK);
      if (!obj->list) 
      {
         HpdfMemDestroy(mmgr, obj);
         obj = NULL;
      }
   }

   return obj;
}

HpdfArray *
   HPDF_Box_Array_New(
      HpdfMemMgr * const mmgr,
      HPDF_Box   box)
{
   HpdfArray *obj;
   HpdfStatus ret = HPDF_OK;

   HPDF_PTRACE((" HPDF_Box_Array_New\n"));

   obj = HPDF_Array_New(mmgr);
   if (!obj)
   {
      return NULL;
   }

   ret += HPDF_Array_Add(obj, HpdfValueNumRealCreate(mmgr, box.left));
   ret += HPDF_Array_Add(obj, HpdfValueNumRealCreate(mmgr, box.bottom));
   ret += HPDF_Array_Add(obj, HpdfValueNumRealCreate(mmgr, box.right));
   ret += HPDF_Array_Add(obj, HpdfValueNumRealCreate(mmgr, box.top));

   if (ret != HPDF_OK) 
   {
      HPDF_Array_Free(obj);
      return NULL;
   }

   return obj;
}

void
   HPDF_Array_Free(
      HpdfArray * const array)
{
   if (!array)
      return;

   HPDF_PTRACE((" HPDF_Array_Free\n"));

   HPDF_Array_Clear(array);

   HPDF_List_Free(array->list);

   array->header.obj_class = 0;

   HpdfMemDestroy(array->mmgr, array);
}


HpdfStatus
   HPDF_Array_Write(
      HpdfArray const * const array,
      HPDF_Stream   stream,
      HPDF_Encrypt  e)
{
   HpdfUInt i;
   HpdfStatus ret;

   HPDF_PTRACE((" HPDF_Array_Write\n"));

   ret = HPDF_Stream_WriteStr(stream, "[ ");
   if (ret != HPDF_OK)
      return ret;

   for (i = 0; i < array->list->count; i++) {
      void * element = HPDF_List_ItemAt(array->list, i);

      ret = HPDF_Obj_Write(element, stream, e);
      if (ret != HPDF_OK)
         return ret;

      ret = HPDF_Stream_WriteChar(stream, ' ');
      if (ret != HPDF_OK)
         return ret;
   }

   ret = HPDF_Stream_WriteChar(stream, ']');

   return ret;
}

HpdfStatus
   HPDF_Array_AddNumber(
      HpdfArray * const array,
      HpdfInt32  value)
{
   HpdfValueNumInt *n = HpdfValueNumIntCreate(array->mmgr, value);

   HPDF_PTRACE((" HPDF_Array_AddNumber\n"));

   if (!n)
   {
      return HPDF_Error_GetCode(array->error);
   }

   return HPDF_Array_Add(array, n);
}

HpdfStatus
   HPDF_Array_AddReal(
      HpdfArray * const array,
      HpdfReal   value)
{
   HpdfValueNumReal *r = HpdfValueNumRealCreate(array->mmgr, value);

   HPDF_PTRACE((" HPDF_Array_AddReal\n"));

   if (!r)
   {
      return HPDF_Error_GetCode(array->error);
   }
   
   return HPDF_Array_Add(array, r);
}

HpdfStatus
   HPDF_Array_AddNull(  
      HpdfArray * const array)
{
   HpdfValueNull *n = HpdfValueNullCreate(array->mmgr);

   HPDF_PTRACE((" HPDF_Array_AddNull\n"));

   if (!n)
   {
      return HPDF_Error_GetCode(array->error);
   }

   return HPDF_Array_Add(array, n);
}

HpdfStatus
   HPDF_Array_AddName(
      HpdfArray * const array,
      char const *value)
{
   HpdfValueName *n = HpdfValueNameCreate(array->mmgr, value);

   HPDF_PTRACE((" HPDF_Array_AddName\n"));

   if (!n)
   {
      return HPDF_Error_GetCode(array->error);
   }
   
   return HPDF_Array_Add(array, n);
}

HpdfStatus
   HPDF_Array_Add(
      HpdfArray * const array,
      void        *obj)
{
   HPDF_Obj_Header *header;
   HpdfStatus ret;

   HPDF_PTRACE((" HPDF_Array_Add\n"));

   if (!obj) {
      if (HPDF_Error_GetCode(array->error) == HPDF_OK)
         return HPDF_SetError(array->error, HPDF_INVALID_OBJECT, 0);
      else
         return HPDF_INVALID_OBJECT;
   }

   header = (HPDF_Obj_Header *) obj;

   if (header->obj_id & HPDF_OTYPE_DIRECT)
      return HPDF_SetError(array->error, HPDF_INVALID_OBJECT, 0);

   if (array->list->count >= HPDF_LIMIT_MAX_ARRAY) {
      HPDF_PTRACE((" HPDF_Array_Add exceed limitatin of array count(%d)\n",
         HPDF_LIMIT_MAX_ARRAY));

      HPDF_Obj_Free(array->mmgr, obj);
      return HPDF_SetError(array->error, HPDF_ARRAY_COUNT_ERR, 0);
   }

   if (header->obj_id & HPDF_OTYPE_INDIRECT) {
      HPDF_Proxy proxy = HPDF_Proxy_New(array->mmgr, obj);

      if (!proxy) {
         HPDF_Obj_Free(array->mmgr, obj);
         return HPDF_Error_GetCode(array->error);
      }

      proxy->header.obj_id |= HPDF_OTYPE_DIRECT;
      obj = proxy;
   }
   else
      header->obj_id |= HPDF_OTYPE_DIRECT;

   ret = HPDF_List_Add(array->list, obj);
   if (ret != HPDF_OK)
      HPDF_Obj_Free(array->mmgr, obj);

   return ret;
}

HpdfUInt
   HPDF_Array_Items(
      HpdfArray const * const array)
{
   return array->list->count;
}

HpdfStatus
   HPDF_Array_Insert(
      HpdfArray * const array,
      void        *target,
      void        *obj)
{
   HPDF_Obj_Header *header;
   HpdfStatus ret;
   HpdfUInt i;

   HPDF_PTRACE((" HPDF_Array_Insert\n"));

   if (!obj) {
      if (HPDF_Error_GetCode(array->error) == HPDF_OK)
         return HPDF_SetError(array->error, HPDF_INVALID_OBJECT, 0);
      else
         return HPDF_INVALID_OBJECT;
   }

   header = (HPDF_Obj_Header *) obj;

   if (header->obj_id & HPDF_OTYPE_DIRECT) {
      HPDF_PTRACE((" HPDF_Array_Add this object cannot owned by array "
         "obj=0x%08X\n", (HpdfUInt) array));

      return HPDF_SetError(array->error, HPDF_INVALID_OBJECT, 0);
   }

   if (array->list->count >= HPDF_LIMIT_MAX_ARRAY) {
      HPDF_PTRACE((" HPDF_Array_Add exceed limitatin of array count(%d)\n",
         HPDF_LIMIT_MAX_ARRAY));

      HPDF_Obj_Free(array->mmgr, obj);

      return HPDF_SetError(array->error, HPDF_ARRAY_COUNT_ERR, 0);
   }

   if (header->obj_id & HPDF_OTYPE_INDIRECT) {
      HPDF_Proxy proxy = HPDF_Proxy_New(array->mmgr, obj);

      if (!proxy) {
         HPDF_Obj_Free(array->mmgr, obj);
         return HPDF_Error_GetCode(array->error);
      }

      proxy->header.obj_id |= HPDF_OTYPE_DIRECT;
      obj = proxy;
   }
   else
      header->obj_id |= HPDF_OTYPE_DIRECT;

   /* get the target-object from object-list
    * consider that the pointer contained in list may be proxy-object.
    */
   for (i = 0; i < array->list->count; i++) {
      void *ptr = HPDF_List_ItemAt(array->list, i);
      void *obj_ptr;

      header = (HPDF_Obj_Header *) ptr;
      if (header->obj_class == HPDF_OCLASS_PROXY)
         obj_ptr = ((HPDF_Proxy) ptr)->obj;
      else
         obj_ptr = ptr;

      if (obj_ptr == target) {
         ret = HPDF_List_Insert(array->list, ptr, obj);
         if (ret != HPDF_OK)
            HPDF_Obj_Free(array->mmgr, obj);

         return ret;
      }
   }

   HPDF_Obj_Free(array->mmgr, obj);

   return HPDF_ITEM_NOT_FOUND;
}


void *
   HPDF_Array_GetItem(
      HpdfArray * const array,
      HpdfUInt    index,
      HpdfUInt16  obj_class)
{
   void *obj;
   HPDF_Obj_Header *header;

   HPDF_PTRACE((" HPDF_Array_GetItem\n"));

   obj = HPDF_List_ItemAt(array->list, index);

   if (!obj) {
      HPDF_SetError(array->error, HPDF_ARRAY_ITEM_NOT_FOUND, 0);
      return NULL;
   }

   header = (HPDF_Obj_Header *) obj;

   if (header->obj_class == HPDF_OCLASS_PROXY) {
      obj = ((HPDF_Proxy) obj)->obj;
      header = (HPDF_Obj_Header *) obj;
   }

   if ((header->obj_class & HPDF_OCLASS_ANY) != obj_class) {
      HPDF_SetError(array->error, HPDF_ARRAY_ITEM_UNEXPECTED_TYPE, 0);

      return NULL;
   }

   return obj;
}

void
   HPDF_Array_Clear(
      HpdfArray * const array)
{
   HpdfUInt i;

   HPDF_PTRACE((" HPDF_Array_Clear\n"));

   if (!array)
      return;

   for (i = 0; i < array->list->count; i++) {
      void * obj = HPDF_List_ItemAt(array->list, i);

      if (obj) {
         HPDF_Obj_Free(array->mmgr, obj);
      }
   }

   HPDF_List_Clear(array->list);
}

