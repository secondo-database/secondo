/*******************************************************************************
//paragraph [1] title: [{\Large \bf ]	[}]
//paragraph [2] subtitle: [{\bf ]	[}]
//[->] [$\rightarrow $]
//[=>] [$=> $]
//[TOC] [\tableofcontents]



[1] JPEGAlgebra (and JPEG): Implementation File

February 09, 2004  Neumann (corrections in object destruction)

February 06, 2004  Neumann (formatting, object tracing, c'str + d'str corrected)

February 02   2004   Neumann (correction: CastJINFO uses Std-c'str, formatting)

February 01   2004   Schoenhammer (jreadinfo overloaded to work with streams)

January 31    2004   Schoenhammer (working with streams in compbrightdist,
                                   comprgbdist)

January 26-27 2004   Schoenhammer (correct Methods compBright, compRGB)

January 20-23 2004   Schoenhammer (class JPEG: Methods compBright, compRGB,
                                   class JINFO, operator jreadinfo)

January 22, 2004 Schoenhammer (JINFO, jreadinfo)

January  14    2004  Schoenhammer (color-/brightness-distribution, testing),
                     Neumann (integration in JPEGSourceMgr, general revision)

January  08/09 2004  Neumann (JPEGSourceMgr: libjpeg w/o file; OO-Separation of
Interfaces)

        ( ... formerly including in JPEGAlgebra.h: )

December 31, 2003  Neumann (extensions, 'defined', some error-tests, 'alpha'--)

December 30, 2003  Schoenhammer (InJPEG-WorkAround)

December 29, 2003  Neumann (first revision)

prior to
December 24, 2003  Schoenhammer

February 27, 2004  M. Spiekermann. A problem with the tiles operator was fixed. However,
                     other problems still remain.  

*******************************************************************************/
/*******************************************************************************
\rule {100 pt}{1 pt} \linebreak

*******************************************************************************/


#include "JPEGAlgebra.h"

// runtime switches for trace messages to be configured in SecondoConfig.ini
#include "LogMsg.h"
static const string JPEG_RT_DEBUG = "JPEGAlgebra:Debug";

extern NestedList* nl;
extern QueryProcessor *qp;

bool jpegAlgGlobalError; // Global flag: any error occurred in JPEGAlgebra?

/*******************************************************************************
2 Type Constructor ~jpeg~

*******************************************************************************/

/******************************************************************************
2.1 Class ~JPEG~  Implementation

******************************************************************************/

// Four class attributes and methods
int JPEG::numJpegsAlive = 0; // (initialization static/class variable)
int JPEG::jpegCounter   = 0; // number of JPEGs c'str'd inJPEGAlgebra lifetime
int JPEG::castJPEGs     = 0; // that is: memory is managed by Secondo System
int JPEG::clonedJPEGs   = 0;
int JPEG::createdJPEGs  = 0;
int JPEG::closedJPEGs   = 0;
int JPEG::deletedJPEGs  = 0;

/** number of Jpegs currently existing, w/o casts ( managed by Secondo ) */
int JPEG::GetNumAlive(){
  return numJpegsAlive;
}


/**
2.1.1 Constructors (c'str) and Destructors (d'str)

*/

JPEG::JPEG( const int size ) : // (initialiation list: declaration order!)
src(0), // needs the FLOB
defined(false),  // no valid picture yet
picture( size ), // success => memory/FLOB reserved, header not yet set.
canDelete( false ),
height(HEIGHT_INVALID),
width(WIDTH_INVALID),
coloured(false),
compLevel(COMPLEVEL_INVALID),
comment(COMMENT_INVALID),
destructing(false),
colorSpace(COLORSPACE_INVALID),
numComponents(NUMCOMPS_INVALID),
brightnessDcomputed(false),
rgbDcomputed(false),
cmykDcomputed(false),
srcConnected(false)
{
  // defined remains false since attributes not yet initialized by SrcMgr
  ++numJpegsAlive;
  ++jpegCounter;

  #ifdef DEBUGJPEG
  cout << "\nGeneral Constructor JPEG(int). alive (w/o casts): "
       << numJpegsAlive
       << "   c'str-calls: " << jpegCounter <<  "   including casts: "
       << castJPEGs << endl;
  #endif
  
  InitializeBrightnessDistribution(); // initialize Array []   with 0.0;
  InitializeRGBDistribution();	      // initialize Array [][] with 0.0;
  InitializeCMYKDistribution();       // initialize Array [][] with 0.0;
}


/**
InitializeJPEG() finishes the work of the c'str

*/
bool JPEG::InitializeJPEG()
{
  // Precondition: decoded FLOB 'picture' present

  // Construct the source manager, which will do all the work related
  // to libjpeg  (JPEG can concentrate on operator-/appl.- logic):

//  assert(!src);
  src = new JPEGSourceMgr(this); // sets connection to JPEG.
  srcConnected = (src != 0);

  #ifdef DEBUGJPEG
  cout << "\nJPEG::InitializeJPEG() after JPEGSourceMgr(this)" << endl;
  #endif

  // Two steps in construction required. The second one is init(), which :
  // (a) scans FLOB.
  // (b) sets the most important and often used members of JPEG. The other
  // ones are set by JPEGSourceMgr if needed. This greatly improves average
  // performance since no computations are done which are not needed.

  return src->init(); // used to report success to Secondo System
}

void JPEG::InitializeBrightnessDistribution ( )
{
  for (int i = 0; i <=255; i++)
     brightnessDistribution [i] = 0.0;
}

void JPEG::InitializeRGBDistribution ( )
{
  for (int i = 0; i <= 255; i++)
    for (int j = 0; j <=2; j++)
     rgbDistribution [i][j] = 0.0;
}

void JPEG::InitializeCMYKDistribution ( )
{
  for (int i = 0; i <= 255; i++)
    for (int j = 0; j <=3; j++)
     cmykDistribution [i][j] = 0.0;
}

// Destructor of JPEG: deletes its SourceManager after disconnecting from
// it, decrements count of Jpegs alive in the JPEGAlgebra-instance.
JPEG::~JPEG()
{
  if( canDelete )
    picture.Destroy();

  // Destruction mechanism may be changed IF possible extension to JPEG inde-
  // pendent source objects implemented: then a static SourceManager with
  // lifetime of JPEGAlgebra would be notified of JPEG-destruction and could
  // decide if also the src-Object is to be destroyed -- depending on a flag
  // 'isPermanent' set for this object ... As stated, this is currently NOT
  // implemented.
  if(srcConnected)
  {
    DisconnectFromSrc();
    delete src;
  }

  --numJpegsAlive;
  #ifdef DEBUGJPEG
  cout << "Destructor ~JPEG(). w/o casts alive: "
       << numJpegsAlive << "   c'str-calls: "
       << jpegCounter   << endl;
  #endif

  // defined destroyed with *this
}

void JPEG::Destroy()
{
  canDelete = true;
  SetDefined(false);
}


/**
2.1.2 'Ordinary' JPEG - member functions (methods) ...

  * supporting usage as Relation-Attribute ( IsDefined() etc. )

  * Flob-Management ( Encode() etc. using Base64 )

  * (diverse) Support to Operator- and Type Constructor- Functions

  * (Standard-) Member-Access (Get.. and Set.. ...)

*/


/**
[2] Relation-Attribute methods:  need to be revised, completed, made consistent

*/


/*
IsDefined() / SetDefined():
The goal of tracing defined-status is (besides 'Relation-Attribute-Abilities')
also to prevent system crashes if the user works with declared or constructed,
but not initialized objects:  'create' without 'update', for example.

*/
bool JPEG::IsDefined() const
{
  // return true;     // <- switch off defined-tracing
  return defined;
}

void JPEG::SetDefined( bool Defined)
{
  defined = Defined; 
  // defined = true; // <- switch off defined-tracing (caveat: constructor!)
}

size_t JPEG::HashValue()
{
  return height*5+width;
}

void JPEG::CopyFrom(StandardAttribute* right)
{
  bool no_error = true;

  JPEG *r = (JPEG *)right;

  if (!r)
  {
    cerr << "\nerror: CopyFrom() got 0x00 instead of jpeg!\n";
    no_error = false;
  }
    
  if (no_error &&
      r->IsDefined())
  {
    #ifdef DEBUGJPEG
    cout << "\n++ CopyFrom ++ (defined is true)\n";
    #endif

    // Copy the FLOB. (r _may_ also _not_ be connected to a SrcMgr).
    int copysize = r->picture.Size();
    picture.Resize( copysize );

    char *bin = (char *)malloc( copysize );
    if (!bin)
    { no_error = false;
      defined = no_error;
      cerr << "\nCopyFrom(): aborted because of memory leak...\n";
      jpegAlgGlobalError = true;
      return;
    }
    
    r->picture.Get( 0, copysize, bin );
    picture.Put( 0, copysize, bin );

    if (!src)
      src = new JPEGSourceMgr(this);

    // Copying members == scanning and evaluating the flob of ~same~ content
    if (!src)
      no_error = false;
    else
    {
      srcConnected = true;
      if (!src->init())
      {
        no_error = false;
        cerr << "\nCopyFrom(): JPEG initialization failed.\n";
      }
    }

    // Due to SecondoSystem object-management mechanisms which partly do _not_
    // call Destructors, there is no other possibility for releasing the Src-
    // Mgr _in_CopyFrom_ than doing it allready here ... if we want correct
    // object-/memory-management!
    // ... So, _if_ SrcMgr will be needed again for _extended_ features:
    // Re-InitializeJPEG(). Reduce this need by Computing distribution-array now:

    if (no_error)
    {
      switch (colorSpace)
      {
        case grayscale : src->ComputeBrightnessD();
                         break;
        case rgb       : src->ComputeRGBD();
                         break;
        case cmyk      : src->ComputeCMYKD();
                         break;
        default        : cerr << "\nunknown colorSpace."
                                 " couldn't create distribution.";
                         no_error = false;
      }
    }

    if (src)
      DropSrcMgr(); // Not the best way it vould work. see comment above!

    #ifdef DEBUGJPEG
    cout << "CopyFrom(): after src->init()\n";
    #endif
        
    free( bin );
    defined = no_error;
  }
  else
  {
    // defined: even if JPEG WAS defined before: it's NOT defined AS A COPY OF
    // 'right' now... semantical state not valid even if data valid
    
    cerr << "\n!!! CopyFrom() didn't try to copy from undefined JPEG !!!\n";
    no_error = false;
    defined = false;
  }
  
  if (!no_error)
    jpegAlgGlobalError = true;
}

int JPEG::Compare(Attribute * arg)
{
  // (perhaps a comparison of imgage creators comment would be an alternative)

  if(!arg)
  {
    cerr << "\nerror: JPEG::Compare() received 0x00 instead of Attribute *\n";
    jpegAlgGlobalError = true;
    return -2; // indicate error
  }

  JPEG *a = (JPEG *)arg;

  if (!IsDefined())
    return (a->IsDefined()) ? -1:0;  // undefined => worse, if other defined
  if (!a->IsDefined())
    return 1;                        // allways better if other not defined

  if ( (a->comment == comment) &&
       (a->height == height) &&
       (a->width == width) )
       return 0; // assume equal (without considering image content)
  if ( comment != a->comment )
    return (comment > a->comment) ? 1:-1;  // this bigger : this smaller
  if ( height != a->height )
    return (height > a->height) ? 1:-1;
  if ( width != a->width )
    return (width > a->width) ? 1:-1;

  jpegAlgGlobalError = true;
  return -2; // never reached
}

bool JPEG::Adjacent(Attribute * arg)
{
  if(!arg)
  {
    cerr << "\nerror: JPEG::Adjacent() received 0x00 instead of Attribute *\n";
    jpegAlgGlobalError = true;
    return false;
  }

  JPEG *a = (JPEG *)arg;

  if (!IsDefined() || !a->IsDefined())
    return false; // (adjacency is a constraint, un-definedness the opposite)

  if ( (a->comment == comment) && // taken as semantic: 'same sense'
        comment != "" &&
        Compare(a) != 0 )
    return true; // semantically same, technically modified

  return false; // exactly same or unequal in all considered aspects
}

JPEG* JPEG::Clone()
{
  JPEG *newJPEG = new JPEG( 0 );
  newJPEG->CopyFrom( (JPEG*)this ); // also connects to a JPEGSourceMgr
  return newJPEG;
}

ostream& JPEG::Print( ostream &os )
{
  if(IsDefined())
  {
    // print information useful for debugging
    return
           (os
            << "JPEG-Picture-Object with SrcMgr-ID-Nr. "   << src->GetID()
            << "  Existing JPEGs: " <<  numJpegsAlive        << "  from "
            << "overall " << jpegCounter << " c'str'd JPEGs."         
            << "\nCurrently alive SourceMgr(s): "<< src->GetNumAlive()
            << "\n\nPicture - Info:\n"
            << "created with Adobe? : "
            <<  ((Get_cinfo()->saw_Adobe_marker == 1) ? "yes":"no")
            << "\nheight= "                      << height        << endl
            << "width= "                         << width         << endl
            << "coloured= "                      << ((coloured) ? "yes":"no")
            << endl
            );
  }
  else
    return ( os << " existing but undefined JPEG-Picture-Object" << endl);
}

// overload iostream output-operator ( could access members, since a friend )
ostream& operator<<( ostream& o, JPEG& jp )
{
   return jp.Print(o);
}

int JPEG::NumOfFLOBs()
{
  return 1;
}

FLOB *JPEG::GetFLOB(const int i)
{
  assert( i >= 0 && i < NumOfFLOBs() );
  return &picture;
}



/**
[2] Flob-Management methods

*/


/*
Encode / Decode is the task of JPEG, not of the Source-Manager, since it is
related to the Secondo System Architecture. The same applies to Decode and
other FLOB management done inside JPEGAlgebra.

*/
void JPEG::Encode( string& textBytes )
{
  if (IsDefined()) // ... then also picture should be valid
  {
    Base64 b;
    char *bytes = (char *)malloc( picture.Size() );
    picture.Get( 0, picture.Size(), bytes );
    b.encode( bytes, picture.Size(), textBytes );
  }
  else
    textBytes = "undefined JPEG!";
}

void JPEG::Decode( string& textBytes )
{
  Base64 b;
  int sizeDecoded = b.sizeDecoded( textBytes.size() );
  char *bytes = (char *)malloc( sizeDecoded );
  int result = b.decode( textBytes, bytes );

  assert( result <= sizeDecoded );

  picture.Resize( result );
  picture.Put( 0, result, bytes );

  free( bytes );
}



/**
[2] Other methods supporting Operator- and Type Constructor- Functions

*/



bool JPEG::SaveToFile( char *fileName )
{
  if (!IsDefined())
  {
      if (strlen(fileName) >= strlen("undefined.dont_save_me"))
        strcpy(fileName, "undefined.dont_save_me");
      else
        strcpy( fileName, "");
        
      jpegAlgGlobalError = true;
      return false;
  }

  FILE *f = fopen( fileName, "wb" );

  if( f == NULL )
  {
    jpegAlgGlobalError = true;
    return false;
  }

  char *bytes;  // memory reserved by GetAllPictureBytes(), if successful
  if (!GetAllPictureBytes(bytes))
  {
    fclose(f);
    cerr << "SaveToFile: Couldn't get picture-Bytes. canceled.\n";
    jpegAlgGlobalError = true;
    return false;
  }

  if( (int)fwrite( bytes, 1, picture.Size(), f ) != picture.Size() )
  {
    fclose (f); 
    free(bytes);
    jpegAlgGlobalError = true;
    return false;
  }

  fclose( f );
  free(bytes);
  return true;
}

bool JPEG::Show(){ // OS-, even Installation dependent: calls ImageMagic
  if (!defined)
  {
    jpegAlgGlobalError = true;
    return false;
  }  
  bool success = false;

  if (picture.Size() > 0)
    success = SaveToFile("tmpJPEGAlgebra000.jpeg");
  else
    defined = false;
    
  if (success)
  {
    success = (-1 != system("display tmpJPEGAlgebra000.jpeg"));
    if (success)
    {
      success = (-1 != system("rm tmpJPEGAlgebra000.jpeg"));
      if (!success)
        cerr << "Temporary file couldn't be removed.\n";
    }
    else
      cerr << "X11-programm display could not be loaded.\n";
  }
  else
    cerr << "Temporary file couldn't be created. is there write permission to"\
            " directory?.\n";

  if (!success)
    jpegAlgGlobalError = true;
            
  return success;
}

// Demo: usage of JPEGSourceMgr. For Development and Debugging.
bool JPEG::Demo(){
  if (!defined)
  {
    jpegAlgGlobalError = true;
    return false;
  }
  
  bool success = false;
  if (!srcConnected)
    InitializeJPEG();
  success = src->ParseComment();

  if (success)
    cout << "\nDemo:\nPicture a has comment: \"" <<  comment
         << "\"\nwhich has been saved in member var by JPEGSourceMgr.\n";
  else
    cout << "\nDemo:\nNo comment was found in picture. Nothing else done. "\
              "value-map-fn gets false.\n";

  if (src->hasAdobeMarker())
    cout << "\nDemo:\nParsed picture is created with Adobe.\n";
  else
    if (Get_cinfo()->saw_JFIF_marker)
    cout << "\nDemo:\nParsed picture is a JFIF-compatible. ColorSpace is "
         << GetColorSpace("");
    else
        cout << "\nDemo: parsed picture is of valid but unknown Format. "
             << "ColorSpace is "     << GetColorSpace("");
  cout << endl
       << *this << endl;
       
  if (!success)
    jpegAlgGlobalError = true;

  return success;
}


bool JPEG::ColDTest(){  // for testing ColorDistribution-Algor. only 
  if (!defined)
  {
    cerr << "\nColDTest: JPEG is not defined!\n";
    jpegAlgGlobalError = true;
    return false;
  }

  int cs_val = (int)GetColorSpace();

  if (cs_val == 3)
  {
    cerr << "\nColDTest: ungueltiger colorspace.\n";
    jpegAlgGlobalError = true;
    return false;
  }

  double val = 0.0;

  if (!cs_val)  // brightness
  {
    cout << "\n\nBrightnessdistributionstest:\n\n";
    for (int r = 0; r <280; r += 20) // 14 lines
    {
       for (int i = 0; i < 128; i++) // somewhat un-exact
         cout << ((GetBrightnessValue(i) > ((double) (256 - r))/256.0) ?
                 '#':'.');
      cout << endl;
    }

  }
  else // RGB, CMYK  ... for simplicity only first 3 comps
  {
    cout << "\n\n" << ((cs_val == rgb) ? "RGB":"CMYK")
         <<"distributionstest:\n\n";
    for (int r = 0; r <280; r += 20) // 14 lines
    {
       for (int i = 0; i < 256/2; i++) 
       {  val = (cs_val == rgb) ? (GetRGBValue(0,i) +GetRGBValue(0,i+1))/2.0
                              : (GetCMYKValue(0,i)+GetCMYKValue(0,i+1))/2.0;
          cout << ( (val > ((double) (256 - r))/256.0) ? 'r':'.' );
       }
      cout << endl;
    }
    for (int r = 0; r <280; r += 20) // 14 lines
    {
       for (int i = 0; i < 256/2; i++) 
       {  val = (cs_val == rgb) ? (GetRGBValue(1,i)+GetRGBValue(1,i+1))/2.0
                              : (GetCMYKValue(1,i)+GetCMYKValue(1,i+1))/2.0;
          cout << ((val > ((double) (256 - r))/256.0) ? 'g':'.');
       }
      cout << endl;
    }
    for (int r = 0; r <280; r += 20) // 14 lines
    {
       for (int i = 0; i < 256/2; i++) 
       {  val = (cs_val == rgb) ? (GetRGBValue(2,i)+GetRGBValue(2,i+1))/2.0
                              : (GetCMYKValue(2,i)+GetCMYKValue(2,i+1))/2.0;
          cout << ((val > ((double) (256 - r))/256.0) ? 'b':'.');
       }
      cout << endl;
    }
  }
  return true;
}

/**
Cut():

Support for ValueMapping-Fn() of operator ~cut~

*/
bool JPEG::Cut(JPEG * outjpeg, int x, int y, int xdist, int ydist)
{
// outjpeg passed by pointer since actually only flob needed

  bool success = true;
  bool initialized = false;

  if(!src)
  {
    success = InitializeJPEG();
    initialized = success;
  }

  if (success)
    success = src->PrepareCompress(outjpeg->GetFLOB(0));
  if (success)
    success = src->ConfigureCut(x, y, xdist, ydist);
  if (success)
    success = src->Compress();

  outjpeg->SetDefined(success);

  if (!success)
    jpegAlgGlobalError = true;

  if (initialized) // ... initialized here ? so SrcMgr probably not needed
    DropSrcMgr();    
  
  return success;
}

/**
Tile():

Support for ValueMapping-Fn() of operator ~tiles~

*/
bool JPEG::Tile(JPEG * outjpeg, int x, int y, int xdist, int ydist)
{
// outjpeg passed by pointer since actually only flob needed

  bool success = true;
  bool initialized = false;

  if(!src)
  {
    success = InitializeJPEG();
    initialized = success;
    cerr << "InitializeJPEG() called" << endl;
  }

  if (success){
    success = src->PrepareCompress(outjpeg->GetFLOB(0));
    if (!success)
      cerr << "PrepareCompress failed" << endl;
  }
  if (success) {
    success = src->ConfigureCut(x, y, xdist, ydist);
    if (!success)
      cerr << "ConfigureCut failed" << endl;
  }
  if (success) {
    success = src->Compress();
    if (!success)
      cerr << "Compress failed" << endl;
  }

  outjpeg->SetDefined(success);

  if (!success)
    jpegAlgGlobalError = true;

  if (initialized) { // ... initialized here ? so SrcMgr probably not needed
    DropSrcMgr();
    cerr << "DropSrcMgr() called" << endl;
  }

  return success;
}



/**
DownSize():

Support for ValueMapping-Fn() of operator ~downsize~

*/
bool JPEG::DownSize(JPEG * outjpeg, int quality)
{
// outjpeg passed by pointer since actually only flob needed

  bool success = true;
  bool initialized = false;

  if (quality > 100 || quality < 0)
  {
    quality = (quality > 100) ? 100:0;
    cout << "\nQuality Range must be 0 to 100 (meaning per cent), "
               "corrected to " << quality << endl;
  }

  if(!src)
  {
    success = InitializeJPEG();
    initialized = success;
  }

  if (success)
    success = src->PrepareCompressDefault(outjpeg->GetFLOB(0));
  if (success)
    success = src->ConfigureDownSize(quality);
  if (success)
    success = src->Compress();

  outjpeg->SetDefined(success);

  if (!success)
    jpegAlgGlobalError = true;

  if (initialized) // ... initialized here ? so SrcMgr probably not needed
    DropSrcMgr();

  return success;
}




/**
[2] (Standard-)Member-Access methods (Get\_\_ / Is\_\_ and Set\_\_)

*/


/**
Get\_\_ / Is\_\_:

*/

bool JPEG::IsColoured(){
  // special case if image not yet scanned, returns false. No 'INVALID_VALUE'.
  // test: numComponents != NUMCOMPS_INVALID
  return coloured;
}

string JPEG::GetComment(){
  // The comment of the Jpeg-Picture-File is not parsed by default.
  // Let JPEGSourceMgr parse the comment in the Jpeg-file, if not yet done.
  // This must be done only once and is done only if requested:

  if (!src->comment_parsed)
    assert(src->ParseComment()); // currently only partially implemented

  // if successful scr has set the JPEG::comment.
  
  return comment;
}

int JPEG::GetCompLevel(){
  cerr << "\ncompression Level set, get, change not yet implemented.\n";
  return compLevel;
}

size_t JPEG::GetHeight(){
  return height;
}

int JPEG::GetWidth(){
  return width;
}

// (GetAllPictureBytes also reserves memory for buffer!)
// (caller is responsible to free(buffer) again!)
bool JPEG::GetAllPictureBytes(char *& buffer){

  size_t noBytes = picture.Size();
  buffer = (char *) malloc( noBytes );
  if (buffer == NULL)
  {
    cerr << "GetAllPictureBytes: not enough Memory.";
    jpegAlgGlobalError = true;
    return false;
  }
  picture.Get(0, noBytes, buffer);

  return true;
}

// GetColorSpace(): this method is overloaded (for comfortability).
// JPEGSourceMgr 'src' sets JPEG::colorspace in its init()-method.

colorS JPEG::GetColorSpace()
{
  return colorSpace;
}
int JPEG::GetColorSpace(int dummy_int)
{
  return (colorSpace == grayscale) ? 0 :
         (colorSpace == rgb)       ? 1 :
         (colorSpace == cmyk)      ? 2 :
                                     3; // 3 == undefined. could also use -1
                                        // ! no assertion done !
}

string JPEG::GetColorSpace(string dummy_string)
{
  return (colorSpace == grayscale) ? "Grayscale" :
         (colorSpace == rgb)       ? "RGB" :
         (colorSpace == cmyk)      ? "CMYK" :
                                     "not supported ColorSpace";
                                     // 3 == undefined. could also use -1
}

int JPEG::Get_num_components()
{                             
  return numComponents;
}

double JPEG::GetBrightnessValue (int brightnessValue, int dummy_not_used)
{
  // Distribution allready computed?
  if (! brightnessDcomputed)
    brightnessDcomputed = src->ComputeBrightnessD();

  // check if no error
  assert(brightnessDcomputed);
  
  assert ( brightnessValue >= 0 && brightnessValue <=255); 
  return brightnessDistribution [brightnessValue];
}

double JPEG::GetRGBValue ( int colorIndex, int colorNumber)
{
  //cout << "GetRGBValue: Distribution already computed?" << rgbDcomputed << endl;
  if (! rgbDcomputed)
    rgbDcomputed = src->ComputeRGBD();

  assert(rgbDcomputed);

  assert (colorIndex >= 0 && colorIndex <=2);
  assert ( colorNumber >= 0 && colorNumber <=255);
  return rgbDistribution [colorNumber][colorIndex];
}

double JPEG::GetCMYKValue ( int colorIndex, int colorNumber)
{
  // Distribution allready computed?
  if (! cmykDcomputed)
    cmykDcomputed = src->ComputeCMYKD();

  // check if no error
  assert(cmykDcomputed);

  assert (colorIndex >= 0 && colorIndex <=3);
  assert ( colorNumber >= 0 && colorNumber <=255);
  return cmykDistribution [colorNumber][colorIndex];
}

bool JPEG::CompBright (JPEG* injpeg, double dist)
/*
Compares the difference of every BrightnessValue (0..255) of two JPEG's.
If the difference for every value is smaller then dist, true is returned.

*/

{
  JPEG* jin =(JPEG*)(injpeg);
  bool similar = true;
  int idx = 0;

  while (similar && (idx <= 255) )
  {
    similar = ( fabs(this->GetBrightnessValue (idx)  -
                  jin->GetBrightnessValue(idx) ) <= dist);
    idx++;
  }

  return similar;
}

bool JPEG::CompRgb (JPEG* injpeg, double rdist, double gdist, double bdist)
/*
Like CompBright, but for all colors RGB

*/
{
  if (!injpeg)
  {
    cerr << "\nerror: CompRgb() received 0x00 instead of jpeg.\n";
    jpegAlgGlobalError = true;
    return false;
  }

  JPEG* jin =(JPEG*)(injpeg);
  bool rSimilar = true;
  bool gSimilar = true;
  bool bSimilar = true;
  int idx = 0;

  //cout << "CompRGB: Compare the Red-Values" << endl;
  while (rSimilar && (idx <= 255))
  {
    rSimilar = ( fabs(this->GetRGBValue (0, idx) -
                  jin->GetRGBValue (0, idx) ) <= rdist);
    //cout <<"RED:"<<  rSimilar << ": idx=" << idx      << " : this="
    //     << this->GetRGBValue (0, idx)    <<" : jin="
    //     << jin->GetRGBValue (0, idx)     << endl;
    idx++;
  }

  //cout << "CompRGB: Compare the Green-Values" << endl;
  idx = 0;
  while (rSimilar && gSimilar && (idx <= 255))
  {
    gSimilar = ( fabs(this->GetRGBValue (1, idx) -
                  jin->GetRGBValue (1, idx) ) <= gdist);
    //cout <<"Green:" << gSimilar << ": idx=" << idx << " : this="
    //     << this->GetRGBValue (0, idx)      << " : jin="
    //     << jin->GetRGBValue (0, idx)       << endl;
    idx++;
  }

  //cout << "CompRGB: Compare the Blue-Values" << endl;
  idx = 0;
  while (rSimilar && gSimilar && bSimilar && (idx <= 255))
  {
    bSimilar = ( fabs(this->GetRGBValue (2, idx) -
                  jin->GetRGBValue (2, idx) ) <= bdist);
    //cout << "Blue:" << bSimilar << ": idx=" << idx << " : this="
    //     << this->GetRGBValue (0, idx) << " : jin="
    //     << jin->GetRGBValue (0, idx)  << endl;
    idx++;
  }

  return (rSimilar && bSimilar && gSimilar);
}

bool JPEG::CompRgb (JPEG* injpeg, double dist)
{
  return (CompRgb (injpeg, dist, dist, dist));
}


bool JPEG::CInfoValid(){
  if (! srcConnected)
    return false;
  if (src == 0)
  {
    cerr << "\nerror in management of srcConnected.\n";
    jpegAlgGlobalError = true;
    return false;
  }
  return src->CInfoValid();
}

// Get\_cinfo():
// Not recommended! C-style direct manipulation of JPEGSourceMgr::cinfo
// Warning: This evades consistency checks!

jpeg_decompress_struct* JPEG::Get_cinfo()
{
  if (!src)
  {
    cerr << "\nerror: JPEG::Get_cinfo() called, but no SourceMgr connected. "
         << "0 will be returned.\n";
    jpegAlgGlobalError = true;
    return 0;
  }
  return src->Get_cinfo();
}



/**
...: Set\_\_
attribute member-setting is done during constrution time or directly by friend
class JPEGSourceMgr. Set-methods who modify consistently member attribute AND
picture are currently not implemented.

*/



// DisconnectFromSource():
// Securely(?) reset the refencePointer in src (friend-access with check)

void JPEG::DisconnectFromSrc(){
  if (!src)
  {
    cerr << "\nerror: DisconnectFromSrc(), but there is no SrcMgr!\n";
    srcConnected = false;
    jpegAlgGlobalError = true;
    return;
  }
  if (!srcConnected)
  {  cerr << "\nforgot to update 'srcConnected' somewhere in the code?\n";
     jpegAlgGlobalError = true;
  }

  destructing = true;
  // src->Disconnect(this);  This way the src-manager can be notified (only by
  // *this) that its client (*this) is gone or not yet correctly initialized.

  src->DisconnectClient(this);  // also sets src = 0
  srcConnected = false;
}


/** Discard JPEGSourceMgr */
void JPEG::DropSrcMgr(){
    if (!srcConnected)
    {
        cerr << "\nforgot to update 'srcConnected' somewhere in the code?\n";
        jpegAlgGlobalError = true;
    }
    else
      if (!src)
      {
        cerr << "\nforgot to update 'srcConnected' after deleting src?\n";
        jpegAlgGlobalError = true;
      }
      
    if (src) {
      DisconnectFromSrc();  // (sets also srcConnected to false)
      delete src;
      src = 0;
    }
}



/*******************************************************************************
\pagebreak \rule {460 pt}{1 pt} \linebreak

*******************************************************************************/
/**
[2] S e c o n d o - S u p p o r t - F u n c t i o n s,\space\space O p e r a t
o r s\space\space   a n d\space\space   T y p e   C o n s t r u c t o r s
\linebreak f o r \space\space J P E G

*******************************************************************************/

/**
2.2 List Representation

The list representations of a ~binfile~ are

----        ( <file>picturefilename</file---> )
----

and

----        ( <text>filename</text---> )
----

If first representation is used then the entire contents of a picturefile
including JPEGHeader is read into the second representation
This is done automatically by the Secondo parser

*/

/**
2.3 ~Out~-Function

*/

ListExpr
OutJPEG( ListExpr typeInfo, Word value )
{
  ListExpr result = nl->TextAtom();

  JPEG *jpegFile = (JPEG *)value.addr;

  if (!jpegFile)
  {
    cerr << "\nerror: OutJPEG received 0x00 instead of jpeg-address.\n";
    jpegAlgGlobalError = true;
  }  
  else
    if (jpegFile->IsDefined())
    {
      string encoded;
      jpegFile->Encode( encoded );

      nl->AppendText( result, encoded );
    }
    else
    {
      cerr << "\nerror: OutJPEG() will not put out undefined jpeg.\n";
      nl->AppendText( result, "typeerror" );
      jpegAlgGlobalError = true;
    }

  return result;
}

/**
2.4 ~In~-Function

*/
Word
InJPEG( const ListExpr typeInfo, const ListExpr instance,
           const int errorPos, ListExpr& errorInfo, bool& correct )
{
  JPEG *jpegFile = new JPEG( 0 ); // not yet 'defined' nor connected to src 
                                   
  if( !jpegFile)
  {
    cerr << "\nInJPEG() failed in constructing a new JPEG.\n";
    jpegAlgGlobalError = true;
  }

  else if ( nl->IsAtom( instance ) && nl->AtomType( instance ) == TextType )
  {
    // Decode 'picture'(-member-att)

    string encoded;
    nl->Text2String( instance, encoded );
    jpegFile->Decode( encoded );  // needs not be 'defined'

    // now FLOB-member picture of JPEG is read in ... needs to be parsed ...

    // JPEG::InitializeJPEG() finishes the work of the cstr, setting up
    // the associated JPEG-Source-Manager, who parses the decoded
    // 'picture' and sets together with JPEG their member-vars:

    correct = jpegFile->InitializeJPEG();
    // 'defined' == correct. This is set in InitializeJPEG() or earlier.

    cout << "\nParsing of Picture, result:\n"                     << *jpegFile
         << "Colorspace:"       << jpegFile->GetColorSpace("")      << endl
         << "Color Components:" << jpegFile->Get_cinfo()->num_components
         << endl << endl;

    if (correct)
      return SetWord( jpegFile );

    // not correct => release ScrMgr. 'defined' remaines false
    if (jpegFile)
      jpegFile->DropSrcMgr();
  }

  jpegAlgGlobalError = true;
  correct = false;              // notify QP ...
  return SetWord( Address(0) ); 
}


/*
2.5 The ~Property~-function

*/
ListExpr
JPEGProperty()
{
  return (nl->TwoElemList(
          nl->FiveElemList(nl->StringAtom("Signature"),
                            nl->StringAtom("Example Type List"),
                            nl->StringAtom("List Rep"),
                            nl->StringAtom("Example List"),
                            nl->StringAtom("Remarks")),
          nl->FiveElemList(nl->StringAtom("-> DATA"),
                            nl->StringAtom("jpeg"),
                            nl->StringAtom("( <file>filename</file---> )"),
                            nl->StringAtom("( <file>Document.jpg</file---> )"),
                            nl->StringAtom(""))));
}

/*
2.6 ~Create~-function

*/
Word
CreateJPEG( const ListExpr typeInfo )
{
  #ifdef DEBUGJPEG
  cout << "\n++ CreateJPEG() ++";
  #endif

  JPEG::createdJPEGs++;
  JPEG::numJpegsAlive--; // correct this number in advance since the Secondo
                         // System manages destruction w/o calling d'str

  return SetWord( new JPEG( 0 ) ); // not defined yet: no data nor header
}

/*
2.7 ~Delete~-function

*/
void
DeleteJPEG( Word& w )
{
  #ifdef DEBUGJPEG
  cout << "\n-- DeleteJPEG() --";
  #endif

  JPEG *jpegFile = (JPEG *)w.addr;

  if (!jpegFile)
  {
    cerr << "\nerror: DeleteJPEG() received 0x00 instead of jpeg-address.\n";
    jpegAlgGlobalError = true;
    return;
  }
  
  jpegFile->Destroy(); 
  delete jpegFile;
  JPEG::deletedJPEGs++;
  w.addr = 0;
}

/*
2.8 ~Close~-function

*/
void
CloseJPEG( Word& w )
{
  #ifdef DEBUGJPEG
  cout << "\n-- CloseJPEG() --";
  #endif
 
  if (!(w.addr))
  {
    cerr << "\nerror: CloseJPEG() received 0x00 instead of jpeg-address.\n";
    jpegAlgGlobalError = true;
    return;
  }

  delete (JPEG *)w.addr;
  JPEG::closedJPEGs++;
  w.addr = 0;
}

/*
2.9 ~Clone~-function

*/
Word
CloneJPEG( const Word& w )
{
  #ifdef DEBUGJPEG
  cout << "\n++ CloneJPEG() ++";
  #endif

  if (!(w.addr))
  {
    cerr << "\nerror: CloneJPEG() received 0x00 instead of jpeg-address.\n";
    jpegAlgGlobalError = true;
    return SetWord(0);
  }
  
  JPEG::clonedJPEGs++;
  // 'w' will set 'defined' appropriately
  return SetWord( ((JPEG *)w.addr)->Clone() );
}

/*
2.10 ~SizeOf~-function

*/
int
SizeOfJPEG()
{
  return sizeof(JPEG);
}

/*
2.11 ~Cast~-function

*/
void* CastJPEG( void* addr )
                            
{
  #ifdef DEBUGJPEG
  cout << "\n+- CastJPEG() +-";
  #endif

  if (!addr)
  {
    cerr << "\nerror: CastJPEG() received 0x00 instead of jpeg-address.\n";
    jpegAlgGlobalError = true;
    return 0;
  }

  JPEG::castJPEGs++;
  // Std-Cstr will inc numJPEGsAlive, but doesn't know that this is a cast =>
  JPEG::numJpegsAlive--;
  return new (addr) JPEG;  
}

/*
2.12 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~jpeg~ does not have arguments, this is trivial.

*/
bool
CheckJPEG( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "jpeg" ));
}

/*
2.13 Creation of the Type Constructor Instance

*/
TypeConstructor jpeg(
        "jpeg",                     //name
        JPEGProperty,               //property function describing signature
        OutJPEG,      InJPEG,       //Out and In functions
        0,            0,            //SaveToList and RestoreFromList functions
        CreateJPEG,   DeleteJPEG,   //object creation and deletion
        0,            0,            //object open and save
        CloseJPEG,    CloneJPEG,    //object close and clone
        CastJPEG,                   //cast function
        SizeOfJPEG,                 //sizeof function
        CheckJPEG,                  //kind checking function
        0,                          //predefined persistence function for model
        TypeConstructor::DummyInModel,
        TypeConstructor::DummyOutModel,
        TypeConstructor::DummyValueToModel,
        TypeConstructor::DummyValueListToModel );


/**
\pagebreak

*/
/*******************************************************************************
3 Non-Lossy Types for Intermediate Representation of Jpeg-Data:

        not yet implemented ... TIFF version 6 fix 2 e.g., cf. libjpeg-doku

        many operator-applications of lossy transformations
    increasingly would worsen the Jpeg-Image [=>] intermediate
    non-lossy representation required.

*******************************************************************************/



/**
\pagebreak

*/
/*******************************************************************************
4 Operators (currently Jpeg-related only)

*******************************************************************************/

/*
4.1 Operator ~savejpegto~

Saves the binary contents of into a file.

4.1.1 Type mapping function of operator ~savejpegto~

Operator ~savejpegto~ accepts a binary file object and a string representing
the name of the file, and returns a boolean meaning success or not.

----    (binfile string)               -> bool
----

*/
ListExpr
SaveJpegToTypeMap( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength(args) == 2 )
  {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    if ( nl->IsEqual(arg1, "jpeg") && nl->IsEqual(arg2, "string") )
    return nl->SymbolAtom("bool");
  }
  return nl->SymbolAtom("typeerror");
}

/*
4.1.2 Value mapping functions of operator ~saveto~

*/
int
SaveJpegToFun(Word* args, Word& result, int message, Word& local, Supplier s)
{
  result = qp->ResultStorage( s );
  JPEG *jpegFile = (JPEG *)args[0].addr;

  if (!jpegFile)
  {
    cerr << "\nerror: SaveJPEGToFun() received 0x00 as arg for jpeg.\n";
    jpegAlgGlobalError = true;
    ((CcBool *)result.addr)->Set( true, false );
    return 0;
  }

  CcString *fileName = (CcString*)args[1].addr;

  // (SaveToFile returns false and does nothing if jpegFile is not 'defined')

  if( jpegFile->SaveToFile( *(fileName->GetStringval()) ) )
    ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );

  return 0;
}

/*
4.1.3 Specification of operator ~saveto~

*/
const string SaveJpegToSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
        "( <text>(jpeg string) -> bool</text--->"
          "<text>_ savejpegto _</text--->"
          "<text>Saves the image object into a JPEG-image (OS-)file.</text--->"
          "<text>query imgObject savejpegto \"filename.jpg\"</text--->"
      ") )";

/*
4.1.4 Definition of operator ~saveto~

*/
Operator savejpegto (
        "savejpegto",           //name
        SaveJpegToSpec,         //specification
        SaveJpegToFun,          //value mapping
        Operator::DummyModel,   //dummy model mapping, defined in Algebra.h
        Operator::SimpleSelect, //trivial selection function
        SaveJpegToTypeMap       //type mapping
);


/*
4.2 operator ~show~

4.2.1 Type Mapping of operator ~show~

Operator ~show~ accepts a binary file object to show and returns a boolean
meaning success or not.

----    (binfile)               -> bool
----

*/

ListExpr
ShowJpegTypeMap( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength(args) == 1 )
  {
    arg1 = nl->First(args);
    if ( nl->IsEqual(arg1, "jpeg"))
      return nl->SymbolAtom("bool");
  }
  return nl->SymbolAtom("typeerror");
}

/*
4.2.2 Value mapping functions of operator ~show~

*/
int
ShowJpegFun(Word* args, Word& result, int message, Word& local, Supplier s)
{
  result = qp->ResultStorage( s );
  JPEG *jpegObj = (JPEG *)args[0].addr;

  if (!jpegObj)
  {
    cerr << "\nerror: ShowJpegFun() received 0x00 as arg for jpeg.\n";
    jpegAlgGlobalError = true;
    ((CcBool *)result.addr)->Set( true, false );
    return 0;
  }

  if (!jpegObj->IsDefined())
  {
    ((CcBool *)result.addr)->Set( true, false ); // showing failure to QP
    jpegAlgGlobalError = true;
    ((CcBool *)result.addr)->Set( true, false );
    return 0; // indicating error 
  }

  if( jpegObj->Show() )
    ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );

  return 0; // success
}

/*
4.2.3 Specification of operator ~show~

*/
const string ShowJpegSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
        "( <text>(jpeg string) -> bool</text--->"
          "<text>_ show</text--->"
          "<text>Shows JPEG-image object. For Devel. + Test only. OS-dependent."\
          "</text--->"
          "<text>query imgObject show</text--->"
      ") )";

/*
4.2.4 Definition of operator ~show~

*/
Operator show (
        "show",                 //name
        ShowJpegSpec,           //specification
        ShowJpegFun,            //value mapping
        Operator::DummyModel,   //dummy model mapping, defined in Algebra.h
        Operator::SimpleSelect, //trivial selection function
        ShowJpegTypeMap         //type mapping
);

/*
4.3 operator ~demo~

This is only to demonstrate how to use the JPEGSource,
but surely doesn't demo all its flexibilities. (for develop + debug)

4.3.1 Type Mapping of operator ~demo~

Operator ~demo~ accepts a jpeg, parses its comment (dummy-impl.) which by
default is NOT parsed by libjpeg   and  looks up if the image is a Adobe-
created one  which info is not a member variable of JPEG  nor JPEG-SourceMgr.
It returns a boolean meaning success or not.

----    (jpeg)               -> bool
----

*/

ListExpr
DemoJpegTypeMap( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength(args) == 1 )
  {
    arg1 = nl->First(args);
    if ( nl->IsEqual(arg1, "jpeg"))
      return nl->SymbolAtom("bool");
  }
  return nl->SymbolAtom("typeerror");
}

/*
4.3.2 Value mapping functions of operator ~demo~

*/
int
DemoJpegFun(Word* args, Word& result, int message, Word& local, Supplier s)
{
  result = qp->ResultStorage( s );
  JPEG *jpegObj = (JPEG *)args[0].addr;

  if (!jpegObj)
  {
    cerr << "\nerror: DemoJpegFun() received 0x00 as arg for jpeg.\n";
    jpegAlgGlobalError = true;
    ((CcBool *)result.addr)->Set( true, false );
    return 0;
  }

  if (!jpegObj->IsDefined())
  {
    ((CcBool *)result.addr)->Set( true, false );
    jpegAlgGlobalError = true;
    return 0; 
  }

  if( jpegObj->Demo() )
    ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );

  return 0; // success
}

/*
4.3.3 Specification of operator ~demo~

*/
const string DemoJpegSpec  =
          "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
            "( <text>(jpeg string) -> bool</text--->"
              "<text>_ demo</text--->"
              "<text>Demos JPEGSourceMgr. Parses again for comment. Tells "\
                    "if Adobe-Picture.</text--->"
              "<text>query imgObject demo</text--->"
          ") )";

/*
4.3.4 Definition of operator ~demo~

*/
Operator demo (
        "demo",                 //name
        DemoJpegSpec,           //specification
        DemoJpegFun,            //value mapping
        Operator::DummyModel,   //dummy model mapping, defined in Algebra.h
        Operator::SimpleSelect, //trivial selection function
        DemoJpegTypeMap         //type mapping
);


/*
5.1 operator ~col\_d\_test~

5.1.1 Type Mapping of operator ~col\_d\_test~

Operator ~col\_d\_test~ accepts a jpeg object to show
and returns a boolean meaning success or not.

----    (jpeg)               -> bool
----

*/

ListExpr
ColDTestTypeMap( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength(args) == 1 )
  {
    arg1 = nl->First(args);
    if ( nl->IsEqual(arg1, "jpeg"))
      return nl->SymbolAtom("bool");
  }
  return nl->SymbolAtom("typeerror");
}

/*

5.1.2 Value mapping functions of operator ~col\_d\_test~

*/
int
ColDTestFun(Word* args, Word& result, int message, Word& local, Supplier s)
{
  result = qp->ResultStorage( s );
  JPEG *jpegObj = (JPEG *)args[0].addr;

  if (!jpegObj)
  {
    cerr << "\nerror: ColDTestFun() received 0x00 as arg for jpeg.\n";
    jpegAlgGlobalError = true;
    ((CcBool *)result.addr)->Set( true, false );
    return 0;
  }

  if (!jpegObj->IsDefined())
  {
    ((CcBool *)result.addr)->Set( true, false ); // showing failure to QP
    jpegAlgGlobalError = true;
    ((CcBool *)result.addr)->Set( true, false );
    return 0; // indicating error 
  }

  if( jpegObj->ColDTest() )
    ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );

  return 0; // success
}


/*

5.1.3 Specification of operator ~col\_d\_test~

*/
const string ColDTestSpec  =
          "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
            "( <text>(jpeg string) -> bool</text--->"
              "<text>_ col_d_test</text--->"
              "<text>Shows ColorDistribution of JPEG. Test only.</text--->"
              "<text>query imgObject col_d_test</text--->"
          ") )";

/*

5.1.4 Definition of operator ~col\_d\_test~

*/
Operator col_d_test (
        "col_d_test",             //name
        ColDTestSpec,             //specification
        ColDTestFun,              //value mapping
        Operator::DummyModel,     //dummy model mapping, defined in Algebra.h
        Operator::SimpleSelect,   //trivial selection function
        ColDTestTypeMap           //type mapping
);



/*
9.9 Operator ~cut~

Cuts from JPEG injpeg an area of [xdiff, ydiff] from origin (x;y) and yields
the newly constructed JPEG from this.

9.9.1 Type Mapping of operator ~cut~

The list representation is:

----    (jpeg int int int int)-> (jpeg)
----

Example call: query injpeg cut [x, y, xdiff, ydiff];


*/

ListExpr
CutTypeMap( ListExpr args )
{
  ListExpr argjpeg, argx, argy, argxdist, argydist;

  if ( nl->ListLength(args) == 5 )
  {
    argjpeg  = nl->First(args);  //  1    2   3   4   5
    argx     = nl->Second(args); // (jpeg int int int int)
    argy     = nl->Third(args);  
    argxdist = nl->Fourth(args);
    argydist = nl->Fifth(args); 

    if ( !(nl->IsAtom(argjpeg)  ) ||
         !(nl->IsAtom(argx)     ) ||
         !(nl->IsAtom(argy)     ) ||
         !(nl->IsAtom(argxdist) ) ||
         !(nl->IsAtom(argydist) ) ||
         !(nl->IsEqual(argjpeg,   "jpeg")) ||
         !(nl->IsEqual (argx,     "int"))  ||
         !(nl->IsEqual (argy,     "int"))  ||
         !(nl->IsEqual (argxdist, "int"))  ||
         !(nl->IsEqual (argydist, "int"))  )
    {
      string theErrorIs = "unspecified";

      if ( !(nl->IsAtom(argjpeg)) )
        theErrorIs = "Arg 1 is a list, not a JPEG.";
      else if (    !(nl->IsAtom(argx))     || !(nl->IsAtom(argy))
                || !(nl->IsAtom(argxdist)) || !(nl->IsAtom(argydist)) )
        theErrorIs = "Arg 2 to 5 require int-Atoms.";
      else if ( !(nl->IsEqual(argjpeg,   "jpeg")) )
        theErrorIs = "Arg 1 is not a jpeg";
      else if ( !(nl->IsEqual (argx,     "int"))  ||
                !(nl->IsEqual (argy,     "int"))  ||
                !(nl->IsEqual (argxdist, "int"))  ||
                !(nl->IsEqual (argydist, "int"))     )
        theErrorIs = "One of the x, y, xdist, ydist -values is not an int.";

      ErrorReporter::ReportError("Incorrect input types for operator cut:"
                                 + theErrorIs);
    }
    else
      return nl->SymbolAtom("jpeg");  // ret  (jpeg)

    // correct range of int-values can't be checked here
  }
  return nl->SymbolAtom("typeerror");
}


/*
9.9.2 Value mapping function of operator ~cut~

*/
int
CutFun( Word*  args,  Word& result, int  message,
        Word&  local, Supplier s )
{
  JPEG* injpeg;
  JPEG* outjpeg;
  bool  initialized = false;
  int   x, y, xdist, ydist;

  injpeg  = ((JPEG*)args[0].addr); // JPEGSourceMgr constructed, since arg0

  if (!injpeg)
  {
    cerr << "\nerror: CutFun() received 0x00 as arg for jpeg.\n";
    jpegAlgGlobalError = true;
    result = SetWord(new JPEG(0)); // SecondoSystem doesn't cope with 0
    return 0;
  }

  initialized = injpeg->srcConnected;

  if (!initialized)
    injpeg->InitializeJPEG();

  assert(injpeg->srcConnected);
  assert(injpeg->CInfoValid());

  x     = ((CcInt*)args[1].addr)->GetIntval();
  y     = ((CcInt*)args[2].addr)->GetIntval();
  xdist = ((CcInt*)args[3].addr)->GetIntval();
  ydist = ((CcInt*)args[4].addr)->GetIntval();

  // plausibility-check
  if (    x < 0 || y < 0 || xdist <= 0 || ydist <= 0
       || (x + xdist) > (int)injpeg->GetWidth()
       || (y + ydist) > (int)injpeg->GetHeight()
     )
  {
    // this is a user error, not a JPEGAlgebra-Error
    cout << "\nCan't cut: Coordinates are not within source-jpeg.\n";
    if (!initialized)
      injpeg->DropSrcMgr();
    result = SetWord(0); // SecondoSystem doesn't cope with 0
    return 0;

    // perhaps alternatively cutting a default size ???
  }
  
  // estimate destination-FLOB size; SrcMgr will append or shrink:
  int estimatedSize = injpeg->GetFLOB(0)->Size();

  if (estimatedSize == 0)
  {
    // this _is_ a JPEGAlgebra-Error
    cerr << "\nCan't cut: Size of source-jpeg is 0!\n";
    if (!initialized)
      injpeg->DropSrcMgr();

    jpegAlgGlobalError = true;
    result = SetWord(new JPEG(0)); // SecondoSystem doesn't cope with 0
    return 0;
  }
  

  // refine estimation regarding cut-size:
  int divisor = (injpeg->GetHeight() / ydist) * (injpeg->GetWidth() / xdist);
  // injpeg->height and injpeg->width shouldn't be 0 ... but to be sure:
  if (divisor <= 0)
    divisor = 1;
  estimatedSize = estimatedSize / divisor;
                                                                                    
  outjpeg = new JPEG(estimatedSize); // constructs also FLOB of right size
                                     // doesn't need a SourceMgr, injpeg does

  if (!outjpeg)
  {
    cerr << "\nerror: CutFun() could't construct output-Jpeg.\n";
    if (!initialized)
      injpeg->DropSrcMgr();
      
    jpegAlgGlobalError = true;
    result = SetWord(new JPEG(0)); // SecondoSystem doesn't cope with 0
    return 0;
  }
                                 
  injpeg->Cut(outjpeg, x, y, xdist, ydist);
  
  if (!initialized)
    injpeg->DropSrcMgr();

  result = SetWord(outjpeg);

  return (0);
}


/*

9.9.3 Specification of operator ~cut~

*/
const string CutSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>(jpeg int int int int)-> (jpeg)</text---> "
      "<text>_ cut [_, _, _, _]</text--->"
      "<text>Cuts from JPEG injpeg an area of [xdiff, ydiff] from origin (x;y)"
            " and yields the newly constructed JPEG from this.</text--->"
      "<text>query injpeg cut [x, y, xdiff, ydiff];</text--->"
  " ) )";

/*

9.9.4 Definition of operator ~cut~

*/
Operator cut (
        "cut",                  //name
        CutSpec,                //specification
        CutFun,                 //value mapping
        Operator::DummyModel,   //dummy model mapping, defined in Algebra.h
        Operator::SimpleSelect, //trivial selection function
        CutTypeMap              //type mapping
);


/*
9.9 Operator ~tiles~

Disassembles JPEG injpeg tiles of size[xdiff, ydiff] and yields stream of
these tiles. Clips the rests on the right and bottem that doesn't divide the
size.

9.9.1 Type Mapping of operator ~tiles~

The list representation is:

----    (jpeg int int)-> (stream (jpeg))
----

Example call: query injpeg tiles [xdiff, ydiff];


*/

ListExpr
TilesTypeMap( ListExpr args )
{
  ListExpr argjpeg, argxdist, argydist;

  if ( nl->ListLength(args) == 3 )
  {
    argjpeg  = nl->First(args);  //  1    2   3  
    argxdist = nl->Second(args); // (jpeg int int)
    argydist = nl->Third(args);

    if ( !(nl->IsAtom(argjpeg)  ) ||
         !(nl->IsAtom(argxdist) ) ||
         !(nl->IsAtom(argydist) ) ||
         !(nl->IsEqual(argjpeg,   "jpeg")) ||
         !(nl->IsEqual (argxdist, "int"))  ||
         !(nl->IsEqual (argydist, "int"))  )
    {
      string theErrorIs = "unspecified";

      if ( !(nl->IsAtom(argjpeg)) )
        theErrorIs = "Arg 1 is a list, not a JPEG.";
      else if ( !(nl->IsAtom(argxdist)) || !(nl->IsAtom(argydist)) )
        theErrorIs = "Arg 2 to 5 require int-Atoms.";
      else if ( !(nl->IsEqual(argjpeg,   "jpeg")) )
        theErrorIs = "Arg 1 is not a jpeg";
      else if ( !(nl->IsEqual (argxdist, "int"))  ||
                !(nl->IsEqual (argydist, "int"))     )
        theErrorIs = "One of the xdist, ydist -values is not an int.";

      ErrorReporter::ReportError("Incorrect input types for operator tiles:"
                                 + theErrorIs);
    }
    else
      return 
       nl->TwoElemList ( nl->SymbolAtom("stream"),
                         nl->TwoElemList(
                           nl->SymbolAtom("tuple"),
                           nl->OneElemList(
                               nl->TwoElemList(nl->SymbolAtom("pict"),
                                               nl->SymbolAtom("jpeg")))));

//       nl->TwoElemList( nl->SymbolAtom("stream"),
//                              nl->OneElemList(nl->SymbolAtom("jpeg"))
//                            );  // ret  (stream (jpeg))
    // correct range of int-values can't be checked here
  }
  return nl->SymbolAtom("typeerror");
}


// workaround helper class; should be replaced by better data structure
// perhaps use of CTable ....

class TupleStack {

 public:

   TupleStack(int stacksize) {
     if (stacksize < 1)
      stacksize = 1;
     arr = new Tuple*[stacksize];
     maxIndex = stacksize-1;
     size = 0;
   };

   ~TupleStack()
   {
     delete [] arr;
   };

   void push(Tuple * t){
     assert(size <= maxIndex);
     arr[size] = t;
     size++;
   }

   Tuple * pop()
   {
     assert(size > 0);
     Tuple * t = arr[size-1];
     size--;
     return t;
   }

   bool isEmpty()
   {
     return size == 0;
   }

   Tuple ** arr;
   int size;
   int maxIndex;

};



/*
9.9.2 Value mapping function of operator ~tiles~

*/

int
TilesFun( Word*  args,  Word& result, int  message,
          Word&  local, Supplier s )
{
  JPEG* injpeg;
  JPEG* outjpeg;
  Word injpegWord, xdistWord, ydistWord;
  //bool  initialized = false;
  int   xdist, ydist, r, c;
  int tileNo;
  int cols, rows, divisor, estimatedSize;
  ListExpr resultType;
  TupleType *tupleType;
  Tuple *t;
  TupleStack * vTiles;
  
 switch(message)
  {
    case OPEN:
      qp->Request(args[0].addr, injpegWord);
      injpeg  = ((JPEG*)injpegWord.addr); // JPEGSourceMgr constructed, since arg0

      if (!injpeg)
      {
        cerr << "\nerror: TilesFun() received 0x00 as arg for jpeg.\n";
        jpegAlgGlobalError = true;
        result = SetWord(0);
        return 0;
      }

//      initialized = injpeg->srcConnected;

//      if (!initialized)
       injpeg->InitializeJPEG();

//      assert(injpeg->srcConnected);
//      assert(injpeg->CInfoValid());

      qp->Request(args[1].addr, xdistWord);
      qp->Request(args[2].addr, ydistWord);
      xdist = ((CcInt*)xdistWord.addr)->GetIntval();
      ydist = ((CcInt*)ydistWord.addr)->GetIntval();
      rows = injpeg->GetHeight() / ydist;
      cols = injpeg->GetWidth()  / xdist;

      // estimate destination-FLOB size; SrcMgr will append or shrink:
      estimatedSize = injpeg->GetFLOB(0)->Size();

      // refine estimation regarding tile-size:
      divisor = (rows * cols);
      // injpeg->height and injpeg->width shouldn't be 0 ... but to be sure:
      if (divisor <= 0)
        divisor = 1;
      estimatedSize = estimatedSize / divisor;
      
 
      // the following checks only need to be don once, here in OPEN:
    
      // plausibility-check
      if (  xdist <= 0                      || ydist <= 0
         || xdist > (int)injpeg->GetWidth() || ydist > (int)injpeg->GetHeight()
         )
      {
        // this is a user error, not a JPEGAlgebra-Error
        cout << "\nCan't tile: Tile-Size too small or big.\n";
        result = SetWord(0);
        return 0;
      }

      if (estimatedSize == 0)
      {
        // this _is_ a JPEGAlgebra-Error
        cerr << "\nCan't tile: Size of source-jpeg is 0!\n";
        
        jpegAlgGlobalError = true;
        result = SetWord(0); 
        return 0;
      }
     
      if (rows * ydist < (int) injpeg->GetHeight())
        cout << "\nTile-Height doesn't match exactly. Bottom rest will be skipped"
             << endl;
      if (cols * xdist < (int)injpeg->GetWidth())
        cout << "\nTile-Width doesn't match exactly. Rest on the right will be "
             << "skipped\n";

      resultType = GetTupleResultType(s);
      tupleType = new TupleType (nl->Second (resultType) );

      tileNo = 0;
      vTiles = new TupleStack(rows * cols);

      for (r = rows-1; r >= 0 ; r--)
        for (c = cols-1; c >= 0 ; c--)
        {
          outjpeg = new JPEG(estimatedSize); // constructs also FLOB of right size
                                       // doesn't need a SourceMgr, injpeg does
          if (!outjpeg)
          {
            cerr << "\nerror: TilesFun() could't construct output-Jpeg.\n";
            
            jpegAlgGlobalError = true;
            result = SetWord(0); 
            return 0;
          }

          if ( !injpeg->Tile(outjpeg, c*xdist, r*ydist, xdist, ydist) ) {
            cerr << "Error: Problems with injepeg->Tile" << endl;
          } 

          // M. Spiekermann: This is not the idea of stream processing. Stream processing is used
          // to save memory hence data should only be loaded into memory when it is
          // requested.
          t = new Tuple (*tupleType, true);
          t->PutAttribute(0, outjpeg);
          vTiles->push(t);  // The last cvs version of the JPEG group contained
                            // a bug here. However, an output relation will no be created 
                            // but all tiles contain the same picture. Maybe there is a bug
                            // in the SrcMgr code or the extension of the jpeg library functions.

          if ( RTFlag::isActive(JPEG_RT_DEBUG) ) {
            cerr << "push() -> tuple address: " << t << endl;
            cerr << "Attribute address: " << outjpeg << endl;
          }
        }

      local = SetWord(vTiles);
    
      return 0;

    case REQUEST:

      vTiles = (TupleStack*) local.addr;
      if (!vTiles->isEmpty())
      {
        t = vTiles->pop();
        Attribute* attr = t->GetAttribute(0);
        if ( RTFlag::isActive(JPEG_RT_DEBUG) ) {
          cerr << "pop() -> tuple address: " << t << endl;
          cerr << "Attribute dddress: " << attr << endl;
        }
        result = SetWord(t);
        return YIELD;
      }
      return CANCEL;

    case CLOSE:
      vTiles = (TupleStack*) local.addr;
      delete vTiles; // JPEG * need and may not be deleted!    
      return (0);
  }
  
  // we can't arrive here only if 'message' is not faulty
  return -1;  
}

/*
9.9.2 Value mapping function of operator ~tiles~

*/
/*
int
TilesFun( Word*  args,  Word& result, int  message,
          Word&  local, Supplier s )
{
  JPEG* injpeg;
  JPEG* outjpeg;
  bool  initialized = false;
  int   x, y, xdist, ydist;
  CcInt * CcTileNo;
  int tileNo;
  int cols, rows, divisor, estimatedSize;
  Word injpegWord, xdistWord, ydistWord;
  ListExpr resultType;
  TupleType *tupleType;
  Tuple *t;

  qp->Request(args[0].addr, injpegWord);
  injpeg  = ((JPEG*)injpegWord.addr); // JPEGSourceMgr constructed, since arg0

  if (!injpeg)
  {
    cerr << "\nerror: TilesFun() received 0x00 as arg for jpeg.\n";
    jpegAlgGlobalError = true;
    result = SetWord(0);
    return 0;
  }

  initialized = injpeg->srcConnected;

  if (!initialized)
    injpeg->InitializeJPEG();

  assert(injpeg->srcConnected);
  assert(injpeg->CInfoValid());

  qp->Request(args[1].addr, xdistWord);
  qp->Request(args[2].addr, ydistWord);
  xdist = ((CcInt*)xdistWord.addr)->GetIntval();
  ydist = ((CcInt*)ydistWord.addr)->GetIntval();
  rows = injpeg->GetHeight() / ydist;
  cols = injpeg->GetWidth()  / xdist;

  // estimate destination-FLOB size; SrcMgr will append or shrink:
  estimatedSize = injpeg->GetFLOB(0)->Size();

  // refine estimation regarding tile-size:
  divisor = (injpeg->GetHeight() / ydist) * (injpeg->GetWidth() / xdist);
  // injpeg->height and injpeg->width shouldn't be 0 ... but to be sure:
  if (divisor <= 0)
    divisor = 1;
  estimatedSize = estimatedSize / divisor;


  switch(message)
  {
    case OPEN:

      // the following checks only need to be don once, here in OPEN:

      // plausibility-check
      if (  xdist <= 0                      || ydist <= 0
         || xdist > (int)injpeg->GetWidth() || ydist > (int)injpeg->GetHeight()
         )
      {
        // this is a user error, not a JPEGAlgebra-Error
        cout << "\nCan't tile: Tile-Size too small or big.\n";
        if (!initialized)
          injpeg->DropSrcMgr();
        result = SetWord(0);
        return 0;
      }

      if (estimatedSize == 0)
      {
        // this _is_ a JPEGAlgebra-Error
        cerr << "\nCan't tile: Size of source-jpeg is 0!\n";
        if (!initialized)
          injpeg->DropSrcMgr();

        jpegAlgGlobalError = true;
        result = SetWord(0);
        return 0;
      }

      if (rows * ydist < (int) injpeg->GetHeight())
        cout << "\nTile-Height doesn't match exactly. Bottom rest will be skipped"
             << endl;
      if (cols * xdist < (int)injpeg->GetWidth())
        cout << "\nTile-Width doesn't match exactly. Rest on the right will be "
             << "skipped\n";

      tileNo = 0;
      CcTileNo = new CcInt(true, tileNo);
      local = SetWord(CcTileNo);

      return 0;

    case REQUEST:

      CcTileNo = (CcInt*) local.addr;
      tileNo = CcTileNo->GetIntval();
      
      // Increment remembered tile number
      CcTileNo->Set(true, tileNo + 1);

      local = SetWord(CcTileNo);

      // actual origin to cut from?
      x = tileNo % cols;
      y = tileNo / cols;

      outjpeg = new JPEG(estimatedSize); // constructs also FLOB of right size
                                       // doesn't need a SourceMgr, injpeg does
      if (!outjpeg)
      {
        cerr << "\nerror: TielsFun() could't construct output-Jpeg.\n";
        if (!initialized)
          injpeg->DropSrcMgr();

       jpegAlgGlobalError = true;
       result = SetWord(new JPEG(0)); // SecondoSystem doesn't cope with 0
       return CANCEL;
      }

      injpeg->Tile(outjpeg, x, y, xdist, ydist); // same as Cut()

      if (!initialized)
        injpeg->DropSrcMgr();

      resultType = GetTupleResultType(s);
      tupleType = new TupleType (nl->Second (resultType) );
      t = new Tuple (*tupleType, true);
      t->PutAttribute(0, outjpeg);

      result = SetWord(t);
      return YIELD;

    case CLOSE:
      CcTileNo = (CcInt*) local.addr;
      delete CcTileNo;
      return (0);
  }

  // we can't arrive here only if 'message' is not faulty
  return -1;
}
*/
/*

9.9.3 Specification of operator ~tiles~

*/
const string TilesSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>(jpeg int int)-> (stream (jpeg))</text---> "
      "<text>_ tiles [_, _]</text--->"
      "<text>Disassembles JPEG injpeg tiles of size[xdiff, ydiff]"
            " and yields stream of these tiles. Clips the rests "
            "on the right and bottem that doesn't divide the size.</text--->"
      "<text>query injpeg tiles [xdiff, ydiff];</text--->"
  " ) )";

/*

9.9.4 Definition of operator ~tiles~

*/
Operator tiles (
        "tiles",                //name
        TilesSpec,              //specification
        TilesFun,               //value mapping
        Operator::DummyModel,   //dummy model mapping, defined in Algebra.h
        Operator::SimpleSelect, //trivial selection function
        TilesTypeMap            //type mapping
);




/*
9.9 Operator ~downsize~

Reduces image quality (and therefore size) to the desired percentage.

9.9.1 Type Mapping of operator ~downsize~

The list representation is:

----    (jpeg int)-> (jpeg)
----

Example call: query injpeg downsize 45;


*/

ListExpr
DownSizeTypeMap( ListExpr args )
{
  ListExpr argjpeg, argqual;

  if ( nl->ListLength(args) == 2 )
  {
    argjpeg  = nl->First(args);  //  1    2   
    argqual  = nl->Second(args); // (jpeg int)
                                         
    if ( !(nl->IsAtom(argjpeg)  ) ||
         !(nl->IsAtom(argqual)  ) ||
         !(nl->IsEqual(argjpeg,   "jpeg")) ||
         !(nl->IsEqual (argqual,  "int"))  
       )
    {
      string theErrorIs = "unspecified";

      if ( !(nl->IsAtom(argjpeg)) )
        theErrorIs = "Arg 1 is a list, not a JPEG.";
      else if ( !(nl->IsAtom(argqual)) )
        theErrorIs = "Arg 2 requires int-Atom.";
      else if ( !(nl->IsEqual(argjpeg,   "jpeg")) )
        theErrorIs = "Arg 1 is not a jpeg";
      else if ( !(nl->IsEqual (argqual,     "int")) )
        theErrorIs = "Quality-value is not an int.";

      ErrorReporter::ReportError("Incorrect input types for operator downsize:"
                                 + theErrorIs);
    }
    else
      return nl->SymbolAtom("jpeg");  // ret  (jpeg)

    // correct range of int-values can't be checked here
  }
  return nl->SymbolAtom("typeerror");
}


/*
9.9.2 Value mapping function of operator ~downsize~

*/
int
DownSizeFun( Word*  args,  Word& result, int  message,
        Word&  local, Supplier s )
{
  JPEG* injpeg;
  JPEG* outjpeg;
  bool  initialized = false;
  int   quality;

  injpeg  = ((JPEG*)args[0].addr); // JPEGSourceMgr constructed, since arg0

  if (!injpeg)
  {
    cerr << "\nerror: DownSizeFun() received 0x00 as arg for jpeg.\n";
    jpegAlgGlobalError = true;
    result = SetWord(new JPEG(0)); // SecondoSystem doesn't cope with 0
    return 0;
  }

  initialized = injpeg->srcConnected;

  if (!initialized)
    injpeg->InitializeJPEG();

  assert(injpeg->srcConnected);
  assert(injpeg->CInfoValid());

  quality = ((CcInt*)args[1].addr)->GetIntval();

  // estimate destination-FLOB size; SrcMgr will append or shrink:
  int estimatedSize = injpeg->GetFLOB(0)->Size();

  if (estimatedSize == 0)
  {
    // this _is_ a JPEGAlgebra-Error
    cerr << "\nCan't downsize: Size of source-jpeg is 0!\n";
    if (!initialized)
      injpeg->DropSrcMgr();

    jpegAlgGlobalError = true;
    result = SetWord(new JPEG(0)); // SecondoSystem doesn't cope with 0
    return 0;
  }

  outjpeg = new JPEG(estimatedSize); // constructs also FLOB of right size
                                     // doesn't need a SourceMgr, injpeg does

  if (!outjpeg)
  {
    cerr << "\nerror: DownSizeFun() could't construct output-Jpeg.\n";
    if (!initialized)
      injpeg->DropSrcMgr();

    jpegAlgGlobalError = true;
    result = SetWord(new JPEG(0)); // SecondoSystem doesn't cope with 0
    return 0;
  }

  injpeg->DownSize(outjpeg, quality);

  if (!initialized)
    injpeg->DropSrcMgr();

  result = SetWord(outjpeg);

  return (0);
}


/*

9.9.3 Specification of operator ~downsize~

*/
const string DownSizeSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>(jpeg int)-> (jpeg)</text---> "
      "<text>_ downsize _</text--->"
      "<text>Reduces image quality (and therefore size) to the desired percentage.</text--->"
      "<text>query injpeg downsize 45;</text--->"
  " ) )";

/*

9.9.4 Definition of operator ~downsize~

*/
Operator downsize (
        "downsize",                  //name
        DownSizeSpec,                //specification
        DownSizeFun,                 //value mapping
        Operator::DummyModel,   //dummy model mapping, defined in Algebra.h
        Operator::SimpleSelect, //trivial selection function
        DownSizeTypeMap              //type mapping
);



// ======================================================

/*
4.3 Operator ~compbrightdist~

This operator compares the brightnessDistribution of a stream of jpeg's
with the brightnessDistribution of a specified jpeg. So it's possible to
find all pictures with similar brightnessDistribution in the database.

The user must specifiy "how similar" the found jpeg's should be:
the given real-argument defines the per Cent (Prozentwert) distance for
every value in the brightnessDistribution, which is concerned as similar.

4.3.1 Type Mapping of operator ~comparebrightness~

The list representation is:

----    (jpeg (stream (tuple ((xi ti)*) ) ) real string)-> (stream jpeg)
----

or

----    (jpeg jpeg real dummy)                        -> (bool)
----


Operator ~compbrightdist~ accepts a

  * jpeg - object: with the brightness- Distribution of this objects all
  other brightnessDistributions of the stream are compared.

  * stream of tuples: the string represents the name of an attribute in the
  tuple. The type of this attribute must be of type jpeg. The comparison will
  work with this jpeg.

  * or alternativey another jpeg - object: the brightness- Distributions are
  compared between the two jpeg - objects.

  * real-value: the distance for evey value (0..255)

  * string: the name of the compared attribute or only a dummy

*/

ListExpr
CompareBrightnessDistributionTypeMap( ListExpr args )
{
  ListExpr arg1, arg2, arg3, arg4, attrtype;
  string attrname;
  int j;

  if ( nl->ListLength(args) == 4 )
  {
    arg1  = nl->First(args);  //   1    2              3    4
    arg2  = nl->Second(args); // ( jpeg ( stream jpeg) real attrname)
    arg3  = nl->Third(args);  //real - distance
    arg4  = nl->Fourth(args); //string - attrname

    if ( (nl->IsAtom(arg1)          ) &&
         (nl->ListLength (arg2) == 2) &&
         (nl->IsAtom(arg3)          ) &&
         (nl->IsAtom(arg4)          ) &&
         (nl->IsEqual(arg1, "jpeg") ) &&
         (nl->ListLength(nl->Second(arg2)) == 2  ) &&
         (TypeOfRelAlgSymbol(nl->First(arg2)) == stream ) &&
         (TypeOfRelAlgSymbol(nl->First(nl->Second(arg2))) == tuple   ) &&
         (nl->IsEqual (arg3, "real")) )
    {
      attrname = nl->SymbolValue(arg4);
      j = FindAttribute(nl->Second(nl->Second(arg2)), attrname, attrtype);
      if (j && nl->IsEqual (attrtype, "jpeg"))
      {
        return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
                                 nl->OneElemList(nl->IntAtom(j)),
                                 nl->TwoElemList (nl->SymbolAtom("stream"),
                                                  nl->Second(arg2)));
      }
    }

    // In the case of comparison two jpeg-objects, the fourth argument is not
    // needed.

    if ( (nl->IsAtom(arg1)          ) &&   //single-jpeg object         'jpeg'
         (nl->IsAtom(arg2)          ) &&   //single-jpeg object         'jpeg'
         (nl->IsAtom(arg3)          ) &&   //real = distance value      'real'
         (nl->IsEqual(arg1, "jpeg") ) &&
         (nl->IsEqual(arg2, "jpeg") ) &&
         (nl->IsEqual(arg3, "real") ) )
    {
      return nl->SymbolAtom("bool");
    }
  }

  ErrorReporter::ReportError("Incorrect input for operator compbrightdist.");
  return nl->SymbolAtom("typeerror");
}

/*
4.3.2 Selection function of overloaded operator ~compbrightdist~

*/

static int
JPEGSelectCompbrightdist (ListExpr args)
{
  ListExpr arg1 = nl->First (args);
  ListExpr arg2 = nl->Second(args);

  if ( nl->IsEqual(arg1, "jpeg") && (nl->IsEqual(arg2, "jpeg")) )
    return (1);
  if ( nl->IsEqual(arg1, "jpeg") && (nl->IsEqual (nl->First(arg2), "stream")) )
    return (0);

  return (-1); // This point should never be reached
}

/*
4.3.2 Value mapping functions of operator ~compbrightdist~

*/

struct CompBrightDistLocalInfo
{
  double distance;
  bool isValidFirstJpeg;
  int pictIndex;
};

int
CompBDistFunJS (Word*  args,  Word&    result, int message,
                Word&  local, Supplier s)
{
  CompBrightDistLocalInfo* cli;
  Word indexWord, distance, elem, singleJpeg;
  JPEG* elemptr;
  JPEG* singleJpegPtr;

  switch (message)
  {
    case OPEN:
      //cout << "OPEN" << endl;
      qp->Open(args[1].addr);

      cli = new CompBrightDistLocalInfo;

      qp->Request(args[2].addr, distance);  // color-distance
      cli->distance = ((CcReal*)distance.addr)->GetRealval();

      qp->Request(args[0].addr, singleJpeg);
      
      singleJpegPtr = (JPEG*)singleJpeg.addr; // initialized in q-tree c'str'n
      if (!singleJpegPtr)
      {
        cerr << "\nerror: CompBDistFunJS() received 0x00 as arg for jpeg.\n";
        jpegAlgGlobalError = true;
        return 0;
      }

      cli->isValidFirstJpeg = (singleJpegPtr->GetColorSpace() == grayscale );

      qp->Request(args[4].addr, indexWord);
      cli->pictIndex = ((CcInt*)indexWord.addr)->GetIntval();

      local = SetWord (cli);
      return 0;

    case REQUEST:
      //cout << "REQUEST" << endl;
      cli = (CompBrightDistLocalInfo*) local.addr;

      qp->Request(args[0].addr, singleJpeg);            //singleJpeg
      if (singleJpeg.addr == 0)
      {
        cerr << "\nerror: CompBDistFunJS() received 0x00 as arg for jpeg.\n";
        jpegAlgGlobalError = true;
        return 0;
      }

      if (cli->isValidFirstJpeg)                   //singleJpeg == grayscale ??
      {
        qp->Request (args[1].addr, elem);               // next stream-element

        while (qp->Received(args[1].addr) )
        {
          Tuple *tu = (Tuple*)elem.addr;
          elemptr = (JPEG*) tu->GetAttribute(cli->pictIndex - 1);
          elemptr->InitializeJPEG();

          if ( elemptr->GetColorSpace() == 0 )
          {
            if ( ((JPEG*)singleJpeg.addr)->CompBright(elemptr, cli->distance) )
            {
              result = elem;
              elemptr->DropSrcMgr();
              return YIELD;
            }
            else
            {   //cout << "Do nothing" << endl;
            }
          }

          elemptr->DropSrcMgr();
          qp->Request (args[1].addr, elem);             // next stream-element
        }
      }
      else  // only consume the stream, when singleJpeg not a grayscale
      {
        qp->Request (args[1].addr, elem);               // next stream-element
        while (qp->Received(args[1].addr) )
          qp->Request (args[1].addr, elem);
      }
      return CANCEL;

    case CLOSE:
      //cout << "CLOSE" << endl;
      qp->Close(args[1].addr);
      delete cli;
      return 0;
  }

  /* should not happen */
  return -1;
}


int
CompBDistFunJJ( Word*  args,  Word&    result, int  message,
                Word&  local, Supplier s )
{
  JPEG* jfirst;
  JPEG* jsecond;
  double dist;

  jfirst  = ((JPEG*)args[0].addr);
  jsecond = ((JPEG*)args[1].addr);

  if (!jfirst || !jsecond)
  {
    cerr << "\nerror: CompBDistFunJJ() received 0x00 as one of its jpegs.\n";
    jpegAlgGlobalError = true;
    return 0;
  }

  dist = ((CcReal*)args[2].addr)->GetRealval();

  result = qp->ResultStorage (s);
  if ( (jfirst->GetColorSpace() == grayscale) &&
       (jsecond->GetColorSpace() == grayscale) )
  {
    ((CcBool*)result.addr)->Set (true, jfirst->CompBright (jsecond, dist));
  }
  else
  {
    cout << "ERROR: Operator compbrightdist expects two grayscale objects.\n";
    ((CcBool*)result.addr)->Set (true, false);
  }

  return (0);
}

/*

5.10.3 Value Mapping Vector for operator ~compbrightdist~

*/

ValueMapping JPEGCompBrightDistMap [] = {CompBDistFunJS,
                                          CompBDistFunJJ };

/*
4.3.1 The dummy model mapping:

*/
static Word
JPEGNoModelMapping( ArgVector arg, Supplier opTreeNode )
{
  return (SetWord( Address( 0 ) ));
}

/*

5.10.3 Cost Model Mapping Vector for operator ~compbrightdist~

*/

ModelMapping JPEGNoModelMap[] = {JPEGNoModelMapping,
                                 JPEGNoModelMapping};

/*

5.10.3 Specification of operator ~compbrightdist~

*/
const string CompareBrightnessDistributionSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>(jpeg stream (x) real string) -> (stream (x)),"
            "((jpeg jpeg) -> bool)</text---> "
      "<text>_ _ compbrightdist [_, _]</text--->"
      "<text>Compares the BrightnessDistribution of jpeg with all Brightness"
            "Distribution of (stream(x)) within a specified distance and given"
            " jpegAttributeName. "
            "Distance is a per Cent value for each BrightnessValue.</text--->"
      "<text>query jpegObj jpegRel feed compbrightdist [0.5, pict] count; "
            "query jpegObj1 jpegObj2 compbrightdist [0.5, dummy];</text--->"
  " ) )";

/*

5.10.4 Definition of operator ~compbrightdist~

*/
Operator jpegcompbrightdist (
         "compbrightdist",                    // name
         CompareBrightnessDistributionSpec,   // specification
         2,                                   // Number of overloaded functions
         JPEGCompBrightDistMap,               // value mapping
         JPEGNoModelMap,           // dummy model mapping, defines in Algebra.h
         JPEGSelectCompbrightdist,            //  selection function
         CompareBrightnessDistributionTypeMap // type mapping
);

/*
4.3 Operator ~comprgbdist~

This operator compares the rgbDistribution of two jpeg-objects. Also a
stream of tuples with jpeg-attribute's may be given as input,
then the rgbDistribution of the single jpeg-object is compared with the
rgbDistribution of a selected jpeg-attribute of the stream.

So it's possible to find all pictures with similar rgbDistribution in the
database.

The user must specifiy "how similar" the found jpeg's should be:
the given real-argument defines the per Cent (Prozentwert) distance for
every value in the brightnessDistribution, which is concerned as similar.

The operator expects three real values, the first represents the desired
distance for red, the second the distance for green and the third the
distance for blue.

4.3.1 Type Mapping of operator ~comprgbdist~

The list representation is:

---- (jpeg (stream(tuple((xi ti)*))) real real real string)-> (stream jpeg)
----

or

----    (jpeg jpeg real real real dummy)                        -> (bool)
----

For an textual description see operator ~compbrightdist~.

*/

ListExpr
CompareRgbDistributionTypeMap( ListExpr args )
{
  ListExpr arg1, arg2, arg3, arg4, arg5, arg6, attrtype;
  string attrname;
  int j;

  if ( nl->ListLength(args) == 6 )
  {
    arg1  = nl->First(args);
    arg2  = nl->Second(args);
    arg3  = nl->Third(args);
    arg4  = nl->Fourth(args);
    arg5  = nl->Fifth(args);
    arg6  = nl->Sixth(args);

    if ( (nl->IsAtom(arg1)          ) &&
         (nl->ListLength (arg2) == 2) &&
         (nl->IsAtom(arg3)          ) &&
         (nl->IsAtom(arg4)          ) &&
         (nl->IsAtom(arg5)          ) &&
         (nl->IsAtom(arg6)          ) &&
         (nl->ListLength(nl->Second(arg2)) == 2  ) &&
         (TypeOfRelAlgSymbol(nl->First(arg2)) == stream ) &&
         (TypeOfRelAlgSymbol(nl->First(nl->Second(arg2))) == tuple   ) &&
         (nl->IsEqual (arg1, "jpeg")) &&
         (nl->IsEqual (arg3, "real")) &&
         (nl->IsEqual (arg4, "real")) &&
         (nl->IsEqual (arg5, "real")) )
    {
      attrname = nl->SymbolValue(arg6);
      j = FindAttribute(nl->Second(nl->Second(arg2)), attrname, attrtype);
      if (j && nl->IsEqual (attrtype, "jpeg"))
      {
        return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
                                 nl->OneElemList(nl->IntAtom(j)),
                                 nl->TwoElemList (nl->SymbolAtom("stream"),
                                                  nl->Second(arg2)));
      }
    }

    if ( (nl->IsAtom(arg1)          ) &&   //single-jpeg object         'jpeg'
         (nl->IsAtom(arg2)          ) &&   //single-jpeg object         'jpeg'
         (nl->IsAtom(arg3)          ) &&   //real = distance value      'real'
         (nl->IsAtom(arg4)          ) &&
         (nl->IsAtom(arg5)          ) &&
         (nl->IsEqual(arg1, "jpeg") ) &&
         (nl->IsEqual(arg2, "jpeg") ) &&
         (nl->IsEqual(arg3, "real") ) &&
         (nl->IsEqual(arg4, "real") ) &&
         (nl->IsEqual(arg5, "real") ) )
    {
        return nl->SymbolAtom("bool");
    }

  }
  ErrorReporter::ReportError("Incorrect input for operator compbrightdist.");
  return nl->SymbolAtom("typeerror");
}

/*
4.3.2 Selection function of overloaded operator ~compbrightdist~

*/

static int
JPEGSelectComprgbdist (ListExpr args)
{
  ListExpr arg1 = nl->First (args);
  ListExpr arg2 = nl->Second(args);

  if ( nl->IsEqual(arg1, "jpeg") && (nl->IsEqual(arg2, "jpeg")) )
    return (1);
  if ( nl->IsEqual(arg1, "jpeg") &&
     ( !(nl->IsAtom(arg2)) &&
      (nl->IsEqual (nl->First(arg2), "stream"))) )
    return (0);


  return (-1); // This point should never be reached
}

/*
4.3.2 Value mapping functions of operator ~compbrightdist~

*/

struct CompRgbDistLocalInfo
{
  double rdist;
  double gdist;
  double bdist;
  bool isValidFirstJpeg;
  int pictIndex;
};

int
CompRgbDistribFunJS(Word* args, Word& result, int message,
                                             Word& local, Supplier s)
{
  CompRgbDistLocalInfo* rgbli;
  Word rdist, gdist, bdist, elem, singleJpeg, indexWord;
  JPEG* elemptr;
  JPEG* singleJpegPtr;

  switch (message)
  {
    case OPEN:
      //cout << "OPEN" << endl;
      qp->Open(args[1].addr);	// opens input-Stream (arg 2)

      rgbli = new CompRgbDistLocalInfo;

      qp->Request(args[2].addr, rdist);  // r color-distance
      rgbli->rdist = ((CcReal*)rdist.addr)->GetRealval();

      qp->Request(args[3].addr, gdist);  // g color-distance
      rgbli->gdist = ((CcReal*)gdist.addr)->GetRealval();

      qp->Request(args[4].addr, bdist);  // b color-distance
      rgbli->bdist = ((CcReal*)bdist.addr)->GetRealval();

      qp->Request(args[0].addr, singleJpeg);
      singleJpegPtr = (JPEG*)singleJpeg.addr;
      if (!singleJpegPtr)
      {
        cerr << "\nerror: CompRgbDistribFunJS() received 0x00 as arg "
                "for jpeg.\n";
        jpegAlgGlobalError = true;
        return 0;
      }

      rgbli->isValidFirstJpeg = (singleJpegPtr->GetColorSpace() == 1 );

      qp->Request(args[6].addr, indexWord);
      rgbli->pictIndex = ((CcInt*)indexWord.addr)->GetIntval();

      local = SetWord (rgbli);
      return 0;

    case REQUEST:
      //cout << "REQUEST" << endl;
      rgbli = (CompRgbDistLocalInfo*) local.addr;

      qp->Request(args[0].addr, singleJpeg);            //singleJpeg
      singleJpegPtr = (JPEG*)singleJpeg.addr;
      if (!singleJpegPtr)
      {
        cerr << "\nerror: CompRgbDistribFunJS() received 0x00 as arg "
                "for jpeg.\n";
        jpegAlgGlobalError = true;
        return 0;
      }
      
      if (rgbli->isValidFirstJpeg)                      
      {
        qp->Request (args[1].addr, elem);               // next stream-element

        while (qp->Received(args[1].addr) )
        {
          Tuple *tu = (Tuple*)elem.addr;
          elemptr = (JPEG*) tu->GetAttribute(rgbli->pictIndex - 1);
          elemptr->InitializeJPEG();

          if ( elemptr->GetColorSpace() == 1 )
          {
            if ( singleJpegPtr->CompRgb(elemptr, rgbli->rdist,
                                                 rgbli->gdist, rgbli->bdist) )
            {
              result = elem;
              elemptr->DropSrcMgr();
              return YIELD;
            }
          }
          else
          {
            //cout << "Do nothing" << endl;
          }
          elemptr->DropSrcMgr();
          qp->Request (args[1].addr, elem);             // next stream-element
        }
      }
      else
      {
        qp->Request (args[1].addr, elem);               // next stream-element
        while (qp->Received(args[1].addr) )
          qp->Request (args[1].addr, elem);
      }
      return CANCEL;

    case CLOSE:
      //cout << "CLOSE" << endl;
      qp->Close(args[1].addr);
      delete rgbli;
      return 0;
  }
  /* should not happen */
  return -1;
}


int
CompRgbDistribFunJJ(Word* args, Word& result, int message,
                                             Word& local, Supplier s)
{
  JPEG* jfirst;
  JPEG* jsecond;
  double rdist;
  double gdist;
  double bdist;

  jfirst  = ((JPEG*)args[0].addr);
  jsecond = ((JPEG*)args[1].addr);
  if (!jfirst || !jsecond)
  {
    cerr << "\nerror: CompRgbDistribFunJJ() received 0x00 as one of its "
            "jpegs.\n";
    jpegAlgGlobalError = true;
    return 0;
  }
  
  rdist = ((CcReal*)args[2].addr)->GetRealval();
  gdist = ((CcReal*)args[3].addr)->GetRealval();
  bdist = ((CcReal*)args[4].addr)->GetRealval();

  result = qp->ResultStorage (s);
  if ( (jfirst->GetColorSpace() == 1) &&
       (jsecond->GetColorSpace() == 1) )
  {
    ((CcBool*)result.addr)->Set (true, jfirst->CompRgb (jsecond, rdist, gdist, bdist));
  }
  else
  {
    cout << "ERROR: Operator comprgbdist expects two rgb objects." << endl;
    ((CcBool*)result.addr)->Set (true, false);
  }

  return (0);
}

/*

5.10.3 Value Mapping Vector for operator ~comprgbdist~

*/

ValueMapping JPEGCompRgbDistMap [] = {CompRgbDistribFunJS,
                                          CompRgbDistribFunJJ };

/*

5.10.3 Specification of operator ~comprgbdist~

*/
const string CompareRgbDistributionSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>(jpeg (stream (x)) real real real string)) -> (stream (x)),"
            "((jpeg jpeg real real real string) -> bool)</text---> "
      "<text>_ _ comprgbdist [_, _, _, _]</text--->"
      "<text>Compares the RGB-Distributions of jpeg with all "
            "RGB-Distributions of (stream(x)) within a specified distance. "
            "Distance is a per Cent value for each RGB-Value."
            "First real for red, second for green, third for blue."
            "The jpeg-Attributename is given as a string.</text--->"
      "<text>query jpegObj jpegRel feed comprgbdist [0.5, 0.1, 0.3, pict] count; "
            "query jpegObj1 jpegObj2 comprgbdist [0.5, 0.2, 0.3, dummy]</text--->"
  " ) )";

/*

5.10.4 Definition of operator ~comprgbdist~

*/
Operator jpegcomprgbdist (
         "comprgbdist",                       // name
         CompareRgbDistributionSpec,          // specification
         2,                                   // Number of overloaded functions
         JPEGCompRgbDistMap,                  // value mapping
         JPEGNoModelMap,           // dummy model mapping, defines in Algebra.h
         JPEGSelectComprgbdist,               //  selection function
         CompareRgbDistributionTypeMap        // type mapping
);



/*******************************************************************************
\rule {0 pt}{0 pt} \linebreak

*******************************************************************************/
/******************************************************************************
e n d\space\space   S e c o n d o - S u p p o r t - F u n c t i o n s,
        O p e r a t o r s   a n d   T y p e   C o n s t r u c t o r s \space
        f o r \linebreak J P E G

******************************************************************************/
/*******************************************************************************
\rule {460 pt}{1 pt} \linebreak
[newpage]

*******************************************************************************/

/*******************************************************************************
6 Type Constructor ~jinfo~: Implementation

*******************************************************************************/

/*
The intended use for the ~jinfo~-type constructor ist ONLY to get a nested list
representation of jpeg - objects. It is NOT intended to type a "let" command
manually by the user of the secondo-system.

The intended way is the use of the ~jinfo~-type constructor in conjunction with
the ~jreadinfo~ - operator. The ~jreadinfo~ operator computes the hundreds of
values necessary for ~jinfo~ and generates the jinfo objects.

*/

JINFO::JINFO(bool D, int inHeight, int inWidth,
             bool inColoured, int inCS, int inNumComp,
             ListExpr inPict)
{
  defined = D;
  height = inHeight;
  width = inWidth;
  coloured = inColoured;
  colorSpace = (colorS) inCS;
  numComponents = inNumComp;
  for (int j = 0; j <= 3; j++)
    for (int i = 0; i <=255; i++)
      cDistribution[i][j] = 0.0;
  picture = inPict;
}

JINFO::~JINFO() {}

int JINFO::GetHeight()
{
  assert( IsDefined() );
  return height;
}

int JINFO::GetWidth()
{
  assert( IsDefined() );
  return width;
}

bool JINFO::IsColoured()
{
  return coloured;
}

colorS JINFO::GetColorSpace()
{
  return colorSpace;
}

int JINFO::Get_num_components()
{
  return numComponents;
}

void JINFO::GetcDistribution(double outDistr[256][4])
{
  for (int k = 0; k <= 3; k++)
    for (int i = 0; i <= 255; i++)
      outDistr[i][k] = cDistribution[i][k];
}

ListExpr JINFO::GetPictureList()
{
  return picture;
}

void  JINFO::Set( const bool D,
                  const int  inHeight,       const int inWidth,
                  const bool inColoured,     const int inCS,
                  const int  inNumComponents, ListExpr inPict)
{
  defined  = D;
  height   = inHeight;
  width    = inWidth;
  coloured = inColoured;
  colorSpace    = (colorS) inCS;
  numComponents = inNumComponents;
  picture = inPict;
}


/******************************************************************************
[2] The following virtual functions are used for porting ~jinfo~ to Tuple.

******************************************************************************/

/*
The function Compare() defines a total order on the data type ~JINFO~.

*/
int JINFO::Compare(Attribute *arg)
{
 return (0);
}

bool JINFO::Adjacent(Attribute * arg)
{
    return false;
    //for pictures we can not decides whether they are adjacent or not.
}

JINFO* JINFO::Clone()
{
  return new JINFO ( *this );
}

bool JINFO::IsDefined() const
{
  return defined;
}

void JINFO::SetDefined( bool Defined )
{
  defined = Defined;
}

void JINFO:: SetcDistribution (int compIndex, int comp, double value)
{
  cDistribution[compIndex][comp] = value;
}

int  JINFO::Sizeof() const
{
  return sizeof(JINFO);
}

size_t JINFO::HashValue()
{
    if(!defined)  return (0);
    int h;
    int x = GetHeight();
    int y = GetWidth();
    int z = GetColorSpace();
    h = (int) (5 * x + z * y);
    return size_t (h);
}

void JINFO::CopyFrom(StandardAttribute* right)
{
  if (!right)
  {
    cerr << "\nerror: JINFO::CopyFrom() received 0x00 as StandardAttribute*."
         << endl;
    defined = false;
    jpegAlgGlobalError = true;
    return;
  }

  JINFO* j = (JINFO*)right;
  defined = j->IsDefined();
  if (defined)
  {
      Set( true,
           j->GetHeight(),          j->GetWidth(),
           j->IsColoured(),         j->GetColorSpace(),
           j->Get_num_components(), j->GetPictureList() );
  }
}

ostream& operator<<( ostream& o, JINFO& p )
{
  if( p.IsDefined() )
    o << "("  << p.GetHeight()      << ", " << p.GetWidth() << p.IsColoured()
      << ", " << p.GetColorSpace()  << ", " << p.Get_num_components()
      << ", " << p.GetPictureList() << ")";
  else
    o << "undef";

  return o;
}

ostream& JINFO::Print( ostream &os )
{
    if (defined)
      return (os <<  height << ", " << width << coloured << ", " << colorSpace
                 << ", "    << numComponents << ", "     << GetPictureList());
    else    return (os << "undefined");
}

/*
6.1 List Representation


The list representation is given through three sublists:

The first list contains the metadata, which is also given through the jpeg-
header. These are: Height, Width, IsColoured, ColorSpace and
Number\_of\_Color\_Components.

The second list contains four sublists, one sublist for every colorComponent.
Every represents the brightness or ColorDistribution of the jpeg. So the
sublist has 256 values representing the Distribution for this colorCompnent in
ascending order (from 0 to 255).

The third list contains the jpeg-picture in Base64 coded form.


----      ( (height width coloured colorSpace numComponents)
          ( (0.13 0.45 0.00 ..... 0.00 0.01)
            (0.00 0.1 0.2 ....     0.01 0.00)
            (0.00 0.1 0.1 ....      0.00 0.00)
            (0.0 0.0 ...            0.00 0.00) )
            (Base64CodedPicture) )
----

*/

/*
6.2 ~In~ and ~Out~ Functions

*/

/*
~OutJINFO~ generates the Nested List representation from the internal
representation.

*/
ListExpr
OutJINFO( ListExpr typeInfo, Word value )
{
  ListExpr Metadata, comp[4], appendList[4];
  JINFO* j;
  j = (JINFO*)(value.addr);

  if (!j)
    cerr << "\nerror: OutJINFO() received 0x00 as JINFO-address.\n";

  if( j &&
      j->IsDefined() )
  {
    double Distr[256][4];
    j->GetcDistribution(Distr);
    /* for (int k = 0; k <= 3; k++)
         for (int i = 0; i <= 255; i++)
           cout << "Comp:" << k << "   Index:" << i
                << "    Value:" << Distr[i][k] << endl; */

    Metadata = nl->FiveElemList(nl->IntAtom(j->GetHeight()),
                                nl->IntAtom(j->GetWidth()),
                                nl->BoolAtom(j->IsColoured()),
                                nl->IntAtom(j->GetColorSpace()),
                                nl->IntAtom(j->Get_num_components()) );

    for (int k = 0; k <= 3; k++)
    {
      //cout << "Create FirstElementOfList" << endl;
      comp[k] = nl->OneElemList(nl->RealAtom(Distr[0][k]));
      appendList[k] = comp[k];

      for (int l = 1; l <= 255; l++)
      {
        //cout << "Before Create NextElementOfList" << endl;
        appendList[k] = nl->Append(appendList[k],nl->RealAtom(Distr[l][k]));
        //cout << "After Create FirstElementOfList" << endl;
      }
    }

    return nl->ThreeElemList(
                    Metadata,
                    nl->FourElemList(comp[0], comp[1], comp[2], comp[3]),
                    nl->OneElemList(j->GetPictureList()) );
  }
  else
  {
    jpegAlgGlobalError = true;
    return (nl->SymbolAtom("undef"));
  }
}

/*
~InJINFO~ generates the internal representation from the Nested List
representation.

*/
Word
InJINFO( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  JINFO* newjinfo;

  if ( nl->ListLength( instance ) == 3 )
  {
    ListExpr First = nl->First(instance);
    ListExpr Second = nl->Second(instance);
    ListExpr Third = nl->Third(instance);

   if ( (nl->ListLength(First) == 5) &&
         (nl->ListLength(Second) == 4) )
   {
     ListExpr Dist[4];
     Dist[0] = nl->First(Second);
     Dist[1] = nl->Second(Second);
     Dist[2] = nl->Third(Second);
     Dist[3] = nl->Fourth(Second);

     ListExpr First1 = nl->First(First);
     ListExpr First2 = nl->Second(First);
     ListExpr First3 = nl->Third(First);
     ListExpr First4 = nl->Fourth(First);
     ListExpr First5 = nl->Fifth(First);

     if ( (nl->ListLength(Dist[0]) == 256) &&
          (nl->ListLength(Dist[1]) == 256) &&
          (nl->ListLength(Dist[2]) == 256) &&
          (nl->ListLength(Dist[3]) == 256) &&
           nl->IsAtom(First1) && nl->AtomType(First1) == IntType &&
           nl->IsAtom(First2) && nl->AtomType(First2) == IntType &&
           nl->IsAtom(First3) && nl->AtomType(First3) == BoolType &&
           nl->IsAtom(First4) && nl->AtomType(First4) == IntType &&
           nl->IsAtom(First5) && nl->AtomType(First5) == IntType )
     {
       correct = true;
       newjinfo = new JINFO(false, nl->IntValue(First1), nl->IntValue(First2),
                            nl->BoolValue(First3), nl->IntValue(First4),
                            nl->IntValue(First5), Third );

       ListExpr FirstVal, RestVal;
       for (int compIndex = 0; compIndex <= 3; compIndex++)
       {
          RestVal = Dist[compIndex];
          for (int cdx = 0; cdx <= 255; cdx ++)
          {
            FirstVal = nl->First(RestVal);
            RestVal = nl->Rest(RestVal);
            newjinfo->SetcDistribution(cdx, compIndex, nl->RealValue(FirstVal));
          }
       }

       newjinfo->SetDefined (true);

       return SetWord(newjinfo);
     }
    }
  }
  correct = false;
  return SetWord(Address(0));
}

/*
6.3 Functions Describing the Signature of the Type Constructors

This one works for type constructors ~jinfo~.

*/
ListExpr
JINFOProperty()
{
  return (nl->TwoElemList(
     nl->SixElemList(nl->StringAtom("Signature"),
        nl->StringAtom("Example Type List"),
        nl->StringAtom("List Rep"),
        nl->StringAtom("Example List"),
        nl->StringAtom("..."),
        nl->StringAtom("Remarks")),
     nl->SixElemList(nl->StringAtom("-> DATA"),
        nl->StringAtom("jinfo"),
        nl->StringAtom("((height width coloured colorSpace numComponents"),
        nl->StringAtom(") ( 4 lists with( 256 real-values))(B64codPict))"),
        nl->StringAtom("((275 215 TRUE 1 1)( 4 lists (0.1..5.0)(xxx))"),
        nl->StringAtom("All Distributions have exact 256 values!"))));

/*      nl->StringAtom("((height width coloured colorSpace numComponents)"
                       "( (0.13...0.01)(0.00...0.5)(0.0...1.0)(5.0...0.0)))"),
        nl->StringAtom("((275 215 TRUE 1 1)((0.13...0.01)(0.1...0.0)"
                       "(0.0...0.0)(0.0...0.0)))"),
        nl->StringAtom("All Distributions have exact 256 values! Only for use"
                       " with oprator jreadinfo!")))); */
}


Word
CreateJINFO( const ListExpr typeInfo )
{
  return (SetWord( new JINFO( false, 0, 0, false, 0, 0, nl->TheEmptyList())));
}

void
DeleteJINFO( Word& w )
{
  if (w.addr == 0)
  {
    cerr << "\nerror: DeleteJINFO() received 0x00 as JINFO-address.\n";
    jpegAlgGlobalError = true;
  }
  else
    delete (JINFO *)w.addr;
  w.addr = 0;
}

void
CloseJINFO( Word& w )
{
  if (w.addr == 0)
  {
    cerr << "\nerror: CloseJINFO() received 0x00 as JINFO-address.\n";
    jpegAlgGlobalError = true;
  }
  else
    delete (JINFO *)w.addr;
  w.addr = 0;
}

Word
CloneJINFO( const Word& w )
{
  if (w.addr == 0)
  {
      cerr << "\nerror: CloneJINFO() received 0x00 as JINFO-address.\n";
      jpegAlgGlobalError = true;
      return SetWord(0);
  }
  else
    return SetWord( ((JINFO *)w.addr)->Clone() );
}

/*
6.4 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~jinfo~ does not have arguments, this is trivial.

*/
bool
CheckJINFO( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "jinfo" ));
}

/*
6.5 ~Cast~-function

*/
void* CastJINFO( void* addr )
{
  if (!addr)
  {
    cerr << "\nerror: CastJINFO() received 0x00 as JINFO-address.\n";
    jpegAlgGlobalError = true;
    return 0;
  }
  else
    return new (addr) JINFO;
}

/*
6.6 ~SizeOf~-function

*/
int
SizeOfJINFO()
{
  return sizeof(JINFO);
}

/*
6.7 Creation of the Type Constructor Instance

*/
TypeConstructor jinfo(
        "jinfo",                     //name
        JINFOProperty,               //property function describing signature
        OutJINFO,     InJINFO,       //Out and In functions
        0,            0,             //SaveToList and RestoreFromList functions
        CreateJINFO,  DeleteJINFO,   //object creation and deletion
        0, 0, CloseJINFO, CloneJINFO,//object open, save, and close
        CastJINFO,                   //cast function
        SizeOfJINFO,                 //SizeOf function
        CheckJINFO,                  //kind checking function
        0,                           //predef. pers. function for model
        TypeConstructor::DummyInModel,
        TypeConstructor::DummyOutModel,
        TypeConstructor::DummyValueToModel,
        TypeConstructor::DummyValueListToModel );


/*
6.8 Operator ~jreadinfo~

This is only to get the metadata of a jpegObject in the form of a NestedList.

4.3.1 Type Mapping of operator ~jreadinfo~

Operator ~jreadinfo~ accepts a jpeg-Object, reads the metadata and stores this
in a jinfo Object. This Object should/could be used from the viewer.
Further on, a stream-Object is accepted. The attrname of a jpeg-Attribute is
given from the user. For this attribute the jinfo-Object ist created and
passed as a stream of jinfo-Objects.

----    (jpeg)               -> jinfo
        (stream(x) attrname  -> stream (x)
----

*/

ListExpr
JRInfoTypeMap( ListExpr args )
{
  // single jpeg-Object to work with
  if ( nl->ListLength(args) == 2 )
  {
    ListExpr arg1, arg2;
    arg1 = nl->First(args);
    arg2 = nl->Second(args);

    if ( nl->IsAtom (arg1) &&
         nl->IsEqual(arg1, "jpeg") &&
         nl->IsAtom (arg2) )
      return nl->SymbolAtom("jinfo");

    if ( (nl->ListLength(arg1) == 2) &&
         (TypeOfRelAlgSymbol(nl->First(arg1)) == stream) &&
         (TypeOfRelAlgSymbol(nl->First(nl->Second(arg1))) == tuple ) &&
         (nl->IsAtom(arg2)) )
      {
        ListExpr attrtype;
        string attrname = nl->SymbolValue(arg2);
        cout << endl;
        int j = FindAttribute(nl->Second(nl->Second(arg1)),attrname,attrtype);
        if (j && nl->IsEqual (attrtype, "jpeg"))
        {
          ListExpr outList =
              (nl->ThreeElemList(
                   nl->SymbolAtom("APPEND"),
                   nl->OneElemList(nl->IntAtom(j)),
                   nl->TwoElemList (
                       nl->SymbolAtom("stream"),
                       nl->TwoElemList(
                           nl->SymbolAtom("tuple"),
                           nl->OneElemList(
                               nl->TwoElemList(nl->SymbolAtom("info"),
                                               nl->SymbolAtom("jinfo")))))));


          //nl->WriteListExpr (outList, cout); cout << endl;
          return outList;
        }
      }
  }

  return nl->SymbolAtom("typeerror");
}

/*
6.8.1 Selection function of overloaded operator ~jreadinfo~

*/

static int
JRInfoSelect (ListExpr args)
{
  ListExpr arg1 = nl->First (args);

  if ( nl->IsAtom(arg1) && nl->IsEqual(arg1, "jpeg") )
    return (0);
  if ( (nl->ListLength(arg1) == 2) && nl->IsEqual(nl->First(arg1), "stream") )
    return (1);

  return (-1); // This point should never be reached
}

/*
6.8.2 Value mapping functions of operator ~jreadinfo~

*/
int
JRInfoFunJ(Word* args, Word& result, int message, Word& local, Supplier s)
{
  JPEG* jpeg;
  JINFO* jinfo;

  jpeg = ((JPEG*)args[0].addr);
  if (!jpeg)
  {
    cerr << "\nerror: JRInfoFunJ(() received 0x00 as Jpeg-address.\n";
    result = SetWord(0);
    jpegAlgGlobalError = true;
    return 0;
  }

  jinfo = new JINFO (true,
                     (int) jpeg->GetHeight(),
                     (int) jpeg->GetWidth(),
                     jpeg->IsColoured(),
                     (int) jpeg->GetColorSpace(),
                     jpeg->Get_num_components(),
                     OutJPEG(nl->TheEmptyList(), SetWord(jpeg)));

  switch ( jpeg->GetColorSpace() )
  {
    case 0:          //grayscale
      for (int i = 0; i <= 255; i++)
         jinfo->SetcDistribution(i, 0, jpeg->GetBrightnessValue (i));
      break;
    case 1:          //RGB
      for (int comp = 0; comp <= 2; comp++)
        for (int idx = 0; idx <= 255; idx ++)
          jinfo->SetcDistribution(idx, comp, jpeg->GetRGBValue(comp, idx));
      break;
    default:
      jinfo->SetDefined (false);
      break;
  }

  result = qp->ResultStorage(s);
  *((JINFO*)result.addr)=*jinfo;
  return 0;
}

int
JRInfoFunS(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word elem, indexWord;
  Tuple* tu;
  JPEG* elemPtr;
  JINFO* jinfo;
  int index;

  switch (message)
  {
    case OPEN:
    {
      //cout << "OPEN" << endl;

      ListExpr resultType = GetTupleResultType(s);
      //cout << "resultType:"; nl->WriteListExpr (resultType, cout);
      //cout << endl;
      TupleType *tupleType = new TupleType (nl->Second (resultType) );
      local.addr = tupleType;

      qp->Open (args[0].addr);
      return (0);
    }
    case REQUEST:
    {
      //cout << "REQUEST" << endl;
      qp->Request (args[0].addr, elem);
      while (qp->Received(args[0].addr))
      {
        qp->Request(args[2].addr, indexWord);
        index = ((CcInt*)indexWord.addr)->GetIntval();
        //Could be: working with localInfo is more efficient !!

        tu = (Tuple*)elem.addr;
        elemPtr = (JPEG*) tu->GetAttribute(index -1);
        assert(elemPtr);
/*
InitializeJPEG() of JPEG is not allways necessary. E.g. it is not necessary for
accessing width, height, ... It is also not necessary for a simple 'query
reljpeg;' or ' ... consume'. InitializeJPEG() could be placed in CastJPEG but
this would be inefficient for many cases. But for GetBrightnessValue() e.g.,
which delegates filling of the distri-array to JPEGSourceMgr it is necessary.
Without, there is no loss of data (since FLOB resides in picture) but only
loss in operational, computational capabilities (scanning content of FLOB).

*/
        elemPtr->InitializeJPEG(); // JPEGSourceMgr should be destroyed again
        jinfo = new JINFO (true,
                           (int) elemPtr->GetHeight(),
                           (int) elemPtr->GetWidth(),
                           elemPtr->IsColoured(),
                           (int) elemPtr->GetColorSpace(),
                           elemPtr->Get_num_components(),
                           OutJPEG(nl->TheEmptyList(), SetWord(elemPtr)));

        switch ( elemPtr->GetColorSpace() )
        {
          case 0:                       //grayscale
          {
             for (int i = 0; i <= 255; i++)
              jinfo->SetcDistribution(i, 0, elemPtr->GetBrightnessValue (i));
            break;
          }
          case 1:                       //RGB
          {
            for (int comp = 0; comp <= 2; comp++)
              for (int idx = 0; idx <= 255; idx ++)
                jinfo->SetcDistribution(idx, comp, elemPtr->GetRGBValue(comp, idx));
            break;
          }
          default:
          {
            jinfo->SetDefined (false);
            break;
          }
        }

        // create the new tuple for result-stream
        TupleType *tupleType = (TupleType*)local.addr;
        Tuple *t = new Tuple (*tupleType, true); // true means: IsFree == true
        assert ( t->IsFree() );

        t->PutAttribute(0, (JINFO*)jinfo);

        tu->DeleteIfAllowed(); // delete the input-stream-element

        elemPtr->DropSrcMgr(); // // JPEGSourceMgr should be destroyed again

        result = SetWord(t);
        return YIELD;
      }
      return CANCEL;
    }
    case CLOSE:
    {
      //cout << "CLOSE" << endl;
      delete (TupleType*)local.addr;
      qp->Close(args[0].addr);
      return 0;
    }
  }

  return -1; // should never happen
}
/*

6.8.3 Value Mapping Vector for operator ~jreadinfo~

*/

ValueMapping JRInfoMap [] = {JRInfoFunJ,
                             JRInfoFunS };

/*

6.8.4 Cost Model Mapping Vector for operator ~jreadinfo~

*/

ModelMapping JRInfoModelMap[] = {JPEGNoModelMapping,
                                 JPEGNoModelMapping};

/*
6.8.5 Specification of operator ~jreadinfo~

*/
const string JRInfoSpec  =
 "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
   "( <text>(jpeg) -> (jinfo), (stream (x) string )-> stream (jinfo)</text--->"
     "<text>_ jreadinfo [_]</text--->"
     "<text>For use with viewer. Brings jpeg-(Meta)Data in a NestedList-Form."
     "</text--->"
     "<text>query jpegObject jreadinfo [dummy]; "
     "query jpegRelation feed jreadinfo [pict] consume;</text--->"
 ") )";

/*
6.8.6 Definition of operator ~demo~

*/
Operator jreadinfo (
        "jreadinfo",                 //name
        JRInfoSpec,                  //specification
        2,                           //Number of overloaded functions
        JRInfoMap,                   //value mapping
        JRInfoModelMap,        //dummy model mapping, defined in Algebra.h
        JRInfoSelect,                //trivial selection function
        JRInfoTypeMap                //type mapping
);


/*******************************************************************************
\rule {460 pt}{1 pt} \linebreak

*******************************************************************************/
/*******************************************************************************
7 Creating the Algebra  (class ~JPEGAlgebra~)

*******************************************************************************/

class JPEGAlgebra : public Algebra
{
 public:
  JPEGAlgebra() : Algebra()
  {
    AddTypeConstructor( &jpeg );
    AddTypeConstructor( &jinfo );

    jpeg.AssociateKind("DATA");
    jpeg.AssociateKind("FILE");
    jinfo.AssociateKind("DATA");

    AddOperator( &savejpegto );
    AddOperator( &show );
    AddOperator( &demo );
    AddOperator( &col_d_test );
    AddOperator( &cut );
    AddOperator( &tiles );
    AddOperator( &downsize );
    AddOperator( &jpegcompbrightdist);
    AddOperator( &jpegcomprgbdist);
    AddOperator( &jreadinfo );

    jpegAlgGlobalError = false;

    #ifdef DEBUGJPEG
    cout << "\nConstructor JPEGAlgebra. ";
    printStatistics();
    #endif
  }
  ~JPEGAlgebra() {

    cout << "\nDestructor JPEGAlgebra. ";
    printStatistics();

    if (jpegAlgGlobalError)
      cerr << "\n\nERROR: There has occured an error inside JPEGAlgebra:"
           <<   "\n       may be serious or not .... \n\n";
    else
      cout << "\n\nJPEGAlgebra worked without an error found,  :)))\n"
                  "(User errors not counted, nor SecondoSystem-faults)\n\n";
  };

  void printStatistics()
  {
    cout << "Number of JPEGs: "       << JPEG::GetNumAlive()
         << "\n( cast: "               << JPEG::castJPEGs
         << " cloned: "               << JPEG::clonedJPEGs
         << " created: "              << JPEG::createdJPEGs
         << " closed: "               << JPEG::closedJPEGs
         << " deleted: "              << JPEG::deletedJPEGs<< " )"
         << "\nNumber of SourceMgrs: "<< JPEGSourceMgr::GetNumAlive()
         << " of overall c'str'd: "   << JPEGSourceMgr::GetCounter()
                                      << endl;
  }

};

JPEGAlgebra jpegAlgebra;



/*******************************************************************************
\rule {460 pt}{1 pt}

*******************************************************************************/
/******************************************************************************
C o n n e c t i o n . . t o . . t h e . . S e c o n d o - S y s t e m

******************************************************************************/

/*
8 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime.

*/

extern "C"
Algebra*
InitializeJPEGAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&jpegAlgebra);
}

