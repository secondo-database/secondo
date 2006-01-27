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

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

[TOC]

1 Introduction

See the documentation of ~PictureAlgebra.h~ for a general introduction to
the Picture algebra.

This module contains SECONDO operators on ~picture~, which deal with
a ~histogram~ object.

2 Includes and other preparations

*/

using namespace std;

#include "Algebra.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "NestedList.h"

#include "PictureAlgebra.h"

extern NestedList* nl;
extern QueryProcessor *qp;


/*

3 Operators on ~picture~

3.1 Operator ~Colordist~

The function ~Colordist~ creates a ~histogram~-object and returns
this. The calling function should not forget to delete the ~histogram~-object.

*/

Histogram* Picture::Colordist(HistogramChannel channel) {
    if (PA_DEBUG) cerr << "Picture::Colordist() called" << endl;

    //
    //	Create a histogram of this picture.
    //  Don't forget to delete the object !
    //

    Histogram* h = new Histogram();
    if (( channel < 0 ) || ( channel > 3 ))
    {
	h->SetDefined( false );
    }
    else
    {
    	h->CopyFrom( (const StandardAttribute *) &this->histogram[ channel ] );
    }

    return h;
}

/*

3.2 Operator ~Equals~

The ~Equals~-function checks, whether two pictures are equal. Therefore,
it creates for each picture a histogram and compares the datas of
these objects. 

*/

bool Picture::Equals(Picture* pic, int n, int p, bool &valid, double& diff) {
    if (PA_DEBUG) cerr << "Picture::Equals() called" << endl;

    //
    //	Create two histograms and call the equal-function for
    //  histograms.
    //
    Histogram* h1 = &this->histogram[HC_BRIGHTNESS];
    Histogram* h2 = &pic->histogram[HC_BRIGHTNESS];

    //cerr << "Picture::Equals() this="
    //	 << (int) this
    //	 << " h1="
    //	 << (int) h1
    //	 << " h2="
    //	 << (int) h2
    //	 << endl;

    bool rc = h1->Equals( h2, n, p, valid, diff);
    //
    //	Delete everything!
    //
    //  Nope: h1 and h2 are just pointers into the attribute histogram,
    //  therefore the referenced memory must not be deleted...
    //delete h1;
    //delete h2;

    cerr << "Picture::Equals() done" << endl;
    return( rc ); 
}

/*

3.3 Operator ~Like~

The function ~Like~ checks, whether the brightness of the picture 
in the interval of [l,u] differs p+-t percent.

p 	main percentage
t	+- relative percentage
l	minimal range of interval
u	maximal range of interval

~Like~ returns true, if the picture fulfils the specified
parameters. Otherwise it returns false.

*/

bool Picture::Like(int p, int t, int l, int u, bool& valid) {
    if (PA_DEBUG) cerr << "Picture::Like() called" << endl;

    //
    //	Create a histogram and check the brightness in the
    //	interval [l, u].
    //
    Histogram* h = &this->histogram[ HC_BRIGHTNESS ];

    bool rc = h->Like( p, t, l, u, valid );
    if (PA_DEBUG) 
	cerr << "This picture " << ((rc)?"fulfils": "does not fulfil")
		<< " the specified parameter." << endl;
    
    return rc;
}

/*

3.4 Operator ~Like~

The function ~Like~ checks, whether the brightness of the picture 
in the interval of [l,u] differs p+-t percent.

p 	main percentage
t	+- relative percentage
l	minimal range of interval
u	maximal range of interval

~Like~ returns true, if the picture fulfils the specified
parameters. Otherwise it returns false.
Please notice thate the specified parameters ~p~ and ~t~ are type float.

*/

bool Picture::Like(float p, float t, int l, int u, bool& valid) {
    if (PA_DEBUG) cerr << "Picture::Like() called" << endl;

    //
    //	Create a histogram and check the brightness in the
    //	interval [l, u].
    //
    Histogram* h = &this->histogram[ HC_BRIGHTNESS ];

    bool rc = h->Like( p, t, l, u, valid );
    if (PA_DEBUG) 
	cerr << "This picture " << ((rc)?"fulfils": "does not fulfil")
		<< " the specified parameter." << endl;
    
    return rc;
}

/*

4 Type mapping fucntions

These functions check whether the specified nested list parameter structure
is in correct type of parameters for the operatoris ~Colordist~, ~Equals~
and ~Like~.

*/

ListExpr PictureColordistTypeMap(ListExpr args) {
    if (PA_DEBUG) cerr << "PictureColordistTypeMap() called" << endl;

    ListExpr arg1, arg2;

    if ( nl->ListLength(args) != 2 )
    {
        ErrorReporter::ReportError(
            "expected four arguments but received "
            +nl->ListLength(args));

	return( nl->SymbolAtom("typeerror"));
    }

    //
    //	Got 2 different arguments ( picture, int )
    //
    arg1 = nl->First(args);
    arg2 = nl->Second(args);

    if ( ! nl->IsEqual(arg1, "picture")) 
    {
            ErrorReporter::ReportError(
                "expected 'picture' as first argument but received '"
                +nl->SymbolValue(arg1)
                +"'");

	    return (nl->SymbolAtom("typeerror"));
    }
    if ( ! nl->IsEqual(arg2, "int"))
    {
            ErrorReporter::ReportError(
                "expected 'int' as second argument but received '"
                +nl->SymbolValue(arg2)
                +"'");

	    return (nl->SymbolAtom("typeerror"));
    }

    return( nl->SymbolAtom("histogram"));
}

ListExpr PictureEqualsTypeMap(ListExpr args) {
    if (PA_DEBUG) cerr << "PictureEqualsTypeMap() called" << endl;

    ListExpr arg1, arg2, arg3, arg4;

    if ( nl->ListLength(args) != 4 )
    {
        ErrorReporter::ReportError(
            "expected four arguments but received "
            +nl->ListLength(args));

	return( nl->SymbolAtom("typeerror"));
    }

    //
    //	Got 4 different arguments ( picture, picture, int, int )
    //
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    arg3 = nl->Third(args);
    arg4 = nl->Fourth(args);

    if ( ! nl->IsEqual(arg1, "picture") )
    {
            ErrorReporter::ReportError(
                "expected 'picture' as first argument but received '"
                +nl->SymbolValue(arg1)
                +"'");

	    return (nl->SymbolAtom("typeerror"));
    }
    if ( ! nl->IsEqual(arg2, "picture"))
    {
            ErrorReporter::ReportError(
                "expected 'picture' as second argument but received '"
                +nl->SymbolValue(arg2)
                +"'");

	    return (nl->SymbolAtom("typeerror"));
    }
    if ( ! nl->IsEqual(arg3, "int" ))
    {
            ErrorReporter::ReportError(
                "expected 'int' as third argument but received '"
                +nl->SymbolValue(arg3)
                +"'");

	    return (nl->SymbolAtom("typeerror"));
    }
    if ( ! nl->IsEqual(arg4, "int" ))
    {
            ErrorReporter::ReportError(
                "expected 'int' as fourth argument but received '"
                +nl->SymbolValue(arg4)
                +"'");

	    return (nl->SymbolAtom("typeerror"));
    }
    
    return( nl->SymbolAtom("real"));
}

ListExpr PictureLikeTypeMap(ListExpr args) {
    if (PA_DEBUG) cerr << "PictureLikeTypeMap() called" << endl;

    ListExpr arg1, arg2, arg3, arg4, arg5;

    if ( nl->ListLength(args) != 5 )
    {
        ErrorReporter::ReportError(
            "expected five arguments but received "
            +nl->ListLength(args));

	return( nl->SymbolAtom("typeerror"));
    }

    //
    //	Got 5 different arguments ( picture, int, int, int, int )
    //
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    arg3 = nl->Third(args);
    arg4 = nl->Fourth(args);
    arg5 = nl->Fifth(args);

    if ( ! nl->IsEqual(arg1, "picture")) 
    {
            ErrorReporter::ReportError(
                "expected 'picture' as first argument but received '"
                +nl->SymbolValue(arg1)
                +"'");

	    return (nl->SymbolAtom("typeerror"));
    }
    if ( ! nl->IsEqual(arg2, "int") && ! nl->IsEqual(arg2, "real" ))
    {
            ErrorReporter::ReportError(
                "expected 'int' or 'real' as second argument but received '"
                +nl->SymbolValue(arg2)
                +"'");

	    return (nl->SymbolAtom("typeerror"));
    }
    if ( ! nl->IsEqual(arg3, "int" ) && ! nl->IsEqual(arg3, "real" ))
    {
            ErrorReporter::ReportError(
                "expected 'int' or 'real' as third argument but received '"
                +nl->SymbolValue(arg3)
                +"'");

	    return (nl->SymbolAtom("typeerror"));
    }
    if ( ! nl->IsEqual(arg4, "int" ) )
    {
            ErrorReporter::ReportError(
                "expected 'int' as fourth argument but received '"
                +nl->SymbolValue(arg4)
                +"'");

	    return (nl->SymbolAtom("typeerror"));
    }
    if ( ! nl->IsEqual(arg5, "int" ) )
    {
            ErrorReporter::ReportError(
                "expected 'int' as fifth argument but received '"
                +nl->SymbolValue(arg5)
                +"'");

	    return (nl->SymbolAtom("typeerror"));
    }
    if ( ! nl->Equal(arg2, arg3) ) 
    {
            ErrorReporter::ReportError(
                "second and third argument must have same type but received '"
                +nl->SymbolValue(arg2)
                +"' and '"
                +nl->SymbolValue(arg3)
		+"'");

	    return (nl->SymbolAtom("typeerror"));
    }
    
    return( nl->SymbolAtom("bool"));
}

/*

5 Type mapping functions

The following functions are the ~Value Mapping~ functions for the operators
~Colordist~, ~Equals~ and ~Like~.

*/

int PictureColordistValueMap(Word* args,
			     Word& result,
			     int message,
			     Word& local,
			     Supplier s) {
    if (PA_DEBUG) cerr << "PictureColordistValueMap() called" << endl;

    Picture* pic;
    CcInt* channel;

    pic = ((Picture*) args[0].addr);
    channel = ((CcInt*) args[1].addr);

    result = qp->ResultStorage(s);

    Histogram* res = ((Histogram *)result.addr);


    //
    //	I need at least a defined picture. Other parameters are
    //	checked in function ~Colordist~. 
    //
    if ( pic->IsDefined() ) 
    {
    	res->CopyFrom( 
	     (const StandardAttribute *) pic->Colordist(
				HistogramChannel(channel->GetIntval())));
    }
    else
    {
	cerr << endl << endl
             << "The specified picture is not defined." 
             << endl << endl;

	res->SetDefined( false );
    }
    return( 0 );
}

int PictureEqualsValueMap(Word* args,
			  Word& result,
			  int message,
			  Word& local,
			  Supplier s) {
    if (PA_DEBUG) cerr << "PictureEqualsValueMap() called" << endl;

    Picture* pic1;
    Picture* pic2;
    CcInt* numberValue;
    CcInt* averageValue;

    pic1 = ((Picture*) args[0].addr);
    pic2 = ((Picture*) args[1].addr);
    numberValue = ((CcInt *) args[2].addr);
    averageValue = ((CcInt *) args[3].addr);

    result = qp->ResultStorage(s);

    if ( pic1->IsDefined() && pic2->IsDefined() )
    {
      bool valid = false;
      double diff = 0;
	    pic1->Equals( pic2, numberValue->GetIntval(), 
			averageValue->GetIntval(), valid, diff );
        
    	((CcReal*)result.addr)->Set( valid, diff ); 
    }
    else
    {
        cerr << endl << endl
             << "At least one of the specified picture is not defined."
             << endl << endl;

	((CcReal*)result.addr)->Set(false, false);
    }

    return( 0 );
}

static int PictureLikeValueMap_int(Word* args,
				   Word& result,
				   int message,
				   Word& local,
				   Supplier s) {
    if (PA_DEBUG) cerr << "PictureLikeValueMap_int() called" << endl;

    Picture* pic;
    CcInt* percentage;
    CcInt* delta;
    CcInt* minInterval;
    CcInt* maxInterval;

    pic         = ((Picture*) args[0].addr);
    percentage  = ((CcInt *) args[1].addr);
    delta       = ((CcInt *) args[2].addr);
    minInterval = ((CcInt *) args[3].addr);
    maxInterval = ((CcInt *) args[4].addr);

    result = qp->ResultStorage(s);

    if ( pic->IsDefined() )
    {
	bool valid;
	bool rc = pic->Like( percentage->GetIntval(), delta->GetIntval(),
		   minInterval->GetIntval(), maxInterval->GetIntval(), valid); 

    	((CcBool*)result.addr)->Set( valid, rc );
    }
    else
    {
	cerr << endl << endl
             << "The specified picture is not defined"
             << endl << endl;
	((CcBool*)result.addr)->Set(false, false);
    }

    return( 0 );
}

static int PictureLikeValueMap_real(Word* args,
				    Word& result,
				    int message,
				    Word& local,
				    Supplier s) {
    if (PA_DEBUG) cerr << "PictureLikeValueMap_real() called" << endl;

    Picture* pic;
    CcReal*  percentage;
    CcReal*  delta;
    CcInt*   minInterval;
    CcInt*   maxInterval;

    pic         = ((Picture*) args[0].addr);
    percentage  = ((CcReal *) args[1].addr);
    delta       = ((CcReal *) args[2].addr);
    minInterval = ((CcInt *) args[3].addr);
    maxInterval = ((CcInt *) args[4].addr);

    result = qp->ResultStorage(s);

    if ( pic->IsDefined() )
    {
	bool valid;
        bool rc = pic->Like( 	percentage->GetRealval(), 
				delta->GetRealval(),
				minInterval->GetIntval(), 
				maxInterval->GetIntval(), valid );

	((CcBool*)result.addr)->Set( valid, rc );
    }
    else
    {
	cerr << endl << endl
             << "The specified picture is not defined"
             << endl << endl;
	((CcBool*)result.addr)->Set(false, false);
    }
  
    return(0);
}

ValueMapping pictureLikeValueMap[] = {
    PictureLikeValueMap_int, 
    PictureLikeValueMap_real
};

/*

6 Select function

This function is necessary because the ~Like~ operator is overloaded.

*/

int PictureLikeSelect(ListExpr args) {
    if (PA_DEBUG) cerr << "PictureLikeSelect() called" << endl;

    // It is sufficient to check second parameter because type mapping
    // function already assured that third parameter has same type as
    // second parameter.

    ListExpr arg2 = nl->Second(args);

    return nl->IsEqual(arg2, "int" ) ? 0 : 1;
}
