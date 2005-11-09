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

using namespace std;

#include "PictureAlgebra.h"
#include "JPEGPicture.h"
#include "NestedList.h"
#include "QueryProcessor.h"

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
	pic->jpegData = 0;
	pic->isDefined = false;
	return;
    }

    unsigned long size	= 0;
    char *buffer	= GetJPEGData(size);
    // create picture
    JPEGPicture *jpg	= new JPEGPicture((unsigned char*)buffer, (unsigned long)size);

    // scale
    JPEGPicture *scale = jpg->scale(w, h);

    // delete picture
    delete jpg;

    // generate scaled jpg
    buffer = (char*)scale->GetJpegData(size);

    // delete scale
    delete scale;

    pic->Set(buffer, 
	     size, 
	     filename, 
	     category, 
	     w > h ? false : true, 
	     date);
}

void Picture::Cut(Picture *pic, int x, int y, int w, int h) {
    if (PA_DEBUG) cerr << "Picture::Cut() called" << endl;

    // check for invalid parameters (further checks done in jpg->cut() later)
    if (x < 0 || y < 0 || w <= 0 || h <= 0) {
	if (PA_DEBUG) cerr << "Picture::Cut() error #1" << endl;

	//pic->CopyFrom((StandardAttribute*) this);
	pic->jpegData = 0;
	pic->isDefined = false;
	return;
    }

    unsigned long size	= 0;
    char *buffer	= GetJPEGData(size);
    // create picture
    JPEGPicture *jpg	= new JPEGPicture((unsigned char*)buffer, (unsigned long)size);

    // cut the image
    JPEGPicture *cut = jpg->cut(x, y, w, h);

    // delete picture
    delete jpg;

    // check for errors
    if (cut == 0) {
	if (PA_DEBUG) cerr << "Picture::Cut() error #2" << endl;

	pic->jpegData = 0;
	pic->isDefined = false;
	return;
    }

    // generate cutted jpg
    buffer = (char*)cut->GetJpegData(size);

    // delete image
    delete cut;

    pic->Set(buffer, 
	     size, 
	     filename, 
	     category, 
	     w > h ? false : true, 
	     date);
}

void Picture::FlipLeft(Picture *pic, int n) {
    if (PA_DEBUG) cerr << "Picture:FlipLeft() called" << endl;

    // check for invalid parameters 
    if (n < 0) {
	pic->jpegData = 0;
	pic->isDefined = false;
	return;
    }

    unsigned long size	= 0;
    char *buffer	= GetJPEGData(size);
    // create picture
     JPEGPicture *jpg	= new JPEGPicture((unsigned char*)buffer, (unsigned long)size);

    // flip
    JPEGPicture *flip = jpg->flipleft((unsigned long)n);

    // delete picture
    delete jpg;

    // generate flipped jpg
    buffer = (char*)flip->GetJpegData(size);

    // delete flip
    delete flip;

    pic->Set(buffer, 
	     size, 
	     filename, 
	     category, 
	     n % 2 == 0 ? isPortrait : !isPortrait, 
	     date);
}

void Picture::Mirror(Picture *pic, bool dir) {
    if (PA_DEBUG) cerr << "Picture::Mirror() called" << endl;

    unsigned long size	= 0;
    char *buffer	= GetJPEGData(size);
    // create picture
    JPEGPicture *jpg	= new JPEGPicture((unsigned char*)buffer, (unsigned long)size);

    // mirror
    JPEGPicture *mirror = jpg->mirror(dir);

    // delete picture
    delete jpg;

    // generate mirrored jpg
    buffer = (char*)mirror->GetJpegData(size);

    // delete mirror
    delete mirror;

    pic->Set(buffer, size, filename, category, isPortrait, date);
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
      if (nl->IsEqual(nl->First(args), "picture") &&
          nl->IsEqual(nl->Second(args), "int") &&
	  nl->IsEqual(nl->Third(args), "int"))
      {
	  return nl->SymbolAtom("picture");
      }
      else
      {
      	string s;
	nl->WriteToString(s, args);
	ErrorReporter::ReportError("expected 'picture x [int int]' as argument");
        return nl->SymbolAtom("typeerror");
      }
    }
    else
    {
      ErrorReporter::ReportError(
	    "expected three arguments but received "
	    +nl->ListLength(args));
    }

    return nl->SymbolAtom("typeerror");
}

ListExpr PictureCutTypeMap(ListExpr args) {
    if (PA_DEBUG) cerr << "PictureCutTypeMap() called" << endl;

    if ( nl->ListLength(args) == 5 )
    {
      if (nl->IsEqual(nl->First(args), "picture") &&
          nl->IsEqual(nl->Second(args), "int") &&
          nl->IsEqual(nl->Third(args), "int") &&
          nl->IsEqual(nl->Fourth(args), "int") &&
	  nl->IsEqual(nl->Fifth(args), "int"))
      {
	  return nl->SymbolAtom("picture");
      }
      else
      {
      	string s;
	nl->WriteToString(s, args);
	ErrorReporter::ReportError("expected 'picture x [int int int int]' as argument");
        return nl->SymbolAtom("typeerror");
      }
    }
    else
    {
      ErrorReporter::ReportError(
	    "expected five arguments but received "
	    +nl->ListLength(args));
    }

    return nl->SymbolAtom("typeerror");
}

ListExpr PictureFlipleftTypeMap(ListExpr args) {
    if (PA_DEBUG) cerr << "PictureFlipleftTypeMap() called" << endl;

    if ( nl->ListLength(args) == 2 )
    {
      if (nl->IsEqual(nl->First(args), "picture") &&
	  nl->IsEqual(nl->Second(args), "int"))
      {
	  return nl->SymbolAtom("picture");
      }
      else
      {
      	string s;
	nl->WriteToString(s, args);
	ErrorReporter::ReportError("expected 'picture x [int]' as argument");
        return nl->SymbolAtom("typeerror");
      }
    }
    else
    {
      ErrorReporter::ReportError(
	    "expected two arguments but received "
	    +nl->ListLength(args));
    }

    return nl->SymbolAtom("typeerror");
}

ListExpr PictureMirrorTypeMap(ListExpr args) {
    if (PA_DEBUG) cerr << "PictureMirrorTypeMap() called" << endl;

    if ( nl->ListLength(args) == 2 )
    {
      if (nl->IsEqual(nl->First(args), "picture") &&
	  nl->IsEqual(nl->Second(args), "bool"))
      {
	  return nl->SymbolAtom("picture");
      }
      else
      {
      	string s;
	nl->WriteToString(s, args);
	ErrorReporter::ReportError("expected 'picture x [bool]' as argument");
        return nl->SymbolAtom("typeerror");
      }
    }
    else
    {
      ErrorReporter::ReportError(
	    "expected two arguments but received "
	    +nl->ListLength(args));
    }

    return nl->SymbolAtom("typeerror");
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

