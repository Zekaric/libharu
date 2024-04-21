/*
 * << Haru Free PDF Library >> -- hpdf_doc_png.c
 *
 * URL: http://libharu.org
 *
 * Copyright(c) 1999-2006 Takeshi Kanno <takeshi_kanno@est.hi-ho.ne.jp>
 * Copyright(c) 2007-2009 Antony Dovgal <tony@daylessday.org>
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
#include "hpdf_image.h"


#ifdef LIBHPDF_HAVE_LIBPNG
static HPDF_Image    LoadPngImageFromStream(HpdfDoc * const doc, HPDF_Stream imagedata, HpdfBool delayed_loading);

HPDF_EXPORT(HPDF_Image)
   HPDF_LoadPngImageFromMem(
      HpdfDoc const * const doc,
      HpdfByte    const *const buffer,
      HpdfUInt     size)
{
   HPDF_Stream imagedata;
   HPDF_Image image;

   HPDF_PTRACE((" HPDF_LoadPngImageFromMem\n"));

   if (!HPDF_HasDoc(doc)) 
   {
      return NULL;
   }

   /* create file stream */
   imagedata = HPDF_MemStream_New(doc->mmgr, size);

   if (!HPDF_Stream_Validate(imagedata)) 
   {
      HPDF_RaiseError(&doc->error, HPDF_INVALID_STREAM, 0);
      return NULL;
   }

   if (HPDF_Stream_Write(imagedata, buffer, size) != HPDF_OK) 
   {
      HPDF_Stream_Free(imagedata);
      return NULL;
   }

   image = LoadPngImageFromStream(doc, imagedata, HPDF_FALSE);

   /* destroy file stream */
   HPDF_Stream_Free(imagedata);

   if (!image) 
   {
      HPDF_CheckError(&doc->error);
   }

   return image;
}

HPDF_EXPORT(HPDF_Image)
   HPDF_LoadPngImageFromFile(
      HpdfDoc const * const doc,
      char const *filename)
{
   HPDF_Stream imagedata;
   HPDF_Image image;

   HPDF_PTRACE((" HPDF_LoadPngImageFromFile\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   /* create file stream */
   imagedata = HPDF_FileReader_New(doc->mmgr, filename);

   if (HPDF_Stream_Validate(imagedata))
   {
      image = LoadPngImageFromStream(doc, imagedata, HPDF_FALSE);
   }
   else
   {
      image = NULL;
   }

   /* destroy file stream */
   if (imagedata)
   {
      HPDF_Stream_Free(imagedata);
   }

   if (!image)
   {
      HPDF_CheckError(&doc->error);
   }

   return image;
}

/* delaied loading version of HPDF_LoadPngImageFromFile */
HPDF_EXPORT(HPDF_Image)
   HPDF_LoadPngImageFromFile2(
      HpdfDoc const * const doc,
      char const *filename)
{
   HPDF_Stream imagedata;
   HPDF_Image image;
   HPDF_String fname;

   HPDF_PTRACE((" HPDF_LoadPngImageFromFile2\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   /* check whether file name is valid or not. */
   imagedata = HPDF_FileReader_New(doc->mmgr, filename);

   if (HPDF_Stream_Validate(imagedata))
   {
      image = LoadPngImageFromStream(doc, imagedata, HPDF_TRUE);
   }
   else
   {
      image = NULL;
   }

   /* destroy file stream */
   if (imagedata)
   {
      HPDF_Stream_Free(imagedata);
   }

   if (!image) 
   {
      HPDF_CheckError(&doc->error);
      return NULL;
   }

   /* add file-name to image dictionary as a hidden entry.
   ** it is used when the image data is needed. */
   fname = HPDF_String_New(doc->mmgr, filename, NULL);
   if (!fname) 
   {
      HPDF_CheckError(&doc->error);
      return NULL;
   }

   fname->header.obj_id |= HPDF_OTYPE_HIDDEN;

   if ((HPDF_Dict_Add(image, "_FILE_NAME", fname)) != HPDF_OK) 
   {
      HPDF_CheckError(&doc->error);
      return NULL;
   }

   return image;
}

#if defined(WIN32)
HPDF_EXPORT(HPDF_Image)
   HPDF_LoadPngImageFromFileW(
      HpdfDoc const * const doc,
      const wchar_t *filename)
{
   HPDF_Stream imagedata;
   HPDF_Image image;

   HPDF_PTRACE((" HPDF_LoadPngImageFromFile\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   /* create file stream */
   imagedata = HPDF_FileReader_NewW(doc->mmgr, filename);

   if (HPDF_Stream_Validate(imagedata))
   {
      image = LoadPngImageFromStream(doc, imagedata, HPDF_FALSE);
   }
   else
   {
      image = NULL;
   }

   /* destroy file stream */
   if (imagedata)
   {
      HPDF_Stream_Free(imagedata);
   }

   if (!image)
   {
      HPDF_CheckError(&doc->error);
   }

   return image;
}

/* delaied loading version of HPDF_LoadPngImageFromFile */
HPDF_EXPORT(HPDF_Image)
   HPDF_LoadPngImageFromFile2W(
      HpdfDoc const * const doc,
      const wchar_t *filename)
{
   HPDF_Stream imagedata;
   HPDF_Image image;
   HPDF_String fname;

   HPDF_PTRACE((" HPDF_LoadPngImageFromFile\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   /* check whether file name is valid or not. */
   imagedata = HPDF_FileReader_NewW(doc->mmgr, filename);

   if (HPDF_Stream_Validate(imagedata))
   {
      image = LoadPngImageFromStream(doc, imagedata, HPDF_TRUE);
   }
   else
   {
      image = NULL;
   }

   /* destroy file stream */
   if (imagedata)
   {
      HPDF_Stream_Free(imagedata);
   }

   if (!image) 
   {
      HPDF_CheckError(&doc->error);
      return NULL;
   }

   /* add file-name to image dictionary as a hidden entry.
   ** it is used when the image data is needed. */
   fname = HPDF_String_New(doc->mmgr, filename, NULL);
   if (!fname) 
   {
      HPDF_CheckError(&doc->error);
      return NULL;
   }

   fname->header.obj_id |= HPDF_OTYPE_HIDDEN;

   if ((HPDF_Dict_Add(image, "_FILE_NAME", fname)) != HPDF_OK) 
   {
      HPDF_CheckError(&doc->error);
      return NULL;
   }

   return image;
}
#endif

static HPDF_Image
   LoadPngImageFromStream(
      HpdfDoc const * const doc,
      HPDF_Stream   imagedata,
      HpdfBool     delayed_loading)
{
   HPDF_Image image;
   HPDF_Dict smask;

   HPDF_PTRACE((" HPDF_LoadPngImageFromStream\n"));

   image = HPDF_Image_LoadPngImage(doc->mmgr, imagedata, doc->xref, delayed_loading);

   if (image &&(doc->compression_mode & HPDF_COMP_IMAGE)) 
   {
      image->filter = HPDF_STREAM_FILTER_FLATE_DECODE;

      // is there an alpha layer? then compress it also
      smask = HPDF_Dict_GetItem(image, "SMask", HPDF_OCLASS_DICT);
      if (smask) smask->filter = HPDF_STREAM_FILTER_FLATE_DECODE;
   }

   return image;
}

#else
static HPDF_Image
   LoadPngImageFromStream(
      HpdfDoc * const doc,
      HPDF_Stream   imagedata,
      HpdfBool     delayed_loading)
{
   HPDF_SetError(&doc->error, HPDF_UNSUPPORTED_FUNC, 0);
   HPDF_UNUSED(delayed_loading);
   HPDF_UNUSED(imagedata);

   return NULL;
}

#endif /* LIBHPDF_HAVE_PNGLIB */
