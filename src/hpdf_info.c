/*
 * << Haru Free PDF Library >> -- hpdf_info.c
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
#include "hpdf_info.h"

static char const * const HPDF_INFO_ATTR_NAMES[] = {
    "CreationDate",
    "ModDate",
    "Author",
    "Creator",
    "Producer",
    "Title",
    "Subject",
    "Keywords",
    "Trapped",
    "GTS_PDFXVersion",
    NULL
};


static char const*
InfoTypeToName(HPDF_InfoType  type);


/*---------------------------------------------------------------------------*/

static char const*
InfoTypeToName(HPDF_InfoType  type)
{
   HpdfUInt idx = (HpdfUInt) type;

   return HPDF_INFO_ATTR_NAMES[idx];
}

HpdfStatus
   HPDF_Info_SetInfoAttr(
      HPDF_Dict             info,
      HPDF_InfoType         type,
      char const           *value,
      HpdfEncoder * const   encoder)
{
   char const* name = InfoTypeToName(type);

   HPDF_PTRACE((" HPDF_Info_SetInfoAttr\n"));

   if (type <= HPDF_INFO_MOD_DATE)
   {
      return HPDF_SetError(info->error, HPDF_INVALID_PARAMETER, 0);
   }

   if (type == HPDF_INFO_TRAPPED)
   {
      return HPDF_Dict_AddName(info, name, value);
   }

   return HPDF_Dict_Add(info, name, HpdfObjStringCreate(info->mmgr, value, encoder));
}

char const *
   HPDF_Info_GetInfoAttr(
      HPDF_Dict      info,
      HPDF_InfoType  type)
{
   char const        *name = InfoTypeToName(type);
   HpdfObjString   *s;

   HPDF_PTRACE((" HPDF_Info_GetInfoAttr\n"));

   if (!info)
   {
      return NULL;
   }

   s = HPDF_Dict_GetItem(info, name, HPDF_OCLASS_STRING);
   if (!s)
   {
      return NULL;
   }

   return (char const *) (s->value);
}

HpdfStatus
   HPDF_Info_SetInfoDateAttr(
      HPDF_Dict      info,
      HPDF_InfoType  type,
      HPDF_Date      value)
{
   char         tmp[HPDF_DATE_TIME_STR_LEN + 1];
   char        *ptmp;
   char const  *name = InfoTypeToName(type);

   HPDF_PTRACE((" HPDF_Info_SetInfoDateAttr\n"));

   if (type > HPDF_INFO_MOD_DATE)
   {
      return HPDF_SetError(info->error, HPDF_INVALID_PARAMETER, 0);
   }

   HpdfMemClear(tmp, HPDF_DATE_TIME_STR_LEN + 1);
   if (value.month < 1 || 12 < value.month ||
       value.day < 1                       ||
       23 < value.hour                     ||
       59 < value.minutes                  ||
       59 < value.seconds                  ||
       (value.ind != '+' && 
        value.ind != '-' && 
        value.ind != 'Z' &&
        value.ind != ' ')                  ||
       23 < value.off_hour                 ||
       59 < value.off_minutes) 
   {
      return HPDF_SetError(info->error, HPDF_INVALID_DATE_TIME, 0);
   }

   switch (value.month) 
   {
   case 1:
   case 3:
   case 5:
   case 7:
   case 8:
   case 10:
   case 12:
      if (value.day > 31)
      {
         return HPDF_SetError(info->error, HPDF_INVALID_DATE_TIME, 0);
      }
      break;

   case 4:
   case 6:
   case 9:
   case 11:
      if (value.day > 30)
      {
         return HPDF_SetError(info->error, HPDF_INVALID_DATE_TIME, 0);
      }
      break;

   case 2:
      if (value.day > 29 || 
          (value.day == 29 &&
            (value.year % 4 != 0 ||
               (value.year % 100 == 0 && 
                value.year % 400 != 0))))
      {
         return HPDF_SetError(info->error, HPDF_INVALID_DATE_TIME, 0);
      }
      break;

   default:
      return HPDF_SetError(info->error, HPDF_INVALID_DATE_TIME, 0);
   }

   ptmp = (char *) HPDF_MemCpy((HpdfByte *) tmp, (HpdfByte *) "D:", 2);
   ptmp =          HPDF_IToA2(ptmp,              value.year,        5);
   ptmp =          HPDF_IToA2(ptmp,              value.month,       3);
   ptmp =          HPDF_IToA2(ptmp,              value.day,         3);
   ptmp =          HPDF_IToA2(ptmp,              value.hour,        3);
   ptmp =          HPDF_IToA2(ptmp,              value.minutes,     3);
   ptmp =          HPDF_IToA2(ptmp,              value.seconds,     3);
   if (value.ind != ' ') 
   {
      *ptmp++ = value.ind;
      ptmp    = HPDF_IToA2(ptmp, value.off_hour, 3);
      *ptmp++ = '\'';
      ptmp    = HPDF_IToA2(ptmp, value.off_minutes, 3);
      *ptmp++ = '\'';
   }
   *ptmp = 0;

   return HPDF_Dict_Add(info, name, HpdfObjStringCreate(info->mmgr, tmp, NULL));
}
