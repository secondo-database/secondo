/*******************************************************************************
//paragraph [1] title: [{\Large \bf ]	[}]
//paragraph [2] subtitle: [{\bf ]	[}]
//[->] [$\rightarrow $]
//[=>] [$=> $]
//[TOC] [\tableofcontents]
//[newpage] [\newpage]


[1] JPEGAlgebra (and JPEG): Header-File

February 06, 2004  Neumann (formatting 1, Object c'str + d'str + tracing
corrected)

February 02   2004   Neumann (Std-c'str for use in CastJINFO(), formatting)

January 21,  2004  Schoenhammer (Type jinfo)

January  14    2004  Schoenhammer (color-/brightness-distribution, testing),
                     Neumann (integration in JPEGSourceMgr, general revision)

January  08/09 2004  Neumann (JPEGSourceMgr: libjpeg w/o file; OO-Separation of
Interfaces)

        ( ... formerly included in JPEGAlgebra.cpp: )

December 31, 2003  Neumann (extensions, 'defined', some error-tests, 'alpha'--)

December 30, 2003  Schoenhammer (InJPEG-WorkAround)

December 29, 2003  Neumann (first revision)

prior to
December 24, 2003  Schoenhammer

*/

/*******************************************************************************
\rule {100 pt}{1 pt} \linebreak

*******************************************************************************/

/*
1 JPEGAlgebra: Defines, includes, typedefs

*/

using namespace std;

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "StandardAttribute.h"
#include "FLOB.h"
#include "Base64.h"
#include "../Relation-C++/RelationAlgebra.h"

// out-comment to disable object-tracing-outputs
//#define DEBUGJPEG

#include "JPEGSourceMgr.h"

#ifndef JPEGALGEBRA_H
#define JPEGALGEBRA_H

// Color-Space:       0,   1,    2,              3
enum colorS { grayscale, rgb, cmyk, colS_undefined };

// marker-values for (yet) invalid member values
#define NUMCOMPS_INVALID    -1
#define COLORSPACE_INVALID  colS_undefined
#define HEIGHT_INVALID      777777
#define WIDTH_INVALID       777777
#define COMPLEVEL_INVALID   -1
#define COMMENT_INVALID     "Jpeg-Comment has not been scanned."

/*
2 Type Constructor ~jpeg~

2.1 Data Structure - Class ~JPEG~

*/
class JPEG : public StandardAttribute
{
/**
[2] Class JPEGSourceMgr as a 'friend'

Instead of putting here in JPEG everything, we separate JPEGLib-C-Code from
DB-related C++ Code found here,making the interface to libjpeg more flexible
and easing a possible existence of libjpeg-Parsing-results *independent* from
this JPEG-Object here.

This also leads to Extensibility: without changing size and data-element-count
and / or types of the 'root-object', we can later decide to access further
information available from the libjpeg-cinfo structure by defining new methods
and / or members in JPEGSourceMgr. These would not be local or totally global.

*/
  friend class JPEGSourceMgr;
  friend ostream& operator<<( ostream& o, JPEG& jp );

  public:

/**
Constructors and destructors

*/
  JPEG() {              // standard constructor
    src          = 0;
    srcConnected = false;
    destructing  = false;
    ++numJpegsAlive;
    ++jpegCounter;
      
    #ifdef DEBUGJPEG
    cout << "\nStd-c'str JPEG(). alive (w/o casts): " << numJpegsAlive
       << "   c'str-calls: " << jpegCounter <<  "   including casts: "
       << castJPEGs << endl;
    #endif
  };

  JPEG( const int size );
  ~JPEG();
    

/**
Methods

*/
  // public methods

  // Enabling usage as relation attribute type:
  bool      IsDefined() const;
  void      SetDefined( bool Defined);
  void      Destroy(); // support-fn() for consistency of FLOB-states.
  size_t    HashValue();
  void      CopyFrom(StandardAttribute* right);
  int       Compare(Attribute * arg);
  bool      Adjacent(Attribute * arg);
  JPEG*     Clone();
  ostream&  Print( ostream &os );
  int       NumOfFLOBs();
  FLOB *    GetFLOB(const int i);

  // member access functions: Get__ / Is__
  bool    IsColoured();
  int     GetWidth();
  size_t  GetHeight();
  int     GetCompLevel();
  string  GetComment();
  bool    GetAllPictureBytes(char *& buffer); // also reserves buffers mem
  /** Get cinfo from source. Not recommended. Evades consistency checks! */
  struct  jpeg_decompress_struct* Get_cinfo();
  bool CInfoValid();
  double  GetBrightnessValue (int brightnessValue, int dummy = 0);
          /* dummy-parameter is for having same signature */
  double  GetRGBValue (int colorIndex, int colorNumber);
  double  GetCMYKValue (int colorIndex, int colorNumber);
  colorS  GetColorSpace(); // overloaded for comfortability
  int     GetColorSpace(int dummy_int);  
  string  GetColorSpace(string dummy); 
  int     Get_num_components(); // get # of color components (per pixel)
  static int GetNumAlive(); // number of JPEGs currently existing, not
                            // managed by Secondo-System

  // ...: Set__

  // initialization
  bool  InitializeJPEG(); // finishes work of c'str including init() of SourceMgr
  void  InitializeBrightnessDistribution();
  void  InitializeRGBDistribution();
  void  InitializeCMYKDistribution();
  void  DropSrcMgr(); // Discard associated JPEGSourceMgr

  // IO-Functions
  void  Encode  ( string& textBytes );   // for IO to Secondo System
  void  Decode  ( string& textBytes );   

  // Usage for operator-implementations (supporting value mapping etc.)
  bool  SaveToFile ( char *fileName ); // IO to OS-file
  bool  Show       ();                 // devel. + test: calls ImageMagic ...
  bool  Demo       ();                 // ... of JPEGSourceMgr-usage

  // (helper fn()s for database-specific operations / for Secondo-operators)
  bool  ColDTest   ();
  bool  CompBright (JPEG* injpeg, double dist);
  bool  CompRgb    (JPEG* injpeg, double rgbdist);
  bool  CompRgb    (JPEG* injpeg, double rdist, double gdist, double bdist);
  bool  Cut        (JPEG * outjpeg, int x, int y, int xdist, int ydist);
  bool  DownSize   (JPEG * outjpeg, int quality);
  bool  Tile(JPEG * outjpeg, int x, int y, int xdist, int ydist);

  // (for Operations on 'picture'(-raster) etc. ->JPEGSourceMgrs 'services')

 private: // Private methods
  void DisconnectFromSrc  (); // notify src(-Mgr) if destructor called

/**
 Attributes

*/    
 private: // Private attributes

  JPEGSourceMgr* src; // does all detailed work related to libjpeg
  bool    defined;    // trace validity of JPEG (also on a semantical level)
  FLOB    picture;  
  bool    canDelete;
  size_t  height;
  size_t  width;
  bool    coloured;    
  int     compLevel;    // not yet supported
  string  comment;      // only partially supported by now
  bool    destructing;  // for DisconnectFromSrc()
  colorS  colorSpace;  
  int     numComponents;  //0: gray, 3 or 4: color

  bool brightnessDcomputed;   // set true with computation by src (-manager)
  bool rgbDcomputed;          // with these flags C/B-distributions only
  bool cmykDcomputed;         //  need to be computed if used

  double brightnessDistribution[256];    //for grayscale. 
  double rgbDistribution       [256][3]; //for RGB; index 0=R, 1=G, 2=B
  double cmykDistribution      [256][4]; //for CMYK; index 0=C, 1=M, 2=Y, 3=K

 public: // Public attributes

  bool srcConnected; // valid JPEGSourceMgr connected? dynamically set

  // for object tracing:
  static int numJpegsAlive;  // #(constructed-destructed) _by_JPEGAlgebra_
                             // memory of the cast ones managed by Secondo
  static int jpegCounter;    // # c'str'd in JPEGAlgebra-lifetime (with casts)

  static int castJPEGs;
  static int clonedJPEGs;
  static int createdJPEGs;
  static int closedJPEGs;
  static int deletedJPEGs;
 
};
/**
[newpage]

*/

/*
3 Type Constructor ~JINFO~

3.1 Data Structure - Class ~JINFO~

*/
class JINFO: public StandardAttribute
{

 public:

/*
Declaration Part with definition of the DataStructure and of the operators.

*/
  JINFO(){}  // Std.-C'str (needed for CastINFO())


/*
Constructor-Declaration

*/
  JINFO ( const bool defined,
          const int inHeight, const int inWidth,
          bool inColoured, int inCS, int inNumComp, ListExpr inPict );
/*
Destructor-Declaration

*/
  ~JINFO();
/*
Declaration of the class-methods.

*/
  int           GetHeight         ();
  int           GetWidth          ();
  bool          IsColoured        ();
  int           Get_num_components();
  colorS        GetColorSpace();
  void          GetcDistribution  ( double outDistr [256][4] );
  void          SetcDistribution  ( int compIndex, int comp, double value );
  ListExpr      GetPictureList    ();
  void          Set               ( const bool D,       const int inHeight,
                                    const int  inWidth, bool      inColoured,
                                    int        inCS,    int       inNumComp,
                                    ListExpr   inPict);

/*
The following virtual classes are needed to use ~JINFO~ as an
attribute type in tuple definitions.

*/
  int           Compare   (Attribute * arg);
  bool          Adjacent  (Attribute * arg);
  JINFO*        Clone     ();
  bool          IsDefined () const;
  void          SetDefined( bool Defined );
  int           Sizeof    () const;
  size_t        HashValue ();
  void          CopyFrom  (StandardAttribute* right);
  ostream&      Print     ( ostream &os );

/*
Private Attributes of the class ~JINFO~.

*/
 private:
  bool     defined;
  int      height;
  int      width;
  bool     coloured;
  colorS   colorSpace;
  int      numComponents;
  double   cDistribution[256][4];
  ListExpr picture;
};

#endif
