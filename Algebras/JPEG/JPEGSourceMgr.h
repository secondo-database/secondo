/*
//paragraph [1] title: [{\Large \bf ]	[}]
//paragraph [2] subtitle: [{\bf ]	[}]
//[->] [$\rightarrow $]
//[=>] [$=> $]
//[TOC] [\tableofcontents]
//[newpage] [\newpage]


[1] (JPEG) Source Manager for JPEGAlgebra, Header-File

February 06    2004  Neumann (formatting,  removed unused methods)

January  14    2004  Schoenhammer (color-/brightness-distribution, testing),
                     Neumann (integration in JPEGSourceMgr, general revision)

January 08/09  2004    Neumann


1 JPEGSourceMgr: Defines, includes, types, forward declarations

*/

#ifndef JPEGSOURCEMGR_H
#define JPEGSOURCEMGR_H

using namespace std;

#include "JPEGAlgebra.h"

extern "C" {

// out-comment to see internal libjpeg-datastructures for debugging:
#define JPEG_INTERNALS

#ifdef JPEG_INTERNALS
#include "./DebugOnLinuxGNU/jpeglib.h"
#else
#include "jpeglib.h"
#endif

#include "jerror.h"
// for jmp_buf in followin struct and setjmp(jerr.setjmp_buffer) in init():
#include <setjmp.h>
#include "../Relation-C++/RelationAlgebra.h"


// The Error-Manager is afforded for (de-)compress-fns();
// jpeg\_error\_mgr: jpeglib.h line 643 ff, stores also user-error-functions:

  struct my_error_mgr
  {
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
  };

typedef struct my_error_mgr * my_error_ptr;

}

// forward declaration needed for compiler's signature checks:

class JPEG;

/**
2 Class JPEGSourceMgr - The libjpeg side of the interface. Interface.

Class JPEGSourceMgr is essentially a libjpeg-wrapper (C-Code !) for JPEG.

Goal: Separation of Interfaces to (a) libjpeg (this)  and  (b) Secondo (JPEG).

Possible later extension, that is changing of interface:

JPEGSourceMgr-instances surviving the lifetimes of their owners (JPEGs):

This may yield a gain of performance and flexibility (not at least a possible
emancipation of some unnecessairy constraints of the Secondo-Control-Mechanism
-- concerning object-lifetime-bound information and hence performance-loss in
regaining such information).

(see also Implementation description in JPEGSourceMgr.cpp).

*/

class JPEGSourceMgr {

/****************************************************************************
[2] Public constructors, destructor and methods

****************************************************************************/
 public:
  JPEGSourceMgr(JPEG* jObj);
  ~JPEGSourceMgr();

/**
[2] Initialization

*/
  void       DisconnectClient(JPEG * jObj);  // nulls jpegObj if destructed
  bool       CheckConnection();        // error check: JPEG<->JPEGSourceMgr
  bool       init();                   // Finishing work of cstr
  
/**
[2] Access-methods

*/
  int        GetID();
  static int GetNumAlive();   // get # of currently existing JPEGSourceMgrs
  static int GetCounter();    // # of all SrcMgrs seen in JPEGAlgebra lifetime
  jpeg_error_mgr* GetErrorMgr();     // access to libjpeg Error-Manager
  jpeg_decompress_struct* Get_cinfo();
  bool CInfoValid();
  
/**
[2] Decompression support. Extended JPEG-Info.

*/
  void parseFlobAgain();             // for use after reconfiguring cinfo
  void SetParseComment(bool yes_no); // demo: special cinfo-configure command
  void SetParseOnlyComment();
  bool ParseComment();               // dummy-Implem. for demo
  bool hasAdobeMarker();             // saw_Adobe_marker?
  bool Decompress(); // into rasterImage; FLOB-head allready had been parsed
  void ClearRasterImg();     // Discard rasterImg, e.g. before a new Decompress

/**
[2] Compression - with different configurations

*/

/**
PrepareCompress() and copyRasterToImageBuffer()

The first compression steps for all purposes: Source and Destination.

*/
  // Destination:
  bool PrepareCompress(FLOB * flob);
  // ... including whole rasterImg as Source:
  bool PrepareCompressDefault(FLOB * flob);

  // copyRasterToImageBuffer() -- Source:
  // Select data and create jpeg_write_scanlines-compatible imgBuffer from
  // rasterImage. Only one imgBuffer can be created, but this can be repea-
  // ted if the old imgBuffer has been processed.
  bool copyRasterToImageBuffer(int x, int y, int xdist, int ydist);
  bool copyRasterToImageBuffer();

/**
Configure-Methods:
The HowTo of Compression for various purposes, e.g. cut or size-reducing
compression.

*/
  bool ConfigureCut(int x, int y, int xdist, int ydist);

  bool ConfigureDownSize(int quality);   // not yet implemented

  bool ConfigureConvertToColor(colorS cs); // not yet implemented
    
/**
Execution of Compression: Compress()

Compresses the ImageBuffer to Jpeg-picture (valid Jpeg with
header etc.).  Checks before if (a) ImageBuffer exists, (b) flob.Size()
is sufficient, (c) the compress-info 'store\_info' is compatible with size
of ImageBuffer.

*/

  // Compress(): 
  // Preconditions: (a), (b) this is Secondo-related and hence the task of the
  // JPEG-methods/operators to provide initialized FLOB, (c) JPEG-method have
  // used appropriate JPEGSourceMgr-methods to configure 'store\_info' appro-
  // priately. (perhaps also writing such an compr\_info themselfes and giving
  // it to JPEGSourceMgr (not recommended)).

  bool Compress();

/**
[2] Color-Services.

*/

  // Add_XYZ_Distribution():
  // Augmenting jpegObj's values for frequency of the intensity of a color-
  // component or of brightness-intensities.
  // Used by compute_XYZ_D() to compute the distributions of color-compo-
  // nent-intensities (in the pixels) resp. compute the brightness-distribu-
  // tion (frequencies of intensities) (in the pixels):

  void AddBrightnessDistribution (double perCent, int valueIndex);
  void AddRGBDistribution (double perCent,
                           int colorIndex,    // 0 = R, 1 = G, 2 = B
                           int colorNumber);  // 0 .. 255
  void AddCMYKDistribution (double perCent,
                            int colorIndex,   // 0 = C, 1 = M, 2 = Y, 3 = K
                            int colorNumber); // 0 .. 255
  // compute_XYZ_D():
  bool ComputeBrightnessD(); // Computes Bri..Distribution from rasterImage
  bool ComputeRGBD();        //          RGB..
  bool ComputeCMYKD();       //          CMYK..



/*****************************************************************************
Public attributes

*****************************************************************************/
 public:

/**
[2] Data Structures

*/

  // comments in Jpeg-files are not parsed by default in libjpeg. parsing them
  // may be cpu-time-consuming. so remember if done.
  bool comment_parsed;
  bool store_info_valid;

/*****************************************************************************
Protected attributes

******************************************************************************/
 protected:

  JPEG * jpegObj; // the client to which this class instance serves.

  jpeg_decompress_struct cinfo; // libjpeg-DSs
  my_error_mgr jerr;

  int id; // id: identifier unique during live-time of JPEGAlgebra
  static int srcCounter; // # c'str'd in JPEGAlgebra lifetime
  static int numSrcAlive; // count (constructed-destructed) JPEGSourceMgrs

  // store_info:
  // to be used with (not yet implemented configuration methods, imgBuffer and
  // ->Compress(FLOB*)

  struct jpeg_compress_struct store_info;
  struct jpeg_error_mgr j_store_error;
  bool readyToCompress;
  bool compressCreated;


  // cinfo is correctly read in ?  have care: a ~valid~ cinfo doesn't imply
  // necessarily that its informations are ~consistent~ (after user configu-
  // ration actions).
  bool cinfoValid;


/*****************************************************************************
Private attributes

******************************************************************************/
 private:

/**
rasterImage:
decompressed image  resp. image to compress. The image is NOT automatically
scanned in init() for performance-reasons and since some configuration is
possibly required.

             [rows][columns\_in\_row][colour\_components\_in\_cell]

*/
  JSAMPIMAGE rasterImage;

  unsigned int rasterImgHeight; // cinfo.output_height - snapshot
  unsigned int rasterImgWidth;  // cinfo.output_width - snapshot
  unsigned int rasterImgComps;  // cinfo.output_components - snapshot
  bool decompressed;            // image data decompressed (, not only header)

/**
imgBuffer:
Buffer ready to compress. Compression allowed if 'store\_info' configured right,
and a FLOB of sufficient size has been provided by JPEG.
[->]see method Compress(FLOB[*])

*/
  JSAMPLE* imgBuffer;
  int imgBufHeight;
  int imgBufWidth;
};

#endif
