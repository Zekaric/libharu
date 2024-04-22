/*
 * << Haru Free PDF Library >> -- hpdf_list.c
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
#include "hpdf_consts.h"
#include "hpdf_list.h"

static HpdfStatus Resize(HpdfList * const list, HpdfUInt count);

/*
 *  HPDF_List_new
 *
 *  mmgr :  handle to a HpdfMemMgr * object.
 *  items_per_block :  number of increases of pointers.
 *
 *  return:  If HPDF_List_New success, it returns a handle to new HPDF_List
 *           object, otherwise it returns NULL.
 *
 */

HpdfList *
   HPDF_List_New(
      HpdfMemMgr * const mmgr,
      HpdfUInt  items_per_block)
{
   HpdfList *list;

   HPDF_PTRACE((" HPDF_List_New\n"));

   if (mmgr == NULL)
      return NULL;

   list = HpdfMemCreateType(mmgr, HpdfList);
   if (list) 
   {
      list->mmgr            = mmgr;
      list->error           = mmgr->error;
      list->block_siz       = 0;
      list->items_per_block = (items_per_block <= 0 ? HPDF_DEF_ITEMS_PER_BLOCK : items_per_block);
      list->count           = 0;
      list->obj             = NULL;
   }

   return list;
}

/*
 *  HPDF_List_add
 *
 *  list :  Pointer to a HpdfList const * const object.
 *  item :  Pointer to a object to be added.
 *
 *  return:  If HPDF_List_Add success, it returns HPDF_OK.
 *           HPDF_FAILD_TO_ALLOC_MEM is returned when the expansion of the
 *           object list is failed.
 *
 */

HpdfStatus
   HPDF_List_Add(
      HpdfList * const  list,
      void       *item)
{
   HPDF_PTRACE((" HPDF_List_Add\n"));

   if (list->count >= list->block_siz) 
   {
      HpdfStatus ret = Resize(list, list->block_siz + list->items_per_block);

      if (ret != HPDF_OK) 
      {
         return ret;
      }
   }

   list->obj[list->count++] = item;
   return HPDF_OK;
}


/*
 *  HPDF_List_Insert
 *
 *  list   :  Pointer to a HpdfList const * const object.
 *  target :  Pointer to the target object.
 *  item   :  Pointer to a object to be inserted.
 *
 *  insert the item before the target.
 *
 *  return:  If HPDF_List_Add success, it returns HPDF_OK.
 *           HPDF_FAILD_TO_ALLOC_MEM is returned when the expansion of the
 *           object list is failed.
 *           HPDF_ITEM_NOT_FOUND is returned where the target object is not
 *           found.
 *
 */

HpdfStatus
   HPDF_List_Insert(
      HpdfList * const  list,
      void       *target,
      void       *item)
{
   HpdfInt target_idx = HPDF_List_Find(list, target);
   void   *last_item  = list->obj[list->count - 1];
   HpdfInt i;

   HPDF_PTRACE((" HPDF_List_Insert\n"));

   if (target_idx < 0)
   {
      return HPDF_ITEM_NOT_FOUND;
   }

   /* move the item of the list to behind one by one. */
   for (i = list->count - 2; i >= target_idx; i--)
   {
      list->obj[i + 1] = list->obj[i];
   }

   list->obj[target_idx] = item;

   return HPDF_List_Add(list, last_item);
}

/*
 *  HPDF_List_Remove
 *
 *  Remove the object specified by item parameter from the list object. The
 *  memory area that the object uses is not released.
 *
 *  list :  Pointer to a HpdfList const * const object.
 *  item :  Pointer to a object to be remove.
 *
 *  return:  If HPDF_List_Remove success, it returns HPDF_OK.
 *           HPDF_ITEM_NOT_FOUND is returned when the object specified by item
 *           parameter is not found.
 *
 */

HpdfStatus
   HPDF_List_Remove(
      HpdfList * const list,
      void            *item)
{
   HpdfUInt i;
   void **obj = list->obj;

   HPDF_PTRACE((" HPDF_List_Remove\n"));

   for (i = 0; i < list->count; i++) 
   {
      if (*obj == item) 
      {
         HPDF_List_RemoveByIndex(list, i);
         return HPDF_OK;
      }
      else
      {
         obj++;
      }
   }

   return HPDF_ITEM_NOT_FOUND;
}

/*
 *  HPDF_List_RemoveByIndex
 *
 *  Remove the object by index number.
 *
 *  list :  Pointer to a HpdfList const * const object.
 *  index :  Index of a object to be remove.
 *
 *  return:  If HPDF_List_RemoveByIndex success, it returns HPDF_OK.
 *           HPDF_ITEM_NOT_FOUND is returned when the value which is specified
 *           by index parameter is invalid.
 *
 */

void *
   HPDF_List_RemoveByIndex(
      HpdfList * const list,
      HpdfUInt  index)
{
   void *tmp;

   HPDF_PTRACE((" HPDF_List_RemoveByIndex\n"));

   if (list->count <= index)
      return NULL;

   tmp = list->obj[index];

   while (index < list->count - 1) {
      list->obj[index] = list->obj[index + 1];
      index++;
   }

   list->count--;

   return tmp;
}

/*
 *  HPDF_List_ItemAt
 *
 *  list :  Pointer to a HpdfList const * const object.
 *  index :  Index of a object.
 *
 *  return:  If HPDF_List_at success, it returns a pointer to the object.
 *           otherwise it returns NULL.
 *
 */

void *
   HPDF_List_ItemAt(
      HpdfList const * const  list,
      HpdfUInt  index)
{
   HPDF_PTRACE((" HPDF_List_ItemAt\n"));

   return (list->count <= index) ? NULL : list->obj[index];
}

/*
 *  HPDF_List_free
 *
 *  list :  Pointer to a HpdfList const * const object.
 *
 */

void
   HPDF_List_Free(
      HpdfList * const  list)
{
   HPDF_PTRACE((" HPDF_List_Free\n"));

   if (!list)
      return;

   HPDF_List_Clear(list);
   HpdfMemDestroy(list->mmgr, list);
}

/*
 *  HPDF_List_Clear
 *
 *  list :  Pointer to a HpdfList const * const object.
 *
 */

void
   HPDF_List_Clear(
      HpdfList * const list)
{
   HPDF_PTRACE((" HPDF_List_Clear\n"));

   if (list->obj)
      HpdfMemDestroy(list->mmgr, list->obj);

   list->block_siz = 0;
   list->count = 0;
   list->obj = NULL;
}

/*
 *  Resize
 *
 *  list :  Pointer to a HpdfList const * const object.
 *  count : The size of array of pointers.
 *
 *  return:  If Resize success, it returns HPDF_OK.
 *           otherwise it returns error-code which is set by HpdfMemMgr * object.
 *
 */

static HpdfStatus
   Resize(
      HpdfList * const list,
      HpdfUInt         count)
{
   void **new_obj;

   HPDF_PTRACE((" HPDF_List_Resize\n"));

   if (list->count >= count) 
   {
      if (list->count == count)
      {
         return HPDF_OK;
      }
      else
      {
         return HPDF_INVALID_PARAMETER;
      }
   }

   new_obj = HpdfMemCreateTypeArray(list->mmgr, void *, count);
   if (!new_obj)
   {
      return HPDF_Error_GetCode(list->error);
   }

   if (list->obj)
   {
      HPDF_MemCpy((HpdfByte *) new_obj, (HpdfByte *) list->obj, list->block_siz * sizeof(void *));
   }

   list->block_siz = count;
   if (list->obj)
   {
      HpdfMemDestroy(list->mmgr, list->obj);
   }
   list->obj = new_obj;

   return HPDF_OK;
}

/*
 *  HPDF_List_Find
 *
 *  list :  Pointer to a HpdfList const * const object.
 *  count : the size of array of pointers.
 *
 *  return:  If HPDF_List_Find success, it returns index of the object.
 *           otherwise it returns negative value.
 *
 */

HpdfInt32
   HPDF_List_Find(
      HpdfList const * const list,
      void                  *item)
{
   HpdfUInt i;

   HPDF_PTRACE((" HPDF_List_Find\n"));

   for (i = 0; i < list->count; i++) 
   {
      if (list->obj[i] == item)
      {
         return i;
      }
   }

   return -1;
}

