/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[ue] [\"{u}]
//[ae] [\"{a}]
//[TOC] [\tableofcontents]

[1] Picture Algebra: Class Definitions

Dezember 2004 Christian Bohnbuck, Uwe Hartmann, Marion Langen and Holger
M[ue]nx during Prof. G[ue]ting's practical course
'Extensible Database Systems' at Fernuniversit[ae]t Hagen.

[TOC]

1 Introduction

See the documentation of ~PictureAlgebra.h~ for a general introduction to
the Picture algebra.

This module contains SECONDO operators on ~picture~, which
deal with graphical operations like scale, cut, mirror and flipleft.

2 Includes and other preparations

*/


#include "PictureAlgebra.h"
#include "JPEGPicture.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "StringUtils.h"

using namespace std;

extern NestedList* nl;
extern QueryProcessor *qp;

/*

3 Implementation of class ~Picture~

Please note that other methods are located in other modules of this
algebra!

See the documentation of ~PictureAlgebra.h~ for details on the behaviour
of the methods implemented here.

*/

void Picture::Scale(Picture *pic, int w, int h) {
    if (PA_DEBUG) cerr << "Picture::Scale() called" << endl;

    // check for invalid parameters
    if (w <= 0 || h <= 0) {
       pic->jpegData.clean();
       pic->SetDefined(false);
       return;
    }

    unsigned long size      = 0;
    char *buffer = GetJPEGData(size);
    // create picture
    JPEGPicture *jpg = new JPEGPicture((unsigned char*)buffer,
                                       (unsigned long)size);
    delete[] buffer;

    // scale
    JPEGPicture *scale = jpg->scale(w, h);

    // delete picture
    delete jpg;

    // generate scaled jpg
    char *buf = (char *)scale->GetJpegData(size);

    // delete scale
    delete scale;

    pic->Set(buf,
           size,
           filename,
           category,
           w > h ? false : true,
           date);
    delete[] buf;
}

void Picture::Cut(Picture *pic, int x, int y, int w, int h) {
    if (PA_DEBUG) cerr << "Picture::Cut() called" << endl;

    // check for invalid parameters (further checks done in jpg->cut() later)
    if (x < 0 || y < 0 || w <= 0 || h <= 0) {
      if (PA_DEBUG) cerr << "Picture::Cut() error #1" << endl;

      pic->jpegData.clean();
      pic->SetDefined(false);
      return;
    }

    unsigned long size      = 0;
    char* buffer      = GetJPEGData(size);
    // create picture
    JPEGPicture *jpg  = new JPEGPicture((unsigned char*)buffer,
                                        (unsigned long)size);
    delete[] buffer;

    // cut the image
    JPEGPicture *cut = jpg->cut(x, y, w, h);

    // delete picture
    delete jpg;

    // check for errors
    if (cut == 0) {
      if (PA_DEBUG) cerr << "Picture::Cut() error #2" << endl;

      pic->jpegData.clean();
      pic->SetDefined(false);
      return;
    }

    // generate cutted jpg
    char *buf = (char*)cut->GetJpegData(size);

    // delete image
    delete cut;

    pic->Set(buf,
           size,
           filename,
           category,
           w > h ? false : true,
           date);
     delete[] buf;
}

void Picture::FlipLeft(Picture *pic, int n) {
    if (PA_DEBUG) cerr << "Picture:FlipLeft() called" << endl;

    // check for invalid parameters
    if (n < 0) {
      pic->jpegData.clean();
      pic->SetDefined(false);
      return;
    }

    unsigned long size      = 0;
    char *buffer      = GetJPEGData(size);
    // create picture
     JPEGPicture *jpg = new JPEGPicture((unsigned char*)buffer,
                                        (unsigned long)size);

     delete[] buffer;
    // flip
    JPEGPicture *flip = jpg->flipleft((unsigned long)n);

    // delete picture
    delete jpg;

    // generate flipped jpg
    char *buf = (char*)flip->GetJpegData(size);

    // delete flip
    delete flip;

    pic->Set(buf,
           size,
           filename,
           category,
           n % 2 == 0 ? isPortrait : !isPortrait,
           date);
    delete[] buf;
}

void Picture::Mirror(Picture *pic, bool dir) {
    if (PA_DEBUG) cerr << "Picture::Mirror() called" << endl;

    unsigned long size      = 0;
    char *buffer      = GetJPEGData(size);
    // create picture
    JPEGPicture *jpg  = new JPEGPicture((unsigned char*)buffer,
                                        (unsigned long)size);
    delete[] buffer;

    // mirror
    JPEGPicture *mirror = jpg->mirror(dir);

    // delete picture
    delete jpg;

    // generate mirrored jpg
    char *buf = (char*)mirror->GetJpegData(size);

    // delete mirror
    delete mirror;

    pic->Set(buf, size, filename, category, isPortrait, date);
    delete[] buf;
}

/*

4 Type mapping functions

These functions check if the given nested list parameter structure is in
correct type of parameters for the operators scale, cut, mirror and flitleft.

*/

ListExpr PictureScaleTypeMap(ListExpr args) {
    if (PA_DEBUG) cerr << "PictureScaleTypeMap() called" << endl;

    if ( nl->ListLength(args) == 3 )
    {
      if (nl->IsEqual(nl->First(args), Picture::BasicType()) &&
          nl->IsEqual(nl->Second(args), CcInt::BasicType()) &&
        nl->IsEqual(nl->Third(args), CcInt::BasicType()))
      {
        return nl->SymbolAtom(Picture::BasicType());
      }
      else
      {
            string s;
      nl->WriteToString(s, args);
      ErrorReporter::ReportError("expected 'picture x [int int]' as argument");
        return nl->SymbolAtom(Symbol::TYPEERROR());
      }
    }
    else
    {
      ErrorReporter::ReportError(
          "expected three arguments but received "
          +stringutils::int2str(nl->ListLength(args)));
    }

    return nl->SymbolAtom(Symbol::TYPEERROR());
}

ListExpr PictureCutTypeMap(ListExpr args) {
    if (PA_DEBUG) cerr << "PictureCutTypeMap() called" << endl;

    if ( nl->ListLength(args) == 5 )
    {
      if (nl->IsEqual(nl->First(args), Picture::BasicType()) &&
          nl->IsEqual(nl->Second(args), CcInt::BasicType()) &&
          nl->IsEqual(nl->Third(args), CcInt::BasicType()) &&
          nl->IsEqual(nl->Fourth(args), CcInt::BasicType()) &&
        nl->IsEqual(nl->Fifth(args), CcInt::BasicType()))
      {
        return nl->SymbolAtom(Picture::BasicType());
      }
      else
      {
            string s;
      nl->WriteToString(s, args);
      ErrorReporter::ReportError("expected 'picture x "
                                 "[int int int int]' as argument");
        return nl->SymbolAtom(Symbol::TYPEERROR());
      }
    }
    else
    {
      ErrorReporter::ReportError(
          "expected five arguments but received "
          +stringutils::int2str(nl->ListLength(args)));
    }

    return nl->SymbolAtom(Symbol::TYPEERROR());
}

ListExpr PictureFlipleftTypeMap(ListExpr args) {
    if (PA_DEBUG) cerr << "PictureFlipleftTypeMap() called" << endl;

    if ( nl->ListLength(args) == 2 )
    {
      if (nl->IsEqual(nl->First(args), Picture::BasicType()) &&
        nl->IsEqual(nl->Second(args), CcInt::BasicType()))
      {
        return nl->SymbolAtom(Picture::BasicType());
      }
      else
      {
            string s;
      nl->WriteToString(s, args);
      ErrorReporter::ReportError("expected 'picture x [int]' as argument");
        return nl->SymbolAtom(Symbol::TYPEERROR());
      }
    }
    else
    {
      ErrorReporter::ReportError(
          "expected two arguments but received "
          +stringutils::int2str(nl->ListLength(args)));
    }

    return nl->SymbolAtom(Symbol::TYPEERROR());
}

ListExpr PictureMirrorTypeMap(ListExpr args) {
    if (PA_DEBUG) cerr << "PictureMirrorTypeMap() called" << endl;

    if ( nl->ListLength(args) == 2 )
    {
      if (nl->IsEqual(nl->First(args), Picture::BasicType()) &&
        nl->IsEqual(nl->Second(args), CcBool::BasicType()))
      {
        return nl->SymbolAtom(Picture::BasicType());
      }
      else
      {
            string s;
      nl->WriteToString(s, args);
      ErrorReporter::ReportError("expected 'picture x [bool]' as argument");
        return nl->SymbolAtom(Symbol::TYPEERROR());
      }
    }
    else
    {
      ErrorReporter::ReportError(
          "expected two arguments but received "
          +stringutils::int2str(nl->ListLength(args)));
    }

    return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*

5 Type mapping functions

The following functions are the Value Mapping functions for the
operators scale, cut, flipleft and mirror.

*/

int PictureCutValueMap(Word* args,
                   Word& result,
                   int message,
                   Word& local,
                   Supplier s) {
    if (PA_DEBUG) cerr << "PictureCutValueMap() called" << endl;

    Picture *pic;
    CcInt* w,*x;
    CcInt* h,*y;
    pic = ((Picture*)args[0].addr);
    x = ((CcInt*)args[1].addr);
    y = ((CcInt*)args[2].addr);
    w = ((CcInt*)args[3].addr);
    h = ((CcInt*)args[4].addr);

    result = qp->ResultStorage(s);
    Picture* res = ((Picture*)result.addr);

    if ( pic->IsDefined() )
          pic->Cut(res, x->GetIntval(), y->GetIntval(),
                   w->GetIntval(), h->GetIntval());
    else
      res->SetDefined( false );

    return 0;
}

int PictureScaleValueMap(Word* args,
                   Word& result,
                   int message,
                   Word& local,
                   Supplier s) {
    if (PA_DEBUG) cerr << "PictureScaleValueMap() called" << endl;

    Picture *pic;
    CcInt* w;
    CcInt* h;
    pic = ((Picture*)args[0].addr);
    w = ((CcInt*)args[1].addr);
    h = ((CcInt*)args[2].addr);

    result = qp->ResultStorage(s);
    Picture* res = ((Picture*)result.addr);

    if ( pic->IsDefined() )
          pic->Scale(res, w->GetIntval(), h->GetIntval());
    else
      res->SetDefined( false);

    return 0;
}

int PictureFlipleftValueMap(Word* args,
                      Word& result,
                      int message,
                      Word& local,
                      Supplier s) {
    if (PA_DEBUG) cerr << "PictureFlipleftValueMap() called" << endl;

    Picture *pic;
    CcInt* n;
    pic = ((Picture*)args[0].addr);
    n = ((CcInt*)args[1].addr);

    result = qp->ResultStorage(s);
    Picture* res = ((Picture*)result.addr);

    if ( pic->IsDefined() )
          pic->FlipLeft(res, n->GetIntval());
    else
      res->SetDefined( false );

    return 0;
}

int PictureMirrorValueMap(Word* args,
                    Word& result,
                    int message,
                    Word& local,
                    Supplier s) {
    if (PA_DEBUG) cerr << "PictureMirrorValueMap() called" << endl;


    Picture *pic;
    CcBool* dir;
    pic = ((Picture*)args[0].addr);
    dir = ((CcBool*)args[1].addr);

    result = qp->ResultStorage(s);
    Picture* res = ((Picture*)result.addr);
    if ( pic->IsDefined() )
       pic->Mirror(res, dir->GetBoolval());
    else
      res->SetDefined( false );
    return 0;
}

