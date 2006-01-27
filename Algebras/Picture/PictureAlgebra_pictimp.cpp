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

using namespace std;

#include <unistd.h>
#include <fstream>
#include <ctype.h>
#include <string.h>

#include "Algebra.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "NestedList.h"
#include "Base64.h"

#include "PictureAlgebra.h"
#include "JPEGPicture.h"
#include "ImageConverter.h"

extern NestedList* nl;
extern QueryProcessor *qp;

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
		 string dt) {
    if (PA_DEBUG) cerr << "Picture::Picture()-1 called" << endl;

    strcpy(filename, fn.c_str());
    strcpy(category, cat.c_str());
    strcpy(date, dt.c_str());
    isPortrait = isp;
    isDefined = true;

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

    STRING name;
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
	jpegData = 0;
	isDefined = false;
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
	    jpegData = 0;
	    isDefined = false;
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

    //jpegData = new FLOB(size);
    jpegData = 0;
    jpegData.Resize(len);
    jpegData.Put(0, len, imgdata);

    //if (PA_DEBUG)
    //	cerr << "Picture::Picture()-1 jpegData="
    //	     << (unsigned int) jpegData
    //	     << endl;

    createHistograms(imgdata, len);

    delete[] imgdata;

    if (PA_DEBUG) cerr << "Picture::Picture()-1 done" << endl;
}

Picture::Picture(char* imgdata,
		 unsigned long size,
		 string fn,
		 string cat,
		 bool isp,
		 string dt) {
    if (PA_DEBUG) cerr << "Picture::Picture()-2 called" << endl;

    Set(imgdata, size, fn, cat, isp, dt);

    if (PA_DEBUG) cerr << "Picture::Picture()-2 done" << endl;
}

Picture::~Picture(void) {
    if (PA_DEBUG) cerr << "Picture::~Picture() called" << endl;

    //if (PA_DEBUG) 
    //	cerr << "Picture::~Picture() jpegData=" 
    //	     << (unsigned int) jpegData 
    //	     << endl;

    //if (jpegData) delete jpegData;

    //jpegData = 0;

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

    histogram[HC_RED] = Histogram( rgbData, rgbSize, HC_RED ); 	
    if ( histogram[HC_RED].GetHistogramMaxValue() > maxValue )
	maxValue = histogram[HC_RED].GetHistogramMaxValue();

    histogram[HC_GREEN] = Histogram( rgbData, rgbSize, HC_GREEN ); 	
    if ( histogram[HC_GREEN].GetHistogramMaxValue() > maxValue )
	maxValue = histogram[HC_GREEN].GetHistogramMaxValue();

    histogram[HC_BLUE] = Histogram( rgbData, rgbSize, HC_BLUE ); 	
    if ( histogram[HC_BLUE].GetHistogramMaxValue() > maxValue )
	maxValue = histogram[HC_BLUE].GetHistogramMaxValue();

    histogram[HC_BRIGHTNESS] = Histogram( rgbData, rgbSize, HC_BRIGHTNESS );
    if ( histogram[HC_BRIGHTNESS].GetHistogramMaxValue() > maxValue )
	maxValue = histogram[HC_BRIGHTNESS].GetHistogramMaxValue();

    histogram[HC_RED].SetHistogramMaxValue( maxValue );
    histogram[HC_GREEN].SetHistogramMaxValue( maxValue );
    histogram[HC_BLUE].SetHistogramMaxValue( maxValue );
    histogram[HC_BRIGHTNESS].SetHistogramMaxValue( maxValue );
}

/*

3.3 Accessing image data

*/

string Picture::GetJPEGBase64Data(void) {
    if (PA_DEBUG) cerr << "Picture::GetJPEGBase64Data() called" << endl;

    //if (PA_DEBUG) 
    //	cerr << "Picture::GetJPEGBase64Data() jpegData->Size()=" 
    //	     << jpegData->Size() 
    //	     << endl;

    Base64 b64;

    unsigned long size;
    const char* imgdata = GetJPEGData(size);

    string res;

    b64.encode(imgdata, size, res);

    if (PA_DEBUG) 
	cerr << "Picture::GetJPEGBase64Data() res.length()=" 
	     << res.length() 
	     << endl;

    return res;
}

const char* Picture::GetJPEGData(unsigned long& size) const {
    if (PA_DEBUG) cerr << "Picture::GetJPEGData() called" << endl;

    size = jpegData.Size();
    const char* buf;
    jpegData.Get(0, &buf);
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
    if (PA_DEBUG) cerr << "Picture::Set()-2 called" << endl;

    strcpy(filename, fn.c_str());
    strcpy(category, cat.c_str());
    strcpy(date, dt.c_str());
    isPortrait = isp;
    isDefined = true;

    //jpegData = new FLOB(size);
    jpegData = 0;
    jpegData.Resize(size);
    jpegData.Put(0, size, imgdata);

    createHistograms(imgdata, size);

    if (PA_DEBUG) cerr << "Picture::Set()-2 done" << endl;
}

/*

3.5 ~StandardAttribute~ implementation

*/

size_t Picture::HashValue(void) const {
    if (PA_DEBUG) cerr << "Picture::HashValue() called" << endl;

    if (!isDefined) return 0;

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
    const char* buf = GetJPEGData(size);
    for (unsigned int i = 0; i < size; i++) h = 5*h+buf[i];

    return h;
}

void Picture::CopyFrom(const StandardAttribute* attr) {
    if (PA_DEBUG) cerr << "Picture::CopyFrom() called" << endl;

    const Picture* p = (const Picture*) attr;

    // copy simple attributes

    jpegData = 0;
    isDefined = p->isDefined;

    if (isDefined) {
	isPortrait = p->isPortrait;
	strcpy(filename, p->filename);
	strcpy(category, p->category);
	strcpy(date, p->date);

	if (PA_DEBUG) 
	    cerr << "Picture::CopyFrom() filename" << p->filename << endl;
    
	unsigned long size;
	const char* buf = p->GetJPEGData(size);
    
	jpegData.Resize(size);
	jpegData.Put(0, size, buf);

	memcpy(histogram, p->histogram, sizeof(histogram));
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

    Picture* p = new Picture;

    if (PA_DEBUG) cerr << "Picture::Clone() address is " << (int) p << endl;

    p->CopyFrom((const StandardAttribute*) this);

    return p;
}

/*

3.6 Methods required for SECONDO operators

*/

bool Picture::Export(string filename) {
    if (PA_DEBUG) cerr << "Picture::Export() called" << endl;

    unsigned long size;
    const char* buf = GetJPEGData(size);

    ofstream ofs(filename.c_str(), ios::out|ios::trunc|ios::binary);
    if (!ofs) {
	cerr << "could not create file '" << filename << "'" << endl;
	return false;
    }

    ofs.write(buf, size);
    ofs.close();

    return true;
}

bool Picture::Display(void) {
    if (PA_DEBUG) cerr << "Picture::Display() called" << endl;

#ifndef SECONDO_WIN32 

    static unsigned int fileCtr = 0;
    unsigned long size;
    const char* buf = GetJPEGData(size);

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
	return false;
    }

    if (PA_DEBUG) cerr << "Picture::Display() temp file " << filename << endl;

    unsigned int len = write(fd, buf, size);
    if (len < 0) {
	cerr << "Picture::Display() could not write to temp file '"
	     << filename
	     << "': ";
	perror("write");
	unlink(filename);
	return false;
    } else if (len != size) {
	cerr << "Picture::Display() could only partially write to temp file '"
	     << filename
	     << "'"
	     << endl;
	unlink(filename);
	return false;
    }
    
    close(fd);

    string cmd = PROG_DISPLAY;
    cmd += " ";
    cmd += filename;

    cerr << "Picture::Display() display command is '" << cmd << "'" << endl;

    system(cmd.c_str());

    if (unlink(filename) < 0) {
	cerr << "Picture::Display() could not remove temporary file '"
	     << filename
	     << "': ";
	perror("unlink");
	return false;
    }

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
    const char* buf1 = GetJPEGData(size1);

    unsigned long size2;
    const char* buf2 = p->GetJPEGData(size2);

    int size = size1 < size2 ? size1 : size2;

    int res = memcmp(buf1, buf2, size);

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

    if (PA_DEBUG) cerr << "OutPicture() #1 p=" << (unsigned int) p << endl;
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
	return nl->SymbolAtom("undef");
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

	cerr << nl->StringValue(filename) << endl;
	cerr << nl->StringValue(category) << endl;
	cerr << nl->StringValue(date) << endl;

	if (nl->IsAtom(filename)
	    && nl->AtomType(filename) == StringType
	    && nl->IsAtom(date)
	    && nl->AtomType(date) == StringType
	    && nl->IsAtom(isportrait)
	    && nl->AtomType(category) == StringType
	    && nl->IsAtom(isportrait)
	    && nl->AtomType(isportrait) == BoolType
	    && nl->IsAtom(imgdata)
	    && nl->AtomType(imgdata) == TextType) {
	    if (PA_DEBUG) 
		cerr << "InPicture() list elements are correct" << endl;

	    DateTime dt;

	    if (dt.ReadFrom(nl->StringValue(date))) {
		if (PA_DEBUG) 
		    cerr << "InPicture() date is correct" << endl;
		string imgdataBase64 = "";
		string current;
		while (nl->GetNextText(imgdata, current, 72*1000+1000)) {
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
		Picture* pic =
		    new Picture(imgdataBase64,
				nl->StringValue(filename),
				nl->StringValue(category),
				nl->BoolValue(isportrait),
				nl->StringValue(date));
		if (PA_DEBUG) 
		    cerr << "InPicture() created picture at " 
			 << (int) pic 
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
		nl->StringAtom("picture"),
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
    if (PA_DEBUG) cerr << "CreatePicture() address is " << (int) p << endl;
    return SetWord(p);
}

static void DeletePicture(const ListExpr typeInfo, Word& w) {
    if (PA_DEBUG) 
	cerr << "DeletePicture() called for " 
	     << (int) w.addr 
	     << endl;

    delete (Picture*) w.addr;
    if (PA_DEBUG) cerr << "DeletePicture() #2" << endl;
    w.addr = 0;
    if (PA_DEBUG) cerr << "DeletePicture() done" << endl;
}

static void ClosePicture(const ListExpr typeInfo, Word& w) {
    if (PA_DEBUG) 
	cerr << "ClosePicture() called for " 
	     << (int) w.addr 
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

    bool isDefined;
    bool isPortrait;
    STRING filename;
    STRING category;
    STRING date;

    unsigned int pos = 0;

    if (rec.Read(&isDefined, sizeof(bool), pos) != sizeof(bool)) {
	cerr << "OpenPicture() could not read defined flag" << endl;
	return false;
    }
    if (PA_DEBUG) cerr << "OpenPicture() isDefined=" << isDefined << endl;

    if (!isDefined) {
	w.addr = new Picture(false);
	return true;
    }

    pos += sizeof(bool);

    if (rec.Read(filename, sizeof(STRING), pos) != sizeof(STRING)) {
	cerr << "OpenPicture() could not read filename" << endl;
	return false;
    }
    pos += sizeof(STRING);
    if (PA_DEBUG) cerr << "OpenPicture() filename=" << filename << endl;

    if (rec.Read(category, sizeof(STRING), pos) != sizeof(STRING)) {
	cerr << "OpenPicture() could not read category" << endl;
	return false;
    }
    pos += sizeof(STRING);
    if (PA_DEBUG) cerr << "OpenPicture() category=" << category << endl;

    if (rec.Read(date, sizeof(STRING), pos) != sizeof(STRING)) {
	cerr << "OpenPicture() could not read date" << endl;
	return false;
    }
    pos += sizeof(STRING);
    if (PA_DEBUG) cerr << "OpenPicture() date=" << date << endl;

    if (rec.Read(&isPortrait, sizeof(bool), pos) != sizeof(bool)) {
	cerr << "OpenPicture() could not read portrait flag" << endl;
	return false;
    }
    pos += sizeof(bool);
    if (PA_DEBUG) cerr << "OpenPicture() isPortrait=" << isPortrait << endl;

    unsigned int jpegSize;

    if (rec.Read(&jpegSize, sizeof(unsigned int), pos) 
	!= sizeof(unsigned int)) {
	cerr << "OpenPicture() could not read JPEG data size" << endl;
	return false;
    }
    pos += sizeof(unsigned int);
    if (PA_DEBUG) cerr << "OpenPicture() jpegSize=" << jpegSize << endl;

    char* jpegData = new char[jpegSize];

    if (rec.Read(jpegData, jpegSize, pos) != jpegSize) {
	cerr << "OpenPicture() could not read JPEG data" << endl;
	return false;
    }

    w.addr = 
	new Picture(jpegData, jpegSize, filename, category, isPortrait, date);
	   
    delete[] jpegData;

    return true;
}

static bool SavePicture(SmiRecord& rec, size_t& offset,
			const ListExpr typeInfo,
			Word& w) {
    if (PA_DEBUG) cerr << "SavePicture() called" << endl;

    Picture* p = (Picture*) w.addr;

    bool isDefined = p->IsDefined();

    unsigned int pos = 0;

    if (rec.Write(&isDefined, sizeof(bool), pos) != sizeof(bool)) {
	cerr << "SavePicture() could not write defined flag of " 
	     << (int) p 
	     << endl;
	return false;
    }

    if (!isDefined) return true;

    pos += sizeof(bool);

    if (rec.Write(p->GetFilename().c_str(), sizeof(STRING), pos) 
	!= sizeof(STRING)) {
	cerr << "SavePicture() could not write filename of " 
	     << (int) p 
	     << endl;
	return false;
    }
    pos += sizeof(STRING);
    if (rec.Write(p->GetCategory().c_str(), sizeof(STRING), pos) 
	!= sizeof(STRING)) {
	cerr << "SavePicture() could not write category of " 
	     << (int) p 
	     << endl;
	return false;
    }
    pos += sizeof(STRING);
    if (rec.Write(p->GetDate().c_str(), sizeof(STRING), pos) 
	!= sizeof(STRING)) {
	cerr << "SavePicture() could not write date of " 
	     << (int) p 
	     << endl;
	return false;
    }
    pos += sizeof(STRING);

    bool isPortrait = p->IsPortrait();

    if (rec.Write(&isPortrait, sizeof(bool), pos) != sizeof(bool)) {
	cerr << "SavePicture() could not write portrait flag of " 
	     << (int) p 
	     << endl;
	return false;
    }
    pos += sizeof(bool);

    unsigned long jpegSize;
    const char* jpegData = p->GetJPEGData(jpegSize);

    if (rec.Write(&jpegSize, sizeof(unsigned int), pos) 
	!= sizeof(unsigned int)) {
	cerr << "SavePicture() could not write JPEG data size of " 
	     << (int) p 
	     << endl;
	return false;
    }
    pos += sizeof(unsigned int);

    if (rec.Write(jpegData, jpegSize, pos) != jpegSize) {
	cerr << "SavePicture() could not write JPEG data of " 
	     << (int) p 
	     << endl;
	return false;
    }

    return true;
}

/*

7 Kind checking of ~Picture~

*/

static bool CheckPicture(ListExpr type, ListExpr& errorInfo) {
    if (PA_DEBUG) cerr << "CheckPicture() called" << endl;

    return nl->IsEqual(type, "picture");
}

/*

8 Cast function of ~Picture~

*/

static void* CastPicture(void* addr) {
    if (PA_DEBUG) 
	cerr << "CastPicture() called, addr=" 
	     << (unsigned int) addr
	     << endl;

    return new (addr) Picture;
}

/*

9 Type constructor of ~Picture~

*/

TypeConstructor picture(
    "picture",                                //name
    PictureProperty,                          //property function describing 
                                              //  signature
    OutPicture, InPicture,                    //Out and In functions
    0, 0,                                     //SaveToList and RestoreFromList 
                                              //  functions
    CreatePicture, DeletePicture,             //object creation and deletion
//    0, 0,                                     //object open and save
    OpenPicture, SavePicture,                 //object open and save
    ClosePicture, ClonePicture,               //object close and clone
    CastPicture,                              //cast function
    SizeOfPicture,                            //sizeof function
    CheckPicture                              //kind checking function
);
/*

10 Type mapping functions

*/

ListExpr PictureDateTypeMap(ListExpr args) {
    if (PA_DEBUG) cerr << "PictureDateTypeMap() called" << endl;

    if (nl->ListLength(args) == 1) {
	if (nl->IsEqual(nl->First(args), "picture"))
	    return nl->SymbolAtom("instant");
	else {
	    string lexpr;
	    nl->WriteToString(lexpr, nl->First(args));
	    ErrorReporter::ReportError(
		"expected 'picture' argument but received '"+lexpr+"'");
	}
    } else 
	ErrorReporter::ReportError(
	    "expected one argument but received "
	    +nl->ListLength(args));

    return nl->SymbolAtom("typeerror");
}

ListExpr PictureExportTypeMap(ListExpr args) {
    if (PA_DEBUG) cerr << "PictureExportTypeMap() called" << endl;

    string lexpr;

    if (nl->ListLength(args) == 2) {
	if (!nl->IsEqual(nl->First(args), "picture")) {
	    nl->WriteToString(lexpr, nl->First(args));
	    ErrorReporter::ReportError(
		"expected 'picture' as first argument but received '"
		+lexpr
		+"'");
	} else if (!nl->IsEqual(nl->Second(args), "string")) {
	    nl->WriteToString(lexpr, nl->Second(args));
	    ErrorReporter::ReportError(
		"expected 'string' as second argument but received '"
		+lexpr
		+"'");
	} else 
	    return nl->SymbolAtom("bool");
    } else
	ErrorReporter::ReportError(
	    "expected two arguments but received "
	    +nl->ListLength(args));

    return nl->SymbolAtom("typeerror");
}

ListExpr PictureSimpleEqualsTypeMap(ListExpr args) {
    if (PA_DEBUG) cerr << "PictureSimpleEqualTypeMap() called" << endl;

    string lexpr;

    if (nl->ListLength(args) == 2) {
	if (!nl->IsEqual(nl->First(args), "picture")) {
	    nl->WriteToString(lexpr, nl->First(args));
	    ErrorReporter::ReportError(
		"expected 'picture' as first argument but received '"
		+lexpr
		+"'");
        } else if (!nl->IsEqual(nl->Second(args), "picture")) {
	    nl->WriteToString(lexpr, nl->Second(args));
	    ErrorReporter::ReportError(
		"expected 'string' as second argument but received '"
		+lexpr
		+"'");
	} else 
	    return nl->SymbolAtom("bool");
    } else
	ErrorReporter::ReportError(
	    "expected two arguments but received "
	    +nl->ListLength(args));

    return nl->SymbolAtom("typeerror");
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
				       (STRING*) p->GetFilename().c_str());
    else
	((CcString*) result.addr)->Set(false, (STRING*) "");

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
				       (STRING*) p->GetCategory().c_str());
    else
	((CcString*) result.addr)->Set(false, (STRING*) "");

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
    CcString* str = (CcString*) args[1].addr;

    result = qp->ResultStorage(s);

    if (p->IsDefined())
	((CcBool*) result.addr)->Set(true, 
				     p->Export((char*) str->GetStringval()));
    else
	((CcBool*) result.addr)->Set(false, false);

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

