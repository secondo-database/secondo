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
deal with basic attributes such as image heigh or width.

2 Includes and other preparations

*/

using namespace std;

#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "NestedList.h"
#include "JPEGPicture.h"
#include "PictureAlgebra.h"

extern NestedList* nl;
extern QueryProcessor *qp;

/*

3 Implementation of class ~Picture~

Please note that other methods are located in other modules of this
algebra!

See the documentation of ~PictureAlgebra.h~ for details on the behaviour
of the methods implemented here.

M. Spiekermann. The attributes width, height, etc. are stored
in private member variables. 

*/

int Picture::GetWidth(void) {
    if (PA_DEBUG) cerr << "Picture::Width() called" << endl;
    return width;
}

int Picture::GetHeight(void) {
    if (PA_DEBUG) cerr << "Picture::Height() called" << endl;
    return height;
}

bool Picture::IsGrayScale(void) {
    if (PA_DEBUG) cerr << "Picture::IsGrayScale() called" << endl;
    return isGrayscale;
}

/*

4 Value mapping functions

Please note that no value mapping functions are required here because
the generic function template ~Picture2ScalarTypeMapReturnType~
provided in moduel ~PictureAlgebra\_base.cpp~ is used for all operators
of this module.

*/

int PictureHeightValueMap(Word* args,
			  Word& result,
			  int message,
			  Word& local,
			  Supplier s) {
    if (PA_DEBUG) cerr << "PictureHeightValueMap() called" << endl;

    Picture* p = ((Picture*) args[0].addr);

    result = qp->ResultStorage(s);

    if (p->IsDefined())
	((CcInt*) result.addr)->Set(true, (int) p->GetHeight());
    else
	((CcInt*) result.addr)->Set(false, 0);

    return 0;
}

int PictureWidthValueMap(Word* args,
			 Word& result,
			 int message,
			 Word& local,
			 Supplier s) {
    if (PA_DEBUG) cerr << "PictureWidthValueMap() called" << endl;

    Picture* p = ((Picture*) args[0].addr);

    result = qp->ResultStorage(s);

    if (p->IsDefined())
	((CcInt*) result.addr)->Set(true, (int) p->GetWidth());
    else
	((CcInt*) result.addr)->Set(false, 0);

    return 0;
}

int PictureIsGrayscaleValueMap(Word* args,
			       Word& result,
			       int message,
			       Word& local,
			       Supplier s) {
    if (PA_DEBUG) cerr << "PictureIsGrayscaleValueMap() called" << endl;

    Picture* p = ((Picture*) args[0].addr);

    result = qp->ResultStorage(s);

    if (p->IsDefined())
	((CcBool*) result.addr)->Set(true, (bool) p->IsGrayScale());
    else
	((CcBool*) result.addr)->Set(false, 0);

    return 0;
}

