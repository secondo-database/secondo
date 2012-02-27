
/*
----
This file is part of SECONDO.

Copyright (C) 2004-2007, University in Hagen, Department of Computer Science,
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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

 01590 Fachpraktikum "Erweiterbare Datenbanksysteme" 
 WS 2011 / 2012

 Svenja Fuhs
 Regine Karg
 Jan Kristof Nidzwetzki
 Michael Teutsch 
 C[ue]neyt Uysal

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of SuffixTreeAlgebra

[TOC]

1 Includes and Defines

*/
#ifndef SUFFIXTREEALGEBRA_H_
#define SUFFIXTREEALGEBRA_H_

#include "Attribute.h"
#include "ListUtils.h"
#include "../../Tools/Flob/Flob.h"
#include "../../Tools/Flob/DbArray.h"
#include "SuffixTree.h"
#include "SimpleTreeBuilder.h"
#include "UkkonenTreeBuilder.h"
#include "FTextAlgebra.h"

//namespace to avoid name conflicts
namespace sta{

const char terminationCharacter = (char)0;
const char textSeparator = (char)1;
/*

2 Class ~SuffixTreeAlgebra~

*/
class SuffixTree : public Attribute
{
  public:

    SuffixTree(SuffixTreeVertex* rootVertex);
    SuffixTree(bool);

    virtual ~SuffixTree();

    SuffixTreeVertex* GetRootVertex();

    bool Equal(Attribute * arg) const;

    //for attribute
    size_t HashValue() const;
    void CopyFrom(const Attribute* right);

    int Compare(const Attribute * arg) const;

    bool Adjacent(const Attribute * arg) const;
    size_t Sizeof() const;
    SuffixTree* Clone() const;
    static void* Cast(void* addr);
    static void Delete(const ListExpr typeInfo, Word& w);
    static void Close(const ListExpr typeInfo, Word& w);
    static bool Open( SmiRecord& valueRecord, size_t& offset, 
      const ListExpr typeInfo, Word& value); 
    static int SizeOfSuffixTree();

    ostream& Print( ostream &os ) const;
    int NumOfFLOBs() const;
    Flob *GetFLOB(const int i);
    SuffixTreeVertex *GetInMemoryTree();

    //support functions
    static bool      KindCheck( ListExpr type, ListExpr& errorInfo );

    static Word     In( const ListExpr typeInfo, const ListExpr instance,
                          const int errorPos, ListExpr& errorInfo,
                          bool& correct );

    static ListExpr Out( ListExpr typeInfo, Word value );

    static Word     Create( const ListExpr typeInfo );

    static Word     CloneSuffixTree( const ListExpr typeInfo, const Word& w );

/*
The following function defines the name of the type constructor,
resp. the name Secondo uses for this type.

*/
    static const string BasicType() 
    { 
      return "suffixtree";
    }

    static const bool CheckType(const ListExpr list)
    {
      return listutils::isSymbol(list, BasicType());
    }

    /*
      Create a persistent representation of our
      transient SuffixTree

    */
        void SaveToPersistent(SuffixTreeVertex*);

    /*
      Create a transient representation of our
      persistent SuffixTree

    */
      SuffixTreeVertex* LoadFromPersistent();

  private :

     SuffixTree() : mMemoryTree(NULL) {};
/*
This constructor should not be used.

*/
     friend class ConstructorFunctions<SuffixTree>;


     //member
    DbArray<size_t> mSuffixTree;
    DbArray<size_t> mSuffixIndex;
    Flob mInput;
    SuffixTreeVertex *mMemoryTree;
};

struct CslLocalData
{
  queue<int> *posQueue;
  TupleType *tupleType;
};

}//end namespace sta
#endif /* SUFFIXTREEALGEBRA_H_ */
