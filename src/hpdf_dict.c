/*
 * << Haru Free PDF Library >> -- hpdf_dict.c
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

HpdfDictElement *GetElement(HPDF_Dict      dict, char const   *key);

/*--------------------------------------------------------------------------*/

HPDF_Dict
   HPDF_Dict_New(
      HpdfMemMgr * const mmgr)
{
   HPDF_Dict obj;

   obj = HpdfMemCreateType(mmgr, HPDF_Dict_Rec);
   if (obj)
   {
      HpdfMemClearType(obj, HPDF_Dict_Rec);
      obj->header.obj_class = HPDF_OCLASS_DICT;
      obj->mmgr = mmgr;
      obj->error = mmgr->error;
      obj->list = HPDF_List_New(mmgr, HPDF_DEF_ITEMS_PER_BLOCK);
      obj->filter = HPDF_STREAM_FILTER_NONE;
      if (!obj->list) {
         HpdfMemDestroy(mmgr, obj);
         obj = NULL;
      }
   }

   return obj;
}

HPDF_Dict
   HPDF_DictStream_New( 
      HpdfMemMgr * const mmgr,
      HPDF_Xref  xref)
{
   HPDF_Dict    obj;
   HpdfObjNumInt  *length;
   HpdfStatus   ret = 0;

   obj = HPDF_Dict_New(mmgr);
   if (!obj)
   {
      return NULL;
   }

   /* only stream object is added to xref automatically */
   ret += HPDF_Xref_Add(xref, obj);
   if (ret != HPDF_OK)
   {
      return NULL;
   }

   length = HpdfObjNumIntCreate(mmgr, 0);
   if (!length)
   {
      return NULL;
   }

   ret = HPDF_Xref_Add(xref, length);
   if (ret != HPDF_OK)
   {
      return NULL;
   }

   ret = HPDF_Dict_Add(obj, "Length", length);
   if (ret != HPDF_OK)
   {
      return NULL;
   }

   obj->stream = HPDF_MemStream_New(mmgr, HPDF_STREAM_BUF_SIZ);
   if (!obj->stream)
   {
      return NULL;
   }

   return obj;
}

void
HPDF_Dict_Free(HPDF_Dict  dict)
{
   HpdfUInt i;

   if (!dict)
      return;

   if (dict->free_fn)
      dict->free_fn(dict);

   for (i = 0; i < dict->list->count; i++)
   {
      HpdfDictElement *element = (HpdfDictElement *) HPDF_List_ItemAt(dict->list, i);
      if (element) 
      {
         HPDF_Obj_Free(dict->mmgr, element->value);
         HpdfMemDestroy(dict->mmgr, element);
      }
   }

   if (dict->stream)
      HPDF_Stream_Free(dict->stream);

   HPDF_List_Free(dict->list);

   dict->header.obj_class = 0;

   HpdfMemDestroy(dict->mmgr, dict);
}

HpdfStatus
HPDF_Dict_Add_FilterParams(HPDF_Dict    dict, HPDF_Dict filterParam)
{
   HpdfArray *paramArray;
   /* prepare params object */
   paramArray = HPDF_Dict_GetItem(dict, "DecodeParms",
      HPDF_OCLASS_ARRAY);
   if (paramArray==NULL) {
      paramArray = HPDF_Array_New(dict->mmgr);
      if (!paramArray)
         return HPDF_Error_GetCode(dict->error);

      /* add parameters */
      HPDF_Dict_Add(dict, "DecodeParms", paramArray);
   }
   HPDF_Array_Add(paramArray, filterParam);
   return HPDF_OK;
}


HpdfStatus
   HPDF_Dict_Write(
      HPDF_Dict     dict,
      HPDF_Stream   stream,
      HPDF_Encrypt  e)
{
   HpdfUInt   i;
   HpdfStatus ret;

   ret = HPDF_Stream_WriteStr(stream, "<<\012");
   if (ret != HPDF_OK)
   {
      return ret;
   }

   if (dict->before_write_fn) 
   {
      if ((ret = dict->before_write_fn(dict)) != HPDF_OK)
      {
         return ret;
      }
   }

   /* encrypt-dict must not be encrypted. */
   if (dict->header.obj_class == (HPDF_OCLASS_DICT | HPDF_OSUBCLASS_ENCRYPT))
   {
      e = NULL;
   }

   if (dict->stream) 
   {
      /* set filter element */
      if (dict->filter == HPDF_STREAM_FILTER_NONE)
      {
         HPDF_Dict_RemoveElement(dict, "Filter");
      }
      else 
      {
         HpdfArray *array = HPDF_Dict_GetItem(dict, "Filter", HPDF_OCLASS_ARRAY);

         if (!array) 
         {
            array = HPDF_Array_New(dict->mmgr);
            if (!array)
            {
               return HPDF_Error_GetCode(dict->error);
            }

            ret = HPDF_Dict_Add(dict, "Filter", array);
            if (ret != HPDF_OK)
            {
               return ret;
            }
         }

         HPDF_Array_Clear(array);

#ifdef LIBHPDF_HAVE_ZLIB
         if (dict->filter & HPDF_STREAM_FILTER_FLATE_DECODE)
         {
            HPDF_Array_AddName(array, "FlateDecode");
         }
#endif /* LIBHPDF_HAVE_ZLIB */

         if (dict->filter & HPDF_STREAM_FILTER_DCT_DECODE)
         {
            HPDF_Array_AddName(array, "DCTDecode");
         }

         if (dict->filter & HPDF_STREAM_FILTER_CCITT_DECODE)
         {
            HPDF_Array_AddName(array, "CCITTFaxDecode");
         }

         if (dict->filterParams!=NULL)
         {
            HPDF_Dict_Add_FilterParams(dict, dict->filterParams);
         }
      }
   }

   for (i = 0; i < dict->list->count; i++) 
   {
      HpdfDictElement *element = (HpdfDictElement *) HPDF_List_ItemAt(dict->list, i);
      HpdfObjHeader *header  = (HpdfObjHeader *) element->value;

      if (!element->value)
      {
         return HPDF_SetError(dict->error, HPDF_INVALID_OBJECT, 0);
      }

      if (header->obj_id & HPDF_OTYPE_HIDDEN) 
      {
         HPDF_PTRACE(
            (" HPDF_Dict_Write obj=%p skipped obj_id=0x%08X\n",
             element->value, 
             (HpdfUInt) header->obj_id));
      }
      else 
      {
         ret = HPDF_Stream_WriteEscapeName(stream, element->key);
         if (ret != HPDF_OK)
         {
            return ret;
         }

         ret = HPDF_Stream_WriteChar(stream, ' ');
         if (ret != HPDF_OK)
         {
            return ret;
         }

         ret = HPDF_Obj_Write(element->value, stream, e);
         if (ret != HPDF_OK)
         {
            return ret;
         }

         ret = HPDF_Stream_WriteStr(stream, "\012");
         if (ret != HPDF_OK)
         {
            return ret;
         }
      }
   }

   if (dict->write_fn) 
   {
      if ((ret = dict->write_fn(dict, stream)) != HPDF_OK)
      {
         return ret;
      }
   }

   if ((ret = HPDF_Stream_WriteStr(stream, ">>")) != HPDF_OK)
   {
      return ret;
   }

   if (dict->stream) 
   {
      HpdfUInt32   strptr;
      HpdfObjNumInt  *length;

      /* get "length" element */
      length = (HpdfObjNumInt *) HPDF_Dict_GetItem(dict, "Length", HPDF_OCLASS_NUMBER);
      if (!length)
      {
         return HPDF_SetError(dict->error, HPDF_DICT_STREAM_LENGTH_NOT_FOUND, 0);
      }

      /* "length" element must be indirect-object */
      if (!(length->header.obj_id & HPDF_OTYPE_INDIRECT)) 
      {
         return HPDF_SetError(dict->error, HPDF_DICT_ITEM_UNEXPECTED_TYPE, 0);
      }

      /* Acrobat 8.15 requires both \r and \n here */
      if ((ret = HPDF_Stream_WriteStr(stream, "\012stream\015\012")) != HPDF_OK)
      {
         return ret;
      }

      strptr = stream->size;

      if (e)
      {
         HPDF_Encrypt_Reset(e);
      }

      if ((ret = HPDF_Stream_WriteToStream(dict->stream, stream, dict->filter, e)) != HPDF_OK)
      {
         return ret;
      }

      HpdfObjNumIntSet(length, stream->size - strptr);

      ret = HPDF_Stream_WriteStr(stream, "\012endstream");
   }

   /* 2006.08.13 add. */
   if (dict->after_write_fn) 
   {
      if ((ret = dict->after_write_fn(dict)) != HPDF_OK)
      {
         return ret;
      }
   }

   return ret;
}

HpdfStatus
HPDF_Dict_Add(HPDF_Dict        dict,
   char const *key,
   void             *obj)
{
   HpdfObjHeader *header;
   HpdfStatus       ret = HPDF_OK;
   HpdfDictElement *element;

   if (!obj) {
      if (HPDF_Error_GetCode(dict->error) == HPDF_OK)
         return HPDF_SetError(dict->error, HPDF_INVALID_OBJECT, 0);
      else
         return HPDF_INVALID_OBJECT;
   }

   header = (HpdfObjHeader *) obj;

   if (header->obj_id & HPDF_OTYPE_DIRECT)
      return HPDF_SetError(dict->error, HPDF_INVALID_OBJECT, 0);

   if (!key) {
      HPDF_Obj_Free(dict->mmgr, obj);
      return HPDF_SetError(dict->error, HPDF_INVALID_OBJECT, 0);
   }

   if (dict->list->count >= HPDF_LIMIT_MAX_DICT_ELEMENT) {
      HPDF_PTRACE((" HPDF_Dict_Add exceed limitatin of dict count(%d)\n",
         HPDF_LIMIT_MAX_DICT_ELEMENT));

      HPDF_Obj_Free(dict->mmgr, obj);
      return HPDF_SetError(dict->error, HPDF_DICT_COUNT_ERR, 0);
   }

   /* check whether there is an object which has same name */
   element = GetElement(dict, key);

   if (element) {
      HPDF_Obj_Free(dict->mmgr, element->value);
      element->value = NULL;
   }
   else
   {
      element = HpdfMemCreateType(dict->mmgr, HpdfDictElement);
      if (!element) {
         /* cannot create element object */
         if (!(header->obj_id & HPDF_OTYPE_INDIRECT))
            HPDF_Obj_Free(dict->mmgr, obj);

         return HPDF_Error_GetCode(dict->error);
      }

      HPDF_StrCpy(element->key, key, element->key +
         HPDF_LIMIT_MAX_NAME_LEN + 1);
      element->value = NULL;

      ret = HPDF_List_Add(dict->list, element);
      if (ret != HPDF_OK) {
         if (!(header->obj_id & HPDF_OTYPE_INDIRECT))
            HPDF_Obj_Free(dict->mmgr, obj);

         HpdfMemDestroy(dict->mmgr, element);

         return HPDF_Error_GetCode(dict->error);
      }
   }

   if (header->obj_id & HPDF_OTYPE_INDIRECT) {
      HPDF_Proxy proxy = HPDF_Proxy_New(dict->mmgr, obj);

      if (!proxy) {
         HPDF_Obj_Free(dict->mmgr, obj);
         return HPDF_Error_GetCode(dict->error);
      }

      element->value = proxy;
      proxy->header.obj_id |= HPDF_OTYPE_DIRECT;
   }
   else {
      element->value = obj;
      header->obj_id |= HPDF_OTYPE_DIRECT;
   }

   return ret;
}

HpdfStatus
   HPDF_Dict_AddName(
      HPDF_Dict        dict,
      char const *key,
      char const *value)
{
   HpdfObjName *name = HpdfObjNameCreate(dict->mmgr, value);
   if (!name)
   {
      return HPDF_Error_GetCode(dict->error);
   }

   return HPDF_Dict_Add(dict, key, name);
}

HpdfStatus
   HPDF_Dict_AddNumber(
      HPDF_Dict   dict,
      char const *key,
      HpdfInt32   value)
{
   HpdfObjNumInt *number = HpdfObjNumIntCreate(dict->mmgr, value);
   if (!number)
   {
      return HPDF_Error_GetCode(dict->error);
   }

   return HPDF_Dict_Add(dict, key, number);
}

HpdfStatus
HPDF_Dict_AddReal(HPDF_Dict        dict,
   char const *key,
   HpdfReal        value)
{
   HpdfObjNumReal *real = HpdfObjNumRealCreate(dict->mmgr, value);

   if (!real)
   {
      return HPDF_Error_GetCode(dict->error);
   }

   return HPDF_Dict_Add(dict, key, real);
}

HpdfStatus
   HPDF_Dict_AddBoolean(
      HPDF_Dict     dict,
      char const   *key,
      HpdfBool      value)
{
   HpdfObjBool *obj = HpdfObjBoolCreate(dict->mmgr, value);

   if (!obj)
   {
      return HPDF_Error_GetCode(dict->error);
   }

   return HPDF_Dict_Add(dict, key, obj);
}

void*
HPDF_Dict_GetItem(HPDF_Dict        dict,
   char const *key,
   HpdfUInt16      obj_class)
{
   HpdfDictElement *element = GetElement(dict, key);
   void *obj;

   if (element && HpdfStrIsEqual(key, element->key))
   {
      HpdfObjHeader *header = (HpdfObjHeader *) element->value;

      if (header->obj_class == HPDF_OCLASS_PROXY) {
         HPDF_Proxy p = element->value;
         header = (HpdfObjHeader *) p->obj;
         obj = p->obj;
      }
      else
         obj = element->value;

      if ((header->obj_class & HPDF_OCLASS_ANY) != obj_class) {
         HPDF_PTRACE((" HPDF_Dict_GetItem dict=%p key=%s obj_class=0x%08X\n",
            dict, key, (HpdfUInt) header->obj_class));
         HPDF_SetError(dict->error, HPDF_DICT_ITEM_UNEXPECTED_TYPE, 0);

         return NULL;
      }

      return obj;
   }

   return NULL;
}


HpdfDictElement *
   GetElement(
      HPDF_Dict   dict,
      char const *key)
{
   HpdfUInt i;

   for (i = 0; i < dict->list->count; i++) 
   {
      HpdfDictElement *element = (HpdfDictElement *) HPDF_List_ItemAt(dict->list, i);

      if (HpdfStrIsEqual(key, element->key))
      {
         return element;
      }
   }

   return NULL;
}


HpdfStatus
HPDF_Dict_RemoveElement(HPDF_Dict        dict,
   char const *key)
{
   HpdfUInt i;

   for (i = 0; i < dict->list->count; i++) 
   {
      HpdfDictElement *element = (HpdfDictElement *) HPDF_List_ItemAt(dict->list, i);

      if (HpdfStrIsEqual(key, element->key))
      {
         HPDF_List_Remove(dict->list, element);

         HPDF_Obj_Free(dict->mmgr, element->value);
         HpdfMemDestroy(dict->mmgr, element);

         return HPDF_OK;
      }
   }

   return HPDF_DICT_ITEM_NOT_FOUND;
}

char const*
HPDF_Dict_GetKeyByObj(HPDF_Dict  dict,
   void       *obj)
{
   HpdfUInt i;

   for (i = 0; i < dict->list->count; i++) {
      HpdfObjHeader *header;
      HpdfDictElement *element = (HpdfDictElement *) HPDF_List_ItemAt(dict->list, i);

      header = (HpdfObjHeader *) (element->value);
      if (header->obj_class == HPDF_OCLASS_PROXY) {
         HPDF_Proxy p = element->value;

         if (p->obj == obj)
            return element->key;
      }
      else {
         if (element->value == obj)
            return element->key;
      }
   }

   return NULL;
}

