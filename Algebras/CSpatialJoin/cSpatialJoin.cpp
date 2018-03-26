/*
----
This file is part of SECONDO.

Copyright (C) 2018,
Faculty of Mathematics and Computer Science,
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

\tableofcontents
 
1 Cache-conscious spatial join

The ~cspatialjoin~ operator is a cache-conscious spatial-join operator, 
which performs a partitioned spatial-join on two streams of tuple blocks.
As arguments it expects two streams of tuple blocks and the name of the 
join attribute for each argument relation.

1.1 Imports

*/
        
#include "NestedList.h"         
#include "QueryProcessor.h"    
#include "AlgebraManager.h"         
#include "StandardTypes.h"     
#include "Symbols.h"           
#include "ListUtils.h"          
#include <iostream>
#include <math.h> 
#include "BinaryTuple.h"
#include "CRel.h"
#include "CRelTC.h"
#include "OperatorUtils.h"
#include "RelationAlgebra.h"
#include "Stream.h"
#include "TBlock.h"
#include "TBlockTC.h"
#include "cSpatialJoin.h"
      

extern NestedList *nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

using namespace std;
using namespace CRelAlgebra;

namespace csj {
/*
1.2 Class OperatorInfo

A subclass of class ~OperatorInfo~ is defined with information on the operator.

*/

// Store signature as String.
const string inSignature =
  "stream (tblock (a ((x1 t1) ... (xn tn)))) x \n"
  "stream (tblock (b ((y1 d1) ... (ym dm)))) x \n"
  "xi x yj \n";
const string outSignature =
  "-> \n"
  "stream (tblock (c ((x1 t1) ... (xn tn) (y1 d1) ... (ym dm))))";
  
size_t fDim;
size_t sDim;

class cSpatialJoin::Info: public OperatorInfo {
public:
  Info() {
    name = "cspatialjoin";
    signature = inSignature + outSignature;
    syntax = "_ _ cspatialjoin [ _ , _ , _ , _]";
    meaning = "Cache-conscious spatial join operator performing"
              "a partitioned spatial join on two tuple-streams,"
              "where xi and yj are the names of the join"
              "attributes of the first and second stream,"
              "respectively.";
    example = "query CityNode feed CityWay feed "
              "cspatialjoin[NodeId, NodeRef, 32, 100] count";
  }
};

// constructor
cSpatialJoin::cSpatialJoin() : Operator(Info(),
                            valueMappings,
                            SelectValueMapping,
                            cspatialjoinTM) {
  SetUsesMemory();
}

// destructor
cSpatialJoin::~cSpatialJoin() {
}
/* 
 1.3 Type Mapping

The type mapping checks if exactly four arguments are passed to the operator. 
The first two arguments must be streams of tuple blocks. The second two 
arguments must be the names of the join attributes of the first and the 
second stream, respectively.
 
*/ 
ListExpr cSpatialJoin::cspatialjoinTM(ListExpr args) { 
// check the number of arguments
// last two arguments are intended for testing purposes
// Argument number 5 contains the size of the auxiliary structure 
// (list, grid structure or interval tree) in KByte
// Argument number 6 contains strip width used in sweep-plane


    if(!nl->HasLength(args, 6)) {
        return listutils::typeError("Wrong number of arguments");
    }

    // first argument must be a stream of tuple-blocks
    if(!listutils::isStream(nl->First(args))) {
        return listutils::typeError("Error in  first argument: "
                                    "Stream expected.");
    }
    if(!TBlockTI::Check(nl->Second(nl->First(args)))) {
        return listutils::typeError("Error in  first argument: "
                                    "Stream of tuple-blocks expected.");
    }

    // second argument must be a stream of tuple-blocks
    if(!listutils::isStream(nl->Second(args))) {
        return listutils::typeError("Error in  second argument.: "
                                    "Stream expected.");
    }
    if(!TBlockTI::Check(nl->Second(nl->Second(args)))) {
        return listutils::typeError("Error in  second argument: "
                                    "Stream of tuple-blocks expected.");
    }

    // third argument must be an attribute name
    if(!listutils::isSymbol(nl->Third(args))) {
    return listutils::typeError("Error in third argument: "
                                "Attribute name expected.");
    }

    // fourth argument must be an attribute name
    if(!listutils::isSymbol(nl->Fourth(args))) {
        return listutils::typeError("Error in fourth argument: "
                                    "Attribute name expected");
    }

    // fifth argument must an integer
    if(!CcInt::checkType(nl->Fifth(args))) {
        return listutils::typeError("Error in fifth argument: "
                                    "Integer value expected");
    }

    // sixth argument must an integer
    if(!CcInt::checkType(nl->Sixth(args))) {
        return listutils::typeError("Error in sixth argument: "
                                    "Integer value expected");
    }	

    // extract information about tuple block from args[]
    TBlockTI fTBlockInfo = TBlockTI(nl->Second(nl->First(args)), false);
    TBlockTI sTBlockInfo = TBlockTI(nl->Second(nl->Second(args)), false);

    // extract names of column of attribute from args[]
    string fAttrName = nl->SymbolValue(nl->Third(args));
    string sAttrName = nl->SymbolValue(nl->Fourth(args));

    // search for column index in the first relation
    uint64_t fNameIndex;
    if(!GetIndexOfColumn(fTBlockInfo, fAttrName, fNameIndex)) {
        return listutils::typeError("Error in third argument: "
                                    "Invalid column name");
    }
    
    // search for column index in the second relation
    uint64_t sNameIndex;
    if(!GetIndexOfColumn(sTBlockInfo, sAttrName, sNameIndex)) {
       return listutils::typeError("Error in fourth argument: "
                                   "Invalid column name");
    }

    // join attributes must be a kind of SPATIALATTRARRAY2D
    // or SPATIALATTRARRAY3D

    if(listutils::isKind(fTBlockInfo.columnInfos[fNameIndex].type,
                     Kind::SPATIALATTRARRAY2D())) {
		fDim = 2; 
	} else
	      if(listutils::isKind(fTBlockInfo.columnInfos[fNameIndex].type,
                     Kind::SPATIALATTRARRAY3D())) {
		      fDim = 3;
		   } else {
	               fDim = 0;
                   return listutils::typeError("Attribute " + 
                          fTBlockInfo.columnInfos[fNameIndex].name
                          + " is not of kind " +
                          "SPATIALATTRARRAY2D " +
                          "or SPATIALATTRARRAY3D");
	               }
	               
    if(listutils::isKind(sTBlockInfo.columnInfos[sNameIndex].type,
                     Kind::SPATIALATTRARRAY2D())) {
		sDim = 2; 
	} else
	      if(listutils::isKind(sTBlockInfo.columnInfos[sNameIndex].type,
                     Kind::SPATIALATTRARRAY3D())) {
		      sDim = 3;
		   } else {
	               sDim = 0;
                   return listutils::typeError("Attribute " + 
                          sTBlockInfo.columnInfos[sNameIndex].name
                          + " is not of kind " +
                          "SPATIALATTRARRAY2D " +
                          "or SPATIALATTRARRAY3D");
	               }

	
	// Initialize the type and size of result tuple block
	// and check for duplicates column names
	TBlockTI rTBlockInfo = TBlockTI(false);
	set<string> columnNames; // structure helps to eliminate the duplicates
	
	if(fTBlockInfo.GetDesiredBlockSize() >
	   sTBlockInfo.GetDesiredBlockSize()) {
		
	rTBlockInfo.SetDesiredBlockSize(fTBlockInfo.GetDesiredBlockSize());
	} 
	else {
	rTBlockInfo.SetDesiredBlockSize(sTBlockInfo.GetDesiredBlockSize());
	}
	
	for(size_t i=0; i < fTBlockInfo.columnInfos.size(); i++) {
		columnNames.insert(fTBlockInfo.columnInfos[i].name);
		rTBlockInfo.columnInfos.push_back(fTBlockInfo.columnInfos[i]);
	}
	
	for(size_t i=0; i < sTBlockInfo.columnInfos.size(); i++) {
		if(!columnNames.insert(sTBlockInfo.columnInfos[i].name).second){
			return listutils::typeError("Column name "
			+ sTBlockInfo.columnInfos[i].name
			+ " exists in both relations");
		}
		rTBlockInfo.columnInfos.push_back(sTBlockInfo.columnInfos[i]);
	}
	for (size_t i=0; i < rTBlockInfo.columnInfos.size(); i++)
        cout<<"Columninfo: "<<rTBlockInfo.columnInfos[i].name<<'\n';

	// Return result type
	ListExpr commonNames = nl->TwoElemList(nl->IntAtom(fNameIndex),
				nl->IntAtom(sNameIndex));


	return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()),
		commonNames,
		rTBlockInfo.GetTypeExpr(true));
}

/*
 1.4 LocalInfo-Class

*/
 
class LocalInfo {
	public:    
    
    // constructor
	LocalInfo(Word fs, Word ss, Word fi, Word si, Supplier su):
		fStream(fs),
		sStream(ss),
		s(su) {
		
			
		CcInt* index;
		// extract information about column index in the first relation
		index = static_cast<CcInt*>(fi.addr);
		fIndex = index->GetValue();
		
		// extract information about column index in the second relation
		index = static_cast<CcInt*>(si.addr);
		sIndex = index->GetValue();
		 
		fStream.open();
		sStream.open();
	}
	
	// destructor
	~LocalInfo() {
		fStream.close();
		sStream.close();
	}
	
	private:
		Stream<TBlock> fStream;
		Stream<TBlock> sStream;
		uint64_t fIndex;
		uint64_t sIndex;
		Supplier s;
};

/*
 1.5 Value Mapping
 
*/
 
int cspatialjoinVM(Word* args, Word& result, int message,
			Word& local, Supplier s) {
			
	const uint64_t stripLimit = ((CcInt*)args[5].addr)->GetValue();
	const uint64_t memLimit = ((CcInt*)args[4].addr)->GetValue() * 1024;
	cout<<"Memory limit: "<<memLimit<<'\n';
	cout<<"Strip limit: "<<stripLimit<<'\n';
	cout<<"Dim1: "<<fDim<<'\n';
	cout<<"Dim2: "<<sDim<<'\n';
				
	return 0;
}	

ValueMapping cSpatialJoin::valueMappings[] = {
    cspatialjoinVM,
    nullptr 
};

int cSpatialJoin::SelectValueMapping(ListExpr args) {
  return 0;
}

} // end of namespace csj

