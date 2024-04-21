/*
 * << Haru Free PDF Library >> -- hpdf_shading.c
 *
 * URL: http://libharu.org
 *
 * Copyright (c) 1999-2006 Takeshi Kanno <takeshi_kanno@est.hi-ho.ne.jp>
 * Copyright (c) 2007-2009 Antony Dovgal <tony@daylessday.org>
 * Copyright (c) 2017 Kitware <kitware@kitware.com>
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 * It is provided "as is" without express or implied warranty.
 *
 */

#include "hpdf.h"
#include "hpdf_utils.h"

#include "assert.h"

typedef struct _RGBVertex
{
   HpdfUInt8 EdgeFlag;
   HpdfUInt32 X;
   HpdfUInt32 Y;
   HpdfUInt8 RGB[3];
} RGBVertex;

static const char *COL_CMYK = "DeviceCMYK";
static const char *COL_RGB = "DeviceRGB";
static const char *COL_GRAY = "DeviceGray";

/* bbox is filled with xMin, xMax, yMin, yMax */
static HpdfBool _GetDecodeArrayVertexValues(HPDF_Shading shading,
   HpdfReal *bbox)
{
   HpdfArray *decodeArray;
   HPDF_Real r;
   int i;

   if (!shading) {
      return HPDF_FALSE;
   }

   decodeArray = (HpdfArray *) (HPDF_Dict_GetItem(shading, "Decode", HPDF_OCLASS_ARRAY));
   if (!decodeArray) 
   {
      return HPDF_FALSE;
   }

   for (i = 0; i < 4; ++i)
   {
      r = HPDF_Array_GetItem(decodeArray, i, HPDF_OCLASS_REAL);
      if (!r) {
         return HPDF_FALSE;
      }

      bbox[i] = r->value;
   }

   return HPDF_TRUE;
}

static void UINT32Swap(HpdfUInt32  *value)
{
   HpdfByte b[4];

   HPDF_MemCpy(b, (HpdfByte *) value, 4);
   *value = (HpdfUInt32) ((HpdfUInt32) b[0] << 24 |
      (HpdfUInt32) b[1] << 16 |
      (HpdfUInt32) b[2] << 8 |
      (HpdfUInt32) b[3]);
}

/* Encode a position coordinate for writing */
static HpdfUInt32 _EncodeValue(HpdfReal x, HpdfReal xMin, HpdfReal xMax)
{
   HpdfDouble norm = (x - xMin) / (xMax - xMin);
   HpdfDouble max = (HpdfDouble) (0xFFFFFFFF);
   HpdfUInt32 enc = (HpdfUInt32) (norm * max);
   UINT32Swap(&enc);
   return enc;
}

HPDF_EXPORT(HPDF_Shading)
HPDF_Shading_New(
   HpdfDoc * const doc,
   HPDF_ShadingType type,
   HPDF_ColorSpace  colorSpace,
   HpdfReal xMin, HpdfReal xMax,
   HpdfReal yMin, HpdfReal yMax)
{
   HPDF_Shading shading;
   HpdfArray *decodeArray;
   HpdfStatus ret = HPDF_OK;
   int i;

   HPDF_PTRACE((" HPDF_Shading_New\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   /* Validate shading type: */
   switch (type)
   {
   case HPDF_SHADING_FREE_FORM_TRIANGLE_MESH:
      break;

   default:
      HPDF_SetError(doc->mmgr->error, HPDF_INVALID_SHADING_TYPE, 0);
      return NULL;
   }

   decodeArray = HPDF_Array_New(doc->mmgr);
   if (!decodeArray)
   {
      return NULL;
   }

   /* X-range */
   ret += HPDF_Array_AddReal(decodeArray, xMin);
   ret += HPDF_Array_AddReal(decodeArray, xMax);

   /* Y-range */
   ret += HPDF_Array_AddReal(decodeArray, yMin);
   ret += HPDF_Array_AddReal(decodeArray, yMax);

   const char *colName = NULL;
   switch (colorSpace)
   {
   case HPDF_CS_DEVICE_RGB:
      colName = COL_RGB;
      for (i = 0; i < 3; ++i)
      {
         ret += HPDF_Array_AddReal(decodeArray, 0.0);
         ret += HPDF_Array_AddReal(decodeArray, 1.0);
      }
      break;

   default:
      HPDF_SetError(doc->mmgr->error, HPDF_INVALID_COLOR_SPACE, 0);
      return NULL;
   }

   if (ret != HPDF_OK)
   {
      return NULL;
   }

   shading = HPDF_DictStream_New(doc->mmgr, doc->xref);
   if (!shading)
   {
      return NULL;
   }

   shading->header.obj_class |= HPDF_OSUBCLASS_SHADING;
   ret += HPDF_Dict_AddNumber(shading, "ShadingType", type);
   ret += HPDF_Dict_AddName(shading, "ColorSpace", colName);

   switch (type)
   {
   case HPDF_SHADING_FREE_FORM_TRIANGLE_MESH:
      ret += HPDF_Dict_AddNumber(shading, "BitsPerCoordinate", 32);
      ret += HPDF_Dict_AddNumber(shading, "BitsPerComponent", 8);
      ret += HPDF_Dict_AddNumber(shading, "BitsPerFlag", 8);
      ret += HPDF_Dict_Add(shading, "Decode", decodeArray);
      break;

   default:
      HPDF_SetError(doc->mmgr->error, HPDF_INVALID_SHADING_TYPE, 0);
      return NULL;
   }

   if (ret != HPDF_OK)
   {
      return NULL;
   }

   return shading;
}

HPDF_EXPORT(HpdfStatus)
HPDF_Shading_AddVertexRGB(HPDF_Shading shading,
   HPDF_Shading_FreeFormTriangleMeshEdgeFlag edgeFlag,
   HpdfReal x, HpdfReal y,
   HpdfUInt8 r, HpdfUInt8 g, HpdfUInt8 b)
{
   HpdfStatus ret = HPDF_OK;
   RGBVertex vert;
   float bbox[4];

   HPDF_PTRACE((" HPDF_Shading_AddVertexRGB\n"));

   if (!shading) {
      return HPDF_INVALID_OBJECT;
   }

   if (_GetDecodeArrayVertexValues(shading, bbox) != HPDF_TRUE) {
      return HPDF_SetError(shading->error, HPDF_INVALID_OBJECT, 0);
   }

   vert.EdgeFlag = (HpdfUInt8) edgeFlag;
   vert.X = _EncodeValue(x, bbox[0], bbox[1]);
   vert.Y = _EncodeValue(y, bbox[2], bbox[3]);
   vert.RGB[0] = r;
   vert.RGB[1] = g;
   vert.RGB[2] = b;

   ret = HPDF_Stream_Write(shading->stream,
      (HpdfByte *) (&vert.EdgeFlag), sizeof(vert.EdgeFlag));
   if (ret != HPDF_OK)
   {
      return ret;
   }

   ret = HPDF_Stream_Write(shading->stream,
      (HpdfByte *) (&vert.X), sizeof(vert.X));
   if (ret != HPDF_OK)
   {
      return ret;
   }

   ret = HPDF_Stream_Write(shading->stream,
      (HpdfByte *) (&vert.Y), sizeof(vert.Y));
   if (ret != HPDF_OK)
   {
      return ret;
   }

   ret = HPDF_Stream_Write(shading->stream,
      (HpdfByte *) (&vert.RGB), sizeof(vert.RGB));

   return ret;
}
