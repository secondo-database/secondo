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

This module contains all methods required for ~Histogram~
to represent a SECONDO ~histogram~ object plus functions required by
SECONDO to use ~Histogram~ plus basic SECONDO operators on ~histogram~.

2 Includes and other preparations

*/

using namespace std;

#include <cmath>
#include "NestedList.h"
#include "PictureAlgebra.h"
#include "JPEGPicture.h"

extern NestedList* nl;
extern QueryProcessor *qp;


static double DIFFERENZ_DELTA =  0.001;

/*

3 Implementation of class ~Histogram~

For further information please check ~PictureAlgebra.h~.

3.1 Constructors and destructors

3.1.1 Constructor

The constructor transforms the specified ~rgbData~ into a
histgram. Furthermore some private variables such as ~channel~ and the
~histogramMaxValue~ etc. are set.

*/
Histogram::Histogram( unsigned char * rgbData,
		      unsigned long rgbSize,
		      HistogramChannel _channel) {

    if (PA_DEBUG) cerr << "Histogram::Histogram()-1 called" << endl;
    
    //
    //	Initialise the histogram array and the channel !
    //
    histogramMaxValue = 0.0;
    channel = HistogramChannel(0);
    isDefined = false;
    for ( int i=0; i<256; i++ )
	histogram[i] = 0.0;

    if (( _channel < 0 ) || ( _channel > 3 ) )
    {
	cerr << endl << endl
	     << "Channel has a wrong value. Valid values are [0, 3]. "
	     << "Specified value: " << _channel << endl<< endl;

	return;
    }

    channel = _channel;
    isDefined = true;


    unsigned int no;

    if ( channel < HistogramChannel(HC_BRIGHTNESS) )
    {
	//
	//	Create Histogram for Red, Green, Blue
	//
    	for ( unsigned int i=channel; i< rgbSize-3; i+=3 )
    	{
		no = (unsigned int) rgbData[i];
		if ( (no < 0) || (no > 255 ))
			cerr << no  <<", ";
		else
		{
			histogram[ no ]++;
		}
	
	 }
	 cerr << endl;
    }
    else
    {
	//
	//	Create Histogram for BRIGHTNESS
        //
        double y = 0.0;
        int index = 0;

	for ( unsigned int i=0; i< rgbSize-3; i+=3 )
	{
		y = 0.3  * (unsigned int) rgbData[i] +
                    0.59 * (unsigned int) rgbData[i+1] + 
                    0.11 * (unsigned int) rgbData[i+2];

		index = (int) y;

		histogram[ index ]++;
	}
    }


    //
    //	The percentage is required, therefore another calculation
    //  is necessary.
    //
    //	rgbSize is the size of Red, Green and Blue data. 
    //
    histogramMaxValue = 0.0;
    for (int i = 0; i < 256; i++)
    {
	histogram[i] = 
		((double)histogram[i]) * 100.0 * 3.0 / (double)(rgbSize+1.0);

	if ( histogramMaxValue < histogram[i] )
		histogramMaxValue = histogram[i];
    }


    //
    //	Print out for debug purposes
    //
    if (PA_DEBUG)
    {
    	cerr << "Histogram::Histogram histogram: " <<  endl;
	for ( int i=0; i< 256; i++ )
	{
		cerr << histogram[i] ;
		if ( (i%10) == 0 )
			cerr << endl;
		else
			cerr << ", ";
	}
	cerr << endl;
    	cerr << "Histogram::Histogram channel: " << channel << endl;
    	cerr << "Max Value: " << histogramMaxValue << endl;
    }

}

/*

3.1.2 Constructor

This constructor gets data of a ~histogram~, the channel and the maximum value of
the ~histogram~. These information are saved in the ~histogram~ object.

*/
Histogram::Histogram( double * _histogram,
		      HistogramChannel _channel,
                      double maxValue) {
    if (PA_DEBUG) cerr << "Histogram::Histogram()-2 called" << endl;

    //
    //	Initialise the histogram array and the channel !
    //
    channel = HistogramChannel(0);
    isDefined = false;
    for ( int i=0; i<256; i++ )
	histogram[i] = 0.0;

    //
    //	Check the specified parameter !
    //
    if (( _channel < 0 ) || ( _channel > 3 ) )
    {
	cerr << endl << endl
	     << "Channel has a wron value. Valid values are [0, 3]. "
	     << "Specified value: " << _channel << endl<< endl;

	return;
    }

    //
    //	Everything is fine !
    //
    channel = _channel;
    histogramMaxValue = maxValue;
    isDefined = true;

    for ( int i=0; i<256; i++ )
    {
	histogram[i] = _histogram[i];
    }

    //
    //	Print out for debug purposes
    //
//  if (PA_DEBUG)
//  {
//    	cerr << "Histogram::Histogram channel: " << channel << endl;
//    	cerr << "Histogram::Histogram histogram: " <<  endl;
//	for ( int i=0; i< 256; i++ )
//	{
//		cerr << histogram[i] ;
//		if ( (i%10) == 0 )
//			cerr << endl;
//		else
//			cerr << ", ";
//	}
//	cerr << endl;
//   }

}

/*

3.2 ~StandardAttribute~ implementation

*/

size_t Histogram::HashValue(void) const {
    if (PA_DEBUG) cerr << "Histogram::HashValue() called" << endl;

    unsigned long h = 0;

    if ( !isDefined) return 0;

    for ( int i=0; i<256; i++ )
	h += ((int) (histogram[i]*100)) * (i+1) + channel;

    return h;
}

void Histogram::CopyFrom(const StandardAttribute* attr) {
    if (PA_DEBUG) cerr << "Histogram::CopyFrom() called" << endl;

    const Histogram* h = (const Histogram*) attr;

    //
    //	copy all attributes
    //
    isDefined = h->isDefined;

    if ( ! isDefined )
	return;

    channel   = h->channel;
    histogramMaxValue = h->histogramMaxValue;

    for ( int i=0; i<256; i++ )
	histogram[i] = h->histogram[i];
}

int Histogram::Compare(const Attribute* a) const {
    if (PA_DEBUG) cerr << "Histogram::Compare() called" << endl;

    const Histogram * h = (const Histogram*) a;

    //
    //	At first, check the channel
    //
    if ( channel < h->channel )
	return -1;
    else
    if ( channel > h->channel )
	return +1;

    //
    //	There is no difference in terms of channel, so check
    //	the histograms.
    //
    for ( int i=0; i< 256; i++ )
    {
	if ( fabs( histogram[i] - h->histogram[i]) < DIFFERENZ_DELTA )
		continue;

	if ( histogram[i] < h->histogram[i] )
		return -1;
	else
		return 1;	
    }

    //
    //	In this case, both histograms are equal! 
    //
    return 0;
}

Histogram* Histogram::Clone(void) const {
    if (PA_DEBUG) cerr << "Histogram::Clone() called" << endl;

    Histogram * h = new Histogram();

    h->CopyFrom((const StandardAttribute*) this);

    return h;
}

/*

3.3 Methods required for SECONDO operators

3.3.1 Function ~GetHistogramData~

This function returns the data of the ~histogram~.

*/
double* Histogram::GetHistogramData(void) {
    if (PA_DEBUG) cerr << "Histogram::GetHistogramData() called" << endl;

    return histogram;
}

/*

3.3.2 Function ~GetHistogramChannel~

The function ~GetHistogramChannel~ returns the channel of the ~histogram~.

*/
HistogramChannel Histogram::GetHistogramChannel(void) {
    if (PA_DEBUG) cerr << "Histogram::GetHistogramChannel() called" << endl;

    return channel;
}

/*

3.3.3 Function ~GetHistogramMaxValue~

~GetHistogramMaxValue~ returns the maximum value of all four ~histograms~.

*/
double Histogram::GetHistogramMaxValue(void) {
    if (PA_DEBUG) cerr << "Histogram::GetHistogramMaxValue() called" << endl;

    if (PA_DEBUG)
	cerr << "Max Value: " << histogramMaxValue << endl;
    return histogramMaxValue;
}

/*
 
3.4  Operators on ~histogram~

3.4.1 Operator ~Equals~

The function compares two ~histogram~ objects whether they are equal or not.
Therefore, it  calculates the average values of an specified number ~n~
of values in a row. The ~histogram~ objects are defined as equal if the average values differs less 
than ~p/10000~ from each other. 

*/

bool Histogram::Equals(Histogram* h, int n, int p, bool& valid, double& diff) {

    static const double Tolerance_Unit = 0.0001;

    if (PA_DEBUG) cerr << "Histogram::Equals() called" << endl;

    if ( n <= 0 )
    {
        cerr << endl << endl
	     << "Function: equals: picture x picture x int x int"  << endl	
	     << "Third argument " << n << " must be positive. It will be set to 1." 
             << endl << endl;
       
	n=1;
    }
    if ( n > 256 )
    {
        cerr << endl << endl
	     << "Function: equals: picture x picture x int x int"  << endl	
	     << "Third argmument " << n << " must be lower equal 256. It will be set to 1." 
             << endl << endl;
       
	n=1;
    }
    if ( (p<0) || ((double)p > (100/Tolerance_Unit)) )
    {
        cerr << endl << endl
	     << "Function: equals: picture x picture x int x int"  << endl	
	     << "The tolerance value must be between 0 and " << 100/Tolerance_Unit << endl
             << "Specified value: " << p << " will be set to 100" << endl << endl;
       
	p=100;
    }

    double m1=0, m2=0;
    valid = true;
    bool isEqual = true;

    //
    //	Create intervals of n numbers in row ! Than calculate the 
    //  average values of these n numbers for comparision.
    //
    for( int i=0; i<=256-n; i++ )
    {
	m1 = 0.0;
	m2 = 0.0;

	for ( int j=i; j<i+n; j++ )
	{
		m1 += histogram[ j ];
		m2 += h->histogram[ j ];

		//if (histogram[j] < 0 
		//    || histogram[j] > 100
		//    || h->histogram[j] < 0
		//    || h->histogram[j] > 100) {
		//    cerr << "Histogram::Equals() this looks fishy! j="
		//	 << j
		//	 << " histogram[j]="
		//	 << histogram[j]
		//	 << " h->histogram[j]="
		//	 << h->histogram[j]
		//	 << " this="
		//	 << (int) this
		// 	 << " h="
		//	 << (int) h
		//	 << endl;
		//}
	}
	
	m1 = m1 / (double) n;
	m2 = m2 / (double) n;

       
	//
	//	Check, whether these pictures are equal !
	//
        double dist = abs( m1 - m2 );
        double maxdiff = p * Tolerance_Unit;
        if ( dist > maxdiff )
	{
        if (PA_DEBUG)
		cerr << i<< ". [" << m1-maxdiff << ", " << m1+maxdiff 
				<< "] , m2 = " << m2 << endl;
		isEqual = false;
                diff += dist; 
	}
        if (PA_DEBUG)
		cerr << i<< ". [" << m1-maxdiff << ", " 
			<< m1+maxdiff << "] (, m1 = " << m1 
			<< "), m2 = " << m2 << endl;
    }

    cerr << "histogram difference: " << diff << endl; 
    if (PA_DEBUG) cerr << "These pictures are " 
			<< ((isEqual)? "": "not " ) << "equal." << endl;
    return( isEqual );

}

/*

3.4.2 Operator ~Like~

This function checks whether  ~p~ +- ~t~ percent of all pixel are in the
specified interval [~l~, ~u~].

*/

bool Histogram::Like(int p, int t, int l, int u, bool& valid) {
    if (PA_DEBUG) cerr << "Histogram::Like() called (Type:Int)" << endl;

    //
    //	Check the specified arguments
    //
    if (( p < 0 ) || ( p > 100 ))
    {
        cerr << endl << endl
	     << "Function: like: picture x int x int x int x int"  << endl	
             << "The second argument must be in the interval "
             << "[0, 100]." << endl
             << "Specified argument: " << p << endl << endl;

	valid = false;
	return false;
    }
    if (( t < 0 ) || ( t > 100 ))
    {
        cerr << endl << endl
	     << "Function: like: picture x int x int x int x int"  << endl	
             << "The third argument must be in the interval "
             << "[0, 100]." << endl
             << "Specified argument: " << t << endl << endl;

	valid = false;
	return false;
    }
    //
    //	Actually, I expect that l < u, but nevertheless, I'll check it.
    //
    int min = (l<u) ? l:u;
    int max = (l<u) ? u:l;

    if (( min < 0 ) || ( max > 255 ))
    {
        cerr << endl << endl
	     << "Function: like: picture x int x int x int x int"  << endl	
             << "The fourth and fifth argument must be in the interval"
             << "[0, 255]." << endl
             << "Specified arguments: " << l << ", " << u << endl << endl;

	valid = false;
	return false;
    }
    if (( p-t < 0 ) || ( p+t > 100 ))
    {
        cerr << endl << endl
	     << "Function: like: picture x int x int x int x int"  << endl	
             << "The values calculated by " << endl
             << "   second argument + third argument OR " << endl
             << "   second argument - third argument " << endl 
             << "must be in the interval  [0, 100]." << endl
             << "Specified arguments: " << endl
             << "  Second argument: " << p  << endl
             << "  Third argument: " << t  << endl
             << "  => 0<= " << p-t<<", "<< p+t <<" <= 100 " << endl << endl;

	valid = false;
	return false;
    }


    double percentageOfPixel = 0.0;
    valid = true;

    for ( int i=min; i<=max; i++ )
    {
	percentageOfPixel += histogram[i];
    }

    if (PA_DEBUG)
    {
	cerr << "[" << p-t << ", " << p+t << "], l:"
			<< percentageOfPixel << endl;

    	cerr << " p-t <= percentageOfPixel: " << 
		((p-t)<=percentageOfPixel) << endl;
    	cerr << " p+t >= percentageOfPixel: " << 
		((p+t)>=percentageOfPixel) << endl;
    }

    if (( ((double)p-t) <= percentageOfPixel ) && 
	( percentageOfPixel <= ((double)p+t) ))
    {
	return true;
    }

    return false;
}

/*

3.4.3 Operator ~Like~

This function checks whether  ~p~ +- ~t~ percent of all pixel are in the
specified interval [~l~, ~u~]. The main differences of this method in
comparision with the method described in 3.4.2 are the specified real
values ~p~ and ~t~.

*/
bool Histogram::Like(float p, float t, int l, int u, bool& valid) {

    if (PA_DEBUG) cerr << "Histogram::Like() called Type(Real)" << endl;

    //
    //	Check the specified arguments
    //
    if (( p < 0.0 ) || ( p > 100.0 ))
    {
        cerr << endl << endl
	     << "Function: like: picture x real x real x int x int"  << endl	
             << "The second argument must be in the interval "
             << "[0, 100]." << endl
             << "Specified argument: " << p << endl << endl;

	valid = false;
	return false;
    }
    if (( t < 0.0 ) || ( t > 100.0 ))
    {
        cerr << endl << endl
	     << "Function: like: picture x real x real x int x int"  << endl	
             << "The third argument must be in the interval "
             << "[0, 100]." << endl
             << "Specified argument: " << t << endl << endl;

	valid = false;
	return false;
    }
    //
    //	Actually, I expect that l < u, but nevertheless, I'll check it.
    //
    int min = (l<u) ? l:u;
    int max = (l<u) ? u:l;

    if (( min < 0 ) || ( max > 255 ))
    {
        cerr << endl << endl
	     << "Function: like: picture x int x int x int x int"  << endl	
             << "The fourth and fifth argument must be in the interval"
             << "[0, 255]." << endl
             << "Specified arguments: " << l << ", " << u << endl << endl;

	valid = false;
	return false;
    }
    if (( p-t < 0.0 ) || ( p+t > 100.0 ))
    {
        cerr << endl << endl
	     << "Function: like: picture x real x real x int x int"  << endl	
             << "The values calculated by " << endl
             << "   second argument + third argument OR " << endl
             << "   second argument - third argument " << endl 
             << "must be in the interval  [0, 100]." << endl
             << "Specified arguments: " << endl
             << "  Second argument: " << p  << endl
             << "  Third argument: " << t  << endl
             << "  => 0.0<= " << p-t<<", "<< p+t <<" <= 100.0 " << endl << endl;

	valid = false;
	return false;
    }


    double percentageOfPixel = 0.0;
    valid = true;

    for ( int i=min; i<=max; i++ )
    {
	percentageOfPixel += histogram[i];
    }

    if (PA_DEBUG)
    {
	cerr << "[" << p-t << ", " << p+t << "], l:"
			<< percentageOfPixel << endl;
    }

    //
    //	query pic1 like [ 100,0, 0, 255 ] liefert false !!! Erwartet wurde
    //	True
    //

    cerr << " p-t <= percentageOfPixel: " << 
		((p-t)<=percentageOfPixel) << endl;
    cerr << " p+t >= percentageOfPixel: " << 
		((p+t)>=percentageOfPixel) << endl;
		
    if (( (p-t) <= percentageOfPixel ) && ( percentageOfPixel <= (p+t) ))
    {
	return true;
    }

    return false;
}

/*

4 Nested list representaiton of class ~Histogram~

*/

static ListExpr OutHistogram(ListExpr typeInfo, Word value) {
    if (PA_DEBUG) cerr << "OutHistogram() called" << endl;

    Histogram * h = (Histogram*) value.addr;

    if ( h->IsDefined() )
    {
	double * histoData = h->GetHistogramData();

	ListExpr result = nl->OneElemList( nl->RealAtom( histoData[0]));
	ListExpr histoDataList = result;

	for ( int i=1; i<256; i++ )
	{
		histoDataList = nl->Append( histoDataList, 
				   nl->RealAtom( histoData[i]) ); 
	}

	if (PA_DEBUG)
	    cerr << "MaxValue: " << h->GetHistogramMaxValue() << endl;
	return( nl->ThreeElemList(
			(int)nl->IntAtom(h->GetHistogramChannel()),
			nl->RealAtom(h->GetHistogramMaxValue()),
			result ));
	
    }
    
    return nl->SymbolAtom( "undef" );
}

static Word InHistogram(const ListExpr typeInfo,
		       const ListExpr instance,
		       const int errorPos,
		       ListExpr& errorInfo,
		       bool& correct) {
    if (PA_DEBUG) cerr << "InHistogram() called" << endl;


    double histoData[ 256 ];
    for ( int i=0; i<256; i++ )
	histoData[i] = 0.0;
    
    ListExpr channel;
    ListExpr maxValue;
    ListExpr color;

    if ( nl->ListLength(instance) == 3 )
    {
	//
	//	Get & check the channel !
	//
	channel = nl->First(instance);

	if ( ! (nl->IsAtom( channel ) && (nl->AtomType(channel) == IntType )))
	{
		if (PA_DEBUG) 
			cerr << "InHistogram() channel as 'Int' expected" 
				<< endl;
		correct = false;

		return( SetWord(Address(0)));
	}

	//
	//	Get & check the channel !
	//
	maxValue = nl->Second(instance);

	if ( !(nl->IsAtom( maxValue ) && (nl->AtomType(maxValue)==RealType)))
	{
		if (PA_DEBUG) 
			cerr << "InHistogram() maxValue as 'Real' expected" 
				<< endl;
		correct = false;

		return( SetWord(Address(0)));
	}

	//
	//	Get & check the histogram-data  !
	//
	int index = 0;
	ListExpr colorList = nl->Third( instance );

	while ( ! nl->IsEmpty( colorList ) )
	{
		color = nl->First( colorList );
		colorList = nl->Rest( colorList );

		if ( ! (nl->IsAtom(color) && (nl->AtomType(color)==RealType)))
		{
			if (PA_DEBUG) 
				cerr << "InHistogram() colorNo. " << index
				     << " as 'Int' expected" << endl;
			correct = false;

			return( SetWord(Address(0)));

		}
		histoData[ index ] = nl->RealValue(color);
		index++;
	}
    }
    else
    {
	if (PA_DEBUG) 
		cerr << "InHistogram() Wrong Number of Parameter! Got " 
			<< nl->ListLength( instance )
		        << " parameter, but expected three." << endl;
	correct = false;

	return( SetWord(Address(0)));
    }

    correct = true;

    return SetWord( new Histogram( histoData,
			HistogramChannel(nl->IntValue( channel)),
    			nl->RealValue(maxValue) )); 
}

/*

5 Description Signature Type Constructor of ~Histogram~

*/
static ListExpr HistogramProperty(void) {
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
		nl->StringAtom("histogram"),
		nl->StringAtom(
		    "(<channel> <brightness_0> ... <brightness_255>)"),
		nl->StringAtom(
		    "(0 0 0 1 2 .. 255 255)"),
		nl->StringAtom("n/a")));
}

/*

6 Persistant storage of ~Histogram~

*/

static Word CreateHistogram(const ListExpr typeInfo) {
    if (PA_DEBUG) cerr << "CreateHistogram() called" << endl;

    Histogram* h = new Histogram();
    return(SetWord( h ));
}

static void DeleteHistogram(const ListExpr typeInfo, Word& w) {
    if (PA_DEBUG) cerr << "DeleteHistogram() called" << endl;

    delete (Histogram*) w.addr;
    w.addr = 0;
}

static void CloseHistogram(const ListExpr typeInfo, Word& w) {
    if (PA_DEBUG) cerr << "CloseHistogram() called" << endl;

    delete (Histogram *) w.addr;
    w.addr = 0;
}

static Word CloneHistogram(const ListExpr typeInfo, const Word& w) {
    if (PA_DEBUG) cerr << "CloneHistogram() called" << endl;

    return SetWord(((Histogram*) w.addr)->Clone());
}

static int SizeOfHistogram(void) {
    if (PA_DEBUG) cerr << "SizeOfHistogram() called" << endl;

    return( sizeof(Histogram));
}

/*

7 Kind checking of ~Histogram~

*/

static bool CheckHistogram(ListExpr type, ListExpr& errorInfo) {
    if (PA_DEBUG) cerr << "CheckHistogram() called" << endl;

    return( nl->IsEqual( type, "histogram" ));
}

/*

8 Cast function of ~Histogram~

*/

static void* CastHistogram(void* addr) {
    if (PA_DEBUG) cerr << "CastHistogram() called" << endl;

    return new (addr) Histogram;
}

/*

9 Type constructor of ~Histogram~

*/

TypeConstructor histogram(
    "histogram",                              //name
    HistogramProperty,                        //property function describing 
                                              //  signature
    OutHistogram, InHistogram,                //Out and In functions
    0, 0,                                     //SaveToList and RestoreFromList 
                                              //  functions
    CreateHistogram, DeleteHistogram,         //object creation and deletion
    0, 0,                                     //object open and save
    CloseHistogram, CloneHistogram,           //object close and clone
    CastHistogram,                            //cast function
    SizeOfHistogram,                          //sizeof function
    CheckHistogram );                         //kind checking function

