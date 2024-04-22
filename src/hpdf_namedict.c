/*
 * << Haru Free PDF Library >> -- hpdf_namedict.c
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
#include "hpdf_consts.h"
#include "hpdf_namedict.h"

#ifndef HPDF_UNUSED
#define HPDF_UNUSED(a) ((void)(a))
#endif

static char const * const HPDF_NAMEDICT_KEYS[] = {
                                        "EmbeddedFiles"
};

HpdfValueNameDict
   HpdfValueNameDict_New(
      HpdfMemMgr * const mmgr,
      HPDF_Xref  xref)
{
   HpdfValueNameDict ndict;

   HPDF_PTRACE((" HpdfValueNameDict_New\n"));

   ndict = HPDF_Dict_New(mmgr);
   if (!ndict)
      return NULL;

   if (HPDF_Xref_Add(xref, ndict) != HPDF_OK)
      return NULL;

   ndict->header.obj_class |= HPDF_OSUBCLASS_NAMEDICT;

   return ndict;
}

HpdfValueNameTree
HpdfValueNameDict_GetNameTree(HpdfValueNameDict     namedict,
   HpdfValueNameDictKey  key)
{
   if (!namedict)
      return NULL;
   return HPDF_Dict_GetItem(namedict, HPDF_NAMEDICT_KEYS[key], HPDF_OCLASS_DICT);
}

HpdfStatus
HpdfValueNameDict_SetNameTree(HpdfValueNameDict     namedict,
   HpdfValueNameDictKey  key,
   HpdfValueNameTree     ntree)
{
   return HPDF_Dict_Add(namedict, HPDF_NAMEDICT_KEYS[key], ntree);
}

HpdfBool
HpdfValueNameDict_Validate(HpdfValueNameDict  namedict)
{
   if (!namedict)
      return HPDF_FALSE;

   if (namedict->header.obj_class != (HPDF_OSUBCLASS_NAMEDICT |
      HPDF_OCLASS_DICT)) {
      HPDF_SetError(namedict->error, HPDF_INVALID_OBJECT, 0);
      return HPDF_FALSE;
   }

   return HPDF_TRUE;
}


/*------- NameTree -------*/

HpdfValueNameTree
   HpdfValueNameTree_New(
      HpdfMemMgr * const mmgr,
      HPDF_Xref  xref)
{
   HpdfStatus ret = HPDF_OK;
   HpdfValueNameTree ntree;
   HpdfArray *items;

   HPDF_PTRACE((" HpdfValueNameTree_New\n"));

   ntree = HPDF_Dict_New(mmgr);
   if (!ntree)
      return NULL;

   if (HPDF_Xref_Add(xref, ntree) != HPDF_OK)
      return NULL;

   ntree->header.obj_class |= HPDF_OSUBCLASS_NAMETREE;

   items = HPDF_Array_New(mmgr);
   if (!ntree)
      return NULL;

   ret += HPDF_Dict_Add(ntree, "Names", items);
   if (ret != HPDF_OK)
      return NULL;

   return ntree;
}

HpdfStatus
   HpdfValueNameTree_Add(
      HpdfValueNameTree        tree,
      HpdfValueString * const  name,
      void                    *obj)
{
   HpdfArray   *items;
   HpdfInt32    i, 
                icount;

   if (!tree ||
       !name)
   {
      return HPDF_INVALID_PARAMETER;
   }

   items = HPDF_Dict_GetItem(tree, "Names", HPDF_OCLASS_ARRAY);
   if (!items)
   {
      return HPDF_INVALID_OBJECT;
   }

   /* "The keys shall be sorted in lexical order" -- 7.9.6, Name Trees.
   ** Since we store keys sorted, it's best to do a linear insertion sort
   ** Find the first element larger than 'key', and insert 'key' and then
   ** 'obj' into the items. */
   icount = HPDF_Array_Items(items);

   for (i = 0; i < icount; i += 2) 
   {
      HpdfValueString *elem = HPDF_Array_GetItem(items, i, HPDF_OCLASS_STRING);
      if (HpdfValueStringCmp(name, elem) < 0) 
      {
         HPDF_Array_Insert(items, elem, name);
         HPDF_Array_Insert(items, elem, obj);
         return HPDF_OK;
      }
   }

   /* Items list is empty */
   HPDF_Array_Add(items, name);
   HPDF_Array_Add(items, obj);
   return HPDF_OK;
}

HpdfBool
HpdfValueNameTree_Validate(HpdfValueNameTree  nametree)
{
   if (!nametree)
      return HPDF_FALSE;

   if (nametree->header.obj_class != (HPDF_OSUBCLASS_NAMETREE |
      HPDF_OCLASS_DICT)) {
      HPDF_SetError(nametree->error, HPDF_INVALID_OBJECT, 0);
      return HPDF_FALSE;
   }

   return HPDF_TRUE;
}


/*------- EmbeddedFile -------*/

HPDF_EmbeddedFile
   HPDF_EmbeddedFile_New(
      HpdfMemMgr * const mmgr,
      HPDF_Xref          xref,
      char const        *file)
{
   HpdfStatus         ret = HPDF_OK;
   HPDF_Dict          ef;            /* the dictionary for the embedded file: /Type /EF */
   HpdfValueString   *name;          /* the name of the file: /F (name) */
   HPDF_Dict          eff;           /* ef has an /EF <<blah>> key - this is it */
   HPDF_Dict          filestream;    /* the stream that /EF <</F _ _ R>> refers to */
   HPDF_Stream        stream;

   ef = HPDF_Dict_New(mmgr);
   if (!ef)
   {
      return NULL;
   }

   if (HPDF_Xref_Add(xref, ef) != HPDF_OK)
   {
      return NULL;
   }

   filestream = HPDF_DictStream_New(mmgr, xref);
   if (!filestream)
   {
      return NULL;
   }

   stream = HPDF_FileReader_New(mmgr, file);
   if (!stream)
   {
      return NULL;
   }

   HPDF_Stream_Free(filestream->stream);
   filestream->stream = stream;
   filestream->filter = HPDF_STREAM_FILTER_FLATE_DECODE;

   eff = HPDF_Dict_New(mmgr);
   if (!eff)
   {
      return NULL;
   }

   name = HpdfValueStringCreate(mmgr, file, NULL);
   if (!name)
   {
      return NULL;
   }

   ret += HPDF_Dict_AddName(ef, "Type", "F");
   ret += HPDF_Dict_Add(ef, "F", name);
   ret += HPDF_Dict_Add(ef, "EF", eff);
   ret += HPDF_Dict_Add(eff, "F", filestream);

   if (ret != HPDF_OK)
   {
      return NULL;
   }

   return ef;
}

#if defined(WIN32)
HPDF_EmbeddedFile
   HPDF_EmbeddedFile_NewW(
      HpdfMemMgr * const mmgr,
      HPDF_Xref          xref,
      wchar_t const     *file)
{
   HpdfStatus         ret = HPDF_OK;
   HPDF_Dict          ef;           /* the dictionary for the embedded file: /Type /EF */
   HpdfValueString   *name;         /* the name of the file: /F (name) */
   HPDF_Dict          eff;          /* ef has an /EF <<blah>> key - this is it */
   HPDF_Dict          filestream;   /* the stream that /EF <</F _ _ R>> refers to */
   HPDF_Stream        stream;

   ef = HPDF_Dict_New(mmgr);
   if (!ef)
   {
      return NULL;
   }

   if (HPDF_Xref_Add(xref, ef) != HPDF_OK)
   {
      return NULL;
   }

   filestream = HPDF_DictStream_New(mmgr, xref);
   if (!filestream)
   {
      return NULL;
   }

   stream = HPDF_FileReader_NewW(mmgr, file);
   if (!stream)
   {
      return NULL;
   }

   HPDF_Stream_Free(filestream->stream);
   filestream->stream = stream;
   filestream->filter = HPDF_STREAM_FILTER_FLATE_DECODE;

   eff = HPDF_Dict_New(mmgr);
   if (!eff)
   {
      return NULL;
   }

   name = HpdfValueStringCreateW(mmgr, file, NULL);
   if (!name)
   {
      return NULL;
   }

   ret += HPDF_Dict_AddName(ef,  "Type", "F");
   ret += HPDF_Dict_Add(    ef,  "F",    name);
   ret += HPDF_Dict_Add(    ef,  "EF",   eff);
   ret += HPDF_Dict_Add(    eff, "F",    filestream);

   if (ret != HPDF_OK)
   {
      return NULL;
   }

   return ef;
}
#endif

HpdfBool
HPDF_EmbeddedFile_Validate(HPDF_EmbeddedFile  emfile)
{
   HPDF_UNUSED(emfile);
   return HPDF_TRUE;
}
