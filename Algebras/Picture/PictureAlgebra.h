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

The Picture algebra for SECONDO provides operations to store and
manipulate images in JPEG format. It consists of two data types. A ~picture~ 
object represents a single JPEG image and allows to scale, cut, mirror, rotate
and compare images. ~histogram~ represents the histogram of a ~picture~ object
and is created from a ~picture~ object.

The implementation of the Picture algebra is spread over multiple modules,
both to structure the code in a useful way and to enable the team to work
in parallel without causing frequent conflicts.

  * ~PictureAlgebra.h~ (this file): Contains class definitions.

  * ~PictureAlgebra\_base.cpp~: Contains all code related to creating 
    the Picture algebra itself. Code relating to the implementation of
    SECONDO types ~picture~ and ~histogram~ are located in other modules, as 
    shown below.

  * ~PictureAlgebra\_pictimp.cpp~: All methods required for ~Picture~
    to represent a SECONDO ~picture~ object plus functions required by
    SECONDO to use ~Picture~ plus basic SECONDO operators on ~picture~.

  * ~PictureAlgebra\_attrops.cpp~: SECONDO operators on ~picture~, which
    deal with basic attributes such as image heigh or width.

  * ~PictureAlgebra\_graphops.cpp~: SECONDO operators on ~picture~, which
    perform graphical operations such as scaling an image.

  * ~PictureAlgebra\_histimp.cpp~: All methods required for 
    ~Histogram~ to represent a SECONDO ~histogram~ object plus functions.
    required by SECONDO to use ~Histogram~.

  * ~PictureAlgebra\_histops.cpp~: SECONDO operators on ~picture~, which
    implicitely use ~histogram~.

  * ~JPEGPicture.cpp~: The class ~JPEGPicture~ is used to compress and
    decompress JPEG images and to examine specific JPEG image attributes.

  * ~JPEGMemManager.cpp~: ~libjpeg~ memory manager, as required by
    ~JPEGPicture.cpp~.

2 General definitions

If ~PA\_DEBUG~ is set to ~true~, verbose debug output is generated. If you
are running SECONDO in client/server mode, note that the output is shown in 
the server.

~PROG\_DISPLAY~ is the command used by the ~display~ operator to display
JPEG images. We suggest to use the ~display~ program from ImageMagick.

*/

#ifndef __PICTURE_ALGEBRA_H__
#define __PICTURE_ALGEBRA_H__ 

#include "DateTime.h"
#include "StandardAttribute.h"
#include "StandardTypes.h"

const bool PA_DEBUG = false;
const string PROG_DISPLAY = "/usr/bin/display";

/*

3 Class ~Histogram~

As class ~Picture~ is used in class ~Histogram~ although being defined later
in this file, it must be declared abead here.

*/

class Picture;

/*

With the enum ~HistogramChannel~, applications can specify which 
color channel should be used to create the histogram. For ~HC\_BRIGHTNESS~,
the brightness (or luminance) $Y$ of each pixel is calculated with 
$Y = 0.3 \cdot R + 0.59 \cdot G + 0.11 \cdot B$, where $R$, $G$ and $B$
are the red, green and blue components of the pixel.

*/

enum HistogramChannel { HC_RED, HC_GREEN, HC_BLUE, HC_BRIGHTNESS };

/*

As the class ~Picture~ implements ~StandardAttribute~, ~picture~ objects
can be used in relations.

*/

class Histogram : StandardAttribute {

/*

3.1 Private attributes

~histogram~ contains the percentage of the pixels in the specified channel
(see below) with a specific brightness; each of the 256 possible brightness
levels is represented by one array element.

~channel~ specifies the channel, which has been used to create this object.
See the description of ~HistogramChannel~ for details.

~histogramMaxValue~ contains the maximum value of all four histograms of 
the original image. See private attribute ~histogram~ of class ~Picture~
for details on this attribute.

~isDefined~ is used to represent undefined ~histogram~ objects.

*/
 private:
    bool isDefined;
    double histogram[256];
    HistogramChannel channel;
    double histogramMaxValue;

/*

3.2 Constructor

The default constructor does not do anything. This is required to assure that
no attributes are changed when ~Histogram~ objects are copied.

*/
 public:
    Histogram(void) { }

/*

Creates a ~Histogram~ object from the RGB data in the array ~imgdata~ of
size ~size~ for the specified ~channel~ (see the description of 
~HistogramChannel~ for details on this parameter).

*/
    Histogram(unsigned char* imgdata, 
	      unsigned int size, 
	      HistogramChannel channel);

/*

Create a ~Histogram~ object from pre-calculated histogram data. ~\_histogram~
is a pointer to the histogram data, ~\_channel~ the histogram channel (again,
see the description of ~HistogramChannel~ for details on this parameter) and
~maxValue~ is the maximum value of all four histograms.

*/
    Histogram(double * _histogram, HistogramChannel _channel, double maxValue);

/*

3.3 Attribute read and write methods

Returns an pointer to an array with 256 unsigned int elements. Each of
the 256 elements represents a brightness level and contains the percentage 
of pixels of this brightness in the selected channel
in the original image, from which the ~Histogram~ object has been
created, .

*/
    double* GetHistogramData(void);

/*

Returns the channel in the original image, from which the ~Histogram~ 
object has been created (see the description of ~HistogramChannel~
for details).

*/
    HistogramChannel GetHistogramChannel(void);

/*

Returns the maximum value of all four histograms of the original image, from
which the ~Histogram~ object has been created.

*/
   double GetHistogramMaxValue(void);

/*

Set the maximum value of all four histograms of the original image, from
which the ~Histogram~ object has ben created.

*/
   void SetHistogramMaxValue(double maxValue){ histogramMaxValue = maxValue;};

/*

Return ~true~ if this ~Histogram~ object represents an undefined
SECONDO ~histogram~ object.

*/
    bool IsDefined(void) const { return isDefined; };

/*

3.4 Comparison operations

The operations ~Picture::Equals()~ and ~Picture::Like()~ actually operate
on the histograms of images. Therefore, they use the methods of this
class to provide their functionality. See the description of class
~Picture~ for details.

*/
    bool Equals(Histogram* h, int n, int p, bool& valid);
    bool Like(int p, int t, int l, int u, bool& valid);
    bool Like(float p, float t, int l, int u, bool& valid);

/*

3.5 Methods required to implement ~StandardAttribute~

See the description of ~StandardAttribute~ and the SECONDO Programmer's
Guide for details.

*/
    void SetDefined(const bool d) { isDefined = d; };
    size_t HashValue(void);
    void CopyFrom(StandardAttribute*);
    int Compare(Attribute* a);
    bool Adjacent(Attribute* attr) { return false; }
    Histogram* Clone(void);
};

/*

4 Class ~Picture~

As the class ~Picture~ implements ~StandardAttribute~, ~picture~ objects
can be used in relations.

*/

class Picture : StandardAttribute {

/*

4.1 Private attributes

The attributes ~filename~, ~category~, ~date~ and ~isPortrait~ contain the
filename, the category, the creation data and a flag for the orientation of
the image. These attributes do not have an specific sementic in the Picture
algebra but are just used to store application data.

As SECONDO does not provide means to store dynamically allocated memory
segments without using FLOBs, all string attributes are defined as ~STRING~,
which is an array of 48+1 characters, ie. this array has exactly the size
of the maximum length of a SECONDO string.

Attribute ~date~ contains the image's date in the string representation of the 
DateTime algebra.

~isDefined~ is used to represent undefined SECONDO ~picture~ objects.

~jpegData~ is used to store the binary JPEG data of the image in SECONDO.

The four histograms in attribute ~histogram~ (for red, green, blue and 
luminance) are created during object construction. Main reason is the 
graphical representation of histograms: If multiple histograms are displayed 
in one graph, the maximum value of each histogram must be known for proper 
scaling of the Y axis. As the histogram viewer receives a single
histogram during each call only and cannot access the other histograms, each
histogram needs to contain an attribute specifying the maximum value, which
can be used by the viewer. As all four histograms of a ~Picture~ object are
generated during object creating, the maximum value can be set for all four
objects too. Moreover, calculating the histograms during object creation
is efficient, because ~Histogram~ objects are small and not to be generated
only once even if used multiple times.

*/

 private:
    STRING filename;
    STRING category;
    STRING date;
    bool isPortrait;
    bool isDefined;
    FLOB jpegData;
    Histogram histogram[4];

/*

4.2 Private methods

Private method ~createHistograms()~ creates all four histograms for the
attribute ~histogram~.

*/
    void createHistograms(char * imgdata, unsigned int size);

/*

4.3 Constructors and destructor

The default constructor does not do anything. This is required to assure that
no attributes are changed when ~Picture~ objects are copied.

*/

 public:
    Picture(bool) : isDefined(false), jpegData(0) { };
    Picture(void) { };

/*

This constructor creates a new ~Picture~ object from JPEG image data in
base64 format. ~imgdataB64~ contains the JPEG image data in
base64 format. The remaining parameters are used to the set the values of
the respective private attributes.

*/
    Picture(string imgdataB64,
	    string fn,
	    string cat,
	    bool isp,
	    string dt);

/*

This constructor creates a new ~Picture~ object from the JPEG image data
in binary format. ~imgdata~ contains the JPEG image data in binary format.
The remaining parameters are used to the set the values of the
respective private attributes.

*/
    Picture(char* imgdata,
	    unsigned int size,
	    string fn,
	    string cat,
	    bool isp,
	    string dt);

/*

The destructor does not do anything in the current version of the
implementation and is included for completeness only.

*/
    ~Picture(void);

/*

Sets the values to an empty picture object. This function is used in the
value mapping functions of the picture operators to fill the result picture.

*/
    void Set(char* imgdata,
	    unsigned int size,
	    string fn,
	    string cat,
	    bool isp,
	    string dt);

/*

4.4 Accessing JPEG image data

Return the JPEG image data in base64 format as ~string~ as stored in the
object's FLOB.

*/
    string GetJPEGBase64Data(void);

/*

Return a pointer to the JPEG image data as binary data, ie. as sequence of 
bytes in format RGBRGBRGB... Note that the memory containing the JPEG image
data is explicitely allocated during this call. Once you have completed 
working with the data, you have to free the allocated memory to avoid
memory leaks. As the image data is stored in a FLOB, there is no more
efficient way to implement this method. ~size~ will be set to the total size 
in bytes of the binary data returned.

*/
    char* GetJPEGData(unsigned int& size);

/*

4.5 Attribute read and write methods

The following five methods return the value of the respective private 
attribute.

*/
    string GetFilename(void) { return filename; }
    string GetCategory(void) { return category; }
    string GetDate(void) { return date; };
    bool IsPortrait(void) { return isPortrait; }
    bool IsDefined(void) const { return isDefined; }

/*

Sets the ~isDefined~ private attribute to the value of the paramter.

*/
    void SetDefined(const bool d) { isDefined = d; }

/*

4.6 Export and display operations

Save JPEG image to specified file and return ~true~ on success.

*/
    bool Export(string filename);

/*

Display the file with program ~PROG\_DISPLAY~ and return ~true~ on success

*/
    bool Display(void);

/*

4.7 Graphical operations

Return height and width of the image.

*/
    int GetHeight(void);
    int GetWidth(void);

/*

Based on the color space in the JPEG image, return ~true~ if the image
is a grayscale image.

*/
    bool IsGrayScale(void);

/*

Scale image to width ~w~ and height ~h~ and return pointer to new ~Picture~ 
object containing the result of the operation.

*/
    void Scale(Picture *pic, int w, int h);

/*

Cut rectangle with width ~w~ and height ~h~ at position ~x~ and ~y~ from
image and return pointer to new ~Picture~ object containing the result
of the operation.

*/
    void Cut(Picture *pic, int x, int y, int w, int h);

/*

Rotate image counter-clock-wise by $n * 90$ degrees.

*/
    void FlipLeft(Picture *pic, int n);

/*

Mirror image
and return pointer to new ~Picture~ object containing the result of the
operation. If parameter ~dir~ is ~true~, mirror vertically, and mirror
horizontally otherwise.

*/
    void Mirror(Picture *pic, bool dir);

/*

4.8 ~Histogram~ and comparison operations

As the comparison operations are based on histograms, they go into the same
section as the ~Colordist~ method.

Method ~Colordist~ returns a pointer to a copy of the specified channel's 
histogram, as stored in private attribute ~histogram~ during object creation.

*/
    Histogram* Colordist(HistogramChannel channel);

/*

Returns ~true~ if two images are similar. Two images are similar if no
averages of ~n~ consecutive elements of the histograms of both images exists
so that the averages have a differences of more than ~p~ percent.

*/
    bool Equals(Picture* pic, int n, int p, bool & valid);

/*

Returns ~true~ if $p \pm t$ percent of all pixels have are in brightness
interval $[l, u]$.

*/
    bool Like(int p, int t, int l, int u, bool & valid);
    bool Like(float p, float t, int l, int u, bool& valid);
    
/*

4.9 Methods required to implement ~StandardAttribute~

See the description of ~StandardAttribute~ and the SECONDO Programmer's
Guide for details.

*/
    size_t HashValue(void);
    void CopyFrom(StandardAttribute*);
    int Compare(Attribute* a);
    bool Adjacent(Attribute* attr) { return false; }
    Picture* Clone(void);
    int NumOfFLOBs() { return 1; }
    FLOB *GetFLOB(const int i) { assert( i == 0 ); return &jpegData; }

/*

4.10 Other methods

Returns ~true~ if the image data of the ~Picture~ object referenced by ~a~
is identical with this object's image data. Unlike the ~Compare()~ method,
this method does not compare any other attributes of the ~Picture~ class.

*/

    int SimpleCompare(Attribute* a);
};


#endif // __PICTURE_ALGEBRA_H__
