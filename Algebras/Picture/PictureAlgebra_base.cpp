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

This module contains all code related to creating 
the Picture algebra itself. Code relating to the implementation of
SECONDO types ~picture~ and ~histogram~ are located in other modules.

2 Includes and other preparations

*/

using namespace std;

#include "QueryProcessor.h"

#include "PictureAlgebra.h"

extern NestedList* nl;
extern QueryProcessor *qp;

/*

3 Generic $picture\to scalar$ type mapping function

Multiple SECONDO operators map a ~picture~ object to ~int~, ~bool~ or
~string~. The following function template is used as an abbreviated way
to provide the respective type mapping function.

The ~enum~ is used to determine the actual return type of the type mapping
function.

*/

enum Picture2ScalarTypeMapReturnType { P2STM_INT, P2STM_BOOL, P2STM_STRING };

template<Picture2ScalarTypeMapReturnType returnType>
static ListExpr Picture2ScalarTypeMap(ListExpr args) {
    if (PA_DEBUG) 
	cerr << "Picture2ScalarTypeMap() called with returnType=" 
	     << returnType
	     << endl;

    if (PA_DEBUG) 
	cerr << "Picture2ScalarTypeMap() nl->ListLength(args)=" 
	     << nl->ListLength(args)
	     << endl;
    if (PA_DEBUG) 
	cerr << "Picture2ScalarTypeMap() nl->First(args)=" 
	     << nl->First(args)
	     << endl;

    if (nl->ListLength(args) == 1) {
	if (nl->IsEqual(nl->First(args), "picture")) {
	    if (PA_DEBUG) cerr << "Picture2ScalarTypeMap() #1" << endl;
    
	    if (returnType == P2STM_INT)
		return nl->SymbolAtom("int");
	    else if (returnType == P2STM_BOOL)
		return nl->SymbolAtom("bool");
	    else
		return nl->SymbolAtom("string");
	} else {
	    if (PA_DEBUG) cerr << "Picture2ScalarTypeMap() #2" << endl;
    
	    string lexpr;
	    nl->WriteToString(lexpr, nl->First(args));
	    ErrorReporter::ReportError(
		"expected 'picture' argument but received '"+lexpr+"'");
	}
    } else 
	ErrorReporter::ReportError(
	    "expected only one argument but received "
	    +nl->ListLength(args));

    return nl->SymbolAtom("typeerror");
}

/*

4 Operator specifications

*/

static const string pictureWidthSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>picture -> int</text--->"
    "<text>_ width</text--->"
    "<text>Return width of JPEG picture.</text--->"
    "<text>pic width</text--->"
    ") )";

static const string pictureHeightSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>picture -> int</text--->"
    "<text>_ height</text--->"
    "<text>Return height of JPEG picture.</text--->"
    "<text>pic height</text--->"
    ") )";

static const string pictureIsGrayscaleSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>picture -> bool</text--->"
    "<text>_ isgrayscale</text--->"
    "<text>Returns TRUE if JPEG picture is grayscale.</text--->"
    "<text>pic grayscale</text--->"
    ") )";

static const string pictureFilenameSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>picture -> string</text--->"
    "<text>_ filename</text--->"
    "<text>Returns the filename of the JPEG picture.</text--->"
    "<text>pic filename</text--->"
    ") )";

static const string pictureCategorySpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>picture -> string</text--->"
    "<text>_ category</text--->"
    "<text>Returns the category of the JPEG picture.</text--->"
    "<text>pic category</text--->"
    ") )";

static const string pictureDateSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>picture -> instant</text--->"
    "<text>_ instant</text--->"
    "<text>Returns the date of the JPEG picture.</text--->"
    "<text>pic category</text--->"
    ") )";

static const string pictureIsPortraitSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>picture -> bool</text--->"
    "<text>_ isportrait</text--->"
    "<text>Returns TRUE if picture has portrait format.</text--->"
    "<text>pic isportrait</text--->"
    ") )";

static const string pictureColordistSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>picture int -> histogram</text--->"
    "<text>_ colordist [ _ ]</text--->"
    "<text>Calculate specified histogram for picture.</text--->"
    "<text>pic colordist [ 0 ]</text--->"
    ") )";

static const string pictureEqualsSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>picture picture int int -> real</text--->"
    "<text>p1 p2 equals [ n, p ]</text--->"
    "<text>Returns 0 if pictures are 'similar' in the sense "
    "that |avg_n(p1_hist)-avg_n(p2_hist)| < t/10000. If this "
    "tolerance is exceeded the sum of all"
    "aberrations will be calculated.</text--->"
    "<text>pic1 pic2 equals [ 5, 100 ]</text--->"
    ") )";

static const string pictureSimpleEqualsSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>picture picture-> bool</text--->"
    "<text>_ sequals _</text--->"
    "<text>Returns TRUE if pictures are identical.</text--->"
    "<text>pic1 equals pic2</text--->"
    ") )";

static const string pictureLikeSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>picture int int int int -> bool, "
    "picture real real int int -> bool</text--->"
    "<text>_ like [ _, _, _, _ ]</text--->"
    "<text>TRUE if picture is 'similar' to parameters.</text--->"
    "<text>pic like [ 50, 5, 100, 200 ], "
    "pic like [ 50.0, 5.0, 100, 200 ]</text--->"
    ") )";

static const string pictureCutSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>picture int int int int -> picture</text--->"
    "<text>_ cut [ _, _, _, _ ]</text--->"
    "<text>Cut area with specified x, y, width, height.</text--->"
    "<text>pic cut [ 100, 100, 400, 200 ] </text--->"
    ") )";

static const string pictureScaleSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>picture int int int int -> picture</text--->"
    "<text>_ scale [ _, _ ]</text--->"
    "<text>Scale picture to specified width and height.</text--->"
    "<text>pic scale [ 800, 533 ] </text--->"
    ") )";

static const string pictureFlipleftSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>picture int -> picture</text--->"
    "<text>_ flipleft [ _ ]</text--->"
    "<text>Performs 90 degrees left turns on picture.</text--->"
    "<text>pic flipleft [ 1 ] </text--->"
    ") )";

static const string pictureMirrorSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>picture bool -> picture</text--->"
    "<text>_ mirror [ _ ]</text--->"
    "<text>Mirror picture vertically or horizontally.</text--->"
    "<text>pic mirror [ TRUE ] </text--->"
    ") )";

static const string pictureDisplaySpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>picture -> bool</text--->"
    "<text>_ display</text--->"
    "<text>Display image, return TRUE on success.</text--->"
    "<text>pic display</text--->"
    ") )";

static const string pictureExportSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>picture text -> bool</text--->"
    "<text>_ export [ _ ]</text--->"
    "<text>Save picture to file, return TRUE on success.</text--->"
    "<text>pic export [ \"/tmp/schmuh.jpg\" ]</text--->"
    ") )";

/*

5 External declarations

Numerous functions and the type constructors are implemented in other modules
for better structure. As these are used in this module only, we do not 
provide these declarations in PictureAlgebra.h but directly in this section.

Each of the following sub-sections contain the declarations for one module.

5.1 Implemented in ~PictureAlgebra\_pictimp.cpp~

*/

ListExpr PictureDateTypeMap(ListExpr args);
ListExpr PictureExportTypeMap(ListExpr args);
ListExpr PictureSimpleEqualsTypeMap(ListExpr args);

int PictureFilenameValueMap(Word* args,
			    Word& result,
			    int message,
			    Word& local,
			    Supplier s);
int PictureCategoryValueMap(Word* args,
			    Word& result,
			    int message,
			    Word& local,
			    Supplier s);
int PictureDateValueMap(Word* args,
			Word& result,
			int message,
			Word& local,
			Supplier s);
int PictureIsPortraitValueMap(Word* args,
			      Word& result,
			      int message,
			      Word& local,
			      Supplier s);
int PictureDisplayValueMap(Word* args,
			   Word& result,
			   int message,
			   Word& local,
			   Supplier s);
int PictureExportValueMap(Word* args,
			  Word& result,
			  int message,
			  Word& local,
			  Supplier s);
int PictureSimpleEqualsValueMap(Word* args,
				Word& result,
				int message,
				Word& local,
				Supplier s);

extern TypeConstructor picture;

/*

5.2 Implemented in ~PictureAlgebra\_attrops.cpp~

*/

extern int PictureWidthValueMap(Word* args,
				Word& result,
				int message,
				Word& local,
				Supplier s);
extern int PictureHeightValueMap(Word* args,
				 Word& result,
				 int message,
				 Word& local,
				 Supplier s);
extern int PictureIsGrayscaleValueMap(Word* args,
				      Word& result,
				      int message,
				      Word& local,
				      Supplier s);

/*

5.3 Implemented in ~PictureAlgebra\_histimp.cpp~

*/

extern TypeConstructor histogram;

/*

5.4 Implemented in ~PictureAlgebra\_histops.cpp~

*/

extern ListExpr PictureColordistTypeMap(ListExpr args);
extern ListExpr PictureEqualsTypeMap(ListExpr args);
extern ListExpr PictureLikeTypeMap(ListExpr args);

extern int PictureColordistValueMap(Word* args,
				    Word& result,
				    int message,
				    Word& local,
				    Supplier s);
extern int PictureEqualsValueMap(Word* args,
				 Word& result,
				 int message,
				 Word& local,
				 Supplier s);

extern ValueMapping pictureLikeValueMap[];

extern int PictureLikeSelect(ListExpr args);


/*

5.5 Implemented in ~PictureAlgebra\_graphops.cpp~

*/

extern ListExpr PictureScaleTypeMap(ListExpr args);
extern ListExpr PictureCutTypeMap(ListExpr args);
extern ListExpr PictureFlipleftTypeMap(ListExpr args);
extern ListExpr PictureMirrorTypeMap(ListExpr args);

extern int PictureCutValueMap(Word* args,
			      Word& result,
			      int message,
			      Word& local,
			      Supplier s);
extern int PictureScaleValueMap(Word* args,
				Word& result,
				int message,
				Word& local,
				Supplier s);
extern int PictureFlipleftValueMap(Word* args,
				   Word& result,
				   int message,
				   Word& local,
				   Supplier s);
extern int PictureMirrorValueMap(Word* args,
				 Word& result,
				 int message,
				 Word& local,
				 Supplier s);

/*

6 Operator creation

The dummy selection function ~SimpleSelect()~ is required for non-overloaded
operators.

*/

static int SimpleSelect(ListExpr args) {
    return 0;
}

/*

As we do not use a model mapping, we are providing a dummy model mapping.
Please note that this array needs to be extended if an operator is overloaded
with more than two functions. Yes, this is prone to errors!

*/

ModelMapping dummymodelmap[] = 
    { Operator::DummyModel, Operator::DummyModel };

/*

The creation of the actual operators follows. Operator ~like~
is overloaded.

*/

static Operator height(
    "height",                              //name
    pictureHeightSpec,                     //specification
    PictureHeightValueMap,                 //value mapping
    Operator::DummyModel,                  //dummy model mapping
    SimpleSelect,                          //mapping selection function
    Picture2ScalarTypeMap<P2STM_INT>       //type mapping
);

static Operator width(
    "width",                               //name
    pictureWidthSpec,                      //specification
    PictureWidthValueMap,                  //value mapping
    Operator::DummyModel,                  //dummy model mapping
    SimpleSelect,                          //mapping selection function
    Picture2ScalarTypeMap<P2STM_INT>       //type mapping
);

static Operator isgrayscale(
    "isgrayscale",                         //name
    pictureIsGrayscaleSpec,                //specification
    PictureIsGrayscaleValueMap,            //value mapping
    Operator::DummyModel,                  //dummy model mapping
    SimpleSelect,                          //mapping selection function
    Picture2ScalarTypeMap<P2STM_BOOL>      //type mapping
);

static Operator filename(
    "filename",                            //name
    pictureFilenameSpec,                   //specification
    PictureFilenameValueMap,               //value mapping
    Operator::DummyModel,                  //dummy model mapping
    SimpleSelect,                          //mapping selection function
    Picture2ScalarTypeMap<P2STM_STRING>    //type mapping
);

static Operator category(
    "category",                            //name
    pictureCategorySpec,                   //specification
    PictureCategoryValueMap,               //value mapping
    Operator::DummyModel,                  //dummy model mapping
    SimpleSelect,                          //mapping selection function
    Picture2ScalarTypeMap<P2STM_STRING>    //type mapping
);

static Operator date(
    "picturedate",                         //name
    pictureDateSpec,                       //specification
    PictureDateValueMap,                   //value mapping
    Operator::DummyModel,                  //dummy model mapping
    SimpleSelect,                          //mapping selection function
    PictureDateTypeMap                     //type mapping
);

static Operator isportrait(
    "isportrait" ,                         //name
    pictureIsPortraitSpec,                 //specification
    PictureIsPortraitValueMap,             //value mapping
    Operator::DummyModel,                  //dummy model mapping
    SimpleSelect,                          //mapping selection function
    Picture2ScalarTypeMap<P2STM_BOOL>      //type mapping
);

static Operator colordist(
    "colordist",                           //name
    pictureColordistSpec,                  //specification
    PictureColordistValueMap,              //value mapping
    Operator::DummyModel,                  //dummy model mapping
    SimpleSelect,                          //mapping selection function
    PictureColordistTypeMap                //type mapping
);

static Operator simpleequals(
    "simpleequals",                        //name
    pictureSimpleEqualsSpec,               //specification
    PictureSimpleEqualsValueMap,           //value mapping
    Operator::DummyModel,                  //dummy model mapping
    SimpleSelect,                          //mapping selection function
    PictureSimpleEqualsTypeMap             //type mapping
);

static Operator equals(
    "equals",                              //name
    pictureEqualsSpec,                     //specification
    PictureEqualsValueMap,                 //value mapping
    Operator::DummyModel,                  //dummy model mapping
    SimpleSelect,                          //mapping selection function
    PictureEqualsTypeMap                   //type mapping
);

static Operator like(
    "like",                                //name
    pictureLikeSpec,                       //specification
    2,                                     //number of overloaded functions
    pictureLikeValueMap,                   //value mapping
    dummymodelmap,                         //array with dummy model mapping
    PictureLikeSelect,                     //value mapping selection function
    PictureLikeTypeMap                     //type mapping
);

static Operator scale(
    "scale",                               //name
    pictureScaleSpec,                      //specification
    PictureScaleValueMap,                  //value mapping
    Operator::DummyModel,                  //dummy model mapping
    SimpleSelect,                          //mapping selection function
    PictureScaleTypeMap                    //type mapping
);

static Operator cut(
    "cut",                                 //name
    pictureCutSpec,                        //specification
    PictureCutValueMap,                    //value mapping
    Operator::DummyModel,                  //dummy model mapping
    SimpleSelect,                          //mapping selection function
    PictureCutTypeMap                      //type mapping
);

static Operator flipleft(
    "flipleft",                            //name
    pictureFlipleftSpec,                   //specification
    PictureFlipleftValueMap,               //value mapping
    Operator::DummyModel,                  //dummy model mapping
    SimpleSelect,                          //mapping selection function
    PictureFlipleftTypeMap                 //type mapping
);

static Operator mirror(
    "mirror",                              //name
    pictureMirrorSpec,                   //specification
    PictureMirrorValueMap,               //value mapping
    Operator::DummyModel,                  //dummy model mapping
    SimpleSelect,                          //mapping selection function
    PictureMirrorTypeMap                 //type mapping
);

static Operator display(
    "display",                             //name
    pictureDisplaySpec,                    //specification
    PictureDisplayValueMap,                //value mapping
    Operator::DummyModel,                  //dummy model mapping
    SimpleSelect,                          //mapping selection function
    Picture2ScalarTypeMap<P2STM_BOOL>      //type mapping
);

static Operator exportop(
    "export",                              //name
    pictureExportSpec,                     //specification
    PictureExportValueMap,                 //value mapping
    Operator::DummyModel,                  //dummy model mapping
    SimpleSelect,                          //mapping selection function
    PictureExportTypeMap                   //type mapping
);

/*

7 Algebra creation and initialisation

*/

class PictureAlgebra: public Algebra {
public:
    PictureAlgebra() : Algebra() {
	if (PA_DEBUG) cerr << "initializing PictureAlgebra" << endl;

	AddTypeConstructor(&picture);
	AddTypeConstructor(&histogram);

	picture.AssociateKind("DATA");
	histogram.AssociateKind("DATA");

	AddOperator(&height);
	AddOperator(&width);
	AddOperator(&isgrayscale);
	AddOperator(&filename);
	AddOperator(&category);
	AddOperator(&date);
	AddOperator(&isportrait);
	AddOperator(&colordist);
	AddOperator(&equals);
	AddOperator(&simpleequals);
	AddOperator(&like);
	AddOperator(&scale);
	AddOperator(&cut);
	AddOperator(&flipleft);
	AddOperator(&mirror);
	AddOperator(&display);
	AddOperator(&exportop);
    }
    ~PictureAlgebra() {}
};

static PictureAlgebra picturealgebra;

extern "C" 
Algebra* InitializePictureAlgebra(NestedList* nlPar, QueryProcessor *qpPar) {
    nl = nlPar;
    qp = qpPar;
    return &picturealgebra;
}
