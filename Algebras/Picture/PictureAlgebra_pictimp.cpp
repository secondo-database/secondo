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

March 2005, M. Spiekermann. Function strtupr renamed to xstrtupr
since a name conflict with /mingw/include/string.h must be resolved.

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

[TOC]

1 Introduction

See the documentation of ~PictureAlgebra.h~ for a general introduction to
the Picture algebra.

This module contains all methods required for ~Picture~
to represent a SECONDO ~picture~ object plus functions required by
SECONDO to use ~Picture~ plus basic SECONDO operators on ~picture~.

2 Includes and other preparations

*/


#include <unistd.h>
#include <fstream>
#include <ctype.h>
#include <string.h>

#include "Algebra.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "Symbols.h"
#include "NestedList.h"
#include "Base64.h"

#include "PictureAlgebra.h"
#include "JPEGPicture.h"
#include "ImageConverter.h"

#include "DateTime.h"
#include "LogMsg.h"
#include "../../Tools/Flob/Flob.h"
#include "StringUtils.h"

#include "Algebras/FText/FTextAlgebra.h"
#include "ListUtils.h"


#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <array>
#include <stdexcept>


using namespace std;

extern NestedList* nl;
extern QueryProcessor *qp;

using namespace datetime;

/*

Helper function to convert a string to upper case.

*/



static void xstrupr(char *str)
{
        size_t i,len;

        len = strlen(str);
        for (i=0; i < len; ++i)
        {
                str[i] = toupper(str[i]);
        }
}



/*

3 Implementation of class ~Picture~

Please note that other methods are located in other modules of this
algebra!

See the documentation of ~PictureAlgebra.h~ for details on the behaviour
of the methods implemented here.

3.1 Constructors and destructor

*/

Picture::Picture(string imgdataB64,
                 string fn,
                 string cat,
                 bool isp,
                 string dt,
                 bool autoPortrait) : Attribute(true),jpegData(0) {
    if (PA_DEBUG) cerr << "Picture::Picture()-1 called" << endl;

    strcpy(filename, fn.c_str());
    strcpy(category, cat.c_str());
    strcpy(date, dt.c_str());
    isPortrait = isp;
    SetDefined(true);

    if (PA_DEBUG) {
        cerr << "Picture::Picture()-1 imgdataBase64.length()="
             << imgdataB64.length()
             << endl;
        cerr << "Picture::Picture()-1 filename="
             << filename
             << endl;
        cerr << "Picture::Picture()-1 category="
             << category
             << endl;
        cerr << "Picture::Picture()-1 date="
             << date
             << endl;
        cerr << "Picture::Picture()-1 isPortrait="
             << isPortrait
             << endl;
    }

    Base64 b64;

    unsigned long size = b64.sizeDecoded(imgdataB64.size());
    char* imgdata = new char[size];
    unsigned long len = b64.decode(imgdataB64, imgdata);

    if (PA_DEBUG) {
        cerr << "Picture::Picture()-1 size=" << size << endl;
        cerr << "Picture::Picture()-1 len=" << len << endl;
    }

    assert( len <= size );

    STRING_T name;
    CImageConverter *converter;
    ImageType imgType;

    strcpy(name, filename);
    xstrupr(name);

    if (PA_DEBUG) cerr << "Picture::Picture() upper value " << name << endl;

    if (strstr(name, ".JPG")) {
        if (PA_DEBUG)
            cerr << "Picture::Picture() JPEG Format detected" << endl;

        imgType = IMAGE_JPEG;
    } else if (strstr(name, ".TGA")) {
        if (PA_DEBUG)
            cerr << "Picture::Picture() TGA Format detected" << endl;

        imgType = IMAGE_TGA;
    } else if (strstr(name, ".PCX")) {
        if (PA_DEBUG)
            cerr << "Picture::Picture() PCX Format detected" << endl;

        imgType = IMAGE_PCX;
    } else if (strstr(name, ".BMP")) {
        if (PA_DEBUG)
            cerr << "Picture::Picture() Bitmap Format detected" << endl;

        imgType = IMAGE_BMP;
    } else {
        if (PA_DEBUG)
            cerr << "Picture::Picture() unknown image format: "
                 << filename
                 << endl;

        delete[] imgdata;
        jpegData.clean();
        SetDefined(false);
        return;
    }

    if (imgType != IMAGE_JPEG) {
        converter =
            new CImageConverter(imgType,
                                (unsigned char*)imgdata,
                                (unsigned long)size);

        if (!converter->Defined()) {
            if (PA_DEBUG)
                cerr << "Picture::Picture() cannot read image: "
                     << filename
                     << endl;
            delete[] imgdata;
            jpegData.clean();
            SetDefined(false);
            return;
        } else {
            delete[] imgdata;

            // generate jpeg data
            imgdata = (char*)converter->GetJpegData(len);

            strcat(filename, ".JPG");

            if (PA_DEBUG)
                cerr << "Picture::Picture() image converted, size: "
                     << len
                     << endl;
        }

        delete converter;
    }

    jpegData.resize(len);
    jpegData.write(imgdata, len);

    //if (PA_DEBUG)
    //  cerr << "Picture::Picture()-1 jpegData="
    //       << (unsigned int) jpegData
    //       << endl;



    createHistograms(imgdata, len);
    if(autoPortrait){
      isPortrait = GetWidth() < GetHeight();
    }


    delete[] imgdata;

    if (PA_DEBUG) cerr << "Picture::Picture()-1 done" << endl;
}

Picture::Picture(char* imgdata,
                 unsigned long size,
                 string fn,
                 string cat,
                 bool isp,
                 string dt) : Attribute(true),jpegData(0) {
    if (PA_DEBUG) cerr << "Picture::Picture()-2 called" << endl;

    Set(imgdata, size, fn, cat, isp, dt);

    if (PA_DEBUG) cerr << "Picture::Picture()-2 done" << endl;
}

Picture::~Picture(void) {
    if (PA_DEBUG) cerr << "Picture::~Picture() called" << endl;

    //if (PA_DEBUG)
    //  cerr << "Picture::~Picture() jpegData="
    //       << (unsigned int) jpegData
    //       << endl;

    if (PA_DEBUG) cerr << "Picture::~Picture() done" << endl;
}

/*

3.2 Pre-calculate histograms

*/

void Picture::createHistograms(char* imgdata, unsigned long size) {
    if (PA_DEBUG) cerr << "Picture::createHistograms called" << endl;

    JPEGPicture rgb( (unsigned char *) imgdata, size );

    unsigned long int rgbSize;
    unsigned char * rgbData = rgb.GetImageData( rgbSize );

    double maxValue = 0.0;

    // store picture attributes
    width = JPEGPicture::GetWidth((unsigned char*)imgdata, size);
    height = JPEGPicture::GetHeight((unsigned char*)imgdata, size);
    isGrayscale = JPEGPicture::IsGrayScale((unsigned char*)imgdata, size);

    const unsigned int numOfPixels = rgbSize/3;
    if ( !( (3 * numOfPixels) == rgbSize ))
    {
      cerr << "height: " << height << endl;
      cerr << "width: " << width << endl;
      cerr << "rgbSize: " << rgbSize << endl;
    }
    //assert( numOfPixels == (height*width) );
    const double relFactor = 100.0 * 1.0 / (double)numOfPixels;

    // compute percentage values

    for (int i=HC_RED; i<=HC_BRIGHTNESS; i++)
    {
      histogram[i] = Histogram( rgbData, rgbSize, (HistogramChannel)i );
      if ( histogram[i].GetHistogramMaxValue() > maxValue )
          maxValue = histogram[i].GetHistogramMaxValue();
      histogram[i].SetHistogramMaxValue( maxValue );
      histogram[i].ScaleValues(relFactor);

      if (RTFlag::isActive("PA:histogramCheck")) {
        histogram[i].CheckSum((int)numOfPixels);
        histogram[i].CheckSum(100.0);
      }
    }

}

/*

3.3 Accessing image data

*/

string Picture::GetJPEGBase64Data(void) {
    if (PA_DEBUG) cerr << "Picture::GetJPEGBase64Data() called" << endl;

    //if (PA_DEBUG)
    //  cerr << "Picture::GetJPEGBase64Data() jpegData->Size()="
    //       << jpegData->Size()
    //       << endl;

    Base64 b64;

    unsigned long size;
    char* imgdata = GetJPEGData(size);

    string res;

    b64.encode(imgdata, size, res);

    delete [] imgdata;

    if (PA_DEBUG)
        cerr << "Picture::GetJPEGBase64Data() res.length()="
             << res.length()
             << endl;

    return res;
}

char* Picture::GetJPEGData(unsigned long& size) const {
    if (PA_DEBUG) cerr << "Picture::GetJPEGData() called" << endl;

    size = jpegData.getSize();
    char* buf = new char[size];
    jpegData.read(buf, size);
    return buf;
}

/*

3.4 Setting object value

*/

void Picture::Set(char* imgdata,
                  unsigned long size,
                  string fn,
                  string cat,
                  bool isp,
                  string dt) {

    if (PA_DEBUG) 
         cerr << "Picture::Set()-2 for " 
              << fn << " called" << endl;

    strcpy(filename, fn.c_str());
    strcpy(category, cat.c_str());
    strcpy(date, dt.c_str());
    isPortrait = isp;
    SetDefined(true);

    jpegData.resize(size);
    jpegData.write(imgdata, size);

    try {
       createHistograms(imgdata, size);
    } catch (std::runtime_error &e) {
       cerr << "Got exception: " << e.what() << endl;
       SetDefined(false);
    }

    if (PA_DEBUG) cerr << "Picture::Set()-2 done" << endl;
}

/*

3.5 ~Attribute~ implementation

*/

size_t Picture::HashValue(void) const {
    if (PA_DEBUG) cerr << "Picture::HashValue() called" << endl;

    if (!IsDefined()) return 0;

    unsigned long h = 0;

    string str = filename;
    str += category;
    str += date;
    const char *s = str.c_str();
    while (*s != 0) {
        h = 5*h+*s;
        s++;
    }

    h = 5*h+(isPortrait ? 1 : 0);

    unsigned long size;
    char* buf = GetJPEGData(size);
    for (unsigned int i = 0; i < size; i++) h = 5*h+buf[i];
    delete [] buf;

    return h;
}

void Picture::CopyFrom(const Attribute* attr) {
    if (PA_DEBUG) cerr << "Picture::CopyFrom() called" << endl;

    const Picture* p = (const Picture*) attr;

    // copy simple attributes

    jpegData.clean();
    SetDefined( p->IsDefined());

    if (IsDefined()) {
        isPortrait = p->isPortrait;
        strcpy(filename, p->filename);
        strcpy(category, p->category);
        strcpy(date, p->date);

        if (PA_DEBUG)
            cerr << "Picture::CopyFrom() filename" << p->filename << endl;

        jpegData.copyFrom(p->jpegData);
        memcpy((void*)histogram,(void*) p->histogram, sizeof(histogram));
    }
}

int Picture::Compare(const Attribute* a) const  {
    if (PA_DEBUG) cerr << "Picture::Compare() called" << endl;

    const Picture* p = (const Picture*) a;

    if (PA_DEBUG) {
        cerr << "Picture::Compare() filename=" << filename << endl;
        cerr << "Picture::Compare() p->filename=" << p->filename << endl;
    }

    int res = strcmp(filename, p->filename);
    if (res != 0) return res;

    if (PA_DEBUG) cerr << "Picture::Compare() filename equal" << endl;

    res = strcmp(category, p->category);
    if (res != 0) return res;

    if (PA_DEBUG) cerr << "Picture::Compare() category equal" << endl;

    res = strcmp(date, p->date);
    if (res != 0) return res;

    if (PA_DEBUG) cerr << "Picture::Compare() date equal" << endl;

    if (isPortrait && !p->isPortrait) return -1;
    if (!isPortrait && p->isPortrait) return 1;

    if (PA_DEBUG) cerr << "Picture::Compare() isPortrait equal" << endl;

    return SimpleCompare(a);
}

Picture* Picture::Clone(void) const {
    if (PA_DEBUG) cerr << "Picture::Clone() called" << endl;

    Picture* p = new Picture(0);

    if (PA_DEBUG) cerr << "Picture::Clone() address is " << (void*) p << endl;

    p->CopyFrom((const Attribute*) this);

    return p;
}

/*

3.6 Methods required for SECONDO operators

*/

bool Picture::Export(string filename) {
    if (PA_DEBUG) cerr << "Picture::Export() called" << endl;

    unsigned long size;
    char* buf = GetJPEGData(size);

    ofstream ofs(filename.c_str(), ios::out|ios::trunc|ios::binary);
    if (!ofs) {
        cerr << "could not create file '" << filename << "'" << endl;
        return false;
    }

    ofs.write(buf, size);
    ofs.close();
    delete[] buf;
    return true;
}

bool Picture::Display(void) {
    if (PA_DEBUG) cerr << "Picture::Display() called" << endl;

#ifndef SECONDO_WIN32

    static unsigned int fileCtr = 0;
    unsigned long size;
    char* buf = GetJPEGData(size);

    stringstream fileStr;
    fileStr << "/tmp/SECONDO.PictureAlgebra.";
    fileStr << ++fileCtr;

    char* filename = strdup(fileStr.str().c_str());
    int fd = mkstemp(filename);
    if (fd < 0) {
        cerr << "could not create temporary file '"
             << filename
             << "': "
             << endl;
        perror("mkstemp");
        free(filename);
        delete[] buf;
        return false;
    }

    if (PA_DEBUG) cerr << "Picture::Display() temp file " << filename << endl;

    unsigned int len = write(fd, buf, size);
    if (len != size) {
        cerr << "Picture::Display() could only partially write to temp file '"
             << filename
             << "'"
             << endl;
        unlink(filename);
        free(filename);
        delete[] buf;
        return false;
    }

    close(fd);

    string cmd = PROG_DISPLAY;
    cmd += " ";
    cmd += filename;


    if(system(cmd.c_str()) != 0){
       cerr << " problem in executing external command " << cmd << endl;
    }

    if (unlink(filename) < 0) {
        cerr << "Picture::Display() could not remove temporary file '"
             << filename
             << "': ";
        perror("unlink");
        free(filename);
        delete[] buf;
        return false;
    }

    free(filename);
    delete[] buf;

#else

  cerr << "Not yet implemented for win32 systems!" << endl;
#endif

    return true;
}

/*

3.7 Other methods

*/

int Picture::SimpleCompare(const Attribute* a) const {
    if (PA_DEBUG) cerr << "Picture::SimpleCompare() called" << endl;

    const Picture* p = (const Picture*) a;

    unsigned long size1;
    char* buf1 = GetJPEGData(size1);

    unsigned long size2;
    char* buf2 = p->GetJPEGData(size2);

    int size = size1 < size2 ? size1 : size2;

    int res = memcmp(buf1, buf2, size);

    delete [] buf1;
    delete [] buf2;

    if (res != 0)
        return res;
    else if (size1 < size2)
        return -1;
    else if (size1 > size2)
        return 1;
    else
        return 0;
}

/*

4 Nested list representation of class ~Picture~

*/

static ListExpr OutPicture(ListExpr typeInfo, Word value) {
    if (PA_DEBUG) cerr << "OutPicture() called" << endl;

    Picture* p = (Picture*) value.addr;

    if (PA_DEBUG) cerr << "OutPicture() #1 p=" << (void*) p << endl;
    if (PA_DEBUG) cerr << "OutPicture() filename=" << p->GetFilename() << endl;

    if (p->IsDefined()) {
        ListExpr imgdata = nl->TextAtom();
        nl->AppendText(imgdata, p->GetJPEGBase64Data());

        return
            nl->FiveElemList(
                nl->StringAtom(p->GetFilename()),
                nl->StringAtom(p->GetDate()),
                nl->StringAtom(p->GetCategory()),
                nl->BoolAtom(p->IsPortrait()),
                imgdata);
    } else
        return nl->SymbolAtom(Symbol::UNDEFINED());
}

static Word InPicture(const ListExpr typeInfo,
                      const ListExpr instance,
                      const int errorPos,
                      ListExpr& errorInfo,
                      bool& correct) {
    if (PA_DEBUG) cerr << "InPicture() called" << endl;

    if (nl->ListLength(instance) == 5) {
        if (PA_DEBUG) cerr << "InPicture() list length is correct" << endl;

        ListExpr filename = nl->First(instance);
        ListExpr date = nl->Second(instance);
        ListExpr category = nl->Third(instance);
        ListExpr isportrait = nl->Fourth(instance);
        ListExpr imgdata = nl->Fifth(instance);

        if(PA_DEBUG){
           cerr << nl->StringValue(filename) << endl;
           cerr << nl->StringValue(category) << endl;
           cerr << nl->StringValue(date) << endl;
        }

        if (nl->IsAtom(filename)
            && nl->AtomType(filename) == StringType
            && nl->IsAtom(date)
            && nl->AtomType(date) == StringType
            && nl->IsAtom(isportrait)
            && nl->AtomType(category) == StringType
            && nl->IsAtom(isportrait)
            && (   (nl->AtomType(isportrait) == BoolType)
                || nl->IsEqual(isportrait,"auto"))
            && nl->IsAtom(imgdata)
            && nl->AtomType(imgdata) == TextType) {
            if (PA_DEBUG)
                cerr << "InPicture() list elements are correct" << endl;

            DateTime dt(instanttype);

            if (dt.ReadFrom(nl->StringValue(date))) {
                if (PA_DEBUG)
                    cerr << "InPicture() date is correct" << endl;
                string imgdataBase64 = "";
                string current;
                TextScanInfo info;
                while (nl->GetNextText(imgdata, current, 72*1000+1000, info)) {
                    imgdataBase64 += current;
                    if (PA_DEBUG)
                        cerr << "InPicture() position="
                             << imgdataBase64.length()
                             << endl;
                }

                if (PA_DEBUG)
                    cerr << "InPicture() imgdataBase64.length()="
                         << imgdataBase64.length()
                         << endl;

                correct = true;
                bool isp = false;
                bool autoPortrait = false;
                if(nl->AtomType(isportrait)==BoolType){
                   isp = nl->BoolValue(isportrait);
                } else {
                   autoPortrait = true;
                }

                Picture* pic =
                    new Picture(imgdataBase64,
                                nl->StringValue(filename),
                                nl->StringValue(category),
                                isp,
                                nl->StringValue(date),
                                autoPortrait);
                if (PA_DEBUG)
                    cerr << "InPicture() created picture at "
                         << (void*) pic
                         << endl;
                return SetWord(pic);
            } else
                if (PA_DEBUG)
                    cerr << "InPicture() date is not correct" << endl;

        }
    }

    correct = false;
    return SetWord(Address(0));
}

/*

5 Description Signature Type Constructor of ~Picture~

*/

static ListExpr PictureProperty(void) {
    return
        nl->TwoElemList(
            nl->FiveElemList(
                nl->StringAtom("Signature"),
                nl->StringAtom("Example Type List"),
                nl->StringAtom("List Rep"),
                nl->StringAtom("Example List"),
                nl->StringAtom("Remarks")),
            nl->FiveElemList(
                nl->StringAtom("-> DATA"),
                nl->StringAtom(Picture::BasicType()),
                nl->StringAtom(
                    "(<file> <date> <category> <isportrait> <data> )"),
                nl->StringAtom("n/a"),
                nl->StringAtom("<date> is in instant format.")));
}

/*

6 Persistant storage of ~Picture~

*/

static Word CreatePicture(const ListExpr typeInfo) {
    if (PA_DEBUG) cerr << "CreatePicture() called" << endl;

    Picture* p = new Picture(false);
    if (PA_DEBUG) cerr << "CreatePicture() address is " << (void*) p << endl;
    return SetWord(p);
}

static void DeletePicture(const ListExpr typeInfo, Word& w) {
    if (PA_DEBUG)
        cerr << "DeletePicture() called for "
             << (void*) w.addr
             << endl;

    delete (Picture*) w.addr;
    if (PA_DEBUG) cerr << "DeletePicture() #2" << endl;
    w.addr = 0;
    if (PA_DEBUG) cerr << "DeletePicture() done" << endl;
}

static void ClosePicture(const ListExpr typeInfo, Word& w) {
    if (PA_DEBUG)
        cerr << "ClosePicture() called for "
             << (void*) w.addr
             << endl;

    delete (Picture*) w.addr;
    if (PA_DEBUG) cerr << "ClosePicture() #1" << endl;
    w.addr = 0;
    if (PA_DEBUG) cerr << "ClosePicture() done" << endl;
}

static Word ClonePicture(const ListExpr typeInfo, const Word& w) {
    if (PA_DEBUG) cerr << "ClonePicture() called" << endl;

    return SetWord(((Picture*) w.addr)->Clone());
}

static int SizeOfPicture(void) {
    if (PA_DEBUG) cerr << "SizeOfPicture() called" << endl;

    return sizeof(Picture);
}

static bool OpenPicture(SmiRecord& rec, size_t& offset,
                        const ListExpr typeInfo,
                        Word& w) {
    if (PA_DEBUG) cerr << "OpenPicture() called" << endl;

    return OpenAttribute<Picture>(rec, offset, typeInfo,w);
}

static bool SavePicture(SmiRecord& rec, size_t& offset,
                        const ListExpr typeInfo,
                        Word& w) {
    if (PA_DEBUG) cerr << "SavePicture() called" << endl;
    return SaveAttribute<Picture>(rec, offset, typeInfo, w);
}

/*

7 Kind checking of ~Picture~

*/

static bool CheckPicture(ListExpr type, ListExpr& errorInfo) {
    if (PA_DEBUG) cerr << "CheckPicture() called" << endl;
    return  nl->IsEqual(type, Picture::BasicType());
}

/*

8 Cast function of ~Picture~

*/

static void* CastPicture(void* addr) {
    if (PA_DEBUG)
        cerr << "CastPicture() called, addr="
             << (void*) addr
             << endl;

    return new (addr) Picture;
}

/*

9 Type constructor of ~Picture~

*/

TypeConstructor* picture = 0;

 void initPicture(){
  picture = new TypeConstructor (
    Picture::BasicType(),                                //name
    PictureProperty,                          //property function describing
                                              //  signature
    OutPicture, InPicture,                    //Out and In functions
    0, 0,                                     //SaveToList and RestoreFromList
                                              //  functions
    CreatePicture, DeletePicture,             //object creation and deletion
    OpenPicture, SavePicture,                 //object open and save
    ClosePicture, ClonePicture,               //object close and clone
    CastPicture,                              //cast function
    SizeOfPicture,                            //sizeof function
    CheckPicture                              //kind checking function
);
}


/*

10 Type mapping functions

*/

ListExpr PictureDateTypeMap(ListExpr args) {
    if (PA_DEBUG) cerr << "PictureDateTypeMap() called" << endl;

    if (nl->ListLength(args) == 1) {
        if (nl->IsEqual(nl->First(args), Picture::BasicType()))
            return nl->SymbolAtom(Instant::BasicType());
        else {
            string lexpr;
            nl->WriteToString(lexpr, nl->First(args));
            ErrorReporter::ReportError(
                "expected 'picture' argument but received '"+lexpr+"'");
        }
    } else
        ErrorReporter::ReportError(
            "expected one argument but received "
            +stringutils::int2str(nl->ListLength(args)));

    return nl->SymbolAtom(Symbol::TYPEERROR());
}



ListExpr PictureExportTypeMap(ListExpr args) {
    if (PA_DEBUG) cerr << "PictureExportTypeMap() called" << endl;

    string lexpr;

    if (nl->ListLength(args) == 2) {
        if (!nl->IsEqual(nl->First(args), Picture::BasicType())) {
            nl->WriteToString(lexpr, nl->First(args));
            ErrorReporter::ReportError(
                "expected 'picture' as first argument but received '"
                +lexpr
                +"'");
        } else if (!nl->IsEqual(nl->Second(args), FText::BasicType())) {
            nl->WriteToString(lexpr, nl->Second(args));
            ErrorReporter::ReportError(
                "expected 'FText' as second argument but received '"
                +lexpr
                +"'");
        } else
            return nl->SymbolAtom(CcBool::BasicType());
    } else
        ErrorReporter::ReportError(
            "expected two arguments but received "
            +stringutils::int2str(nl->ListLength(args)));

    return nl->SymbolAtom(Symbol::TYPEERROR());       
    
}






ListExpr PictureImportpictureTypeMap(ListExpr args) {
    if (PA_DEBUG) cerr << "PicturereadpictureTypeMap() called" << endl;

    string lexpr;

    if (nl->ListLength(args) == 1) {
        ListExpr first = nl->First(args);
        if (!FText::checkType(first)) {
            nl->WriteToString(lexpr, nl->First(args));
            ErrorReporter::ReportError(
                "expected text as first argument but received '"
                +lexpr
                +"'");
             return nl->SymbolAtom(Symbol::TYPEERROR());       
          }
         } 
         else { ErrorReporter::ReportError(
             "expected one argument but received "
             +stringutils::int2str(nl->ListLength(args)));
             return nl->SymbolAtom(Symbol::TYPEERROR());       
         
                
              }
            
         
      return nl->SymbolAtom(Picture::BasicType());
   
}










ListExpr PictureSimpleEqualsTypeMap(ListExpr args) {
    if (PA_DEBUG) cerr << "PictureSimpleEqualTypeMap() called" << endl;

    string lexpr;

    if (nl->ListLength(args) == 2) {
        if (!nl->IsEqual(nl->First(args), Picture::BasicType())) {
            nl->WriteToString(lexpr, nl->First(args));
            ErrorReporter::ReportError(
                "expected 'picture' as first argument but received '"
                +lexpr
                +"'");
        } else if (!nl->IsEqual(nl->Second(args), Picture::BasicType())) {
            nl->WriteToString(lexpr, nl->Second(args));
            ErrorReporter::ReportError(
                "expected 'picture' as second argument but received '"
                +lexpr
                +"'");
        } else
            return nl->SymbolAtom(CcBool::BasicType());
    } else
        ErrorReporter::ReportError(
            "expected two arguments but received "
            +stringutils::int2str(nl->ListLength(args)));

    return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*

11 Value mapping functions

*/

int PictureFilenameValueMap(Word* args,
                            Word& result,
                            int message,
                            Word& local,
                            Supplier s) {
    if (PA_DEBUG) cerr << "PictureFilenameValueMap() called" << endl;

    Picture* p = ((Picture*) args[0].addr);

    result = qp->ResultStorage(s);

    if (p->IsDefined())
        ((CcString*) result.addr)->Set(true,
                                       (STRING_T*) p->GetFilename().c_str());
    else
        ((CcString*) result.addr)->Set(false, (STRING_T*) "");

    return 0;
}

int PictureCategoryValueMap(Word* args,
                            Word& result,
                            int message,
                            Word& local,
                            Supplier s) {
    if (PA_DEBUG) cerr << "PictureCategoryValueMap() called" << endl;

    Picture* p = ((Picture*) args[0].addr);

    result = qp->ResultStorage(s);

    if (p->IsDefined())
        ((CcString*) result.addr)->Set(true,
                                       (STRING_T*) p->GetCategory().c_str());
    else
        ((CcString*) result.addr)->Set(false, (STRING_T*) "");

    return 0;
}

int PictureDateValueMap(Word* args,
                        Word& result,
                        int message,
                        Word& local,
                        Supplier s) {
    if (PA_DEBUG) cerr << "PictureCategoryValueMap() called" << endl;

    Picture* p = ((Picture*) args[0].addr);

    result = qp->ResultStorage(s);

    if (p->IsDefined())
        ((DateTime*) result.addr)->ReadFrom(p->GetDate());
    else
        ((DateTime*) result.addr)->ReadFrom("undefined");

    return 0;
}

int PictureIsPortraitValueMap(Word* args,
                              Word& result,
                              int message,
                              Word& local,
                              Supplier s) {
    if (PA_DEBUG) cerr << "PictureIsPortraitValueMap() called" << endl;

    Picture* p = ((Picture*) args[0].addr);

    result = qp->ResultStorage(s);

    if (p->IsDefined())
        ((CcBool*) result.addr)->Set(true, p->IsPortrait());
    else
        ((CcBool*) result.addr)->Set(false, false);

    return 0;
}

int PictureDisplayValueMap(Word* args,
                           Word& result,
                           int message,
                           Word& local,
                           Supplier s) {
    if (PA_DEBUG) cerr << "PictureDisplayValueMap() called" << endl;

    Picture* p = (Picture*) args[0].addr;

    if (PA_DEBUG)
        cerr << "PictureDisplayValueMap() filename"
             << p->GetFilename()
             << endl;

    result = qp->ResultStorage(s);

    if (p->IsDefined()) {
        ((CcBool*) result.addr)->Set(true, p->Display());
    } else
        ((CcBool*) result.addr)->Set(false, false);

    return 0;
}





int PictureExportValueMap(Word* args,
                          Word& result,
                          int message,
                          Word& local,
                          Supplier s) {
    if (PA_DEBUG) cerr << "PictureExportValueMap() called" << endl;

    Picture* p = (Picture*) args[0].addr;
    FText* str  = static_cast<FText*>(args[1].addr);
    string name = str->GetValue();
   
    result = qp->ResultStorage(s);

    if (p->IsDefined())
        ((CcBool*) result.addr)->Set(true,
                                     p->Export(name));
    else
        ((CcBool*) result.addr)->Set(false, false);

    return 0;
}







int PictureImportpictureValueMap(Word* args,
                          Word& result,
                          int message,
                          Word& local,
                          Supplier s) {
    if (PA_DEBUG) cerr << "PictureImportpictureValueMap() called" << endl;

    FText* str  = static_cast<FText*>(args[0].addr);
    result = qp->ResultStorage(s);
    Picture* pic =  static_cast<Picture*>( result.addr );
    bool portrait = false;
    string file;
    
    
    if(!str->IsDefined()){
        pic->SetDefined(false);
        return 0;
    }    
    
    
      
   string name = str->GetValue();
    
   ifstream  in (name.c_str(), ios::binary | ios::in); 
   
   if(!in){ 
      pic->SetDefined(false);   
      return 0;
   }
   
   in.seekg (0, in.end);
   size_t len = in.tellg();
   in.seekg (0, in.beg);     
  
   char* buffer = new char [len];   
   in.read (buffer,len);   
   in.close(); 
   
  
  std::array<char, 64> buf;
  buf.fill(0);
  time_t rawtime;
  time(&rawtime);
  const auto timeinfo = localtime(&rawtime);
  strftime(buf.data(), sizeof(buf), "%Y-%m-%d-%H:%M:%S", timeinfo);
  std::string timeStr(buf.data());
  
  
  pic->Set(buffer,len, "unknown","unknown",false,timeStr); 
  
  int height =  (int) pic->GetHeight();
  int width =  (int) pic->GetWidth();
  
  
  
  
  if (height > width)
    {
      portrait = true;
    } 
    
   
   
   
  size_t count1 = std::count(name.begin(), name.end(), '/');
  
  if (count1 == 0)
   {           
        size_t end = name.find_last_of('/');
        size_t lens = 0;
        file  = name.substr(lens, end);	
        
       
        
   }   
   
  else { 
        size_t begin = name.find_first_of('/');
        size_t end = name.find_last_of('/');
        size_t lens = end - begin;
        file  = name.substr(lens+1,end);	
       }
    
  
   
    pic->Set(buffer,len, file,"unknown",portrait,timeStr); 
    
    
   
    
   
    delete[] buffer;
    
     
    return 0;
}

int PictureSimpleEqualsValueMap(Word* args,
                                Word& result,
                                int message,
                                Word& local,
                                Supplier s) {
    if (PA_DEBUG) cerr << "PictureSimpleEqualsValueMap() called" << endl;

    Picture* p1 = (Picture*) args[0].addr;
    Picture* p2 = (Picture*) args[1].addr;

    result = qp->ResultStorage(s);

    if (p1->IsDefined() && p2->IsDefined())
        ((CcBool*) result.addr)->Set(true,
                                     p1->SimpleCompare((Attribute*) p2) == 0);
    else
        ((CcBool*) result.addr)->Set(false, false);

    return 0;
}


Picture1024* Picture::ConvertToPicture1024() {
  Picture1024 *result = 0;
  Scale(result, 32, 32);
  return result;
}

/*
9 Implementation of class ~Picture1024~

*/
Picture1024::Picture1024(std::string imgdataB64, std::string fn,
                         std::string cat, bool isp, std::string dt,
                         bool autoPortrait /* = false */) :
  Picture(imgdataB64, fn, cat, isp, dt, autoPortrait) {
  Scale(this, 32, 32);
}

Picture1024::Picture1024(char* imgdata, unsigned long size, std::string fn,
                         std::string cat, bool isp, std::string dt) :
  Picture(imgdata, size, fn, cat, isp, dt) {
  Scale(this, 32, 32);
}

void Picture1024::Scale(Picture1024 *pic, int w, int h) {
  cout << "size remains 32 x 32 for this data type" << endl;
  pic = (Picture1024*)(this->Clone());
}

double Picture1024::DistanceRGB(const Picture1024& pic) const {
  cout << "start DistanceRGB" << endl;
  unsigned long size1;
  char* jpegData1 = GetJPEGData(size1);
  JPEGPicture *rgb1 = new JPEGPicture((unsigned char*)jpegData1, size1);
  unsigned long int rgbSize1;
  unsigned char *rgbData1 = rgb1->GetImageData(rgbSize1);
  cout << "  getImageData ok for 1" << endl;
  assert(rgbSize1 == 3072);

  unsigned long size2;
  char* jpegData2 = pic.GetJPEGData(size2);
  cout << "GetJPEGData successful for 2" << endl;
  JPEGPicture *rgb2 = new JPEGPicture((unsigned char*)jpegData2, size2);
  cout << "  JPEGPicture created for 2" << endl;
  unsigned long int rgbSize2;
  unsigned char *rgbData2 = rgb2->GetImageData(rgbSize2);
  cout << "  getImageData ok for 2" << endl;
  assert(rgbSize2 == 3072);
  double result = 0.0;
  for (unsigned int i = 0; i < rgbSize1; i++) {
    // cout << "[" << (int)rgbData1[i] << " " << (int)rgbData2[i] << "] ";
    result += abs((int)rgbData1[i] - (int)rgbData2[i]);
  }

  delete rgb1;
  delete[] jpegData1;
  delete rgb2;
  delete[] jpegData2;
  cout << " ... return result" << endl;
  return result;
}

const bool Picture1024::checkType(const ListExpr type) {
  return listutils::isSymbol(type, BasicType());
}

static ListExpr Picture1024Property(void) {
  return nl->TwoElemList(
           nl->FiveElemList(
             nl->StringAtom("Signature"),
             nl->StringAtom("Example Type List"),
             nl->StringAtom("List Rep"),
             nl->StringAtom("Example List"),
             nl->StringAtom("Remarks")),
           nl->FiveElemList(
             nl->StringAtom("-> DATA"),
             nl->StringAtom(Picture1024::BasicType()),
             nl->StringAtom("(<file> <date> <category> <isportrait> <data> )"),
             nl->StringAtom("n/a"),
             nl->StringAtom("<date> is in instant format.")));
}

static bool CheckPicture1024(ListExpr type, ListExpr& errorInfo) {
  if (PA_DEBUG) {
    cerr << "CheckPicture1024() called" << endl;
  }
  return nl->IsEqual(type, Picture1024::BasicType());
}

/*

9 Type constructor of ~Picture1024~

*/

TypeConstructor* picture1024 = 0;

void initPicture1024() {
  picture1024 = new TypeConstructor (
    Picture1024::BasicType(),         //name
    Picture1024Property,              //property function describing signature
    OutPicture, InPicture,            //Out and In functions
    0, 0,                             //SaveToList and RestoreFromList functions
    CreatePicture, DeletePicture,     //object creation and deletion
    OpenPicture, SavePicture,         //object open and save
    ClosePicture, ClonePicture,       //object close and clone
    CastPicture,                      //cast function
    SizeOfPicture,                    //sizeof function
    CheckPicture1024                  //kind checking function
  );
}
