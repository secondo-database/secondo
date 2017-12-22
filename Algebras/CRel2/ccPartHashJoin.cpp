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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

December 07, 2017

Author: Nicolas Napp

\tableofcontents

1 Cache-conscious iterative partitioned Hash-Join

The ~ccPartHashJoin~ operator is a cache-conscious equi-join operator, which performs a partitioned hash join on two streams of tuple blocks. As arguments, it expects two streams of tuple blocks and the name of the join attribute for each argument relation. The operator is part of SECONDO's CRel-Algebra.

Initially, the operator loads as many tuple blocks from stream R and S as fit into the allotted memory space. Then it extracts the relevant data, i.e. the hash-values of the join-attributes, from all loaded tuples of R and S and stores them in two ~temporary binary relations~. A binary relation consists of an array of ~binary tuples~ storing the extracted hash-values and the references to the actual tuples.

Both binary relations are partitioned and hash-joined together on the basis of the ~hash values~ of the join attributes. When a matching binary tuple pair has been found, the actual tuples are retrieved and the join attributes are compared directly. If the match can be confirmed, then the tuples are concatenated and appended to the result relation.

Once this phase is complete the operator checks if stream ~S~ has been ~fully loaded~. If this is the case, then it iterates over all remaining tuple blocks in stream R and joins them to the tuple blocks of S. The join is performed in the same fashion as explained above.

If stream S is ~not~ fully loaded, then the operator iterates over all remaining tuple blocks in stream S and joins them to the tuple blocks R. Once this is complete, the next tuple blocks R are loaded and stream S is run past these blocks again. This is repeated until all tuple blocks R have been processed.

1.1 Imports

*/

#include "ccPartHashJoin.h"
#include <iostream>
#include "BinaryTuple.h"
#include "CRel.h"
#include "CRelTC.h"
#include <cstdint>
#include <exception>
#include "ListUtils.h"
#include "OperatorUtils.h"
#include "QueryProcessor.h"
#include "RelationAlgebra.h"
#include "StandardTypes.h"
#include "Stream.h"
#include <string>
#include "TBlock.h"
#include "TBlockTC.h"
#include "ccPartHashJoinUtils.h"

using namespace CRelAlgebra;

using std::set;
using std::string;
using std::vector;
using std::exception;

extern NestedList *nl;
extern QueryProcessor *qp;

namespace CRel2Algebra {

/*
1.1 Class OperatorInfo

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
const string err =
  "\nExpected:\n"
  "stream(tblock1) x stream(tblock2) x attr1 x attr2";

class ccPartHashJoin::Info: public OperatorInfo
{
public:
  Info()
  {
    name = "ccPartHashJoin";
    signature = inSignature + outSignature;
    syntax = "_ _ ccPartHashJoin [ _ , _ ]";
    meaning =
      "Cache-conscious equi-join operator performing a partitioned "
      "hash join on two tuple-streams, where xi and yj are the names "
      "of the join attributes of the first and second stream, respectively.";
    example = "query CityNode feed CityWay feed "
                "ccPartHashJoin[NodeId, NodeRef] totuples count";
    remark = "myRemark";  // optional
  }
};

// constructor
ccPartHashJoin::ccPartHashJoin() :
    Operator(Info(), valueMappings, SelectValueMapping, TypeMapping)
{
  SetUsesMemory();
}

// destructor
ccPartHashJoin::~ccPartHashJoin()
{
}

/*
1.1 Type Mapping

The type mapping checks if exactly four arguments are passed to the operator. The first two arguments must be streams of tuple blocks. The second two arguments must be the names of the join attributes of the first and the second stream, respectively.

The column indexes of the join attributes are also extracted here for later use and appended to the argument list:

----
((stream (tblock (a ((x1 t1) ... (xn tn)))))
 (stream (tblock (b ((y1 d1) ... (ym dm))))) xi yj)
  -> (APPEND
      (i j)
      (stream (tblock (c ((x1 t1) ... (xn tn) (y1 d1) ... (ym dm))))))
----

*/

ListExpr ccPartHashJoin::TypeMapping(ListExpr args)
{
  // Check number of arguments
  if (!nl->HasLength(args, 4))
  {
    return listutils::typeError("Expecting 4 arguments" + err);
  }

  // Stream R: first argument must be a stream of type "tblock".
  if (!listutils::isStream(nl->First(args)))
  {
    return listutils::typeError("First argument not a stream" + err);
  }
  if (!TBlockTI::Check(nl->Second(nl->First(args))))
  {
    return listutils::typeError(
        "First argument not a stream of type tblock" + err);
  }

  // Stream S: second argument must be a stream of type "tblock".
  if (!listutils::isStream(nl->Second(args)))
  {
    return listutils::typeError("Second argument not a stream" + err);
  }
  if (!TBlockTI::Check(nl->Second(nl->Second(args))))
  {
    return listutils::typeError(
        "Second argument not a stream of type tblock" + err);
  }

  // Column-name R: third argument must be a valid attribute name.
  if (nl->AtomType(nl->Third(args)) != SymbolType)
  {
    return listutils::typeError(
        "Third argument not a valid attribute name" + err);
  }

  // Column-name S: fourth argument must be a valid attribute name.
  if (nl->AtomType(nl->Fourth(args)) != SymbolType)
  {
    return listutils::typeError(
        "Fourth argument not a valid attribute name" + err);
  }

  // Extract info on tblocks from args
  TBlockTI tblockRInfo = TBlockTI(nl->Second(nl->First(args)), false);
  TBlockTI tblockSInfo = TBlockTI(nl->Second(nl->Second(args)), false);

  // Extract column-names of equi-join attributes from args
  string nameR = nl->SymbolValue(nl->Third(args));
  string nameS = nl->SymbolValue(nl->Fourth(args));

  // Find index of column-name in R
  uint64_t nameRIndex;
  if (!GetIndexOfColumn(tblockRInfo, nameR, nameRIndex))
  {
    return listutils::typeError(
        "Third argument is not a valid column-name of R");
  }

  // Find index of column-name in S
  uint64_t nameSIndex;
  if (!GetIndexOfColumn(tblockSInfo, nameS, nameSIndex))
  {
    return listutils::typeError(
        "Fourth argument is not a valid column-name of S");
  }

  // The join attributes must be of the same type
  if (!nl->Equal(tblockRInfo.columnInfos[nameRIndex].type,
      tblockSInfo.columnInfos[nameSIndex].type))
  {
    return listutils::typeError("The join attributes are not of the same type");
  }

  //Initialize result type from both tblock-types R and S.
  //Check for duplicate column-names.

  TBlockTI tblockResultInfo = TBlockTI(false);
  tblockResultInfo.SetDesiredBlockSize(
    (tblockRInfo.GetDesiredBlockSize() > tblockSInfo.GetDesiredBlockSize()) ?
     tblockRInfo.GetDesiredBlockSize() :
     tblockSInfo.GetDesiredBlockSize());
  set<string> columnNames;

  for (const TBlockTI::ColumnInfo &columnInfo : tblockRInfo.columnInfos)
  {
    columnNames.insert(columnInfo.name); // ".second" deleted.
    // Store copy of columnInfo in tblockResultInfo.
    tblockResultInfo.columnInfos.push_back(columnInfo);
  }

  for (const TBlockTI::ColumnInfo &columnInfo : tblockSInfo.columnInfos)
  {
    // see: http://www.cplusplus.com/reference/set/set/insert/
    if (!columnNames.insert(columnInfo.name).second)
    {
      return listutils::typeError(
          "Column-name " + columnInfo.name + "exists in both streams");
    }
    // Store copy of columnInfo in tblockResultInfo.
    tblockResultInfo.columnInfos.push_back(columnInfo);
  }

  ListExpr appendArgs = nl->TwoElemList(nl->IntAtom(nameRIndex),
      nl->IntAtom(nameSIndex));

  return nl->ThreeElemList(nl->SymbolAtom(Symbols::APPEND()), appendArgs,
      tblockResultInfo.GetTypeExpr(true));

} // End TypeMapping

/*
1.1 Class LocalInfo

*/

class LocalInfo
{
public:

/*
The constructor initializes all variables and opens streams R and S.

*/

  LocalInfo(Word R, Word S, Word indexR, Word indexS, Supplier su) :
    streamR(R),
    streamS(S),
    s(su),
    streamRExhausted(false),
    streamSExhausted(false),
    firstRequest(true),
    streamSFullyLoaded(false),
    memLimit(qp->GetMemorySize(s) * 1024ULL * 1024ULL),
    memTBlockR(0),
    memTBlockS(0),
    numTuplesR(0),
    numTuplesS(0),
    maxTuplesPerTBlock(0),
    // Extract TBlock type-info for result TBlock.
    tblockTypeInfo(TBlockTI(qp->GetType(s), false)),
    tblockInfo(tblockTypeInfo.GetBlockInfo()),
    resultTBlockSize(tblockTypeInfo.GetDesiredBlockSize() 
      * TBlockTI::blockSizeFactor),
    hashJoinState(nullptr)
  {
    CcInt* index;
    index = static_cast<CcInt*>(indexR.addr);
    joinIndexR = index->GetValue();

    index = static_cast<CcInt*>(indexS.addr);
    joinIndexS = index->GetValue();

    streamR.open();
    streamS.open();
  }

/*
The destructor decreases the reference counters for all loaded tuple blocks, deletes the instance of class ~HashJoinState~, if it exists, and closes both streams R and S.

*/

  ~LocalInfo()
  {
    /* Decrease reference-counter, if necessary.
       If it reaches zero, object will be deleted. */

    for (TBlock* tblock : tBlockVectorR)
    {
      if (tblock)
      {
        tblock->DecRef();
      }
    }

    for (TBlock* tblock : tBlockVectorS)
    {
      if (tblock)
      {
        tblock->DecRef();
      }
    }

    if (hashJoinState)
    {
      delete hashJoinState;
    }

    streamR.close();
    streamS.close();
  }

/*
1.1.1 Support Functions

The following function requests tuple blocks from stream R and stores them in an instance of class ~vector~.

*/

  bool requestR()
  {
    TBlock* tBlock = nullptr;

    if (!(tBlock = streamR.request()))
    {
      streamRExhausted = true;

      return false;
    }

    // Comment!! maxTuplesPerTBlock needed for tupleref (num bits...)
    uint64_t numRows = tBlock->GetRowCount();
    if (numRows > maxTuplesPerTBlock)
    {
      maxTuplesPerTBlock = numRows;
    }

    tBlockVectorR.push_back(tBlock);
    memTBlockR += tBlock->GetSize();
    numTuplesR += numRows;

    return true;
  }

/*
The following function requests tuple blocks from stream S and stores them in an instance of class ~vector~.

*/
  bool requestS()
  {
    TBlock* tBlock = nullptr;

    if (!(tBlock = streamS.request()))
    {
      streamSExhausted = true;

      return false;
    }

    // Comment!! maxTuplesPerTBlock needed for tupleref (num bits...)
    uint64_t numRows = tBlock->GetRowCount();
    if (numRows > maxTuplesPerTBlock)
    {
      maxTuplesPerTBlock = numRows;
    }

    tBlockVectorS.push_back(tBlock);
    memTBlockS += tBlock->GetSize();
    numTuplesS += numRows;

    return true;
  }

/*
The following function deletes all tuple blocks R from the operator and sets both the memory and tuple counters for R back to zero.

*/

  void clearMemR()
  {
    // Clear memory for currently loaded TBlocks R
    for (TBlock* tblock : tBlockVectorR)
    {
      if (tblock)
      {
        tblock->DecRef();
      }
    }
    tBlockVectorR.clear(); // retains memory-space for deleted objects

    memTBlockR = 0;
    numTuplesR = 0;
  }

/*
The following function deletes all tuple blocks S from the operator and sets both the memory and tuple counters for S back to zero.

*/

  void clearMemS()
  {
    // Clear memory for currently loaded TBlocks S
    for (TBlock* tblock : tBlockVectorS)
    {
      if (tblock)
      {
        tblock->DecRef();
      }
    }
    tBlockVectorS.clear(); // retains memory-space for deleted objects

    memTBlockS = 0;
    numTuplesS = 0;
  }

/*
The following function determines the amount of memory currently used by the operator. This is the sum of all materialized tuple blocks and the extra amount of memory needed to partition the binary relations.

*/
  size_t getMemUsed()
  {
    return memTBlockR + tBlockVectorR.size() * sizeof(TBlock*) + memTBlockS
        + tBlockVectorS.size() * sizeof(TBlock*)
        + ((numTuplesR > numTuplesS) ? numTuplesR : numTuplesS) * 3ULL * 8ULL;
  }

/*
This function loads tuple blocks from stream R and S until either the alloted memory space is used up or both streams are exhausted. It always loads from the stream which has fewer tuples materialized.

*/

  void requestRandS()
  {
    while ((getMemUsed() < memLimit) && 
        (!streamRExhausted || !streamSExhausted))
    {
      if (streamRExhausted)
      {
        requestS();

      } else if (streamSExhausted)
      {
        requestR();

      } else if (numTuplesR > numTuplesS) // Both streams are NOT exhausted.
      {
        requestS(); // Fewer tuples from S are materialized.

      } else
      {
        requestR(); // Fewer tuples from R are materialized.
      }
    }
  }

/*
1.1.1 Next Tuple Block

This function handles the requests for tuple blocks from stream R and S and hands on the next result tuple block as soon as it has reached its specified size in MiB.

*/

  TBlock* getNext()
  {

/*
Instantiate a new result tuple block.

*/

    TBlock* resultTBlock = new TBlock(tblockInfo, 0, 0); // no "delete"

/*
This while-loop only breaks if either a result tuple block is complete and passed on in the stream or both argument streams are exhausted and all tuple blocks have been joined together.

*/

    while (true)
    {
      if (hashJoinState)
      {
        // Class exists, thus start or resume joining current TBlocks.
        if (!hashJoinState->nextTBlock(resultTBlock))
        {
          // One result TBlock is complete; currently loaded TBlocks 
          // are not yet fully joined.
          return resultTBlock;
        }
      }

/*
These nested if-statements handle the loading and deleting of all tuple blocks from both argument streams R and S.

*/

      if (streamSExhausted)
      {
        if (streamRExhausted) // Join complete.
        {

/*
The join is complete. Both streams R and S are exhausted and all tuple blocks have been joined together. If there is still an unfinished result tuple block, then it is passed on in the stream.

*/
          if (hashJoinState) // delete "hashJoinState" if it exists.
          {
            delete hashJoinState;
            hashJoinState = nullptr;
          }
          if (resultTBlock->GetRowCount() == 0)
          {
            resultTBlock->DecRef();
            return 0;
          } else
          {
            return resultTBlock;
          }
        } else // (streamSExhausted && !streamRExhausted)
        {
          // Clear memory for currently loaded TBlocks R.
          clearMemR();

          if (streamSFullyLoaded)
          {

/*
Stream S has been fully loaded. Now iterate over all remaining tuple blocks in stream R.

*/

            // Load as many TBlocks from R as possible.
            do
            {
              if (!requestR())
              {
                break;
              }
            } while (getMemUsed()
                < memLimit);

            // Create new BatR, if at least one TBlock R has been loaded.
            if (tBlockVectorR.size() > 0)
            {

/*
Only tuple blocks from stream R have been newly loaded, whereas stream S had been fully loaded before. An instance of class ~HashJoinState~ exists. Thus, only the binary relation for R must be newly built and partitioned. The binary relation for S, however, will be kept and used again.

*/

              hashJoinState->newBatR();
            }
          } else // (!streamSFullyLoaded)
          {

/*
Stream S is exhausted, but stream R is not. Clear all tuple blocks from the operator and restart stream S. Now load as many tuple blocks from stream R and S as fit into the alloted memory space.

*/

            // Clear memory for currently loaded TBlocks S.
            clearMemS();

            delete hashJoinState;
            hashJoinState = nullptr;

            if (requestR()) // Load next TBlock R.
            {
              // Restart stream S and load as many TBlocks S as possible.
              streamS.close();
              streamS.open();
              streamSExhausted = false;

/*
Load tuple blocks until memory is used up or both streams are exhausted. Always load from the stream which has fewer tuples materialized.

*/

              requestRandS();
            }
          }
        }
      } else // (!streamSExhausted)
      {
        if (firstRequest)
        {

/*
This is the first time tuple blocks are requested from the arguement streams.

Load one tuple block from each stream, if possible. If not, return; there is nothing to do...

*/

          if (!requestR())
          {
            // Stream R empty; nothing to do.
            resultTBlock->DecRef();
            return 0;
          }

          if (!requestS())
          {
            // Stream S empty; nothing to do.
            resultTBlock->DecRef();
            return 0;
          }

/*
Load tuple blocks until memory is used up or both streams are exhausted. Always load from stream which has fewer tuples materialized.

*/

          requestRandS();

/*
If all tuple blocks of S are materialized, mark S as fully loaded.

*/

          if (streamSExhausted)
          {
            streamSFullyLoaded = true; // All TBlocks S materialized.
          }
          firstRequest = false;

        } else // (!firstRequest)
        {

/*
Tuple blocks from stream R have been loaded before and will be kept. Now delete all tuple blocks S and load as many new tuple blocks from stream S as possible. Only the binary relation for S needs to be rebuilt and partitioned. The binary relation for R, however, will be kept and used again.

*/

          // Clear memory for currently loaded TBlocks S
          clearMemS();

          // Load as many TBlocks from stream S as possible
          do
          {
            if (!requestS())
            {
              break;
            }
          } while (getMemUsed()
              < memLimit);

          // Create new BatS, if at least one TBlock S has been loaded.
          if (tBlockVectorS.size() > 0)
          {
            hashJoinState->newBatS();
          }
        }
      } // END if(!streamSExhausted)

/*
If there are ~no~ tuple blocks from R ~and~ S currently loaded, then the join is complete. However, if there are, then check if there is an instance of class ~HashJoinState~. If there is ~not~, then instantiate it.

*/

      if (tBlockVectorR.size() > 0 && tBlockVectorS.size() > 0)
      {
        if (!hashJoinState)  // If class does not exist, instantiate it.
        {

/*
The size of a ~binary tuple~ is determined dynamically at runtime. They come in three different sizes:

\begin{itemize}
\item binaryTupleSmall (4 Byte),
\item binaryTupleMedium (8 Byte), or 
\item binaryTupleLarge (16 Byte).
\end{itemize}

The goal is to use as few bytes as possible and still be able to encode the reference to each tuple (i.e. the tuple block and row number) in the field ~tupelref~.

*/

          // Determine max number of TBlocks materialized from stream R or S.
          uint64_t maxTBlocks =
              tBlockVectorR.size() > tBlockVectorS.size() ?
                  tBlockVectorR.size() : tBlockVectorS.size();

          // Add safety margins, in case the next iteration needs larger values.
          maxTBlocks *= 1.2;
          maxTBlocks += 1;
          maxTuplesPerTBlock *= 1.2;
          maxTuplesPerTBlock += 10;

          uint64_t minBitsTBlock = 0;
          while (true)
          {
            if ((1ULL << ++minBitsTBlock) > maxTBlocks)
              break;
          }

          uint64_t minBitsTuple = 0;
          while (true)
          {
            if ((1ULL << ++minBitsTuple) > maxTuplesPerTBlock)
              break;
          }

          uint64_t sumBits = minBitsTBlock + minBitsTuple;
          uint64_t bits;
          uint64_t bitsTupleRef;
          uint64_t sizeBinaryTuple;

          if (sumBits <= 16)
          {
            bits = 16;
            sizeBinaryTuple = 4;

          } else if (sumBits <= 32)
          {
            bits = 32;
            sizeBinaryTuple = 8;

          } else if (sumBits <= 64)
          {
            bits = 64;
            sizeBinaryTuple = 16;

          } else
          {
            throw std::runtime_error(
                "Too many TBlocks and Tuples in stream R or S or both.");
          }

          bitsTupleRef = bits - minBitsTBlock - (bits - sumBits) / 2;

/*
Now instantiate the class ~HashJoinState~ with the right template parameter.

*/

          if (sizeBinaryTuple == 4)
          {
            hashJoinState = new HashJoinState<binaryTupleSmall>(tBlockVectorR,
                tBlockVectorS, joinIndexR, joinIndexS, numTuplesR, numTuplesS,
                bitsTupleRef, resultTBlockSize);

          } else if (sizeBinaryTuple == 8)
          {
            hashJoinState = new HashJoinState<binaryTupleMedium>(tBlockVectorR,
                tBlockVectorS, joinIndexR, joinIndexS, numTuplesR, numTuplesS,
                bitsTupleRef, resultTBlockSize);

          } else if (sizeBinaryTuple == 16)
          {
            hashJoinState = new HashJoinState<binaryTupleLarge>(tBlockVectorR,
                tBlockVectorS, joinIndexR, joinIndexS, numTuplesR, numTuplesS,
                bitsTupleRef, resultTBlockSize);

          } else
          {
            throw std::runtime_error(
                sizeBinaryTuple
                  + ": wrong size of binaryTuple. 4, 8, and 16 allowed only.");
          }
        }
      } else // No more tuple blocks to join.
      {
        if (hashJoinState)
        {
          delete hashJoinState;
          hashJoinState = nullptr;
        }
      }
    } // End while(true) loop
  } // END getNext()

private:
  Stream<TBlock> streamR;
  Stream<TBlock> streamS;
  uint64_t joinIndexR;
  uint64_t joinIndexS;
  Supplier s;
  bool streamRExhausted;
  bool streamSExhausted;
  bool firstRequest;
  bool streamSFullyLoaded;
  uint64_t memLimit;
  uint64_t memTBlockR;  // Amount of memory currently occupied by TBlocks R.
  uint64_t memTBlockS;  // Amount of memory currently occupied by TBlocks S.
  uint64_t numTuplesR;  // Number of tuples from streamR currently in memory.
  uint64_t numTuplesS;  // Number of tuples  from streamRcurrently in memory.
  uint64_t maxTuplesPerTBlock;
  const TBlockTI tblockTypeInfo;
  const PTBlockInfo tblockInfo;
  const uint64_t resultTBlockSize;
  vector<TBlock*> tBlockVectorR;
  vector<TBlock*> tBlockVectorS;
  HashJoinStateBase* hashJoinState;
}; // End class LocalInfo


/*
1.1 Value Mapping

*/

int ccPartHashJoinVM(Word* args, Word& result, int message, Word& local,
    Supplier s)
{

  LocalInfo* localInfo = static_cast<LocalInfo*>(local.addr);

  switch (message)
  {
  case OPEN:
  {
    if (localInfo)
    {
      delete localInfo;
    }
    local.addr = new LocalInfo(args[0], args[1], args[4], args[5], s);
    return 0;
  }
  case REQUEST:
  {
    try
    {
      result.addr = localInfo ? localInfo->getNext() : 0;
      return result.addr ? YIELD : CANCEL;
    } catch (const exception& e)
    {
      cout << "Aborting!\n";
      cout << "Exception: " << e.what() << '\n';
      if (localInfo)
      {
        delete localInfo;
        local.addr = 0;
      }
    }
    return CANCEL;
  }
  case CLOSE:
  {
    if (localInfo)
    {
      delete localInfo;
      local.addr = 0;
    }
    return 0;
  }
  } // End switch
  return 0;
}

ValueMapping ccPartHashJoin::valueMappings[] = {
    ccPartHashJoinVM,
    nullptr };

int ccPartHashJoin::SelectValueMapping(ListExpr args)
{
  return 0;
}

} /* namespace CRel2Algebra */
