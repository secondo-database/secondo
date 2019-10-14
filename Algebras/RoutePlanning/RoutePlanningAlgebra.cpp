/*
----
This file is part of SECONDO.

Copyright (C) 2017, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software{} you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation{} either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY{} without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO{} if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

*/

#include "Stream.h"
#include "LogMsg.h"
#include "Tools/Flob/DbArray.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Algebras/Rectangle/RectangleAlgebra.h"
#include "Attribute.h"
#include "Algebra.h"
#include "NestedList.h"
#include "NList.h"
#include "AlgebraManager.h"
#include "Operator.h"
#include "StandardTypes.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "QueryProcessor.h"
#include "ConstructorTemplates.h"
#include "ShortestPathLF.h"
#include "Symbols.h"
#include "ListUtils.h"
#include "LCompose.h"
#include "Algebras/Collection/CollectionAlgebra.h"

extern NestedList *nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

using namespace std;

namespace routeplanningalgebra {

/*
Type constructors Cpoint, Cpoints, Cpointnode, PointCloud and
operator importpointcloud added and maintained 
by Gundula Swidersky, Dec 2017.

Pointcloud and related types exported into PointcloudAlgebra

Type Mappings

*/

    ListExpr shortestpathlfTM(ListExpr args) {
       ListExpr lrealList = nl->TheEmptyList();
       ListExpr firstPointList = nl->TheEmptyList();
       ListExpr secondPointList = nl->TheEmptyList();
       ListExpr prefs = nl->TheEmptyList();
       int argNo = nl->ListLength(args);
        
        if(argNo != 4 && argNo != 6){ 
            return listutils::typeError(
                "shortestpathlf expects 4 or 6 arguments");
        }
        
        //Check for undefined elements
        if(listutils::isSymbolUndefined(nl->First(args)) ||
           listutils::isSymbolUndefined(nl->Second(args)) ||
           listutils::isSymbolUndefined(nl->Third(args)) ||
           listutils::isSymbolUndefined(nl->Fourth(args)))
        {
            return listutils::typeError("undefined elements in query");
        }
        
        ListExpr orelList = nl->First(args);
        ListExpr slineList = nl->Second(args);
        
        //enable use with and without height data
        switch(argNo){
            case 4:
                firstPointList = nl->Third(args);
                secondPointList = nl->Fourth(args);
                break;
            case 6:
                if(listutils::isSymbolUndefined(nl->Fifth(args)) ||
                   listutils::isSymbolUndefined(nl->Sixth(args))){
                    return listutils::typeError(
                        "fifth or sixth arg is undefined");}
                
                    lrealList = nl->Third(args);
                    firstPointList = nl->Fourth(args);
                    secondPointList = nl->Fifth(args);
                    prefs = nl->Sixth(args);
                break;
        }           
        
        //Check of first argument
        if(!listutils::isOrelDescription(orelList)){
           return listutils::typeError("expects orel as 1. argument");
        }
        
        if(listutils::isSymbolUndefined(nl->Second(orelList))){
            return listutils::typeError("undefined tuple description");
        }
           
        ListExpr orelTuple = nl->Second(orelList);
        if (!listutils::isTupleDescription(orelTuple)){
            return listutils::typeError("second value is not of type tuple");
        }

        if(listutils::isSymbolUndefined(nl->Second(orelTuple))){
            return listutils::typeError("attribute list is undefined");
        }
          
        ListExpr orelAttrList(nl->Second(orelTuple));
        if (!listutils::isAttrList(orelAttrList)){
            return listutils::typeError("Error in orel attrlist.");
        }
        
        //Extract attribute indices for use in value mapping
        string sourceString = "Source";
        ListExpr sourceType;
        string targetString = "Target";
        ListExpr targetType;
        string sourcePosString = "SourcePos";
        ListExpr sourcePosType;
        string targetPosString = "TargetPos";
        ListExpr targetPosType;
        
        int sourceIndex = listutils::findAttribute(orelAttrList, 
                                                   sourceString, sourceType);
        if(sourceIndex <= 0){
            return listutils::typeError("Attribute Source not found");
        }
        else{
            if(nl->SymbolValue(sourceType) != "int"){
                return listutils::typeError("Attribute Source is not int");
            }
        }
        
        int targetIndex = listutils::findAttribute(orelAttrList, 
                                                   targetString, targetType);
        if(targetIndex <= 0){
            return listutils::typeError("Attribute Target not found");
        }
        else{
            if(nl->SymbolValue(targetType) != "int"){
                return listutils::typeError("Attribute Target is not int");
            }
        }
        
        int sourcePosIndex = listutils::findAttribute(orelAttrList, 
             sourcePosString, sourcePosType);
        if(sourcePosIndex <= 0){
            return listutils::typeError("Attribute SourcePos not found");
        }
        else{
            if(nl->SymbolValue(sourcePosType) != "point"){
                return listutils::typeError("Attribute SourcePos is not point");
            }
        }
        
        int targetPosIndex = listutils::findAttribute(orelAttrList, 
             targetPosString, targetPosType);
        if(targetPosIndex <= 0){
            return listutils::typeError("Attribute TargetPos not found");
        }
        else{
            if(nl->SymbolValue(targetPosType) != "point"){
                return listutils::typeError("Attribute TargetPos is not point");
            }
        }
        
        //Check of second argument
        string stringValue = nl->SymbolValue(slineList);
        ListExpr slineType;
        int slineIndex = listutils::findAttribute(orelAttrList, 
             stringValue, slineType);    
        if(slineIndex <= 0){
            return listutils::typeError("Attribute not found");
        }
        else{
        //Check that type is sline
            if (nl->SymbolValue(slineType) != "sline"){
                return listutils::typeError("second a. must be sline");
            }
        }
        
        //Check of lreal argument
        int lrealIndex = -1;
        if(argNo == 6){
            stringValue = nl->SymbolValue(lrealList);
            ListExpr lrealType;
            lrealIndex = listutils::findAttribute(orelAttrList,
                stringValue, lrealType);
            if(lrealIndex <=0){
                return listutils::typeError("Attribute not found");
            }
            else{
                //Check that type is lreal
                if (nl->SymbolValue(lrealType) != "lreal"){
                    return listutils::typeError("Third a. must be lreal");
                }
            }
        }
    
        //Check of first point argument
        if(!Point::checkType(firstPointList)){
            if(argNo == 4){
                return listutils::typeError("Third a. must be point");}
            else{
                return listutils::typeError("Fourth a. must be point");}
        }
        
        //Check of second point argument
        if(!Point::checkType(secondPointList)){
            if(argNo == 4){
                return listutils::typeError("Fourth a. must be point");}
            else{
                return listutils::typeError("Sixth a. must be point");}
        }
        
        //Secondo syntax: [const vector(int) value(1 2 3 4 5)]
        //Check user preferences
        if(argNo == 6){
           if(!Vector::checkType(prefs)){
                return listutils::typeError("Prefs must be vector");}
            if(nl->SymbolValue(nl->Second(prefs)) != "int"){
                return listutils::typeError(
                    "Prefs vector must contain ints");}
        }
            
        //Make correct return value: tuple stream
        //Copy tuple attributes
        NList extOrelAttrList(nl->TheEmptyList());
        for (int i = 0; i < nl->ListLength(orelAttrList); i++){
            NList attr(nl->Nth(i+1,orelAttrList));
            extOrelAttrList.append(attr);
        }
        
        ListExpr valueList = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                                        nl->TwoElemList(
                                            nl->SymbolAtom(Tuple::BasicType()),
                                            extOrelAttrList.listExpr()));
        
        //Appending the indices of the sline and lreal attributes 
        //allows for their usage in value mapping
        ListExpr outlist;
        if(argNo == 4){
            outlist = nl->ThreeElemList(
                                 nl->SymbolAtom(Symbols::APPEND()),
                                 nl->FiveElemList(
                                        nl->IntAtom(slineIndex),
                                        nl->IntAtom(sourceIndex),
                                        nl->IntAtom(targetIndex),
                                        nl->IntAtom(sourcePosIndex),
                                        nl->IntAtom(targetPosIndex)
                                 ),
                                 valueList);
        }
        else{
            outlist = nl->ThreeElemList(
                nl->SymbolAtom(Symbols::APPEND()),
                nl->SixElemList(
                    nl->IntAtom(slineIndex),
                    nl->IntAtom(sourceIndex),
                    nl->IntAtom(targetIndex),
                    nl->IntAtom(sourcePosIndex),
                    nl->IntAtom(targetPosIndex),
                    nl->IntAtom(lrealIndex)
                ),
                valueList);}
        
        return outlist;
    }
    
     /*
    Value Mapping
    */
    
    int shortestpathlfVM(
            Word *args, Word &result, int message, Word &local, Supplier s) {

        shortestpathlfLI* li = (shortestpathlfLI*) local.addr;
        
        switch(message){
            case OPEN: {
                if(li){delete li;}
                ListExpr tupleType = GetTupleResultType(s);
                int noArgs = qp->GetNoSons(s);
                OrderedRelation* orel = (OrderedRelation*) args[0].addr;
                Point* startPoint=0;
                Point* endPoint=0;
                int sourceIdx, targetIdx;
                int sourcePosIdx, targetPosIdx;
                int slineIdx, lrealIdx;
                collection::Collection* preferences = 0;
                switch(noArgs){
                    case 9:
                        startPoint = (Point*) args[2].addr;
                        endPoint = (Point*) args[3].addr;
                        slineIdx =(((CcInt*)args[4].addr)->GetIntval())-1;
                        sourceIdx =(((CcInt*)args[5].addr)->GetIntval())-1; 
                        targetIdx =(((CcInt*)args[6].addr)->GetIntval())-1;
                        sourcePosIdx =(((CcInt*)args[7].addr)->GetIntval())-1;
                        targetPosIdx =(((CcInt*)args[8].addr)->GetIntval())-1;
                        li = new shortestpathlfLI(orel, tupleType, sourceIdx,
                                                  targetIdx, sourcePosIdx, 
                                                  targetPosIdx, slineIdx, -1);
                        break;
                    case 12:
                        startPoint = (Point*) args[3].addr;
                        endPoint = (Point*) args[4].addr;
                        preferences = (collection::Collection*) args[5].addr;
                        slineIdx =(((CcInt*)args[6].addr)->GetIntval())-1;
                        sourceIdx =(((CcInt*)args[7].addr)->GetIntval())-1; 
                        targetIdx =(((CcInt*)args[8].addr)->GetIntval())-1;
                        sourcePosIdx =(((CcInt*)args[9].addr)->GetIntval())-1;
                        targetPosIdx =(((CcInt*)args[10].addr)->GetIntval())-1;
                        lrealIdx =(((CcInt*) args[11].addr)->GetIntval())-1;
                        //check correctness of user preferences, in
                        //case of error, process without prefs
                        if(preferences->GetNoUniqueComponents() != 5){
                            preferences = 0;
                            cout << "faulty prefs vector";
                            cout << " fallback to base processing" << endl;
                        }
                        else{
                            CcInt* First = (CcInt*) 
                                           preferences->GetComponent(0);
                            int first = First->GetIntval();
                            delete First;
                            CcInt* Second=(CcInt*) 
                                           preferences->GetComponent(1); 
                            int second = Second->GetIntval();
                            delete Second;
                            CcInt* Third=(CcInt*) 
                                          preferences->GetComponent(2); 
                            int third = Third->GetIntval();
                            delete Third;
                            CcInt* Fourth =(CcInt*) 
                                           preferences->GetComponent(3) ;
                            int fourth = Fourth->GetIntval();
                            delete Fourth;
                            CcInt* Fifth = (CcInt*) 
                                           preferences->GetComponent(4);
                            int fifth = Fifth->GetIntval();
                            delete Fifth;
                            if(first < 0 || second < 0 || third < 0 
                               || fourth < 0 || fifth < 0 ){
                                preferences = 0;
                                cout << "faulty prefs vector";
                                cout << " fallback to base processing" << endl;
                                }
                            }
                            li = new shortestpathlfLI(orel, tupleType, 
                                              sourceIdx, targetIdx, 
                                              sourcePosIdx, 
                                              targetPosIdx, slineIdx, lrealIdx);
                            break;
                       }
                local.setAddr(li);
                li->getShortestPath(startPoint, endPoint, preferences);
                return 0;}
            case REQUEST: {
               result.addr = li?li->getNext():0;
               return result.addr?YIELD:CANCEL; }
            case CLOSE:{
                if(li){
                    delete li;
                    local.addr = 0;
                }
                return 0; }
        }       
        return 0;
    }

    /*
    Operator Specs
    
    */
    
    //Version noch ohne Höhendaten
    OperatorSpec shortestpathlfSpec(
        "orel(tuple(X)) x IDENT x point2 -> stream(tuple(X))",
        "_ shortestpathlf[ _,_,_]",
        "Finds the shortest A* path between 2 tuples",
        "query Edges shortestpathlf[Curve, start, end] consume"
    );
    
    /*
    Operator shortestpathlf
    
    */

    Operator shortestpathlfOp(
        "shortestpathlf",
        shortestpathlfSpec.getStr(),
        shortestpathlfVM,
        Operator::SimpleSelect,
        shortestpathlfTM
    );

    /*
    class RoutePlanningAlgebra
    
    */    
    class RoutePlanningAlgebra : public Algebra {
        public:
        RoutePlanningAlgebra() : Algebra() {

            AddOperator(&LCompose::lcompose, false);
            AddOperator(&LCompose::PointcloudToTin::pointcloud2Tin, false);
            AddOperator(&shortestpathlfOp, false);
            shortestpathlfOp.SetUsesMemory();

        }
        ~RoutePlanningAlgebra() {};
    };
}

extern "C"
Algebra *
InitializeRoutePlanningAlgebra(NestedList *nlRef, QueryProcessor *qpRef) {
    return new routeplanningalgebra::RoutePlanningAlgebra;
}



