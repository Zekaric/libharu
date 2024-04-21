/*
 * << Haru Free PDF Library >> -- hpdf_u3d.c
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
#include "hpdf.h"

#include <string.h>

#ifndef M_PI
 /* Not defined in MSVC6 */
#define M_PI       3.14159265358979323846
#endif

HPDF_U3D
HPDF_U3D_LoadU3D(HPDF_MMgr        mmgr,
   HPDF_Stream      u3d_data,
   HPDF_Xref        xref);

static HpdfStatus Get3DStreamType(HPDF_Stream  stream, const char **type)
{
   static const char sign_u3d[] = "U3D";
   static const char sign_prc[] = "PRC";

   HpdfByte tag[4];
   HpdfUInt len;

   HPDF_PTRACE((" HPDF_U3D_Get3DStreamType\n"));

   len = 4;
   if (HPDF_Stream_Read(stream, tag, &len) != HPDF_OK) {
      return HPDF_Error_GetCode(stream->error);
   }

   if (HPDF_Stream_Seek(stream, 0, HPDF_SEEK_SET) != HPDF_OK) {
      return HPDF_Error_GetCode(stream->error);
   }

   if (HpdfMemIsEqual(tag, (HpdfByte *) sign_u3d, 4/* yes, \0 is required */))
   {
      *type = sign_u3d;
      return HPDF_OK;
   }

   if (HpdfMemIsEqual(tag, (HpdfByte *) sign_prc, 3))
   {
      *type = sign_prc;
      return HPDF_OK;
   }

   return HPDF_INVALID_U3D_DATA;
}


HPDF_EXPORT(HPDF_Image)
HPDF_LoadU3DFromFile(
   HpdfDoc * const doc,
   char const *filename)
{
   HPDF_Stream imagedata;
   HPDF_Image image;

   HPDF_PTRACE((" HPDF_LoadU3DFromFile\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   if (doc->pdf_version < HPDF_VER_17)
   {
      doc->pdf_version = HPDF_VER_17;
   }

   /* create file stream */
   imagedata = HPDF_FileReader_New(doc->mmgr, filename);

   if (HPDF_Stream_Validate(imagedata))
   {
      image = HPDF_U3D_LoadU3D(doc->mmgr, imagedata, doc->xref);
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
HPDF_LoadU3DFromFileW(
   HpdfDoc * const doc,
   const wchar_t *filename)
{
   HPDF_Stream imagedata;
   HPDF_Image image;

   HPDF_PTRACE((" HPDF_LoadU3DFromFileW\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   if (doc->pdf_version < HPDF_VER_17)
   {
      doc->pdf_version = HPDF_VER_17;
   }

   /* create file stream */
   imagedata = HPDF_FileReader_NewW(doc->mmgr, filename);

   if (HPDF_Stream_Validate(imagedata))
   {
      image = HPDF_U3D_LoadU3D(doc->mmgr, imagedata, doc->xref);
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
HPDF_LoadU3DFromMem(
   HpdfDoc * const doc,
   HpdfByte const *const buffer,
   HpdfUInt size)
{
   HPDF_Stream imagedata;
   HPDF_Image image;

   HPDF_PTRACE((" HPDF_LoadU3DFromMem\n"));

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

   if (HPDF_Stream_Validate(imagedata))
   {
      image = HPDF_U3D_LoadU3D(doc->mmgr, imagedata, doc->xref);
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

HPDF_U3D
HPDF_U3D_LoadU3D(HPDF_MMgr        mmgr,
   HPDF_Stream      u3d_data,
   HPDF_Xref        xref)
{
   HPDF_Dict u3d;
   const char *type = NULL;

   HPDF_PTRACE((" HPDF_U3D_LoadU3D\n"));

   u3d = HPDF_DictStream_New(mmgr, xref);
   if (!u3d) {
      return NULL;
   }

   u3d->filter = HPDF_STREAM_FILTER_FLATE_DECODE;

   if (HPDF_Dict_AddName(u3d, "Type", "3D") != HPDF_OK) {
      HPDF_Dict_Free(u3d);
      return NULL;
   }

   if (Get3DStreamType(u3d_data, &type) != HPDF_OK) {
      HPDF_Dict_Free(u3d);
      return NULL;
   }

   if (HPDF_Dict_AddName(u3d, "Subtype", type) != HPDF_OK) {
      HPDF_Dict_Free(u3d);
      return NULL;
   }

   for (;;) {
      HpdfByte buf[HPDF_STREAM_BUF_SIZ];
      HpdfUInt len = HPDF_STREAM_BUF_SIZ;
      HpdfStatus ret = HPDF_Stream_Read(u3d_data, buf, &len);

      if (ret != HPDF_OK) {
         if (ret == HPDF_STREAM_EOF) {
            if (len > 0) {
               ret = HPDF_Stream_Write(u3d->stream, buf, len);
               if (ret != HPDF_OK) {
                  HPDF_Dict_Free(u3d);
                  return NULL;
               }
            }
            break;
         }
         else {
            HPDF_Dict_Free(u3d);
            return NULL;
         }
      }

      if (HPDF_Stream_Write(u3d->stream, buf, len) != HPDF_OK) {
         HPDF_Dict_Free(u3d);
         return NULL;
      }
   }

   return u3d;
}

HPDF_EXPORT(HPDF_Dict) HPDF_Create3DView(HPDF_MMgr mmgr, const char *name)
{
   HpdfStatus ret = HPDF_OK;
   HPDF_Dict view;

   HPDF_PTRACE((" HPDF_Create3DView\n"));

   if (name == NULL || name[0] == '\0') {
      return NULL;
   }

   view = HPDF_Dict_New(mmgr);
   if (!view) {
      return NULL;
   }

   ret = HPDF_Dict_AddName(view, "TYPE", "3DView");
   if (ret != HPDF_OK) {
      HPDF_Dict_Free(view);
      return NULL;
   }

   ret = HPDF_Dict_Add(view, "XN", HPDF_String_New(mmgr, name, NULL));
   if (ret != HPDF_OK) {
      HPDF_Dict_Free(view);
      return NULL;
   }

   ret = HPDF_Dict_Add(view, "IN", HPDF_String_New(mmgr, name, NULL));
   if (ret != HPDF_OK) {
      HPDF_Dict_Free(view);
      return NULL;
   }

   return view;
}

HPDF_EXPORT(HpdfStatus) HPDF_U3D_Add3DView(HPDF_U3D u3d, HPDF_Dict view)
{
   HPDF_Array views = NULL;
   HpdfStatus ret = HPDF_OK;

   HPDF_PTRACE((" HPDF_Add3DView\n"));

   if (u3d == NULL || view == NULL) {
      return HPDF_INVALID_U3D_DATA;
   }

   views = (HPDF_Array) HPDF_Dict_GetItem(u3d, "VA", HPDF_OCLASS_ARRAY);
   if (views == NULL) {
      views = HPDF_Array_New(u3d->mmgr);
      if (!views) {
         return HPDF_Error_GetCode(u3d->error);
      }

      ret = HPDF_Dict_Add(u3d, "VA", views);
      if (ret == HPDF_OK) {
         ret = HPDF_Dict_AddNumber(u3d, "DV", 0);
      }
      else {
         HPDF_Array_Free(views);
         return ret;
      }
   }

   if (ret == HPDF_OK) {
      ret = HPDF_Array_Add(views, view);
   }

   return ret;
}


HPDF_EXPORT(HpdfStatus) HPDF_U3D_AddOnInstanciate(HPDF_U3D u3d, HPDF_JavaScript javascript)
{
   HpdfStatus ret = HPDF_OK;

   HPDF_PTRACE((" HPDF_U3D_AddOnInstanciate\n"));

   if (u3d == NULL || javascript == NULL) {
      return HPDF_INVALID_U3D_DATA;
   }

   ret = HPDF_Dict_Add(u3d, "OnInstantiate", javascript);

   return ret;
}


HPDF_EXPORT(HpdfStatus) HPDF_U3D_SetDefault3DView(HPDF_U3D u3d, const char *name)
{
   HpdfStatus ret = HPDF_OK;

   HPDF_PTRACE((" HPDF_U3D_SetDefault3DView\n"));

   if (u3d == NULL || name == NULL || name[0] == '\0') {
      return HPDF_INVALID_U3D_DATA;
   }

   ret = HPDF_Dict_Add(u3d, "DV", HPDF_String_New(u3d->mmgr, name, NULL));
   return ret;
}

HPDF_EXPORT(HpdfStatus) HPDF_3DView_AddNode(HPDF_Dict view, HPDF_Dict node)
{
   HPDF_Array nodes = NULL;
   HpdfStatus ret = HPDF_OK;

   HPDF_PTRACE((" HPDF_3DView_AddNode\n"));

   if (view == NULL) {
      return HPDF_INVALID_U3D_DATA;
   }

   nodes = (HPDF_Array) HPDF_Dict_GetItem(view, "NA", HPDF_OCLASS_ARRAY);
   if (nodes == NULL) {
      nodes = HPDF_Array_New(view->mmgr);
      if (!nodes) {
         return HPDF_Error_GetCode(view->error);
      }

      ret = HPDF_Dict_Add(view, "NA", nodes);
      if (ret != HPDF_OK) {
         HPDF_Array_Free(nodes);
         return ret;
      }
   }

   ret = HPDF_Array_Add(nodes, node);
   if (ret != HPDF_OK) {
      HPDF_Array_Free(nodes);
      return ret;
   }

   return ret;
}

HPDF_EXPORT(HPDF_Dict) HPDF_3DView_CreateNode(HPDF_Dict view, const char *name)
{
   HPDF_Dict  node;
   HpdfStatus ret = HPDF_OK;

   HPDF_PTRACE((" HPDF_3DView_CreateNode\n"));

   node = HPDF_Dict_New(view->mmgr);
   if (!node) {
      return NULL;
   }

   ret = HPDF_Dict_AddName(node, "Type", "3DNode");
   if (ret != HPDF_OK) {
      HPDF_Dict_Free(node);
      return NULL;
   }

   ret = HPDF_Dict_Add(node, "N", HPDF_String_New(view->mmgr, name, NULL));
   if (ret != HPDF_OK) {
      HPDF_Dict_Free(node);
      return NULL;
   }

   return node;
}

HPDF_EXPORT(HpdfStatus) HPDF_3DViewNode_SetOpacity(HPDF_Dict node, HpdfReal opacity)
{
   HpdfStatus ret = HPDF_OK;

   HPDF_PTRACE((" HPDF_3DViewNode_SetOpacity\n"));

   if (node == NULL) {
      return HPDF_INVALID_U3D_DATA;
   }

   ret = HPDF_Dict_AddReal(node, "O", opacity);
   return ret;
}

HPDF_EXPORT(HpdfStatus) HPDF_3DViewNode_SetVisibility(HPDF_Dict node, HpdfBool visible)
{
   HpdfStatus ret = HPDF_OK;

   HPDF_PTRACE((" HPDF_3DViewNode_SetVisibility\n"));

   if (node == NULL) {
      return HPDF_INVALID_U3D_DATA;
   }

   ret = HPDF_Dict_AddBoolean(node, "V", visible);
   return ret;
}

HPDF_EXPORT(HpdfStatus) HPDF_3DViewNode_SetMatrix(HPDF_Dict node, HPDF_3DMatrix Mat3D)
{
   HpdfStatus ret = HPDF_OK;
   HPDF_Array array_m;

   HPDF_PTRACE((" HPDF_3DViewNode_SetMatrix\n"));

   if (node == NULL) {
      return HPDF_INVALID_U3D_DATA;
   }

   array_m = HPDF_Array_New(node->mmgr);
   if (!array_m)
      return HPDF_INVALID_U3D_DATA;

   ret = HPDF_Dict_Add(node, "M", array_m);
   if (ret != HPDF_OK) {
      return ret;
   }

   ret += HPDF_Array_AddReal(array_m, Mat3D.a);
   ret += HPDF_Array_AddReal(array_m, Mat3D.b);
   ret += HPDF_Array_AddReal(array_m, Mat3D.c);

   ret += HPDF_Array_AddReal(array_m, Mat3D.d);
   ret += HPDF_Array_AddReal(array_m, Mat3D.e);
   ret += HPDF_Array_AddReal(array_m, Mat3D.f);

   ret += HPDF_Array_AddReal(array_m, Mat3D.g);
   ret += HPDF_Array_AddReal(array_m, Mat3D.h);
   ret += HPDF_Array_AddReal(array_m, Mat3D.i);

   ret += HPDF_Array_AddReal(array_m, Mat3D.tx);
   ret += HPDF_Array_AddReal(array_m, Mat3D.ty);
   ret += HPDF_Array_AddReal(array_m, Mat3D.tz);

   return ret;
}

HPDF_EXPORT(HpdfStatus) HPDF_3DView_SetLighting(HPDF_Dict view, const char *scheme)
{
   HpdfStatus ret = HPDF_OK;
   HPDF_Dict lighting;
   int i;
   static const char * const schemes[] =
   { "Artwork", "None", "White", "Day", "Night", "Hard", "Primary", "Blue", "Red", "Cube", "CAD", "Headlamp" };

   HPDF_PTRACE((" HPDF_3DView_SetLighting\n"));

   if (view == NULL || scheme == NULL || scheme[0] == '\0') {
      return HPDF_INVALID_U3D_DATA;
   }

   for (i = 0; i < 12; i++) {
      if (!strcmp(scheme, schemes[i])) {
         break;
      }
   }

   if (i == 12) {
      return HPDF_INVALID_U3D_DATA;
   }

   lighting = HPDF_Dict_New(view->mmgr);
   if (!lighting) {
      return HPDF_Error_GetCode(view->error);
   }

   ret = HPDF_Dict_AddName(lighting, "Type", "3DLightingScheme");
   if (ret != HPDF_OK) {
      HPDF_Dict_Free(lighting);
      return ret;
   }

   ret = HPDF_Dict_AddName(lighting, "Subtype", scheme);
   if (ret != HPDF_OK) {
      HPDF_Dict_Free(lighting);
      return ret;
   }

   ret = HPDF_Dict_Add(view, "LS", lighting);
   if (ret != HPDF_OK) {
      HPDF_Dict_Free(lighting);
      return ret;
   }
   return ret;
}

HPDF_EXPORT(HpdfStatus) HPDF_3DView_SetBackgroundColor(HPDF_Dict view, HpdfReal r, HpdfReal g, HpdfReal b)
{
   HPDF_Array  color;
   HpdfStatus ret = HPDF_OK;
   HPDF_Dict background;

   HPDF_PTRACE((" HPDF_3DView_SetBackgroundColor\n"));

   if (view == NULL || r < 0 || r > 1 || g < 0 || g > 1 || b < 0 || b > 1) {
      return HPDF_INVALID_U3D_DATA;
   }

   background = HPDF_Dict_New(view->mmgr);
   if (!background) {
      return HPDF_Error_GetCode(view->error);
   }

   color = HPDF_Array_New(view->mmgr);
   if (!color) {
      HPDF_Dict_Free(background);
      return HPDF_Error_GetCode(view->error);
   }

   ret = HPDF_Array_AddReal(color, r);
   if (ret != HPDF_OK) {
      HPDF_Array_Free(color);
      HPDF_Dict_Free(background);
      return ret;
   }

   ret = HPDF_Array_AddReal(color, g);
   if (ret != HPDF_OK) {
      HPDF_Array_Free(color);
      HPDF_Dict_Free(background);
      return ret;
   }

   ret = HPDF_Array_AddReal(color, b);
   if (ret != HPDF_OK) {
      HPDF_Array_Free(color);
      HPDF_Dict_Free(background);
      return ret;
   }


   ret = HPDF_Dict_AddName(background, "Type", "3DBG");
   if (ret != HPDF_OK) {
      HPDF_Array_Free(color);
      HPDF_Dict_Free(background);
      return ret;
   }

   ret = HPDF_Dict_Add(background, "C", color);
   if (ret != HPDF_OK) {
      HPDF_Array_Free(color);
      HPDF_Dict_Free(background);
      return ret;
   }

   ret = HPDF_Dict_Add(view, "BG", background);
   if (ret != HPDF_OK) {
      HPDF_Array_Free(color);
      HPDF_Dict_Free(background);
      return ret;
   }
   return ret;
}

HPDF_EXPORT(HpdfStatus) HPDF_3DView_SetPerspectiveProjection(HPDF_Dict view, HpdfReal fov)
{
   HpdfStatus ret = HPDF_OK;
   HPDF_Dict projection;

   HPDF_PTRACE((" HPDF_3DView_SetPerspectiveProjection\n"));

   if (view == NULL || fov < 0 || fov > 180) {
      return HPDF_INVALID_U3D_DATA;
   }

   projection = HPDF_Dict_New(view->mmgr);
   if (!projection) {
      return HPDF_Error_GetCode(view->error);
   }

   ret = HPDF_Dict_AddName(projection, "Subtype", "P");
   if (ret != HPDF_OK) {
      HPDF_Dict_Free(projection);
      return ret;
   }

   ret = HPDF_Dict_AddName(projection, "PS", "Min");
   if (ret != HPDF_OK) {
      HPDF_Dict_Free(projection);
      return ret;
   }

   ret = HPDF_Dict_AddReal(projection, "FOV", fov);
   if (ret != HPDF_OK) {
      HPDF_Dict_Free(projection);
      return ret;
   }

   ret = HPDF_Dict_Add(view, "P", projection);
   if (ret != HPDF_OK) {
      HPDF_Dict_Free(projection);
      return ret;
   }
   return ret;
}

HPDF_EXPORT(HpdfStatus) HPDF_3DView_SetOrthogonalProjection(HPDF_Dict view, HpdfReal mag)
{
   HpdfStatus ret = HPDF_OK;
   HPDF_Dict projection;

   HPDF_PTRACE((" HPDF_3DView_SetOrthogonalProjection\n"));

   if (view == NULL || mag <= 0) {
      return HPDF_INVALID_U3D_DATA;
   }

   projection = HPDF_Dict_New(view->mmgr);
   if (!projection) {
      return HPDF_Error_GetCode(view->error);
   }

   ret = HPDF_Dict_AddName(projection, "Subtype", "O");
   if (ret != HPDF_OK) {
      HPDF_Dict_Free(projection);
      return ret;
   }

   ret = HPDF_Dict_AddReal(projection, "OS", mag);
   if (ret != HPDF_OK) {
      HPDF_Dict_Free(projection);
      return ret;
   }

   ret = HPDF_Dict_Add(view, "P", projection);
   if (ret != HPDF_OK) {
      HPDF_Dict_Free(projection);
      return ret;
   }
   return ret;
}

#define normalize(x, y, z)        \
{                    \
    HpdfReal modulo;            \
    modulo = (float)sqrt((float)(x*x) + (float)(y*y) + (float)(z*z));    \
    if (modulo != 0.0)            \
    {                    \
        x = x/modulo;            \
        y = y/modulo;            \
        z = z/modulo;            \
    }                    \
}

/* building the transformation matrix*/
/* #1,#2,#3 centre of orbit coordinates (coo)*/
/* #4,#5,#6 centre of orbit to camera direction vector (c2c)*/
/* #7 orbital radius (roo)*/
/* #8 camera roll (roll)*/

HPDF_EXPORT(HpdfStatus) HPDF_3DView_SetCamera(HPDF_Dict view, HpdfReal coox, HpdfReal cooy, HpdfReal cooz, HpdfReal c2cx, HpdfReal c2cy, HpdfReal c2cz, HpdfReal roo, HpdfReal roll)
{
   HpdfReal viewx, viewy, viewz;
   HpdfReal leftx, lefty, leftz;
   HpdfReal upx, upy, upz;
   HpdfReal transx, transy, transz;

   HPDF_Array  matrix;
   HpdfStatus ret = HPDF_OK;

   HPDF_PTRACE((" HPDF_3DView_SetCamera\n"));

   if (view == NULL) {
      return HPDF_INVALID_U3D_DATA;
   }

   /* view vector (opposite to c2c) */
   viewx = -c2cx;
   viewy = -c2cy;
   viewz = -c2cz;

   /* c2c = (0, -1, 0) by default */
   if (viewx == 0.0 && viewy == 0.0 && viewz == 0.0) {
      viewy = 1.0;
   }
   /* normalize view vector */
   normalize(viewx, viewy, viewz);

   /* rotation matrix */

   /* top and bottom views */
   leftx = -1.0f;
   lefty =  0.0f;
   leftz =  0.0f;

   /* up-vector */
   if (viewz < 0.0) /* top view*/
   {
      upx = 0.0f;
      upy = 1.0f;
      upz = 0.0f;
   }
   else /* bottom view*/
   {
      upx = 0.0f;
      upy =-1.0f;
      upz = 0.0f;
   }

   if (fabs(viewx) + fabs(viewy) != 0.0f) /* other views than top and bottom*/
   {
      /* up-vector = up_world - (up_world dot view) view*/
      upx = -viewz*viewx;
      upy = -viewz*viewy;
      upz = -viewz*viewz + 1.0f;
      /* normalize up-vector*/
      normalize(upx, upy, upz);
      /* left vector = up x view*/
      leftx = viewz*upy - viewy*upz;
      lefty = viewx*upz - viewz*upx;
      leftz = viewy*upx - viewx*upy;
      /* normalize left vector*/
      normalize(leftx, lefty, leftz);
   }
   /* apply camera roll*/
   {
      HpdfReal leftxprime, leftyprime, leftzprime;
      HpdfReal upxprime, upyprime, upzprime;
      HpdfReal sinroll, cosroll;

      sinroll =  (HpdfReal) sin((roll/180.0f)*M_PI);
      cosroll =  (HpdfReal) cos((roll/180.0f)*M_PI);
      leftxprime = leftx*cosroll + upx*sinroll;
      leftyprime = lefty*cosroll + upy*sinroll;
      leftzprime = leftz*cosroll + upz*sinroll;
      upxprime = upx*cosroll + leftx*sinroll;
      upyprime = upy*cosroll + lefty*sinroll;
      upzprime = upz*cosroll + leftz*sinroll;
      leftx = leftxprime;
      lefty = leftyprime;
      leftz = leftzprime;
      upx = upxprime;
      upy = upyprime;
      upz = upzprime;
   }

   /* translation vector*/
   roo = (HpdfReal) fabs(roo);
   if (roo == 0.0) {
      roo = (HpdfReal) 0.000000000000000001;
   }
   transx = coox - roo*viewx;
   transy = cooy - roo*viewy;
   transz = cooz - roo*viewz;

   /* transformation matrix*/
   matrix = HPDF_Array_New(view->mmgr);
   if (!matrix) {
      return HPDF_Error_GetCode(view->error);
   }

   ret = HPDF_Array_AddReal(matrix, leftx);
   if (ret != HPDF_OK) goto failed;

   ret = HPDF_Array_AddReal(matrix, lefty);
   if (ret != HPDF_OK) goto failed;

   ret = HPDF_Array_AddReal(matrix, leftz);
   if (ret != HPDF_OK) goto failed;

   ret = HPDF_Array_AddReal(matrix, upx);
   if (ret != HPDF_OK) goto failed;

   ret = HPDF_Array_AddReal(matrix, upy);
   if (ret != HPDF_OK) goto failed;

   ret = HPDF_Array_AddReal(matrix, upz);
   if (ret != HPDF_OK) goto failed;

   ret = HPDF_Array_AddReal(matrix, viewx);
   if (ret != HPDF_OK) goto failed;

   ret = HPDF_Array_AddReal(matrix, viewy);
   if (ret != HPDF_OK) goto failed;

   ret = HPDF_Array_AddReal(matrix, viewz);
   if (ret != HPDF_OK) goto failed;

   ret = HPDF_Array_AddReal(matrix, transx);
   if (ret != HPDF_OK) goto failed;

   ret = HPDF_Array_AddReal(matrix, transy);
   if (ret != HPDF_OK) goto failed;

   ret = HPDF_Array_AddReal(matrix, transz);
   if (ret != HPDF_OK) goto failed;

   ret = HPDF_Dict_AddName(view, "MS", "M");
   if (ret != HPDF_OK) goto failed;

   ret = HPDF_Dict_Add(view, "C2W", matrix);
   if (ret != HPDF_OK) goto failed;

   ret = HPDF_Dict_AddReal(view, "CO", roo);

failed:
   if (ret != HPDF_OK) {
      HPDF_Array_Free(matrix);
      return ret;
   }
   return ret;
}

HPDF_EXPORT(HpdfStatus) HPDF_3DView_SetCameraByMatrix(HPDF_Dict view, HPDF_3DMatrix Mat3D, HpdfReal co)
{
   HPDF_Array  matrix;
   HpdfStatus ret = HPDF_OK;

   HPDF_PTRACE((" HPDF_3DView_SetCameraByMatrix\n"));

   if (view == NULL) {
      return HPDF_INVALID_U3D_DATA;
   }

   /* transformation matrix*/
   matrix = HPDF_Array_New(view->mmgr);
   if (!matrix) {
      return HPDF_Error_GetCode(view->error);
   }

   ret = HPDF_Array_AddReal(matrix, Mat3D.a);
   if (ret != HPDF_OK) goto failed;

   ret = HPDF_Array_AddReal(matrix, Mat3D.b);
   if (ret != HPDF_OK) goto failed;

   ret = HPDF_Array_AddReal(matrix, Mat3D.c);
   if (ret != HPDF_OK) goto failed;

   ret = HPDF_Array_AddReal(matrix, Mat3D.d);
   if (ret != HPDF_OK) goto failed;

   ret = HPDF_Array_AddReal(matrix, Mat3D.e);
   if (ret != HPDF_OK) goto failed;

   ret = HPDF_Array_AddReal(matrix, Mat3D.f);
   if (ret != HPDF_OK) goto failed;

   ret = HPDF_Array_AddReal(matrix, Mat3D.g);
   if (ret != HPDF_OK) goto failed;

   ret = HPDF_Array_AddReal(matrix, Mat3D.h);
   if (ret != HPDF_OK) goto failed;

   ret = HPDF_Array_AddReal(matrix, Mat3D.i);
   if (ret != HPDF_OK) goto failed;

   ret = HPDF_Array_AddReal(matrix, Mat3D.tx);
   if (ret != HPDF_OK) goto failed;

   ret = HPDF_Array_AddReal(matrix, Mat3D.ty);
   if (ret != HPDF_OK) goto failed;

   ret = HPDF_Array_AddReal(matrix, Mat3D.tz);
   if (ret != HPDF_OK) goto failed;

   ret = HPDF_Dict_AddName(view, "MS", "M");
   if (ret != HPDF_OK) goto failed;

   ret = HPDF_Dict_Add(view, "C2W", matrix);
   if (ret != HPDF_OK) goto failed;

   ret = HPDF_Dict_AddReal(view, "CO", co);
   if (ret != HPDF_OK) goto failed;

failed:
   if (ret != HPDF_OK) {
      HPDF_Array_Free(matrix);
      return ret;
   }
   return ret;
}

HPDF_EXPORT(HpdfStatus)
HPDF_3DView_SetCrossSectionOn(
   HPDF_Dict      view,
   HpdfPoint3D const center,
   HpdfReal      Roll,
   HpdfReal      Pitch,
   HpdfReal      opacity,
   HpdfBool      showintersection)
{
   HpdfStatus ret = HPDF_OK;
   HPDF_Array array_b;
   HPDF_Array array_sa;
   HPDF_Dict  crosssection;

   HPDF_PTRACE((" HPDF_3DView_SetCrossSectionOn\n"));

   if (view == NULL)
   {
      return HPDF_INVALID_U3D_DATA;
   }

   crosssection = HPDF_Dict_New(view->mmgr);
   if (!crosssection)
   {
      return HPDF_Error_GetCode(view->error);
   }

   ret = HPDF_Dict_AddName(crosssection, "Type", "3DCrossSection");
   if (ret != HPDF_OK)
   {
      HPDF_Dict_Free(crosssection);
      return ret;
   }

   array_b = HPDF_Array_New(view->mmgr);
   if (!array_b)
   {
      HPDF_Dict_Free(crosssection);
      return HPDF_Error_GetCode(view->error);
   }

   if (HPDF_Dict_Add(crosssection, "C", array_b) != HPDF_OK)
   {
      HPDF_Dict_Free(crosssection);
      return HPDF_INVALID_U3D_DATA;
   }

   ret += HPDF_Array_AddReal(array_b, center.x);
   ret += HPDF_Array_AddReal(array_b, center.y);
   ret += HPDF_Array_AddReal(array_b, center.z);

   array_b = HPDF_Array_New(view->mmgr);
   if (!array_b)
   {
      HPDF_Dict_Free(crosssection);
      return HPDF_Error_GetCode(view->error);
   }

   if (HPDF_Dict_Add(crosssection, "O", array_b) != HPDF_OK)
   {
      HPDF_Dict_Free(crosssection);
      return HPDF_INVALID_U3D_DATA;
   }

   ret += HPDF_Array_AddNull(array_b);
   ret += HPDF_Array_AddReal(array_b, Roll);
   ret += HPDF_Array_AddReal(array_b, Pitch);

   ret += HPDF_Dict_AddReal(crosssection, "PO", opacity);

   ret += HPDF_Dict_AddBoolean(crosssection, "IV", showintersection);

   array_b = HPDF_Array_New(view->mmgr);
   if (!array_b)
   {
      HPDF_Dict_Free(crosssection);
      return HPDF_Error_GetCode(view->error);
   }
   if (HPDF_Dict_Add(crosssection, "IC", array_b) != HPDF_OK)
   {
      HPDF_Dict_Free(crosssection);
      return HPDF_INVALID_U3D_DATA;
   }
   ret += HPDF_Array_AddName(array_b, "DeviceRGB");
   ret += HPDF_Array_AddReal(array_b, 1.0);
   ret += HPDF_Array_AddReal(array_b, 0.0);
   ret += HPDF_Array_AddReal(array_b, 0.0);

   array_sa = HPDF_Array_New(view->mmgr);
   if (!array_sa)
   {
      HPDF_Dict_Free(crosssection);
      return HPDF_Error_GetCode(view->error);
   }

   if (HPDF_Dict_Add(view, "SA", array_sa) != HPDF_OK)
   {
      HPDF_Dict_Free(crosssection);
      return HPDF_INVALID_U3D_DATA;
   }

   if (HPDF_Array_Add(array_sa, crosssection) != HPDF_OK)
   {
      HPDF_Dict_Free(crosssection);
      return HPDF_INVALID_U3D_DATA;
   }

   return ret;
}

HPDF_EXPORT(HpdfStatus) HPDF_3DView_SetCrossSectionOff(HPDF_Dict view)
{
   HpdfStatus ret = HPDF_OK;
   HPDF_Array array_sa;

   HPDF_PTRACE((" HPDF_3DView_SetCrossSectionOff\n"));

   if (view == NULL) {
      return HPDF_INVALID_U3D_DATA;
   }

   array_sa = HPDF_Array_New(view->mmgr);
   if (!array_sa) {
      return HPDF_Error_GetCode(view->error);
   }

   if (HPDF_Dict_Add(view, "SA", array_sa) != HPDF_OK)
   {
      return HPDF_INVALID_U3D_DATA;
   }

   return ret;
}

HPDF_Dict HPDF_3DView_New(HPDF_MMgr  mmgr, HPDF_Xref  xref, HPDF_U3D u3d, const char *name)
{
   HpdfStatus ret = HPDF_OK;
   HPDF_Dict view;

   HPDF_PTRACE((" HPDF_3DView_New\n"));

   if (name == NULL || name[0] == '\0') {
      return NULL;
   }

   view = HPDF_Dict_New(mmgr);
   if (!view) {
      return NULL;
   }

   if (HPDF_Xref_Add(xref, view) != HPDF_OK)
      return NULL;

   ret = HPDF_Dict_AddName(view, "TYPE", "3DView");
   if (ret != HPDF_OK) {
      HPDF_Dict_Free(view);
      return NULL;
   }

   ret = HPDF_Dict_Add(view, "XN", HPDF_String_New(mmgr, name, NULL));
   if (ret != HPDF_OK) {
      HPDF_Dict_Free(view);
      return NULL;
   }

   ret = HPDF_Dict_Add(view, "IN", HPDF_String_New(mmgr, name, NULL));
   if (ret != HPDF_OK) {
      HPDF_Dict_Free(view);
      return NULL;
   }

   ret = HPDF_U3D_Add3DView(u3d, view);
   if (ret != HPDF_OK) {
      HPDF_Dict_Free(view);
      return NULL;
   }

   return view;
}


HPDF_EXPORT(HpdfStatus)
HPDF_3DView_Add3DC3DMeasure(HPDF_Dict       view,
   HPDF_3DMeasure measure)
{

   HpdfStatus ret = HPDF_OK;
   HPDF_Array array;
   void* a;

   a = HPDF_Dict_GetItem(view, "MA", HPDF_OCLASS_ARRAY);

   if (a)
   {
      array = (HPDF_Array) a;
   }
   else
   {
      array = HPDF_Array_New(view->mmgr);
      if (!array)
         return 0;

      if (HPDF_Dict_Add(view, "MA", array) != HPDF_OK)
         return 0;
   }

   ret = HPDF_Array_Add(array, measure);

   return ret;
}


HPDF_EXPORT(HPDF_JavaScript)
HPDF_CreateJavaScript(
   HpdfDoc const * const doc,
   const char *code)
{
   HPDF_JavaScript javaScript;
   HpdfUInt len;

   HPDF_PTRACE((" HPDF_CreateJavaScript\n"));

   javaScript = (HPDF_JavaScript) HPDF_DictStream_New(doc->mmgr, doc->xref);
   if (!javaScript)
   {
      return NULL;
   }

   javaScript->filter = HPDF_STREAM_FILTER_FLATE_DECODE;

   len = (HpdfUInt) strlen(code);
   if (HPDF_Stream_Write(javaScript->stream, (HpdfByte *) code, len) != HPDF_OK)
   {
      HPDF_Dict_Free(javaScript);
      return NULL;
   }

   return javaScript;
}

HPDF_EXPORT(HPDF_JavaScript)
HPDF_LoadJSFromFile(
   HpdfDoc * const doc,
   const char *filename)
{
   HPDF_Stream js_data;
   HPDF_JavaScript js = NULL;

   HPDF_PTRACE((" HPDF_LoadJSFromFile\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   /* create file stream */
   js_data = HPDF_FileReader_New(doc->mmgr, filename);

   if (!HPDF_Stream_Validate(js_data))
   {
      HPDF_Stream_Free(js_data);
      return NULL;
   }

   js = (HPDF_JavaScript) HPDF_DictStream_New(doc->mmgr, doc->xref);
   if (!js)
   {
      HPDF_Stream_Free(js_data);
      return NULL;
   }

   js->filter = HPDF_STREAM_FILTER_FLATE_DECODE; // or HPDF_STREAM_FILTER_NONE

   for (;;)
   {
      HpdfByte buf[HPDF_STREAM_BUF_SIZ];
      HpdfUInt len = HPDF_STREAM_BUF_SIZ;
      HpdfStatus ret = HPDF_Stream_Read(js_data, buf, &len);

      if (ret != HPDF_OK)
      {
         if (ret == HPDF_STREAM_EOF)
         {
            if (len > 0)
            {
               ret = HPDF_Stream_Write(js->stream, buf, len);
               if (ret != HPDF_OK)
               {
                  HPDF_Stream_Free(js_data);
                  HPDF_Dict_Free(js);
                  return NULL;
               }
            }
            break;
         }
         else
         {
            HPDF_Stream_Free(js_data);
            HPDF_Dict_Free(js);
            return NULL;
         }
      }

      if (HPDF_Stream_Write(js->stream, buf, len) != HPDF_OK)
      {
         HPDF_Stream_Free(js_data);
         HPDF_Dict_Free(js);
         return NULL;
      }
   }

   /* destroy file stream */
   HPDF_Stream_Free(js_data);

   return js;
}

#if defined(WIN32)
HPDF_EXPORT(HPDF_JavaScript)
HPDF_LoadJSFromFileW(
   HpdfDoc * const doc,
   const wchar_t *filename)
{
   HPDF_Stream js_data;
   HPDF_JavaScript js = NULL;

   HPDF_PTRACE((" HPDF_LoadJSFromFileW\n"));

   if (!HPDF_HasDoc(doc))
   {
      return NULL;
   }

   /* create file stream */
   js_data = HPDF_FileReader_NewW(doc->mmgr, filename);

   if (!HPDF_Stream_Validate(js_data))
   {
      HPDF_Stream_Free(js_data);
      return NULL;
   }

   js = (HPDF_JavaScript) HPDF_DictStream_New(doc->mmgr, doc->xref);
   if (!js)
   {
      HPDF_Stream_Free(js_data);
      return NULL;
   }

   js->filter = HPDF_STREAM_FILTER_FLATE_DECODE; // or HPDF_STREAM_FILTER_NONE

   for (;;)
   {
      HpdfByte buf[HPDF_STREAM_BUF_SIZ];
      HpdfUInt len = HPDF_STREAM_BUF_SIZ;
      HpdfStatus ret = HPDF_Stream_Read(js_data, buf, &len);

      if (ret != HPDF_OK)
      {
         if (ret == HPDF_STREAM_EOF)
         {
            if (len > 0)
            {
               ret = HPDF_Stream_Write(js->stream, buf, len);
               if (ret != HPDF_OK)
               {
                  HPDF_Stream_Free(js_data);
                  HPDF_Dict_Free(js);
                  return NULL;
               }
            }
            break;
         }
         else
         {
            HPDF_Stream_Free(js_data);
            HPDF_Dict_Free(js);
            return NULL;
         }
      }

      if (HPDF_Stream_Write(js->stream, buf, len) != HPDF_OK)
      {
         HPDF_Stream_Free(js_data);
         HPDF_Dict_Free(js);
         return NULL;
      }
   }

   /* destroy file stream */
   HPDF_Stream_Free(js_data);

   return js;
}
#endif

#undef normalize
