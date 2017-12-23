/*
----
This file is part of SECONDO.

Copyright (C) 2008, University in Hagen, Department of Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[ue] [\"u]
//[ae] [\"a]
//[oe] [\"o]
//[x] [$\times $]
//[->] [$\rightarrow $]

[1] Implementation of the MRegionOpsAlgebra

April - November 2008, M. H[oe]ger for bachelor thesis.
Mai - November 2017, U. Wiesecke for master thesis.

[TOC]

1 Introduction

2 Defines and Includes

*/

#include "Selftest.h"
#include "SetOps.h"

#include "Algebra.h"
#include "NestedList.h"
#include "NList.h"
#include "LogMsg.h"
#include "QueryProcessor.h"
#include "ConstructorTemplates.h"
#include "StandardTypes.h"
#include <string>
#include "TypeMapUtils.h"
#include "Symbols.h"

extern NestedList*     nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;

namespace temporalalgebra {
  namespace mregionops3 {
      
/*
3 Type Mapping Functions

*/
    // Type mapping for set operations
    ListExpr setOpTypeMap(ListExpr args) {
      NList type(args);
      const std::string errMsg = "Expecting two movingregions.";
      if (type.length() != 2)
        return NList::typeError(errMsg);
      // movingregion x movingregion -> movingregion
      if (type.first().isSymbol(MRegion::BasicType()) &&
          type.second().isSymbol(MRegion::BasicType())) {
        return NList(MRegion::BasicType()).listExpr();
      }// if  
      return NList::typeError(errMsg);
    }// 
    
    // Type mapping for predicate operations
    ListExpr predicateTypeMap(ListExpr args) {
      NList type(args);
      const std::string errMsg = "Expecting two movingregions.";
      if (type.length() != 2)
        return NList::typeError(errMsg);
      // movingregion x movingregion -> mbool
      if (type.first().isSymbol(MRegion::BasicType()) &&
          type.second().isSymbol(MRegion::BasicType())) {
        return NList(MBool::BasicType()).listExpr();
      }// if  
      return NList::typeError(errMsg);
    }// 
    
    // Type mapping for Operator selftest
    ListExpr selftestTM(ListExpr args){
      std::string err = "No paramters expected";
      if(!nl->HasLength(args,0)){ 
        return listutils::typeError(err);
      }// if
      return listutils::basicSymbol<CcBool>();
    }// TypeMapping  
/*
4 Value Mapping Functions

*/
    // Value mapping for operator Intersection
    int intersectionValueMap(Word* args, Word& result, int message, Word& local,
                             Supplier s) {
      MRegion* mrA = static_cast<MRegion*>(args[0].addr );
      MRegion* mrB = static_cast<MRegion*>(args[1].addr );
      result = qp->ResultStorage(s);
      MRegion* res = static_cast<MRegion*>(result.addr );
      SetOperator so(mrA, mrB, res);
      so.operate(INTERSECTION);
      return 0;
    }// intersectionValueMap
    
    // Value mapping for operator Union
    int unionValueMap(Word* args, Word& result, int message, Word& local,
                      Supplier s) {
      MRegion* mrA = static_cast<MRegion*>(args[0].addr );
      MRegion* mrB = static_cast<MRegion*>(args[1].addr );
      result = qp->ResultStorage(s);
      MRegion* res = static_cast<MRegion*>(result.addr );
      SetOperator so(mrA, mrB, res);
      so.operate(UNION);
      return 0;
    }// unionValueMap

    // Value Mapping for operator Minus
    int minusValueMap(Word* args, Word& result, int message, Word& local,
                    Supplier s) {
      MRegion* mrA = static_cast<MRegion*>(args[0].addr );
      MRegion* mrB = static_cast<MRegion*>(args[1].addr );
      result = qp->ResultStorage(s);
      MRegion* res = static_cast<MRegion*>(result.addr );
      SetOperator so(mrA, mrB, res);
      so.operate(MINUS);
      return 0;
    }// minusValueMap
    
    // Value Mapping for predicate Intersects
    int intersectsValueMap(Word* args, Word& result, int message, Word& local,
                           Supplier s) {
      MRegion* mrA = static_cast<MRegion*>(args[0].addr );
      MRegion* mrB = static_cast<MRegion*>(args[1].addr );
      result = qp->ResultStorage(s);
      MBool* res = static_cast<MBool*>(result.addr );
      PredicateOperator po(mrA, mrB, res);
      po.operate(INTERSECTS);
      return 0;
    }// intersectsValueMap
    
    // Value Mapping for predicate Inside
    int insideValueMap(Word* args, Word& result, int message, Word& local,
                           Supplier s) {
      MRegion* mrA = static_cast<MRegion*>(args[0].addr );
      MRegion* mrB = static_cast<MRegion*>(args[1].addr );
      result = qp->ResultStorage(s);
      MBool* res = static_cast<MBool*>(result.addr );
      PredicateOperator po(mrA, mrB, res);
      po.operate(INSIDE);
      return 0;
    }// insideValueMap
    
    // Value mapping for operator Selftest
    int mregionops3testVM(Word* args, Word& result, int message, Word& local, 
                  Supplier s){ 
      Selftest test;
      bool res;
      res = test.run();
      result = qp->ResultStorage(s);
      CcBool* b = (CcBool*) result.addr;    
      b->Set(true,res);
      return 0;
    }// mregionops3testVM
/*
5 Operator Descriptions

*/
    // Description for operator Intersection
    struct IntersectionInfo:OperatorInfo {
      IntersectionInfo() {
        name      = "intersection";
        signature = "mregion x mregion -> mregion";
        syntax    = "intersection(_, _)";
        meaning   = "Intersection operation for two moving regions.";
      }// IntersectionInfo
    };//struct IntersectionInfo
    
    // Description for operator Union
    struct UnionInfo:OperatorInfo {
      UnionInfo() {
        name      = "union";
        signature = "mregion x mregion -> mregion";
        syntax    = "_ union _";
        meaning   = "Union operation for two moving regions.";
      }// UnionInfo
    };//  struct UnionInfo

    struct MinusInfo:OperatorInfo {
      MinusInfo() {
        name      = "minus";
        signature = "mregion x mregion -> mregion";
        syntax    = "_ minus _";
        meaning   = "Minus operation for two moving regions.";
      }// MinusInfo            
    };// struct MinusInfo   
    
    struct IntersectsInfo:OperatorInfo {
      IntersectsInfo() {
        name      = "intersects";
        signature = "mregion x mregion -> mbool";
        syntax    = "_ intersects _";
        meaning   = "predicate intersects for two moving regions.";
      }// IntersectsInfo            
    };// struct IntersectsInfo  
    
    struct InsideInfo:OperatorInfo {
      InsideInfo() {
        name      = "inside";
        signature = "mregion x mregion -> mbool";
        syntax    = "_ inside _";
        meaning   = "predicate inside for two moving regions.";
      }// InsideInfo            
    };// struct InsideInfo 
        
    struct Mregionops3testInfo:OperatorInfo {
      Mregionops3testInfo(){
        name      = "mregionops3test";
        signature = " x -> bool";
        syntax    = "selftest";
        meaning   = "tests all classes in this algebra.";
      }// Mregionops3testInfo            
    };// struct Mregionops3testInfo 

/*
6 Implementation of the algebra class

*/
    class MRegionOps3Algebra : public Algebra {
    public:
      MRegionOps3Algebra(): Algebra() {
        AddOperator(IntersectionInfo(), intersectionValueMap, setOpTypeMap);
        AddOperator(UnionInfo(), unionValueMap, setOpTypeMap);
        AddOperator(MinusInfo(), minusValueMap, setOpTypeMap);
        AddOperator(Mregionops3testInfo(),mregionops3testVM, selftestTM);
        AddOperator(IntersectsInfo(),intersectsValueMap,predicateTypeMap);
        AddOperator(InsideInfo(),insideValueMap,predicateTypeMap);
      }// Konstruktor
          
      ~MRegionOps3Algebra() {
      }// Destruktor
        
    }; // class MRegionOps3Algebra
    
  } // end of namespace mregionops3
} // end of namespace temporalalgebra

extern "C" Algebra* InitializeMRegionOps3Algebra(NestedList*     nlRef,
                                                 QueryProcessor* qpRef,
                                                 AlgebraManager* amRef) {
  nl = nlRef;
  qp = qpRef;
  am = amRef;
  return new temporalalgebra::mregionops3::MRegionOps3Algebra();
}