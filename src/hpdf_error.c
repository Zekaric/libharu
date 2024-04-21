/*
 * << Haru Free PDF Library >> -- hpdf_error.c
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
#include "hpdf_error.h"
#include "hpdf_consts.h"
#include "hpdf.h"

#ifndef HPDF_STDCALL
#ifdef HPDF_DLL_MAKE
#define HPDF_STDCALL __stdcall
#else
#ifdef HPDF_DLL
#define HPDF_STDCALL __stdcall
#else
#define HPDF_STDCALL
#endif
#endif
#endif

void HPDF_CopyError( HpdfError       * const dst, HpdfError const * const src);

void 
   HPDF_Error_Init(
      HpdfError       * const error, 
      void *user_data)
{
   HpdfMemClearType(error, HpdfError);

   error->user_data = user_data;
}

HpdfStatus 
   HPDF_Error_GetCode(
      HpdfError const * const error)
{
   return error->error_no;
}

HpdfStatus
   HPDF_Error_GetDetailCode(
      HpdfError const * const error)
{
   return error->detail_no;
}

void
   HPDF_CopyError(
      HpdfError       * const dst,
      HpdfError const * const src)
{
    dst->error_no  = src->error_no;
    dst->detail_no = src->detail_no;
    dst->error_fn  = src->error_fn;
    dst->user_data = src->user_data;
}

HpdfStatus
   HPDF_SetError(
      HpdfError * const error,
      HpdfStatus  error_no,
      HpdfStatus  detail_no)
{
   HPDF_PTRACE(
      (" HPDF_SetError: error_no=0x%04X detail_no=0x%04X\n", 
       (HpdfUInt) error_no, 
       (HpdfUInt) detail_no));

   error->error_no  = error_no;
   error->detail_no = detail_no;

   return error_no;
}

HPDF_EXPORT(HpdfStatus)
   HPDF_CheckError(
      HpdfError const * const error)
{
   HPDF_PTRACE(
      (" HPDF_CheckError: error_no=0x%04X detail_no=0x%04X\n",
       (HpdfUInt) error->error_no, 
       (HpdfUInt) error->detail_no));

   if (error->error_no != HPDF_OK && 
       error->error_fn)
   {
      error->error_fn(error->error_no, error->detail_no, error->user_data);
   }

   return error->error_no;
}

HpdfStatus
   HPDF_RaiseError(
      HpdfError * const error,
      HpdfStatus  error_no,
      HpdfStatus  detail_no)
{
   HPDF_SetError(error, error_no, detail_no);

   return HPDF_CheckError(error);
}

void
   HPDF_Error_Reset(
      HpdfError * const error)
{
   error->error_no  = HPDF_NOERROR;
   error->detail_no = HPDF_NOERROR;
}
