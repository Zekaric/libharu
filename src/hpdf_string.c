/*
 * << Haru Free PDF Library >> -- hpdf_string.c
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

#include <string.h>

#if defined(WIN32)
#include <windows.h>
#endif

#include "hpdf_conf.h"
#include "hpdf_utils.h"
#include "hpdf_objects.h"

static HpdfByte const UNICODE_HEADER[] = 
{
   0xFE, 0xFF
};


HPDF_String
   HPDF_String_New(
      HpdfMemMgr * const mmgr,
      char const *value,
      HPDF_Encoder     encoder)
{
   HPDF_String obj;

   HPDF_PTRACE((" HPDF_String_New\n"));

   obj = HpdfMemCreateType(mmgr, HPDF_String_Rec);
   if (obj) 
   {
      HpdfMemClearType(&obj->header, HPDF_Obj_Header);
      obj->header.obj_class = HPDF_OCLASS_STRING;

      obj->mmgr = mmgr;
      obj->error = mmgr->error;
      obj->encoder = encoder;
      obj->value = NULL;
      obj->len = 0;

      if (HPDF_String_SetValue(obj, value) != HPDF_OK) {
         HpdfMemDestroy(obj->mmgr, obj);
         return NULL;
      }
   }

   return obj;
}


#if defined(WIN32)
HPDF_String
   HPDF_String_NewW(
      HpdfMemMgr * const mmgr,
      const wchar_t   *value,
      HPDF_Encoder     encoder)
{
   HPDF_String obj;
   size_t      valueLen;
   char       *utf8Value;
   int         utf8ValueLen;

   HPDF_PTRACE((" HPDF_String_NewW\n"));

   /* Convert the wchar_t to UTF8 char * */
   valueLen = wcslen(value);

   utf8ValueLen = WideCharToMultiByte(CP_UTF8, 0, value, (int) valueLen, 0, 0, NULL, NULL);
   utf8Value    = HpdfMemCreateTypeArray(mmgr, char, utf8ValueLen + 1);
   if (utf8Value)
   {
      WideCharToMultiByte(CP_UTF8, 0, value, (int) valueLen, utf8Value, utf8ValueLen, NULL, NULL);
      utf8Value[utf8ValueLen] = '\0';
   }

   /* Use the existing function to get the string. */
   obj = HPDF_String_New(mmgr, utf8Value, encoder);

   /* Clean up */
   HpdfMemDestroy(mmgr, utf8Value);

   return obj;
}
#endif

HpdfStatus
HPDF_String_SetValue(HPDF_String      obj,
   char const *value)
{
   HpdfUInt len;
   HpdfStatus ret = HPDF_OK;

   HPDF_PTRACE((" HPDF_String_SetValue\n"));

   if (obj->value) {
      HpdfMemDestroy(obj->mmgr, obj->value);
      obj->len = 0;
   }

   len = HPDF_StrLen(value, HPDF_LIMIT_MAX_STRING_LEN + 1);

   if (len > HPDF_LIMIT_MAX_STRING_LEN)
      return HPDF_SetError(obj->error, HPDF_STRING_OUT_OF_RANGE, 0);

   obj->value = HpdfMemCreateTypeArray(obj->mmgr, HpdfByte, len + 1);
   if (!obj->value)
   {
      return HPDF_Error_GetCode(obj->error);
   }

   HPDF_StrCpy((char *) obj->value, value, (char *) obj->value + len);
   obj->len = len;

   return ret;
}

void
HPDF_String_Free(HPDF_String  obj)
{
   if (!obj)
      return;

   HPDF_PTRACE((" HPDF_String_Free\n"));

   HpdfMemDestroy(obj->mmgr, obj->value);
   HpdfMemDestroy(obj->mmgr, obj);
}


HpdfStatus
HPDF_String_Write(HPDF_String   obj,
   HPDF_Stream   stream,
   HPDF_Encrypt  e)
{
   HpdfStatus ret;

   /*
    *  When encoder is not NULL, text is changed to unicode using encoder,
    *  and it outputs by HPDF_write_binary method.
    */

   HPDF_PTRACE((" HPDF_String_Write\n"));

   if (e)
      HPDF_Encrypt_Reset(e);

   if (obj->encoder == NULL) {
      if (e) {
         if ((ret = HPDF_Stream_WriteChar(stream, '<')) != HPDF_OK)
            return ret;

         if ((ret = HPDF_Stream_WriteBinary(stream, obj->value,
            HPDF_StrLen((char *) obj->value, -1), e)) != HPDF_OK)
            return ret;

         return HPDF_Stream_WriteChar(stream, '>');
      }
      else {
         return HPDF_Stream_WriteEscapeText(stream, (char *) obj->value);
      }
   }
   else {
      HpdfByte *src = obj->value;
      HpdfByte buf[HPDF_TEXT_DEFAULT_LEN * 2];
      HpdfUInt tmp_len = 0;
      HpdfByte *pbuf = buf;
      HpdfInt32 len = obj->len;
      HpdfParseText parse_state;
      HpdfUInt i;

      if ((ret = HPDF_Stream_WriteChar(stream, '<')) != HPDF_OK)
         return ret;

      if ((ret = HPDF_Stream_WriteBinary(stream, UNICODE_HEADER, 2, e))
         != HPDF_OK)
         return ret;

      HPDF_Encoder_SetParseText(obj->encoder, &parse_state, src, len);

      for (i = 0; (HpdfInt32) i < len; i++) {
         HpdfByte b = src[i];
         HpdfUnicode tmp_unicode;
         HPDF_ByteType btype = HPDF_Encoder_ByteType(obj->encoder,
            &parse_state);

         if (tmp_len >= HPDF_TEXT_DEFAULT_LEN - 1) {
            if ((ret = HPDF_Stream_WriteBinary(stream, buf,
               tmp_len * 2, e)) != HPDF_OK)
               return ret;

            tmp_len = 0;
            pbuf = buf;
         }

         if (btype != HPDF_BYTE_TYPE_TRAIL) {
            if (btype == HPDF_BYTE_TYPE_LEAD) {
               HpdfByte b2 = src[i + 1];
               HpdfUInt16 char_code = (HpdfUInt16) ((HpdfUInt) b * 256 + b2);

               tmp_unicode = HPDF_Encoder_ToUnicode(obj->encoder,
                  char_code);
            }
            else {
               tmp_unicode = HPDF_Encoder_ToUnicode(obj->encoder, b);
            }

            HPDF_UInt16Swap(&tmp_unicode);
            HPDF_MemCpy(pbuf, (HpdfByte *) &tmp_unicode, 2);
            pbuf += 2;
            tmp_len++;
         }
      }

      if (tmp_len > 0) {
         if ((ret = HPDF_Stream_WriteBinary(stream, buf, tmp_len * 2, e))
            != HPDF_OK)
            return ret;
      }

      if ((ret = HPDF_Stream_WriteChar(stream, '>')) != HPDF_OK)
         return ret;
   }

   return HPDF_OK;
}


HpdfInt32
HPDF_String_Cmp(HPDF_String s1,
   HPDF_String s2)
{
   HpdfInt32 res;
   HpdfUInt  minLen;

   minLen = s1->len;
   if (s1->len > s2->len) {
      minLen = s2->len;
   }

   res = memcmp(s1->value, s2->value, minLen);
   if (res == 0) {
      if (s1->len < s2->len) res = -1;
      if (s1->len > s2->len) res = +1;
   }

   return res;
}
