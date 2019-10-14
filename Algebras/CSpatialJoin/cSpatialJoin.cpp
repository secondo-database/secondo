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
#include "Algebras/CRel/TypeConstructors/CRelTC.h"
#include "Algebras/CRel/Operators/OperatorUtils.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Stream.h"
#include "Algebras/CRel/TBlock.h"
#include "Algebras/CRel/TypeConstructors/TBlockTC.h"
#include "cSpatialJoin.h"
#include "BinaryTuple.h"
#include "cSpatialJoinProcessingL.h"

extern NestedList *nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

namespace csj {
 
/*
1.2 Class OperatorInfo

A subclass of class ~OperatorInfo~ is defined with information on the operator.

*/

// Store signature as String.
const std::string inSignature =
  "stream (tblock (a ((x1 t1) ... (xn tn)))) x \n"
  "stream (tblock (b ((y1 d1) ... (ym dm)))) x \n"
  "xi x yj \n";
const std::string outSignature =
  "-> \n"
  "stream (tblock (c ((x1 t1) ... (xn tn) (y1 d1) ... (ym dm))))";
  
size_t fDim;
size_t sDim;

class cSpatialJoin::Info: public OperatorInfo {
public:
  Info() {
    name = "cspatialjoin";
    signature = inSignature + outSignature;
    syntax = "_ _ cspatialjoin [ _ , _ ]";
    meaning = "Cache-conscious spatial join operator performing "
              "a partitioned spatial join on two streams of tuple blocks, "
              "where xi and yj are the names of the join "
              "attributes of the first and second stream, "
              "respectively.";
    example = "query Roads feed toblocks[1000] {a}"
              "Roads feed toblocks[1000] {b}"
              "cspatialjoin[GeoData_a, GeoData_b] count";
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
// last two arguments are included for testing purposes


  const uint64_t argNum = nl->ListLength(args);

  if (argNum != 4)
  {
    return listutils::typeError("Expected four arguments.");
  }

  // first argument must be a stream of tuple-blocks
  if(!listutils::isStream(nl->First(args))) {
    return listutils::typeError("Error in  first argument: "
                                  "Stream expected.");
  }
  if(!CRelAlgebra::TBlockTI::Check(nl->Second(nl->First(args)))) {
    return listutils::typeError("Error in  first argument: "
                                 "Stream of tuple-blocks expected.");
  }

  // second argument must be a stream of tuple-blocks
  if(!listutils::isStream(nl->Second(args))) {
    return listutils::typeError("Error in  second argument.: "
                                    "Stream expected.");
  }
  if(!CRelAlgebra::TBlockTI::Check(nl->Second(nl->Second(args)))) {
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

  // extract information about tuple block from args[]
  CRelAlgebra::TBlockTI fTBlockInfo =
                     CRelAlgebra::TBlockTI(nl->Second(nl->First(args)), false);
  CRelAlgebra::TBlockTI sTBlockInfo =
                     CRelAlgebra::TBlockTI(nl->Second(nl->Second(args)), false);

  // extract names of column of attribute from args[]
  std::string fAttrName = nl->SymbolValue(nl->Third(args));
  std::string sAttrName = nl->SymbolValue(nl->Fourth(args));

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
  CRelAlgebra::TBlockTI rTBlockInfo = CRelAlgebra::TBlockTI(false);
  // structure helps to eliminate the duplicates
  std::set<std::string> columnNames; 

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
    LocalInfo(Word fs, Word ss,
              Word fi, Word si,
              Supplier su):
        fStream(fs),
        sStream(ss),
        s(su),
        fStreamIsEmpty(false),
        sStreamIsEmpty(false),
        firstRequest(true),
        fIsFulyyLoaded(false),
        memLimit(qp->GetMemorySize(s)*1024*1024), 
        fMemTBlock(0),
        sMemTBlock(0),
        fNumTuples(0),
        sNumTuples(0),
        
        // Extract information about result tuple block type
        // and size
        rTBlockTypeInfo(CRelAlgebra::TBlockTI(qp->GetType(s), false)),
        rTBlockInfo(rTBlockTypeInfo.GetBlockInfo()),
        rTBlockSize(rTBlockTypeInfo.GetDesiredBlockSize()
        * CRelAlgebra::TBlockTI::blockSizeFactor),
        joinState(nullptr) {

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

  // destructor decreases the reference counters for all loaded tuple
  // blocks and closes both streams
  ~LocalInfo() {

    clearMemF();
    clearMemS();

		if(joinState) {
      delete joinState;
    }
		
    fStream.close();
    sStream.close();
  }
    
  // Support functions
    
  // Funktion requests tuple block from first stream and stores
  // them in fTBlockVector
  bool requestFirstStream() {
    CRelAlgebra::TBlock* tupleBlock = nullptr;
        
    if(!(tupleBlock = fStream.request())) {
      fStreamIsEmpty = true;
      
      return false;
		}

    uint64_t rows = tupleBlock->GetRowCount();
     
		fTBlockVector.push_back(tupleBlock);
		fMemTBlock += tupleBlock->GetSize();
		fNumTuples += rows;

    return true;        
	}

  // Funktion requests tuple block from second stream and stores
  // them in sTBlockVector
  bool requestSecondStream() {
    CRelAlgebra::TBlock* tupleBlock = nullptr;
        
    if(!(tupleBlock = sStream.request())) {
      sStreamIsEmpty = true;
      
      return false;
		}

    uint64_t rows = tupleBlock->GetRowCount();
    
		sTBlockVector.push_back(tupleBlock);
		sMemTBlock += tupleBlock->GetSize();
		sNumTuples += rows;

    return true;
  }

  // Function deletes all tuple blocks of the first stream from the
  // operator and sets both the memory and tuple counters for first 
  // stream to zero
  void clearMemF() {
    
    for(CRelAlgebra::TBlock* tb : fTBlockVector) {
      if(tb) {
        tb->DecRef();
      }
    }
          
    fTBlockVector.clear(); 
    fMemTBlock = 0;
    fNumTuples = 0;
  }

  // Function deletes all tuple blocks of the second stream from the
  // operator and sets both the memory and tuple counters for second 
  // stream to zero
  void clearMemS() {
    
    for(CRelAlgebra::TBlock* tb : sTBlockVector) {
      if(tb) {
        tb->DecRef();
			}
		}
    
    sTBlockVector.clear(); 
    sMemTBlock = 0;
    sNumTuples = 0;
  }
    
  // Function computes amount of memory in use:
  // size of two block vectors and size of all binary tables
  size_t getMemUsed() {

    return fMemTBlock
         + sMemTBlock
         + ((fNumTuples > sNumTuples) ?
            fNumTuples : sNumTuples)*sizeof(binaryTuple)*4;
  }
    
  // Function loads tuple blocks from first and second streams until
  // either the alloted memory space is used or both streams are empty
  void streamsRequest() {
 
    while ((getMemUsed() < memLimit)
           && (!fStreamIsEmpty || !sStreamIsEmpty)) {

      if(fStreamIsEmpty) {
			  requestSecondStream();
      }
			else
			  if(sStreamIsEmpty) {
				  requestFirstStream();
        }
			  else // both streams are not empty
			    if(fNumTuples > sNumTuples) {
            requestSecondStream();
          }
				  else {
            requestFirstStream();
				  }
    }
  }
    
  CRelAlgebra::TBlock* getNext() {

    CRelAlgebra::TBlock* rTBlock = new CRelAlgebra::TBlock(rTBlockInfo, 0, 0);
         
    while(true) {

      if(joinState) {
        if(!joinState->nextTBlock(rTBlock)) {
          return rTBlock;
        }
      }

      if(fStreamIsEmpty) {
        if(sStreamIsEmpty) {
        // join complete
          if(joinState) {
            delete joinState;
            joinState = nullptr;
          }
          if(rTBlock->GetRowCount() == 0) {
            rTBlock->DecRef();
            return 0;
          }
          else {
            return rTBlock;
          }  
        } // end of sStreamIsEmpty
        else {
        // first stream is empty, but second stream is not
          clearMemS();
          if(fIsFulyyLoaded) {
            do {
              if(!requestSecondStream()) {
                 break;
               }
            } while(getMemUsed() < memLimit);
          } // end of if fIsFullyLoaded
          else {
            clearMemF();
            delete joinState;
            joinState = nullptr;

            if(requestSecondStream()) {
              fStream.close();
              fStream.open();
              fStreamIsEmpty = false;
              streamsRequest();
            }
          } // end of if !fIsFullyLoaded
        } // end of !sStreamIsEmpty
      } // end of fStreamIsEmpty
      else {
        if(firstRequest) {
          // if both streams are empty,
          // then nothing to do
          if(!requestFirstStream()) {
            rTBlock->DecRef();
            return 0; 
          }
          if(!requestSecondStream()) {
            rTBlock->DecRef();
            return 0;
          }

          // both streams are not empty
          streamsRequest();

          if(fStreamIsEmpty) {
            fIsFulyyLoaded = true;
          }

          firstRequest = false;
        }
        else {
        // not first request
          clearMemF();
            do {
              if(!requestFirstStream()) {
                 break;
               }
            } while(getMemUsed() < memLimit);
        } // end of !firstRequest
      } // end of !fStreamIsEmpty

      if((fTBlockVector.size() > 0) && (sTBlockVector.size() > 0)) {
        
        if(joinState) {          
          delete joinState;
          joinState = nullptr;
        }

        joinState = new SpatialJoinState(fTBlockVector,
                                         sTBlockVector,
                                         fIndex,
                                         sIndex,
                                         rTBlockSize,
                                         numOfPartStripes,
                                         maxTuplePerPart,
                                         fDim,
                                         sDim);
      }
      
    } // end of while loop
  } // end of getNext()

  private:
    Stream<CRelAlgebra::TBlock> fStream;
    Stream<CRelAlgebra::TBlock> sStream;
    Supplier s;

    bool fStreamIsEmpty; // first stream is empty
    bool sStreamIsEmpty; // second stream is empty
    bool firstRequest; 
    bool fIsFulyyLoaded;

    uint64_t fIndex; // index of join attribute from first stream
    uint64_t sIndex; // index of join attribute from second stream
    uint64_t memLimit; // memory limit for operator
    uint64_t numOfPartStripes;
    uint64_t maxTuplePerPart;
    uint64_t fMemTBlock; // memory used by first stream
    uint64_t sMemTBlock; // memory used by second stream
    uint64_t fNumTuples; // number of tuples from first stream
    uint64_t sNumTuples; // number of tuples from second stream
        
    const CRelAlgebra::TBlockTI rTBlockTypeInfo;
    const CRelAlgebra::PTBlockInfo rTBlockInfo;
    const uint64_t rTBlockSize;
        
    std::vector<CRelAlgebra::TBlock*> fTBlockVector;
    std::vector<CRelAlgebra::TBlock*> sTBlockVector;
        
    SpatialJoinState* joinState;
		
}; //End class LocalInfo

/*
 1.5 Value Mapping
 
*/
int cspatialjoinVM(Word* args, Word& result, int message,
                   Word& local, Supplier s) {

  LocalInfo* localInfo = static_cast<LocalInfo*>(local.addr);

  switch (message) {
    
    case OPEN: {        
      if (localInfo) {
        delete localInfo;
      }
      local.addr = new LocalInfo(args[0], args[1], args[4], args[5], s);
      return 0;
    }
        
    case REQUEST: {
      result.addr = localInfo ? localInfo->getNext() : 0;
      return result.addr ? YIELD : CANCEL;
    }
      
    case CLOSE: {
      if (localInfo) {
        delete localInfo;
        local.addr = 0;
      }    
    return 0;
    }
  } // End switch

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

