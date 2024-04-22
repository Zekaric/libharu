/*
 * << Haru Free PDF Library >> -- hpdf_utils.c
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

#include <math.h>
#include <stdlib.h>
#include "hpdf_utils.h"
#include "hpdf_consts.h"

 /*---------------------------------------------------------------------------*/

HpdfInt
   HPDF_AToI(
      char const * const str)
{
   HpdfBool     flg     = HPDF_FALSE;
   HpdfInt      v       = 0;
   char const  *strTemp = str;

   if (!strTemp) 
   {
      return 0;
   }

   /* increment pointer until the character of 'str' is not
   ** white-space-character. */
   while (*strTemp) 
   {
      if (HPDF_IS_WHITE_SPACE(*strTemp))
      {
         strTemp++;
      }
      else 
      {
         if (*strTemp == '-') 
         {
            flg = HPDF_TRUE;
            strTemp++;
         }
         break;
      }
   }

   while ('0' <= *strTemp && *strTemp <= '9') 
   {
      v *= 10;
      v += *strTemp - '0';
      strTemp++;
   }

   if (flg)
   {
      v *= -1;
   }

   return v;
}

HpdfDouble
   HPDF_AToF(
      char const * const str)
{
   HpdfBool     flg = HPDF_FALSE;
   HpdfInt      i = 0;
   HpdfDouble   v;
   HpdfInt      tmp = 1;
   char const  *strTemp = str;

   /* increment pointer until the character of 'str' is not
    * white-space-character.
    */
   while (*strTemp)
   {
      if (HPDF_IS_WHITE_SPACE(*strTemp))
      {
         strTemp++;
      }
      else 
      {
         if (*strTemp == '-') 
         {
            flg = HPDF_TRUE;
            strTemp++;
         }
         break;
      }
   }

   while ('0' <= *strTemp && *strTemp <= '9') 
   {
      if (i > 3276)
      {
         break;
      }

      i *= 10;
      i += *strTemp - '0';
      strTemp++;
   }

   if (*strTemp == '.') 
   {
      strTemp++;
      while ('0' <= *str && *str <= '9') 
      {
         if (i > 214748364)
         {
            break;
         }

         i *= 10;
         i += *strTemp - '0';
         strTemp++;
         tmp *= 10;
      }
   }

   v = (HpdfDouble) i / tmp;

   if (flg)
   {
      v *= -1;
   }

   return v;
}

char *
   HPDF_IToA(
      char        * const str,
      HpdfInt32           val,
      char        * const eptr)
{
   char  *t;
   char   buf[HpdfInt_LEN + 1];
   char  *strTemp = str;

   if      (val < 0) 
   {
      if (val < HPDF_LIMIT_MIN_INT)
      {
         val = HPDF_LIMIT_MIN_INT;
      }
      *strTemp++ = '-';
      val = -val;
   }
   else if (val > HPDF_LIMIT_MAX_INT) 
   {
      val = HPDF_LIMIT_MAX_INT;
   }
   else if (val == 0) 
   {
      *strTemp++ = '0';
   }

   t    = buf + HpdfInt_LEN;
   *t-- = 0;

   while (val > 0) 
   {
      *t = (char) ((char) (val % 10) + '0');
      val /= 10;
      t--;
   }

   t++;
   while (strTemp < eptr && *t != 0)
   {
      *strTemp++ = *t++;
   }
   *strTemp = 0;

   return strTemp;
}

char *
   HPDF_IToA2(
      char        * const str,
      HpdfUInt32          val,
      HpdfUInt            len)
{
   char *t;
   char *u;
   char *strTemp = str;

   if (val > HPDF_LIMIT_MAX_INT)
   {
      val = HPDF_LIMIT_MAX_INT;
   }

   u  = strTemp + len - 1;
   *u = 0;
   t  = u - 1;
   while (val > 0 && t >= strTemp)
   {
      *t = (char) ((char) (val % 10) + '0');
      val /= 10;
      t--;
   }

   while (strTemp <= t)
   {
      *t-- = '0';
   }

   return strTemp + len - 1;
}

char *
   HPDF_FToA(
      char     * const str,
      HpdfReal         val,
      char     * const eptr)
{
   HpdfReal     int_val;
   HpdfReal     fpart_val;
   HpdfReal     dig;
   char         buf[HpdfReal_LEN + 1];
   char        *strPtr  = str,
               *strTemp = str;
   char        *t;
   HpdfInt32    logVal;
   HpdfUInt32   prec;

   if (val > HPDF_LIMIT_MAX_REAL)
   {
      val = HPDF_LIMIT_MAX_REAL;
   }
   else
   {
      if (val < HPDF_LIMIT_MIN_REAL)
      {
         val = HPDF_LIMIT_MIN_REAL;
      }
   }

   t    = buf;
   *t++ = 0;

   if (val < 0) 
   {
      *strTemp++ = '-';
      val        = -val;
   }

   /* compute the decimal precision to write at least 5 significant figures */
   logVal = (HpdfInt32) (val > 1e-20 ? log10(val) : 0.);
   if (logVal >= 0) 
   {
      prec = 5;
   }
   else 
   {
      prec = -logVal + 5;
   }

   /* separate an integer part and a fractional part. */
   fpart_val = modff(val, &int_val);

   /* process integer part */
   do 
   {
      dig  = modff(int_val/10.0f, &int_val);
      *t++ = (char) (dig*10.0 + 0.5) + '0';
   } while (int_val > 0);

   /* copy to destination buffer */
   t--;
   while (strTemp <= eptr && *t != 0)
   {
      *strTemp++ = *t--;
   }

   /* process fractional part */
   *strTemp++ = '.';
   if (fpart_val != 0.0) 
   {
      for (HpdfUInt32 i = 0; i < prec; i++) 
      {
         fpart_val  = modff(fpart_val * 10.0f, &int_val);
         *strTemp++ = (char) (int_val + 0.5) + '0';
      }
   }

   /* delete an excessive decimal portion. */
   strTemp--;
   while (strTemp > strPtr) 
   {
      if (*strTemp == '0')
      {
         *strTemp = 0;
      }
      else 
      {
         if (*strTemp == '.')
         {
            *strTemp = 0;
         }
         break;
      }
      strTemp--;
   }

   return (*strTemp == 0) ? strTemp : ++strTemp;
}

HpdfByte *
   HPDF_MemCpy(
      HpdfByte       * const out,
      HpdfByte const * const in,
      HpdfUInt               count)
{
   HpdfByte       *outTemp = out;
   HpdfByte const *inTemp  = in;

   while (count > 0) 
   {
      *outTemp++ = *inTemp++;
      count--;
   }

   return out;
}

HpdfByte *
   HPDF_StrCpy(
      char       * const out,
      char const * const in,
      char       * const eptr)
{
   char       *outTemp = out;
   char const *inTemp  = in;

   if (in != NULL) 
   {
      while (eptr > outTemp && *inTemp != 0)
      {
         *outTemp++ = *inTemp++;
      }
   }

   *outTemp = 0;

   return (HpdfByte *) outTemp;
}

HpdfInt
   HPDF_MemCmp(
      HpdfByte const * const s1,
      HpdfByte const * const s2,
      HpdfUInt              count)
{
   HpdfByte const *s1Temp = s1;
   HpdfByte const *s2Temp = s2;

   if (count == 0)
   {
      return 0;
   }

   while (*s1Temp == *s2Temp) 
   {
      count--;
      if (count == 0)
      {
         return 0;
      }
      s1Temp++;
      s2Temp++;
   }

   return *s1Temp - *s2Temp;
}

HpdfBool
   HpdfMemIsEqual(
      HpdfByte const * const buf1,
      HpdfByte const * const buf2,
      HpdfUInt               count)
{
   return (HPDF_MemCmp(buf1, buf2, count) == 0);
}

HpdfInt
   HPDF_StrCmp(
      char const * const s1,
      char const * const s2)
{
   char const *s1Temp = s1;
   char const *s2Temp = s2;

   if (!s1Temp ||
       !s2Temp)
   {
      if (!s1Temp && 
          s2Temp)
      {
         return -1;
      }

      return 1;
   }

   while (*s1Temp == *s2Temp) 
   {
      s1Temp++;
      s2Temp++;
      if (*s1Temp == 0 || 
          *s2Temp == 0)
      {
         break;
      }
   }

   return (HpdfByte) *s1Temp - (HpdfByte) *s2Temp;
}

HpdfBool
   HpdfStrIsEqual(
      char const * const s1,
      char const * const s2)
{
   return (HPDF_StrCmp(s1, s2) == 0);
}

void *
   HPDF_MemSet(
      void     * const str,
      HpdfByte         value,
      HpdfUInt         count)
{
   HpdfByte *strTemp = (HpdfByte *) str;

   while (count > 0) 
   {
      *strTemp = value;
      strTemp++;
      count--;
   }

   return strTemp;
}

HpdfUInt
   HPDF_StrLen(
      char const * const str,
      HpdfInt            maxlen)
{
   char const *strTemp = str;
   HpdfInt     len     = 0;

   if (!strTemp)
   { 
      return 0;
   }

   while (*strTemp != 0 && 
          (maxlen < 0 || 
           len    < maxlen)) 
   {
      strTemp++;
      len++;
   }

   return (HpdfUInt) len;
}

char const *
   HPDF_StrStr(
      char const * const s1,
      char const * const s2,
      HpdfUInt           maxlen)
{
   char const *s1Temp = s1;
   HpdfUInt    len    = HPDF_StrLen(s2, -1);

   if (!s1Temp)
   {
      return NULL;
   }

   if (len == 0)
   {
      return s1Temp;
   }

   if (maxlen == 0)
   {
      maxlen = HPDF_StrLen(s1Temp, -1);
   }

   if (maxlen < len)
   {
      return NULL;
   }

   maxlen -= len;
   maxlen++;

   while (maxlen > 0) 
   {
      if (HpdfMemIsEqual((HpdfByte *) s1Temp, (HpdfByte *) s2, len))
      {
         return s1Temp;
      }

      s1Temp++;
      maxlen--;
   }

   return NULL;
}

HPDF_Box
   HPDF_ToBox(
      HpdfInt16 left,
      HpdfInt16 bottom,
      HpdfInt16 right,
      HpdfInt16 top)
{
   HPDF_Box box;

   box.left   = left;
   box.bottom = bottom;
   box.right  = right;
   box.top    = top;

   return box;
}

HpdfPoint
   HPDF_ToPoint(
      HpdfInt16 x,
      HpdfInt16 y)
{
   HpdfPoint point;

   point.x = x;
   point.y = y;

   return point;
}

HpdfRect
   HPDF_ToRect(
      HpdfReal left,
      HpdfReal bottom,
      HpdfReal right,
      HpdfReal top)
{
   HpdfRect rect;

   rect.left   = left;
   rect.bottom = bottom;
   rect.right  = right;
   rect.top    = top;

   return rect;
}

void
   HPDF_UInt16Swap(
      HpdfUInt16 * const value)
{
   HpdfByte u[2];

   HPDF_MemCpy(u, (HpdfByte *) value, 2);

   *value = (HpdfUInt16) ((HpdfUInt16) u[0] << 8 | (HpdfUInt16) u[1]);
}
