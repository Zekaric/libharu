/*
 * << Haru Free PDF Library >> -- hpdf_doc.c
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
#include "hpdf_config.h"
#include "hpdf_utils.h"
#include "hpdf_encryptdict.h"
#include "hpdf_namedict.h"
#include "hpdf_destination.h"
#include "hpdf_info.h"
#include "hpdf_page_label.h"
#include "hpdf_version.h"
#include "hpdf.h"


static char const * const HPDF_VERSION_STR[6] = {
                "%PDF-1.2\012%\267\276\255\252\012",
                "%PDF-1.3\012%\267\276\255\252\012",
                "%PDF-1.4\012%\267\276\255\252\012",
                "%PDF-1.5\012%\267\276\255\252\012",
                "%PDF-1.6\012%\267\276\255\252\012",
                "%PDF-1.7\012%\267\276\255\252\012"
};


static HpdfStatus    _WriteHeader(              HpdfDoc const * const doc, HPDF_Stream stream);
static HpdfStatus    _PrepareTrailer(           HpdfDoc const * const doc);
static void           _FreeEncoderList(          HpdfDoc       * const doc);
static void           _FreeFontDefList(          HpdfDoc       * const doc);
static void           _CleanupFontDefList(       HpdfDoc const * const doc);
static HPDF_Dict      _GetInfo(                  HpdfDoc       * const doc);
static HpdfStatus    _InternalSaveToStream(     HpdfDoc       * const doc, HPDF_Stream stream);
static char const   *_LoadType1FontFromStream(  HpdfDoc       * const doc, HPDF_Stream afmdata, HPDF_Stream pfmdata);
static char const   *_LoadTTFontFromStream(     HpdfDoc       * const doc, HPDF_Stream font_data, HpdfBool embedding);
static char const   *_LoadTTFontFromStream2(    HpdfDoc       * const doc, HPDF_Stream font_data, HpdfUInt index, HpdfBool embedding);


/******************************************************************************
func: HPDF_GetVersion
******************************************************************************/
HPDF_EXPORT(char const *)
   HPDF_GetVersion(
      void)
{
   return HPDF_VERSION_TEXT;
}

HpdfBool
   HPDF_Doc_Validate(
      HpdfDoc const * const doc)
{
   HPDF_PTRACE((" HPDF_Doc_Validate\n"));

   if (!doc || doc->sig_bytes != HPDF_SIG_BYTES)
   {
      return HPDF_FALSE;
   }
   else
   {
      return HPDF_TRUE;
   }
}

HPDF_EXPORT(HpdfBool)
   HPDF_HasDoc(
      HpdfDoc * const doc)
{
   HPDF_PTRACE((" HPDF_HasDoc\n"));

   if (!doc ||
      doc->sig_bytes != HPDF_SIG_BYTES)
   {
      return HPDF_FALSE;
   }

   if (!doc->catalog || 
       doc->error.error_no != HPDF_NOERROR) 
   {
      HPDF_RaiseError(&doc->error, HPDF_INVALID_DOCUMENT, 0);
      return HPDF_FALSE;
   }
   else
   {
      return HPDF_TRUE;
   }
}

HPDF_EXPORT(HpdfMemMgr *)
   HPDF_GetDocMMgr(
      HpdfDoc const * const doc)
{
   HPDF_PTRACE((" HPDF_GetDocMMgr\n"));

   return doc->mmgr;
}

HPDF_EXPORT(HpdfDoc *)
   HPDF_New(
      HPDF_Error_Handler user_error_fn,
      void              *user_data)
{
   HPDF_PTRACE((" HPDF_New\n"));

   return HPDF_NewEx(user_error_fn, NULL, NULL, 0, user_data);
}

HPDF_EXPORT(HpdfDoc *)
   HPDF_NewEx(
      HPDF_Error_Handler    user_error_fn,
      HPDF_Alloc_Func       user_alloc_fn,
      HPDF_Free_Func        user_free_fn,
      HpdfUInt             mem_pool_buf_size,
      void                 *user_data)
{
   HpdfDoc     *doc;
   HpdfMemMgr  *mmgr;
   HpdfError    tmp_error;

   HPDF_PTRACE((" HPDF_NewEx\n"));

   /* initialize temporary-error object */
   HPDF_Error_Init(&tmp_error, user_data);

   /* create memory-manager object */
   mmgr = HpdfMemMgrCreate(&tmp_error, mem_pool_buf_size, user_alloc_fn, user_free_fn);
   if (!mmgr) 
   {
      HPDF_CheckError(&tmp_error);
      return NULL;
   }

   /* now create pdf_doc object */
   doc = HpdfMemCreateType(mmgr, HpdfDoc);
   if (!doc) 
   {
      HpdfMemMgrDestroy(mmgr);
      HPDF_CheckError(&tmp_error);
      return NULL;
   }

   HpdfMemClearType(doc, HpdfDoc);
   doc->sig_bytes = HPDF_SIG_BYTES;
   doc->mmgr = mmgr;
   doc->pdf_version = HPDF_VER_13;
   doc->compression_mode = HPDF_COMP_NONE;

   /* copy the data of temporary-error object to the one which is
   ** included in pdf_doc object */
   doc->error = tmp_error;

   /* switch the error-object of memory-manager */
   mmgr->error = &doc->error;

   if (HPDF_NewDoc(doc) != HPDF_OK) 
   {
      HPDF_Free(doc);
      HPDF_CheckError(&tmp_error);
      return NULL;
   }

   doc->error.error_fn = user_error_fn;

   return doc;
}

HPDF_EXPORT(void)
   HPDF_Free(
      HpdfDoc * const doc)
{
   HPDF_PTRACE((" HPDF_Free\n"));

   if (doc)
   {
      HpdfMemMgr *mmgr = doc->mmgr;

      HPDF_FreeDocAll(doc);

      doc->sig_bytes = 0;

      HpdfMemDestroy(mmgr, doc);
      HpdfMemMgrDestroy(mmgr);
   }
}

HPDF_EXPORT(HpdfStatus)
   HPDF_NewDoc(
      HpdfDoc * const doc)
{
   char buf[HPDF_TMP_BUF_SIZ];
   char *ptr = buf;
   char *eptr = buf + HPDF_TMP_BUF_SIZ - 1;
   char const *version;

   HPDF_PTRACE((" HPDF_NewDoc\n"));

   if (!HPDF_Doc_Validate(doc))
   {
      return HPDF_DOC_INVALID_OBJECT;
   }

   HPDF_FreeDoc(doc);

   doc->xref = HPDF_Xref_New(doc->mmgr, 0);
   if (!doc->xref)
   {
      return HPDF_CheckError(&doc->error);
   }

   doc->trailer = doc->xref->trailer;

   doc->font_mgr = HPDF_List_New(doc->mmgr, HPDF_DEF_ITEMS_PER_BLOCK);
   if (!doc->font_mgr)
   {
      return HPDF_CheckError(&doc->error);
   }

   if (!doc->fontdef_list) 
   {
      doc->fontdef_list = HPDF_List_New(doc->mmgr, HPDF_DEF_ITEMS_PER_BLOCK);
      if (!doc->fontdef_list)
      {
         return HPDF_CheckError(&doc->error);
      }
   }

   if (!doc->encoder_list) 
   {
      doc->encoder_list = HPDF_List_New(doc->mmgr, HPDF_DEF_ITEMS_PER_BLOCK);
      if (!doc->encoder_list)
      {
         return HPDF_CheckError(&doc->error);
      }
   }

   doc->catalog = HPDF_Catalog_New(doc->mmgr, doc->xref);
   if (!doc->catalog)
   {
      return HPDF_CheckError(&doc->error);
   }

   doc->root_pages = HPDF_Catalog_GetRoot(doc->catalog);
   if (!doc->root_pages)
   {
      return HPDF_CheckError(&doc->error);
   }

   doc->page_list = HPDF_List_New(doc->mmgr, HPDF_DEF_PAGE_LIST_NUM);
   if (!doc->page_list)
   {
      return HPDF_CheckError(&doc->error);
   }

   doc->cur_pages = doc->root_pages;

   ptr = (char *)HPDF_StrCpy(ptr, (char const *)"Haru Free PDF Library ", eptr);
   version = HPDF_GetVersion();
   HPDF_StrCpy(ptr, version, eptr);

   if (HPDF_SetInfoAttr(doc, HPDF_INFO_PRODUCER, buf) != HPDF_OK)
   {
      return HPDF_CheckError(&doc->error);
   }

   return HPDF_OK;
}

HPDF_EXPORT(void)
   HPDF_FreeDoc(
      HpdfDoc * const doc)
{
   HPDF_PTRACE((" HPDF_FreeDoc\n"));

   if (HPDF_Doc_Validate(doc)) 
   {
      if (doc->xref) 
      {
         HPDF_Xref_Free(doc->xref);
         doc->xref = NULL;
      }

      if (doc->font_mgr) 
      {
         HPDF_List_Free(doc->font_mgr);
         doc->font_mgr = NULL;
      }

      if (doc->fontdef_list)
      {
         _CleanupFontDefList(doc);
      }

      HpdfMemClear(doc->ttfont_tag, 6);

      doc->pdf_version    = HPDF_VER_13;
      doc->outlines       = NULL;
      doc->catalog        = NULL;
      doc->root_pages     = NULL;
      doc->cur_pages      = NULL;
      doc->cur_page       = NULL;
      doc->encrypt_on     = HPDF_FALSE;
      doc->cur_page_num   = 0;
      doc->cur_encoder    = NULL;
      doc->def_encoder    = NULL;
      doc->page_per_pages = 0;

      if (doc->page_list) 
      {
         HPDF_List_Free(doc->page_list);
         doc->page_list = NULL;
      }

      doc->encrypt_dict = NULL;
      doc->info         = NULL;

      HPDF_Error_Reset(&doc->error);

      if (doc->stream) 
      {
         HPDF_Stream_Free(doc->stream);
         doc->stream = NULL;
      }
   }
}

HPDF_EXPORT(void)
   HPDF_FreeDocAll(
      HpdfDoc * const doc)
{
   HPDF_PTRACE((" HPDF_FreeDocAll\n"));

   if (HPDF_Doc_Validate(doc)) 
   {
      HPDF_FreeDoc(doc);

      if (doc->fontdef_list)
      {
         _FreeFontDefList(doc);
      }

      if (doc->encoder_list)
      {
         _FreeEncoderList(doc);
      }

      doc->compression_mode = HPDF_COMP_NONE;

      HPDF_Error_Reset(&doc->error);
   }
}

HPDF_EXPORT(HpdfStatus)
   HPDF_SetPagesConfiguration(
      HpdfDoc * const doc,
      HpdfUInt   page_per_pages)
{
   HPDF_PTRACE((" HPDF_SetPagesConfiguration\n"));

   if (!HPDF_HasDoc(doc))
   {
      return HPDF_INVALID_DOCUMENT;
   }

   if (doc->cur_page)
   {
      return HPDF_RaiseError(&doc->error, HPDF_INVALID_DOCUMENT_STATE, 0);
   }

   if (page_per_pages > HPDF_LIMIT_MAX_ARRAY)
   {
      return HPDF_RaiseError(&doc->error, HPDF_INVALID_PARAMETER, 0);
   }

   if (doc->cur_pages == doc->root_pages) 
   {
      doc->cur_pages = HPDF_Doc_AddPagesTo(doc, doc->root_pages);
      if (!doc->cur_pages)
      {
         return doc->error.error_no;
      }
      doc->cur_page_num = 0;
   }

   doc->page_per_pages = page_per_pages;

   return HPDF_OK;
}

static HpdfStatus
   _WriteHeader(
      HpdfDoc const * const doc,
      HPDF_Stream   stream)
{
   HpdfUInt idx = (HpdfInt)doc->pdf_version;

   HPDF_PTRACE((" _WriteHeader\n"));

   if (HPDF_Stream_WriteStr(stream, HPDF_VERSION_STR[idx]) != HPDF_OK)
   {
      return doc->error.error_no;
   }

   return HPDF_OK;
}

static HpdfStatus
   _PrepareTrailer(
      HpdfDoc const * const doc)
{
   HPDF_PTRACE((" _PrepareTrailer\n"));

   if (HPDF_Dict_Add(doc->trailer, "Root", doc->catalog) != HPDF_OK)
   {
      return doc->error.error_no;
   }

   if (HPDF_Dict_Add(doc->trailer, "Info", doc->info) != HPDF_OK)
   {
      return doc->error.error_no;
   }

   return HPDF_OK;
}

HpdfStatus
   HPDF_Doc_SetEncryptOn(
      HpdfDoc * const doc)
{
   HPDF_PTRACE((" HPDF_Doc_SetEncryptOn\n"));

   if (doc->encrypt_on)
   {
      return HPDF_OK;
   }

   if (!doc->encrypt_dict)
   {
      return HPDF_SetError(&doc->error, HPDF_DOC_ENCRYPTDICT_NOT_FOUND, 0);
   }

   if (doc->encrypt_dict->header.obj_id == HPDF_OTYPE_NONE)
   {
      if (HPDF_Xref_Add(doc->xref, doc->encrypt_dict) != HPDF_OK)
      {
         return doc->error.error_no;
      }
   }

   if (HPDF_Dict_Add(doc->trailer, "Encrypt", doc->encrypt_dict) != HPDF_OK)
   {
      return doc->error.error_no;
   }

   doc->encrypt_on = HPDF_TRUE;

   return HPDF_OK;
}

HPDF_EXPORT(HpdfStatus)
   HPDF_SetPassword(
      HpdfDoc * const doc,
      char const *owner_passwd,
      char const *user_passwd)
{
   HPDF_PTRACE((" HPDF_SetPassword\n"));

   if (!HPDF_HasDoc(doc))
   {
      return HPDF_DOC_INVALID_OBJECT;
   }

   if (!doc->encrypt_dict) 
   {
      doc->encrypt_dict = HPDF_EncryptDict_New(doc->mmgr, doc->xref);

      if (!doc->encrypt_dict)
      {
         return HPDF_CheckError(&doc->error);
      }
   }

   if (HPDF_EncryptDict_SetPassword(doc->encrypt_dict, owner_passwd, user_passwd) != HPDF_OK)
   {
      return HPDF_CheckError(&doc->error);
   }

   return HPDF_Doc_SetEncryptOn(doc);
}

HPDF_EXPORT(HpdfStatus)
   HPDF_SetPermission(
      HpdfDoc * const doc,
      HpdfUInt   permission)
{
   HPDF_Encrypt e;

   HPDF_PTRACE((" HPDF_SetPermission\n"));

   if (!HPDF_HasDoc(doc))
   {
      return HPDF_DOC_INVALID_OBJECT;
   }

   e = HPDF_EncryptDict_GetAttr(doc->encrypt_dict);

   if (!e)
   {
      return HPDF_RaiseError(&doc->error, HPDF_DOC_ENCRYPTDICT_NOT_FOUND, 0);
   }
   else
   {
      e->permission = permission;
   }

   return HPDF_OK;
}

HPDF_EXPORT(HpdfStatus)
   HPDF_SetEncryptionMode(
      HpdfDoc * const doc,
      HPDF_EncryptMode   mode,
      HpdfUInt          key_len)
{
   HPDF_Encrypt e;

   HPDF_PTRACE((" HPDF_SetEncryptionMode\n"));

   if (!HPDF_Doc_Validate(doc))
   {
      return HPDF_DOC_INVALID_OBJECT;
   }

   e = HPDF_EncryptDict_GetAttr(doc->encrypt_dict);

   if (!e)
   {
      return HPDF_RaiseError(&doc->error, HPDF_DOC_ENCRYPTDICT_NOT_FOUND, 0);
   }
   else 
   {
      if (mode == HPDF_ENCRYPT_R2)
      {
         e->key_len = 5;
      }
      else 
      {
         /* if encryption mode is specified revision-3, the version of
         ** pdf file is set to 1.4 */
         if (doc->pdf_version < HPDF_VER_14)
         {
            doc->pdf_version = HPDF_VER_14;
         }

         if      (5 <= key_len && key_len <= 16)
         {
            e->key_len = key_len;
         }
         else if (key_len == 0)
         {
            e->key_len = 16;
         }
         else
         {
            return HPDF_RaiseError(&doc->error, HPDF_INVALID_ENCRYPT_KEY_LEN, 0);
         }
      }
      e->mode = mode;
   }

   return HPDF_OK;
}

HpdfStatus
   HPDF_Doc_SetEncryptOff(
      HpdfDoc * const doc)
{
   HPDF_PTRACE((" HPDF_Doc_SetEncryptOff\n"));

   if (!doc->encrypt_on)
   {
      return HPDF_OK;
   }

   /* if encrypy-dict object is registered to cross-reference-table,
   ** replace it to null-object.
   ** additionally remove encrypt-dict object from trailer-object. */
   if (doc->encrypt_dict) 
   {
      HpdfUInt obj_id = doc->encrypt_dict->header.obj_id;

      if (obj_id & HPDF_OTYPE_INDIRECT) 
      {
         HPDF_XrefEntry entry;
         HpdfValueNull *null_obj;

         HPDF_Dict_RemoveElement(doc->trailer, "Encrypt");

         entry = HPDF_Xref_GetEntryByObjectId(doc->xref, obj_id & 0x00FFFFFF);

         if (!entry) 
         {
            return HPDF_SetError(&doc->error, HPDF_DOC_ENCRYPTDICT_NOT_FOUND, 0);
         }

         null_obj = HpdfValueNullCreate(doc->mmgr);
         if (!null_obj)
         {
            return doc->error.error_no;
         }

         entry->obj = null_obj;
         null_obj->header.obj_id = obj_id | HPDF_OTYPE_INDIRECT;

         doc->encrypt_dict->header.obj_id = HPDF_OTYPE_NONE;
      }
   }

   doc->encrypt_on = HPDF_FALSE;
   return HPDF_OK;
}

HpdfStatus 
   HPDF_Doc_PrepareEncryption(
      HpdfDoc * const doc)
{
   HPDF_Encrypt e= HPDF_EncryptDict_GetAttr(doc->encrypt_dict);
   HPDF_Dict info = _GetInfo(doc);
   HpdfArray *id;

   if (!e)
   {
      return HPDF_DOC_ENCRYPTDICT_NOT_FOUND;
   }

   if (!info)
   {
      return doc->error.error_no;
   }

   if (HPDF_EncryptDict_Prepare(doc->encrypt_dict, info, doc->xref) != HPDF_OK)
   {
      return doc->error.error_no;
   }

   /* reset 'ID' to trailer-dictionary */
   id = HPDF_Dict_GetItem(doc->trailer, "ID", HPDF_OCLASS_ARRAY);
   if (!id) 
   {
      id = HPDF_Array_New(doc->mmgr);

      if (!id || 
          HPDF_Dict_Add(doc->trailer, "ID", id) != HPDF_OK)
      {
         return doc->error.error_no;
      }
   }
   else
   {
      HPDF_Array_Clear(id);
   }

   if (HPDF_Array_Add(id, HPDF_Binary_New(doc->mmgr, e->encrypt_id, HPDF_ID_LEN)) != HPDF_OK)
   {
      return doc->error.error_no;
   }

   if (HPDF_Array_Add(id, HPDF_Binary_New(doc->mmgr, e->encrypt_id, HPDF_ID_LEN)) != HPDF_OK)
   {
      return doc->error.error_no;
   }

   return HPDF_OK;
}

static HpdfStatus
   _InternalSaveToStream(
      HpdfDoc * const doc,
      HPDF_Stream   stream)
{
   HpdfStatus ret;

   if ((ret = _WriteHeader(doc, stream)) != HPDF_OK)
   {
      return ret;
   }

   /* prepare trailer */
   if ((ret = _PrepareTrailer(doc)) != HPDF_OK)
   {
      return ret;
   }

   /* prepare encryption */
   if (doc->encrypt_on) 
   {
      HPDF_Encrypt e= HPDF_EncryptDict_GetAttr(doc->encrypt_dict);

      if ((ret = HPDF_Doc_PrepareEncryption(doc)) != HPDF_OK)
      {
         return ret;
      }

      if ((ret = HPDF_Xref_WriteToStream(doc->xref, stream, e)) != HPDF_OK)
      {
         return ret;
      }
   }
   else 
   {
      if ((ret = HPDF_Xref_WriteToStream(doc->xref, stream, NULL)) != HPDF_OK)
      {
         return ret;
      }
   }

   return HPDF_OK;
}

HPDF_EXPORT(HpdfStatus)
   HPDF_SaveToStream(
      HpdfDoc * const doc)
{
   HPDF_PTRACE((" HPDF_SaveToStream\n"));

   if (!HPDF_HasDoc(doc))
   {
      return HPDF_INVALID_DOCUMENT;
   }

   if (!doc->stream)
   {
      doc->stream = HPDF_MemStream_New(doc->mmgr, HPDF_STREAM_BUF_SIZ);
   }

   if (!HPDF_Stream_Validate(doc->stream))
   {
      return HPDF_RaiseError(&doc->error, HPDF_INVALID_STREAM, 0);
   }

   HPDF_MemStream_FreeData(doc->stream);

   if (_InternalSaveToStream(doc, doc->stream) != HPDF_OK)
   {
      return HPDF_CheckError(&doc->error);
   }

   return HPDF_OK;
}

HPDF_EXPORT(HpdfStatus)
   HPDF_GetContents(
      HpdfDoc * const doc,
      HpdfByte  * const buf,
      HpdfUInt32  *size)
{
   HPDF_Stream stream;
   HpdfUInt isize = *size;
   HpdfStatus ret;

   HPDF_PTRACE((" HPDF_GetContents\n"));

   if (!HPDF_HasDoc(doc)) 
   {
      return HPDF_INVALID_DOCUMENT;
   }

   stream = HPDF_MemStream_New(doc->mmgr, HPDF_STREAM_BUF_SIZ);

   if (!stream) 
   {
      return HPDF_CheckError(&doc->error);
   }

   if (_InternalSaveToStream(doc, stream) != HPDF_OK) 
   {
      HPDF_Stream_Free(stream);
      return HPDF_CheckError(&doc->error);
   }

   ret = HPDF_Stream_Read(stream, buf, &isize);

   *size = isize;
   HPDF_Stream_Free(stream);

   return ret;
}

HPDF_EXPORT(HpdfUInt32)
   HPDF_GetStreamSize(
      HpdfDoc * const doc)
{
   HPDF_PTRACE((" HPDF_GetStreamSize\n"));

   if (!HPDF_HasDoc(doc))
   {
      return HPDF_INVALID_DOCUMENT;
   }

   if (!HPDF_Stream_Validate(doc->stream))
   {
      return 0;
   }

   return HPDF_Stream_Size(doc->stream);
}

HPDF_EXPORT(HpdfStatus)
   HPDF_ReadFromStream(
      HpdfDoc * const doc,
      HpdfByte     * const buf,
      HpdfUInt32   *size)
{
   HpdfUInt isize = *size;
   HpdfStatus ret;

   if (!HPDF_HasDoc(doc))
   {
      return HPDF_INVALID_DOCUMENT;
   }

   if (!HPDF_Stream_Validate(doc->stream))
   {
      return HPDF_RaiseError(&doc->error, HPDF_INVALID_OPERATION, 0);
   }

   if (*size == 0)
   {
      return HPDF_RaiseError(&doc->error, HPDF_INVALID_PARAMETER, 0);
   }

   ret = HPDF_Stream_Read(doc->stream, buf, &isize);

   *size = isize;

   if (ret != HPDF_OK)
   {
      HPDF_CheckError(&doc->error);
   }

   return ret;
}

HPDF_EXPORT(HpdfStatus)
   HPDF_ResetStream(
      HpdfDoc * const doc)
{
   if (!HPDF_HasDoc(doc))
   {
      return HPDF_INVALID_DOCUMENT;
   }

   if (!HPDF_Stream_Validate(doc->stream))
   {
      return HPDF_RaiseError(&doc->error, HPDF_INVALID_OPERATION, 0);
   }

   return HPDF_Stream_Seek(doc->stream, 0, HPDF_SEEK_SET);
}

HPDF_EXPORT(HpdfStatus)
   HPDF_SaveToFile(
      HpdfDoc * const doc,
      char const *file_name)
{
   HPDF_Stream stream;

   HPDF_PTRACE((" HPDF_SaveToFile\n"));

   if (!HPDF_HasDoc(doc))
   {
      return HPDF_INVALID_DOCUMENT;
   }

   stream = HPDF_FileWriter_New(doc->mmgr, file_name);
   if (!stream)
   {
      return HPDF_CheckError(&doc->error);
   }

   _InternalSaveToStream(doc, stream);

   HPDF_Stream_Free(stream);

   return HPDF_CheckError(&doc->error);
}

#if defined(WIN32)
HPDF_EXPORT(HpdfStatus)
   HPDF_SaveToFileW(
      HpdfDoc * const doc,
      wchar_t const  *file_name)
{
   HPDF_Stream stream;

   HPDF_PTRACE((" HPDF_SaveToFileW\n"));

   if (!HPDF_HasDoc(doc))
   {
      return HPDF_INVALID_DOCUMENT;
   }

   stream = HPDF_FileWriter_NewW(doc->mmgr, file_name);
   if (!stream)
   {
      return HPDF_CheckError(&doc->error);
   }

   _InternalSaveToStream(doc, stream);

   HPDF_Stream_Free(stream);

   return HPDF_CheckError(&doc->error);
}
#endif

HPDF_EXPORT(HPDF_Page)
   HPDF_GetCurrentPage(
      HpdfDoc * const doc)
{
   HPDF_PTRACE((" HPDF_GetCurrentPage\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   return doc->cur_page;
}

HPDF_EXPORT(HPDF_Page)
   HPDF_GetPageByIndex(
      HpdfDoc * const doc,
      HpdfUInt   index)
{
   HPDF_Page ret;

   HPDF_PTRACE((" HPDF_GetPageByIndex\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   ret = HPDF_List_ItemAt(doc->page_list, index);
   if (!ret) 
   {
      HPDF_RaiseError(&doc->error, HPDF_INVALID_PAGE_INDEX, 0);
      return NULL;
   }

   return ret;
}

HPDF_Pages
   HPDF_Doc_GetCurrentPages(
      HpdfDoc * const doc)
{
   HPDF_PTRACE((" HPDF_GetCurrentPages\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   return doc->cur_pages;
}

HpdfStatus
   HPDF_Doc_SetCurrentPages(
      HpdfDoc * const doc,
      HPDF_Pages   pages)
{
   HPDF_PTRACE((" HPDF_Doc_SetCurrentPages\n"));

   if (!HPDF_HasDoc(doc))
   {
      return HPDF_INVALID_DOCUMENT;
   }

   if (!HPDF_Pages_Validate(pages))
   {
      return HPDF_SetError(&doc->error, HPDF_INVALID_PAGES, 0);
   }

   /* check whether the pages belong to the pdf */
   if (doc->mmgr != pages->mmgr)
   {
      return HPDF_SetError(&doc->error, HPDF_INVALID_PAGES, 0);
   }

   doc->cur_pages = pages;

   return HPDF_OK;
}

HpdfStatus
   HPDF_Doc_SetCurrentPage(
      HpdfDoc * const doc,
      HPDF_Page   page)
{
   HPDF_PTRACE((" HPDF_Doc_SetCurrentPage\n"));

   if (!HPDF_HasDoc(doc))
   {
      return HPDF_INVALID_DOCUMENT;
   }

   if (!HPDF_Page_Validate(page))
   {
      return HPDF_SetError(&doc->error, HPDF_INVALID_PAGE, 0);
   }

   /* check whether the page belong to the pdf */
   if (doc->mmgr != page->mmgr)
   {
      return HPDF_SetError(&doc->error, HPDF_INVALID_PAGE, 0);
   }

   doc->cur_page = page;

   return HPDF_OK;
}

HPDF_EXPORT(HPDF_Page)
   HPDF_AddPage(
      HpdfDoc * const doc)
{
   HPDF_Page page;
   HpdfStatus ret;

   HPDF_PTRACE((" HPDF_AddPage\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   if (doc->page_per_pages) 
   {
      if (doc->page_per_pages <= doc->cur_page_num) 
      {
         doc->cur_pages = HPDF_Doc_AddPagesTo(doc, doc->root_pages);
         if (!doc->cur_pages)
         {
            return NULL;
         }
         doc->cur_page_num = 0;
      }
   }

   page = HPDF_Page_New(doc->mmgr, doc->xref);
   if (!page) 
   {
      HPDF_CheckError(&doc->error);
      return NULL;
   }

   if ((ret = HPDF_Pages_AddKids(doc->cur_pages, page)) != HPDF_OK) 
   {
      HPDF_RaiseError(&doc->error, ret, 0);
      return NULL;
   }

   if ((ret = HPDF_List_Add(doc->page_list, page)) != HPDF_OK) 
   {
      HPDF_RaiseError(&doc->error, ret, 0);
      return NULL;
   }

   doc->cur_page = page;

   if (doc->compression_mode & HPDF_COMP_TEXT)
   {
      HPDF_Page_SetFilter(page, HPDF_STREAM_FILTER_FLATE_DECODE);
   }

   doc->cur_page_num++;

   return page;
}

HPDF_Pages
   HPDF_Doc_AddPagesTo(
      HpdfDoc * const doc,
      HPDF_Pages   parent)
{
   HPDF_Pages pages;

   HPDF_PTRACE((" HPDF_AddPagesTo\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   if (!HPDF_Pages_Validate(parent)) 
   {
      HPDF_RaiseError(&doc->error, HPDF_INVALID_PAGES, 0);
      return NULL;
   }

   /* check whether the page belong to the pdf */
   if (doc->mmgr != parent->mmgr) 
   {
      HPDF_RaiseError(&doc->error, HPDF_INVALID_PAGES, 0);
      return NULL;
   }

   pages = HPDF_Pages_New(doc->mmgr, parent, doc->xref);
   if (pages)
   {
      doc->cur_pages = pages;
   }
   else
   {
      HPDF_CheckError(&doc->error);
   }

   return  pages;
}

HPDF_EXPORT(HPDF_Page)
   HPDF_InsertPage(
      HpdfDoc * const doc,
      HPDF_Page   target)
{
   HPDF_Page page;
   HpdfStatus ret;

   HPDF_PTRACE((" HPDF_InsertPage\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   if (!HPDF_Page_Validate(target)) 
   {
      HPDF_RaiseError(&doc->error, HPDF_INVALID_PAGE, 0);
      return NULL;
   }

   /* check whether the page belong to the pdf */
   if (doc->mmgr != target->mmgr) 
   {
      HPDF_RaiseError(&doc->error, HPDF_INVALID_PAGE, 0);
      return NULL;
   }

   page = HPDF_Page_New(doc->mmgr, doc->xref);
   if (!page) 
   {
      HPDF_CheckError(&doc->error);
      return NULL;
   }

   if ((ret = HPDF_Page_InsertBefore(page, target)) != HPDF_OK) 
   {
      HPDF_RaiseError(&doc->error, ret, 0);
      return NULL;
   }

   if ((ret = HPDF_List_Insert(doc->page_list, target, page)) != HPDF_OK) 
   {
      HPDF_RaiseError(&doc->error, ret, 0);
      return NULL;
   }

   if (doc->compression_mode & HPDF_COMP_TEXT)
   {
      HPDF_Page_SetFilter(page, HPDF_STREAM_FILTER_FLATE_DECODE);
   }

   return page;
}

HPDF_EXPORT(HpdfStatus)
   HPDF_SetErrorHandler(
      HpdfDoc * const doc,
      HPDF_Error_Handler   user_error_fn)
{
   if (!HPDF_Doc_Validate(doc))
   {
      return HPDF_INVALID_DOCUMENT;
   }

   doc->error.error_fn = user_error_fn;

   return HPDF_OK;
}

/*----- font handling -------------------------------------------------------*/

static void
   _FreeFontDefList(
      HpdfDoc * const doc)
{
   HpdfList *list = doc->fontdef_list;
   HpdfUInt i;

   HPDF_PTRACE((" HPDF_Doc_FreeFontDefList\n"));

   for (i = 0; i < list->count; i++) 
   {
      HPDF_FontDef def = (HPDF_FontDef)HPDF_List_ItemAt(list, i);

      HPDF_FontDef_Free(def);
   }

   HPDF_List_Free(list);

   doc->fontdef_list = NULL;
}

static void
   _CleanupFontDefList(
      HpdfDoc const * const doc)
{
   HpdfList *list = doc->fontdef_list;
   HpdfUInt i;

   HPDF_PTRACE((" _CleanupFontDefList\n"));

   for (i = 0; i < list->count; i++) 
   {
      HPDF_FontDef def = (HPDF_FontDef)HPDF_List_ItemAt(list, i);

      HPDF_FontDef_Cleanup(def);
   }
}

HPDF_FontDef
   HPDF_Doc_FindFontDef(
      HpdfDoc const * const doc,
      char const *font_name)
{
   HpdfList *list = doc->fontdef_list;
   HpdfUInt i;

   HPDF_PTRACE((" HPDF_Doc_FindFontDef\n"));

   for (i = 0; i < list->count; i++) 
   {
      HPDF_FontDef def = (HPDF_FontDef)HPDF_List_ItemAt(list, i);

      if (HpdfStrIsEqual(font_name, def->base_font)) 
      {
         if (def->type == HPDF_FONTDEF_TYPE_UNINITIALIZED) 
         {
            if (!def->init_fn ||
                def->init_fn(def) != HPDF_OK)
            {
               return NULL;
            }
         }

         return def;
      }
   }

   return NULL;
}

HpdfStatus
   HPDF_Doc_RegisterFontDef(
      HpdfDoc * const doc,
      HPDF_FontDef   fontdef)
{
   HpdfStatus ret;

   HPDF_PTRACE((" HPDF_Doc_RegisterFontDef\n"));

   if (!fontdef)
   {
      return HPDF_SetError(&doc->error, HPDF_INVALID_OBJECT, 0);
   }

   if (HPDF_Doc_FindFontDef(doc, fontdef->base_font) != NULL) 
   {
      HPDF_FontDef_Free(fontdef);
      return HPDF_SetError(&doc->error, HPDF_DUPLICATE_REGISTRATION, 0);
   }

   if ((ret = HPDF_List_Add(doc->fontdef_list, fontdef)) != HPDF_OK) 
   {
      HPDF_FontDef_Free(fontdef);
      return HPDF_SetError(&doc->error, ret, 0);
   }

   return HPDF_OK;
}

HPDF_FontDef
   HPDF_GetFontDef(
      HpdfDoc * const doc,
      char const *font_name)
{
   HpdfStatus ret;
   HPDF_FontDef def;

   HPDF_PTRACE((" HPDF_GetFontDef\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   def = HPDF_Doc_FindFontDef(doc, font_name);

   if (!def) 
   {
      def = HPDF_Base14FontDef_New(doc->mmgr, font_name);

      if (!def)
      {
         return NULL;
      }

      if ((ret = HPDF_List_Add(doc->fontdef_list, def)) != HPDF_OK) 
      {
         HPDF_FontDef_Free(def);
         HPDF_RaiseError(&doc->error, ret, 0);
         def = NULL;
      }
   }

   return def;
}

/*----- encoder handling ----------------------------------------------------*/

HpdfEncoder *
   HPDF_Doc_FindEncoder(
      HpdfDoc const * const doc,
      char const *encoding_name)
{
   HpdfList *list = doc->encoder_list;
   HpdfUInt i;

   HPDF_PTRACE((" HPDF_Doc_FindEncoder\n"));

   for (i = 0; i < list->count; i++) 
   {
      HpdfEncoder *encoder = (HpdfEncoder *)HPDF_List_ItemAt(list, i);

      if (HpdfStrIsEqual(encoding_name, encoder->name)) 
      {
         /* if encoder is uninitialize, call init_fn() */
         if (encoder->type == HPDF_ENCODER_TYPE_UNINITIALIZED) 
         {
            if (!encoder->init_fn ||
                encoder->init_fn(encoder) != HPDF_OK)
            {
               return NULL;
            }
         }

         return encoder;
      }
   }

   return NULL;
}

HpdfStatus
   HPDF_Doc_RegisterEncoder(
      HpdfDoc * const doc,
      HpdfEncoder * const encoder)
{
   HpdfStatus ret;

   if (!encoder)
   {
      return HPDF_SetError(&doc->error, HPDF_INVALID_OBJECT, 0);
   }

   if (HPDF_Doc_FindEncoder(doc, encoder->name) != NULL) 
   {
      HPDF_Encoder_Free(encoder);
      return HPDF_SetError(&doc->error, HPDF_DUPLICATE_REGISTRATION, 0);
   }

   if ((ret = HPDF_List_Add(doc->encoder_list, encoder)) != HPDF_OK) 
   {
      HPDF_Encoder_Free(encoder);
      return HPDF_SetError(&doc->error, ret, 0);
   }

   return HPDF_OK;
}

HPDF_EXPORT(HpdfEncoder *)
   HPDF_GetEncoder(
      HpdfDoc * const doc,
      char const *encoding_name)
{
   HpdfEncoder *encoder;
   HpdfStatus ret;

   HPDF_PTRACE((" HPDF_GetEncoder\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   encoder = HPDF_Doc_FindEncoder(doc, encoding_name);

   if (!encoder) 
   {
      encoder = HPDF_BasicEncoder_New(doc->mmgr, encoding_name);

      if (!encoder) 
      {
         HPDF_CheckError(&doc->error);
         return NULL;
      }

      if ((ret = HPDF_List_Add(doc->encoder_list, encoder)) != HPDF_OK) 
      {
         HPDF_Encoder_Free(encoder);
         HPDF_RaiseError(&doc->error, ret, 0);
         return NULL;
      }
   }

   return encoder;
}

HPDF_EXPORT(HpdfEncoder *)
   HPDF_GetCurrentEncoder(
      HpdfDoc * const doc)
{
   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   return doc->cur_encoder;
}

HPDF_EXPORT(HpdfStatus)
   HPDF_SetCurrentEncoder(
      HpdfDoc * const doc,
      char const *encoding_name)
{
   HpdfEncoder *encoder;

   if (!HPDF_HasDoc(doc))
   {
      return HPDF_GetError(doc);
   }

   encoder = HPDF_GetEncoder(doc, encoding_name);

   if (!encoder)
   {
      return HPDF_GetError(doc);
   }

   doc->cur_encoder = encoder;

   return HPDF_OK;
}

static void
   _FreeEncoderList(
      HpdfDoc * const doc)
{
   HpdfList *list = doc->encoder_list;
   HpdfUInt i;

   HPDF_PTRACE((" _FreeEncoderList\n"));

   for (i = 0; i < list->count; i++) 
   {
      HpdfEncoder *encoder = (HpdfEncoder *) HPDF_List_ItemAt(list, i);

      HPDF_Encoder_Free(encoder);
   }

   HPDF_List_Free(list);

   doc->encoder_list = NULL;
}

/*----- font handling -------------------------------------------------------*/

HPDF_Font
   HPDF_Doc_FindFont(
      HpdfDoc const * const doc,
      char const *font_name,
      char const *encoding_name)
{
   HpdfUInt i;
   HPDF_Font font;

   HPDF_PTRACE((" HPDF_Doc_FindFont\n"));

   for (i = 0; i < doc->font_mgr->count; i++) 
   {
      HPDF_FontAttr attr;

      font = (HPDF_Font)HPDF_List_ItemAt(doc->font_mgr, i);
      attr = (HPDF_FontAttr)font->attr;

      if (HpdfStrIsEqual(attr->fontdef->base_font, font_name) &&
          HpdfStrIsEqual(attr->encoder->name,      encoding_name))
      {
         return font;
      }
   }

   return NULL;
}

HPDF_EXPORT(HPDF_Font)
   HPDF_GetFont(
      HpdfDoc * const doc,
      char const *font_name,
      char const *encoding_name)
{
   HPDF_FontDef fontdef = NULL;
   HpdfEncoder *encoder = NULL;
   HPDF_Font font;

   HPDF_PTRACE((" HPDF_GetFont\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   if (!font_name) 
   {
      HPDF_RaiseError(&doc->error, HPDF_INVALID_FONT_NAME, 0);
      return NULL;
   }

   /* if encoding-name is not specified, find default-encoding of fontdef */
   if (!encoding_name) 
   {
      fontdef = HPDF_GetFontDef(doc, font_name);

      if (fontdef) 
      {
         HPDF_Type1FontDefAttr attr = (HPDF_Type1FontDefAttr)fontdef->attr;

         if (fontdef->type == HPDF_FONTDEF_TYPE_TYPE1 &&
             HpdfStrIsEqual(attr->encoding_scheme, HPDF_ENCODING_FONT_SPECIFIC))
         {
            encoder = HPDF_GetEncoder(doc, HPDF_ENCODING_FONT_SPECIFIC);
         }
         else
         {
            encoder = HPDF_GetEncoder(doc, HPDF_ENCODING_STANDARD);
         }
      }
      else
      {
         HPDF_CheckError(&doc->error);
         return NULL;
      }

      if (!encoder)
      {
         HPDF_CheckError(&doc->error);
         return NULL;
      }

      font = HPDF_Doc_FindFont(doc, font_name, encoder->name);
   }
   else
   {
      font = HPDF_Doc_FindFont(doc, font_name, encoding_name);
   }

   if (font)
   {
      return font;
   }

   if (!fontdef) 
   {
      fontdef = HPDF_GetFontDef(doc, font_name);

      if (!fontdef) 
      {
         HPDF_CheckError(&doc->error);
         return NULL;
      }
   }

   if (!encoder) 
   {
      encoder = HPDF_GetEncoder(doc, encoding_name);

      if (!encoder)
      {
         return NULL;
      }
   }

   switch (fontdef->type) 
   {
   case HPDF_FONTDEF_TYPE_TYPE1:
      font = HPDF_Type1Font_New(doc->mmgr, fontdef, encoder, doc->xref);

      if (font)
      {
         HPDF_List_Add(doc->font_mgr, font);
      }

      break;

   case HPDF_FONTDEF_TYPE_TRUETYPE:
      if (encoder->type == HPDF_ENCODER_TYPE_DOUBLE_BYTE)
      { 
         font = HPDF_Type0Font_New(doc->mmgr, fontdef, encoder, doc->xref);
      }
      else
      {
         font = HPDF_TTFont_New(doc->mmgr, fontdef, encoder, doc->xref);
      }

      if (font)
      {
         HPDF_List_Add(doc->font_mgr, font);
      }

      break;

   case HPDF_FONTDEF_TYPE_CID:
      font = HPDF_Type0Font_New(doc->mmgr, fontdef, encoder, doc->xref);

      if (font)
      {
         HPDF_List_Add(doc->font_mgr, font);
      }

      break;

   default:
      HPDF_RaiseError(&doc->error, HPDF_UNSUPPORTED_FONT_TYPE, 0);
      return NULL;
   }

   if (!font)
   {
      HPDF_CheckError(&doc->error);
   }

   if (font && (doc->compression_mode & HPDF_COMP_METADATA))
   {
      font->filter = HPDF_STREAM_FILTER_FLATE_DECODE;
   }

   return font;
}

HPDF_EXPORT(char const*)
   HPDF_LoadType1FontFromFile(
      HpdfDoc * const doc,
      char const *afm_file_name,
      char const *data_file_name)
{
   HPDF_Stream afm;
   HPDF_Stream pfm = NULL;
   char const *ret;

   HPDF_PTRACE((" HPDF_LoadType1FontFromFile\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   /* create file stream */
   afm = HPDF_FileReader_New(doc->mmgr, afm_file_name);

   if (data_file_name)
   {
      pfm = HPDF_FileReader_New(doc->mmgr, data_file_name);
   }

   if (HPDF_Stream_Validate(afm) &&
       (!data_file_name || 
        HPDF_Stream_Validate(pfm))) 
   {
      ret = _LoadType1FontFromStream(doc, afm, pfm);
   }
   else
   {
      ret = NULL;
   }

   /* destroy file stream */
   if (afm)
   {
      HPDF_Stream_Free(afm);
   }

   if (pfm)
   {
      HPDF_Stream_Free(pfm);
   }

   if (!ret)
   {
      HPDF_CheckError(&doc->error);
   }

   return ret;
}

#if defined(WIN32)
HPDF_EXPORT(char const*)
   HPDF_LoadType1FontFromFileW(
      HpdfDoc * const doc,
      wchar_t const  *afm_file_name,
      wchar_t const  *data_file_name)
{
   HPDF_Stream afm;
   HPDF_Stream pfm = NULL;
   char const *ret;

   HPDF_PTRACE((" HPDF_LoadType1FontFromFileW\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   /* create file stream */
   afm = HPDF_FileReader_NewW(doc->mmgr, afm_file_name);

   if (data_file_name)
   {
      pfm = HPDF_FileReader_NewW(doc->mmgr, data_file_name);
   }

   if (HPDF_Stream_Validate(afm) &&
       (!data_file_name || 
        HPDF_Stream_Validate(pfm))) 
   {
      ret = _LoadType1FontFromStream(doc, afm, pfm);
   }
   else
   {
      ret = NULL;
   }

   /* destroy file stream */
   if (afm)
   {
      HPDF_Stream_Free(afm);
   }

   if (pfm)
   {
      HPDF_Stream_Free(pfm);
   }

   if (!ret)
   {
      HPDF_CheckError(&doc->error);
   }

   return ret;
}
#endif

static char const*
   _LoadType1FontFromStream(
      HpdfDoc * const doc,
      HPDF_Stream   afmdata,
      HPDF_Stream   pfmdata)
{
   HPDF_FontDef def;

   HPDF_PTRACE((" HPDF_LoadType1FontFromStream\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   def = HPDF_Type1FontDef_Load(doc->mmgr, afmdata, pfmdata);
   if (def) 
   {
      HPDF_FontDef  tmpdef = HPDF_Doc_FindFontDef(doc, def->base_font);
      if (tmpdef) 
      {
         HPDF_FontDef_Free(def);
         HPDF_SetError(&doc->error, HPDF_FONT_EXISTS, 0);
         return NULL;
      }

      if (HPDF_List_Add(doc->fontdef_list, def) != HPDF_OK) 
      {
         HPDF_FontDef_Free(def);
         return NULL;
      }
      return def->base_font;
   }
   return NULL;
}

HPDF_EXPORT(HPDF_FontDef)
   HPDF_GetTTFontDefFromFile(
      HpdfDoc const * const doc,
      char const  *file_name,
      HpdfBool     embedding)
{
   HPDF_Stream font_data;
   HPDF_FontDef def;

   HPDF_PTRACE((" HPDF_GetTTFontDefFromFile\n"));

   /* create file stream */
   font_data = HPDF_FileReader_New(doc->mmgr, file_name);

   if (HPDF_Stream_Validate(font_data)) 
   {
      def = HPDF_TTFontDef_Load(doc->mmgr, font_data, embedding);
   }
   else 
   {
      HPDF_CheckError(&doc->error);
      return NULL;
   }

   return def;
}

HPDF_EXPORT(char const*)
   HPDF_LoadTTFontFromFile(
      HpdfDoc * const doc,
      char const     *file_name,
      HpdfBool        embedding)
{
   HPDF_Stream font_data;
   char const *ret;

   HPDF_PTRACE((" HPDF_LoadTTFontFromFile\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   /* create file stream */
   font_data = HPDF_FileReader_New(doc->mmgr, file_name);

   if (HPDF_Stream_Validate(font_data)) 
   {
      ret = _LoadTTFontFromStream(doc, font_data, embedding);
   }
   else
   {
      ret = NULL;
   }

   if (!ret)
   {
      HPDF_CheckError(&doc->error);
   }

   return ret;
}

#if defined(WIN32)
HPDF_EXPORT(HPDF_FontDef)
   HPDF_GetTTFontDefFromFileW(
      HpdfDoc const * const doc,
      wchar_t const        *file_name,
      HpdfBool              embedding)
{
   HPDF_Stream font_data;
   HPDF_FontDef def;

   HPDF_PTRACE((" HPDF_GetTTFontDefFromFileW\n"));

   /* create file stream */
   font_data = HPDF_FileReader_NewW(doc->mmgr, file_name);

   if (HPDF_Stream_Validate(font_data)) 
   {
      def = HPDF_TTFontDef_Load(doc->mmgr, font_data, embedding);
   }
   else 
   {
      HPDF_CheckError(&doc->error);
      return NULL;
   }

   return def;
}

HPDF_EXPORT(char const*)
   HPDF_LoadTTFontFromFileW(
      HpdfDoc * const doc,
      wchar_t const  *file_name,
      HpdfBool        embedding)
{
   HPDF_Stream font_data;
   char const *ret;

   HPDF_PTRACE((" HPDF_LoadTTFontFromFileW\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   /* create file stream */
   font_data = HPDF_FileReader_NewW(doc->mmgr, file_name);

   if (HPDF_Stream_Validate(font_data)) 
   {
      ret = _LoadTTFontFromStream(doc, font_data, embedding);
   }
   else
   {
      ret = NULL;
   }

   if (!ret)
   {
      HPDF_CheckError(&doc->error);
   }

   return ret;
}
#endif

static char const*
   _LoadTTFontFromStream(
      HpdfDoc * const doc,
      HPDF_Stream      font_data,
      HpdfBool        embedding)
{
   HPDF_FontDef def;

   HPDF_PTRACE((" HPDF_LoadTTFontFromStream\n"));

   def = HPDF_TTFontDef_Load(doc->mmgr, font_data, embedding);
   if (def) 
   {
      HPDF_FontDef  tmpdef = HPDF_Doc_FindFontDef(doc, def->base_font);
      if (tmpdef) 
      {
         HPDF_FontDef_Free(def);
         return tmpdef->base_font;
      }

      if (HPDF_List_Add(doc->fontdef_list, def) != HPDF_OK) 
      {
         HPDF_FontDef_Free(def);
         return NULL;
      }
   }
   else
   {
      return NULL;
   }

   if (embedding) 
   {
      if (doc->ttfont_tag[0] == 0) 
      {
         HPDF_MemCpy(doc->ttfont_tag, (HpdfByte *) "HPDFAA", 6);
      }
      else 
      {
         HpdfInt i;

         for (i = 5; i >= 0; i--) 
         {
            doc->ttfont_tag[i] += 1;
            if (doc->ttfont_tag[i] > 'Z')
            {
               doc->ttfont_tag[i] = 'A';
            }
            else
            {
               break;
            }
         }
      }

      HPDF_TTFontDef_SetTagName(def, (char *)doc->ttfont_tag);
   }

   return def->base_font;
}

HPDF_EXPORT(char const*)
   HPDF_LoadTTFontFromFile2(
      HpdfDoc * const doc,
      char const     *file_name,
      HpdfUInt        index,
      HpdfBool        embedding)
{
   HPDF_Stream font_data;
   char const *ret;

   HPDF_PTRACE((" HPDF_LoadTTFontFromFile2\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   /* create file stream */
   font_data = HPDF_FileReader_New(doc->mmgr, file_name);

   if (HPDF_Stream_Validate(font_data)) 
   {
      ret = _LoadTTFontFromStream2(doc, font_data, index, embedding);
   }
   else
   {
      ret = NULL;
   }

   if (!ret)
   {
      HPDF_CheckError(&doc->error);
   }

   return ret;
}

#if defined(WIN32)
HPDF_EXPORT(char const*)
   HPDF_LoadTTFontFromFile2W(
      HpdfDoc * const doc,
      wchar_t const  *file_name,
      HpdfUInt        index,
      HpdfBool        embedding)
{
   HPDF_Stream font_data;
   char const *ret;

   HPDF_PTRACE((" HPDF_LoadTTFontFromFile2\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   /* create file stream */
   font_data = HPDF_FileReader_NewW(doc->mmgr, file_name);

   if (HPDF_Stream_Validate(font_data)) 
   {
      ret = _LoadTTFontFromStream2(doc, font_data, index, embedding);
   }
   else
   {
      ret = NULL;
   }

   if (!ret)
   {
      HPDF_CheckError(&doc->error);
   }

   return ret;
}
#endif

static char const*
   _LoadTTFontFromStream2(
      HpdfDoc * const doc,
      HPDF_Stream      font_data,
      HpdfUInt        index,
      HpdfBool        embedding)
{
   HPDF_FontDef def;

   HPDF_PTRACE((" HPDF_LoadTTFontFromStream2\n"));

   def = HPDF_TTFontDef_Load2(doc->mmgr, font_data, index, embedding);
   if (def) 
   {
      HPDF_FontDef  tmpdef = HPDF_Doc_FindFontDef(doc, def->base_font);
      if (tmpdef) 
      {
         HPDF_FontDef_Free(def);
         return tmpdef->base_font;
      }

      if (HPDF_List_Add(doc->fontdef_list, def) != HPDF_OK) 
      {
         HPDF_FontDef_Free(def);
         return NULL;
      }
   }
   else
   {
      return NULL;
   }

   if (embedding) 
   {
      if (doc->ttfont_tag[0] == 0) 
      {
         HPDF_MemCpy(doc->ttfont_tag, (HpdfByte *) "HPDFAA", 6);
      }
      else 
      {
         HpdfInt i;

         for (i = 5; i >= 0; i--) 
         {
            doc->ttfont_tag[i] += 1;
            if (doc->ttfont_tag[i] > 'Z')
            {
               doc->ttfont_tag[i] = 'A';
            }
            else
            {
               break;
            }
         }
      }

      HPDF_TTFontDef_SetTagName(def, (char *)doc->ttfont_tag);
   }

   return def->base_font;
}

HPDF_EXPORT(HPDF_Image)
   HPDF_LoadRawImageFromFile(
      HpdfDoc * const doc,
      char const      *filename,
      HpdfUInt         width,
      HpdfUInt         height,
      HPDF_ColorSpace   color_space)
{
   HPDF_Stream imagedata;
   HPDF_Image image;

   HPDF_PTRACE((" HPDF_LoadRawImageFromFile\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   /* create file stream */
   imagedata = HPDF_FileReader_New(doc->mmgr, filename);

   if (HPDF_Stream_Validate(imagedata))
   {
      image = HPDF_Image_LoadRawImage(doc->mmgr, imagedata, doc->xref, width, height, color_space);
   }
   else
   {
      image = NULL;
   }

   /* destroy file stream */
   HPDF_Stream_Free(imagedata);

   if (!image)
   {
      HPDF_CheckError(&doc->error);
   }

   if (image && doc->compression_mode & HPDF_COMP_IMAGE)
   {
      image->filter = HPDF_STREAM_FILTER_FLATE_DECODE;
   }

   return image;
}

#if defined(WIN32)
HPDF_EXPORT(HPDF_Image)
   HPDF_LoadRawImageFromFileW(
      HpdfDoc * const doc,
      wchar_t const * const filename,
      HpdfUInt         width,
      HpdfUInt         height,
      HPDF_ColorSpace   color_space)
{
   HPDF_Stream imagedata;
   HPDF_Image image;

   HPDF_PTRACE((" HPDF_LoadRawImageFromFileW\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   /* create file stream */
   imagedata = HPDF_FileReader_NewW(doc->mmgr, filename);

   if (HPDF_Stream_Validate(imagedata))
   {
      image = HPDF_Image_LoadRawImage(doc->mmgr, imagedata, doc->xref, width, height, color_space);
   }
   else
   {
      image = NULL;
   }

   /* destroy file stream */
   HPDF_Stream_Free(imagedata);

   if (!image)
   {
      HPDF_CheckError(&doc->error);
   }

   if (image && doc->compression_mode & HPDF_COMP_IMAGE)
   {
      image->filter = HPDF_STREAM_FILTER_FLATE_DECODE;
   }

   return image;
}
#endif

HPDF_EXPORT(HPDF_Image)
   HPDF_LoadRawImageFromMem(
      HpdfDoc * const doc,
      HpdfByte   const * const buf,
      HpdfUInt          width,
      HpdfUInt          height,
      HPDF_ColorSpace    color_space,
      HpdfUInt          bits_per_component)
{
   HPDF_Image image;

   HPDF_PTRACE((" HPDF_LoadRawImageFromMem\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   /* Use directly HPDF_Image_LoadRaw1BitImageFromMem to save B/W images */
   if (color_space == HPDF_CS_DEVICE_GRAY && 
       bits_per_component == 1) 
   {
      return HPDF_Image_LoadRaw1BitImageFromMem(doc, buf, width, height, (width+7)/8, HPDF_TRUE, HPDF_TRUE);
   }

   image = HPDF_Image_LoadRawImageFromMem(doc->mmgr, buf, doc->xref, width, height, color_space, bits_per_component);

   if (!image)
   {
      HPDF_CheckError(&doc->error);
   }

   if (image && 
       doc->compression_mode & HPDF_COMP_IMAGE) 
   {
      image->filter = HPDF_STREAM_FILTER_FLATE_DECODE;
   }

   return image;
}

HPDF_EXPORT(HPDF_Image)
   HPDF_LoadJpegImageFromFile(
      HpdfDoc * const doc,
      char const *filename)
{
   HPDF_Stream imagedata;
   HPDF_Image image;

   HPDF_PTRACE((" HPDF_LoadJpegImageFromFile\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   /* create file stream */
   imagedata = HPDF_FileReader_New(doc->mmgr, filename);

   if (HPDF_Stream_Validate(imagedata))
   {
      image = HPDF_Image_LoadJpegImage(doc->mmgr, imagedata, doc->xref);
   }
   else
   {
      image = NULL;
   }

   /* destroy file stream */
   HPDF_Stream_Free(imagedata);

   if (!image)
   {
      HPDF_CheckError(&doc->error);
   }

   return image;
}

#if defined(WIN32)
HPDF_EXPORT(HPDF_Image)
   HPDF_LoadJpegImageFromFileW(
      HpdfDoc * const doc,
      wchar_t const  *filename)
{
   HPDF_Stream imagedata;
   HPDF_Image image;

   HPDF_PTRACE((" HPDF_LoadJpegImageFromFileW\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   /* create file stream */
   imagedata = HPDF_FileReader_NewW(doc->mmgr, filename);

   if (HPDF_Stream_Validate(imagedata))
   {
      image = HPDF_Image_LoadJpegImage(doc->mmgr, imagedata, doc->xref);
   }
   else
   {
      image = NULL;
   }

   /* destroy file stream */
   HPDF_Stream_Free(imagedata);

   if (!image)
   {
      HPDF_CheckError(&doc->error);
   }

   return image;
}
#endif

HPDF_EXPORT(HPDF_Image)
   HPDF_LoadJpegImageFromMem(
      HpdfDoc * const doc,
      HpdfByte   const * const buffer,
      HpdfUInt    size)
{
   HPDF_Image image;

   HPDF_PTRACE((" HPDF_LoadJpegImageFromMem\n"));

   if (!HPDF_HasDoc(doc)) 
   {
      return NULL;
   }

   image = HPDF_Image_LoadJpegImageFromMem(doc->mmgr, buffer, size, doc->xref);

   if (!image) 
   {
      HPDF_CheckError(&doc->error);
   }

   return image;
}

/*----- Catalog ------------------------------------------------------------*/

HPDF_EXPORT(HPDF_PageLayout)
   HPDF_GetPageLayout(
      HpdfDoc * const doc)
{
   HPDF_PTRACE((" HPDF_GetPageLayout\n"));

   if (!HPDF_HasDoc(doc))
   {
      return HPDF_PAGE_LAYOUT_SINGLE;
   }

   return HPDF_Catalog_GetPageLayout(doc->catalog);
}

HPDF_EXPORT(HpdfStatus)
   HPDF_SetPageLayout(
      HpdfDoc * const doc,
      HPDF_PageLayout   layout)
{
   HpdfStatus ret;

   HPDF_PTRACE((" HPDF_GetPageLayout\n"));

   if (!HPDF_HasDoc(doc))
   {
      return HPDF_INVALID_DOCUMENT;
   }

   if (layout < 0 || layout >= HPDF_PAGE_LAYOUT_EOF)
   {
      return HPDF_RaiseError(&doc->error, HPDF_PAGE_LAYOUT_OUT_OF_RANGE, (HpdfStatus)layout);
   }

   if ((layout == HPDF_PAGE_LAYOUT_TWO_PAGE_LEFT || 
        layout == HPDF_PAGE_LAYOUT_TWO_PAGE_RIGHT) && 
       doc->pdf_version < HPDF_VER_15)
   {
      doc->pdf_version = HPDF_VER_15;
   }

   ret = HPDF_Catalog_SetPageLayout(doc->catalog, layout);
   if (ret != HPDF_OK)
   {
      HPDF_CheckError(&doc->error);
   }

   return HPDF_OK;
}

HPDF_EXPORT(HPDF_PageMode)
   HPDF_GetPageMode(
      HpdfDoc * const doc)
{
   HPDF_PTRACE((" HPDF_GetPageMode\n"));

   if (!HPDF_HasDoc(doc))
   {
      return HPDF_PAGE_MODE_USE_NONE;
   }

   return HPDF_Catalog_GetPageMode(doc->catalog);
}

HPDF_EXPORT(HpdfStatus)
   HPDF_SetPageMode(
      HpdfDoc * const doc,
      HPDF_PageMode   mode)
{
   HpdfStatus ret;

   HPDF_PTRACE((" HPDF_GetPageMode\n"));

   if (!HPDF_HasDoc(doc))
   {
      return HPDF_INVALID_DOCUMENT;
   }

   if (mode < 0 || mode >= HPDF_PAGE_MODE_EOF)
   {
      return HPDF_RaiseError(&doc->error, HPDF_PAGE_MODE_OUT_OF_RANGE, (HpdfStatus)mode);
   }

   ret = HPDF_Catalog_SetPageMode(doc->catalog, mode);
   if (ret != HPDF_OK)
   {
      return HPDF_CheckError(&doc->error);
   }

   return HPDF_OK;
}

HPDF_EXPORT(HpdfStatus)
   HPDF_SetOpenAction(
      HpdfDoc * const doc,
      HpdfDestination * const open_action)
{
   HpdfStatus ret;

   HPDF_PTRACE((" HPDF_SetOpenAction\n"));

   if (!HPDF_HasDoc(doc))
   {
      return HPDF_INVALID_DOCUMENT;
   }

   if (open_action && !HPDF_Destination_Validate(open_action))
   {
      return HPDF_RaiseError(&doc->error, HPDF_INVALID_DESTINATION, 0);
   }

   ret = HPDF_Catalog_SetOpenAction(doc->catalog, open_action);
   if (ret != HPDF_OK)
   {
      return HPDF_CheckError(&doc->error);
   }

   return HPDF_OK;
}

HPDF_EXPORT(HpdfUInt)
   HPDF_GetViewerPreference(
      HpdfDoc * const doc)
{
   HPDF_PTRACE((" HPDF_Catalog_GetViewerPreference\n"));

   if (!HPDF_HasDoc(doc))
   {
      return 0;
   }

   return HPDF_Catalog_GetViewerPreference(doc->catalog);
}

HPDF_EXPORT(HpdfStatus)
   HPDF_SetViewerPreference(
      HpdfDoc * const doc,
      HpdfUInt    value)
{
   HpdfStatus ret;

   HPDF_PTRACE((" HPDF_Catalog_SetViewerPreference\n"));

   if (!HPDF_HasDoc(doc))
   {
      return HPDF_INVALID_DOCUMENT;
   }

   ret = HPDF_Catalog_SetViewerPreference(doc->catalog, value);
   if (ret != HPDF_OK)
   {
      return HPDF_CheckError(&doc->error);
   }

   doc->pdf_version = HPDF_VER_16;

   return HPDF_OK;
}

HPDF_EXPORT(HpdfStatus)
   HPDF_AddPageLabel(
      HpdfDoc * const doc,
      HpdfUInt            page_num,
      HPDF_PageNumStyle    style,
      HpdfUInt            first_page,
      char const    *prefix)
{
   HPDF_Dict page_label;
   HpdfStatus ret;

   HPDF_PTRACE((" HPDF_AddPageLabel\n"));

   if (!HPDF_HasDoc(doc))
   {
      return HPDF_INVALID_DOCUMENT;
   }

   page_label = HPDF_PageLabel_New(doc, style, first_page, prefix);

   if (!page_label)
   {
      return HPDF_CheckError(&doc->error);
   }

   if (style < 0 || style >= HPDF_PAGE_NUM_STYLE_EOF)
   {
      return HPDF_RaiseError(&doc->error, HPDF_PAGE_NUM_STYLE_OUT_OF_RANGE, (HpdfStatus) style);
   }

   ret = HPDF_Catalog_AddPageLabel(doc->catalog, page_num, page_label);
   if (ret != HPDF_OK)
   {
      return HPDF_CheckError(&doc->error);
   }

   return HPDF_OK;
}

HPDF_EXPORT(HPDF_EmbeddedFile)
   HPDF_AttachFile(
      HpdfDoc * const doc,
      char const     *file)
{
   HpdfValueNameDict  names;
   HpdfValueNameTree  ntree;
   HPDF_EmbeddedFile  efile;
   HpdfValueString   *name;
   HpdfStatus         ret = HPDF_OK;

   HPDF_PTRACE((" HPDF_AttachFile\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   names = HPDF_Catalog_GetNames(doc->catalog);
   if (!names) 
   {
      names = HpdfValueNameDict_New(doc->mmgr, doc->xref);
      if (!names)
      {
         return NULL;
      }

      ret = HPDF_Catalog_SetNames(doc->catalog, names);
      if (ret != HPDF_OK)
      {
         return NULL;
      }
   }

   ntree = HpdfValueNameDict_GetNameTree(names, HPDF_NAME_EMBEDDED_FILES);
   if (!ntree) 
   {
      ntree = HpdfValueNameTree_New(doc->mmgr, doc->xref);
      if (!ntree)
      {
         return NULL;
      }

      ret = HpdfValueNameDict_SetNameTree(names, HPDF_NAME_EMBEDDED_FILES, ntree);
      if (ret != HPDF_OK)
      {
         return NULL;
      }
   }

   efile = HPDF_EmbeddedFile_New(doc->mmgr, doc->xref, file);
   if (!efile)
   {
      return NULL;
   }

   name = HpdfValueStringCreate(doc->mmgr, file, NULL);
   if (!name)
   {
      return NULL;
   }

   ret += HpdfValueNameTree_Add(ntree, name, efile);
   if (ret != HPDF_OK)
   {
      return NULL;
   }

   return efile;
}

#if defined(WIN32)
HPDF_EXPORT(HPDF_EmbeddedFile)
   HPDF_AttachFileW(
      HpdfDoc * const doc,
      wchar_t const  *file)
{
   HpdfValueNameDict  names;
   HpdfValueNameTree  ntree;
   HPDF_EmbeddedFile  efile;
   HpdfValueString   *name;
   HpdfStatus         ret = HPDF_OK;

   HPDF_PTRACE((" HPDF_AttachFileW\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   names = HPDF_Catalog_GetNames(doc->catalog);
   if (!names) 
   {
      names = HpdfValueNameDict_New(doc->mmgr, doc->xref);
      if (!names)
      {
         return NULL;
      }

      ret = HPDF_Catalog_SetNames(doc->catalog, names);
      if (ret != HPDF_OK)
      {
         return NULL;
      }
   }

   ntree = HpdfValueNameDict_GetNameTree(names, HPDF_NAME_EMBEDDED_FILES);
   if (!ntree) 
   {
      ntree = HpdfValueNameTree_New(doc->mmgr, doc->xref);
      if (!ntree)
      {
         return NULL;
      }

      ret = HpdfValueNameDict_SetNameTree(names, HPDF_NAME_EMBEDDED_FILES, ntree);
      if (ret != HPDF_OK)
      {
         return NULL;
      }
   }

   efile = HPDF_EmbeddedFile_NewW(doc->mmgr, doc->xref, file);
   if (!efile)
   {
      return NULL;
   }

   name = HpdfValueStringCreateW(doc->mmgr, file, NULL);
   if (!name)
   {
      return NULL;
   }

   ret += HpdfValueNameTree_Add(ntree, name, efile);
   if (ret != HPDF_OK)
   {
      return NULL;
   }

   return efile;
}
#endif

/*----- Info ---------------------------------------------------------------*/

static HPDF_Dict
   _GetInfo(
      HpdfDoc * const doc)
{
   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   if (!doc->info) 
   {
      doc->info = HPDF_Dict_New(doc->mmgr);

      if (!doc->info || 
          HPDF_Xref_Add(doc->xref, doc->info) != HPDF_OK)
      {
         doc->info = NULL;
      }
   }

   return doc->info;
}

HPDF_EXPORT(HpdfStatus)
   HPDF_SetInfoAttr(
      HpdfDoc * const doc,
      HPDF_InfoType     type,
      char const *value)
{
   HpdfStatus ret;
   HPDF_Dict info = _GetInfo(doc);

   HPDF_PTRACE((" HPDF_SetInfoAttr\n"));

   if (!info)
   {
      return HPDF_CheckError(&doc->error);
   }

   ret = HPDF_Info_SetInfoAttr(info, type, value, doc->cur_encoder);
   if (ret != HPDF_OK)
   {
      return HPDF_CheckError(&doc->error);
   }

   return ret;
}

HPDF_EXPORT(char const*)
   HPDF_GetInfoAttr(
      HpdfDoc * const doc,
      HPDF_InfoType   type)
{
   char const *ret = NULL;
   HPDF_Dict info = _GetInfo(doc);

   HPDF_PTRACE((" HPDF_GetInfoAttr\n"));

   if (info)
   {
      ret = HPDF_Info_GetInfoAttr(info, type);
   }
   else
   {
      HPDF_CheckError(&doc->error);
   }

   return ret;
}

HPDF_EXPORT(HpdfStatus)
   HPDF_SetInfoDateAttr(
      HpdfDoc * const doc,
      HPDF_InfoType   type,
      HPDF_Date       value)
{
   HpdfStatus ret;
   HPDF_Dict info = _GetInfo(doc);

   HPDF_PTRACE((" HPDF_SetInfoDateAttr\n"));

   if (!info)
   {
      return HPDF_CheckError(&doc->error);
   }

   ret = HPDF_Info_SetInfoDateAttr(info, type, value);
   if (ret != HPDF_OK)
   {
      return HPDF_CheckError(&doc->error);
   }

   return ret;
}

HPDF_EXPORT(HPDF_Outline)
   HPDF_CreateOutline(
      HpdfDoc * const doc,
      HPDF_Outline   parent,
      char const   *title,
      HpdfEncoder * const encoder)
{
   HPDF_Outline outline;

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   if (!parent) 
   {
      if (doc->outlines) 
      {
         parent = doc->outlines;
      }
      else 
      {
         doc->outlines = HPDF_OutlineRoot_New(doc->mmgr, doc->xref);

         if (doc->outlines) 
         {
            HpdfStatus ret = HPDF_Dict_Add(doc->catalog, "Outlines", doc->outlines);
            if (ret != HPDF_OK) 
            {
               HPDF_CheckError(&doc->error);
               doc->outlines = NULL;
               return NULL;
            }

            parent = doc->outlines;
         }
         else 
         {
            HPDF_CheckError(&doc->error);
            return NULL;
         }
      }
   }

   if (!HPDF_Outline_Validate(parent) || 
       doc->mmgr != parent->mmgr) 
   {
      HPDF_RaiseError(&doc->error, HPDF_INVALID_OUTLINE, 0);
      return NULL;
   }

   outline = HPDF_Outline_New(doc->mmgr, parent, title, encoder, doc->xref);
   if (!outline)
   {
      HPDF_CheckError(&doc->error);
   }

   return outline;
}

HPDF_EXPORT(HPDF_ExtGState)
   HPDF_CreateExtGState(
      HpdfDoc * const doc)
{
   HPDF_ExtGState ext_gstate;

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   doc->pdf_version = HPDF_VER_14;

   ext_gstate = HPDF_ExtGState_New(doc->mmgr, doc->xref);
   if (!ext_gstate)
   {
      HPDF_CheckError(&doc->error);
   }

   return ext_gstate;
}

HPDF_EXPORT(HpdfStatus)
   HPDF_SetCompressionMode(
      HpdfDoc * const doc,
      HpdfUInt   mode)
{
   if (!HPDF_Doc_Validate(doc))
   {
      return HPDF_INVALID_DOCUMENT;
   }

   if (mode != (mode & HPDF_COMP_MASK))
   {
      return HPDF_RaiseError(&doc->error, HPDF_INVALID_COMPRESSION_MODE, 0);
   }

#ifdef LIBHPDF_HAVE_ZLIB
   doc->compression_mode = mode;

   return HPDF_OK;

#else /* LIBHPDF_HAVE_ZLIB */

   return HPDF_INVALID_COMPRESSION_MODE;

#endif /* LIBHPDF_HAVE_ZLIB */
}

HPDF_EXPORT(HpdfStatus)
   HPDF_GetError(
      HpdfDoc const * const doc)
{
   if (!HPDF_Doc_Validate(doc))
   {
      return HPDF_INVALID_DOCUMENT;
   }

   return HPDF_Error_GetCode(&doc->error);
}

HPDF_EXPORT(HpdfStatus)
   HPDF_GetErrorDetail(
      HpdfDoc const * const doc)
{
   if (!HPDF_Doc_Validate(doc))
   {
      return HPDF_INVALID_DOCUMENT;
   }

   return HPDF_Error_GetDetailCode(&doc->error);
}

HPDF_EXPORT(void)
   HPDF_ResetError(
      HpdfDoc * const doc)
{
   if (!HPDF_Doc_Validate(doc))
   {
      return;
   }

   HPDF_Error_Reset(&doc->error);
}

//TODO exported function not in any .h files!
/* create an intent entry */
HPDF_EXPORT(HPDF_OutputIntent)
   HPDF_OutputIntent_New(
      HpdfDoc * const doc,
      char const* identifier,
      char const* condition,
      char const* registry,
      char const* info,
      HpdfArray * const outputprofile)
{
   HPDF_OutputIntent intent;
   HpdfStatus ret = HPDF_OK;

   HPDF_PTRACE((" HPDF_OutputIntent_New\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   intent = HPDF_Dict_New(doc->mmgr);
   if (!intent)
   {
      return NULL;
   }

   if (HPDF_Xref_Add(doc->xref, intent) != HPDF_OK) 
   {
      HPDF_Dict_Free(intent);
      return NULL;
   }

   ret += HPDF_Dict_AddName(intent, "Type", "OutputIntent");
   ret += HPDF_Dict_AddName(intent, "S",    "GTS_PDFX");
   ret += HPDF_Dict_Add(    
      intent, 
      "OutputConditionIdentifier", 
      HpdfValueStringCreate(doc->mmgr, identifier, NULL));
   ret += HPDF_Dict_Add(    
      intent, 
      "OutputCondition", 
      HpdfValueStringCreate(doc->mmgr, condition,  NULL));
   ret += HPDF_Dict_Add(    
      intent, 
      "RegistryName", 
      HpdfValueStringCreate(doc->mmgr, registry,   NULL));

   if (info != NULL) 
   {
      ret += HPDF_Dict_Add(intent, "Info", HpdfValueStringCreate(doc->mmgr, info, NULL));
   }

   /* add ICC base stream */
   if (outputprofile != NULL) 
   {
      ret += HPDF_Dict_Add(intent, "DestOutputProfile ", outputprofile);
   }

   if (ret != HPDF_OK) 
   {
      HPDF_Dict_Free(intent);
      return NULL;
   }

   return intent;
}

//TODO exported function not in any .h files!
HPDF_EXPORT(HpdfStatus)
   HPDF_AddIntent(
      HpdfDoc * const doc,
      HPDF_OutputIntent  intent)
{
   HpdfArray *intents;
   
   if (!HPDF_HasDoc(doc))
   {
      return HPDF_INVALID_DOCUMENT;
   }

   intents = HPDF_Dict_GetItem(doc->catalog, "OutputIntents", HPDF_OCLASS_ARRAY);
   if (intents == NULL) 
   {
      intents = HPDF_Array_New(doc->mmgr);
      if (intents) 
      {
         HpdfStatus ret = HPDF_Dict_Add(doc->catalog, "OutputIntents", intents);
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

/* "Perceptual", "RelativeColorimetric", "Saturation", "AbsoluteColorimetric" */
HPDF_EXPORT(HPDF_OutputIntent)
   HPDF_ICC_LoadIccFromMem(
      HpdfDoc * const doc,
      HpdfMemMgr * const mmgr,
      HPDF_Stream iccdata,
      HPDF_Xref   xref,
      int         numcomponent)
{
   HPDF_OutputIntent icc;
   HpdfStatus ret;

   HPDF_PTRACE((" HPDF_ICC_LoadIccFromMem\n"));

   icc = HPDF_DictStream_New(mmgr, xref);
   if (!icc)
   {
      return NULL;
   }

   HPDF_Dict_AddNumber(icc, "N", numcomponent);
   switch (numcomponent) 
   {
   case 1:
      HPDF_Dict_AddName(icc, "Alternate", "DeviceGray");
      break;
   
   case 3:
      HPDF_Dict_AddName(icc, "Alternate", "DeviceRGB");
      break;
   
   case 4:
      HPDF_Dict_AddName(icc, "Alternate", "DeviceCMYK");
      break;
   
   default: /* unsupported */
      HPDF_RaiseError(&doc->error, HPDF_INVALID_ICC_COMPONENT_NUM, 0);
      HPDF_Dict_Free(icc);
      return NULL;
   }

   for (;;) 
   {
      HpdfByte buf[HPDF_STREAM_BUF_SIZ];
      HpdfUInt len = HPDF_STREAM_BUF_SIZ;
      ret = HPDF_Stream_Read(iccdata, buf, &len);

      if (ret != HPDF_OK) 
      {
         if (ret == HPDF_STREAM_EOF) 
         {
            if (len > 0) 
            {
               ret = HPDF_Stream_Write(icc->stream, buf, len);
               if (ret != HPDF_OK) 
               {
                  HPDF_Dict_Free(icc);
                  return NULL;
               }
            }
            break;
         }
         else 
         {
            HPDF_Dict_Free(icc);
            return NULL;
         }
      }

      if (HPDF_Stream_Write(icc->stream, buf, len) != HPDF_OK) 
      {
         HPDF_Dict_Free(icc);
         return NULL;
      }
   }

   return icc;
}

//TODO exported function not in any .h files!
HPDF_EXPORT(HpdfArray *)
   HPDF_AddColorspaceFromProfile(
      HpdfDoc * const doc,
      HPDF_Dict icc)
{
   HpdfStatus ret = HPDF_OK;
   HpdfArray *iccentry;

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   iccentry = HPDF_Array_New(doc->mmgr);
   if (!iccentry)
   {
      return NULL;
   }

   ret = HPDF_Array_AddName(iccentry, "ICCBased");
   if (ret != HPDF_OK) 
   {
      HPDF_Array_Free(iccentry);
      HPDF_CheckError(&doc->error);
      return NULL;
   }

   ret = HPDF_Array_Add(iccentry, icc);
   if (ret != HPDF_OK) 
   {
      HPDF_Array_Free(iccentry);
      return NULL;
   }
   return iccentry;
}

HPDF_EXPORT(HPDF_OutputIntent)
   HPDF_LoadIccProfileFromFile(
      HpdfDoc * const doc,
      char const* icc_file_name,
      int numcomponent)
{
   HPDF_Stream iccdata;
   HPDF_OutputIntent iccentry;

   HPDF_PTRACE((" HPDF_LoadIccProfileFromFile\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   /* create file stream */
   iccdata = HPDF_FileReader_New(doc->mmgr, icc_file_name);

   if (HPDF_Stream_Validate(iccdata)) 
   {
      iccentry = HPDF_ICC_LoadIccFromMem(doc, doc->mmgr, iccdata, doc->xref, numcomponent);
   }
   else
   {
      iccentry = NULL;
   }

   /* destroy file stream */
   if (iccdata)
   {
      HPDF_Stream_Free(iccdata);
   }

   if (!iccentry)
   {
      HPDF_CheckError(&doc->error);
   }

   return iccentry;
}

#if defined(WIN32)
HPDF_EXPORT(HPDF_OutputIntent)
   HPDF_LoadIccProfileFromFileW(
      HpdfDoc * const doc,
      wchar_t const  *icc_file_name,
      int numcomponent)
{
   HPDF_Stream iccdata;
   HPDF_OutputIntent iccentry;

   HPDF_PTRACE((" HPDF_LoadIccProfileFromFileW\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   /* create file stream */
   iccdata = HPDF_FileReader_NewW(doc->mmgr, icc_file_name);

   if (HPDF_Stream_Validate(iccdata)) 
   {
      iccentry = HPDF_ICC_LoadIccFromMem(doc, doc->mmgr, iccdata, doc->xref, numcomponent);
   }
   else
   {
      iccentry = NULL;
   }

   /* destroy file stream */
   if (iccdata)
   {
      HPDF_Stream_Free(iccdata);
   }

   if (!iccentry)
   {
      HPDF_CheckError(&doc->error);
   }

   return iccentry;
}
#endif
