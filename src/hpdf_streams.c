/*
 * << Haru Free PDF Library >> -- hpdf_streams.c
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
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef UNDER_CE
#include <errno.h>
#endif
#ifndef HPDF_UNUSED
#define HPDF_UNUSED(a) ((void)(a))
#endif

#include "hpdf_conf.h"
#include "hpdf_consts.h"
#include "hpdf_utils.h"
#include "hpdf_streams.h"

#ifdef LIBHPDF_HAVE_ZLIB
#include <zlib.h>
#include <zconf.h>
#endif /* LIBHPDF_HAVE_ZLIB */

HpdfStatus  HPDF_MemStream_WriteFunc(              HPDF_Stream stream, HpdfByte  const * const ptr, HpdfUInt        siz);
HpdfStatus  HPDF_MemStream_SeekFunc(               HPDF_Stream stream, HpdfInt         pos, HPDF_WhenceMode  mode);
HpdfStatus  HPDF_MemStream_ReadFunc(               HPDF_Stream stream, HpdfByte    * const buf, HpdfUInt    *size); 
HpdfInt32   HPDF_MemStream_TellFunc(               HPDF_Stream stream); 
HpdfUInt32  HPDF_MemStream_SizeFunc(               HPDF_Stream stream);
void        HPDF_MemStream_FreeFunc(               HPDF_Stream stream); 
HpdfStatus  HPDF_MemStream_InWrite(                HPDF_Stream stream, HpdfByte const **ptr, HpdfUInt        *count);
HpdfStatus  HPDF_Stream_WriteToStreamWithDeflate(  HPDF_Stream src, HPDF_Stream  dst, HPDF_Encrypt  e);
HpdfStatus  HPDF_FileReader_ReadFunc(              HPDF_Stream stream, HpdfByte    * const ptr, HpdfUInt    *siz);
HpdfStatus  HPDF_FileReader_SeekFunc(              HPDF_Stream stream, HpdfInt         pos, HPDF_WhenceMode  mode);
HpdfInt32   HPDF_FileStream_TellFunc(              HPDF_Stream stream); 
HpdfUInt32  HPDF_FileStream_SizeFunc(              HPDF_Stream stream);  
HpdfStatus  HPDF_FileWriter_WriteFunc(             HPDF_Stream stream, HpdfByte  const * const ptr, HpdfUInt        siz);
void        HPDF_FileStream_FreeFunc(              HPDF_Stream stream);

/*
 *  HPDF_Stream_Read
 *
 *  stream : Pointer to a HPDF_Stream object.
 *  ptr : Pointer to a buffer to copy read data.
 *  size : Pointer to a variable which indecates buffer size.
 *
 *  HPDF_Stream_read returns HPDF_OK when success. On failure, it returns
 *  error-code returned by reading function of this stream.
 *
 */

HpdfStatus
HPDF_Stream_Read(HPDF_Stream  stream,
   HpdfByte    *ptr,
   HpdfUInt    *size)
{
   if (!(stream->read_fn))
      return HPDF_SetError(stream->error, HPDF_INVALID_OPERATION, 0);

   /*
   if (HPDF_Error_GetCode(stream->error) != HPDF_NOERROR)
       return HPDF_THIS_FUNC_WAS_SKIPPED;
   */

   return stream->read_fn(stream, ptr, size);
}


/*
 *  HPDF_Stream_ReadLn
 *
 *  stream : Pointer to a HPDF_Stream object.
 *  s : Pointer to a buffer to copy read data.
 *  size : buffer-size of s.
 *
 *  Read from stream until the buffer is exhausted or line-feed character is
 *  read.
 *
 */
HpdfStatus
HPDF_Stream_ReadLn(HPDF_Stream  stream,
   char    *s,
   HpdfUInt    *size)
{
   char buf[HPDF_STREAM_BUF_SIZ];
   HpdfUInt r_size = *size;
   HpdfUInt read_size = HPDF_STREAM_BUF_SIZ;

   HPDF_PTRACE((" HPDF_Stream_ReadLn\n"));

   if (!stream)
      return HPDF_INVALID_PARAMETER;

   if (!s || *size == 0)
      return HPDF_SetError(stream->error, HPDF_INVALID_PARAMETER, 0);

   if (!(stream->seek_fn) || !(stream->read_fn))
      return HPDF_SetError(stream->error, HPDF_INVALID_OPERATION, 0);

   if (r_size < HPDF_STREAM_BUF_SIZ)
      read_size = r_size;

   *size = 0;

   while (r_size > 1) {
      char *pbuf = buf;
      HpdfStatus ret = HPDF_Stream_Read(stream, (HpdfByte *) buf, &read_size);

      if (ret != HPDF_OK && read_size == 0)
         return ret;

      r_size -= read_size;

      while (read_size > 0) {
         if (*pbuf == 0x0A || *pbuf == 0x0D) {
            *s = 0;
            read_size--;

            /* handling CR-LF marker */
            if (*pbuf == 0x0D || read_size > 1) {
               pbuf++;

               if (*pbuf == 0x0A)
                  read_size--;
            }

            if (read_size > 0)
               return HPDF_Stream_Seek(stream, 0 - read_size,
                  HPDF_SEEK_CUR);
            else
               return HPDF_OK;
         }

         *s++ = *pbuf++;
         read_size--;
         (*size)++;
      }

      if (r_size < HPDF_STREAM_BUF_SIZ)
         read_size = r_size;
      else
         read_size = HPDF_STREAM_BUF_SIZ;

      if (ret == HPDF_STREAM_EOF)
         return HPDF_STREAM_EOF;
   }

   *s = 0;

   return HPDF_STREAM_READLN_CONTINUE;
}


/*
 * HPDF_Stream_Write
 *
 *  stream : Pointer to a HPDF_Stream object.
 *  ptr : Pointer to a buffer to write.
 *  siz : The size of buffer to write.
 *
 *  HPDF_Stream_Write returns HPDF_OK when success. On failure, it returns
 *  error-code returned by writing function of this stream.
 *
 */
HpdfStatus
HPDF_Stream_Write(HPDF_Stream      stream,
   HpdfByte  const * const ptr,
   HpdfUInt        size)
{
   HpdfStatus ret;

   HPDF_PTRACE((" HPDF_Stream_Write\n"));

   if (!(stream->write_fn))
      return HPDF_SetError(stream->error, HPDF_INVALID_OPERATION, 0);

   /*
   if (HPDF_Error_GetCode(stream->error) != HPDF_NOERROR)
       return HPDF_THIS_FUNC_WAS_SKIPPED;
   */

   ret = stream->write_fn(stream, ptr, size);

   if (ret != HPDF_OK)
      return ret;

   stream->size += size;

   return HPDF_OK;
}


HpdfStatus
HPDF_Stream_WriteChar(HPDF_Stream  stream,
   char    value)
{
   return HPDF_Stream_Write(stream, (HpdfByte *) &value, sizeof(char));
}


HpdfStatus
HPDF_Stream_WriteStr(HPDF_Stream      stream,
   char const *value)
{
   HpdfUInt len = HPDF_StrLen(value, -1);

   return HPDF_Stream_Write(stream, (HpdfByte *) value, len);
}

HpdfStatus
HPDF_Stream_WriteUChar(HPDF_Stream  stream,
   HpdfByte    value)
{
   return HPDF_Stream_Write(stream, &value, sizeof(HpdfByte));
}

HpdfStatus
HPDF_Stream_WriteInt(HPDF_Stream  stream,
   HpdfInt     value)
{
   char buf[HpdfInt_LEN + 1];

   char* p = HPDF_IToA(buf, value, buf + HpdfInt_LEN);

   return HPDF_Stream_Write(stream, (HpdfByte *) buf, (HpdfUInt) (p - buf));
}

HpdfStatus
HPDF_Stream_WriteUInt(HPDF_Stream  stream,
   HpdfUInt    value)
{
   return HPDF_Stream_WriteInt(stream, (HpdfInt) value);
}

HpdfStatus
HPDF_Stream_WriteReal(HPDF_Stream  stream,
   HpdfReal    value)
{
   char buf[HpdfReal_LEN + 1];

   char* p = HPDF_FToA(buf, value, buf + HpdfReal_LEN);

   return HPDF_Stream_Write(stream, (HpdfByte *) buf, (HpdfUInt) (p - buf));
}

void
HPDF_Stream_Free(HPDF_Stream  stream)
{
   if (!stream)
      return;

   if (stream->free_fn)
      stream->free_fn(stream);

   stream->sig_bytes = 0;

   HpdfMemDestroy(stream->mmgr, stream);
}

HpdfStatus
HPDF_Stream_Seek(HPDF_Stream      stream,
   HpdfInt         pos,
   HPDF_WhenceMode  mode)
{
   HPDF_PTRACE((" HPDF_Stream_Seek\n"));

   if (!(stream->seek_fn))
      return HPDF_SetError(stream->error, HPDF_INVALID_OPERATION, 0);

   if (HPDF_Error_GetCode(stream->error) != 0)
      return HPDF_THIS_FUNC_WAS_SKIPPED;

   return stream->seek_fn(stream, pos, mode);
}


HpdfInt32
HPDF_Stream_Tell(HPDF_Stream  stream)
{
   HPDF_PTRACE((" HPDF_Stream_Tell\n"));

   if (!(stream->tell_fn))
      return HPDF_SetError(stream->error, HPDF_INVALID_OPERATION, 0);

   if (HPDF_Error_GetCode(stream->error) != 0)
      return HPDF_THIS_FUNC_WAS_SKIPPED;

   return stream->tell_fn(stream);
}


HpdfUInt32
HPDF_Stream_Size(HPDF_Stream  stream)
{
   HPDF_PTRACE((" HPDF_Stream_Tell\n"));

   if (stream->write_fn)
      return stream->size;

   if (!(stream->size_fn)) {
      HPDF_SetError(stream->error, HPDF_INVALID_OPERATION, 0);
      return 0;
   }

   if (HPDF_Error_GetCode(stream->error) != 0)
      return 0;

   return stream->size_fn(stream);
}


HpdfStatus
HPDF_Stream_WriteEscapeName(HPDF_Stream      stream,
   char const *value)
{
   char tmp_char[HPDF_LIMIT_MAX_NAME_LEN * 3 + 2];
   HpdfUInt len;
   HpdfInt i;
   const HpdfByte * pos1;
   char* pos2;

   HPDF_PTRACE((" HPDF_Stream_WriteEscapeName\n"));

   len = HPDF_StrLen(value, HPDF_LIMIT_MAX_NAME_LEN);
   pos1 = (HpdfByte *) value;
   pos2 = tmp_char;

   *pos2++ = '/';
   for (i = 0; i < (HpdfInt32) len; i++) {
      HpdfByte c = *pos1++;
      if (HPDF_NEEDS_ESCAPE(c)) {
         *pos2++ = '#';
         *pos2 = (char) (c >> 4);
         if (*pos2 <= 9)
            *pos2 += 0x30;
         else
            *pos2 += 0x41 - 10;
         pos2++;

         *pos2 = (char) (c & 0x0f);
         if (*pos2 <= 9)
            *pos2 += 0x30;
         else
            *pos2 += 0x41 - 10;
         pos2++;
      }
      else
         *pos2++ = c;
   }
   *pos2 = 0;

   return HPDF_Stream_Write(stream, (HpdfByte *) tmp_char, HPDF_StrLen(tmp_char, -1));
}

HpdfStatus
HPDF_Stream_WriteEscapeText2(HPDF_Stream    stream,
   char const   *text,
   HpdfUInt      len)
{
   char buf[HPDF_TEXT_DEFAULT_LEN];
   HpdfUInt idx = 0;
   HpdfInt i;
   const char* p = text;
   HpdfStatus ret;

   HPDF_PTRACE((" HPDF_Stream_WriteEscapeText2\n"));

   /* The following block is commented out because it violates "PDF Spec 7.3.4.2 Literal Strings".
            * It states that the two matching parentheses must still be present to represent an empty
            * string of zero length.
            */
            /*
             if (!len)
                 return HPDF_OK;
            */

   buf[idx++] = '(';

   for (i = 0; i < (HpdfInt) len; i++) {
      HpdfByte c = (HpdfByte) *p++;
      if (HPDF_NEEDS_ESCAPE(c)) {
         buf[idx++] = '\\';

         buf[idx] = (char) (c >> 6);
         buf[idx] += 0x30;
         idx++;
         buf[idx] = (char) ((c & 0x38) >> 3);
         buf[idx] += 0x30;
         idx++;
         buf[idx] = (char) (c & 0x07);
         buf[idx] += 0x30;
         idx++;
      }
      else
         buf[idx++] = c;

      if (idx > HPDF_TEXT_DEFAULT_LEN - 4) {
         ret = HPDF_Stream_Write(stream, (HpdfByte *) buf, idx);
         if (ret != HPDF_OK)
            return ret;
         idx = 0;
      }
   }
   buf[idx++] = ')';

   ret = HPDF_Stream_Write(stream, (HpdfByte *) buf, idx);

   return ret;
}

HpdfStatus
HPDF_Stream_WriteEscapeText(HPDF_Stream    stream,
   char const   *text)
{
   HpdfUInt len;

   HPDF_PTRACE((" HPDF_Stream_WriteEscapeText\n"));

   len = (text == NULL) ? 0 : HPDF_StrLen(text, HPDF_LIMIT_MAX_STRING_LEN);

   return HPDF_Stream_WriteEscapeText2(stream, text, len);
}

HpdfStatus
HPDF_Stream_WriteBinary(HPDF_Stream      stream,
   HpdfByte  const * const data,
   HpdfUInt        len,
   HPDF_Encrypt     e)
{
   char buf[HPDF_TEXT_DEFAULT_LEN];
   HpdfByte ebuf[HPDF_TEXT_DEFAULT_LEN];
   HpdfByte *pbuf = NULL;
   HpdfBool flg = HPDF_FALSE;
   HpdfUInt idx = 0;
   HpdfUInt i;
   HpdfByte const * p;
   HpdfStatus ret = HPDF_OK;

   HPDF_PTRACE((" HPDF_Stream_WriteBinary\n"));

   if (e) 
   {
      if (len <= HPDF_TEXT_DEFAULT_LEN)
      {
         pbuf = ebuf;
      }
      else 
      {
         pbuf = HpdfMemCreateTypeArray(stream->mmgr, HpdfByte, len);
         flg = HPDF_TRUE;
      }

      HPDF_Encrypt_CryptBuf(e, data, pbuf, len);
      p = pbuf;
   }
   else {
      p = data;
   }

   for (i = 0; i < len; i++, p++) {
      char c = (char) (*p >> 4);

      if (c <= 9)
         c += 0x30;
      else
         c += 0x41 - 10;
      buf[idx++] = c;

      c = (char) (*p & 0x0f);
      if (c <= 9)
         c += 0x30;
      else
         c += 0x41 - 10;
      buf[idx++] = c;

      if (idx > HPDF_TEXT_DEFAULT_LEN - 2) {
         ret = HPDF_Stream_Write(stream, (HpdfByte *) buf, idx);
         if (ret != HPDF_OK) {
            if (flg)
               HpdfMemDestroy(stream->mmgr, pbuf);
            return ret;
         }
         idx = 0;
      }
   }

   if (idx > 0) {
      ret = HPDF_Stream_Write(stream, (HpdfByte *) buf, idx);
   }

   if (flg)
      HpdfMemDestroy(stream->mmgr, pbuf);

   return ret;
}


HpdfStatus
HPDF_Stream_WriteToStreamWithDeflate(HPDF_Stream  src,
   HPDF_Stream  dst,
   HPDF_Encrypt  e)
{
#ifdef LIBHPDF_HAVE_ZLIB

#define DEFLATE_BUF_SIZ  ((HpdfInt)(HPDF_STREAM_BUF_SIZ * 1.1) + 13)

   HpdfStatus ret;
   HpdfBool flg;

   z_stream strm;
   Bytef inbuf[HPDF_STREAM_BUF_SIZ];
   Bytef otbuf[DEFLATE_BUF_SIZ];
   HpdfByte ebuf[DEFLATE_BUF_SIZ];

   HPDF_PTRACE((" HPDF_Stream_WriteToStreamWithDeflate\n"));

   /* initialize input stream */
   ret = HPDF_Stream_Seek(src, 0, HPDF_SEEK_SET);
   if (ret != HPDF_OK)
      return ret;

   /* initialize decompression stream. */
   HpdfMemClearType(&strm, z_stream);
   strm.next_out = otbuf;
   strm.avail_out = DEFLATE_BUF_SIZ;

   ret = deflateInit_(&strm, Z_DEFAULT_COMPRESSION, ZLIB_VERSION,
      sizeof(z_stream));
   if (ret != Z_OK)
      return HPDF_SetError(src->error, HPDF_ZLIB_ERROR, ret);

   strm.next_in = inbuf;
   strm.avail_in = 0;

   flg = HPDF_FALSE;
   for (;;) {
      HpdfUInt size = HPDF_STREAM_BUF_SIZ;

      ret = HPDF_Stream_Read(src, inbuf, &size);

      strm.next_in = inbuf;
      strm.avail_in = size;

      if (ret != HPDF_OK) {
         if (ret == HPDF_STREAM_EOF) {
            flg = HPDF_TRUE;
            if (size == 0)
               break;
         }
         else {
            deflateEnd(&strm);
            return ret;
         }
      }

      while (strm.avail_in > 0) {
         ret = deflate(&strm, Z_NO_FLUSH);
         if (ret != Z_OK && ret != Z_STREAM_END) {
            deflateEnd(&strm);
            return HPDF_SetError(src->error, HPDF_ZLIB_ERROR, ret);
         }

         if (strm.avail_out == 0) {
            if (e) {
               HPDF_Encrypt_CryptBuf(e, otbuf, ebuf, DEFLATE_BUF_SIZ);
               ret = HPDF_Stream_Write(dst, ebuf, DEFLATE_BUF_SIZ);
            }
            else
               ret = HPDF_Stream_Write(dst, otbuf, DEFLATE_BUF_SIZ);

            if (ret != HPDF_OK) {
               deflateEnd(&strm);
               return HPDF_SetError(src->error, HPDF_ZLIB_ERROR, ret);
            }

            strm.next_out = otbuf;
            strm.avail_out = DEFLATE_BUF_SIZ;
         }
      }

      if (flg)
         break;
   }

   flg = HPDF_FALSE;
   for (;;) {
      ret = deflate(&strm, Z_FINISH);
      if (ret != Z_OK && ret != Z_STREAM_END) {
         deflateEnd(&strm);
         return HPDF_SetError(src->error, HPDF_ZLIB_ERROR, ret);
      }

      if (ret == Z_STREAM_END)
         flg = HPDF_TRUE;

      if (strm.avail_out < DEFLATE_BUF_SIZ) {
         HpdfUInt osize = DEFLATE_BUF_SIZ - strm.avail_out;
         if (e) {
            HPDF_Encrypt_CryptBuf(e, otbuf, ebuf, osize);
            ret = HPDF_Stream_Write(dst, ebuf, osize);
         }
         else
            ret = HPDF_Stream_Write(dst, otbuf, osize);

         if (ret != HPDF_OK) {
            deflateEnd(&strm);
            return HPDF_SetError(src->error, HPDF_ZLIB_ERROR, ret);
         }

         strm.next_out = otbuf;
         strm.avail_out = DEFLATE_BUF_SIZ;
      }

      if (flg)
         break;
   }

   deflateEnd(&strm);
   return HPDF_OK;
#else /* LIBHPDF_HAVE_ZLIB */
   HPDF_UNUSED(e);
   HPDF_UNUSED(dst);
   HPDF_UNUSED(src);
   return HPDF_UNSUPPORTED_FUNC;
#endif /* LIBHPDF_HAVE_ZLIB */
}

HpdfStatus
HPDF_Stream_WriteToStream(HPDF_Stream  src,
   HPDF_Stream  dst,
   HpdfUInt    filter,
   HPDF_Encrypt  e)
{
   HpdfStatus ret;
   HpdfByte buf[HPDF_STREAM_BUF_SIZ];
   HpdfByte ebuf[HPDF_STREAM_BUF_SIZ];
   HpdfBool flg;

   HPDF_PTRACE((" HPDF_Stream_WriteToStream\n"));
   HPDF_UNUSED(filter);

   if (!dst || !(dst->write_fn)) {
      HPDF_SetError(src->error, HPDF_INVALID_OBJECT, 0);
      return HPDF_INVALID_OBJECT;
   }

   if (HPDF_Error_GetCode(src->error) != HPDF_NOERROR ||
      HPDF_Error_GetCode(dst->error) != HPDF_NOERROR)
      return HPDF_THIS_FUNC_WAS_SKIPPED;

   /* initialize input stream */
   if (HPDF_Stream_Size(src) == 0)
      return HPDF_OK;

#ifdef LIBHPDF_HAVE_ZLIB
   if (filter & HPDF_STREAM_FILTER_FLATE_DECODE)
      return HPDF_Stream_WriteToStreamWithDeflate(src, dst, e);
#endif /* LIBHPDF_HAVE_ZLIB */

   ret = HPDF_Stream_Seek(src, 0, HPDF_SEEK_SET);
   if (ret != HPDF_OK)
      return ret;

   flg = HPDF_FALSE;
   for (;;) {
      HpdfUInt size = HPDF_STREAM_BUF_SIZ;

      ret = HPDF_Stream_Read(src, buf, &size);

      if (ret != HPDF_OK) {
         if (ret == HPDF_STREAM_EOF) {
            flg = HPDF_TRUE;
            if (size == 0)
               break;
         }
         else {
            return ret;
         }
      }

      if (e) {
         HPDF_Encrypt_CryptBuf(e, buf, ebuf, size);
         ret = HPDF_Stream_Write(dst, ebuf, size);
      }
      else {
         ret = HPDF_Stream_Write(dst, buf, size);
      }

      if (ret != HPDF_OK)
         return ret;

      if (flg)
         break;
   }

   return HPDF_OK;
}

HPDF_Stream
   HPDF_FileReader_New(
      HpdfMemMgr * const mmgr,
      char const *fname)
{
   HPDF_Stream stream;
   HPDF_FILEP fp = HPDF_FOPEN(fname, "rb");

   HPDF_PTRACE((" HPDF_FileReader_New\n"));

   if (!fp) {
#ifdef UNDER_CE
      HPDF_SetError(mmgr->error, HPDF_FILE_OPEN_ERROR, GetLastError());
#else
      HPDF_SetError(mmgr->error, HPDF_FILE_OPEN_ERROR, errno);
#endif
      return NULL;
   }

   stream = HpdfMemCreateType(mmgr, HPDF_Stream_Rec);
   if (stream) 
   {
      HpdfMemClearType(stream, HPDF_Stream_Rec);
      stream->sig_bytes = HPDF_STREAM_SIG_BYTES;
      stream->type = HPDF_STREAM_FILE;
      stream->error = mmgr->error;
      stream->mmgr = mmgr;
      stream->read_fn = HPDF_FileReader_ReadFunc;
      stream->seek_fn = HPDF_FileReader_SeekFunc;
      stream->tell_fn = HPDF_FileStream_TellFunc;
      stream->size_fn = HPDF_FileStream_SizeFunc;
      stream->free_fn = HPDF_FileStream_FreeFunc;
      stream->attr = fp;
   }

   return stream;
}

#if defined(WIN32)
HPDF_Stream
   HPDF_FileReader_NewW(
      HpdfMemMgr * const mmgr,
      const wchar_t  *fname)
{
   HPDF_Stream stream;
   HPDF_FILEP fp = HPDF_FOPEN_W(fname, L"rb");

   HPDF_PTRACE((" HPDF_FileReader_NewW\n"));

   if (!fp) {
#ifdef UNDER_CE
      HPDF_SetError(mmgr->error, HPDF_FILE_OPEN_ERROR, GetLastError());
#else
      HPDF_SetError(mmgr->error, HPDF_FILE_OPEN_ERROR, errno);
#endif
      return NULL;
   }

   stream = HpdfMemCreateType(mmgr, HPDF_Stream_Rec);
   if (stream) 
   {
      HpdfMemClearType(stream, HPDF_Stream_Rec);
      stream->sig_bytes = HPDF_STREAM_SIG_BYTES;
      stream->type = HPDF_STREAM_FILE;
      stream->error = mmgr->error;
      stream->mmgr = mmgr;
      stream->read_fn = HPDF_FileReader_ReadFunc;
      stream->seek_fn = HPDF_FileReader_SeekFunc;
      stream->tell_fn = HPDF_FileStream_TellFunc;
      stream->size_fn = HPDF_FileStream_SizeFunc;
      stream->free_fn = HPDF_FileStream_FreeFunc;
      stream->attr = fp;
   }

   return stream;
}
#endif

/*
 *  HPDF_FileReader_ReadFunc
 *
 *  Reading data function for HPDF_FileReader.
 *
 */

HpdfStatus
HPDF_FileReader_ReadFunc(HPDF_Stream  stream,
   HpdfByte    * const ptr,
   HpdfUInt    *siz)
{
   HPDF_FILEP fp = (HPDF_FILEP) stream->attr;
   HpdfUInt rsiz;

   HPDF_PTRACE((" HPDF_FileReader_ReadFunc\n"));

   HpdfMemClear(ptr, *siz);
   rsiz = (HpdfUInt) HPDF_FREAD(ptr, 1, *siz, fp);

   if (rsiz != *siz) {
      if (HPDF_FEOF(fp)) {

         *siz = rsiz;

         return HPDF_STREAM_EOF;
      }

      return HPDF_SetError(stream->error, HPDF_FILE_IO_ERROR, HPDF_FERROR(fp));
   }

   return HPDF_OK;
}

/*
 *  HPDF_FileReader_SeekFunc
 *
 *  Seeking data function for HPDF_FileReader.
 *
 *  stream : Pointer to a HPDF_Stream object.
 *  pos : New position of stream object.
 *  HPDF_whence_mode : Seeking mode describing below.
 *                     HPDF_SEEK_SET : Absolute file position
 *                     HPDF_SEEK_CUR : Relative to the current file position
 *                     HPDF_SEEK_END : Relative to the current end of file.
 *
 *  HPDF_FileReader_seek_fn returns HPDF_OK when successful. On failure
 *  the result is HPDF_FILE_IO_ERROR and HPDF_Error_GetCode2() returns the
 *  error which returned by file seeking function of platform.
 *
 */

HpdfStatus
HPDF_FileReader_SeekFunc(HPDF_Stream     stream,
   HpdfInt         pos,
   HPDF_WhenceMode  mode)
{
   HPDF_FILEP fp = (HPDF_FILEP) stream->attr;
   HpdfInt whence;

   HPDF_PTRACE((" HPDF_FileReader_SeekFunc\n"));

   switch (mode) {
   case HPDF_SEEK_CUR:
      whence = SEEK_CUR;
      break;
   case HPDF_SEEK_END:
      whence = SEEK_END;
      break;
   default:
      whence = SEEK_SET;
   }

   if (HPDF_FSEEK(fp, pos, whence) != 0) {
      return HPDF_SetError(stream->error, HPDF_FILE_IO_ERROR, HPDF_FERROR(fp));
   }

   return HPDF_OK;
}


HpdfInt32
HPDF_FileStream_TellFunc(HPDF_Stream   stream)
{
   HpdfInt32 ret;
   HPDF_FILEP fp = (HPDF_FILEP) stream->attr;

   HPDF_PTRACE((" HPDF_FileReader_TellFunc\n"));

   if ((ret = HPDF_FTELL(fp)) < 0) {
      return HPDF_SetError(stream->error, HPDF_FILE_IO_ERROR,
         HPDF_FERROR(fp));
   }

   return ret;
}


HpdfUInt32
HPDF_FileStream_SizeFunc(HPDF_Stream   stream)
{
   HpdfInt size;
   HpdfInt ptr;
   HPDF_FILEP fp = (HPDF_FILEP) stream->attr;

   HPDF_PTRACE((" HPDF_FileReader_SizeFunc\n"));

   /* save current file-pointer */
   if ((ptr = HPDF_FTELL(fp)) < 0) {
      HPDF_SetError(stream->error, HPDF_FILE_IO_ERROR,
         HPDF_FERROR(fp));
      return 0;
   }

   /* move file-pointer to the end of the file */
   if (HPDF_FSEEK(fp, 0, SEEK_END) < 0) {
      HPDF_SetError(stream->error, HPDF_FILE_IO_ERROR,
         HPDF_FERROR(fp));
      return 0;
   }

   /* get the pointer of the end of the file */
   if ((size = HPDF_FTELL(fp)) < 0) {
      HPDF_SetError(stream->error, HPDF_FILE_IO_ERROR,
         HPDF_FERROR(fp));
      return 0;
   }

   /* restore current file-pointer */
   if (HPDF_FSEEK(fp, ptr, SEEK_SET) < 0) {
      HPDF_SetError(stream->error, HPDF_FILE_IO_ERROR,
         HPDF_FERROR(fp));
      return 0;
   }

   return (HpdfUInt32) size;
}


HPDF_Stream
   HPDF_FileWriter_New(
      HpdfMemMgr * const mmgr,
      char const *fname)
{
   HPDF_Stream stream;
   HPDF_FILEP fp = HPDF_FOPEN(fname, "wb");

   HPDF_PTRACE((" HPDF_FileWriter_New\n"));

   if (!fp) {
#ifdef UNDER_CE
      HPDF_SetError(mmgr->error, HPDF_FILE_OPEN_ERROR, GetLastError());
#else
      HPDF_SetError(mmgr->error, HPDF_FILE_OPEN_ERROR, errno);
#endif
      return NULL;
   }

   stream = HpdfMemCreateType(mmgr, HPDF_Stream_Rec);
   if (stream) 
   {
      HpdfMemClearType(stream, HPDF_Stream_Rec);
      stream->sig_bytes = HPDF_STREAM_SIG_BYTES;
      stream->error = mmgr->error;
      stream->mmgr = mmgr;
      stream->write_fn = HPDF_FileWriter_WriteFunc;
      stream->free_fn = HPDF_FileStream_FreeFunc;
      stream->tell_fn = HPDF_FileStream_TellFunc;
      stream->attr = fp;
      stream->type = HPDF_STREAM_FILE;
   }

   return stream;
}

#if defined(WIN32)
HPDF_Stream
   HPDF_FileWriter_NewW(
      HpdfMemMgr * const mmgr,
      const wchar_t  *fname)
{
   HPDF_Stream stream;
   HPDF_FILEP fp = HPDF_FOPEN_W(fname, L"wb");

   HPDF_PTRACE((" HPDF_FileWriter_NewW\n"));

   if (!fp) {
#ifdef UNDER_CE
      HPDF_SetError(mmgr->error, HPDF_FILE_OPEN_ERROR, GetLastError());
#else
      HPDF_SetError(mmgr->error, HPDF_FILE_OPEN_ERROR, errno);
#endif
      return NULL;
   }

   stream = HpdfMemCreateType(mmgr, HPDF_Stream_Rec);
   if (stream) 
   {
      HpdfMemClearType(stream, HPDF_Stream_Rec);
      stream->sig_bytes = HPDF_STREAM_SIG_BYTES;
      stream->error = mmgr->error;
      stream->mmgr = mmgr;
      stream->write_fn = HPDF_FileWriter_WriteFunc;
      stream->free_fn = HPDF_FileStream_FreeFunc;
      stream->tell_fn = HPDF_FileStream_TellFunc;
      stream->attr = fp;
      stream->type = HPDF_STREAM_FILE;
   }

   return stream;
}
#endif

HpdfStatus
HPDF_FileWriter_WriteFunc(HPDF_Stream      stream,
   HpdfByte  const * const ptr,
   HpdfUInt        siz)
{
   HPDF_FILEP fp;
   HpdfUInt ret;

   HPDF_PTRACE((" HPDF_FileWriter_WriteFunc\n"));

   fp = (HPDF_FILEP) stream->attr;
   ret = (HpdfUInt) HPDF_FWRITE(ptr, 1, siz, fp);

   if (ret != siz) {
      return HPDF_SetError(stream->error, HPDF_FILE_IO_ERROR, HPDF_FERROR(fp));
   }

   return HPDF_OK;
}


void
HPDF_FileStream_FreeFunc(HPDF_Stream  stream)
{
   HPDF_FILEP fp;

   HPDF_PTRACE((" HPDF_FileStream_FreeFunc\n"));

   fp = (HPDF_FILEP) stream->attr;

   if (fp)
      HPDF_FCLOSE(fp);

   stream->attr = NULL;
}

HpdfStatus
   HPDF_MemStream_InWrite(
      HPDF_Stream      stream,
      HpdfByte const **ptr,
      HpdfUInt * const count)
{
   HPDF_MemStreamAttr attr    = (HPDF_MemStreamAttr) stream->attr;
   HpdfUInt           rsize   = attr->buf_siz - attr->w_pos;

   HPDF_PTRACE((" HPDF_MemStream_InWrite\n"));

   if (*count <= 0)
      return HPDF_OK;

   if (rsize >= *count) {
      HPDF_MemCpy(attr->w_ptr, *ptr, *count);
      attr->w_ptr += *count;
      attr->w_pos += *count;
      *count = 0;
   }
   else {
      if (rsize > 0) {
         HPDF_MemCpy(attr->w_ptr, *ptr, rsize);
         *ptr   += rsize;
         *count -= rsize;
      }
      attr->w_ptr = HpdfMemCreateTypeArray(stream->mmgr, HpdfByte, attr->buf_siz);
      if (attr->w_ptr == NULL)
      {
         return HPDF_Error_GetCode(stream->error);
      }

      if (HPDF_List_Add(attr->buf, attr->w_ptr) != HPDF_OK) {
         HpdfMemDestroy(stream->mmgr, attr->w_ptr);
         attr->w_ptr = NULL;

         return HPDF_Error_GetCode(stream->error);
      }
      attr->w_pos = 0;
   }
   return HPDF_OK;
}


HpdfStatus
HPDF_MemStream_WriteFunc(
   HPDF_Stream      stream,
   HpdfByte  const *ptr,
   HpdfUInt         siz)
{
   HpdfUInt wsiz = siz;

   HPDF_PTRACE((" HPDF_MemStream_WriteFunc\n"));

   if (HPDF_Error_GetCode(stream->error) != 0)
      return HPDF_THIS_FUNC_WAS_SKIPPED;

   while (wsiz > 0) 
   {
      HpdfStatus ret = HPDF_MemStream_InWrite(stream, &ptr, &wsiz);
      if (ret != HPDF_OK)
         return ret;
   }

   return HPDF_OK;
}


HpdfInt32
HPDF_MemStream_TellFunc(HPDF_Stream  stream)
{
   HpdfInt32 ret;
   HPDF_MemStreamAttr attr = (HPDF_MemStreamAttr) stream->attr;

   HPDF_PTRACE((" HPDF_MemStream_TellFunc\n"));

   ret = attr->r_ptr_idx * attr->buf_siz;
   ret += attr->r_pos;

   return ret;
}


HpdfUInt32
HPDF_MemStream_SizeFunc(HPDF_Stream  stream)
{
   HPDF_PTRACE((" HPDF_MemStream_SizeFunc\n"));

   return stream->size;
}

HpdfStatus
HPDF_MemStream_SeekFunc(HPDF_Stream      stream,
   HpdfInt         pos,
   HPDF_WhenceMode  mode)
{
   HPDF_MemStreamAttr attr = (HPDF_MemStreamAttr) stream->attr;

   HPDF_PTRACE((" HPDF_MemStream_SeekFunc\n"));

   if (mode == HPDF_SEEK_CUR) {
      pos += (attr->r_ptr_idx * attr->buf_siz);
      pos += attr->r_pos;
   }
   else if (mode == HPDF_SEEK_END)
      pos = stream->size - pos;

   if (pos > (HpdfInt) stream->size) {
      return HPDF_SetError(stream->error, HPDF_STREAM_EOF, 0);
   }

   if (stream->size == 0) {
      return HPDF_OK;
   }

   attr->r_ptr_idx = pos / attr->buf_siz;
   attr->r_pos = pos % attr->buf_siz;
   attr->r_ptr = (HpdfByte *) HPDF_List_ItemAt(attr->buf, attr->r_ptr_idx);
   if (attr->r_ptr == NULL) {
      HPDF_SetError(stream->error, HPDF_INVALID_OBJECT, 0);
      return HPDF_INVALID_OBJECT;
   }
   else
      attr->r_ptr += attr->r_pos;

   return HPDF_OK;
}


HpdfByte *
HPDF_MemStream_GetBufPtr(HPDF_Stream  stream,
   HpdfUInt    index,
   HpdfUInt    *length)
{
   HpdfByte *ret;
   HPDF_MemStreamAttr attr;

   HPDF_PTRACE((" HPDF_MemStream_GetBufPtr\n"));

   if (stream->type != HPDF_STREAM_MEMORY) {
      HPDF_SetError(stream->error, HPDF_INVALID_OBJECT, 0);
      return NULL;
   }

   attr = (HPDF_MemStreamAttr) stream->attr;

   ret = (HpdfByte *) HPDF_List_ItemAt(attr->buf, index);
   if (ret == NULL) {
      HPDF_SetError(stream->error, HPDF_INVALID_PARAMETER, 0);
      *length = 0;
      return NULL;
   }

   *length = (attr->buf->count - 1 == index) ? attr->w_pos : attr->buf_siz;
   return ret;
}


void
HPDF_MemStream_FreeData(HPDF_Stream  stream)
{
   HPDF_MemStreamAttr attr;
   HpdfUInt i;

   HPDF_PTRACE((" HPDF_MemStream_FreeData\n"));

   if (!stream || stream->type != HPDF_STREAM_MEMORY)
      return;

   attr = (HPDF_MemStreamAttr) stream->attr;

   for (i = 0; i < attr->buf->count; i++)
      HpdfMemDestroy(stream->mmgr, HPDF_List_ItemAt(attr->buf, i));

   HPDF_List_Clear(attr->buf);

   stream->size = 0;
   attr->w_pos = attr->buf_siz;
   attr->w_ptr = NULL;
   attr->r_ptr_idx = 0;
   attr->r_pos = 0;
}

void
HPDF_MemStream_FreeFunc(HPDF_Stream  stream)
{
   HPDF_MemStreamAttr attr;

   HPDF_PTRACE((" HPDF_MemStream_FreeFunc\n"));

   attr = (HPDF_MemStreamAttr) stream->attr;
   HPDF_MemStream_FreeData(stream);
   HPDF_List_Free(attr->buf);
   HpdfMemDestroy(stream->mmgr, attr);
   stream->attr = NULL;
}

HPDF_Stream
   HPDF_MemStream_New(
      HpdfMemMgr * const mmgr,
      HpdfUInt  buf_siz)
{
   HPDF_Stream stream;

   HPDF_PTRACE((" HPDF_MemStream_New\n"));

   /* Create new HPDF_Stream object. */
   stream = HpdfMemCreateType(mmgr, HPDF_Stream_Rec);
   if (stream) 
   {
      /* Create attribute struct. */
      HPDF_MemStreamAttr attr = HpdfMemCreateType(mmgr, HPDF_MemStreamAttr_Rec);
      if (!attr) 
      {
         HpdfMemDestroy(mmgr, stream);
         return NULL;
      }

      HpdfMemClearType(stream, HPDF_Stream_Rec);
      HpdfMemClearType(attr,   HPDF_MemStreamAttr_Rec);

      attr->buf = HPDF_List_New(mmgr, HPDF_DEF_ITEMS_PER_BLOCK);
      if (!attr->buf) {
         HpdfMemDestroy(mmgr, stream);
         HpdfMemDestroy(mmgr, attr);
         return NULL;
      }

      stream->sig_bytes = HPDF_STREAM_SIG_BYTES;
      stream->type = HPDF_STREAM_MEMORY;
      stream->error = mmgr->error;
      stream->mmgr = mmgr;
      stream->attr = attr;
      attr->buf_siz = (buf_siz > 0) ? buf_siz : HPDF_STREAM_BUF_SIZ;
      attr->w_pos = attr->buf_siz;

      stream->write_fn = HPDF_MemStream_WriteFunc;
      stream->read_fn = HPDF_MemStream_ReadFunc;
      stream->seek_fn = HPDF_MemStream_SeekFunc;
      stream->tell_fn = HPDF_MemStream_TellFunc;
      stream->size_fn = HPDF_MemStream_SizeFunc;
      stream->free_fn = HPDF_MemStream_FreeFunc;
   }

   return stream;
}

HpdfUInt
HPDF_MemStream_GetBufSize(HPDF_Stream  stream)
{
   HPDF_MemStreamAttr attr;

   HPDF_PTRACE((" HPDF_MemStream_GetBufSize\n"));

   if (!stream || stream->type != HPDF_STREAM_MEMORY)
      return 0;

   attr = (HPDF_MemStreamAttr) stream->attr;
   return attr->buf_siz;
}

HpdfUInt
HPDF_MemStream_GetBufCount(HPDF_Stream  stream)
{
   HPDF_MemStreamAttr attr;

   HPDF_PTRACE((" HPDF_MemStream_GetBufCount\n"));

   if (!stream || stream->type != HPDF_STREAM_MEMORY)
      return 0;

   attr = (HPDF_MemStreamAttr) stream->attr;
   return attr->buf->count;
}

HpdfStatus
HPDF_MemStream_ReadFunc(HPDF_Stream  stream,
   HpdfByte    * const buf,
   HpdfUInt    *size)
{
   HPDF_MemStreamAttr attr = (HPDF_MemStreamAttr) stream->attr;
   HpdfUInt buf_size;
   HpdfUInt rlen = *size;
   HpdfByte *bufTemp = buf;

   HPDF_PTRACE((" HPDF_MemStream_ReadFunc\n"));

   *size = 0;

   while (rlen > 0) {
      HpdfUInt tmp_len;

      if (attr->buf->count == 0)
         return HPDF_STREAM_EOF;

      if (attr->buf->count - 1 > attr->r_ptr_idx)
         tmp_len = attr->buf_siz - attr->r_pos;
      else if (attr->buf->count - 1 == attr->r_ptr_idx)
         tmp_len = attr->w_pos - attr->r_pos;
      else
         return HPDF_STREAM_EOF;

      if (!attr->r_ptr)
         attr->r_ptr = (HpdfByte *) HPDF_List_ItemAt(attr->buf,
            attr->r_ptr_idx);

      if (tmp_len >= rlen) {
         HPDF_MemCpy(bufTemp, attr->r_ptr, rlen);
         attr->r_pos += rlen;
         *size += rlen;
         attr->r_ptr += rlen;
         return HPDF_OK;
      }
      else 
      {
         bufTemp = HPDF_MemCpy(bufTemp, attr->r_ptr, tmp_len);
         rlen -= tmp_len;
         *size += tmp_len;

         if (attr->r_ptr_idx == attr->buf->count - 1) {
            attr->r_ptr += tmp_len;
            attr->r_pos += tmp_len;
            return HPDF_STREAM_EOF;
         }

         attr->r_ptr_idx++;
         attr->r_pos = 0;
         attr->r_ptr = HPDF_MemStream_GetBufPtr(stream, attr->r_ptr_idx,
            &buf_size);
      }
   }

   return HPDF_OK;
}


HpdfStatus
HPDF_MemStream_Rewrite(HPDF_Stream  stream,
   HpdfByte    * const buf,
   HpdfUInt    size)
{
   HPDF_MemStreamAttr attr = (HPDF_MemStreamAttr) stream->attr;
   HpdfUInt buf_size;
   HpdfUInt rlen = size;
   HpdfByte *bufTemp = buf;

   HPDF_PTRACE((" HPDF_MemStream_Rewrite\n"));

   while (rlen > 0) {
      HpdfUInt tmp_len;

      if (attr->buf->count <= attr->r_ptr_idx) {
         HpdfStatus ret = HPDF_MemStream_WriteFunc(stream, buf, rlen);
         attr->r_ptr_idx = attr->buf->count;
         attr->r_pos = attr->w_pos;
         attr->r_ptr = attr->w_ptr;
         return ret;
      }
      else if (attr->buf->count == attr->r_ptr_idx)
         tmp_len = attr->w_pos - attr->r_pos;
      else
         tmp_len = attr->buf_siz - attr->r_pos;

      if (tmp_len >= rlen) {
         HPDF_MemCpy(attr->r_ptr, bufTemp, rlen);
         attr->r_pos += rlen;
         attr->r_ptr += rlen;
         return HPDF_OK;
      }
      else {
         HPDF_MemCpy(attr->r_ptr, bufTemp, tmp_len);
         bufTemp += tmp_len;
         rlen -= tmp_len;
         attr->r_ptr_idx++;

         if (attr->buf->count > attr->r_ptr_idx) {
            attr->r_pos = 0;
            attr->r_ptr = HPDF_MemStream_GetBufPtr(stream, attr->r_ptr_idx,
               &buf_size);
         }
      }
   }
   return HPDF_OK;
}

/*
 *  HPDF_CallbackReader_new
 *
 *  Constructor for HPDF_CallbackReader.
 *
 *  mmgr : Pointer to a HpdfMemMgr * object.
 *  read_fn : Pointer to a user function for reading data.
 *  seek_fn : Pointer to a user function for seeking data.
 *  data : Pointer to a data which defined by user.
 *
 *  return: If success, It returns pointer to new HPDF_Stream object,
 *          otherwise, it returns NULL.
 *
 */


HPDF_Stream
   HPDF_CallbackReader_New(
      HpdfMemMgr * const mmgr,
      HPDF_Stream_Read_Func  read_fn,
      HPDF_Stream_Seek_Func  seek_fn,
      HPDF_Stream_Tell_Func  tell_fn,
      HPDF_Stream_Size_Func  size_fn,
      void*                  data)
{
   HPDF_Stream stream;

   HPDF_PTRACE((" HPDF_CallbackReader_New\n"));

   stream = HpdfMemCreateType(mmgr, HPDF_Stream_Rec);
   if (stream) 
   {
      HpdfMemClearType(stream, HPDF_Stream_Rec);
      stream->sig_bytes = HPDF_STREAM_SIG_BYTES;
      stream->error = mmgr->error;
      stream->mmgr = mmgr;
      stream->read_fn = read_fn;
      stream->seek_fn = seek_fn;
      stream->tell_fn = tell_fn;
      stream->size_fn = size_fn;
      stream->attr = data;
      stream->type = HPDF_STREAM_CALLBACK;
   }

   return stream;
}

/*
 *  HPDF_CallbackWriter_new
 *
 *  Constructor for HPDF_CallbackWriter.
 *
 *  mmgr : Pointer to a HpdfMemMgr * object.
 *  read_fn : Pointer to a user function for writing data.
 *  data : Pointer to a data which defined by user.
 *
 *  return: If success, It returns pointer to new HPDF_Stream object,
 *          otherwise, it returns NULL.
 *
 */


HPDF_Stream
   HPDF_CallbackWriter_New(
      HpdfMemMgr * const mmgr,
      HPDF_Stream_Write_Func  write_fn,
      void  *data)
{
   HPDF_Stream stream;

   HPDF_PTRACE((" HPDF_CallbackWriter_New\n"));

   stream = HpdfMemCreateType(mmgr, HPDF_Stream_Rec);
   if (stream) 
   {
      HpdfMemClearType(stream, HPDF_Stream_Rec);
      stream->sig_bytes = HPDF_STREAM_SIG_BYTES;
      stream->error = mmgr->error;
      stream->mmgr = mmgr;
      stream->write_fn = write_fn;
      stream->attr = data;
      stream->type = HPDF_STREAM_CALLBACK;
   }

   return stream;
}



HpdfStatus
HPDF_Stream_Validate(HPDF_Stream  stream)
{
   if (!stream || stream->sig_bytes != HPDF_STREAM_SIG_BYTES)
      return HPDF_FALSE;
   else
      return HPDF_TRUE;
}

