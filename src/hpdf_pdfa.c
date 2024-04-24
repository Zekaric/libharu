/*
 * << Haru Free PDF Library >> -- hpdf_pdfa.c
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
 /* This is used to avoid warnings on 'ctime' when compiling in MSVC 9 */
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <time.h>
#include "hpdf_utils.h"
#include "hpdf.h"
#include <string.h>


#define HEADER                   "<?xpacket begin='' id='W5M0MpCehiHzreSzNTczkc9d'?><x:xmpmeta xmlns:x='adobe:ns:meta/' x:xmptk='XMP toolkit 2.9.1-13, framework 1.6'><rdf:RDF xmlns:rdf='http://www.w3.org/1999/02/22-rdf-syntax-ns#' xmlns:iX='http://ns.adobe.com/iX/1.0/'>"
#define DC_HEADER                "<rdf:Description xmlns:dc='http://purl.org/dc/elements/1.1/' rdf:about=''>"
#define DC_TITLE_STARTTAG        "<dc:title><rdf:Alt><rdf:li xml:lang=\"x-default\">"
#define DC_TITLE_ENDTAG          "</rdf:li></rdf:Alt></dc:title>"
#define DC_CREATOR_STARTTAG      "<dc:creator><rdf:Seq><rdf:li>"
#define DC_CREATOR_ENDTAG        "</rdf:li></rdf:Seq></dc:creator>"
#define DC_DESCRIPTION_STARTTAG  "<dc:description><rdf:Alt><rdf:li xml:lang=\"x-default\">"
#define DC_DESCRIPTION_ENDTAG    "</rdf:li></rdf:Alt></dc:description>"
#define DC_FOOTER                "</rdf:Description>"
#define XMP_HEADER               "<rdf:Description xmlns:xmp='http://ns.adobe.com/xap/1.0/' rdf:about=''>"
#define XMP_CREATORTOOL_STARTTAG "<xmp:CreatorTool>"
#define XMP_CREATORTOOL_ENDTAG   "</xmp:CreatorTool>"
#define XMP_CREATE_DATE_STARTTAG "<xmp:CreateDate>"
#define XMP_CREATE_DATE_ENDTAG   "</xmp:CreateDate>"
#define XMP_MOD_DATE_STARTTAG    "<xmp:ModifyDate>"
#define XMP_MOD_DATE_ENDTAG      "</xmp:ModifyDate>"
#define XMP_FOOTER               "</rdf:Description>"
#define PDF_HEADER               "<rdf:Description xmlns:doc='http://ns.adobe.com/doc/1.3/' rdf:about=''>"
#define PDF_KEYWORDS_STARTTAG    "<doc:Keywords>"
#define PDF_KEYWORDS_ENDTAG      "</doc:Keywords>"
#define PDF_PRODUCER_STARTTAG    "<doc:Producer>"
#define PDF_PRODUCER_ENDTAG      "</doc:Producer>"
#define PDF_FOOTER               "</rdf:Description>"
#define PDFAID_PDFA1A            "<rdf:Description rdf:about='' xmlns:pdfaid='http://www.aiim.org/pdfa/ns/id/' pdfaid:part='1' pdfaid:conformance='A'/>"
#define PDFAID_PDFA1B            "<rdf:Description rdf:about='' xmlns:pdfaid='http://www.aiim.org/pdfa/ns/id/' pdfaid:part='1' pdfaid:conformance='B'/>"
#define FOOTER                   "</rdf:RDF></x:xmpmeta><?xpacket end='w'?>"


/*
 * Convert date in PDF specific format: D:YYYYMMDDHHmmSS
 * to XMP value in format YYYY-MM-DDTHH:mm:SS+offH:offMin
 */
HpdfStatus ConvertDateToXMDate(HPDF_Stream stream, char const *pDate)
{
   HpdfStatus ret;

   if (pDate==NULL) return HPDF_INVALID_PARAMETER;
   if (strlen(pDate)<16) return HPDF_INVALID_PARAMETER;
   if (pDate[0]!='D'||
      pDate[1]!=':') return HPDF_INVALID_PARAMETER;
   pDate+=2;
   /* Copy YYYY */
   ret = HPDF_Stream_Write(stream, (HpdfByte const *) pDate, 4);
   if (ret != HPDF_OK)
      return ret;
   pDate+=4;
   /* Write -MM */
   ret = HPDF_Stream_Write(stream, (HpdfByte const *) "-", 1);
   if (ret != HPDF_OK)
      return ret;
   ret = HPDF_Stream_Write(stream, (HpdfByte const *) pDate, 2);
   if (ret != HPDF_OK)
      return ret;
   pDate+=2;
   /* Write -DD */
   ret = HPDF_Stream_Write(stream, (HpdfByte const *) "-", 1);
   if (ret != HPDF_OK)
      return ret;
   ret = HPDF_Stream_Write(stream, (HpdfByte const *) pDate, 2);
   if (ret != HPDF_OK)
      return ret;
   pDate+=2;
   /* Write THH */
   ret = HPDF_Stream_Write(stream, (HpdfByte const *) "T", 1);
   if (ret != HPDF_OK)
      return ret;
   ret = HPDF_Stream_Write(stream, (HpdfByte const *) pDate, 2);
   if (ret != HPDF_OK)
      return ret;
   pDate+=2;
   /* Write :mm */
   ret = HPDF_Stream_Write(stream, (HpdfByte const *) ":", 1);
   if (ret != HPDF_OK)
      return ret;
   ret = HPDF_Stream_Write(stream, (HpdfByte const *) pDate, 2);
   if (ret != HPDF_OK)
      return ret;
   pDate+=2;
   /* Write :SS */
   ret = HPDF_Stream_Write(stream, (HpdfByte const *) ":", 1);
   if (ret != HPDF_OK)
      return ret;
   ret = HPDF_Stream_Write(stream, (HpdfByte const *) pDate, 2);
   if (ret != HPDF_OK)
      return ret;
   pDate+=2;
   /* Write +... */
   if (pDate[0]==0 || pDate[0]=='Z') {
      ret = HPDF_Stream_Write(stream, (HpdfByte const *) "Z", 1);
      return ret;
   }
   if (pDate[0]=='+'||pDate[0]=='-') {
      ret = HPDF_Stream_Write(stream, (HpdfByte const *) pDate, 3);
      if (ret != HPDF_OK)
         return ret;
      pDate+=4;
      ret = HPDF_Stream_Write(stream, (HpdfByte const *) ":", 1);
      if (ret != HPDF_OK)
         return ret;
      ret = HPDF_Stream_Write(stream, (HpdfByte const *) pDate, 2);
      if (ret != HPDF_OK)
         return ret;
      return ret;
   }
   return HPDF_SetError(stream->error, HPDF_INVALID_PARAMETER, 0);
}

/* Write XMP Metadata for PDF/A */

HpdfStatus
HPDF_PDFA_SetPDFAConformance(
   HpdfDoc * const doc,
   HPDF_PDFAType pdfatype)
{
   HPDF_OutputIntent xmp;
   HpdfStatus ret;

   char const *dc_title       = NULL;
   char const *dc_creator     = NULL;
   char const *dc_description = NULL;

   char const *xmp_CreatorTool = NULL;
   char const *xmp_CreateDate  = NULL;
   char const *xmp_ModifyDate  = NULL;

   char const *pdf_Keywords    = NULL;
   char const *pdf_Producer    = NULL;

   if (!HPDF_HasDoc(doc))
   {
      return HPDF_INVALID_DOCUMENT;
   }

   dc_title       = (char const *) HPDF_GetInfoAttr(doc, HPDF_INFO_TITLE);
   dc_creator     = (char const *) HPDF_GetInfoAttr(doc, HPDF_INFO_AUTHOR);
   dc_description = (char const *) HPDF_GetInfoAttr(doc, HPDF_INFO_SUBJECT);

   xmp_CreateDate  = (char const *) HPDF_GetInfoAttr(doc, HPDF_INFO_CREATION_DATE);
   xmp_ModifyDate  = (char const *) HPDF_GetInfoAttr(doc, HPDF_INFO_MOD_DATE);
   xmp_CreatorTool = (char const *) HPDF_GetInfoAttr(doc, HPDF_INFO_CREATOR);

   pdf_Keywords = (char const *) HPDF_GetInfoAttr(doc, HPDF_INFO_KEYWORDS);
   pdf_Producer = (char const *) HPDF_GetInfoAttr(doc, HPDF_INFO_PRODUCER);

   if ((dc_title        != NULL) ||
      (dc_creator      != NULL) ||
      (dc_description  != NULL) ||
      (xmp_CreateDate  != NULL) ||
      (xmp_ModifyDate  != NULL) ||
      (xmp_CreatorTool != NULL) ||
      (pdf_Keywords    != NULL))
   {
      xmp = HPDF_DictStream_New(doc->mmgr, doc->xref);
      if (!xmp)
      {
         return HPDF_INVALID_STREAM;
      }

      /* Update the PDF number version */
      doc->pdf_version = HPDF_VER_14;

      HPDF_Dict_AddName(xmp, "Type", "Metadata");
      HPDF_Dict_AddName(xmp, "Subtype", "XML");

      ret = HPDF_OK;
      ret += HPDF_Stream_WriteStr(xmp->stream, HEADER);

      /* Add the dc block */
      if ((dc_title       != NULL) ||
         (dc_creator     != NULL) ||
         (dc_description != NULL))
      {
         ret += HPDF_Stream_WriteStr(xmp->stream, DC_HEADER);

         if (dc_title != NULL)
         {
            ret += HPDF_Stream_WriteStr(xmp->stream, DC_TITLE_STARTTAG);
            ret += HPDF_Stream_WriteStr(xmp->stream, dc_title);
            ret += HPDF_Stream_WriteStr(xmp->stream, DC_TITLE_ENDTAG);
         }

         if (dc_creator != NULL)
         {
            ret += HPDF_Stream_WriteStr(xmp->stream, DC_CREATOR_STARTTAG);
            ret += HPDF_Stream_WriteStr(xmp->stream, dc_creator);
            ret += HPDF_Stream_WriteStr(xmp->stream, DC_CREATOR_ENDTAG);
         }

         if (dc_description != NULL)
         {
            ret += HPDF_Stream_WriteStr(xmp->stream, DC_DESCRIPTION_STARTTAG);
            ret += HPDF_Stream_WriteStr(xmp->stream, dc_description);
            ret += HPDF_Stream_WriteStr(xmp->stream, DC_DESCRIPTION_ENDTAG);
         }

         ret += HPDF_Stream_WriteStr(xmp->stream, DC_FOOTER);
      }

      /* Add the xmp block */
      if ((xmp_CreateDate != NULL) ||
         (xmp_ModifyDate != NULL) ||
         (xmp_CreatorTool != NULL))
      {
         ret += HPDF_Stream_WriteStr(xmp->stream, XMP_HEADER);

         /* Add CreateDate, ModifyDate, and CreatorTool */
         if (xmp_CreatorTool != NULL)
         {
            ret += HPDF_Stream_WriteStr(xmp->stream, XMP_CREATORTOOL_STARTTAG);
            ret += HPDF_Stream_WriteStr(xmp->stream, xmp_CreatorTool);
            ret += HPDF_Stream_WriteStr(xmp->stream, XMP_CREATORTOOL_ENDTAG);
         }

         if (xmp_CreateDate != NULL)
         {
            ret += HPDF_Stream_WriteStr(xmp->stream, XMP_CREATE_DATE_STARTTAG);
            /* Convert date to XMP compatible format */
            ret += ConvertDateToXMDate(xmp->stream, xmp_CreateDate);
            ret += HPDF_Stream_WriteStr(xmp->stream, XMP_CREATE_DATE_ENDTAG);
         }

         if (xmp_ModifyDate != NULL)
         {
            ret += HPDF_Stream_WriteStr(xmp->stream, XMP_MOD_DATE_STARTTAG);
            ret += ConvertDateToXMDate(xmp->stream, xmp_ModifyDate);
            ret += HPDF_Stream_WriteStr(xmp->stream, XMP_MOD_DATE_ENDTAG);
         }

         ret += HPDF_Stream_WriteStr(xmp->stream, XMP_FOOTER);
      }

      /* Add the doc block */
      if ((pdf_Keywords != NULL) ||
         (pdf_Producer != NULL))
      {
         ret += HPDF_Stream_WriteStr(xmp->stream, PDF_HEADER);

         if (pdf_Keywords != NULL)
         {
            ret += HPDF_Stream_WriteStr(xmp->stream, PDF_KEYWORDS_STARTTAG);
            ret += HPDF_Stream_WriteStr(xmp->stream, pdf_Keywords);
            ret += HPDF_Stream_WriteStr(xmp->stream, PDF_KEYWORDS_ENDTAG);
         }

         if (pdf_Producer != NULL)
         {
            ret += HPDF_Stream_WriteStr(xmp->stream, PDF_PRODUCER_STARTTAG);
            ret += HPDF_Stream_WriteStr(xmp->stream, pdf_Producer);
            ret += HPDF_Stream_WriteStr(xmp->stream, PDF_PRODUCER_ENDTAG);
         }

         ret += HPDF_Stream_WriteStr(xmp->stream, PDF_FOOTER);
      }

      /* Add the pdfaid block */
      switch (pdfatype)
      {
      case HPDF_PDFA_1A:
         ret += HPDF_Stream_WriteStr(xmp->stream, PDFAID_PDFA1A);
         break;

      case HPDF_PDFA_1B:
         ret += HPDF_Stream_WriteStr(xmp->stream, PDFAID_PDFA1B);
         break;
      }

      ret += HPDF_Stream_WriteStr(xmp->stream, FOOTER);

      if (ret != HPDF_OK)
      {
         return HPDF_INVALID_STREAM;
      }

      if ((ret = HPDF_Dict_Add(doc->catalog, "Metadata", xmp)) != HPDF_OK)
      {
         return ret;
      }

      return HPDF_PDFA_GenerateID(doc);
   }

   return HPDF_OK;
}

/* Generate an ID for the trailer dict, PDF/A needs this.
   TODO: Better algorithm for generate unique ID.
*/
HpdfStatus
HPDF_PDFA_GenerateID(
   HpdfDoc const * const doc)
{
   HpdfArray *id;
   HpdfByte *currentTime;
   HpdfByte idkey[HPDF_MD5_KEY_LEN];
   HPDF_MD5_CTX md5_ctx;
   time_t ltime;

   ltime = time(NULL);
   currentTime = (HpdfByte *) ctime(&ltime);

   id = HPDF_Dict_GetItem(doc->trailer, "ID", HPDF_OCLASS_ARRAY);
   if (!id)
   {
      id = HPDF_Array_New(doc->mmgr);

      if (!id || HPDF_Dict_Add(doc->trailer, "ID", id) != HPDF_OK)
      {
         return doc->error.error_no;
      }

      HPDF_MD5Init(&md5_ctx);
      HPDF_MD5Update(&md5_ctx, (HpdfByte *) "libHaru", sizeof("libHaru") - 1);
      HPDF_MD5Update(&md5_ctx, currentTime, HPDF_StrLen((char const *) currentTime, -1));
      HPDF_MD5Final(idkey, &md5_ctx);

      if (HPDF_Array_Add(id, HpdfObjBinaryCreate(doc->mmgr, idkey, HPDF_MD5_KEY_LEN)) != HPDF_OK)
      {
         return doc->error.error_no;
      }

      if (HPDF_Array_Add(id, HpdfObjBinaryCreate(doc->mmgr, idkey, HPDF_MD5_KEY_LEN)) != HPDF_OK)
      {
         return doc->error.error_no;
      }

      return HPDF_OK;
   }

   return HPDF_OK;
}

/* Function to add one outputintents to the PDF
 * iccname - name of default ICC profile
 * iccdict - dictionary containing number of components
 *           and stream with ICC profile
 *
 * How to use:
 * 1. Create dictionary with ICC profile
 *    HPDF_Dict icc = HPDF_DictStream_New (pDoc->mmgr, pDoc->xref);
 *    if(icc==NULL) return false;
 *    HPDF_Dict_AddNumber (icc, "N", 3);
 *    HpdfStatus ret = HPDF_Stream_Write (icc->stream, (const HpdfByte *)pICCData, dwICCSize);
 *    if(ret!=HPDF_OK) {
 *      HPDF_Dict_Free(icc);
 *      return false;
 *    }
 *
 * 2. Call this function
 */

HpdfStatus
HPDF_PDFA_AppendOutputIntents(
   HpdfDoc * const doc,
   char const *iccname,
   HPDF_Dict iccdict)
{
   HpdfArray *intents;
   HPDF_Dict intent;
   HpdfStatus ret;
   if (!HPDF_HasDoc(doc))
   {
      return HPDF_INVALID_DOCUMENT;
   }

   /* prepare intent */
   intent = HPDF_Dict_New(doc->mmgr);
   ret = HPDF_Xref_Add(doc->xref, intent);
   if (ret != HPDF_OK)
   {
      HPDF_Dict_Free(intent);
      return ret;
   }
   ret += HPDF_Dict_AddName(intent, "Type",               "OutputIntent");
   ret += HPDF_Dict_AddName(intent, "S",                  "GTS_PDFA1");
   ret += HPDF_Dict_Add(
      intent, 
      "OutputConditionIdentifier", 
      HpdfObjStringCreate(doc->mmgr, iccname, NULL));
   ret += HPDF_Dict_Add(
      intent, 
      "OutputCondition", 
      HpdfObjStringCreate(doc->mmgr, iccname, NULL));
   ret += HPDF_Dict_Add(
      intent, 
      "Info",
      HpdfObjStringCreate(doc->mmgr, iccname, NULL));
   ret += HPDF_Dict_Add(    intent, "DestOutputProfile ", iccdict);
   if (ret != HPDF_OK)
   {
      HPDF_Dict_Free(intent);
      return ret;
   }

   /* Copied from HPDF_AddIntent - not public function */
   intents = HPDF_Dict_GetItem(doc->catalog, "OutputIntents", HPDF_OCLASS_ARRAY);
   if (intents == NULL)
   {
      intents = HPDF_Array_New(doc->mmgr);
      if (intents)
      {
         ret = HPDF_Dict_Add(doc->catalog, "OutputIntents", intents);
         if (ret != HPDF_OK)
         {
            HPDF_CheckError(&doc->error);
            return HPDF_Error_GetDetailCode(&doc->error);
         }
      }
   }

   HPDF_Array_Add(intents, intent);
   return HPDF_Error_GetDetailCode(&doc->error);
}
