/*
 * << Haru Free PDF Library >> -- hpdf_destination.c
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
#include "hpdf.h"

char const * const HPDF_DESTINATION_TYPE_NAMES[] = {
        "XYZ",
        "Fit",
        "FitH",
        "FitV",
        "FitR",
        "FitB",
        "FitBH",
        "FitBV",
        NULL
};

/*----------------------------------------------------------------------------*/
/*----- HpdfDestination ------------------------------------------------------*/

HpdfDestination *
   HPDF_Destination_New(
      HpdfMemMgr * const mmgr,
      HPDF_Page   target,
      HPDF_Xref   xref)
{
   HpdfDestination *dst;

   HPDF_PTRACE((" HPDF_Destination_New\n"));

   if (!HPDF_Page_Validate(target)) 
   {
      HPDF_SetError(mmgr->error, HPDF_INVALID_PAGE, 0);
      return NULL;
   }

   dst = HpdfArrayCreate(mmgr);
   if (!dst)
   {
      return NULL;
   }

   dst->header.obj_class |= HPDF_OSUBCLASS_DESTINATION;

   if (HPDF_Xref_Add(xref, dst) != HPDF_OK)
      return NULL;

   /* first item of array must be target page */
   if (HpdfArrayAdd(dst, target) != HPDF_OK)
   {
      return NULL;
   }

   /* default type is HPDF_FIT */
   if (HpdfArrayAddName(dst, HPDF_DESTINATION_TYPE_NAMES[(HpdfInt) HPDF_FIT]) != HPDF_OK)
   {
      return NULL;
   }

   return dst;
}


HpdfBool
HPDF_Destination_Validate(
   HpdfDestination * const dst)
{
   HpdfObjHeader *header = (HpdfObjHeader *) dst;
   HPDF_Page target;

   if (!dst || header->obj_class !=
      (HPDF_OCLASS_ARRAY | HPDF_OSUBCLASS_DESTINATION))
      return HPDF_FALSE;

   /* destination-types not defined. */
   if (dst->list->count < 2)
      return HPDF_FALSE;

   target = (HPDF_Page) HpdfArrayGetItem(dst, 0, HPDF_OCLASS_DICT);
   if (!HPDF_Page_Validate(target)) {
      HPDF_SetError(dst->error, HPDF_INVALID_PAGE, 0);
      return HPDF_FALSE;
   }

   return HPDF_TRUE;
}


HPDF_EXPORT(HpdfStatus)
HPDF_Destination_SetXYZ(HpdfDestination * const dst,
   HpdfReal         left,
   HpdfReal         top,
   HpdfReal         zoom)
{
   HpdfStatus ret = HPDF_OK;
   HPDF_Page target;

   HPDF_PTRACE((" HPDF_Destination_SetXYZ\n"));

   if (!HPDF_Destination_Validate(dst))
      return HPDF_INVALID_DESTINATION;

   if (left < 0 || top < 0 || zoom < 0.08 || zoom > 32)
      return HPDF_RaiseError(dst->error, HPDF_INVALID_PARAMETER, 0);


   target = (HPDF_Page) HpdfArrayGetItem(dst, 0, HPDF_OCLASS_DICT);

   if (dst->list->count > 1)
   {
      HpdfArrayClear(dst);
      ret += HpdfArrayAdd(dst, target);
   }

   ret += HpdfArrayAddName(dst, HPDF_DESTINATION_TYPE_NAMES[(HpdfInt) HPDF_XYZ]);
   ret += HpdfArrayAddReal(dst, left);
   ret += HpdfArrayAddReal(dst, top);
   ret += HpdfArrayAddReal(dst, zoom);

   if (ret != HPDF_OK)
      return HPDF_CheckError(dst->error);

   return HPDF_OK;
}


HPDF_EXPORT(HpdfStatus)
HPDF_Destination_SetFit(HpdfDestination * const dst)
{
   HpdfStatus ret = HPDF_OK;
   HPDF_Page target;

   HPDF_PTRACE((" HPDF_Destination_SetFit\n"));

   if (!HPDF_Destination_Validate(dst))
      return HPDF_INVALID_DESTINATION;

   target = (HPDF_Page) HpdfArrayGetItem(dst, 0, HPDF_OCLASS_DICT);

   if (dst->list->count > 1)
   {
      HpdfArrayClear(dst);
      ret += HpdfArrayAdd(dst, target);
   }

   ret += HpdfArrayAddName(dst, HPDF_DESTINATION_TYPE_NAMES[(HpdfInt) HPDF_FIT]);

   if (ret != HPDF_OK)
   {
      return HPDF_CheckError(dst->error);
   }

   return HPDF_OK;
}


HPDF_EXPORT(HpdfStatus)
HPDF_Destination_SetFitH(HpdfDestination * const dst,
   HpdfReal         top)
{
   HpdfStatus ret = HPDF_OK;
   HPDF_Page target;

   HPDF_PTRACE((" HPDF_Destination_SetFitH\n"));

   if (!HPDF_Destination_Validate(dst))
      return HPDF_INVALID_DESTINATION;

   target = (HPDF_Page) HpdfArrayGetItem(dst, 0, HPDF_OCLASS_DICT);

   if (dst->list->count > 1)
   {
      HpdfArrayClear(dst);
      ret += HpdfArrayAdd(dst, target);
   }

   ret += HpdfArrayAddName(dst, HPDF_DESTINATION_TYPE_NAMES[(HpdfInt) HPDF_FIT_H]);
   ret += HpdfArrayAddReal(dst, top);

   if (ret != HPDF_OK)
      return HPDF_CheckError(dst->error);

   return HPDF_OK;
}

HPDF_EXPORT(HpdfStatus)
HPDF_Destination_SetFitV(HpdfDestination * const dst,
   HpdfReal         left)
{
   HpdfStatus ret = HPDF_OK;
   HPDF_Page target;

   HPDF_PTRACE((" HPDF_Destination_SetFitV\n"));

   if (!HPDF_Destination_Validate(dst))
      return HPDF_INVALID_DESTINATION;

   target = (HPDF_Page) HpdfArrayGetItem(dst, 0, HPDF_OCLASS_DICT);

   if (dst->list->count > 1)
   {
      HpdfArrayClear(dst);
      ret += HpdfArrayAdd(dst, target);
   }

   ret += HpdfArrayAddName(dst, HPDF_DESTINATION_TYPE_NAMES[(HpdfInt) HPDF_FIT_V]);
   ret += HpdfArrayAddReal(dst, left);

   if (ret != HPDF_OK)
      return HPDF_CheckError(dst->error);

   return HPDF_OK;
}


HPDF_EXPORT(HpdfStatus)
HPDF_Destination_SetFitR(HpdfDestination * const dst,
   HpdfReal         left,
   HpdfReal         bottom,
   HpdfReal         right,
   HpdfReal         top)
{
   HpdfStatus ret = HPDF_OK;
   HPDF_Page target;

   HPDF_PTRACE((" HPDF_Destination_SetFitR\n"));

   if (!HPDF_Destination_Validate(dst))
      return HPDF_INVALID_DESTINATION;

   target = (HPDF_Page) HpdfArrayGetItem(dst, 0, HPDF_OCLASS_DICT);

   if (dst->list->count > 1)
   {
      HpdfArrayClear(dst);
      ret += HpdfArrayAdd(dst, target);
   }

   ret += HpdfArrayAddName(dst, HPDF_DESTINATION_TYPE_NAMES[(HpdfInt) HPDF_FIT_R]);
   ret += HpdfArrayAddReal(dst, left);
   ret += HpdfArrayAddReal(dst, bottom);
   ret += HpdfArrayAddReal(dst, right);
   ret += HpdfArrayAddReal(dst, top);

   if (ret != HPDF_OK)
      return HPDF_CheckError(dst->error);

   return HPDF_OK;
}


HPDF_EXPORT(HpdfStatus)
HPDF_Destination_SetFitB(HpdfDestination * const dst)
{
   HpdfStatus ret = HPDF_OK;
   HPDF_Page target;

   HPDF_PTRACE((" HPDF_Destination_SetFitB\n"));

   if (!HPDF_Destination_Validate(dst))
      return HPDF_INVALID_DESTINATION;

   target = (HPDF_Page) HpdfArrayGetItem(dst, 0, HPDF_OCLASS_DICT);

   if (dst->list->count > 1)
   {
      HpdfArrayClear(dst);
      ret += HpdfArrayAdd(dst, target);
   }

   ret += HpdfArrayAddName(dst, HPDF_DESTINATION_TYPE_NAMES[(HpdfInt) HPDF_FIT_B]);
   
   if (ret != HPDF_OK)
   {
      return HPDF_CheckError(dst->error);
   }

   return HPDF_OK;
}


HPDF_EXPORT(HpdfStatus)
HPDF_Destination_SetFitBH(HpdfDestination * const dst,
   HpdfReal         top)
{
   HpdfStatus ret = HPDF_OK;
   HPDF_Page target;

   HPDF_PTRACE((" HPDF_Destination_SetFitBH\n"));

   if (!HPDF_Destination_Validate(dst))
      return HPDF_INVALID_DESTINATION;

   target = (HPDF_Page) HpdfArrayGetItem(dst, 0, HPDF_OCLASS_DICT);

   if (dst->list->count > 1)
   {
      HpdfArrayClear(dst);
      ret += HpdfArrayAdd(dst, target);
   }

   ret += HpdfArrayAddName(dst, HPDF_DESTINATION_TYPE_NAMES[(HpdfInt) HPDF_FIT_BH]);
   ret += HpdfArrayAddReal(  dst, top);

   if (ret != HPDF_OK)
      return HPDF_CheckError(dst->error);

   return HPDF_OK;
}

HPDF_EXPORT(HpdfStatus)
HPDF_Destination_SetFitBV(HpdfDestination * const dst,
   HpdfReal         left)
{
   HpdfStatus ret = HPDF_OK;
   HPDF_Page target;

   HPDF_PTRACE((" HPDF_Destination_SetFitBV\n"));

   if (!HPDF_Destination_Validate(dst))
      return HPDF_INVALID_DESTINATION;

   target = (HPDF_Page) HpdfArrayGetItem(dst, 0, HPDF_OCLASS_DICT);

   if (dst->list->count > 1) 
   {
      HpdfArrayClear(dst);
      ret += HpdfArrayAdd(dst, target);
   }

   ret += HpdfArrayAddName(dst, HPDF_DESTINATION_TYPE_NAMES[(HpdfInt) HPDF_FIT_BV]);
   ret += HpdfArrayAddReal(dst, left);

   if (ret != HPDF_OK)
      return HPDF_CheckError(dst->error);

   return HPDF_OK;

}

