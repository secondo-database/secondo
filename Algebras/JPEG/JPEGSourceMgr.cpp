/*
//paragraph [1] title: [{\Large \bf ]	[}]
//paragraph [2] subtitle: [{\bf ]	[}]
//[->] [$\rightarrow $]
//[=>] [$=> $]
//[TOC] [\tableofcontents]
//[newpage] [\newpage]


[1] (JPEG) Source Manager for JPEGAlgebra, Implementation-File

February 09, 2004  Neumann (corrections in object destruction)

February 06    2004  Neumann (formatting,  removed unused methods)

January  14    2004  Schoenhammer (color-/brightness-distribution, testing),
                     Neumann (integration in JPEGSourceMgr, general revision)

January 08/09  2004    Neumann

*/
/*******************************************************************************
\rule {100 pt}{1 pt} \linebreak

*******************************************************************************/
/*******************************************************************************
1 JPEGSourceMgr: Additional Defines and includes for Implementation

*******************************************************************************/

using namespace std;

#include "JPEGSourceMgr.h"
#include "JPEGAlgebra.h"

extern "C" {

// for jpeg_flob_src()/jpeg_flob_dest() (libjpeg-eXtension):
#include "jdatasrcX.c"
#include "jdatadstX.c"

#ifdef JPEG_INTERNALS
#include "./DebugOnLinuxGNU/jdapimin.c"
#include "./DebugOnLinuxGNU/jdinput.c"
#include "./DebugOnLinuxGNU/jdmarker.c"
#endif
}

/**
2 Class JPEGSourceMgr - The libjpeg side of the interface. Implementation.

Class JPEGSourceMgr is essentially a libjpeg-wrapper (C-Code !) for JPEG, but
has possible later extensions.

Goal: Separation of Interfaces to (a) libjpeg (this)  and  (b) Secondo (JPEG).

(see also description in JPEGSourceMgr.h)

Implementation issues:

JPEGSourceMgr is friend of JPEG.

With an independent interface to libjpeg having information-storage- and hence
information-exchange- capabilities the coordination of the implementation of
separated Secondo-operators promises to be easier:

Otherwise operator 'a' doesn't 'know' what operator 'b' is doing, nor can they
exchange commom information in a coordinated way. Coordination is then much more
left as constraint of different operator-implementors. Common use of (parts of)
algorithm (special ones coping with libjpeg) is also eased.

*/

int JPEGSourceMgr::srcCounter = 0;
int JPEGSourceMgr::numSrcAlive = 0;

/**
2.1 Constructors, Destructor, Connection to JPEG

*/

/**
[2] Constructor JPEGSourceMgr(JPEG[*] )

*/

JPEGSourceMgr::JPEGSourceMgr(JPEG* jObj){
  id               = ++srcCounter;
  ++numSrcAlive;
  jpegObj          = jObj;
  cinfoValid       = false; // header not yet scanned and parsed
  rasterImage      = 0;     // nothing read yet -> don't know size now
  rasterImgHeight  = 0;
  rasterImgWidth   = 0;
  rasterImgComps   = 0;
  decompressed     = false; // data not yet decompressed into rasterImage
  comment_parsed   = false; // assume libjpeg-default not to parse Jpeg-Comments
  imgBuffer        = 0;     // Compression-Source not yet prepared
  imgBufHeight     = 0;
  imgBufWidth      = 0;
  store_info_valid = false; // basic compression params not yet set
  readyToCompress  = false; // compression not yet congigured
  compressCreated  = false; // libjeg compression struct not yet allocated
  
  #ifdef DEBUGJPEG
  cout << "\nConstructor JPEGSourceMgr(). ID: " << id
       << "   alive: " << numSrcAlive << endl;
  #endif
}

/**
[2] Destructor

*/
JPEGSourceMgr::~JPEGSourceMgr(){
  jpeg_destroy_decompress (&cinfo);
  --numSrcAlive;

  // out-comment if Decompress() lets libjpeg reserve memory for rasterImage:
  if (rasterImage != 0)
    ClearRasterImg();

  #ifdef DEBUGJPEG
  cout << "\nDestructor ~JPEGSourceMgr(). ID:" << id
       << " alive: " << numSrcAlive << endl;
  #endif
}

/**
DisconnectClient()

*/
void JPEGSourceMgr::DisconnectClient(JPEG * jObj){
  assert(jObj == jpegObj);
  assert(jpegObj->destructing);
  jpegObj = 0;
}


// CheckConnection:
// This will be a restrictive error check: also the numbers of active src's and
// JPEGs must match, not only the correct association between JPEG and src.
//
// This method does seem to be NOT superfluous because of
//
//   * the complicated construction mechanisms of algebra-object-construction
//     in Secondo,
//   * the possible future extension towards src's surviving their (initial)
//     JPEG-owners -- possibly yielding some performance and flexibility
//     advantage: cf the persistance- and reusage- logic in
//     libjpeg : advantage:jpeg\_stdio\_src().

bool JPEGSourceMgr::CheckConnection(){

  if (!jpegObj) // nulled only by jpegObj->destructor via DisconnectFromSrc()
    return false;
  if (jpegObj->src != this)
    return false;
  // JPEGSourceManager needs a ~valid~ FLOB in jpegObj ... failure here
  // treated as 'bad connection':
  FLOB_Type jFlobType = jpegObj->picture.GetType();
  // missing for class FLOB: FLOB-Type 'Uninitialized' !!!
  if ( (jFlobType != InMemory) &&
       (jFlobType != InDiskSmall) &&  // currently not implemented?
       (jFlobType != InDiskLarge)
#ifdef PERSISTENT_FLOB
       && (jFlobType != InDiskMemory))
#else
      )
#endif
    return false;
  // 1:1 Relation of JPEGs and JPEGSourceMgr's?
  if (numSrcAlive != JPEG::numJpegsAlive)
    return false;

  return true; // nothing buggy in JPEG<->JPEGSourceMgr - connection
}

//METHODDEF (void)  // libjpeg portability-generic-typedef
static void
my_error_exit (j_common_ptr cinfo) // user-error-fn() stored in error-manager
{
  my_error_ptr myerr = ( my_error_ptr ) cinfo->err;
  (*(cinfo->err->output_message)) (cinfo);
  longjmp(myerr->setjmp_buffer, 1);
}


/**
2.2 Inititialization of JPEGSourceMgr ~and~ JPEG

*/


/**
[2] init() (version 1  or )  version 2:

Finishing the work of the cstr. Parsing the jpegObj[->]picture ~now~, since ~now~
the FLOB has been set up by jpegObj. [->]below (after version 2): overloaded
init(JPEGSourceMgr\&) omitting a superfluous parse-procedure.

There are actually two versions of init(), since it is not yet clear which one
is to be preferred. cf. following comments. (Jan 13th 2004: version 1 using
String-Streams removed for clearness (having overview in reading the code))

*/

// ( removed version 1:
// For Short: a String-Stream: FILE * fmemopen(bytes,...) may remain open for
// read and write, using as buffer 'bytes', which allready contains 'picture'.
// Limited portability disadvantage. )


// JPEGSourceMgr::init() - version 2:
// own libjpeg-'data-source-manager' connecting libjpeg to (flob) picture:
// solution   libjpeg:datasrc.c   with   own  jpeg_flob_src().

bool JPEGSourceMgr::init(){

  // Setup Error-Manager
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  if (setjmp(jerr.setjmp_buffer))     // LOW-LEVEL (asm) return point if ERROR
  {
    // jpeg_destroy_decompress(&cinfo); // destroyed in dstr
    return false;
  }

/**
Overview libjpeg Img-Parsing:
DeCompress [->] ReadFlob (or Buffer) [->] ReadHeader ([->] ERRORjmp or CleanUp):

*/
  jpeg_create_decompress(&cinfo);
  jpeg_flob_src(&cinfo, jpegObj->GetFLOB(0));
  jpeg_read_header (&cinfo, TRUE);

  // Code does not arrive here if jpeg-Error-Manager detects error.
  // So let's store the state, that cinfo can be used consistenty from now:
  cinfoValid = true;

/**
Initialize Jpeg-Member-Attributes (besides 'picture'):

*/

// We extract HERE the most important things, doing other extractions (if
// needed) in (JPEGSourceMgr-) methods which we could call only when needed, ..
// including possible usage of Get_cinfo()  for seldomly used requests ...
// Usage of Get_cinfo() is only encouraged for fetching information, ~not~ for
// setting cinfo-values.

  jpegObj->width = cinfo.image_width;
  jpegObj->height = cinfo.image_height;
  jpegObj->numComponents = cinfo.num_components;

  if (comment_parsed)  // in actual implementation *not* parsed
    jpegObj->comment = "JPEGSourceMgr::initialization_dummy_comment";

  switch (cinfo.jpeg_color_space) {
    case 1:
      jpegObj->coloured =  false;
      jpegObj->colorSpace = grayscale;
      break;
    case 3:
      jpegObj->coloured =  true;
      jpegObj->colorSpace = rgb;
      break;
    case 5:
      jpegObj->coloured =  true;
      jpegObj->colorSpace = cmyk;
      break;
    default:
      jpegObj->coloured = (cinfo.num_components > 1);
      jpegObj->colorSpace = colS_undefined;
      cerr << "Colorspace not supported" << endl;
      //jpegObj->SetDefined(false); // NOW picture read AND also Header, since

      //return false; // JPEG::defined is still false; don't need to set
                    // JPEG and InJPEG() will yield 'failure' to QP
  }

  jpegObj->SetDefined(true); // since FLOB AND jpeg-header read now.

  // CleanUP  is somewhat different from typical libjpeg usage:
  // It has been altered for use of cinfo the lifetime of *this and then to
  // be cleaned up in the destructor.  An implementation with temporary cinfo
  // w'd have destroyed cinfo -- as demonstrated in the libjpeg-documentation.

  // jpeg_destroy_decompress (&cinfo); // w'd have been called in libjpeg-demo

  return true; // success will let InJPEG return success + JPEG-Address to QP
}

/**
2.3 Get../ Is.. and Set.. Methods

*/

// GetCounter():
// Get number of all JPEGSourceMgrs c'str'd since JPEGAlgebra-Construction

int JPEGSourceMgr::GetCounter(){
  return srcCounter;
}

int JPEGSourceMgr::GetNumAlive(){ // Get number of currently existing SrcMgrs
  return numSrcAlive;
}

int JPEGSourceMgr::GetID(){       // unique
  return id;
}

/**
Get\_cinfo():

The use of this method has architectural consequences!

It is not very save... delivering the protected cinfo to the public ...
allowing C-Style bugs as before ...

Since a pointer is returned, one could also directly configure cinfo ... for
example to [->]parseFlobAgain() with a configuration largely changed.

BUT: not recommended, it would be better to supply a corresponding method here,
so other operator can use it and must not do the work again.

*/
jpeg_decompress_struct* JPEGSourceMgr::Get_cinfo(){

  return (jpeg_decompress_struct*) &cinfo;
}

bool JPEGSourceMgr::CInfoValid(){
  return cinfoValid == true;
}


// GetErrorMgr():
// another C-Like offer to the JPEGSrcMgr-User (JPEG-operator). It would be
// preferrable instead to write a method like GetErrorId() or GetErrorMsgFor-
// User()... and the like.

jpeg_error_mgr* JPEGSourceMgr::GetErrorMgr(){
  return (jpeg_error_mgr*) &(jerr.pub);
}


/*****************************************************************************
2.4 Color-Services for JPEG

*****************************************************************************/

void JPEGSourceMgr::AddBrightnessDistribution (double perCent, int valueIndex)
{
  assert (valueIndex >= 0 && valueIndex <=255);
  jpegObj->brightnessDistribution  [valueIndex] =
    jpegObj->brightnessDistribution[valueIndex] + perCent;
}

void JPEGSourceMgr::AddRGBDistribution (double perCent, int colorIndex,
                                                        int valueIndex)
{
  assert (colorIndex >= 0 && colorIndex <=2);
  assert (valueIndex >= 0 && valueIndex <=255);

  jpegObj->rgbDistribution   [valueIndex][colorIndex] =
    jpegObj->rgbDistribution [valueIndex][colorIndex] + perCent;

}

void JPEGSourceMgr::AddCMYKDistribution (double perCent, int colorIndex,
                                                         int valueIndex)
{
  assert (colorIndex >= 0 && colorIndex <=3);
  assert (valueIndex >= 0 && valueIndex <=255);

  jpegObj->cmykDistribution   [valueIndex][colorIndex] =
    jpegObj->cmykDistribution [valueIndex][colorIndex] + perCent;
}


bool JPEGSourceMgr::ComputeBrightnessD(){

// to be used with  NEW-VERSION  of Decompress()


  assert(cinfoValid); // if not then probable values for height, width,
                      // color_space not correct.

  // Decompression might not be done yet ...:
  // Imagine in Munich there is a huge Secondo-database containing all bavari-
  // an official Jpegs, about 2.000.000 (including one very nice and commented
  // of Mr. Stoiber visiting Regensburg)   Now we want to find all those who
  // contain the word 'Regensburg'. There is no index. Since Mr. Huber is joi-
  // ning us at 21:00h in the evening (inspecting efficiency of bavarian public
  // servants): Should our other operator examining the Jpeg-comments decom-
  // press the images before?  ;-)
  if (!decompressed)
    Decompress();

  assert(decompressed); // no problem occured .. for examply with memory?

  // Currently only Brightness-Distribution for Grayscale. Possible for color
  // to: average of intensity of color-components. check grayscale:
  assert( rasterImgComps == 1);// <=> JPEG::numComponents == 1 or inconsistent

  // As a comment code copied from method Decompress():
  //
  // rasterImgHeight = cinfo.output_height;     // |[rows]
  // rasterImgWidth  = cinfo.output_width;      // |[ ]..[cells]
  // rasterImgComps  = cinfo.output_components; // |[ ]..[ ]....[colcmps]
  //
  // These are consistent with JPEG::height,width,numComponents if only
  // JPEGSourceMgr has accessed JPEG for setting attributes. Variation of
  // cinfo doesn't matter since rasterImgHeight, ... are its values during
  // decompression time stored to savely access rasterImage in methods like
  // this one.

  double pc; // the perCent-Value of one pixel;
  pc = ( (double)100.0 / (double)
                         ( rasterImgWidth * (double) rasterImgHeight ) );

  unsigned int w, h; // w_idth(#columns), h_eight(#rows)

  for (h = 0; h < rasterImgHeight; h++)
    for (w = 0; w <= rasterImgWidth - 1; w++)
    {
      // cout << w <<":" << (int) buffer[h][w] << "  ";
      AddBrightnessDistribution (pc, ((int) rasterImage [h][w][0])); //###

      // with LIBJPEG-version of Decompress() this would be:
      // AddBrightnessDistribution (pc, ((int) rasterImage [h][0][w])); //###
    }


  // ### as a comment see code copied from Decompress():
  //
  //  for (unsigned int w = 0; w < rasterImgWidth; w++)
  //  {
  //    rasterImage[row_no][w] = new JSAMPLE[rasterImgComps];
  //    for (unsigned int c = 0; c < rasterImgComps; c++)
  //      rasterImage[row_no][w][c] = (int)buffer [0][w*rasterImgComps+c];
  //  }

  return true;
}


bool JPEGSourceMgr::ComputeRGBD(){

// to be used with  NEW-VERSION  of Decompress()

  assert(cinfoValid);

  // Decompression might not be done yet ...
  if (!decompressed)
    Decompress();

  assert(decompressed);
  assert( rasterImgComps == 3);// <=> JPEG::numComponents == 3 or inconsistent

  // As a comment code copied from method Decompress():
  //
  // rasterImgHeight = cinfo.output_height;     // |[rows]
  // rasterImgWidth  = cinfo.output_width;      // |[ ]..[cells]
  // rasterImgComps  = cinfo.output_components; // |[ ]..[ ]....[colcmps]
  //
  // see alse comments in ComputeBrightnessD()

  double pc; // the perCent-Value of one pixel;
  pc = ( (double)100.0 / (double)
                         ( rasterImgWidth * (double) rasterImgHeight ) );

  unsigned int w, h; // w_idth(#columns), h_eight(#rows)

  for (h = 0; h < rasterImgHeight; h++)
    for (w = 0; w <= rasterImgWidth - 1; w++)
    {
       // cout << w <<":" << (int) rasterImage[h][w] << "  ";
       AddRGBDistribution (pc, 0, ( (int) rasterImage [h] [w][0])); // red
       AddRGBDistribution (pc, 1, ( (int) rasterImage [h] [w][1])); // green
       AddRGBDistribution (pc, 2, ( (int) rasterImage [h] [w][2])); // blue

      // with LIBJPEG-version this would be:
      //  AddCMYKDistribution (pc, 0, ( (int) rasterImage [h] [0][w+0])); // r
      //  AddCMYKDistribution (pc, 1, ( (int) rasterImage [h] [0][w+1])); // g
      //  AddCMYKDistribution (pc, 2, ( (int) rasterImage [h] [0][w+2])); // b
    }
  return true;
}


bool JPEGSourceMgr::ComputeCMYKD(){

// to be used with  NEW-VERSION  of Decompress()

  assert(cinfoValid);

  // Decompression might not be done yet ...
  if (!decompressed)
    Decompress();

  assert(decompressed);
  assert( rasterImgComps == 4);// <=> JPEG::numComponents == 4 or inconsistent

  // As a comment code copied from method Decompress():
  //
  // rasterImgHeight = cinfo.output_height;     // |[rows]
  // rasterImgWidth  = cinfo.output_width;      // |[ ]..[cells]
  // rasterImgComps  = cinfo.output_components; // |[ ]..[ ]....[colcmps]
  //
  // see also comments in ComputeBrightnessD()

  double pc; // the perCent-Value of one pixel;
  pc = ( (double)100.0 / (double)
                         ( rasterImgWidth * (double) rasterImgHeight ) );

  unsigned int w, h; // w_idth(#columns), h_eight(#rows)

  for (h = 0; h < rasterImgHeight; h++)
    for (w = 0; w <= rasterImgWidth - 1; w++)//w+=4)
    {
      // cout << w <<":" << (int) rasterImage[h][w][0] << "  ";
      // cout << "Farbindex 0 -> Zeile:" << h << " Spalte:" << w << endl;
      // cout << "Farbindex 1 -> Zeile:" << h << " Spalte:" << w << endl;
      // cout << "Farbindex 2 -> Zeile:" << h << " Spalte:" << w << endl;
      // cout << "Farbindex 3 -> Zeile:" << h << " Spalte:" << w << endl;

      AddCMYKDistribution (pc, 0, ( (int) rasterImage [h][ w] [0])); // C
      AddCMYKDistribution (pc, 1, ( (int) rasterImage [h] [w] [1])); // M
      AddCMYKDistribution (pc, 2, ( (int) rasterImage [h] [w] [2])); // Y
      AddCMYKDistribution (pc, 3, ( (int) rasterImage [h] [w] [3])); // K

      // with LIBJPEG-version this would be:
      //      AddCMYKDistribution (pc, 0, ( (int) rasterImage [h] [0][w+0]));
      //      AddCMYKDistribution (pc, 1, ( (int) rasterImage [h] [0][w+1]));
      //      AddCMYKDistribution (pc, 2, ( (int) rasterImage [h] [0][w+2]));
      //      AddCMYKDistribution (pc, 3, ( (int) rasterImage [h] [0][w+3]));
    }
  return true;
}


/**
2.5 Decompression and preparations

*/

/****************************************************************************
2.5.1 Services for Configuring (libjpeg-) cinfo, flexible reparsing etc.

****************************************************************************/

/**
After reconfiguring parse-options, the FLOB of the associated JPEG can be
parsed again, for example to parse something that was skipped with the libjpeg-
parse-configuration used before. Use with care: not yet tested!

*/
void JPEGSourceMgr::parseFlobAgain(){
  assert(jpegObj != 0);
  assert(jpegObj->IsDefined());

  // cinfo valid till now first must be discarded:
  jpeg_destroy_decompress (&cinfo);

  init();

}



/*****************************************************************************
[2] Different Services simplifying libjpeg-usage

*****************************************************************************/

// SetParseComment():
// Configures cinfo to also parse the comment included in the jpeg-picture
// which is not parsed by default.

void JPEGSourceMgr::SetParseComment(bool yes_no){

  // to do:  not yet implemented;
  // set variable in cinfo?  set another user-defined libjpeg-fn() in cinfo?

  // to_do_set_the_apropriate_options()

  // may be of more general use as only to output this comment to the user.
}

// SetParseOnlyComment():
// Configures cinfo to also parse the jpeg-picture skipping all markers except
// the comment.  The following ->parseFlobAgain() might be much faster with this

void JPEGSourceMgr::SetParseOnlyComment(){

  // not yet implemented

}

// ParseComment():
// demo with dummy-implem.

bool JPEGSourceMgr::ParseComment(){

   // SetParseComment(true); // assume we not just only need the comment
   // ParseFlobAgain();  // have care : not tested

   // fetch the comment from where configured to be stored (by SetParseComment)
   // dummy-impl.:
   string dummy_comment = "this is a dummy-comment by ParseComment()";
                        // the fetched comment

   comment_parsed = true;
   jpegObj->comment = dummy_comment;

   // SetParseComment(false);  // now we have, and can skip it next time ...
   return (dummy_comment != "");
}

//  to do if needed: ... similar methods.

/** Jpegs with Adobe-markers are the only ones whose colorspace is converted to
    CMYK in the libjpeg-default-configuration represented by */

bool JPEGSourceMgr::hasAdobeMarker(){
  return cinfo.saw_Adobe_marker;
}


/*****************************************************************************
2.5.2 Decompress()

Read image into JPEGSourceMgr::rasterImage[][][] after jpeg\_read\_header().

See also [->]init(), [->]parseAgain() and  (optionally) calls to
'decompress-configure-functions()'.

We read into an 3-dim Array instead an 1-dim buffer as needed by compression:
operations on this datastructure are easier to understand, less cryptical and
therefore less error-prone.

3-dim Array remains available till JPEGSourceMgr-Destructor or explicit request
to be discarded.

Decompression is done by demand, but only needed once. Not all operators need
this time consuming computation!

*/
bool JPEGSourceMgr::Decompress(){

// 'NEW-VERSION'   ( ,  NOT  'LIBJPEG-MEM-VERSION' )

  // cf. example.c from libjpeg (author: Lane)

  // don't overwrite a previous scan: must be cleared first
  if (rasterImage != 0)
    return false;

  (void) jpeg_start_decompress (&cinfo);


  // prevent accidential change inco cinfo:
  rasterImgHeight = cinfo.output_height;     // |[rows]
  rasterImgWidth  = cinfo.output_width;      // |[ ]..[cells]
  rasterImgComps  = cinfo.output_components; // |[ ]..[ ]....[colcmps]

  // height == #rows, each pixel in a row has num_components col_comps =>
  // rasterImage is an array of these rows with array-size height
  rasterImage = new JSAMPARRAY [rasterImgHeight];

  int row_stride = rasterImgWidth * rasterImgComps;

  JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)
    ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

  // read single scanlines till no more found
  assert (cinfo.output_scanline == 0); // be sure!
  int row_no = cinfo.output_scanline;

  while ((cinfo.output_scanline < cinfo.output_height) // output_height const?
      && (cinfo.output_scanline < rasterImgHeight )) { // to be sure

    jpeg_read_scanlines(&cinfo, buffer, 1);

    // 'row = new colourC [#pixels_in_row][ _ignored_#of_colourC_in_pixel]'
    rasterImage[row_no] = new JSAMPROW[rasterImgWidth];


    // the following is nicely ordered, but lots slower then with memcpy ...
    // with memcpy: caveat BITS_IN_SAMPLE etc. ... + cryptical usage of DS
    for (unsigned int w = 0; w < rasterImgWidth; w++)
    {
      rasterImage[row_no][w] = new JSAMPLE[rasterImgComps];
      for (unsigned int c = 0; c < rasterImgComps; c++)
        rasterImage[row_no][w][c] = (int)buffer [0][w*rasterImgComps+c];
    }
    ++row_no;

  }///~while

  (void) jpeg_finish_decompress(&cinfo);

  decompressed = true;

  // Lanes 'Step 8: Release JPEG decompression object' is done by d'str
  // or by explicit request

  return true;
}

// ClearRasterImg():
// Discard rasterImg memory, e.g. before a newly configured decompression

void JPEGSourceMgr::ClearRasterImg(){
  assert(rasterImage);
  assert(rasterImgHeight);
  assert(rasterImgWidth);
  assert(rasterImgComps);
  for (unsigned int r = 0; r < rasterImgHeight; r++)
  {
    for (unsigned int w = 0; w < rasterImgWidth; w++)
      delete rasterImage[r][w];
    delete rasterImage[r];
  }
}



/*****************************************************************************
2.6 Compression and preparations

*****************************************************************************/

/**
2.6.1 PrepareCompress() and related methods

Besides copyRasterToImageBuffer the first step in Compression.

*/
bool JPEGSourceMgr::PrepareCompress(FLOB * flob)
{
  assert(flob != 0);
  store_info_valid = true;
  assert(store_info_valid);

  if (!decompressed) // rasterImage allready constructed?
    Decompress();

  store_info.err = jpeg_std_error(&j_store_error);
  jpeg_create_compress(&store_info);
  compressCreated = true; // later checked if memory has to be released
  
  jpeg_flob_dest(&store_info, flob); // connect to destination
  return true;
}

/**
PrepareCompressDefault()

PrepareCompress including copying whole rasterImage as Source.

*/
bool JPEGSourceMgr::PrepareCompressDefault(FLOB * flob)
{
  assert(flob != 0);
  store_info_valid = true;
  assert(store_info_valid);

  if (!decompressed) // rasterImage allready constructed?
    Decompress();

  if (rasterImage == 0)
    return false;

  if (imgBuffer != 0)  // first process previous Compression: memory!
    return false;

  copyRasterToImageBuffer(); // the source: whole rasterImage or part of it

  // check if imgBuffer exists
  if (!imgBuffer)
  {
    cerr << "\nerror in Compress: attempt to Compress from imgBuffer not"
             " yet available!.\n";
    return false;
  }

  store_info.err = jpeg_std_error(&j_store_error);
  jpeg_create_compress(&store_info);
  compressCreated = true; // later checked if memory has to be released

  jpeg_flob_dest(&store_info, flob); // connect to destination

  store_info.image_width = rasterImgWidth;
  store_info.image_height = rasterImgHeight;
  store_info.input_components = cinfo.num_components; // may be overwritten
  store_info.in_color_space = cinfo.out_color_space; // before jpeg_defaults()
  store_info.jpeg_color_space = cinfo.jpeg_color_space;

  return true;
}


// copyRasterToImageBuffer():
// Copy rasterImage to format expected from jpeg\_write\_scanlines()

bool JPEGSourceMgr::copyRasterToImageBuffer(int x, int y, int xdist, int ydist)
{
  // boundary checks allready done by caller
  // further checks:
  
  // cf. preconditions to compress: 1. part of libjpeg : example.c

  if (rasterImage == 0)
    return 0;

  if (imgBuffer != 0)  // first process previous Compression: memory!
    return 0;

  unsigned int rowSize = (unsigned int) xdist * rasterImgComps;
  unsigned long imgSize = (unsigned long) rowSize *
                          (unsigned long) ydist;

  imgBuffer = new JSAMPLE[imgSize];
  imgBufHeight = ydist;
  imgBufWidth = xdist * rasterImgComps;

  // to do: errorcheck!   or   use libjpeg.memorymanager with its error-checks

  // if (failure in new())
  //   return false;

  unsigned int hMax = (unsigned int) (y + ydist);
  unsigned int wMax = (unsigned int) (x + xdist);
  unsigned int hStart = (unsigned int) y ;
  unsigned int wStart = (unsigned int) x ;

  
  for (unsigned int r = hStart; r < hMax; r++)
    for (unsigned int w = wStart; w < wMax; w++)
      for (unsigned int c = 0; c < rasterImgComps; c++)
        imgBuffer[(r-hStart)*rowSize + (w-wStart) * rasterImgComps + c] = rasterImage[r][w][c];

  return true;
}

bool JPEGSourceMgr::copyRasterToImageBuffer()
{
  int x = 0, y = 0, xdist = rasterImgWidth, ydist = rasterImgHeight;
  return copyRasterToImageBuffer(x, y, xdist, ydist);
}

/**
2.6.2 Configuring 'store\_info':

Various configuration methods, configuring 'store\_info' as needed and for
usage with imgBuffer in [->]Compress. ~Before~, Compression must be prepared.

cf. middle of first part in libjpeg: example.c

*/

/**
ConfigureCut():

The configuring Method for JPEGs ~cut~-operator.

*/

bool JPEGSourceMgr::ConfigureCut(int x, int y, int xdist, int ydist)
{
  copyRasterToImageBuffer(x, y, xdist, ydist); // the source: whole rasterImage or part of it

  // check if imgBuffer exists
  if (!imgBuffer)
  {
    cerr << "\nerror in Compress: attempt to Compress from imgBuffer not"
             " yet available!.\n";
    return false;
  }

  store_info.image_width = xdist;
  store_info.image_height = ydist;
  store_info.input_components = cinfo.num_components;
  store_info.in_color_space = cinfo.out_color_space;
  store_info.jpeg_color_space = cinfo.jpeg_color_space;

  jpeg_set_defaults(&store_info);
  readyToCompress = true;

  store_info_valid = true; // optionally: extended check eventually setting
                           // it to false before executing Compress()
  
  return readyToCompress;
}


bool JPEGSourceMgr::ConfigureDownSize(int quality)   
{
  if (!compressCreated)
    return false;
  jpeg_set_defaults(&store_info);
  jpeg_set_quality(&store_info, quality, TRUE ); // baseline
  readyToCompress = true;

  store_info_valid = true; // optionally: extended check eventually setting

  return true;
}

bool JPEGSourceMgr::ConfigureConvertToColor(colorS cs) // not yet implemented
{
  return false;
}
  
/**
2.6.3 Compress()

Last and executing compression step. Must be prepared and configured before!

*/
bool JPEGSourceMgr::Compress()
{
  // prevent error-prone executions
  if (!readyToCompress || !store_info_valid || !compressCreated)
  {
    cerr << "\nCompress(): Attempt to Compress mal-configured data refused.\n";

    if (compressCreated)
      jpeg_destroy_compress(&store_info); // release storage again

    // reset:
    compressCreated = false;
    readyToCompress = false;
    store_info_valid = false;
    return false;
  }

  JSAMPROW row_pointer[1]; // improvement: several rows at a time ...
  int row_stride;

  jpeg_start_compress(&store_info, TRUE);
  row_stride = store_info.image_width * store_info.num_components;

  while (store_info.next_scanline < store_info.image_height) {
    row_pointer[0] = & imgBuffer[store_info.next_scanline * row_stride];
    // store_info works with FLOB => this writes directly into JPEG::picture :
    (void) jpeg_write_scanlines(&store_info, row_pointer, 1);
  }

  jpeg_finish_compress(&store_info);

  jpeg_destroy_compress(&store_info);

  // reset:
  store_info_valid = false;
  compressCreated  = false;
  readyToCompress = false;
  
  return true;
}
