/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
MA  02111-1307  USA
----

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Image Similarity Algebra

March 2017 Michael Loris

The Image Similarity Algebra for SECONDO provides operations to store 
pictures in JPEG format into m-trees. It consists a ~Signature~ object 
representing a the signature of a single JPEG image. The object is a 
subtype of the Attribute class. The Image Similarity Algebra consists of 
multiple files, including external files for k-means clustering.


[TOC]

1.2 Defines and includes

*/

#include <iostream>

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "SecondoSystem.h"
#include "../../Tools/Flob/DbArray.h"
#include "Attribute.h"
#include "Symbols.h"
#include "ListUtils.h"
#include "AlgebraManager.h"
#include "Operator.h"
#include "LogMsg.h"
#include "StandardTypes.h"
#include "../FText/FTextAlgebra.h"

#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include "JPEGImage.h"
#include "ImageSimilarityAlgebra.h"


extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;


namespace ImageSignaturealg{



/*
2 Operator ~readSignatureFromFile~

*/

bool 
ImageSignature::readSignatureFromFile(const std::string _fileName, 
	const int colorSpace, const int texRange, const int percentSamples, 
	const int noClusters)
{
	int range = 10; //todo: range parameter
	
	this->fileName = _fileName;
	
	if(this->fileName == ""
        || !(access (this->fileName.c_str(), F_OK) != -1))
        //|| S_ISDIR(fileName.st_mode)
    {
		std::cerr << "readSignatureFromFile: Cannot open file '" 
		<< this->fileName << "'!" << endl;
		SetDefined(false);
		return false;
	}
	
	JPEGImage ji;
    
    ji.importJPEGFile(this->fileName, colorSpace, 
						texRange, 
						percentSamples, 
						noClusters);
    
    ji.computeCoarsenessValues(false, range);

	ji.computeContrastValues(false, range);

    unsigned int noDataPoints = ji.height * ji.width;

    ji.clusterFeatures(noClusters, DIMENSIONS, noDataPoints);

	for (auto tupl : ji.signature)
    {
		this->Append(tupl);
	}

    SetDefined(true);
	return true;
}


/*
2.1 Type mapper for operator ~readSignatureFromFile~

*/
ListExpr readSignatureFromFileTM(ListExpr args) 
{
	if ( nl->ListLength(args) == 5) 
	{
		ListExpr arg1 = nl->First(args);
		ListExpr arg2 = nl->Second(args);
		ListExpr arg3 = nl->Third(args);
		ListExpr arg4 = nl->Fourth(args);
		ListExpr arg5 = nl->Fifth(args);
		
		if (
			(nl->IsEqual(arg1, FText::BasicType()) ||
			nl->IsEqual(arg1, CcString::BasicType())
			) &&
			nl->IsEqual(arg2, CcInt::BasicType()) &&
			nl->IsEqual(arg3, CcInt::BasicType()) &&
			nl->IsEqual(arg4, CcInt::BasicType()) &&
			nl->IsEqual(arg5, CcInt::BasicType()))
			{
			return nl->SymbolAtom(ImageSignature::BasicType());
		}
	}
	return nl->SymbolAtom(Symbol::TYPEERROR());
}


int readSignatureFromFileSelect(ListExpr args) 
{
	ListExpr arg1 = nl->First(args);
	if (nl->IsEqual(arg1, FText::BasicType())) 
		return 0;
	if (nl->IsEqual(arg1, CcString::BasicType()))
		return 1;
			
	return -1;
}


/*
2.2 Value mapper for operator ~readSignatureFromFile~

*/
template<class StringType>
int readSignatureFromFileFun(Word* args, Word& result,
                   int message, Word& local, Supplier s)
{
	result = qp->ResultStorage(s);
	ImageSignature* res = static_cast<ImageSignature*>(result.addr);
	
	StringType* fileName = static_cast<StringType*>(args[0].addr);
	
	CcInt* colorSpace = static_cast<CcInt*>(args[1].addr);
	CcInt* texRange = static_cast<CcInt*>(args[2].addr);
	CcInt* percentSamples = static_cast<CcInt*>(args[3].addr);
	CcInt* noClusters = static_cast<CcInt*>(args[4].addr);

	if(fileName->IsDefined())
	{
		res->readSignatureFromFile(fileName->GetValue(),
			colorSpace->GetIntval(),texRange->GetIntval(),
			percentSamples->GetIntval(), noClusters->GetIntval());
	} 
	else 
	{
		res->SetDefined(false);
	}
	return 0;
}


/*
2.3 Selection of value mapper for operator ~readSignatureFromFile~

*/

ValueMapping readSignatureFromFileVM[] = 
{
  readSignatureFromFileFun<FText>,
  readSignatureFromFileFun<CcString>
};


/*
2.4 Specificaton for operator ~readSignatureFromFile~

*/
static const std::string readSignatureFromFileSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text> {text|string} -> imagesignature"
    "</text--->"
    "<text>readSignatureFromFile(fileName, col, tex, perc, clus) </text--->"
    "<text>Creates an ImageSignature instance from a jpeg file.</text--->"
    "<text>query readSignatureFromFile('test.jpg', 1, 3, 10, 50)</text--->"
    ") )";

/*
2.5 Creation of an Operator instance for operator ~readSignatureFromFile~

*/
static Operator readSignatureFromFileOp(
						"readSignatureFromFile",
                        readSignatureFromFileSpec,
                        2,
                        readSignatureFromFileVM,
                        readSignatureFromFileSelect,
                        readSignatureFromFileTM);
                        
                        
/*
3 Print functions for tuples

*/

std::ostream& operator<<(std::ostream& os, const ImageSignatureTuple& ist)
{
  os << "(" << ist.weight << "," 
  << ist.centroidXpos << "," 
  << ist.centroidYpos << ")";
  return os;
}


std::ostream& operator<<(std::ostream& os, const ImageSignature& p)
{
  os << " State: " << p.GetState()
  << "<" << p.GetFileName() << ">"
  << "<";

  for(int i = 0; i < p.GetNoImageSignatureTuples(); i++)
    os << p.GetImageSignatureTuple(i) << " ";

  os << ">";

  return os;
}


/*
4 Constructors for ImageSignature class

This first constructor creates a new ImageSignature.

*/
ImageSignature::ImageSignature(const int n) :
  Attribute(true),
  imageSignatureTuples(n)
{
  SetDefined(true);
}


ImageSignature::ImageSignature(const int n, 
	const double *W, const int *X, const int *Y ) :
  Attribute(true),
  imageSignatureTuples(n) 
{
  SetDefined(true);
  
  if( n > 0 )
  {
    for( int i = 0; i < n; i++ )
    {
      ImageSignatureTuple ist(W[i], X[i], Y[i]);
      Append(ist);
    }  
  }
}


/*
4.1 Copy Constructor

*/
ImageSignature::ImageSignature(const ImageSignature& src):
  Attribute(src.IsDefined()),
  imageSignatureTuples(src.imageSignatureTuples.Size()),
  state(src.state),
  fileName(src.fileName)
{
  imageSignatureTuples.copyFrom(src.imageSignatureTuples);
}


/*

4.2 Destructor

*/
ImageSignature::~ImageSignature()
{
}


ImageSignature& ImageSignature::operator=(const ImageSignature& src){
  this->state = src.state;
  imageSignatureTuples.copyFrom(src.imageSignatureTuples);
  return *this;
}


/*
4.3 NumOfFLOBs method

*/
int ImageSignature::NumOfFLOBs() const
{
  return 1;
}


/*
4.4 GetFLOB method

*/
Flob *ImageSignature::GetFLOB(const int i)
{
  assert(i >= 0 && i < NumOfFLOBs());
  return &imageSignatureTuples;
}


/*
4.5 Compare method
todo: Not yet implemented. Needed to be a tuple attribute.

*/
int ImageSignature::Compare(const Attribute*) const
{
  return 0;
}


/*
4.6 HashValue method
Because Compare returns alway 0, we can only return a constant hash value.

*/
size_t ImageSignature::HashValue() const{        //todo
  return  1;
}


/*
4.6 Adjacent method

Not required.

*/
bool ImageSignature::Adjacent(const Attribute*) const
{
  return 0;
}


/*
4.7 Clone method

Returns a new created ImageSignature (clone) which is a
copy of ~this~.

*/
ImageSignature *ImageSignature::Clone() const
{ 
  ImageSignature *is = new ImageSignature(*this);
  return is;
}


/*
4.8 Copy method


*/
void ImageSignature::CopyFrom(const Attribute* right){
  *this = *((ImageSignature*)right);
}


/*
4.9 Sizeof method

*/
size_t ImageSignature::Sizeof() const
{
  return sizeof(*this);
}


/*
5 Print method

*/
std::ostream& ImageSignature::Print(std::ostream& os) const
{
  return (os << *this);
}


/*
5.1 Append method

Appends an ImageSignatureTple ~ist~ at the end of the ImageSignature.

*/
void ImageSignature::Append(const ImageSignatureTuple& ist)
{
  imageSignatureTuples.Append(ist);
}


/*
2.3.10 Complete

Turns the ImageSignature into the ~complete~ state.

*Precondition* ~state == partial~. // todo: remove states

*/


/*
5.2 Correct method

Not yet implemented.

*/
bool ImageSignature::Correct()
{
  return true;
}


/*
5.3 Destroy method

Turns the ImageSignature into the ~closed~ state destroying the
ImageSignatureTuple array.

*/
void ImageSignature::Destroy()
{
  imageSignatureTuples.destroy();
}


/*
5.4 NoImageSignatureTuples

Returns the number of tuples of the ImageSignature.

*/
int ImageSignature::GetNoImageSignatureTuples() const
{
  return imageSignatureTuples.Size();
}


/*
5.5 GetImageSignatureTuple

Returns a signature tuple indexed by ~i~.

*/
ImageSignatureTuple ImageSignature::GetImageSignatureTuple(int i) const
{
 // assert(state == complete);
  assert(0 <= i && i < GetNoImageSignatureTuples());

  ImageSignatureTuple ist;
  imageSignatureTuples.Get(i, &ist);
  return ist;
}


/*
5.6 IsEmpty method

Returns if the ImageSignature is empty or not.

*/
const bool ImageSignature::IsEmpty() const
{
	// todo: report correct number
	return GetNoImageSignatureTuples() == 0;
}


/*
6 ImageSignature Algebra.

*/

/*

6.2 In method, gets information of the object from an nl

*/  
ListExpr ImageSignature::Out(ListExpr typeInfo, Word value)
{	
	ImageSignature* imgsig = static_cast<ImageSignature*>(value.addr);
  
	if(!imgsig->IsDefined())
	{
		return nl->SymbolAtom(Symbol::UNDEFINED());
	}
  
	if( imgsig->IsEmpty() )
	{
		return (nl->TheEmptyList());
	}
	else
	{		
	ListExpr result = nl->OneElemList(nl->ThreeElemList(
		nl->RealAtom(imgsig->GetImageSignatureTuple(0).weight),
		nl->IntAtom(imgsig->GetImageSignatureTuple(0).centroidXpos),
		nl->IntAtom(imgsig->GetImageSignatureTuple(0).centroidYpos))
		);				
     
	ListExpr last = result;

	for(int i = 1; i < imgsig->GetNoImageSignatureTuples(); i++)
	{
		last = nl->Append(last,nl->ThreeElemList(
		nl->RealAtom(imgsig->GetImageSignatureTuple(i).weight),
		nl->IntAtom(imgsig->GetImageSignatureTuple(i).centroidXpos),
		nl->IntAtom(imgsig->GetImageSignatureTuple(i).centroidYpos)));
	}
	return result;
  }
}


/*

6.2 In function, puts an object's information into a nested list

*/  
Word
ImageSignature::In(const ListExpr typeInfo, const ListExpr instance,
           const int errorPos, ListExpr& errorInfo, bool& correct)
{
	ImageSignature* imgsig = new ImageSignature(0);
	
	if(listutils::isSymbolUndefined(instance)){
		imgsig->SetDefined(false);
		correct = true;
		return SetWord(imgsig);
	}
	
	imgsig->SetDefined(true);
	
	
	ListExpr first = nl->Empty();
	ListExpr rest = instance;
  	
	while(!nl->IsEmpty(rest))
	{
		first = nl->First(rest);
		rest = nl->Rest(rest);
		
		if( nl->ListLength(first) == 3 &&
			nl->IsAtom(nl->First(first )) &&
			nl->AtomType(nl->First(first)) == RealType &&
			nl->IsAtom(nl->Second(first)) &&
			nl->AtomType(nl->Second(first)) == IntType && 
			nl->IsAtom(nl->Third(first)) &&
			nl->AtomType(nl->Third(first)) == IntType 
         )
		{
			ImageSignatureTuple ist( 
				nl->RealValue(nl->First(first)),
				nl->IntValue(nl->Second(first)),
                nl->IntValue(nl->Third(first))
            );
			imgsig->Append(ist);
		}
		else
		{
			correct = false;
			delete imgsig;
			return SetWord(Address(0));
		}
	}
	
  imgsig->Complete();
  correct = true;
  return SetWord(imgsig);
}


/*
6.3 Property method, describing the signature of the type constructor

*/
ListExpr ImageSignature::Property()
{
  return (nl->TwoElemList(
         nl->FiveElemList(
			nl->StringAtom("Signature"),
            nl->StringAtom("Example Type List"),
            nl->StringAtom("List Rep"),
            nl->StringAtom("Example List"),
            nl->StringAtom("Remarks")),
         nl->FiveElemList(
			nl->StringAtom("->" + Kind::DATA() ),
            nl->StringAtom(ImageSignature::BasicType()),
            nl->StringAtom("(<tuple>*) where <tuple> is "
            "(real int int)"),
            nl->StringAtom("( (1.2 3 4) (2.3 10 10) )"),
            nl->StringAtom("weight double, x- and y-coordinates int"
                          ))));
}


/*
6.4 Kind method

This method checks whether the type constructor is applied correctly.

*/
bool
ImageSignature::KindCheck( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, ImageSignature::BasicType() ));
}


/*

6.5 ~Create~-function

*/
Word ImageSignature::Create(const ListExpr typeInfo)
{	
	ImageSignature* imgsig = new ImageSignature(0);
	return (SetWord(imgsig));
}


/*
6.6 ~Delete~-function

*/
void ImageSignature::Delete(const ListExpr typeInfo, Word& w)
{
  ImageSignature* imgsig = (ImageSignature*)w.addr;

  imgsig->Destroy();
  delete imgsig;
}


/*
6.7 ~Open~-function

*/
bool
ImageSignature::Open( SmiRecord& valueRecord,
             size_t& offset,
             const ListExpr typeInfo,
             Word& value )
{
  ImageSignature *p = 
  (ImageSignature*)Attribute::Open(valueRecord, offset, typeInfo);
  value.setAddr( p );
  return true;
}


/*
6.7 ~Save~-function

*/
bool
ImageSignature::Save( SmiRecord& valueRecord,
             size_t& offset,
             const ListExpr typeInfo,
             Word& value )
{
  ImageSignature *p = (ImageSignature *)value.addr;
  Attribute::Save( valueRecord, offset, typeInfo, p );
  return true;
}


/*
6.8 ~Close~-function

*/
void ImageSignature::Close(const ListExpr typeInfo, Word& w)
{
  ImageSignature* imgsig = (ImageSignature*)w.addr;
  delete imgsig;
}


/*
6.9 ~Clone~-method

*/
Word ImageSignature::Clone(const ListExpr typeInfo, const Word& w)
{
  return SetWord( ((ImageSignature*)w.addr)->Clone() );
}

/*
7 ~SizeOf~-method

*/
int ImageSignature::SizeOfObj()
{
  return sizeof(ImageSignature);
}


/*
7.1 ~Cast~-method

*/
void* ImageSignature::Cast(void* addr)
{
  return (new (addr) ImageSignature);
}


/*
7.2 Creation of the type constructor instance

*/
TypeConstructor imgsig(
        ImageSignature::BasicType(),               
        ImageSignature::Property,
        ImageSignature::Out,   ImageSignature::In,
        0,              0,                  
        ImageSignature::Create,  ImageSignature::Delete,
        ImageSignature::Open,    ImageSignature::Save,
        ImageSignature::Close,   ImageSignature::Clone,
        ImageSignature::Cast,
        ImageSignature::SizeOfObj,
        ImageSignature::KindCheck);


/*
4 ImageSignatureAlgebra

*/


class ImageSimilarityAlgebra : public Algebra
{
  public:
    ImageSimilarityAlgebra() : Algebra()
    {
      AddTypeConstructor(&imgsig);
      AddOperator(&readSignatureFromFileOp);

      imgsig.AssociateKind(Kind::DATA());
    }
    ~ImageSimilarityAlgebra() {};
};


/*

5 Initialization of Algebra

*/


extern "C"
Algebra*
InitializeImageSimilarityAlgebra(NestedList *nlRef, QueryProcessor *qpRef)
{
  nl = nlRef;
  qp = qpRef;
  return (new ImageSimilarityAlgebra());
}

} // end of namespace
