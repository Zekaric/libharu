/*
 * << Haru Free PDF Library 2.0.8 >> -- hpdf.h
 *
 * URL http://libharu.org/
 *
 * Copyright(c) 1999-2006 Takeshi Kanno
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 * It is provided "as is" without express or implied warranty.
 *
 */

#ifndef _HPDF_H
#define _HPDF_H

#include "hpdf_config.h"
#include "hpdf_version.h"

#define HPDF_UNUSED(a)((void)(a))

#ifdef HPDF_DLL_MAKE
#    define HPDF_EXPORT(A)  __declspec(dllexport) A  __stdcall
#else
#    ifdef HPDF_DLL_MAKE_CDECL
#        define HPDF_EXPORT(A)  __declspec(dllexport) A
#    else
#        ifdef HPDF_SHARED_MAKE
#            define HPDF_EXPORT(A)  extern A
#        endif /* HPDF_SHARED_MAKE */
#    endif /* HPDF_DLL_MAKE_CDECL */
#endif /* HPDF_DLL_MAKE */

#ifdef HPDF_DLL
#    define HPDF_SHARED
#    define HPDF_EXPORT(A)  __declspec(dllimport) A  __stdcall
#else
#    ifdef HPDF_DLL_CDECL
#        define HPDF_SHARED
#        define HPDF_EXPORT(A)  __declspec(dllimport) A
#    endif /* HPDF_DLL_CDECL */
#endif /* HPDF_DLL */

#ifdef HPDF_SHARED

#ifndef HPDF_EXPORT
#define HPDF_EXPORT(A) extern A
#endif /* HPDF_EXPORT */

#include "hpdf_consts.h"
#include "hpdf_types.h"

typedef void          HPDF_HANDLE;
typedef HPDF_HANDLE   HpdfObjBool;
typedef HPDF_HANDLE   HpdfDoc;
typedef HPDF_HANDLE   HPDF_Page;
typedef HPDF_HANDLE   HPDF_Pages;
typedef HPDF_HANDLE   HPDF_Stream;
typedef HPDF_HANDLE   HPDF_Image;
typedef HPDF_HANDLE   HPDF_Font;
typedef HPDF_HANDLE   HPDF_Outline;
typedef HPDF_HANDLE   HpdfEncoder;
typedef HPDF_HANDLE   HPDF_3DMeasure;
typedef HPDF_HANDLE   HPDF_ExData;
typedef HPDF_HANDLE   HpdfDestination;
typedef HPDF_HANDLE   HPDF_XObject;
typedef HPDF_HANDLE   HPDF_Annotation;
typedef HPDF_HANDLE   HPDF_ExtGState;
typedef HPDF_HANDLE   HPDF_FontDef;
typedef HPDF_HANDLE   HPDF_U3D;
typedef HPDF_HANDLE   HPDF_JavaScript;
typedef HPDF_HANDLE   HpdfError;
typedef HPDF_HANDLE   HpdfMemMgr;
typedef HPDF_HANDLE   HPDF_Dict;
typedef HPDF_HANDLE   HPDF_EmbeddedFile;
typedef HPDF_HANDLE   HPDF_OutputIntent;
typedef HPDF_HANDLE   HPDF_Xref;
typedef HPDF_HANDLE   HPDF_Shading;

#else

#ifndef HPDF_EXPORT
#define HPDF_EXPORT(A)  A
#endif /* HPDF_EXPORT  */

#include "hpdf_consts.h"
#include "hpdf_doc.h"
#include "hpdf_error.h"
#include "hpdf_pdfa.h"

#endif /* HPDF_SHARED */

#ifdef __cplusplus
extern "C" {
#endif

HPDF_EXPORT(char const *)           HPDF_GetVersion(                                void);
HPDF_EXPORT(HpdfDoc *)              HPDF_NewEx(                                     HPDF_Error_Handler   user_error_fn, HPDF_Alloc_Func      user_alloc_fn, HPDF_Free_Func       user_free_fn, HpdfUInt            mem_pool_buf_size, void                        *user_data);
HPDF_EXPORT(HpdfDoc *)              HPDF_New(                                       HPDF_Error_Handler   user_error_fn, void                *user_data);
HPDF_EXPORT(HpdfStatus)             HPDF_SetErrorHandler(                           HpdfDoc       * const doc, HPDF_Error_Handler  user_error_fn);
HPDF_EXPORT(void)                   HPDF_Free(                                      HpdfDoc       * const doc);
HPDF_EXPORT(HpdfMemMgr *)           HPDF_GetDocMMgr(                                HpdfDoc const * const doc);
HPDF_EXPORT(HpdfStatus)             HPDF_NewDoc(                                    HpdfDoc       * const doc);
HPDF_EXPORT(void)                   HPDF_FreeDoc(                                   HpdfDoc       * const doc);
HPDF_EXPORT(HpdfBool)               HPDF_HasDoc(                                    HpdfDoc       * const doc);
HPDF_EXPORT(void)                   HPDF_FreeDocAll(                                HpdfDoc       * const doc);
HPDF_EXPORT(HpdfStatus)             HPDF_SaveToStream(                              HpdfDoc       * const doc);
HPDF_EXPORT(HpdfStatus)             HPDF_GetContents(                               HpdfDoc       * const doc, HpdfByte * const buf, HpdfUInt32  *size);
HPDF_EXPORT(HpdfUInt32)             HPDF_GetStreamSize(                             HpdfDoc       * const doc);
HPDF_EXPORT(HpdfStatus)             HPDF_ReadFromStream(                            HpdfDoc       * const doc, HpdfByte * const buf, HpdfUInt32   *size);
HPDF_EXPORT(HpdfStatus)             HPDF_ResetStream(                               HpdfDoc       * const doc);
HPDF_EXPORT(HpdfStatus)             HPDF_SaveToFile(                                HpdfDoc       * const doc, char const *file_name);
HPDF_EXPORT(HpdfStatus)             HPDF_GetError(                                  HpdfDoc const * const doc);
HPDF_EXPORT(HpdfStatus)             HPDF_GetErrorDetail(                            HpdfDoc const * const doc);
HPDF_EXPORT(void)                   HPDF_ResetError(                                HpdfDoc       * const doc);
HPDF_EXPORT(HpdfStatus)             HPDF_CheckError(                                HpdfError const * const error);
HPDF_EXPORT(HpdfStatus)             HPDF_SetPagesConfiguration(                     HpdfDoc       * const doc, HpdfUInt   page_per_pages);
HPDF_EXPORT(HPDF_Page)              HPDF_GetPageByIndex(                            HpdfDoc       * const doc, HpdfUInt   index);

#if defined(WIN32)
HPDF_EXPORT(HpdfStatus)             HPDF_SaveToFileW(                               HpdfDoc       * const doc, wchar_t const *file_name);
#endif

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

HPDF_EXPORT(HpdfMemMgr *)           HPDF_GetPageMMgr(                               HPDF_Page  page);
HPDF_EXPORT(HPDF_PageLayout)        HPDF_GetPageLayout(                             HpdfDoc       * const doc);
HPDF_EXPORT(HpdfStatus)             HPDF_SetPageLayout(                             HpdfDoc       * const doc, HPDF_PageLayout   layout);
HPDF_EXPORT(HPDF_PageMode)          HPDF_GetPageMode(                               HpdfDoc       * const doc);
HPDF_EXPORT(HpdfStatus)             HPDF_SetPageMode(                               HpdfDoc       * const doc, HPDF_PageMode   mode);
HPDF_EXPORT(HpdfUInt)               HPDF_GetViewerPreference(                       HpdfDoc       * const doc);
HPDF_EXPORT(HpdfStatus)             HPDF_SetViewerPreference(                       HpdfDoc       * const doc, HpdfUInt    value);
HPDF_EXPORT(HpdfStatus)             HPDF_SetOpenAction(                             HpdfDoc       * const doc, HpdfDestination * const open_action);

/*---------------------------------------------------------------------------*/
/*----- page handling -------------------------------------------------------*/

HPDF_EXPORT(HPDF_Page)              HPDF_GetCurrentPage(                            HpdfDoc       * const doc);
HPDF_EXPORT(HPDF_Page)              HPDF_AddPage(                                   HpdfDoc       * const doc);
HPDF_EXPORT(HPDF_Page)              HPDF_InsertPage(                                HpdfDoc       * const doc, HPDF_Page   page);
HPDF_EXPORT(HpdfStatus)             HPDF_Page_SetWidth(                             HPDF_Page   page, HpdfReal   value);
HPDF_EXPORT(HpdfStatus)             HPDF_Page_SetHeight(                            HPDF_Page   page, HpdfReal   value);
HPDF_EXPORT(HpdfStatus)             HPDF_Page_SetBoundary(                          HPDF_Page           page, HPDF_PageBoundary   boundary, HpdfReal           left, HpdfReal           bottom, HpdfReal           right, HpdfReal           top);
HPDF_EXPORT(HpdfStatus)             HPDF_Page_SetSize(                              HPDF_Page            page, HPDF_PageSizes       size, HPDF_PageDirection   direction);
HPDF_EXPORT(HpdfStatus)             HPDF_Page_SetRotate(                            HPDF_Page     page, HpdfUInt16   angle);
HPDF_EXPORT(HpdfStatus)             HPDF_Page_SetZoom(                              HPDF_Page     page, HpdfReal     zoom);

/*---------------------------------------------------------------------------*/
/*----- font handling -------------------------------------------------------*/

HPDF_EXPORT(HPDF_Font)              HPDF_GetFont(                                   HpdfDoc       * const doc, char const *font_name, char const *encoding_name);
HPDF_EXPORT(char const*)            HPDF_LoadType1FontFromFile(                     HpdfDoc       * const doc, char const *afm_file_name, char const *data_file_name);
HPDF_EXPORT(HPDF_FontDef)           HPDF_GetTTFontDefFromFile(                      HpdfDoc const * const doc, char const *file_name, HpdfBool    embedding);
HPDF_EXPORT(char const*)            HPDF_LoadTTFontFromFile(                        HpdfDoc       * const doc, char const *file_name, HpdfBool    embedding);
HPDF_EXPORT(char const*)            HPDF_LoadTTFontFromFile2(                       HpdfDoc       * const doc, char const *file_name, HpdfUInt    index, HpdfBool    embedding);
HPDF_EXPORT(HpdfStatus)            HPDF_AddPageLabel(                              HpdfDoc       * const doc, HpdfUInt           page_num, HPDF_PageNumStyle   style, HpdfUInt           first_page, char const        *prefix);
HPDF_EXPORT(HpdfStatus)            HPDF_UseJPFonts(                                HpdfDoc       * const doc);
HPDF_EXPORT(HpdfStatus)            HPDF_UseKRFonts(                                HpdfDoc       * const doc);
HPDF_EXPORT(HpdfStatus)            HPDF_UseCNSFonts(                               HpdfDoc       * const doc);
HPDF_EXPORT(HpdfStatus)            HPDF_UseCNTFonts(                               HpdfDoc       * const doc);

#if defined(WIN32)
HPDF_EXPORT(char const*)            HPDF_LoadType1FontFromFileW(                    HpdfDoc       * const doc, wchar_t const *afm_file_name, wchar_t const *data_file_name);
HPDF_EXPORT(HPDF_FontDef)           HPDF_GetTTFontDefFromFileW(                     HpdfDoc const * const doc, wchar_t const *file_name, HpdfBool      embedding);
HPDF_EXPORT(char const*)            HPDF_LoadTTFontFromFileW(                       HpdfDoc       * const doc, wchar_t const *file_name, HpdfBool      embedding);
HPDF_EXPORT(char const*)            HPDF_LoadTTFontFromFile2W(                      HpdfDoc       * const doc, wchar_t const *file_name, HpdfUInt      index, HpdfBool      embedding);
#endif

/*--------------------------------------------------------------------------*/
/*----- outline ------------------------------------------------------------*/

HPDF_EXPORT(HPDF_Outline)           HPDF_CreateOutline(                             HpdfDoc       * const doc, HPDF_Outline   parent, char const *title, HpdfEncoder * const encoder);
HPDF_EXPORT(HpdfStatus)            HPDF_Outline_SetOpened(                         HPDF_Outline  outline, HpdfBool     opened);
HPDF_EXPORT(HpdfStatus)            HPDF_Outline_SetDestination(                    HPDF_Outline      outline, HpdfDestination * const dst);

/*--------------------------------------------------------------------------*/
/*----- destination --------------------------------------------------------*/

HPDF_EXPORT(HpdfDestination *)     HPDF_Page_CreateDestination(                    HPDF_Page   page);
HPDF_EXPORT(HpdfStatus)            HPDF_Destination_SetXYZ(                        HpdfDestination * const dst, HpdfReal         left, HpdfReal         top, HpdfReal         zoom);
HPDF_EXPORT(HpdfStatus)            HPDF_Destination_SetFit(                        HpdfDestination * const dst);
HPDF_EXPORT(HpdfStatus)            HPDF_Destination_SetFitH(                       HpdfDestination * const dst, HpdfReal         top);
HPDF_EXPORT(HpdfStatus)            HPDF_Destination_SetFitV(                       HpdfDestination * const dst, HpdfReal         left);
HPDF_EXPORT(HpdfStatus)            HPDF_Destination_SetFitR(                       HpdfDestination * const dst, HpdfReal         left, HpdfReal         bottom, HpdfReal         right, HpdfReal         top);
HPDF_EXPORT(HpdfStatus)            HPDF_Destination_SetFitB(                       HpdfDestination * const dst);
HPDF_EXPORT(HpdfStatus)            HPDF_Destination_SetFitBH(                      HpdfDestination * const dst, HpdfReal         top);
HPDF_EXPORT(HpdfStatus)            HPDF_Destination_SetFitBV(                      HpdfDestination * const dst, HpdfReal         left);

/*--------------------------------------------------------------------------*/
/*----- encoder ------------------------------------------------------------*/

HPDF_EXPORT(HpdfEncoder *)           HPDF_GetEncoder(                                HpdfDoc       * const doc, char const *encoding_name);
HPDF_EXPORT(HpdfEncoder *)           HPDF_GetCurrentEncoder(                         HpdfDoc       * const doc);
HPDF_EXPORT(HpdfStatus)            HPDF_SetCurrentEncoder(                         HpdfDoc       * const doc, char const *encoding_name);
HPDF_EXPORT(HpdfEncoderType)       HPDF_Encoder_GetType(                           HpdfEncoder const * const   encoder);
HPDF_EXPORT(HPDF_ByteType)          HPDF_Encoder_GetByteType(                       HpdfEncoder const * const    encoder, char const    *text, HpdfUInt       index);
HPDF_EXPORT(HpdfUnicode)           HPDF_Encoder_GetUnicode(                        HpdfEncoder const * const   encoder, HpdfUInt16    code);
HPDF_EXPORT(HPDF_WritingMode)       HPDF_Encoder_GetWritingMode(                    HpdfEncoder const * const   encoder);
HPDF_EXPORT(HpdfStatus)            HPDF_UseJPEncodings(                            HpdfDoc       * const doc);
HPDF_EXPORT(HpdfStatus)            HPDF_UseKREncodings(                            HpdfDoc       * const doc);
HPDF_EXPORT(HpdfStatus)            HPDF_UseCNSEncodings(                           HpdfDoc       * const doc);
HPDF_EXPORT(HpdfStatus)            HPDF_UseCNTEncodings(                           HpdfDoc       * const doc);
HPDF_EXPORT(HpdfStatus)            HPDF_UseUTFEncodings(                           HpdfDoc       * const doc);

/*--------------------------------------------------------------------------*/
/*----- XObject ------------------------------------------------------------*/

HPDF_EXPORT(HPDF_XObject)           HPDF_Page_CreateXObjectFromImage(               HpdfDoc const * const doc, HPDF_Page page, HpdfRect const rect, HPDF_Image image, HpdfBool zoom);
HPDF_EXPORT(HPDF_XObject)           HPDF_Page_CreateXObjectAsWhiteRect(             HpdfDoc const * const doc, HPDF_Page page, HpdfRect const rect);

/*--------------------------------------------------------------------------*/
/*----- annotation ---------------------------------------------------------*/

HPDF_EXPORT(HPDF_Annotation)        HPDF_Page_Create3DAnnot(                        HPDF_Page       page, HpdfRect const rect, HpdfBool const tb, HpdfBool const np, HPDF_U3D        u3d, HPDF_Image      ap);
HPDF_EXPORT(HPDF_Annotation)        HPDF_Page_CreateTextAnnot(                      HPDF_Page       page, HpdfRect const rect, char const *text, HpdfEncoder * const encoder);
HPDF_EXPORT(HPDF_Annotation)        HPDF_Page_CreateFreeTextAnnot(                  HPDF_Page       page, HpdfRect const rect, char const *text, HpdfEncoder * const encoder);
HPDF_EXPORT(HPDF_Annotation)        HPDF_Page_CreateLineAnnot(                      HPDF_Page       page,                      char const *text, HpdfEncoder * const encoder);
HPDF_EXPORT(HPDF_Annotation)        HPDF_Page_CreateWidgetAnnot_WhiteOnlyWhilePrint(HpdfDoc const * const doc, HPDF_Page page, HpdfRect const rect);
HPDF_EXPORT(HPDF_Annotation)        HPDF_Page_CreateWidgetAnnot(                    HPDF_Page  page, HpdfRect const rect);
HPDF_EXPORT(HPDF_Annotation)        HPDF_Page_CreateLinkAnnot(                      HPDF_Page          page, HpdfRect const rect, HpdfDestination * const dst);
HPDF_EXPORT(HPDF_Annotation)        HPDF_Page_CreateURILinkAnnot(                   HPDF_Page     page, HpdfRect const rect, char const  *uri);
            HPDF_Annotation         HPDF_Page_CreateTextMarkupAnnot(                HPDF_Page     page, HpdfRect const rect, char const *text, HpdfEncoder * const encoder, HPDF_AnnotType subType);
HPDF_EXPORT(HPDF_Annotation)        HPDF_Page_CreateHighlightAnnot(                 HPDF_Page   page, HpdfRect const rect, char const *text, HpdfEncoder * const encoder);
HPDF_EXPORT(HPDF_Annotation)        HPDF_Page_CreateUnderlineAnnot(                 HPDF_Page    page, HpdfRect const rect, char const *text, HpdfEncoder * const encoder);
HPDF_EXPORT(HPDF_Annotation)        HPDF_Page_CreateSquigglyAnnot(                  HPDF_Page    page, HpdfRect const rect, char const *text, HpdfEncoder * const encoder);
HPDF_EXPORT(HPDF_Annotation)        HPDF_Page_CreateStrikeOutAnnot(                 HPDF_Page   page, HpdfRect const rect, char const *text, HpdfEncoder * const encoder);
HPDF_EXPORT(HPDF_Annotation)        HPDF_Page_CreatePopupAnnot(                     HPDF_Page          page, HpdfRect const rect, HPDF_Annotation    parent);
HPDF_EXPORT(HPDF_Annotation)        HPDF_Page_CreateStampAnnot(                     HPDF_Page           page, HpdfRect const rect, HPDF_StampAnnotName name, char const *text, HpdfEncoder * const encoder);
HPDF_EXPORT(HPDF_Annotation)        HPDF_Page_CreateProjectionAnnot(                HPDF_Page page, HpdfRect const rect, char const *text, HpdfEncoder * const encoder);
HPDF_EXPORT(HPDF_Annotation)        HPDF_Page_CreateSquareAnnot(                    HPDF_Page          page, HpdfRect const rect, char const *text, HpdfEncoder * const encoder);
HPDF_EXPORT(HPDF_Annotation)        HPDF_Page_CreateCircleAnnot(                    HPDF_Page          page, HpdfRect const rect, char const *text, HpdfEncoder * const encoder);
HPDF_EXPORT(HpdfStatus)            HPDF_LinkAnnot_SetHighlightMode(                HPDF_Annotation           annot, HPDF_AnnotHighlightMode   mode);
HPDF_EXPORT(HpdfStatus)            HPDF_LinkAnnot_SetJavaScript(                   HPDF_Annotation    annot, HPDF_JavaScript    javascript);
HPDF_EXPORT(HpdfStatus)            HPDF_LinkAnnot_SetBorderStyle(                  HPDF_Annotation  annot, HpdfReal        width, HpdfUInt16      dash_on, HpdfUInt16      dash_off);
HPDF_EXPORT(HpdfStatus)            HPDF_TextAnnot_SetIcon(                         HPDF_Annotation   annot, HPDF_AnnotIcon    icon);
HPDF_EXPORT(HpdfStatus)            HPDF_TextAnnot_SetOpened(                       HPDF_Annotation   annot, HpdfBool          opened);
HPDF_EXPORT(HpdfStatus)            HPDF_Annot_SetRGBColor(                         HPDF_Annotation annot, HPDF_RGBColor color);
HPDF_EXPORT(HpdfStatus)            HPDF_Annot_SetCMYKColor(                        HPDF_Annotation annot, HPDF_CMYKColor color);
HPDF_EXPORT(HpdfStatus)            HPDF_Annot_SetGrayColor(                        HPDF_Annotation annot, HpdfReal color);
HPDF_EXPORT(HpdfStatus)            HPDF_Annot_SetNoColor(                          HPDF_Annotation annot);
HPDF_EXPORT(HpdfStatus)            HPDF_MarkupAnnot_SetTitle(                      HPDF_Annotation annot, char const* name);
HPDF_EXPORT(HpdfStatus)            HPDF_MarkupAnnot_SetSubject(                    HPDF_Annotation annot, char const* name);
HPDF_EXPORT(HpdfStatus)            HPDF_MarkupAnnot_SetCreationDate(               HPDF_Annotation annot, HPDF_Date value);
HPDF_EXPORT(HpdfStatus)            HPDF_MarkupAnnot_SetTransparency(               HPDF_Annotation annot, HpdfReal value);
HPDF_EXPORT(HpdfStatus)            HPDF_MarkupAnnot_SetIntent(                     HPDF_Annotation  annot, HPDF_AnnotIntent  intent);
HPDF_EXPORT(HpdfStatus)            HPDF_MarkupAnnot_SetPopup(                      HPDF_Annotation annot, HPDF_Annotation popup);
HPDF_EXPORT(HpdfStatus)            HPDF_MarkupAnnot_SetRectDiff(                   HPDF_Annotation  annot, HpdfRect const rect); /* RD entry */
HPDF_EXPORT(HpdfStatus)            HPDF_MarkupAnnot_SetCloudEffect(                HPDF_Annotation  annot, HpdfInt cloudIntensity); /* BE entry */
HPDF_EXPORT(HpdfStatus)            HPDF_MarkupAnnot_SetInteriorRGBColor(           HPDF_Annotation  annot, HPDF_RGBColor color); /* IC with RGB entry */
HPDF_EXPORT(HpdfStatus)            HPDF_MarkupAnnot_SetInteriorCMYKColor(          HPDF_Annotation  annot, HPDF_CMYKColor color); /* IC with CMYK entry */
HPDF_EXPORT(HpdfStatus)            HPDF_MarkupAnnot_SetInteriorGrayColor(          HPDF_Annotation  annot, HpdfReal color); /* IC with Gray entry */
HPDF_EXPORT(HpdfStatus)            HPDF_MarkupAnnot_SetInteriorTransparent(        HPDF_Annotation  annot); /* IC with No Color entry */
HPDF_EXPORT(HpdfStatus)            HPDF_TextMarkupAnnot_SetQuadPoints(             HPDF_Annotation annot, HpdfPoint const lb, HpdfPoint const rb, HpdfPoint const rt, HpdfPoint const lt); /* l-left, r-right, b-bottom, t-top positions */
HPDF_EXPORT(HpdfStatus)            HPDF_Annot_Set3DView(                           HpdfMemMgr * const mmgr, HPDF_Annotation annot, HPDF_Annotation annot3d, HPDF_Dict   view);
HPDF_EXPORT(HpdfStatus)            HPDF_PopupAnnot_SetOpened(                      HPDF_Annotation   annot, HpdfBool         opened);
HPDF_EXPORT(HpdfStatus)            HPDF_FreeTextAnnot_SetLineEndingStyle(          HPDF_Annotation annot, HPDF_LineAnnotEndingStyle startStyle, HPDF_LineAnnotEndingStyle endStyle);
HPDF_EXPORT(HpdfStatus)            HPDF_FreeTextAnnot_Set3PointCalloutLine(        HPDF_Annotation annot, HpdfPoint const startPoint, HpdfPoint const kneePoint, HpdfPoint const endPoint); /* Callout line will be in default user space */
HPDF_EXPORT(HpdfStatus)            HPDF_FreeTextAnnot_Set2PointCalloutLine(        HPDF_Annotation annot, HpdfPoint const startPoint, HpdfPoint const endPoint); /* Callout line will be in default user space */
HPDF_EXPORT(HpdfStatus)            HPDF_FreeTextAnnot_SetDefaultStyle(             HPDF_Annotation  annot, char const* style);
HPDF_EXPORT(HpdfStatus)            HPDF_LineAnnot_SetPosition(                     HPDF_Annotation annot, HpdfPoint const startPoint, HPDF_LineAnnotEndingStyle startStyle, HpdfPoint const endPoint, HPDF_LineAnnotEndingStyle endStyle);
HPDF_EXPORT(HpdfStatus)            HPDF_LineAnnot_SetLeader(                       HPDF_Annotation annot, HpdfInt leaderLen, HpdfInt leaderExtLen, HpdfInt leaderOffsetLen);
HPDF_EXPORT(HpdfStatus)            HPDF_LineAnnot_SetCaption(                      HPDF_Annotation annot, HpdfBool showCaption, HPDF_LineAnnotCapPosition position, HpdfInt horzOffset, HpdfInt vertOffset);
HPDF_EXPORT(HpdfStatus)            HPDF_Annotation_SetBorderStyle(                 HPDF_Annotation  annot, HPDF_BSSubtype   subtype, HpdfReal        width, HpdfUInt16      dash_on, HpdfUInt16      dash_off, HpdfUInt16      dash_phase);
HPDF_EXPORT(HpdfStatus)            HPDF_ProjectionAnnot_SetExData(                 HPDF_Annotation annot, HPDF_ExData exdata);

/*--------------------------------------------------------------------------*/
/*----- 3D Measure ---------------------------------------------------------*/

HPDF_EXPORT(HPDF_3DMeasure)         HPDF_Page_Create3DC3DMeasure(                   HPDF_Page       page, HpdfPoint3D const firstanchorpoint, HpdfPoint3D const textanchorpoint);
HPDF_EXPORT(HPDF_3DMeasure)         HPDF_Page_CreatePD33DMeasure(                   HPDF_Page       page, HpdfPoint3D const annotationPlaneNormal, HpdfPoint3D const firstAnchorPoint, HpdfPoint3D const secondAnchorPoint, HpdfPoint3D const leaderLinesDirection, HpdfPoint3D const measurementValuePoint, HpdfPoint3D const textYDirection, HpdfReal const value, char const*     unitsString);
HPDF_EXPORT(HpdfStatus)            HPDF_3DMeasure_SetName(                         HPDF_3DMeasure measure, char const* name);
HPDF_EXPORT(HpdfStatus)            HPDF_3DMeasure_SetColor(                        HPDF_3DMeasure measure, HPDF_RGBColor color);
HPDF_EXPORT(HpdfStatus)            HPDF_3DMeasure_SetTextSize(                     HPDF_3DMeasure measure, HpdfReal textsize);
HPDF_EXPORT(HpdfStatus)            HPDF_3DC3DMeasure_SetTextBoxSize(               HPDF_3DMeasure measure, HpdfInt32 x, HpdfInt32 y);
HPDF_EXPORT(HpdfStatus)            HPDF_3DC3DMeasure_SetText(                      HPDF_3DMeasure measure, char const *text, HpdfEncoder * const encoder);
HPDF_EXPORT(HpdfStatus)            HPDF_3DC3DMeasure_SetProjectionAnotation(       HPDF_3DMeasure measure, HPDF_Annotation projectionanotation);

/*--------------------------------------------------------------------------*/
/*----- External Data ---------------------------------------------------------*/

HPDF_EXPORT(HPDF_ExData)            HPDF_Page_Create3DAnnotExData(                  HPDF_Page       page );
HPDF_EXPORT(HpdfStatus)            HPDF_3DAnnotExData_Set3DMeasurement(            HPDF_ExData exdata, HPDF_3DMeasure measure);

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*----- 3D View ---------------------------------------------------------*/

HPDF_EXPORT(HPDF_Dict)              HPDF_Page_Create3DView(                         HPDF_Page       page, HPDF_U3D        u3d, HPDF_Annotation annot3d, char const *name);
HPDF_EXPORT(HpdfStatus)            HPDF_3DView_Add3DC3DMeasure(                    HPDF_Dict       view, HPDF_3DMeasure measure);

/*--------------------------------------------------------------------------*/
/*----- image data ---------------------------------------------------------*/

HPDF_EXPORT(HPDF_Image)             HPDF_LoadPngImageFromMem(                       HpdfDoc const * const doc, const HpdfByte * const buffer, HpdfUInt     size);
HPDF_EXPORT(HPDF_Image)             HPDF_LoadPngImageFromFile(                      HpdfDoc const * const doc, char const   *filename);
HPDF_EXPORT(HPDF_Image)             HPDF_LoadPngImageFromFile2(                     HpdfDoc const * const doc, char const   *filename);
HPDF_EXPORT(HPDF_Image)             HPDF_LoadJpegImageFromFile(                     HpdfDoc       * const doc, char const   *filename);
HPDF_EXPORT(HPDF_Image)             HPDF_LoadJpegImageFromMem(                      HpdfDoc       * const doc, const HpdfByte * const buffer, HpdfUInt      size);
HPDF_EXPORT(HPDF_Image)             HPDF_LoadU3DFromFile(                           HpdfDoc       * const doc, char const   *filename);
HPDF_EXPORT(HPDF_Image)             HPDF_LoadU3DFromMem(                            HpdfDoc       * const doc, const HpdfByte * const buffer, HpdfUInt      size);
HPDF_EXPORT(HPDF_Image)             HPDF_Image_LoadRaw1BitImageFromMem(             HpdfDoc       * const doc, const HpdfByte * const buf, HpdfUInt          width, HpdfUInt          height, HpdfUInt          line_width, HpdfBool          black_is1, HpdfBool          top_is_first);
HPDF_EXPORT(HPDF_Image)             HPDF_LoadRawImageFromFile(                      HpdfDoc       * const doc, char const        *filename, HpdfUInt          width, HpdfUInt          height, HPDF_ColorSpace    color_space);
HPDF_EXPORT(HPDF_Image)             HPDF_LoadRawImageFromMem(                       HpdfDoc       * const doc, const HpdfByte * const buf, HpdfUInt          width, HpdfUInt          height, HPDF_ColorSpace    color_space, HpdfUInt          bits_per_component);
HPDF_EXPORT(HpdfStatus)             HPDF_Image_AddSMask(                            HPDF_Image    image, HPDF_Image    smask);
HPDF_EXPORT(HpdfPoint)              HPDF_Image_GetSize(                             HPDF_Image  image);
HPDF_EXPORT(HpdfStatus)             HPDF_Image_GetSize2(                            HPDF_Image  image, HpdfPoint * const size);
HPDF_EXPORT(HpdfUInt)               HPDF_Image_GetWidth(                            HPDF_Image   image);
HPDF_EXPORT(HpdfUInt)               HPDF_Image_GetHeight(                           HPDF_Image   image);
HPDF_EXPORT(HpdfUInt)               HPDF_Image_GetBitsPerComponent(                 HPDF_Image  image);
HPDF_EXPORT(char const*)            HPDF_Image_GetColorSpace(                       HPDF_Image  image);
HPDF_EXPORT(HpdfStatus)             HPDF_Image_SetColorMask(                        HPDF_Image   image, HpdfUInt    rmin, HpdfUInt    rmax, HpdfUInt    gmin, HpdfUInt    gmax, HpdfUInt    bmin, HpdfUInt    bmax);
HPDF_EXPORT(HpdfStatus)             HPDF_Image_SetMaskImage(                        HPDF_Image   image, HPDF_Image   mask_image);

#if defined(WIN32)
HPDF_EXPORT(HPDF_Image)             HPDF_LoadPngImageFromFileW(                     HpdfDoc const * const doc, wchar_t const * const filename);
HPDF_EXPORT(HPDF_Image)             HPDF_LoadPngImageFromFile2W(                    HpdfDoc const * const doc, wchar_t const * const filename);
HPDF_EXPORT(HPDF_Image)             HPDF_LoadJpegImageFromFileW(                    HpdfDoc       * const doc, wchar_t const * const filename);
HPDF_EXPORT(HPDF_Image)             HPDF_LoadU3DFromFileW(                          HpdfDoc       * const doc, wchar_t const * const filename);
HPDF_EXPORT(HPDF_Image)             HPDF_LoadRawImageFromFileW(                     HpdfDoc       * const doc, wchar_t const * const filename, HpdfUInt          width, HpdfUInt          height, HPDF_ColorSpace    color_space);
#endif

/*--------------------------------------------------------------------------*/
/*----- info dictionary ----------------------------------------------------*/

HPDF_EXPORT(HpdfStatus)            HPDF_SetInfoAttr(                               HpdfDoc       * const doc, HPDF_InfoType   type, char const    *value);
HPDF_EXPORT(char const*)            HPDF_GetInfoAttr(                               HpdfDoc       * const doc, HPDF_InfoType   type);
HPDF_EXPORT(HpdfStatus)            HPDF_SetInfoDateAttr(                           HpdfDoc       * const doc, HPDF_InfoType   type, HPDF_Date       value);

/*--------------------------------------------------------------------------*/
/*----- encryption ---------------------------------------------------------*/

HPDF_EXPORT(HpdfStatus)            HPDF_SetPassword(                               HpdfDoc       * const doc, char const  *owner_passwd, char const  *user_passwd);
HPDF_EXPORT(HpdfStatus)            HPDF_SetPermission(                             HpdfDoc       * const doc, HpdfUInt   permission);
HPDF_EXPORT(HpdfStatus)            HPDF_SetEncryptionMode(                         HpdfDoc       * const doc, HPDF_EncryptMode   mode, HpdfUInt          key_len);

/*--------------------------------------------------------------------------*/
/*----- compression --------------------------------------------------------*/

HPDF_EXPORT(HpdfStatus)            HPDF_SetCompressionMode(                        HpdfDoc       * const doc, HpdfUInt   mode);

/*--------------------------------------------------------------------------*/
/*----- font ---------------------------------------------------------------*/

HPDF_EXPORT(char const*)            HPDF_Font_GetFontName(                          HPDF_Font    font);
HPDF_EXPORT(char const*)            HPDF_Font_GetEncodingName(                      HPDF_Font    font);
HPDF_EXPORT(HpdfInt)               HPDF_Font_GetUnicodeWidth(                      HPDF_Font       font, HpdfUnicode    code);
HPDF_EXPORT(HPDF_Box)               HPDF_Font_GetBBox(                              HPDF_Font    font);
HPDF_EXPORT(HpdfInt)               HPDF_Font_GetAscent(                            HPDF_Font  font);
HPDF_EXPORT(HpdfInt)               HPDF_Font_GetDescent(                           HPDF_Font  font);
HPDF_EXPORT(HpdfUInt)              HPDF_Font_GetXHeight(                           HPDF_Font  font);
HPDF_EXPORT(HpdfUInt)              HPDF_Font_GetCapHeight(                         HPDF_Font  font);
HPDF_EXPORT(HPDF_TextWidth)         HPDF_Font_TextWidth(                            HPDF_Font          font, const HpdfByte * const text, HpdfUInt          len);
HPDF_EXPORT(HpdfUInt)              HPDF_Font_MeasureText(                          HPDF_Font          font, const HpdfByte * const text, HpdfUInt          len, HpdfReal          width, HpdfReal          font_size, HpdfReal          char_space, HpdfReal          word_space, HpdfBool          wordwrap, HpdfReal         *real_width);

/*--------------------------------------------------------------------------*/
/*----- attachments -------------------------------------------------------*/

HPDF_EXPORT(HPDF_EmbeddedFile)      HPDF_AttachFile(                                HpdfDoc       * const doc, char const *file);

#if defined(WIN32)
HPDF_EXPORT(HPDF_EmbeddedFile)      HPDF_AttachFileW(                               HpdfDoc       * const doc, wchar_t const *file);
#endif

/*--------------------------------------------------------------------------*/
/*----- extended graphics state --------------------------------------------*/

HPDF_EXPORT(HPDF_ExtGState)         HPDF_CreateExtGState(                           HpdfDoc       * const doc);
HPDF_EXPORT(HpdfStatus)            HPDF_ExtGState_SetAlphaStroke(                  HPDF_ExtGState   ext_gstate, HpdfReal        value);
HPDF_EXPORT(HpdfStatus)            HPDF_ExtGState_SetAlphaFill(                    HPDF_ExtGState   ext_gstate, HpdfReal        value);
HPDF_EXPORT(HpdfStatus)            HPDF_ExtGState_SetBlendMode(                    HPDF_ExtGState   ext_gstate, HPDF_BlendMode   mode);

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

HPDF_EXPORT(HpdfReal)               HPDF_Page_TextWidth(                            HPDF_Page    page, char const *text);
HPDF_EXPORT(HpdfUInt)               HPDF_Page_MeasureText(                          HPDF_Page    page, char const *text, HpdfReal    width, HpdfBool    wordwrap, HpdfReal   *real_width);
HPDF_EXPORT(HpdfReal)               HPDF_Page_GetWidth(                             HPDF_Page   page);
HPDF_EXPORT(HpdfReal)               HPDF_Page_GetHeight(                            HPDF_Page   page);
HPDF_EXPORT(HpdfUInt16)             HPDF_Page_GetGMode(                             HPDF_Page   page);
HPDF_EXPORT(HpdfPoint)              HPDF_Page_GetCurrentPos(                        HPDF_Page   page);
HPDF_EXPORT(HpdfStatus)             HPDF_Page_GetCurrentPos2(                       HPDF_Page    page, HpdfPoint * const pos);
HPDF_EXPORT(HpdfPoint)              HPDF_Page_GetCurrentTextPos(                    HPDF_Page   page);
HPDF_EXPORT(HpdfStatus)             HPDF_Page_GetCurrentTextPos2(                   HPDF_Page    page, HpdfPoint * const pos);
HPDF_EXPORT(HPDF_Font)              HPDF_Page_GetCurrentFont(                       HPDF_Page   page);
HPDF_EXPORT(HpdfReal)               HPDF_Page_GetCurrentFontSize(                   HPDF_Page   page);
HPDF_EXPORT(HPDF_TransMatrix)       HPDF_Page_GetTransMatrix(                       HPDF_Page   page);
HPDF_EXPORT(HpdfReal)               HPDF_Page_GetLineWidth(                         HPDF_Page   page);
HPDF_EXPORT(HPDF_LineCap)           HPDF_Page_GetLineCap(                           HPDF_Page   page);
HPDF_EXPORT(HPDF_LineJoin)          HPDF_Page_GetLineJoin(                          HPDF_Page   page);
HPDF_EXPORT(HpdfReal)               HPDF_Page_GetMiterLimit(                        HPDF_Page   page);
HPDF_EXPORT(HPDF_DashMode)          HPDF_Page_GetDash(                              HPDF_Page   page);
HPDF_EXPORT(HpdfReal)               HPDF_Page_GetFlat(                              HPDF_Page   page);
HPDF_EXPORT(HpdfReal)               HPDF_Page_GetCharSpace(                         HPDF_Page   page);
HPDF_EXPORT(HpdfReal)               HPDF_Page_GetWordSpace(                         HPDF_Page   page);
HPDF_EXPORT(HpdfReal)               HPDF_Page_GetHorizontalScalling(                HPDF_Page   page);
HPDF_EXPORT(HpdfReal)               HPDF_Page_GetTextLeading(                       HPDF_Page   page);
HPDF_EXPORT(HPDF_TextRenderingMode) HPDF_Page_GetTextRenderingMode(                 HPDF_Page   page);
HPDF_EXPORT(HpdfReal)               HPDF_Page_GetTextRise(                          HPDF_Page   page);
HPDF_EXPORT(HPDF_RGBColor)          HPDF_Page_GetRGBFill(                           HPDF_Page   page);
HPDF_EXPORT(HPDF_RGBColor)          HPDF_Page_GetRGBStroke(                         HPDF_Page   page);
HPDF_EXPORT(HPDF_CMYKColor)         HPDF_Page_GetCMYKFill(                          HPDF_Page   page);
HPDF_EXPORT(HPDF_CMYKColor)         HPDF_Page_GetCMYKStroke(                        HPDF_Page   page);
HPDF_EXPORT(HpdfReal)               HPDF_Page_GetGrayFill(                          HPDF_Page   page);
HPDF_EXPORT(HpdfReal)               HPDF_Page_GetGrayStroke(                        HPDF_Page   page);
HPDF_EXPORT(HPDF_ColorSpace)        HPDF_Page_GetStrokingColorSpace(                HPDF_Page   page);
HPDF_EXPORT(HPDF_ColorSpace)        HPDF_Page_GetFillingColorSpace(                 HPDF_Page   page);
HPDF_EXPORT(HPDF_TransMatrix)       HPDF_Page_GetTextMatrix(                        HPDF_Page   page);
HPDF_EXPORT(HpdfUInt)               HPDF_Page_GetGStateDepth(                       HPDF_Page   page);

/*--------------------------------------------------------------------------*/
/*----- GRAPHICS OPERATORS -------------------------------------------------*/

/*--- General graphics state ---------------------------------------------*/

/* w */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_SetLineWidth(                         HPDF_Page  page, HpdfReal  line_width);

/* J */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_SetLineCap(                           HPDF_Page     page, HPDF_LineCap  line_cap);

/* j */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_SetLineJoin(                          HPDF_Page      page, HPDF_LineJoin  line_join);

/* M */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_SetMiterLimit(                        HPDF_Page  page, HpdfReal  miter_limit);

/* d */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_SetDash(                              HPDF_Page           page, const HpdfReal  *dash_ptn, HpdfUInt           num_param, HpdfReal           phase);

/* ri --not implemented yet */

/* i */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_SetFlat(                              HPDF_Page    page, HpdfReal    flatness);

/* gs */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_SetExtGState(                         HPDF_Page        page, HPDF_ExtGState   ext_gstate);

/* sh */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_SetShading(                           HPDF_Page    page, HPDF_Shading shading);


/*--- Special graphic state operator --------------------------------------*/

/* q */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_GSave(                                HPDF_Page    page);

/* Q */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_GRestore(                             HPDF_Page    page);

/* cm */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_Concat(                               HPDF_Page    page, HpdfReal    a, HpdfReal    b, HpdfReal    c, HpdfReal    d, HpdfReal    x, HpdfReal    y);

/*--- Path construction operator ------------------------------------------*/

/* m */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_MoveTo(                               HPDF_Page    page, HpdfReal    x, HpdfReal    y);

/* l */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_LineTo(                               HPDF_Page    page, HpdfReal    x, HpdfReal    y);

/* c */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_CurveTo(                              HPDF_Page    page, HpdfReal    x1, HpdfReal    y1, HpdfReal    x2, HpdfReal    y2, HpdfReal    x3, HpdfReal    y3);

/* v */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_CurveTo2(                             HPDF_Page    page, HpdfReal    x2, HpdfReal    y2, HpdfReal    x3, HpdfReal    y3);

/* y */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_CurveTo3(                             HPDF_Page  page, HpdfReal  x1, HpdfReal  y1, HpdfReal  x3, HpdfReal  y3);

/* h */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_ClosePath(                            HPDF_Page  page);

/* re */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_Rectangle(                            HPDF_Page  page, HpdfReal  x, HpdfReal  y, HpdfReal  width, HpdfReal  height);

/*--- Path painting operator ---------------------------------------------*/

/* S */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_Stroke(                               HPDF_Page  page);

/* s */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_ClosePathStroke(                      HPDF_Page  page);

/* f */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_Fill(                                 HPDF_Page  page);

/* f* */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_Eofill(                               HPDF_Page  page);

/* B */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_FillStroke(                           HPDF_Page  page);

/* B* */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_EofillStroke(                         HPDF_Page  page);

/* b */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_ClosePathFillStroke(                  HPDF_Page  page);

/* b* */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_ClosePathEofillStroke(                HPDF_Page  page);

/* n */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_EndPath(                              HPDF_Page  page);

/*--- Clipping paths operator --------------------------------------------*/

/* W */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_Clip(                                 HPDF_Page  page);

/* W* */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_Eoclip(                               HPDF_Page  page);

/*--- Text object operator -----------------------------------------------*/

/* BT */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_BeginText(                            HPDF_Page  page);

/* ET */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_EndText(                              HPDF_Page  page);

/*--- Text state ---------------------------------------------------------*/

/* Tc */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_SetCharSpace(                         HPDF_Page  page, HpdfReal  value);

/* Tw */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_SetWordSpace(                         HPDF_Page  page, HpdfReal  value);

/* Tz */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_SetHorizontalScalling(                HPDF_Page  page, HpdfReal  value);

/* TL */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_SetTextLeading(                       HPDF_Page  page, HpdfReal  value);

/* Tf */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_SetFontAndSize(                       HPDF_Page  page, HPDF_Font  font, HpdfReal  size);

/* Tr */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_SetTextRenderingMode(                 HPDF_Page               page, HPDF_TextRenderingMode  mode);

/* Ts */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_SetTextRise(                          HPDF_Page   page, HpdfReal   value);

/* This function is obsolete. Use HPDF_Page_SetTextRise.  */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_SetTextRaise(                         HPDF_Page   page, HpdfReal   value);

/*--- Text positioning ---------------------------------------------------*/

/* Td */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_MoveTextPos(                          HPDF_Page   page, HpdfReal   x, HpdfReal   y);

/* TD */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_MoveTextPos2(                         HPDF_Page  page, HpdfReal   x, HpdfReal   y);

/* Tm */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_SetTextMatrix(                        HPDF_Page         page, HpdfReal    a, HpdfReal    b, HpdfReal    c, HpdfReal    d, HpdfReal    x, HpdfReal    y);

/* T* */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_MoveToNextLine(                       HPDF_Page  page);

/*--- Text showing -------------------------------------------------------*/

/* Tj */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_ShowText(                             HPDF_Page    page, char const *text);

/* TJ */

/* ' */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_ShowTextNextLine(                     HPDF_Page    page, char const *text);

/* " */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_ShowTextNextLineEx(                   HPDF_Page    page, HpdfReal    word_space, HpdfReal    char_space, char const *text);

/*--- Color showing ------------------------------------------------------*/

/* cs --not implemented yet */
/* CS --not implemented yet */
/* sc --not implemented yet */
/* scn --not implemented yet */
/* SC --not implemented yet */
/* SCN --not implemented yet */

/* g */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_SetGrayFill(                          HPDF_Page   page, HpdfReal   gray);

/* G */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_SetGrayStroke(                        HPDF_Page   page, HpdfReal   gray);

/* rg */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_SetRGBFill(                           HPDF_Page  page, HpdfReal  r, HpdfReal  g, HpdfReal  b);

/* RG */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_SetRGBStroke(                         HPDF_Page  page, HpdfReal  r, HpdfReal  g, HpdfReal  b);

/* k */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_SetCMYKFill(                          HPDF_Page  page, HpdfReal  c, HpdfReal  m, HpdfReal  y, HpdfReal  k);

/* K */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_SetCMYKStroke(                        HPDF_Page  page, HpdfReal  c, HpdfReal  m, HpdfReal  y, HpdfReal  k);

/*--- Shading patterns ---------------------------------------------------*/

/* Notes for docs:
 * - ShadingType must be HPDF_SHADING_FREE_FORM_TRIANGLE_MESH(the only
 *   defined option...)
 * - colorSpace must be HPDF_CS_DEVICE_RGB for now.
 */
HPDF_EXPORT(HPDF_Shading)           HPDF_Shading_New(                               HpdfDoc       * const doc, HPDF_ShadingType type, HPDF_ColorSpace  colorSpace, HpdfReal xMin, HpdfReal xMax, HpdfReal yMin, HpdfReal yMax);
HPDF_EXPORT(HpdfStatus)            HPDF_Shading_AddVertexRGB(                      HPDF_Shading shading, HPDF_Shading_FreeFormTriangleMeshEdgeFlag edgeFlag, HpdfReal x, HpdfReal y, HpdfUInt8 r, HpdfUInt8 g, HpdfUInt8 b);

/*--- In-line images -----------------------------------------------------*/

/* BI --not implemented yet */
/* ID --not implemented yet */
/* EI --not implemented yet */

/*--- XObjects -----------------------------------------------------------*/

/* Do */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_ExecuteXObject(                       HPDF_Page     page, HPDF_XObject  obj);

/*--- Content streams ----------------------------------------------------*/

HPDF_EXPORT(HpdfStatus)            HPDF_Page_New_Content_Stream(                   HPDF_Page page, HPDF_Dict* new_stream);
HPDF_EXPORT(HpdfStatus)            HPDF_Page_Insert_Shared_Content_Stream(         HPDF_Page page, HPDF_Dict shared_stream);

/*--- Marked content -----------------------------------------------------*/

/* BMC --not implemented yet */
/* BDC --not implemented yet */
/* EMC --not implemented yet */
/* MP --not implemented yet */
/* DP --not implemented yet */

/*--- Compatibility ------------------------------------------------------*/

/* BX --not implemented yet */
/* EX --not implemented yet */
HPDF_EXPORT(HpdfStatus)            HPDF_Page_DrawImage(                            HPDF_Page    page, HPDF_Image   image, HpdfReal    x, HpdfReal    y, HpdfReal    width, HpdfReal    height);
HPDF_EXPORT(HpdfStatus)            HPDF_Page_Circle(                               HPDF_Page     page, HpdfReal     x, HpdfReal     y, HpdfReal     ray);
HPDF_EXPORT(HpdfStatus)            HPDF_Page_Ellipse(                              HPDF_Page   page, HpdfReal   x, HpdfReal   y, HpdfReal  xray, HpdfReal  yray);
HPDF_EXPORT(HpdfStatus)            HPDF_Page_Arc(                                  HPDF_Page    page, HpdfReal    x, HpdfReal    y, HpdfReal    ray, HpdfReal    ang1, HpdfReal    ang2);
HPDF_EXPORT(HpdfStatus)            HPDF_Page_TextOut(                              HPDF_Page    page, HpdfReal    xpos, HpdfReal    ypos, char const *text);
HPDF_EXPORT(HpdfStatus)            HPDF_Page_TextRect(                             HPDF_Page            page, HpdfReal            left, HpdfReal            top, HpdfReal            right, HpdfReal            bottom, char const         *text, HPDF_TextAlignment   align, HpdfUInt           *len);
HPDF_EXPORT(HpdfStatus)            HPDF_Page_SetSlideShow(                         HPDF_Page              page, HPDF_TransitionStyle   type, HpdfReal              disp_time, HpdfReal              trans_time);
HPDF_EXPORT(HPDF_OutputIntent)      HPDF_ICC_LoadIccFromMem(                        HpdfDoc       * const doc, HpdfMemMgr * const mmgr, HPDF_Stream iccdata, HPDF_Xref   xref, int         numcomponent);
HPDF_EXPORT(HPDF_OutputIntent)      HPDF_LoadIccProfileFromFile(                    HpdfDoc       * const doc, char const* icc_file_name, int  numcomponent);

#if defined(WIN32)
HPDF_EXPORT(HPDF_OutputIntent)      HPDF_LoadIccProfileFromFileW(                   HpdfDoc       * const doc, wchar_t const *icc_file_name, int  numcomponent);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _HPDF_H */
