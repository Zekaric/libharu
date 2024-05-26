/*
 * << Haru Free PDF Library >> -- hpdf_annotation.c
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
#include "hpdf_info.h"
#include "hpdf_annotation.h"
#include "hpdf.h"

static char const * const HPDF_ANNOT_TYPE_NAMES[] = {
                                        "Text",
                                        "Link",
                                        "Sound",
                                        "FreeText",
                                        "Stamp",
                                        "Square",
                                        "Circle",
                                        "StrikeOut",
                                        "Highlight",
                                        "Underline",
                                        "Ink",
                                        "FileAttachment",
                                        "Popup",
                                        "3D",
                                        "Squiggly",
                                                            "Line",
                                                            "Projection",
                                                            "Widget"
};

static char const * const HPDF_ANNOT_ICON_NAMES_NAMES[] = {
                                        "Comment",
                                        "Key",
                                        "Note",
                                        "Help",
                                        "NewParagraph",
                                        "Paragraph",
                                        "Insert"
};

static char const * const HPDF_ANNOT_INTENT_NAMES[] = {
                                        "FreeTextCallout",
                                        "FreeTextTypeWriter",
                                        "LineArrow",
                                        "LineDimension",
                                        "PolygonCloud",
                                        "PolyLineDimension",
                                        "PolygonDimension"
};

static char const * const HPDF_LINE_ANNOT_ENDING_STYLE_NAMES[] = {
                                        "None",
                                        "Square",
                                        "Circle",
                                        "Diamond",
                                        "OpenArrow",
                                        "ClosedArrow",
                                        "Butt",
                                        "ROpenArrow",
                                        "RClosedArrow",
                                        "Slash"
};

static char const * const HPDF_LINE_ANNOT_CAP_POSITION_NAMES[] = {
                                        "Inline",
                                        "Top"
};

static char const * const HPDF_STAMP_ANNOT_NAME_NAMES[] = {
                                        "Approved",
                                        "Experimental",
                                        "NotApproved",
                                        "AsIs",
                                        "Expired",
                                        "NotForPublicRelease",
                                        "Confidential",
                                        "Final",
                                        "Sold",
                                        "Departmental",
                                        "ForComment",
                                        "TopSecret",
                                        "Draft",
                                        "ForPublicRelease"
};

static HpdfBool
CheckSubType(HPDF_Annotation  annot,
   HPDF_AnnotType  type);


/*----------------------------------------------------------------------------*/
/*------ HPDF_Annotation -----------------------------------------------------*/


HPDF_Annotation
   HPDF_Annotation_New(
      HpdfMemMgr * const mmgr,
      HPDF_Xref       xref,
      HPDF_AnnotType  type,
      HpdfRect const rect)
{
   HPDF_Annotation annot;
   HpdfArray *array;
   HpdfStatus ret = HPDF_OK;
   HpdfRect rectTemp;

   HPDF_PTRACE((" HPDF_Annotation_New\n"));

   annot = HPDF_Dict_New(mmgr);
   if (!annot)
   {
      return NULL;
   }

   if (HPDF_Xref_Add(xref, annot) != HPDF_OK)
   {
      return NULL;
   }

   array = HpdfArrayCreate(mmgr);
   if (!array)
   {
      return NULL;
   }

   if (HPDF_Dict_Add(annot, "Rect", array) != HPDF_OK)
   {
      return NULL;
   }

   rectTemp = rect;
   if (rect.top < rect.bottom) 
   {
      rectTemp.top    = rect.bottom;
      rectTemp.bottom = rect.top;
   }

   ret += HpdfArrayAddReal(array, rectTemp.left);
   ret += HpdfArrayAddReal(array, rectTemp.bottom);
   ret += HpdfArrayAddReal(array, rectTemp.right);
   ret += HpdfArrayAddReal(array, rectTemp.top);

   ret += HPDF_Dict_AddName(annot, "Type",    "Annot");
   ret += HPDF_Dict_AddName(annot, "Subtype", HPDF_ANNOT_TYPE_NAMES[(HpdfInt) type]);

   if (ret != HPDF_OK)
   {
      return NULL;
   }

   annot->header.obj_class |= HPDF_OSUBCLASS_ANNOTATION;

   return annot;
}

HPDF_EXPORT(HpdfStatus)
   HPDF_Annotation_SetBorderStyle(
      HPDF_Annotation  annot,
   HPDF_BSSubtype   subtype,
   HpdfReal        width,
   HpdfUInt16      dash_on,
   HpdfUInt16      dash_off,
   HpdfUInt16      dash_phase)
{
   HPDF_Dict bs;
   HpdfArray *dash;
   HpdfStatus ret;

   HPDF_PTRACE((" HPDF_Annotation_SetBoderStyle\n"));

   bs = HPDF_Dict_New(annot->mmgr);
   if (!bs)
      return HPDF_Error_GetCode(annot->error);

   if ((ret = HPDF_Dict_Add(annot, "BS", bs)) != HPDF_OK)
      return ret;

   if (subtype == HPDF_BS_DASHED) 
   {
      dash = HpdfArrayCreate(annot->mmgr);
      if (!dash)
      {
         return HPDF_Error_GetCode(annot->error);
      }

      if ((ret = HPDF_Dict_Add(bs, "D", dash)) != HPDF_OK)
         return ret;

      ret += HPDF_Dict_AddName(bs, "Type", "Border");
      ret += HpdfArrayAddReal(dash, dash_on);
      ret += HpdfArrayAddReal(dash, dash_off);

      if (dash_phase  != 0)
      {
         ret += HpdfArrayAddReal(dash, dash_off);
      }
   }

   switch (subtype) {
   case HPDF_BS_SOLID:
      ret += HPDF_Dict_AddName(bs, "S", "S");
      break;
   case HPDF_BS_DASHED:
      ret += HPDF_Dict_AddName(bs, "S", "D");
      break;
   case HPDF_BS_BEVELED:
      ret += HPDF_Dict_AddName(bs, "S", "B");
      break;
   case HPDF_BS_INSET:
      ret += HPDF_Dict_AddName(bs, "S", "I");
      break;
   case HPDF_BS_UNDERLINED:
      ret += HPDF_Dict_AddName(bs, "S", "U");
      break;
   default:
      return  HPDF_SetError(annot->error, HPDF_ANNOT_INVALID_BORDER_STYLE, 0);
   }

   if (width != HPDF_BS_DEF_WIDTH)
      ret += HPDF_Dict_AddReal(bs, "W", width);

   if (ret != HPDF_OK)
      return HPDF_Error_GetCode(annot->error);

   return HPDF_OK;
}

HPDF_Annotation
   HPDF_WidgetAnnot_New(
      HpdfMemMgr * const mmgr,
      HPDF_Xref         xref,
      HpdfRect const rect)
{
   HPDF_Annotation annot;

   HPDF_PTRACE((" HPDF_WidgetAnnot_New\n"));

   annot = HPDF_Annotation_New(mmgr, xref, HPDF_ANNOT_WIDGET, rect);
   if (!annot)
   {
      return NULL;
   }

   return annot;
}

HPDF_Annotation
   HPDF_LinkAnnot_New(
      HpdfMemMgr * const mmgr,
      HPDF_Xref         xref,
      HpdfRect const rect,
      HpdfDestination * const dst)
{
   HPDF_Annotation annot;

   HPDF_PTRACE((" HPDF_LinkAnnot_New\n"));

   annot = HPDF_Annotation_New(mmgr, xref, HPDF_ANNOT_LINK, rect);
   if (!annot)
   {
      return NULL;
   }

   if (dst)
   {
      if (HPDF_Dict_Add(annot, "Dest", dst) != HPDF_OK)
      {
         return NULL;
      }
   }

   return annot;
}

HPDF_Annotation
   HPDF_URILinkAnnot_New(
      HpdfMemMgr * const mmgr,
      HPDF_Xref          xref,
      HpdfRect const rect,
      char const  *uri)
{
   HPDF_Annotation annot;
   HPDF_Dict action;
   HpdfStatus ret;

   HPDF_PTRACE((" HPDF_URILinkAnnot_New\n"));

   annot = HPDF_Annotation_New(mmgr, xref, HPDF_ANNOT_LINK, rect);
   if (!annot)
   {
      return NULL;
   }

   /* create action dictionary */
   action = HPDF_Dict_New(mmgr);
   if (!action)
   {
      return NULL;
   }

   ret = HPDF_Dict_Add(annot, "A", action);
   if (ret != HPDF_OK)
   {
      return NULL;
   }

   ret += HPDF_Dict_AddName(action, "Type", "Action");
   ret += HPDF_Dict_AddName(action, "S", "URI");
   ret += HPDF_Dict_Add(    action, "URI", HpdfObjStringCreate(mmgr, uri, NULL));

   if (ret != HPDF_OK)
   {
      return NULL;
   }

   return annot;
}

HPDF_EXPORT(HpdfStatus)
HPDF_LinkAnnot_SetJavaScript(HPDF_Annotation annot, HPDF_JavaScript javascript)
{
   HPDF_Dict action;
   HpdfStatus ret;

   HPDF_PTRACE((" HPDF_LinkAnnot_SetJavaScript\n"));

   if (!CheckSubType(annot, HPDF_ANNOT_LINK))
      return HPDF_INVALID_ANNOTATION;

   /* create action dictionary */
   action = HPDF_Dict_New(annot->mmgr);
   if (!action)
      return HPDF_CheckError(annot->error);

   ret = HPDF_Dict_Add(annot, "A", action);
   if (ret != HPDF_OK)
      return HPDF_CheckError(annot->error);

   ret += HPDF_Dict_Add(action, "JS", javascript);
   ret += HPDF_Dict_AddName(action, "S", "JavaScript");

   if (ret != HPDF_OK)
      return HPDF_CheckError(annot->error);

   return HPDF_OK;
}

HPDF_EXPORT(HpdfStatus)
HPDF_LinkAnnot_SetBorderStyle(HPDF_Annotation  annot,
   HpdfReal        width,
   HpdfUInt16      dash_on,
   HpdfUInt16      dash_off)
{
   HpdfArray *array;
   HpdfStatus ret;

   HPDF_PTRACE((" HPDF_LinkAnnot_SetBorderStyle\n"));

   if (!CheckSubType(annot, HPDF_ANNOT_LINK))
      return HPDF_INVALID_ANNOTATION;

   if (width < 0)
      return HPDF_RaiseError(annot->error, HPDF_INVALID_PARAMETER, 0);

   array = HpdfArrayCreate(annot->mmgr);
   if (!array)
   {
      return HPDF_CheckError(annot->error);
   }

   if ((ret = HPDF_Dict_Add(annot, "Border", array)) != HPDF_OK)
      return HPDF_CheckError(annot->error);

   ret += HpdfArrayAddNumber(array, 0);
   ret += HpdfArrayAddNumber(array, 0);
   ret += HpdfArrayAddReal(array, width);

   if (ret != HPDF_OK)
      return HPDF_CheckError(annot->error);

   if (dash_on && dash_off) 
   {
      HpdfArray *dash = HpdfArrayCreate(annot->mmgr);
      if (!dash)
      {
         return HPDF_CheckError(annot->error);
      }

      ret = HpdfArrayAdd(array, dash);
      if (ret != HPDF_OK)
      {
         return HPDF_CheckError(annot->error);
      }

      ret += HpdfArrayAddNumber(dash, dash_on);
      ret += HpdfArrayAddNumber(dash, dash_off);

      if (ret != HPDF_OK)
         return HPDF_CheckError(annot->error);
   }

   return HPDF_OK;
}

HPDF_EXPORT(HpdfStatus)
HPDF_LinkAnnot_SetHighlightMode(HPDF_Annotation           annot,
   HPDF_AnnotHighlightMode  mode)
{
   HpdfStatus ret;

   HPDF_PTRACE((" HPDF_LinkAnnot_SetHighlightMode\n"));

   if (!CheckSubType(annot, HPDF_ANNOT_LINK))
      return HPDF_INVALID_ANNOTATION;

   switch (mode) {
   case HPDF_ANNOT_NO_HIGHTLIGHT:
      ret = HPDF_Dict_AddName(annot, "H", "N");
      break;
   case HPDF_ANNOT_INVERT_BORDER:
      ret = HPDF_Dict_AddName(annot, "H", "O");
      break;
   case HPDF_ANNOT_DOWN_APPEARANCE:
      ret = HPDF_Dict_AddName(annot, "H", "P");
      break;
   default:  /* HPDF_ANNOT_INVERT_BOX */
      /* default value */
      HPDF_Dict_RemoveElement(annot, "H");
      ret = HPDF_OK;
   }

   if (ret != HPDF_OK)
      return HPDF_CheckError(annot->error);

   return ret;
}


HPDF_Annotation
   HPDF_3DAnnot_New(
      HpdfMemMgr * const mmgr,
      HPDF_Xref        xref,
      HpdfRect const rect,
      HpdfBool        tb,
      HpdfBool        np,
      HPDF_U3D         u3d,
      HPDF_Image       ap)
{
   HPDF_Annotation annot;
   HPDF_Dict       action, 
                   appearance, 
                   stream;
   HpdfStatus      ret;

   HPDF_PTRACE((" HPDF_3DAnnot_New\n"));

   annot = HPDF_Annotation_New(mmgr, xref, HPDF_ANNOT_3D, rect);
   if (!annot) 
   {
      return NULL;
   }

   // include the flags
   HPDF_Dict_AddNumber(annot, "F", 68);
   //Bit 3:Print If set, print the annotation when the page is printed.
   //Bit 7:If set, do not allow the annotation to interact with the user.
   //      The annotation may be displayed or printed (depending on the settings of the NoView and Print flags)
   //      but should not respond to mouse clicks or change its appearance in response to mouse motions.

   HPDF_Dict_Add(annot, "Contents", HpdfObjStringCreate(mmgr, "3D Model", NULL));

   action = HPDF_Dict_New(mmgr);
   if (!action) 
   {
      return NULL;
   }

   ret = HPDF_Dict_Add(annot, "3DA", action);
   if (ret != HPDF_OK) 
   {
      return NULL;
   }

   // enable visibility on page open
   ret += HPDF_Dict_AddName(action, "A", "PO");

   // enable visibility of ToolBar
   ret += HPDF_Dict_AddBoolean(action, "TB", tb);

   // enable visibility of Navigation Panel
   ret += HPDF_Dict_AddBoolean(action, "NP", np);

   // Set behavior of Annotation on Disabling
   ret += HPDF_Dict_AddName(action, "DIS", "U");

   // Set behavior of Annotation upon activation
   ret += HPDF_Dict_AddName(action, "AIS", "L");

   if (ret != HPDF_OK) 
   {
      return NULL;
   }

   if (HPDF_Dict_Add(annot, "3DD", u3d) != HPDF_OK) 
   {
      return NULL;
   }

   appearance = HPDF_Dict_New(mmgr);
   if (!appearance) 
   {
      return NULL;
   }

   ret = HPDF_Dict_Add(annot, "AP", appearance);
   if (ret != HPDF_OK) 
   {
      return NULL;
   }

   if (ap) 
   {
      if (HPDF_Dict_Add(appearance, "N", ap) != HPDF_OK)
      {
         return NULL;
      }
   }
   else 
   {
      stream = HPDF_Dict_New(mmgr);
      if (!stream) 
      {
         return NULL;
      }
      ret = HPDF_Dict_Add(appearance, "N", stream);
   }

   if (ret != HPDF_OK) 
   {
      return NULL;
   }

   return annot;
}

HPDF_Annotation
   HPDF_MarkupAnnot_New(
      HpdfMemMgr * const    mmgr,
      HPDF_Xref             xref,
      HpdfRect const        rect,
      char const           *text,
      HpdfEncoder * const   encoder,
      HPDF_AnnotType        subtype)
{
   HPDF_Annotation    annot;
   HpdfObjString   *s;

   HPDF_PTRACE((" HPDF_MarkupAnnot_New\n"));

   annot = HPDF_Annotation_New(mmgr, xref, subtype, rect);
   if (!annot)
   {
      return NULL;
   }

   s = HpdfObjStringCreate(mmgr, text, encoder);
   if (!s)
   {
      return NULL;
   }

   if (HPDF_Dict_Add(annot, "Contents", s) != HPDF_OK)
   {
      return NULL;
   }

   return annot;
}

HPDF_EXPORT(HpdfStatus)
HPDF_Annot_SetRGBColor(HPDF_Annotation annot, HPDF_RGBColor color)
{
   HpdfArray *cArray;
   HpdfStatus ret = HPDF_OK;

   HPDF_PTRACE((" HPDF_Annot_SetRGBColor\n"));

   cArray = HpdfArrayCreate(annot->mmgr);
   if (!cArray)
   {
      return HPDF_Error_GetCode(annot->error);
   }

   ret += HPDF_Dict_Add(annot, "C", cArray);
   ret += HpdfArrayAddReal(cArray, color.r);
   ret += HpdfArrayAddReal(cArray, color.g);
   ret += HpdfArrayAddReal(cArray, color.b);

   if (ret != HPDF_OK)
      return HPDF_Error_GetCode(annot->error);

   return HPDF_OK;
}

HPDF_EXPORT(HpdfStatus)
HPDF_Annot_SetCMYKColor(HPDF_Annotation annot, HPDF_CMYKColor color)
{
   HpdfArray *cArray;
   HpdfStatus ret = HPDF_OK;

   HPDF_PTRACE((" HPDF_Annot_SetCMYKColor\n"));

   cArray = HpdfArrayCreate(annot->mmgr);
   if (!cArray)
   {
      return HPDF_Error_GetCode(annot->error);
   }

   ret += HPDF_Dict_Add(annot, "C", cArray);
   ret += HpdfArrayAddReal(cArray, color.c);
   ret += HpdfArrayAddReal(cArray, color.m);
   ret += HpdfArrayAddReal(cArray, color.y);
   ret += HpdfArrayAddReal(cArray, color.k);

   if (ret != HPDF_OK)
      return HPDF_Error_GetCode(annot->error);

   return HPDF_OK;
}

HPDF_EXPORT(HpdfStatus)
HPDF_Annot_SetGrayColor(HPDF_Annotation annot, HpdfReal color)
{
   HpdfArray *cArray;
   HpdfStatus ret = HPDF_OK;

   HPDF_PTRACE((" HPDF_Annot_SetGrayColor\n"));

   cArray = HpdfArrayCreate(annot->mmgr);
   if (!cArray)
   {
      return HPDF_Error_GetCode(annot->error);
   }

   ret += HPDF_Dict_Add(annot, "C", cArray);
   ret += HpdfArrayAddReal(cArray, color);

   if (ret != HPDF_OK)
      return HPDF_Error_GetCode(annot->error);

   return HPDF_OK;
}

HPDF_EXPORT(HpdfStatus)
HPDF_Annot_SetNoColor(HPDF_Annotation annot)
{
   HpdfArray *cArray;
   HpdfStatus ret = HPDF_OK;

   HPDF_PTRACE((" HPDF_Annot_SetNoColor\n"));

   cArray = HpdfArrayCreate(annot->mmgr);
   if (!cArray)
   {
      return HPDF_Error_GetCode(annot->error);
   }

   ret = HPDF_Dict_Add(annot, "C", cArray);

   return ret;
}

HPDF_EXPORT(HpdfStatus)
HPDF_TextAnnot_SetIcon(HPDF_Annotation  annot,
   HPDF_AnnotIcon   icon)
{
   HPDF_PTRACE((" HPDF_TextAnnot_SetIcon\n"));

   if (!CheckSubType(annot, HPDF_ANNOT_TEXT_NOTES))
      return HPDF_INVALID_ANNOTATION;

   if (icon >= HPDF_ANNOT_ICON_EOF)
      return HPDF_RaiseError(annot->error, HPDF_ANNOT_INVALID_ICON,
         (HpdfStatus)icon);

   if (HPDF_Dict_AddName(annot, "Name",
      HPDF_ANNOT_ICON_NAMES_NAMES[(HpdfInt)icon]) != HPDF_OK)
      return HPDF_CheckError(annot->error);

   return HPDF_OK;
}

HPDF_EXPORT(HpdfStatus)
   HPDF_TextAnnot_SetOpened(
      HPDF_Annotation  annot,
      HpdfBool        opened)
{
   HpdfObjBool *b;

   HPDF_PTRACE((" HPDF_TextAnnot_SetOpend\n"));

   if (!CheckSubType(annot, HPDF_ANNOT_TEXT_NOTES))
   {
      return HPDF_INVALID_ANNOTATION;
   }

   b = HpdfObjBoolCreate(annot->mmgr, opened);
   if (!b)
   {
      return HPDF_CheckError(annot->error);
   }

   return  HPDF_Dict_Add(annot, "Open", b);
}

HPDF_EXPORT(HpdfStatus)
   HPDF_PopupAnnot_SetOpened(
      HPDF_Annotation  annot,
      HpdfBool        opened)
{
   HpdfObjBool *b;

   HPDF_PTRACE((" HPDF_TextAnnot_SetOpend\n"));

   if (!CheckSubType(annot, HPDF_ANNOT_POPUP))
   {
      return HPDF_INVALID_ANNOTATION;
   }

   b = HpdfObjBoolCreate(annot->mmgr, opened);
   if (!b)
   {
      return HPDF_CheckError(annot->error);
   }

   return  HPDF_Dict_Add(annot, "Open", b);
}

HPDF_EXPORT(HpdfStatus)
HPDF_MarkupAnnot_SetTitle(HPDF_Annotation   annot, char const* name)
{
   HPDF_PTRACE((" HPDF_MarkupAnnot_SetTitle\n"));

   return HPDF_Dict_Add(annot, "T", HpdfObjStringCreate(annot->mmgr, name, NULL));
}

HPDF_EXPORT(HpdfStatus)
HPDF_MarkupAnnot_SetSubject(HPDF_Annotation   annot, char const* name)
{
   HPDF_PTRACE((" HPDF_MarkupAnnot_SetSubject\n"));

   return HPDF_Dict_Add(annot, "Subj", HpdfObjStringCreate(annot->mmgr, name, NULL));
}

HPDF_EXPORT(HpdfStatus)
HPDF_MarkupAnnot_SetCreationDate(HPDF_Annotation   annot, HPDF_Date value)
{
   HPDF_PTRACE((" HPDF_MarkupAnnot_SetCreationDate\n"));

   return HPDF_Info_SetInfoDateAttr(annot, HPDF_INFO_CREATION_DATE, value);
}

HPDF_EXPORT(HpdfStatus)
HPDF_MarkupAnnot_SetTransparency(HPDF_Annotation   annot, HpdfReal value)
{
   HPDF_PTRACE((" HPDF_MarkupAnnot_SetTransparency\n"));

   return HPDF_Dict_AddReal(annot, "CA", value);
}

HPDF_EXPORT(HpdfStatus)
HPDF_MarkupAnnot_SetIntent(HPDF_Annotation  annot,
   HPDF_AnnotIntent  intent)
{
   HPDF_PTRACE((" HPDF_MarkupAnnot_SetIntent\n"));

   if (HPDF_Dict_AddName(annot, "IT",
      HPDF_ANNOT_INTENT_NAMES[(HpdfInt)intent]) != HPDF_OK)
      return HPDF_CheckError(annot->error);

   return HPDF_OK;
}

HPDF_EXPORT(HpdfStatus)
HPDF_MarkupAnnot_SetPopup(HPDF_Annotation  annot,
   HPDF_Annotation  popup)
{
   HPDF_PTRACE((" HPDF_MarkupAnnot_SetPopup\n"));

   return HPDF_Dict_Add(annot, "Popup", popup);
}

HPDF_EXPORT(HpdfStatus)
HPDF_MarkupAnnot_SetInteriorRGBColor(HPDF_Annotation  annot, HPDF_RGBColor color)/* IC with RGB entry */
{
   HpdfArray *cArray;
   HpdfStatus ret = HPDF_OK;

   HPDF_PTRACE((" HPDF_MarkupAnnot_SetInteriorRGBColor\n"));

   cArray = HpdfArrayCreate(annot->mmgr);
   if (!cArray)
   {
      return HPDF_Error_GetCode(annot->error);
   }

   ret += HPDF_Dict_Add(annot, "IC", cArray);
   ret += HpdfArrayAddReal(cArray, color.r);
   ret += HpdfArrayAddReal(cArray, color.g);
   ret += HpdfArrayAddReal(cArray, color.b);

   if (ret != HPDF_OK)
      return HPDF_Error_GetCode(annot->error);

   return HPDF_OK;
}

HPDF_EXPORT(HpdfStatus)
HPDF_MarkupAnnot_SetInteriorCMYKColor(HPDF_Annotation  annot, HPDF_CMYKColor color)/* IC with CMYK entry */
{
   HpdfArray *cArray;
   HpdfStatus ret = HPDF_OK;

   HPDF_PTRACE((" HPDF_MarkupAnnot_SetInteriorCMYKColor\n"));

   cArray = HpdfArrayCreate(annot->mmgr);
   if (!cArray)
   {
      return HPDF_Error_GetCode(annot->error);
   }

   ret += HPDF_Dict_Add(annot, "IC", cArray);
   ret += HpdfArrayAddReal(cArray, color.c);
   ret += HpdfArrayAddReal(cArray, color.m);
   ret += HpdfArrayAddReal(cArray, color.y);
   ret += HpdfArrayAddReal(cArray, color.k);

   if (ret != HPDF_OK)
      return HPDF_Error_GetCode(annot->error);

   return HPDF_OK;
}

HPDF_EXPORT(HpdfStatus)
HPDF_MarkupAnnot_SetInteriorGrayColor(HPDF_Annotation  annot, HpdfReal color)/* IC with Gray entry */
{
   HpdfArray *cArray;
   HpdfStatus ret = HPDF_OK;

   HPDF_PTRACE((" HPDF_MarkupAnnot_SetInteriorGrayColor\n"));

   cArray = HpdfArrayCreate(annot->mmgr);
   if (!cArray)
   {
      return HPDF_Error_GetCode(annot->error);
   }

   ret += HPDF_Dict_Add(annot, "IC", cArray);
   ret += HpdfArrayAddReal(cArray, color);

   if (ret != HPDF_OK)
      return HPDF_Error_GetCode(annot->error);

   return HPDF_OK;
}

HPDF_EXPORT(HpdfStatus)
HPDF_MarkupAnnot_SetInteriorTransparent(HPDF_Annotation  annot) /* IC with No Color entry */
{
   HpdfArray *cArray;
   HpdfStatus ret = HPDF_OK;

   HPDF_PTRACE((" HPDF_MarkupAnnot_SetInteriorTransparent\n"));

   cArray = HpdfArrayCreate(annot->mmgr);
   if (!cArray)
   {
      return HPDF_Error_GetCode(annot->error);
   }

   ret = HPDF_Dict_Add(annot, "IC", cArray);

   return ret;
}

HpdfBool
HPDF_Annotation_Validate(HPDF_Annotation  annot)
{
   HPDF_PTRACE((" HPDF_Annotation_Validate\n"));

   if (!annot)
      return HPDF_FALSE;

   if (annot->header.obj_class !=
      (HPDF_OSUBCLASS_ANNOTATION | HPDF_OCLASS_DICT))
      return HPDF_FALSE;

   return HPDF_TRUE;
}

static HpdfBool
   CheckSubType(
      HPDF_Annotation  annot,
      HPDF_AnnotType  type)
{
   HpdfObjName *subtype;

   HPDF_PTRACE((" HPDF_Annotation_CheckSubType\n"));

   if (!HPDF_Annotation_Validate(annot))
   {
      return HPDF_FALSE;
   }

   subtype = HPDF_Dict_GetItem(annot, "Subtype", HPDF_OCLASS_NAME);

   if (!subtype || 
       !HpdfStrIsEqual(subtype->value, HPDF_ANNOT_TYPE_NAMES[(HpdfInt)type]))
   {
      HPDF_RaiseError(annot->error, HPDF_INVALID_ANNOTATION, 0);
      return HPDF_FALSE;
   }

   return HPDF_TRUE;
}

HPDF_EXPORT(HpdfStatus)
   HPDF_Annot_Set3DView(
      HpdfMemMgr * const mmgr,
      HPDF_Annotation    annot,
      HPDF_Annotation    annot3d,
      HPDF_Dict            view3d)
{
   HPDF_Proxy proxyView3d;
   HPDF_Dict exData = HPDF_Dict_New(mmgr);
   HpdfStatus retS = HPDF_OK;

   retS += HPDF_Dict_AddName(exData, "Type", "ExData");
   retS += HPDF_Dict_AddName(exData, "Subtype", "Markup3D");
   retS += HPDF_Dict_Add(exData, "3DA", annot3d);

   proxyView3d = HPDF_Proxy_New(mmgr, view3d);

   retS += HPDF_Dict_Add(exData, "3DV", proxyView3d);
   retS += HPDF_Dict_Add(annot, "ExData", exData);
   return retS;
}

HPDF_Annotation
   HPDF_PopupAnnot_New(
      HpdfMemMgr * const mmgr,
      HPDF_Xref         xref,
      HpdfRect const rect,
      HPDF_Annotation   parent)
{
   HPDF_Annotation annot;

   HPDF_PTRACE((" HPDF_PopupAnnot_New\n"));

   annot = HPDF_Annotation_New(mmgr, xref, HPDF_ANNOT_POPUP, rect);
   if (!annot)
   {
      return NULL;
   }

   if (HPDF_Dict_Add(annot, "Parent", parent) != HPDF_OK)
   {
      return NULL;
   }

   return annot;
}

HPDF_Annotation
   HPDF_StampAnnot_New(
      HpdfMemMgr * const    mmgr,
      HPDF_Xref             xref,
      HpdfRect const        rect,
      HPDF_StampAnnotName   name,
      char const           *text,
      HpdfEncoder * const   encoder)
{
   HPDF_Annotation    annot;
   HpdfObjString   *s;

   HPDF_PTRACE((" HPDF_StampAnnot_New\n"));

   annot = HPDF_Annotation_New(mmgr, xref, HPDF_ANNOT_STAMP, rect);
   if (!annot)
   {
      return NULL;
   }

   if (HPDF_Dict_AddName(annot, "Name", HPDF_STAMP_ANNOT_NAME_NAMES[name]) != HPDF_OK)
   {
      return NULL;
   }

   s = HpdfObjStringCreate(mmgr, text, encoder);
   if (!s)
   {
      return NULL;
   }

   if (HPDF_Dict_Add(annot, "Contents", s) != HPDF_OK)
   {
      return NULL;
   }

   return annot;
}

HPDF_Annotation
   HPDF_ProjectionAnnot_New(
      HpdfMemMgr * const    mmgr,
      HPDF_Xref             xref,
      HpdfRect const        rect,
      char const           *text,
      HpdfEncoder * const   encoder)
{
   HPDF_Annotation    annot;
   HpdfObjString   *s;

   HPDF_PTRACE((" HPDF_StampAnnot_New\n"));

   annot = HPDF_Annotation_New(mmgr, xref, HPDF_ANNOT_PROJECTION, rect);
   if (!annot)
   {
      return NULL;
   }

   s = HpdfObjStringCreate(mmgr, text, encoder);
   if (!s)
   {
      return NULL;
   }

   if (HPDF_Dict_Add(annot, "Contents", s) != HPDF_OK)
   {
      return NULL;
   }

   return annot;
}


HPDF_EXPORT(HpdfStatus)
   HPDF_TextMarkupAnnot_SetQuadPoints(
      HPDF_Annotation annot,
      HpdfPoint const lb,
      HpdfPoint const rb,
      HpdfPoint const lt,
      HpdfPoint const rt) /* l-left, r-right, b-bottom, t-top positions */
{
   HpdfArray *quadPoints;
   HpdfStatus ret = HPDF_OK;

   HPDF_PTRACE((" HPDF_TextMarkupAnnot_SetQuadPoints\n"));

   quadPoints = HpdfArrayCreate(annot->mmgr);
   if (!quadPoints)
   {
      return HPDF_Error_GetCode(annot->error);
   }

   if ((ret = HPDF_Dict_Add(annot, "QuadPoints", quadPoints)) != HPDF_OK)
   {
      return ret;
   }

   ret += HpdfArrayAddReal(quadPoints, lb.x);
   ret += HpdfArrayAddReal(quadPoints, lb.y);
   ret += HpdfArrayAddReal(quadPoints, rb.x);
   ret += HpdfArrayAddReal(quadPoints, rb.y);
   ret += HpdfArrayAddReal(quadPoints, lt.x);
   ret += HpdfArrayAddReal(quadPoints, lt.y);
   ret += HpdfArrayAddReal(quadPoints, rt.x);
   ret += HpdfArrayAddReal(quadPoints, rt.y);

   if (ret != HPDF_OK)
   {
      return HPDF_Error_GetCode(quadPoints->error);
   }

   return HPDF_OK;
}

HPDF_EXPORT(HpdfStatus)
HPDF_FreeTextAnnot_SetLineEndingStyle(HPDF_Annotation annot, HPDF_LineAnnotEndingStyle startStyle, HPDF_LineAnnotEndingStyle endStyle)
{
   HpdfArray *lineEndStyles;
   HpdfStatus ret = HPDF_OK;

   HPDF_PTRACE((" HPDF_FreeTextAnnot_SetLineEndingStyle\n"));

   lineEndStyles = HpdfArrayCreate(annot->mmgr);
   if (!lineEndStyles)
   {
      return HPDF_Error_GetCode(annot->error);
   }

   if ((ret = HPDF_Dict_Add(annot, "LE", lineEndStyles)) != HPDF_OK)
      return ret;

   ret += HpdfArrayAddName(lineEndStyles, HPDF_LINE_ANNOT_ENDING_STYLE_NAMES[(HpdfInt)startStyle]);
   ret += HpdfArrayAddName(lineEndStyles, HPDF_LINE_ANNOT_ENDING_STYLE_NAMES[(HpdfInt)endStyle]);

   if (ret != HPDF_OK)
   {
      return HPDF_Error_GetCode(lineEndStyles->error);
   }

   return HPDF_OK;
}

HPDF_EXPORT(HpdfStatus)
   HPDF_MarkupAnnot_SetRectDiff(
      HPDF_Annotation  annot, 
      HpdfRect const rect) /* RD entry : this is the difference between Rect and the text annotation rectangle */
{
   HpdfArray *array;
   HpdfStatus ret = HPDF_OK;
   HpdfRect rectTemp;

   HPDF_PTRACE((" HPDF_MarkupAnnot_SetRectDiff\n"));

   array = HpdfArrayCreate(annot->mmgr);
   if (!array)
   {
      return HPDF_Error_GetCode(annot->error);
   }

   if ((ret = HPDF_Dict_Add(annot, "RD", array)) != HPDF_OK)
   {
      return ret;
   }

   rectTemp = rect;
   if (rect.top < rect.bottom) 
   {
      rectTemp.top    = rect.bottom;
      rectTemp.bottom = rect.top;
   }

   ret += HpdfArrayAddReal(array, rectTemp.left);
   ret += HpdfArrayAddReal(array, rectTemp.bottom);
   ret += HpdfArrayAddReal(array, rectTemp.right);
   ret += HpdfArrayAddReal(array, rectTemp.top);

   if (ret != HPDF_OK)
   {
      return HPDF_Error_GetCode(array->error);
   }

   return HPDF_OK;
}

HPDF_EXPORT(HpdfStatus)
   HPDF_FreeTextAnnot_SetDefaultStyle(
      HPDF_Annotation annot,
      char const     *style)
{
   HpdfObjString   *s;
   HpdfStatus         ret = HPDF_OK;

   HPDF_PTRACE((" HPDF_FreeTextAnnot_SetDefaultStyle\n"));

   s = HpdfObjStringCreate(annot->mmgr, style, NULL);
   if (!s)
   {
      return HPDF_Error_GetCode(annot->error);
   }

   ret = HPDF_Dict_Add(annot, "DS", s);

   return ret;
}

HPDF_EXPORT(HpdfStatus)
   HPDF_FreeTextAnnot_Set3PointCalloutLine(
      HPDF_Annotation annot,
      HpdfPoint const startPoint, 
      HpdfPoint const kneePoint, 
      HpdfPoint const endPoint) /* Callout line will be in default user space */
{
   HpdfArray *clPoints;
   HpdfStatus ret = HPDF_OK;

   HPDF_PTRACE((" HPDF_FreeTextAnnot_Set3PointCalloutLine\n"));

   clPoints = HpdfArrayCreate(annot->mmgr);
   if (!clPoints)
   {
      return HPDF_Error_GetCode(annot->error);
   }

   if ((ret = HPDF_Dict_Add(annot, "CL", clPoints)) != HPDF_OK)
   {
      return ret;
   }

   ret += HpdfArrayAddReal(clPoints, startPoint.x);
   ret += HpdfArrayAddReal(clPoints, startPoint.y);
   ret += HpdfArrayAddReal(clPoints, kneePoint.x);
   ret += HpdfArrayAddReal(clPoints, kneePoint.y);
   ret += HpdfArrayAddReal(clPoints, endPoint.x);
   ret += HpdfArrayAddReal(clPoints, endPoint.y);

   if (ret != HPDF_OK)
   {
      return HPDF_Error_GetCode(clPoints->error);
   }

   return HPDF_OK;
}

HPDF_EXPORT(HpdfStatus)
   HPDF_FreeTextAnnot_Set2PointCalloutLine(
      HPDF_Annotation annot, 
      HpdfPoint const startPoint, 
      HpdfPoint const endPoint) /* Callout line will be in default user space */
{
   HpdfArray *clPoints;
   HpdfStatus ret = HPDF_OK;

   HPDF_PTRACE((" HPDF_FreeTextAnnot_Set3PointCalloutLine\n"));

   clPoints = HpdfArrayCreate(annot->mmgr);
   if (!clPoints)
   {
      return HPDF_Error_GetCode(annot->error);
   }

   if ((ret = HPDF_Dict_Add(annot, "CL", clPoints)) != HPDF_OK)
   {
      return ret;
   }

   ret += HpdfArrayAddReal(clPoints, startPoint.x);
   ret += HpdfArrayAddReal(clPoints, startPoint.y);
   ret += HpdfArrayAddReal(clPoints, endPoint.x);
   ret += HpdfArrayAddReal(clPoints, endPoint.y);

   if (ret != HPDF_OK)
   {
      return HPDF_Error_GetCode(clPoints->error);
   }

   return HPDF_OK;
}

HPDF_EXPORT(HpdfStatus)
HPDF_MarkupAnnot_SetCloudEffect(HPDF_Annotation  annot, HpdfInt cloudIntensity) /* BE entry */
{
   HPDF_Dict borderEffect;
   HpdfStatus ret = HPDF_OK;

   HPDF_PTRACE((" HPDF_MarkupAnnot_SetCloudEffect\n"));

   borderEffect = HPDF_Dict_New(annot->mmgr);
   if (!borderEffect)
      return HPDF_Error_GetCode(annot->error);

   ret += HPDF_Dict_Add(annot, "BE", borderEffect);
   ret += HPDF_Dict_AddName(borderEffect, "S", "C");
   ret += HPDF_Dict_AddNumber(borderEffect, "I", cloudIntensity);

   if (ret != HPDF_OK)
      return HPDF_Error_GetCode(annot->error);

   return HPDF_OK;
}

HPDF_EXPORT(HpdfStatus)
   HPDF_LineAnnot_SetPosition(
      HPDF_Annotation annot,
      HpdfPoint const startPoint, HPDF_LineAnnotEndingStyle startStyle,
      HpdfPoint const endPoint,   HPDF_LineAnnotEndingStyle endStyle)
{
   HpdfArray *lineEndPoints;
   HpdfArray *lineEndStyles;
   HpdfStatus ret = HPDF_OK;

   HPDF_PTRACE((" HPDF_LineAnnot_SetPosition\n"));

   lineEndPoints = HpdfArrayCreate(annot->mmgr);
   if (!lineEndPoints)
   {
      return HPDF_Error_GetCode(annot->error);
   }

   if ((ret = HPDF_Dict_Add(annot, "L", lineEndPoints)) != HPDF_OK)
   {
      return ret;
   }

   ret += HpdfArrayAddReal(lineEndPoints, startPoint.x);
   ret += HpdfArrayAddReal(lineEndPoints, startPoint.y);
   ret += HpdfArrayAddReal(lineEndPoints, endPoint.x);
   ret += HpdfArrayAddReal(lineEndPoints, endPoint.y);

   if (ret != HPDF_OK)
   {
      return HPDF_Error_GetCode(lineEndPoints->error);
   }

   lineEndStyles = HpdfArrayCreate(annot->mmgr);
   if (!lineEndStyles)
   {
      return HPDF_Error_GetCode(annot->error);
   }

   if ((ret = HPDF_Dict_Add(annot, "LE", lineEndStyles)) != HPDF_OK)
   {
      return ret;
   }

   ret += HpdfArrayAddName(lineEndStyles, HPDF_LINE_ANNOT_ENDING_STYLE_NAMES[(HpdfInt)startStyle]);
   ret += HpdfArrayAddName(lineEndStyles, HPDF_LINE_ANNOT_ENDING_STYLE_NAMES[(HpdfInt)endStyle]);

   if (ret != HPDF_OK)
   {
      return HPDF_Error_GetCode(lineEndStyles->error);
   }

   return HPDF_OK;
}

HPDF_EXPORT(HpdfStatus)
HPDF_LineAnnot_SetLeader(HPDF_Annotation annot, HpdfInt leaderLen, HpdfInt leaderExtLen, HpdfInt leaderOffsetLen)
{
   HpdfStatus ret = HPDF_OK;

   HPDF_PTRACE((" HPDF_LineAnnot_SetLeader\n"));

   ret += HPDF_Dict_AddNumber(annot, "LL", leaderLen);
   ret += HPDF_Dict_AddNumber(annot, "LLE", leaderExtLen);
   ret += HPDF_Dict_AddNumber(annot, "LLO", leaderOffsetLen);

   if (ret != HPDF_OK)
      return HPDF_Error_GetCode(annot->error);

   return HPDF_OK;
}

HPDF_EXPORT(HpdfStatus)
HPDF_LineAnnot_SetCaption(HPDF_Annotation annot, HpdfBool showCaption, HPDF_LineAnnotCapPosition position, HpdfInt horzOffset, HpdfInt vertOffset)
{
   HpdfStatus ret = HPDF_OK;
   HpdfArray *capOffset;
   HPDF_PTRACE((" HPDF_LineAnnot_SetCaption\n"));

   ret += HPDF_Dict_AddBoolean(annot, "Cap", showCaption);
   ret += HPDF_Dict_AddName(annot, "CP", HPDF_LINE_ANNOT_CAP_POSITION_NAMES[(HpdfInt)position]);

   if (ret != HPDF_OK)
      return HPDF_Error_GetCode(annot->error);

   capOffset = HpdfArrayCreate(annot->mmgr);
   if (!capOffset)
   {
      return HPDF_Error_GetCode(annot->error);
   }

   if ((ret = HPDF_Dict_Add(annot, "CO", capOffset)) != HPDF_OK)
      return ret;

   ret += HpdfArrayAddNumber(capOffset, horzOffset);
   ret += HpdfArrayAddNumber(capOffset, vertOffset);

   if (ret != HPDF_OK)
      return HPDF_Error_GetCode(capOffset->error);

   return HPDF_OK;
}



HPDF_EXPORT(HpdfStatus)
HPDF_ProjectionAnnot_SetExData(HPDF_Annotation annot, HPDF_ExData exdata)
{
   HpdfStatus ret = HPDF_OK;

   ret = HPDF_Dict_Add(annot, "ExData", exdata);

   return ret;
}
