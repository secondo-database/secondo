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

This module contains all code related to creating
the Picture algebra itself. Code relating to the implementation of
SECONDO types ~picture~ and ~histogram~ are located in other modules.

2 Includes and other preparations

*/

#include "Algebra.h"
#include "QueryProcessor.h"
#include "PictureAlgebra.h"
#include "Symbols.h"
#include "StringUtils.h"
#include "hist_hsv.h"
#include "JPEGPicture.h"

using namespace std;

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
        if (nl->IsEqual(nl->First(args), Picture::BasicType())) {
            if (PA_DEBUG) cerr << "Picture2ScalarTypeMap() #1" << endl;

            if (returnType == P2STM_INT)
                return nl->SymbolAtom(CcInt::BasicType());
            else if (returnType == P2STM_BOOL)
                return nl->SymbolAtom(CcBool::BasicType());
            else
                return nl->SymbolAtom(CcString::BasicType());
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
            + stringutils::int2str(nl->ListLength(args)));

    return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*

4 Operator specifications

*/

static const string pictureWidthSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>picture -> int</text--->"
    "<text>_ getWidth</text--->"
    "<text>Return width of JPEG picture.</text--->"
    "<text>pic getWidth</text--->"
    ") )";

static const string pictureHeightSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>picture -> int</text--->"
    "<text>_ getHeight</text--->"
    "<text>Return height of JPEG picture.</text--->"
    "<text>pic getHeight</text--->"
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
    "<text>_ getFilename</text--->"
    "<text>Returns the filename of the JPEG picture.</text--->"
    "<text>pic getFilename</text--->"
    ") )";

static const string pictureCategorySpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>picture -> string</text--->"
    "<text>_ getCategory</text--->"
    "<text>Returns the category of the JPEG picture.</text--->"
    "<text>pic getCategory</text--->"
    ") )";

static const string pictureDateSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>picture -> instant</text--->"
    "<text>_ getPictureDate</text--->"
    "<text>Returns the date of the JPEG picture.</text--->"
    "<text>pic getPictureDate</text--->"
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
    "<text>p1 p2 equals [ n, t ]</text--->"
    "<text>Returns 0 if pictures are 'similar' in the sense "
    "that abs( avg_i^i+n(p1_hist[i]) - avg_i^[i+n](p2_hist[i])) < t/10000 "
    "If this tolerance is exceeded the sum of all "
    "aberrations will be calculated.</text--->"
    "<text>pic1 pic2 equals [ 5, 100 ]</text--->"
    ") )";

static const string pictureContainsSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>picture picture -> bool</text--->"
    "<text>p1 contains p2</text--->"
    "<text>Returns TRUE if the red, green, and blue histogram "
    "curves based on absolute numbers of pixels of p1 are an upper boundary "
    "for the respective curves of p2. This can be used to filter "
    "out candidates "
    "which may contain p2. </text--->"
    "<text>pic1 contains pic2</text--->"
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

extern TypeConstructor* picture;
extern void initPicture();

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

extern TypeConstructor* histogram;
extern void initHistogram();

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

extern int PictureContainsValueMap(Word* args,
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

The creation of the actual operators follows. Operator ~like~
is overloaded.

*/

static Operator height(
    "getHeight",                           //name
    pictureHeightSpec,                     //specification
    PictureHeightValueMap,                 //value mapping
    SimpleSelect,                          //mapping selection function
    Picture2ScalarTypeMap<P2STM_INT>       //type mapping
);

static Operator width(
    "getWidth",                            //name
    pictureWidthSpec,                      //specification
    PictureWidthValueMap,                  //value mapping
    SimpleSelect,                          //mapping selection function
    Picture2ScalarTypeMap<P2STM_INT>       //type mapping
);

static Operator isgrayscale(
    "isgrayscale",                         //name
    pictureIsGrayscaleSpec,                //specification
    PictureIsGrayscaleValueMap,            //value mapping
    SimpleSelect,                          //mapping selection function
    Picture2ScalarTypeMap<P2STM_BOOL>      //type mapping
);

static Operator filename(
    "getFilename",                            //name
    pictureFilenameSpec,                   //specification
    PictureFilenameValueMap,               //value mapping
    SimpleSelect,                          //mapping selection function
    Picture2ScalarTypeMap<P2STM_STRING>    //type mapping
);

static Operator category(
    "getCategory",                         //name
    pictureCategorySpec,                   //specification
    PictureCategoryValueMap,               //value mapping
    SimpleSelect,                          //mapping selection function
    Picture2ScalarTypeMap<P2STM_STRING>    //type mapping
);

static Operator date(
    "getPictureDate",                      //name
    pictureDateSpec,                       //specification
    PictureDateValueMap,                   //value mapping
    SimpleSelect,                          //mapping selection function
    PictureDateTypeMap                     //type mapping
);

static Operator isportrait(
    "isportrait" ,                         //name
    pictureIsPortraitSpec,                 //specification
    PictureIsPortraitValueMap,             //value mapping
    SimpleSelect,                          //mapping selection function
    Picture2ScalarTypeMap<P2STM_BOOL>      //type mapping
);

static Operator colordist(
    "colordist",                           //name
    pictureColordistSpec,                  //specification
    PictureColordistValueMap,              //value mapping
    SimpleSelect,                          //mapping selection function
    PictureColordistTypeMap                //type mapping
);

static Operator simpleequals(
    "simpleequals",                        //name
    pictureSimpleEqualsSpec,               //specification
    PictureSimpleEqualsValueMap,           //value mapping
    SimpleSelect,                          //mapping selection function
    PictureSimpleEqualsTypeMap             //type mapping
);

static Operator equals(
    "equals",                              //name
    pictureEqualsSpec,                     //specification
    PictureEqualsValueMap,                 //value mapping
    SimpleSelect,                          //mapping selection function
    PictureEqualsTypeMap                   //type mapping
);

static Operator containsOp(
    "contains",                            //name
    pictureContainsSpec,                   //specification
    PictureContainsValueMap,               //value mapping
    SimpleSelect,                          //mapping selection function
    PictureSimpleEqualsTypeMap             //type mapping
);


static Operator like(
    "like",                                //name
    pictureLikeSpec,                       //specification
    2,                                     //number of overloaded functions
    pictureLikeValueMap,                   //value mapping
    PictureLikeSelect,                     //value mapping selection function
    PictureLikeTypeMap                     //type mapping
);

static Operator scale(
    "scale",                               //name
    pictureScaleSpec,                      //specification
    PictureScaleValueMap,                  //value mapping
    SimpleSelect,                          //mapping selection function
    PictureScaleTypeMap                    //type mapping
);

static Operator cut(
    "cut",                                 //name
    pictureCutSpec,                        //specification
    PictureCutValueMap,                    //value mapping
    SimpleSelect,                          //mapping selection function
    PictureCutTypeMap                      //type mapping
);

static Operator flipleft(
    "flipleft",                            //name
    pictureFlipleftSpec,                   //specification
    PictureFlipleftValueMap,               //value mapping
    SimpleSelect,                          //mapping selection function
    PictureFlipleftTypeMap                 //type mapping
);

static Operator mirror(
    "mirror",                              //name
    pictureMirrorSpec,                   //specification
    PictureMirrorValueMap,               //value mapping
    SimpleSelect,                          //mapping selection function
    PictureMirrorTypeMap                 //type mapping
);

static Operator display(
    "display",                             //name
    pictureDisplaySpec,                    //specification
    PictureDisplayValueMap,                //value mapping
    SimpleSelect,                          //mapping selection function
    Picture2ScalarTypeMap<P2STM_BOOL>      //type mapping
);

static Operator exportop(
    "export",                              //name
    pictureExportSpec,                     //specification
    PictureExportValueMap,                 //value mapping
    SimpleSelect,                          //mapping selection function
    PictureExportTypeMap                   //type mapping
);


/*
hsv histograms

*/

GenTC<hist_hsv<8,false> > hist_hsv8;
GenTC<hist_hsv<16, false> > hist_hsv16;
GenTC<hist_hsv<32, false> > hist_hsv32;
GenTC<hist_hsv<64, false> > hist_hsv64;
GenTC<hist_hsv<128, false> > hist_hsv128;
GenTC<hist_hsv<256, false> > hist_hsv256;
GenTC<hist_hsv<256, true> > hist_lab256;



/*

Operators on hsv histograms

*/

ListExpr distanceTM(ListExpr args){
  string err = "hist_hsvD x hist_hsvD expected";
  if(!nl->HasLength(args,2)){
    return listutils::typeError(err + " (wrong number of args)");
  }
  if(!nl->Equal(nl->First(args), nl->Second(args))){
    return listutils::typeError(err + " (argument types differ)");
  }
  ListExpr a1 = nl->First(args);
  if(   !hist_hsv<8, false>::checkType(a1)
     && !hist_hsv<16, false>::checkType(a1)
     && !hist_hsv<32, false>::checkType(a1)
     && !hist_hsv<64, false>::checkType(a1)
     && !hist_hsv<128, false>::checkType(a1)
     && !hist_hsv<256, false>::checkType(a1)
     && !hist_hsv<256, true>::checkType(a1)){
    return listutils::typeError(err);
  }
  return listutils::basicSymbol<CcReal>();
}


template<unsigned int dim, bool lab>
int distanceVMT(Word* args, Word& result, int message,
                Word& local, Supplier s){

   hist_hsv<dim, lab>* a1 = (hist_hsv<dim, lab>*) args[0].addr;
   hist_hsv<dim, lab>* a2 = (hist_hsv<dim, lab>*) args[1].addr;
   result = qp->ResultStorage(s);
   CcReal* res = (CcReal*) result.addr;
   bool d;
   double dist;
   a1->distance(*a2,d,dist);
   res->Set(d,dist);
   return 0;
}

ValueMapping distanceVM[] = {
    distanceVMT<8, false>,
    distanceVMT<16, false>,
    distanceVMT<32, false>,
    distanceVMT<64, false>,
    distanceVMT<128, false>,
    distanceVMT<256, false>,
    distanceVMT<256, true>,
};

int distanceSelect(ListExpr args){
  ListExpr a1 = nl->First(args);
  if(hist_hsv<8, false>::checkType(a1)) return 0;
  if(hist_hsv<16, false>::checkType(a1)) return 1;
  if(hist_hsv<32, false>::checkType(a1)) return 2;
  if(hist_hsv<64, false>::checkType(a1)) return 3;
  if(hist_hsv<128, false>::checkType(a1)) return 4;
  if(hist_hsv<256, false>::checkType(a1)) return 5;
  if(hist_hsv<256, true>::checkType(a1)) return 6;
  return -1;
}

OperatorSpec distanceSpec(
   "hist_hsvD x hist_hsvD -> real",
   "distance(_,_)",
   "computes the distances between two hsv histograms",
   "query distance(getHSV8(theater) , getHSV8(paper)"
);

Operator distanceOp(
  "distance",
  distanceSpec.getStr(),
  7,
  distanceVM,
  distanceSelect,
  distanceTM
);



template<unsigned int dim, bool lab>
ListExpr getHistHsvTM(ListExpr args){
  string err = "picture expected";
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err + " (wrong number of args)");
  }
  if(!Picture::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  return listutils::basicSymbol<hist_hsv<dim, lab> >();

}

class HSV{

public:
 HSV(unsigned char r, unsigned char g, unsigned char b){

    unsigned char rgbMin = std::min (std::min (r, g), b); 
    unsigned char rgbMax = std::max (std::max (r, g), b); 
    unsigned char delta = rgbMax - rgbMin;

    // compute h
    if (delta == 0) {   
      h = 0;
    } else {   
      if (rgbMax == r) {   
        h = 60 * (g - b) / delta;
      }   else if (rgbMax == g) {   
        h = 120 * (g - b) / delta;
      }   else  { // rgbMax == b
        h = 240 * (g - b) / delta;
      }   
    }   

    if (h < 0){
        h += 360;
    }

    // compute s
    if (rgbMax == 0)
        s = 0;
    else
        s = 255 * delta / rgbMax;

    // compute v
    v = rgbMax;

 }

 int h;
 int s;
 int v;
};


unsigned* pictureLabOffsetTable=0;


class Lab{
  public:
    Lab (unsigned char r_, unsigned char g_, unsigned char b_){
      double R, G, B;
    double rd = (double) r_ / 255;
    double gd = (double) g_ / 255;
    double bd = (double) b_ / 255;

    if (rd > 0.04045)
        R = std::pow((rd + 0.055) / 1.055, 2.2);
    else
        R = rd / 12.92;

    if (gd > 0.04045)
        G = std::pow ((gd + 0.055) / 1.055, 2.2);
    else
        G = gd / 12.92;

    if (bd > 0.04045)
        B = std::pow ((bd + 0.055) / 1.055, 2.2);
    else
        B = bd / 12.92;

    // compute X,Y,Z coordinates of r,g,b
    double X = 0.4124 * R + 0.3576 * G + 0.1805 * B;
    double Y = 0.2127 * R + 0.7152 * G + 0.0722 * B;
    double Z = 0.0193 * R + 0.1192 * G + 0.9500 * B;

    /* used chromacity coordinates of whitepoint D65:
    x = 0.312713, y = 0.329016

    the respective XYZ coordinates are
    Y = 1,
    X = Y * x / y       = 0.9504492183, and
    Z = Y * (1-x-y) / y = 1.0889166480
    */
    double eps = 0.008856; // = 216 / 24389
    double x = X / 0.95045;
    double y = Y;
    double z = Z / 1.08892;
    long double fx, fy, fz;

    if (x > eps)
        fx = std::pow (x, 0.333333);
    else
        fx = 7.787 * x + 0.137931;

    if (y > eps)
        fy = std::pow (y, 0.333333);
    else
        fy = 7.787 * y + 0.137931;

    if (z > eps)
        fz = std::pow (z, 0.333333);
    else
        fz = 7.787 * z + 0.137931;

    // compute Lab coordinates
    double Lab_Ld = ((116  * fy) - 16);
    double Lab_ad = (500 * (fx - fy));
    double Lab_bd = (200 * (fy - fz));

    L = (signed char) Lab_Ld;
    a = (signed char) Lab_ad;
    b = (signed char) Lab_bd;
 }


    signed char L, a, b;
};

void initLabOffsetTable(){
   assert(!pictureLabOffsetTable);

   pictureLabOffsetTable = new unsigned[64*64*64];
   for(signed char r = 0; r < 64; r++)
     for(signed char g = 0; g < 64; g++)
       for(signed char b = 0; b < 64; b++) {   
         Lab lab (2 + (r*4), 2 + (g*4), 2 + (b*4));

         // map values [0, 99] x [-86, 98] x [-107,94] to
         // [0, 3] x [0, 7] x [0, 7] (4 x 8 x 8 = 256 bins)
         int L_offset = (int) (lab.L / 25);
         int a_offset = (int) ((lab.a + 86) / 23.125);
         int b_offset = (int) ((lab.b + 107) / 25.1);
         pictureLabOffsetTable[r*4096 + g*64 + b] =
             64 * L_offset + 8 * a_offset + b_offset;
       }   
}




unsigned int getHSVIndex(int h, int s, int v, int dim){
  if(dim==8){
    int h_offset = h / 180;  // 2 parts
    int s_offset = s / 128;  // 2 parts
    int v_offset = v / 256; // 2 parts
    return 4*h_offset + 2*s_offset + v_offset;
  }

  if(dim==16){
    int h_offset = h / 90;  // 4 parts
    int s_offset = s / 128;  // 2 parts
    int v_offset = v / 256; // 2 parts
    return 4*h_offset + 2*s_offset + v_offset;
  }
   
  if(dim==32){
    int h_offset = h / 90;  // 4 parts
    int s_offset = s / 128;  // 2 parts
    int v_offset = v / 128; // 4 parts
    return 8*h_offset + 4*s_offset + v_offset;
  }

  if(dim == 64){
    int h_offset = h / 90;  // 4 parts
    int s_offset = s / 64;  // 4 parts
    int v_offset = v / 128; // 4 parts
    return 16*h_offset + 4*s_offset + v_offset;

  }

  if(dim==128){
    int h_offset = h / 45;  // 8 parts
    int s_offset = s / 64;  // 4 parts
    int v_offset = v / 128; // 4 parts
    return 16*h_offset + 4*s_offset + v_offset;

  }

  if(dim==256){
    int h_offset = (int) (h / 22.5); // 16 parts
    int s_offset = s / 64;        // 4 parts
    int v_offset = v / 128;       // 4 parts
    unsigned int res =  16*h_offset + 4*s_offset + v_offset;
    return res;
  }
  assert(false);
  return 0;
}


template<unsigned int dim, bool lab>
void getHistHsv(Picture* picture, hist_hsv<dim, lab>* hist){
   if(!picture->IsDefined()){
     hist->SetDefined(false);
     return;
   }
   unsigned long size;
   const char* imgdata = picture->GetJPEGData (size);
   JPEGPicture rgb ((unsigned char *) imgdata, size);

   unsigned long int rgbSize;
   unsigned char* rgbData = rgb.GetImageData (rgbSize);

    const unsigned int numOfPixels = rgbSize / 3;

    unsigned long hist_abs[dim];

    for (unsigned int i = 0; i < dim; ++i){
        hist_abs[i] = 0;
    }

    for (unsigned long pos = 0; pos < (numOfPixels); ++pos) {   
        unsigned char r = rgbData[ (3*pos) ];
        unsigned char g = rgbData[ (3*pos) +1];
        unsigned char b = rgbData[ (3*pos) +2];
        unsigned int index;
        if(lab){
          index = pictureLabOffsetTable[ ((r/4)*4096) + ((g/4)*64) + b/4];
        } else {
           HSV hsv (r, g, b);
           index = getHSVIndex(hsv.h, hsv.s, hsv.v, dim);
        }
        ++hist_abs[index];
    } 

    delete[] imgdata;

    hist->set(hist_abs, numOfPixels);  
}


template<unsigned int dim,bool lab>
int getHistHsvVMT(Word* args, Word& result, int message,
                Word& local, Supplier s){
  Picture* arg = (Picture*) args[0].addr;
  result = qp->ResultStorage(s);
  hist_hsv<dim, lab>* res = (hist_hsv<dim, lab>*) result.addr;
  getHistHsv<dim,lab>(arg, res);
  return 0;
}

OperatorSpec getHistHsv8Spec(
  "picture -> hist_hsv8",
  "getHistHsv8(_)",
  "computes a histogram from a picture",
  "query getHistHsv8(paper)"
);

OperatorSpec getHistHsv16Spec(
  "picture -> hist_hsv16",
  "getHistHsv16(_)",
  "computes a histogram from a picture",
  "query getHistHsv16(paper)"
);

OperatorSpec getHistHsv32Spec(
  "picture -> hist_hsv32",
  "getHistHsv32(_)",
  "computes a histogram from a picture",
  "query getHistHsv32(paper)"
);

OperatorSpec getHistHsv64Spec(
  "picture -> hist_hsv64",
  "getHistHsv64(_)",
  "computes a histogram from a picture",
  "query getHistHsv64(paper)"
);

OperatorSpec getHistHsv128Spec(
  "picture -> hist_hsv128",
  "getHistHsv8(_)",
  "computes a histogram from a picture",
  "query getHistHsv128(paper)"
);

OperatorSpec getHistHsv256Spec(
  "picture -> hist_hsv256",
  "getHistHsv256(_)",
  "computes a histogram from a picture",
  "query getHistHsv256(paper)"
);

OperatorSpec getHistLab256Spec(
  "picture -> hist_lab_256",
  "getHistLab256(_)",
  "computes a histogram from a picture",
  "query getHistLab256(paper)"
);

Operator getHistHsv8Op(
   "getHistHsv8",
   getHistHsv8Spec.getStr(),
   getHistHsvVMT<8, false>,
   Operator::SimpleSelect,
   getHistHsvTM<8, false>
);

Operator getHistHsv16Op(
   "getHistHsv16",
   getHistHsv16Spec.getStr(),
   getHistHsvVMT<16, false>,
   Operator::SimpleSelect,
   getHistHsvTM<16, false>
);

Operator getHistHsv32Op(
   "getHistHsv32",
   getHistHsv32Spec.getStr(),
   getHistHsvVMT<32, false>,
   Operator::SimpleSelect,
   getHistHsvTM<32, false>
);

Operator getHistHsv64Op(
   "getHistHsv64",
   getHistHsv64Spec.getStr(),
   getHistHsvVMT<64, false>,
   Operator::SimpleSelect,
   getHistHsvTM<64, false>
);

Operator getHistHsv128Op(
   "getHistHsv128",
   getHistHsv128Spec.getStr(),
   getHistHsvVMT<128, false>,
   Operator::SimpleSelect,
   getHistHsvTM<128, false>
);

Operator getHistHsv256Op(
   "getHistHsv256",
   getHistHsv256Spec.getStr(),
   getHistHsvVMT<256, false>,
   Operator::SimpleSelect,
   getHistHsvTM<256, false>
);

Operator getHistLab256Op(
   "getHistLab256",
   getHistLab256Spec.getStr(),
   getHistHsvVMT<256, true>,
   Operator::SimpleSelect,
   getHistHsvTM<256, true>
);

/*

7 Algebra creation and initialisation

*/

class PictureAlgebra: public Algebra {
public:
    PictureAlgebra() : Algebra() {
        if (PA_DEBUG) cerr << "initializing PictureAlgebra" << endl;
        initPicture();
        AddTypeConstructor(picture);
        initHistogram();
        AddTypeConstructor(histogram);

        picture->AssociateKind(Kind::DATA());
        histogram->AssociateKind(Kind::DATA());

        AddTypeConstructor(&hist_hsv8);
        AddTypeConstructor(&hist_hsv16);
        AddTypeConstructor(&hist_hsv32);
        AddTypeConstructor(&hist_hsv64);
        AddTypeConstructor(&hist_hsv128);
        AddTypeConstructor(&hist_hsv256);
        AddTypeConstructor(&hist_lab256);

        hist_hsv8.AssociateKind(Kind::DATA());
        hist_hsv16.AssociateKind(Kind::DATA());
        hist_hsv32.AssociateKind(Kind::DATA());
        hist_hsv64.AssociateKind(Kind::DATA());
        hist_hsv128.AssociateKind(Kind::DATA());
        hist_hsv256.AssociateKind(Kind::DATA());
        hist_lab256.AssociateKind(Kind::DATA());

        AddOperator(&height);
        AddOperator(&width);
        AddOperator(&isgrayscale);
        AddOperator(&filename);
        AddOperator(&category);
        AddOperator(&date);
        AddOperator(&isportrait);
        AddOperator(&colordist);
        AddOperator(&equals);
        AddOperator(&containsOp);
        AddOperator(&simpleequals);
        AddOperator(&like);
        AddOperator(&scale);
        AddOperator(&cut);
        AddOperator(&flipleft);
        AddOperator(&mirror);
        AddOperator(&display);
        AddOperator(&exportop);

        AddOperator(&distanceOp);
        AddOperator(&getHistHsv8Op);
        AddOperator(&getHistHsv16Op);
        AddOperator(&getHistHsv32Op);
        AddOperator(&getHistHsv64Op);
        AddOperator(&getHistHsv128Op);
        AddOperator(&getHistHsv256Op);
        AddOperator(&getHistLab256Op);

        initLabOffsetTable();

    }
    ~PictureAlgebra() {
         delete[] pictureLabOffsetTable;
     }
};

extern "C"
Algebra* InitializePictureAlgebra(NestedList* nlPar, QueryProcessor *qpPar) {
    nl = nlPar;
    qp = qpPar;
//     cerr << "sizeof(Histogram) = " << sizeof(Histogram) << endl;
//     cerr << "sizeof(Picture) = " << sizeof(Picture) << endl;
    return new PictureAlgebra();
}
