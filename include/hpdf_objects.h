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

typedef struct _HPDF_Obj_Header {
    HpdfUInt32  obj_id;
    HpdfUInt16  gen_no;
    HpdfUInt16  obj_class;
} HPDF_Obj_Header;



HpdfStatus
HPDF_Obj_WriteValue  (void          *obj,
                      HPDF_Stream   stream,
                      HPDF_Encrypt  e);


HpdfStatus
HPDF_Obj_Write  (void          *obj,
                 HPDF_Stream   stream,
                 HPDF_Encrypt  e);


void
HPDF_Obj_Free  (HPDF_MMgr    mmgr,
                void         *obj);


void
HPDF_Obj_ForceFree  (HPDF_MMgr    mmgr,
                     void         *obj);


/*---------------------------------------------------------------------------*/
/*----- HPDF_Null -----------------------------------------------------------*/

typedef struct _HPDF_Null_Rec  *HPDF_Null;

typedef struct _HPDF_Null_Rec {
    HPDF_Obj_Header header;
} HPDF_Null_Rec;



HPDF_Null
HPDF_Null_New  (HPDF_MMgr  mmgr);


/*---------------------------------------------------------------------------*/
/*----- HPDF_Boolean --------------------------------------------------------*/

typedef struct _HPDF_Boolean_Rec  *HPDF_Boolean;

typedef struct _HPDF_Boolean_Rec {
    HPDF_Obj_Header  header;
    HpdfBool        value;
} HPDF_Boolean_Rec;



HPDF_Boolean
HPDF_Boolean_New  (HPDF_MMgr  mmgr,
                   HpdfBool  value);


HpdfStatus
HPDF_Boolean_Write  (HPDF_Boolean  obj,
                     HPDF_Stream   stream);


/*---------------------------------------------------------------------------*/
/*----- HPDF_Number ---------------------------------------------------------*/

typedef struct _HPDF_Number_Rec  *HPDF_Number;

typedef struct _HPDF_Number_Rec {
    HPDF_Obj_Header  header;
    HpdfInt32       value;
} HPDF_Number_Rec;



HPDF_Number
HPDF_Number_New  (HPDF_MMgr   mmgr,
                  HpdfInt32  value);


void
HPDF_Number_SetValue  (HPDF_Number  obj,
                       HpdfInt32   value);


HpdfStatus
HPDF_Number_Write  (HPDF_Number  obj,
                    HPDF_Stream  stream);


/*---------------------------------------------------------------------------*/
/*----- HPDF_Real -----------------------------------------------------------*/

typedef struct _HPDF_Real_Rec  *HPDF_Real;

typedef struct _HPDF_Real_Rec {
    HPDF_Obj_Header  header;
    HpdfError       *error;
    HpdfReal        value;
} HPDF_Real_Rec;



HPDF_Real
HPDF_Real_New  (HPDF_MMgr  mmgr,
                HpdfReal  value);


HpdfStatus
HPDF_Real_Write  (HPDF_Real    obj,
                  HPDF_Stream  stream);


HpdfStatus
HPDF_Real_SetValue  (HPDF_Real  obj,
                     HpdfReal  value);


/*---------------------------------------------------------------------------*/
/*----- HPDF_Name -----------------------------------------------------------*/

typedef struct _HPDF_Name_Rec  *HPDF_Name;

typedef struct _HPDF_Name_Rec {
    HPDF_Obj_Header  header;
    HpdfError       *error;
    char        value[HPDF_LIMIT_MAX_NAME_LEN + 1];
} HPDF_Name_Rec;



HPDF_Name
HPDF_Name_New  (HPDF_MMgr        mmgr,
                char const *value);


HpdfStatus
HPDF_Name_SetValue  (HPDF_Name        obj,
                     char const *value);


HpdfStatus
HPDF_Name_Write  (HPDF_Name    obj,
                  HPDF_Stream  stream);

const char*
HPDF_Name_GetValue  (HPDF_Name  obj);


/*---------------------------------------------------------------------------*/
/*----- HPDF_String ---------------------------------------------------------*/

typedef struct _HPDF_String_Rec  *HPDF_String;

typedef struct _HPDF_String_Rec {
    HPDF_Obj_Header   header;
    HPDF_MMgr         mmgr;
    HpdfError        *error;
    HPDF_Encoder      encoder;
    HpdfByte         *value;
    HpdfUInt          len;
} HPDF_String_Rec;



HPDF_String
HPDF_String_New  (HPDF_MMgr        mmgr,
                  char const *value,
                  HPDF_Encoder     encoder);

#if defined(WIN32)
HPDF_String
HPDF_String_NewW (HPDF_MMgr        mmgr,
                  const wchar_t   *value,
                  HPDF_Encoder     encoder);
#endif


HpdfStatus
HPDF_String_SetValue  (HPDF_String      obj,
                       char const *value);


void
HPDF_String_Free  (HPDF_String  obj);


HpdfStatus
HPDF_String_Write  (HPDF_String  obj,
                    HPDF_Stream  stream,
                    HPDF_Encrypt e);

HpdfInt32
HPDF_String_Cmp  (HPDF_String s1,
                  HPDF_String s2);


/*---------------------------------------------------------------------------*/
/*----- HPDF_Binary ---------------------------------------------------------*/

typedef struct _HPDF_Binary_Rec  *HPDF_Binary;

typedef struct _HPDF_Binary_Rec {
    HPDF_Obj_Header  header;
    HPDF_MMgr        mmgr;
    HpdfError       *error;
    HpdfByte        *value;
    HpdfUInt         len;
} HPDF_Binary_Rec;



HPDF_Binary
HPDF_Binary_New  (HPDF_MMgr  mmgr,
                  HpdfByte   * const value,
                  HpdfUInt  len);


HpdfStatus
HPDF_Binary_SetValue  (HPDF_Binary  obj,
                       HpdfByte     * const value,
                       HpdfUInt    len);


HpdfByte *
HPDF_Binary_GetValue  (HPDF_Binary  obj);


void
HPDF_Binary_Free  (HPDF_Binary  obj);


HpdfStatus
HPDF_Binary_Write  (HPDF_Binary  obj,
                    HPDF_Stream  stream,
                    HPDF_Encrypt e);


HpdfUInt
HPDF_Binary_GetLen  (HPDF_Binary  obj);


/*---------------------------------------------------------------------------*/
/*----- HPDF_Array ----------------------------------------------------------*/

typedef struct _HPDF_Array_Rec  *HPDF_Array;

typedef struct _HPDF_Array_Rec {
    HPDF_Obj_Header  header;
    HPDF_MMgr        mmgr;
    HpdfError       *error;
    HPDF_List        list;
} HPDF_Array_Rec;


HPDF_Array
HPDF_Array_New  (HPDF_MMgr  mmgr);


HPDF_Array
HPDF_Box_Array_New  (HPDF_MMgr  mmgr,
                     HPDF_Box   box);


void
HPDF_Array_Free  (HPDF_Array  array);


HpdfStatus
HPDF_Array_Write  (HPDF_Array   array,
                   HPDF_Stream  stream,
                   HPDF_Encrypt e);


HpdfStatus
HPDF_Array_Add  (HPDF_Array  array,
                 void        *obj);


HpdfStatus
HPDF_Array_Insert  (HPDF_Array  array,
                    void        *target,
                    void        *obj);


void*
HPDF_Array_GetItem  (HPDF_Array   array,
                     HpdfUInt    index,
                     HpdfUInt16  obj_class);


HpdfStatus
HPDF_Array_AddNumber  (HPDF_Array  array,
                       HpdfInt32  value);


HpdfStatus
HPDF_Array_AddReal  (HPDF_Array  array,
                     HpdfReal   value);

HpdfStatus
HPDF_Array_AddNull  (HPDF_Array       array);

HpdfStatus
HPDF_Array_AddName  (HPDF_Array       array,
                     char const *value);

void
HPDF_Array_Clear  (HPDF_Array  array);


HpdfUInt
HPDF_Array_Items (HPDF_Array  array);


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
    HPDF_Obj_Header            header;
    HPDF_MMgr                  mmgr;
    HpdfError                 *error;
    HPDF_List                  list;
    HPDF_Dict_BeforeWriteFunc  before_write_fn;
    HPDF_Dict_OnWriteFunc      write_fn;
    HPDF_Dict_AfterWriteFunc   after_write_fn;
    HPDF_Dict_FreeFunc         free_fn;
    HPDF_Stream                stream;
    HpdfUInt                  filter;
    HPDF_Dict                  filterParams;
    void                       *attr;
} HPDF_Dict_Rec;


typedef struct _HPDF_DictElement_Rec *HPDF_DictElement;

typedef struct _HPDF_DictElement_Rec {
    char   key[HPDF_LIMIT_MAX_NAME_LEN + 1];
    void        *value;
} HPDF_DictElement_Rec;


HPDF_Dict
HPDF_Dict_New  (HPDF_MMgr  mmgr);


HPDF_Dict
HPDF_DictStream_New  (HPDF_MMgr  mmgr,
                      HPDF_Xref  xref);


void
HPDF_Dict_Free  (HPDF_Dict  dict);


HpdfStatus
HPDF_Dict_Write  (HPDF_Dict     dict,
                  HPDF_Stream   stream,
                  HPDF_Encrypt  e);


const char*
HPDF_Dict_GetKeyByObj (HPDF_Dict   dict,
                       void        *obj);


HpdfStatus
HPDF_Dict_Add  (HPDF_Dict     dict,
                char const  *key,
                void         *obj);


void*
HPDF_Dict_GetItem  (HPDF_Dict      dict,
                    char const   *key,
                    HpdfUInt16    obj_class);


HpdfStatus
HPDF_Dict_AddName (HPDF_Dict     dict,
                   char const  *key,
                   char const  *value);


HpdfStatus
HPDF_Dict_AddNumber  (HPDF_Dict     dict,
                      char const  *key,
                      HpdfInt32    value);


HpdfStatus
HPDF_Dict_AddReal  (HPDF_Dict     dict,
                    char const  *key,
                    HpdfReal     value);


HpdfStatus
HPDF_Dict_AddBoolean  (HPDF_Dict     dict,
                       char const  *key,
                       HpdfBool     value);


HpdfStatus
HPDF_Dict_RemoveElement  (HPDF_Dict        dict,
                          char const *key);


/*---------------------------------------------------------------------------*/
/*----- HPDF_ProxyObject ----------------------------------------------------*/



typedef struct _HPDF_Proxy_Rec  *HPDF_Proxy;

typedef struct _HPDF_Proxy_Rec {
    HPDF_Obj_Header  header;
    void             *obj;
} HPDF_Proxy_Rec;


HPDF_Proxy
HPDF_Proxy_New  (HPDF_MMgr  mmgr,
                 void       *obj);



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
      HPDF_MMgr    mmgr;
      HpdfError   *error;
      HpdfUInt32  start_offset;
      HPDF_List    entries;
      HpdfUInt    addr;
      HPDF_Xref    prev;
      HPDF_Dict    trailer;
} HPDF_Xref_Rec;


HPDF_Xref
HPDF_Xref_New  (HPDF_MMgr    mmgr,
                HpdfUInt32  offset);


void
HPDF_Xref_Free  (HPDF_Xref  xref);


HpdfStatus
HPDF_Xref_Add  (HPDF_Xref  xref,
                void       *obj);


HPDF_XrefEntry
HPDF_Xref_GetEntry  (HPDF_Xref  xref,
                     HpdfUInt  index);


HpdfStatus
HPDF_Xref_WriteToStream  (HPDF_Xref     xref,
                          HPDF_Stream   stream,
                          HPDF_Encrypt  e);


HPDF_XrefEntry
HPDF_Xref_GetEntryByObjectId  (HPDF_Xref  xref,
                               HpdfUInt  obj_id);



typedef HPDF_Dict  HPDF_EmbeddedFile;
typedef HPDF_Dict  HPDF_NameDict;
typedef HPDF_Dict  HPDF_NameTree;
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
typedef HPDF_Array HPDF_Destination;
typedef HPDF_Dict  HPDF_U3D;
typedef HPDF_Dict  HPDF_OutputIntent;
typedef HPDF_Dict  HPDF_JavaScript;
typedef HPDF_Dict  HPDF_Shading;

/*---------------------------------------------------------------------------*/
/*----- HPDF_Direct ---------------------------------------------------------*/

typedef struct _HPDF_Direct_Rec  *HPDF_Direct;

typedef struct _HPDF_Direct_Rec {
    HPDF_Obj_Header  header;
    HPDF_MMgr        mmgr;
    HpdfError       *error;
    HpdfByte        *value;
    HpdfUInt         len;
} HPDF_Direct_Rec;



HPDF_Direct
HPDF_Direct_New  (HPDF_MMgr  mmgr,
                  HpdfByte   * const value,
                  HpdfUInt  len);


HpdfStatus
HPDF_Direct_SetValue  (HPDF_Direct  obj,
                       HpdfByte     * const value,
                       HpdfUInt    len);

void
HPDF_Direct_Free  (HPDF_Direct  obj);


HpdfStatus
HPDF_Direct_Write  (HPDF_Direct  obj,
                    HPDF_Stream  stream);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _HPDF_OBJECTS_H */

