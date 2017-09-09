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

2 Defines and includes

*/


#ifndef __IMAGE_SIMILARITY_ALGEBRA_H__
#define __IMAGE_SIMILARITY_ALGEBRA_H__

#include "DateTime.h"
#include "Attribute.h"
#include "StandardTypes.h"
#include "Algebra.h"
//#include "../../Tools/Flob/Flob.h"
#include "../../Tools/Flob/DbArray.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "JPEGImage.h"

#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include "JPEGImage.h"

#define DIMENSIONS 7




/*

The class ~ImageSignature~ is derived from ~Attribute~, 
so that ~imageSignature~ objects can be used in relations.

*/



namespace FeatureSignaturealg{


class FeatureSignature : public Attribute
{

  public:
  
    FeatureSignature(const FeatureSignature& src);  
   
    ~FeatureSignature(); 

    FeatureSignature& operator=(const FeatureSignature& src);
    
    FeatureSignature(const int n);
    
    FeatureSignature(const int n, 
        const double *W, 
        const int *X, 
        const int *Y,
        const double *R,
        const double *G,
        const double *B,
        const double *COA,
        const double *CON);    
    
   
    int NumOfFLOBs() const;
    Flob *GetFLOB(const int i);
    int Compare(const Attribute*) const;
    bool Adjacent(const Attribute*) const;
    FeatureSignature *Clone() const;
    size_t Sizeof() const;
    std::ostream& Print(std::ostream& os) const;

    void Append(const FeatureSignatureTuple& ist);
    void Complete();
    bool Correct();
    void Destroy();
    int GetNoFeatureSignatureTuples() const;
    FeatureSignatureTuple GetFeatureSignatureTuple(int i) const;
    const bool IsEmpty() const;
    void CopyFrom(const Attribute* right);
    size_t HashValue() const;

    void ClearDBArray() { this->FeatureSignatureTuples.clean();}
    friend std::ostream& 
    operator <<(std::ostream& os,const FeatureSignature& p);

    static Word     In( const ListExpr typeInfo, 
                        const ListExpr instance,
                        const int errorPos, 
                        ListExpr& errorInfo,
                        bool& correct );
    static ListExpr Out( ListExpr typeInfo, Word value );
    static Word     Create( const ListExpr typeInfo );
    static void     Delete( const ListExpr typeInfo, Word& w );
    static void     Close( const ListExpr typeInfo, Word& w );
    static bool     Save( SmiRecord& valueRecord, size_t& offset,
                          const ListExpr typeInfo, Word& value    );
    static bool     Open( SmiRecord& valueRecord, size_t& offset,
                          const ListExpr typeInfo, Word& value    );
    static Word     Clone( const ListExpr typeInfo, const Word& w );
    static bool     KindCheck( ListExpr type, ListExpr& errorInfo );
    static int      SizeOfObj();
    static ListExpr Property();
    static void* Cast(void* addr);
    static const std::string BasicType() { return "featuresignature"; }
    static const bool checkType(const ListExpr type)
    {
      return listutils::isSymbol(type, BasicType());
    }
    
    // operator for reading a JPEG image from disk
    ListExpr readImageTM(ListExpr args);
    
    int readImageVM(Word* args, Word& result, int message,
                        Word& local, Supplier s);
                        
    
    bool readSignatureFromFile(const std::string _fileName, 
        const int colorSpace,
        const int coaRange,
        const int conRange,
        const int patchSize, 
        const int percentSamples, 
        const int noClusters);

    // operator for calculating SQFD of two signatures
    double calcSQFD(Word* args, Word& result, int message,
                    Word& local, Supplier s);
                    
    
    std::string GetFileName() const { return fileName; };
    void SetFileName(const std::string _fileName) 
    { 
        this->fileName = _fileName; 
    }    

  private:
    FeatureSignature() {} 
    DbArray<FeatureSignatureTuple> FeatureSignatureTuples;    
    std::string fileName;    
};

} // namespace
#endif // __PICTURE_ALGEBRA_H__
