/*
 * << Haru Free PDF Library >> -- fpdf_utils.h
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

#ifndef _HPDF_UTILS_H
#define _HPDF_UTILS_H

#include "hpdf_config.h"
#include "hpdf_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

HpdfInt      HPDF_AToI(       char const * const str);
HpdfDouble   HPDF_AToF(       char const * const str);
char        *HPDF_IToA(       char       * const str, HpdfInt32 value, char * const eptr);
char        *HPDF_IToA2(      char       * const str, HpdfUInt32 value, HpdfUInt len);
char        *HPDF_FToA(       char       * const s, HpdfReal val, char * const eptr);
HpdfByte    *HPDF_MemCpy(     HpdfByte * const out, HpdfByte const * const in, HpdfUInt count);
HpdfByte    *HPDF_StrCpy(     char       * const out, char const * const in, char * const eptr);
HpdfInt      HPDF_MemCmp(     HpdfByte const * const s1, HpdfByte const * const s2, HpdfUInt count);
HpdfInt      HPDF_StrCmp(     char const * const s1, char const * const s2);
HpdfBool     HpdfMemIsEqual(  HpdfByte const * const buf1, HpdfByte const * const buf2, HpdfUInt count);
HpdfBool     HpdfStrIsEqual(  char const * const s1, char const * const s2);
const char  *HPDF_StrStr(     char const * const s1, char const * const s2, HpdfUInt maxlen);
void        *HPDF_MemSet(     void * const buffer, HpdfByte value, HpdfUInt count);
HpdfUInt     HPDF_StrLen(     char const * const str, HpdfInt maxlen);
HPDF_Box     HPDF_ToBox(      HpdfInt16 left, HpdfInt16 bottom, HpdfInt16 right, HpdfInt16 top);
HpdfPoint    HPDF_ToPoint(    HpdfInt16 x,    HpdfInt16 y);
HpdfRect     HPDF_ToRect(     HpdfReal  left, HpdfReal  bottom, HpdfReal  right, HpdfReal  top);
void         HPDF_UInt16Swap( HpdfUInt16 *value);

#define HpdfMemClear(         BUF, BYTECOUNT)   HPDF_MemSet((void *) BUF, 0, (BYTECOUNT))
#define HpdfMemClearType(     BUF, TYPE)        HPDF_MemSet((void *) BUF, 0, sizeof(TYPE))
#define HpdfMemClearTypeArray(BUF, TYPE, COUNT) HPDF_MemSet((void *) BUF, 0, sizeof(TYPE) * (COUNT))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#define HPDF_NEEDS_ESCAPE(c)    (c < 0x20 || \
                                 c > 0x7e || \
                                 c == '\\' || \
                                 c == '%' || \
                                 c == '#' || \
                                 c == '/' || \
                                 c == '(' || \
                                 c == ')' || \
                                 c == '<' || \
                                 c == '>' || \
                                 c == '[' || \
                                 c == ']' || \
                                 c == '{' || \
                                 c == '}' )  \

#define HPDF_IS_WHITE_SPACE(c)   (c == 0x00 || \
                                 c == 0x09 || \
                                 c == 0x0A || \
                                 c == 0x0C || \
                                 c == 0x0D || \
                                 c == 0x20 ) \

/*----------------------------------------------------------------------------*/
/*----- macros for debug -----------------------------------------------------*/

#ifdef LIBHPDF_DEBUG_TRACE
#ifndef HPDF_PTRACE_ON
#define HPDF_PTRACE_ON
#endif /* HPDF_PTRACE_ON */
#endif /* LIBHPDF_DEBUG_TRACE */

#ifdef HPDF_PTRACE_ON
#define HPDF_PTRACE(ARGS)  HPDF_PRINTF ARGS
#else
#define HPDF_PTRACE(ARGS)  /* do nothing */
#endif /* HPDF_PTRACE */

#ifdef LIBHPDF_DEBUG
#define HPDF_PRINT_BINARY(BUF, LEN, CAPTION) HPDF_PrintBinary(BUF, LEN, CAPTION)
#else
#define HPDF_PRINT_BINARY(BUF, LEN, CAPTION) /* do nothing */
#endif

#endif /* _HPDF_UTILS_H */

