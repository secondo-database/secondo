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


//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]


[1] Methods to read and write files

August-February 2015, Daniel Fuchs 

[TOC]

1 Overview


This file contains the implementation of methods to read
and write files.

1.1 Includes
 
*/ 

#include "NestedList.h"
#include "QueryProcessor.h"
#include "ListUtils.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Stream.h"
#include "Attribute.h"
#include "StandardTypes.h"
#include "Algebras/Spatial/SpatialAlgebra.h"
#include "Operator.h"
#include "Symbols.h"
#include "Algebra.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "FileSystem.h"
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>


extern NestedList* nl;
extern QueryProcessor* qp;

#ifndef BINRELWRITEREAD_H
#define BINRELWRITEREAD_H

namespace distributedClustering{
    
/*
1.2 ~checkFile~

This method is used in typmapping to check the type of file resp.
of the stored relation.

*/

boost::mutex nlparsemtx;
boost::mutex createRelMut;
boost::mutex copylistmutex;

#define FILE_BUFFER_SIZE 1048576
  
  bool checkFile(ListExpr& argwt,  ListExpr& relType, std::string& errMsg)
  {
    ListExpr arg = nl->First(argwt);
    if(!FText::checkType(arg))
    { 
        errMsg = "text expected";
        return false;
    }
    
    ListExpr query = nl->Second(argwt);
    Word queryResult;
    std::string typeString = "";
    std::string errorString = "";
    bool correct;
    bool evaluable;
    bool defined;
    bool isFunction;
    qp->ExecuteQuery(query, queryResult,
                     typeString, errorString, correct,
                     evaluable, defined, isFunction);
    if(!correct || !evaluable || !defined || isFunction){
      errMsg = "could not extract filename ("+
      errorString + ")";
      return false;
    }
    std::string filename;
      FText* res = (FText*) queryResult.addr;
      if(!res->IsDefined()){
        res->DeleteIfAllowed();
        errMsg = "undefined filename";
        return false;
      } else {
        filename = res->GetValue();
        res->DeleteIfAllowed();
      }
    
    // access file for extracting the type 
    std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
    if(!in.good()){
      errMsg = "could not open file " + filename;
      return false;
    }
    char marker[4];
    in.read(marker,4);
    if(!in.good()){
      in.close();
      errMsg = "problem in reading from file " 
      + filename;
      return false;
    }
    std::string m(marker,4);
    
    if(m!="srel"){
      in.close();
      errMsg = "not a binary relation file";
      return false;
    }
    uint32_t typeLength;
    in.read((char*) &typeLength, sizeof(uint32_t));
    char* buffer = new char[typeLength];
    in.read(buffer,typeLength);
    if(!in.good()){
      in.close();
      errMsg = "problem in reading from file";
      return false;
    }  
    
    std::string typeS(buffer, typeLength);
    delete [] buffer;
    
    {
      boost::lock_guard<boost::mutex> guard(nlparsemtx);
      if(!nl->ReadFromString(typeS, relType)){
        in.close();
        errMsg = "problem in determining rel type";
        return false;
      } 
    }
    
    if(!Relation::checkType(relType)){
      in.close();
      errMsg = "not a valid relation type " 
      + nl->ToString(relType);
      return false;
    }
    return true;
  }
  
/*
1.3 ~readFile~

Read tuple from a file and store members in a vector. 
The Method is written for all Operators of distributedClustering Algebra

*/

enum OpType {DBSCAN, NEIGHBOR , DISTMERGE, DISTSORT, DISTSORTSAMP};

  template <class TYPE,class MEMB_TYP_CLASS>
  bool readFile(const std::string& filename, const ListExpr& _tt
  , std::string& errMsg 
  ,std::vector <MEMB_TYP_CLASS*>& membArray
  ,std::vector <MEMB_TYP_CLASS*>& membArrayUnt
  ,TupleBuffer* tupleBuffer, int geoPos, int xPicRefPos
  ,OpType optype
  ,int clIdPos = 0, int clTypePos = 0
  ,int arrOffset = 0
  )
  {
    ListExpr fileTypeList;
    TupleType* ftt;
    //for picture
    bool setPictureXYRef = ( TYPE::BasicType() == Picture::BasicType()
    && xPicRefPos >=0 );
    
    TupleType* tt = new TupleType(_tt);
    char* inBuffer = new char[FILE_BUFFER_SIZE];
    std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
    bool ok = in.good();
    if(ok){//read Header -> TupleType of File
      in.rdbuf()->pubsetbuf(inBuffer, FILE_BUFFER_SIZE);
      char marker[4];
      in.read(marker,4);
      std::string ms(marker,4);
      if(ms!="srel"){
        ok = false;
        errMsg = "Type is not srel" ;
        return false;
      }
      uint32_t length;
      in.read((char*) &length,sizeof(uint32_t));
      char* headerBuffer = new char[length];
      in.read(headerBuffer,length);
      std::string list(headerBuffer,length);
      delete[] headerBuffer;
      {
        boost::lock_guard<boost::mutex> guard(nlparsemtx);
        ok = nl->ReadFromString(list,fileTypeList); 
        ListExpr tupleType = nl->Second(fileTypeList);
        tupleType = SecondoSystem::GetCatalog()->NumericType(tupleType);
        ftt = new TupleType(tupleType);
        
        switch (optype)
        {
          case DISTSORT:
          case DISTSORTSAMP:
          case DISTMERGE:
            if(!ftt->equalSchema(*tt)){
              errMsg = "expected scheme does not fit the stored scheme.";
              ok = false;
              return false;
            }
            break;
            
          default:
            break;
        }
      }
      ok = ok && in.good();
    } else {
      errMsg = "could not open file ";
      return false;
    }
    if(ok)
    { // read file and store tuples in tupleBuffer
      TupleId id = arrOffset;
      while(!in.eof() && in.good())
      {
        
        uint32_t size;
        in.read( (char*) &size, sizeof(uint32_t));
        if(size > 0) {
          char* buffer = new char[size];
          in.read(buffer, size);
          if(in.good()){
            Tuple* tuple = new Tuple(ftt); // use fileTupleType
            tuple->ReadFromBin(0,buffer);
            tuple->SetTupleId(id); 
            delete[] buffer;
            TYPE* obj;
            LongInt* member,*membNeighbor;
            CcInt* clId,* clType;
            switch (optype)
            {
              case DBSCAN:
              case DISTMERGE:
                tupleBuffer->AppendTuple(tuple);
                 obj = (TYPE*) tuple->GetAttribute(geoPos);
                if(obj->IsDefined()){
                  MEMB_TYP_CLASS* member = new MEMB_TYP_CLASS(obj);
                  member->setTupleId(id);
                  if(optype == DISTMERGE){
                    clId = (CcInt*) tuple->GetAttribute(clIdPos);
                    member->setClusterNo(clId->GetIntval());
                    clType = (CcInt*) tuple->GetAttribute(clTypePos);
                    member->setClusterType(clType->GetIntval());
                  }
                  if(setPictureXYRef){
                    member->setCoordinates(
                      (TYPE*) tuple->GetAttribute(xPicRefPos),
                      ((CcReal*) 
                      tuple->GetAttribute(xPicRefPos+1))->GetValue());
                  }
                  membArrayUnt.push_back(member);
                  membArray.push_back(member);
                }
                break;
                
              case NEIGHBOR:
                member = (LongInt*) tuple->GetAttribute(0);
                membNeighbor = (LongInt*) tuple->GetAttribute(1);
                if(member->IsDefined() && membNeighbor->IsDefined()){
                  membArrayUnt[member->GetValue()]->addNeighbor( 
                  membArrayUnt[membNeighbor->GetValue()]);
                }
                break;
                
              case DISTSORT:
                tupleBuffer->AppendTuple(tuple);
                break;
                
              case DISTSORTSAMP:
                 obj = (TYPE*) tuple->GetAttribute(geoPos);
                if(obj->IsDefined()){
                  MEMB_TYP_CLASS* member = new MEMB_TYP_CLASS(obj);
                  membArray.push_back(member);
                }
                break;
                
            }
            tuple->DeleteIfAllowed();
            id++;
          }else {
            delete [] buffer;
          }
        } 
      }
      
    }else {
      return false;
    }
    if(tt)
      tt->DeleteIfAllowed();
    if(ftt)
      ftt->DeleteIfAllowed();
    return true;
  }

  
  
/*
1.4  ~writeHeader~
 
*/
  bool writeHeader(std::ofstream& out, std::string& filename, 
                   ListExpr type, std::string& errMsg)
  {
    out.open(filename.c_str(),std::ios::out|std::ios::binary);
    bool ok = out.good();
    char*  buffer = new char[FILE_BUFFER_SIZE];
    out.rdbuf()->pubsetbuf(buffer, FILE_BUFFER_SIZE);
    if(ok){
      
      if(!Relation::checkType(type)){
        errMsg = "invalid relation type " + nl->ToString(type);
        return false;
      }
      
      std::string relTypeS = nl->ToString(type);
      uint32_t length = relTypeS.length();
      std::string magic = "srel";
      out.write(magic.c_str(),4);
      out.write((char*) &length, sizeof(uint32_t));
      out.write(relTypeS.c_str(), length);
    }
    delete [] buffer;
    return ok;
  }
  
/*
1.5 ~writeNextTuple~

Write a given tuple in a file.

*/
  bool writeNextTuple(std::ostream& out,Tuple* tuple)
  {
    // retrieve sizes
    size_t coreSize;
    size_t extensionSize;
    size_t flobSize;
    size_t blocksize = tuple->GetBlockSize(coreSize, extensionSize, 
                                           flobSize);
    // allocate buffer and write flob into it
    char* buffer = new char[blocksize];
    tuple->WriteToBin(buffer, coreSize, extensionSize, flobSize); 
    uint32_t tsize = blocksize;
    TupleId id = out.tellp();
    tuple->SetTupleId(id);
    out.write((char*) &tsize, sizeof(uint32_t));
    out.write(buffer, tsize);
    delete[] buffer;
    return out.good();
  }
  
/*
1.6 ~finish~

Finish file for storing.

*/
  bool finish(std::ostream& out)
  {
    uint32_t marker = 0;
    out.write((char*) &marker, sizeof(uint32_t));
    return out.good();
  }
  
} 
#endif
