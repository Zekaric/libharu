/*
* << Haru Free PDF Library >> -- hpdf_objects.c
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

#ifndef _HPDF_OBJECTS_H
#define _HPDF_OBJECTS_H

#include "hpdf_encoder.h"

#ifdef __cplusplus
extern "C" {
#endif


/* if HPDF_OTYPE_DIRECT bit is set, the object owned by other container
   * object. if HPDF_OTYPE_INDIRECT bit is set, the object managed by xref.
   */

#define  HPDF_OTYPE_NONE              0x00000000
#define  HPDF_OTYPE_DIRECT            0x80000000
#define  HPDF_OTYPE_INDIRECT          0x40000000
#define  HPDF_OTYPE_ANY               (HPDF_OTYPE_DIRECT | HPDF_OTYPE_INDIRECT)
#define  HPDF_OTYPE_HIDDEN            0x10000000

#define  HPDF_OCLASS_UNKNOWN          0x0001
#define  HPDF_OCLASS_NULL             0x0002
#define  HPDF_OCLASS_BOOLEAN          0x0003
#define  HPDF_OCLASS_NUMBER           0x0004
#define  HPDF_OCLASS_REAL             0x0005
#define  HPDF_OCLASS_NAME             0x0006
#define  HPDF_OCLASS_STRING           0x0007
#define  HPDF_OCLASS_BINARY           0x0008
#define  HPDF_OCLASS_ARRAY            0x0010
#define  HPDF_OCLASS_DICT             0x0011
#define  HPDF_OCLASS_PROXY            0x0012
#define  HPDF_OCLASS_DIRECT           0x00A0
#define  HPDF_OCLASS_ANY              0x00FF

#define  HPDF_OSUBCLASS_FONT          0x0100
#define  HPDF_OSUBCLASS_CATALOG       0x0200
#define  HPDF_OSUBCLASS_PAGES         0x0300
#define  HPDF_OSUBCLASS_PAGE          0x0400
#define  HPDF_OSUBCLASS_XOBJECT       0x0500
#define  HPDF_OSUBCLASS_OUTLINE       0x0600
#define  HPDF_OSUBCLASS_DESTINATION   0x0700
#define  HPDF_OSUBCLASS_ANNOTATION    0x0800
#define  HPDF_OSUBCLASS_ENCRYPT       0x0900
#define  HPDF_OSUBCLASS_EXT_GSTATE    0x0A00
#define  HPDF_OSUBCLASS_EXT_GSTATE_R  0x0B00  /* read only object */
#define  HPDF_OSUBCLASS_NAMEDICT      0x0C00
#define  HPDF_OSUBCLASS_NAMETREE      0x0D00
#define  HPDF_OSUBCLASS_SHADING       0x0E00



   /*----------------------------------------------------------------------------*/
   /*------ Values related xref -------------------------------------------------*/

#define HPDF_FREE_ENTRY             'f'
#define HPDF_IN_USE_ENTRY           'n'


/*
*  structure of Object-ID
*
*  1       direct-object
*  2       indirect-object
*  3       reserved
*  4       shadow-object
*  5-8     reserved
*  9-32    object-id
*
*  the real Object-ID is described "obj_id & 0x00FFFFFF"
*/

typedef struct _HpdfObjHeader HpdfObjHeader;

struct _HpdfObjHeader {
   HpdfUInt32  obj_id;
   HpdfUInt16  gen_no;
   HpdfUInt16  obj_class;
};

HpdfStatus HPDF_Obj_WriteValue(void          *obj, HPDF_Stream   stream, HPDF_Encrypt  e);
HpdfStatus HPDF_Obj_Write(void          *obj, HPDF_Stream   stream, HPDF_Encrypt  e);
void HPDF_Obj_Free(HpdfMemMgr * const mmgr, void         *obj);
void HPDF_Obj_ForceFree(HpdfMemMgr * const mmgr, void         *obj);

/*---------------------------------------------------------------------------*/
/*----- HpdfObjNull -------------------------------------------------------*/

typedef struct _HpdfObjNull HpdfObjNull;

struct _HpdfObjNull
{
   HpdfObjHeader header;
};

HpdfObjNull     *HpdfObjNullCreate(   HpdfMemMgr * const mmgr);

/*---------------------------------------------------------------------------*/
/*----- HpdfObjBool -------------------------------------------------------*/

typedef struct _HpdfObjBool HpdfObjBool;

struct _HpdfObjBool
{
   HpdfObjHeader header;
   HpdfBool        value;
};

HpdfObjBool     *HpdfObjBoolCreate(   HpdfMemMgr * const mmgr, HpdfBool value);
HpdfStatus         HpdfObjBoolWrite(    HpdfObjBool const * const obj, HPDF_Stream stream);

/*---------------------------------------------------------------------------*/
/*----- HpdfObjNumInt -----------------------------------------------------*/

typedef struct _HpdfObjNumInt HpdfObjNumInt;

struct _HpdfObjNumInt
{
   HpdfObjHeader header;
   HpdfInt32       value;
};

HpdfObjNumInt   *HpdfObjNumIntCreate( HpdfMemMgr * const mmgr, HpdfInt32 value);
void               HpdfObjNumIntSet(    HpdfObjNumInt       * const obj, HpdfInt32 value);
HpdfStatus         HpdfObjNumIntWrite(  HpdfObjNumInt const * const obj, HPDF_Stream stream);

/*---------------------------------------------------------------------------*/
/*----- HpdfObjNumReal ----------------------------------------------------*/

typedef struct _HpdfObjNumReal HpdfObjNumReal;

struct _HpdfObjNumReal
{
   HpdfObjHeader header;
   HpdfError      *error;
   HpdfReal        value;
};

HpdfObjNumReal  *HpdfObjNumRealCreate(HpdfMemMgr * const mmgr, HpdfReal value);
HpdfStatus         HpdfObjNumRealSet(   HpdfObjNumReal       * const obj, HpdfReal value);
HpdfStatus         HpdfObjNumRealWrite( HpdfObjNumReal const * const obj, HPDF_Stream stream);

/*---------------------------------------------------------------------------*/
/*----- HpdfObjName -------------------------------------------------------*/

typedef struct _HpdfObjName HpdfObjName;

struct _HpdfObjName
{
   HpdfObjHeader header;
   HpdfError      *error;
   char            value[HPDF_LIMIT_MAX_NAME_LEN + 1];
};

HpdfObjName     *HpdfObjNameCreate(   HpdfMemMgr * const mmgr, char const *value);
char const        *HpdfObjNameGet(      HpdfObjName const * const obj);
HpdfStatus         HpdfObjNameSet(      HpdfObjName       * const obj, char const *value);
HpdfStatus         HpdfObjNameWrite(    HpdfObjName const * const obj, HPDF_Stream  stream);

/*---------------------------------------------------------------------------*/
/*----- HpdfObjString -----------------------------------------------------*/

typedef struct _HpdfObjString HpdfObjString;

struct _HpdfObjString
{
   HpdfObjHeader    header;
   HpdfMemMgr        *mmgr;
   HpdfError         *error;
   HpdfEncoder       *encoder;
   HpdfByte          *value;
   HpdfUInt           len;
};

HpdfObjString   *HpdfObjStringCreate(    HpdfMemMgr * const mmgr, char const    *value, HpdfEncoder * const encoder);
#if defined(WIN32)
HpdfObjString   *HpdfObjStringCreateW(   HpdfMemMgr * const mmgr, wchar_t const *value, HpdfEncoder * const encoder);
#endif
HpdfStatus       HpdfObjStringSet(       HpdfObjString       * const obj,char const *value);
void             HpdfObjStringDestroy(   HpdfObjString       * const obj);
HpdfStatus       HpdfObjStringWrite(     HpdfObjString const * const obj, HPDF_Stream stream, HPDF_Encrypt e);
HpdfInt32        HpdfObjStringCmp(       HpdfObjString const * const s1, HpdfObjString const * const s2);

/*---------------------------------------------------------------------------*/
/*----- HpdfObjBinary ----------------------------------------------------------*/

typedef struct _HpdfObjBinary HpdfObjBinary;

struct _HpdfObjBinary 
{
   HpdfObjHeader    header;
   HpdfMemMgr           *mmgr;
   HpdfError         *error;
   HpdfByte          *value;
   HpdfUInt           len;
};

HpdfObjBinary  *HpdfObjBinaryCreate(   HpdfMemMgr * const mmgr, HpdfByte   * const value, HpdfUInt  len);
HpdfStatus      HpdfObjBinarySet(      HpdfObjBinary       * const obj, HpdfByte     * const value, HpdfUInt    len);
HpdfByte       *HpdfObjBinaryGet(      HpdfObjBinary const * const obj);
void            HpdfObjBinaryDestroy(  HpdfObjBinary       * const obj);
HpdfStatus      HpdfObjBinaryWrite(    HpdfObjBinary const * const obj, HPDF_Stream  stream, HPDF_Encrypt e);
HpdfUInt        HpdfObjBinaryGetLength(HpdfObjBinary const * const obj);

/*---------------------------------------------------------------------------*/
/*----- HPDF_Array ----------------------------------------------------------*/

typedef struct _HpdfArray HpdfArray;

struct _HpdfArray 
{
   HpdfObjHeader    header;
   HpdfMemMgr        *mmgr;
   HpdfError         *error;
   HpdfList          *list;
};

HpdfStatus   HpdfArrayAdd(       HpdfArray * const array, void       *obj);
HpdfStatus   HpdfArrayAddName(   HpdfArray * const array, char const *value);
HpdfStatus   HpdfArrayAddNull(   HpdfArray * const array);
HpdfStatus   HpdfArrayAddNumber( HpdfArray * const array, HpdfInt32   value);
HpdfStatus   HpdfArrayAddReal(   HpdfArray * const array, HpdfReal    value);

void         HpdfArrayClear(     HpdfArray * const array);
HpdfArray   *HpdfArrayCreate(    HpdfMemMgr * const mmgr);

void         HpdfArrayDestroy(   HpdfArray * const array);

HpdfUInt     HpdfArrayGetCount(  HpdfArray const * const array);
void        *HpdfArrayGetItem(   HpdfArray * const array, HpdfUInt index, HpdfUInt16  obj_class);

HpdfStatus   HpdfArrayInsert(    HpdfArray * const array, void *target, void *obj);

HpdfStatus   HpdfArrayWrite(     HpdfArray const * const array, HPDF_Stream  stream, HPDF_Encrypt e);

HpdfArray   *HpdfBoxArrayCreate( HpdfMemMgr * const mmgr, HPDF_Box box);

/*---------------------------------------------------------------------------*/
/*----- HPDF_Dict -----------------------------------------------------------*/

typedef struct _HPDF_Xref_Rec *HPDF_Xref;

typedef struct _HPDF_Dict_Rec  *HPDF_Dict;

typedef void
(*HPDF_Dict_FreeFunc)  (HPDF_Dict  obj);

typedef HpdfStatus
(*HPDF_Dict_BeforeWriteFunc)  (HPDF_Dict  obj);

typedef HpdfStatus
(*HPDF_Dict_AfterWriteFunc)  (HPDF_Dict  obj);

typedef HpdfStatus
(*HPDF_Dict_OnWriteFunc)  (HPDF_Dict    obj,
   HPDF_Stream  stream);

typedef struct _HPDF_Dict_Rec {
   HpdfObjHeader             header;
   HpdfMemMgr                 *mmgr;
   HpdfError                  *error;
   HpdfList                   *list;
   HPDF_Dict_BeforeWriteFunc   before_write_fn;
   HPDF_Dict_OnWriteFunc       write_fn;
   HPDF_Dict_AfterWriteFunc    after_write_fn;
   HPDF_Dict_FreeFunc          free_fn;
   HPDF_Stream                 stream;
   HpdfUInt                    filter;
   HPDF_Dict                   filterParams;
   void                       *attr;
} HPDF_Dict_Rec;


typedef struct _HPDF_DictElement_Rec HpdfDictElement;

struct _HPDF_DictElement_Rec
{
   char                       key[HPDF_LIMIT_MAX_NAME_LEN + 1];
   void                      *value;
};

HPDF_Dict HPDF_Dict_New(HpdfMemMgr * const mmgr);
HPDF_Dict HPDF_DictStream_New(HpdfMemMgr * const mmgr, HPDF_Xref  xref);
void HPDF_Dict_Free(HPDF_Dict  dict);
HpdfStatus HPDF_Dict_Write(HPDF_Dict     dict, HPDF_Stream   stream, HPDF_Encrypt  e);
char const *HPDF_Dict_GetKeyByObj(HPDF_Dict   dict, void        *obj);
HpdfStatus HPDF_Dict_Add(HPDF_Dict     dict, char const  *key, void         *obj);
void *HPDF_Dict_GetItem(HPDF_Dict      dict, char const   *key, HpdfUInt16    obj_class);
HpdfStatus HPDF_Dict_AddName(HPDF_Dict     dict, char const  *key,char const  *value);
HpdfStatus HPDF_Dict_AddNumber(HPDF_Dict     dict, char const  *key, HpdfInt32    value);
HpdfStatus HPDF_Dict_AddReal(HPDF_Dict     dict, char const  *key, HpdfReal     value);
HpdfStatus HPDF_Dict_AddBoolean(HPDF_Dict     dict, char const  *key, HpdfBool     value);
HpdfStatus HPDF_Dict_RemoveElement(HPDF_Dict        dict, char const *key);

/*---------------------------------------------------------------------------*/
/*----- HPDF_ProxyObject ----------------------------------------------------*/

typedef struct _HPDF_Proxy_Rec  *HPDF_Proxy;

typedef struct _HPDF_Proxy_Rec {
   HpdfObjHeader  header;
   void             *obj;
} HPDF_Proxy_Rec;

HPDF_Proxy HPDF_Proxy_New(HpdfMemMgr * const mmgr, void       *obj);

/*---------------------------------------------------------------------------*/
/*----- HPDF_Xref -----------------------------------------------------------*/

typedef struct _HPDF_XrefEntry_Rec  *HPDF_XrefEntry;

typedef struct _HPDF_XrefEntry_Rec {
   char    entry_typ;
   HpdfUInt    byte_offset;
   HpdfUInt16  gen_no;
   void*        obj;
} HPDF_XrefEntry_Rec;


typedef struct _HPDF_Xref_Rec {
   HpdfMemMgr  *mmgr;
   HpdfError   *error;
   HpdfUInt32   start_offset;
   HpdfList    *entries;
   HpdfUInt     addr;
   HPDF_Xref    prev;
   HPDF_Dict    trailer;
} HPDF_Xref_Rec;

HPDF_Xref HPDF_Xref_New(HpdfMemMgr * const mmgr, HpdfUInt32  offset);
void HPDF_Xref_Free(HPDF_Xref  xref);
HpdfStatus HPDF_Xref_Add(HPDF_Xref  xref, void       *obj);
HPDF_XrefEntry HPDF_Xref_GetEntry(HPDF_Xref  xref, HpdfUInt  index);
HpdfStatus HPDF_Xref_WriteToStream(HPDF_Xref     xref, HPDF_Stream   stream, HPDF_Encrypt  e);
HPDF_XrefEntry HPDF_Xref_GetEntryByObjectId(HPDF_Xref  xref, HpdfUInt  obj_id);

typedef HPDF_Dict  HPDF_EmbeddedFile;
typedef HPDF_Dict  HpdfObjNameDict;
typedef HPDF_Dict  HpdfObjNameTree;
typedef HPDF_Dict  HPDF_Pages;
typedef HPDF_Dict  HPDF_Page;
typedef HPDF_Dict  HPDF_Annotation;
typedef HPDF_Dict  HPDF_3DMeasure;
typedef HPDF_Dict  HPDF_ExData;
typedef HPDF_Dict  HPDF_XObject;
typedef HPDF_Dict  HPDF_Image;
typedef HPDF_Dict  HPDF_Outline;
typedef HPDF_Dict  HPDF_EncryptDict;
typedef HPDF_Dict  HPDF_Action;
typedef HPDF_Dict  HPDF_ExtGState;
typedef HpdfArray  HpdfDestination;
typedef HPDF_Dict  HPDF_U3D;
typedef HPDF_Dict  HPDF_OutputIntent;
typedef HPDF_Dict  HPDF_JavaScript;
typedef HPDF_Dict  HPDF_Shading;

/*---------------------------------------------------------------------------*/
/*----- HpdfDirect ----------------------------------------------------------*/

typedef struct _HPDF_Direct_Rec HpdfDirect;

struct _HPDF_Direct_Rec
{
   HpdfObjHeader    header;
   HpdfMemMgr           *mmgr;
   HpdfError         *error;
   HpdfByte          *value;
   HpdfUInt           len;
};


HpdfDirect  *HPDF_Direct_New(       HpdfMemMgr * const mmgr,  HpdfByte * const value, HpdfUInt len);
HpdfStatus   HPDF_Direct_SetValue(  HpdfDirect       * const obj, HpdfByte * const value, HpdfUInt len);
void         HPDF_Direct_Free(      HpdfDirect       * const obj);
HpdfStatus   HPDF_Direct_Write(     HpdfDirect const * const obj, HPDF_Stream  stream);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _HPDF_OBJECTS_H */

