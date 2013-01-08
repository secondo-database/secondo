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

August 2004, M. Spiekermann. This comment was inserted to make it a PD-File. Moreover,
implementation of ~GetAlgebraName~ was done.

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hybrid~). Only the executable
level remains. Models are also removed from type constructors.

January 2006. M. Spiekermann TypeCheck will now return a boolean value instead
of a pointer to a function.

*/

#include <string>
#include <utility>
#include <algorithm>
#include <fstream>

using namespace std;

#include "AlgebraManager.h"
#include "Algebra.h"
#include "CharTransform.h"
#include "SecondoSystem.h"
#include "DynamicLibrary.h"
#include "SystemTables.h"
#include "Symbols.h"

AlgebraManager::AlgebraManager( NestedList& nlRef,
                                GetAlgebraEntryFunction getAlgebraEntryFunc )
{
  int j = 0;
  nl = &nlRef;
  getAlgebraEntry = getAlgebraEntryFunc;
  maxAlgebraId = 0;

  for ( j = 0; (*getAlgebraEntry)( j ).algebraId > 0; j++ )
  {
    if ( (*getAlgebraEntry)( j ).useAlgebra )
    {
      if ( (*getAlgebraEntry)( j ).algebraId > maxAlgebraId )
      {
        maxAlgebraId = (*getAlgebraEntry)( j ).algebraId;
      }
    }
  }
  algebra.resize( maxAlgebraId+1 );
  for ( j = 0; j <= maxAlgebraId; j++ )
    algebra[j] = 0;

}

AlgebraManager::~AlgebraManager()
{
  // delete algebra pointers
  for(unsigned int i=0;i<algebra.size();i++){
      delete algebra[i];
      algebra[i] = 0;
  }

}

ListExpr
AlgebraManager::ListAlgebras()
{
  int j = 0;
  ListExpr list = nl->Empty();
  ListExpr lastElem = nl->Empty();

  for ( j = 0; (*getAlgebraEntry)( j ).algebraId > 0; j++ )
  {
    if ( (*getAlgebraEntry)( j ).useAlgebra )
    {
      if ( (*getAlgebraEntry)( j ).algebraInit != 0 )
      {
        ListExpr alg = nl->SymbolAtom( (*getAlgebraEntry)( j ).algebraName );
        if ( list == nl->TheEmptyList() )
        {
          list = nl->OneElemList( alg );
          lastElem = list;
        }
        else
        {
          lastElem = nl->Append( lastElem, alg );
        }
      }
    }
  }
  return (list);
}


int
AlgebraManager::GetAlgebraId(const string& algName)
{
  int j;

  for ( j = 0; (*getAlgebraEntry)( j ).algebraId > 0; j++ )
  {
    if ( (*getAlgebraEntry)( j ).useAlgebra )
    {
      if ( (*getAlgebraEntry)( j ).algebraInit != 0 )
      {
        if ( !(algName.compare( (*getAlgebraEntry)( j ).algebraName )) )
	        return (*getAlgebraEntry)( j ).algebraId;
      }
    }
  }
  return 0;
}

const string&
AlgebraManager::GetAlgebraName( const int algId ) {

  static const string unknown("UnknownAlgebra");
  assert( algId >= 0 && algId <= maxAlgebraId );
  map<int, string>::const_iterator it = algebraNames.find(algId);

  if ( it != algebraNames.end() ) {
    return it->second;
  } else {
    return unknown;
  }
}

void
AlgebraManager::LoadAlgebras()
{
  QueryProcessor* qp = SecondoSystem::GetQueryProcessor();
  TypeConstructor* tc = 0;
  int j = 0, k = 0;

  for ( j = 0; (*getAlgebraEntry)( j ).algebraId > 0; j++ )
  {
    string algNameStr = (*getAlgebraEntry)( j ).algebraName;
    int algId = (*getAlgebraEntry)( j ).algebraId;

    if ( (*getAlgebraEntry)( j ).useAlgebra )
    {
      if ( (*getAlgebraEntry)( j ).algebraInit != 0 )
      {
        algebraNames[algId] = algNameStr;
        algebra[algId] = ((*getAlgebraEntry)( j ).algebraInit)( nl, qp, this );
      }
      else
      {
        bool loaded = false;
        string libraryName  = string( "lib" ) + algNameStr;
        string initFuncName = string( "Initialize" ) + algNameStr;
        (*getAlgebraEntry)( j ).dynlib = new DynamicLibrary();
        transform( libraryName.begin(), libraryName.end(),
                   libraryName.begin(), ToLowerProperFunction );
        if ( (*getAlgebraEntry)( j ).dynlib->Load( libraryName ) )
        {
          AlgebraInitFunction initFunc =
            (AlgebraInitFunction) (*getAlgebraEntry)( j ).dynlib
                                          ->GetFunctionAddress( initFuncName );
          if ( initFunc != 0 )
          {
            algebra[(*getAlgebraEntry)( j ).algebraId]
                                            = (initFunc)( nl, qp, this );
            loaded = true;
          }
          else
          {
            (*getAlgebraEntry)( j ).dynlib->Unload();
          }
        }
        if ( !loaded )
        {
          delete (*getAlgebraEntry)( j ).dynlib;
          (*getAlgebraEntry)( j ).dynlib = 0;
          continue;
        }
      }
      for ( k = 0;
            k < algebra[(*getAlgebraEntry)( j ).algebraId]->GetNumTCs(); k++ )
      {
        tc = algebra[(*getAlgebraEntry)( j ).algebraId]
                                                   ->GetTypeConstructor( k );

	NList::setNLRef(nl);
	//tc ->initKindDataProperties();
        for ( vector<string>::size_type idx = 0;
              idx < tc->kinds.size(); idx++      )
        {
          if ( tc->kinds[idx] != "" )
          {
            kindTable.insert( make_pair( tc->kinds[idx], tc->typeCheckFunc ) );
          }
        }
      }
    }
  }
  InitOpPtrField();
}

void
AlgebraManager::UnloadAlgebras()
{
  for ( int j = 0; (*getAlgebraEntry)( j ).algebraId > 0; j++ )
  {
    if ( (*getAlgebraEntry)( j ).useAlgebra
         && (*getAlgebraEntry)( j ).dynlib != 0 )
    {
      (*getAlgebraEntry)( j ).dynlib->Unload();
      delete (*getAlgebraEntry)( j ).dynlib;
      (*getAlgebraEntry)( j ).dynlib = 0;
    }
  }
}

bool
AlgebraManager::IsAlgebraLoaded( const int algebraId )
{
  return algebraNames.find( algebraId ) != algebraNames.end();
}

int
AlgebraManager::CountAlgebra()
{
  int count = 0;
  for ( int j = 1; j <= maxAlgebraId; j++ )
  {
    if( IsAlgebraLoaded( j ) )
      count++;
  }
  return (count);
}

bool
AlgebraManager::NextAlgebraId( int& algebraId )
{
  bool found = false;
  algebraId++;
  if ( algebraId > 0 && algebraId <= maxAlgebraId )
  {
    while ( !found && algebraId <= maxAlgebraId )
    {
      if( IsAlgebraLoaded( algebraId ) )
      {
        found = true;
      }
      else
      {
        algebraId++;
      }
    }
  }
  else
    algebraId = 0;
  return (found);
}

int
AlgebraManager::OperatorNumber( int algebraId )
{
  return (algebra[algebraId]->GetNumOps());
}

void
AlgebraManager::InitOpPtrField() {

  opPtrField.resize(maxAlgebraId+1);
  for ( int alg = 0; alg <= maxAlgebraId; alg++ ) {

    int numOps = 0;
    if (algebra[alg] != 0) {
    numOps = algebra[alg]->GetNumOps();
    opPtrField[alg].resize(numOps+1);
    for ( int op = 0; op < numOps; op++ ) {
      opPtrField[alg][op] = algebra[alg]->GetOperator( op );
    }
    }
  }
}

int
AlgebraManager::ConstrNumber( int algebraId )
{
  return (algebra[algebraId]->GetNumTCs());
}

TypeConstructor*
AlgebraManager::GetTC( int algebraId, int typeId )
{
  return (algebra[algebraId]->GetTypeConstructor( typeId ));
}


Operator*
AlgebraManager::GetOP( int algebraId, int opId )
{
  if(algebraId < 0 || algebraId >= (int)algebra.size()){
    return 0;
  }
  Algebra* alg = algebra[algebraId];
  if(!alg){
    return 0;
  }
  return  alg->GetOperator(opId);
}


InObject
AlgebraManager::InObj( int algebraId, int typeId )
{
  return (algebra[algebraId]->GetTypeConstructor( typeId )->inFunc);
}

OutObject
AlgebraManager::OutObj( int algebraId, int typeId )
{
  return (algebra[algebraId]->GetTypeConstructor( typeId )->outFunc);
}

OutObject
AlgebraManager::SaveToListObj( int algebraId, int typeId )
{
  if( algebra[algebraId]->GetTypeConstructor( typeId )
                                      ->saveToListFunc != 0 )
    return (algebra[algebraId]->GetTypeConstructor( typeId )
                                               ->saveToListFunc);
  else
    return (algebra[algebraId]->GetTypeConstructor( typeId )->outFunc);
}

InObject
AlgebraManager::RestoreFromListObj( int algebraId, int typeId )
{
  if( algebra[algebraId]->GetTypeConstructor( typeId )
                                 ->restoreFromListFunc != 0 )
    return (algebra[algebraId]->GetTypeConstructor( typeId )
                                           ->restoreFromListFunc);
  else
    return (algebra[algebraId]->GetTypeConstructor( typeId )->inFunc);
}

ObjectCreation
AlgebraManager::CreateObj( int algebraId, int typeId )
{
  return (algebra[algebraId]->GetTypeConstructor( typeId )->createFunc);
}

ObjectDeletion
AlgebraManager::DeleteObj( int algebraId, int typeId )
{
  return (algebra[algebraId]->GetTypeConstructor( typeId )->deleteFunc);
}

bool
AlgebraManager::OpenObj( const int algebraId, const int typeId,
                         SmiRecord& valueRecord, size_t& offset,
                         const ListExpr typeInfo, Word& value )
{
  return (algebra[algebraId]->GetTypeConstructor( typeId )->
    Open( valueRecord, offset, typeInfo, value ));
}

bool
AlgebraManager::SaveObj( const int algebraId, const int typeId,
                         SmiRecord& valueRecord, size_t& offset,
                         const ListExpr typeInfo, Word& value )
{
  return (algebra[algebraId]->GetTypeConstructor( typeId )->
    Save( valueRecord, offset, typeInfo, value ));
}

ObjectClose
AlgebraManager::CloseObj( int algebraId, int typeId )
{
  return (algebra[algebraId]->GetTypeConstructor( typeId )->closeFunc);
}

ObjectClone
AlgebraManager::CloneObj( int algebraId, int typeId )
{
  return (algebra[algebraId]->GetTypeConstructor( typeId )->cloneFunc);
}

ObjectCast
AlgebraManager::Cast( int algebraId, int typeId )
{
  return (algebra[algebraId]->GetTypeConstructor( typeId )->castFunc);
}

ObjectSizeof
AlgebraManager::SizeOfObj( int algebraId, int typeId )
{
  return (algebra[algebraId]->GetTypeConstructor( typeId )->sizeofFunc);
}

bool
AlgebraManager::TypeCheck( int algebraId, int typeId,
                           const ListExpr type, ListExpr& errorInfo )
{
  return (algebra[algebraId]->GetTypeConstructor( typeId )
                                 ->TypeCheck(type, errorInfo));
}

bool
AlgebraManager::CheckKind( const string& kindName,
                           const ListExpr type,
                           ListExpr& errorInfo )
{
  ListExpr tempErrorInfo = errorInfo;
  multimap<string,TypeCheckFunction>::iterator pos;
  for ( pos  = kindTable.lower_bound( kindName );
        pos != kindTable.upper_bound( kindName );
        pos++ )
  {
    if ( (pos->second)( type, errorInfo ) )
    {
      errorInfo = tempErrorInfo;
      return (true);
    }
  }
  errorInfo = nl->Append(
                errorInfo,
                nl->ThreeElemList(
                  nl->IntAtom( 60 ),
                  nl->SymbolAtom( kindName ),
                  type ) );
  return (false);
}

void
AlgebraManager::UpdateOperatorUsage(SystemInfoRel* table) {

  table->clear();
  int algId = 0;
  while ( NextAlgebraId( algId ) )
  {
    const int N = OperatorNumber(algId);
    for (int opId = 0; opId < N; opId++) {

      string name = GetAlgebraName(algId);
      string algebra = GetOP(algId, opId)->GetName();
      Operator* op = opPtrField[algId][opId];
      if(op){
        for(int i=0;i<op->GetNumOfFun();i++){
           OperatorUsageTuple* t = new OperatorUsageTuple;
           t->algebra = algebra;
           t->name = name;
           t->vmid = i;
           t->calls = op->GetCalls(i);
           table->append(t,false);
         }
      }
    }
  }
}



vector< pair< pair<int, int>, ListExpr> >
      AlgebraManager::matchingOperators(const ListExpr arguments){

   vector< pair< pair<int,int>, ListExpr> > result;
   result.clear();
   for(unsigned int a=1 ; a<algebra.size() ; a++){ // algId=0 is prohibited
     matchingOperators(a, arguments, result);
   }
   return result;
}

void AlgebraManager::matchingOperators(const int algId,
                                       const ListExpr arguments,
                       vector< pair< pair<int,int>, ListExpr> >& result){
  assert( (algId>0) && (algId<(int)algebra.size()) ); // 0 is an invalid algId!
  ListExpr typeError = nl->SymbolAtom(Symbol::TYPEERROR());
  Algebra* alg = algebra[algId];
  if(alg!=0){
    for(int o=0 ; o<alg->GetNumOps() ; o++){
      Operator* op = alg->GetOperator(o);
      try{
          ListExpr res = op->CallTypeMapping(arguments);
         // cout << "Check finished" << endl << endl;
          if(!nl->Equal(res,typeError)){
            pair<int, int> p1(algId,o);
            pair<pair<int, int>, ListExpr> p(p1, res);
            result.push_back(p);
          }
      } catch (...){
          cerr << "Problem in Typemapping of operator " << op->GetName()
               << " in Algebra" << GetAlgebraName(algId) << endl;
          cerr << "Throws an exception when called with "
               << nl->ToString(arguments) << endl;
      }
    }
  }
}

void AlgebraManager::findTMExceptions(const string& algName,
                                      const ListExpr argList,
                                      queue<pair<string,string> >& q,
                                      const bool print) {

   if(algName.size()==0){
     for(unsigned int a=1 ; a<algebra.size() ; a++){ // algId=0 is prohibited
       Algebra* alg = algebra[a];
       if(alg!=0){
          if(print){
              cout << "process algebra" << GetAlgebraName(a) << endl;
          }
          for(int o=0;o<alg->GetNumOps(); o++){
             Operator* op = alg->GetOperator(o);
             if(print){
               cout << "process operator " << op->GetName() << endl; 
             }  
             try{
               op->CallTypeMapping(argList);
             } catch(...){
               pair<string,string> p(GetAlgebraName(a), op->GetName());
               q.push(p);  
             }
          }
       }    
     }
   } else {
     int a = GetAlgebraId(algName);
     if(a<1){
        if(print){
          cout << "Algebra " << algName << " not found" << endl;
        }
        return;
     }
     if(print){
         cout << "process algebra" << GetAlgebraName(a) << endl;
     }
     Algebra* alg = algebra[a];
     for(int o=0;o<alg->GetNumOps(); o++){
        Operator* op = alg->GetOperator(o);
        if(print){
          cout << "process operator " << op->GetName() << endl; 
        }  
        try{
          op->CallTypeMapping(argList);
        } catch(...){
          pair<string,string> p(GetAlgebraName(a), op->GetName());
          q.push(p);  
        }
     }
   }
}


bool AlgebraManager::findOperator(const string& name,
                                  const ListExpr argList,
                                  ListExpr& resultList,
                                  int& algId,
                                  int& opId){
 int funId=0;
 return findOperator(name,argList, resultList, algId, opId,funId);
}


bool AlgebraManager::findOperator(const string& name,
                                  const ListExpr argList,
                                  ListExpr& resultList,
                                  int& algId,
                                  int& opId,
                                  int& funId){

   ListExpr typeError = nl->SymbolAtom(Symbol::TYPEERROR());

   NestedList* nl_orig = NList::getNLRef();
   NList::setNLRef(nl);

   for(unsigned int a=0;a<algebra.size();a++){
     Algebra* alg = algebra[a];
     if(alg!=0){
       for(int o=0; o< alg->GetNumOps(); o++){
         Operator* op = alg->GetOperator(o);
         if(op->GetName() == name){
           try{
              ListExpr res = op->CallTypeMapping(argList);
              if(!nl->Equal(res,typeError)){ //  appropriate operator found
                 algId = a;
                 opId = o;
                 funId = op->Select(argList);
                 resultList = res;
                 NList::setNLRef(nl_orig);
                 return true;
              }
           } catch (...){
              cerr << "Problem in Typemapping of operator " << op->GetName()
                   << " in Algebra" << GetAlgebraName(a) << endl;
              cerr << "Throws an exception when called with "
                   << nl->ToString(argList) << endl;
           }
        }
      }
    }
  }
  algId = 0;
  opId = 0;
  resultList = nl->TheEmptyList();
  NList::setNLRef(nl_orig);
  return false;
}


/*
~getCostEstimation~

Returns the costestimation for a specified value mapping.
If there is no one, null is returned.

*/

CostEstimation* AlgebraManager::getCostEstimation( const int algId,
                                   const int opId,
                                   const int funId){
   Operator* op = getOperator(algId,opId);
   return op?op->getCostEstimation(funId):0;
}

/*
~getCosts~

The next functions return costs for a specified operator when number of tuples
and size of a single tuple is given. If the operator does not provide a
cost estimation function or the getCost function is not implemented,
the return value is false.

*/

bool AlgebraManager::getCosts(const int algId,
              const int opId,
              const int funId,
              const size_t noTuples,
              const size_t sizeOfTuple,
              const size_t memoryMB,
              size_t& costs){


   CostEstimation* ce = getCostEstimation(algId,opId,funId);
   costs = 0;
   double mem=memoryMB; double co=0;
   bool res = ce?ce->getCosts(noTuples,sizeOfTuple,mem,co):false;
   if(co>0 && co <1){
     costs = 1;
   } else {
     costs = (size_t) co;
   }
   return res;
}


bool AlgebraManager::getCosts(const int algId,
              const int opId,
              const int funId,
              const size_t noTuples1,
              const size_t sizeOfTuple1,
              const size_t noTuples2,
              const size_t sizeOfTuple2,
              const size_t memoryMB,
              size_t& costs) {
   CostEstimation* ce = getCostEstimation(algId,opId,funId);
   costs = 0;
   double mem = memoryMB;
   double co=0;
   bool res = ce?ce->getCosts(noTuples1,sizeOfTuple1,
                          noTuples2,sizeOfTuple2,mem,co)
            :false;
   if(co>0 && co <1){
     costs = 1;
   } else {
     costs = (size_t) co;
   }
   return  res;
}

/*
~getLinearParams~

Retrieves the parameters for estimating the cost function of an operator
in a linear way.

*/
bool AlgebraManager::getLinearParams( const int algId,
                      const int opId,
                      const int funId,
                      const size_t noTuples1,
                      const size_t sizeOfTuple1,
                      double& sufficientMemory,
                      double& timeAtSuffMemory,
                      double& timeAt16MB) {

   CostEstimation* ce = getCostEstimation(algId,opId,funId);
   sufficientMemory = 0;
   timeAtSuffMemory = 0; 
   timeAt16MB = 0;
   return ce?ce->getLinearParams(noTuples1,sizeOfTuple1,
                                 sufficientMemory,timeAtSuffMemory,timeAt16MB)
            :false;
}


bool AlgebraManager::getLinearParams( const int algId,
                      const int opId,
                      const int funId,
                      const size_t noTuples1,
                      const size_t sizeOfTuple1,
                      const size_t noTuples2,
                      const size_t sizeOfTuple2,
                      double& sufficientMemory,
                      double& timeAtSuffMemory,
                      double& timeAt16MB){
   CostEstimation* ce = getCostEstimation(algId,opId,funId);
   sufficientMemory = 0;
   timeAtSuffMemory = 0; 
   timeAt16MB = 0;
   return ce?ce->getLinearParams(noTuples1,sizeOfTuple1,noTuples2,sizeOfTuple2,
                                 sufficientMemory,timeAtSuffMemory,timeAt16MB)
            :false;
}

/*
~getFunction~

Returns an approximation of the cost function of a specified value mapping as
a parametrized function.

*/
bool AlgebraManager::getFunction(
                 const int algId,
                 const int opId,
                 const int funId,
                 const size_t noTuples,
                 const size_t sizeOfTuple,
                 int& funType,
                 double& sufficientMemory,
                 double& timeAtSuffMemory,
                 double& timeAt16MB,
                 double& a, double& b, double&c, double& d){

   CostEstimation* ce = getCostEstimation(algId,opId,funId);
   funType = -1;
   sufficientMemory = 0;
   timeAtSuffMemory = 0; 
   timeAt16MB = 0;
   a = b = c = d = 0;
   return ce?ce->getFunction(noTuples, sizeOfTuple,
                             funType, sufficientMemory,
                             timeAtSuffMemory, timeAt16MB,
                             a,b,c,d)
            : false;
}
                      


bool AlgebraManager::getFunction(const int algId,
                 const int opId,
                 const int funId,
                 const size_t noTuples1,
                 const size_t sizeOfTuple1,
                 const size_t noTuples2,
                 const size_t sizeOfTuple2,
                 int& funType,
                 double& sufficientMemory,
                 double& timeAtSuffMemory,
                 double& timeAt16MB,
                 double& a, double& b, double&c, double& d) {

   CostEstimation* ce = getCostEstimation(algId,opId,funId);
   funType = -1;
   sufficientMemory = 0;
   timeAtSuffMemory = 0; 
   timeAt16MB = 0;
   a = b = c = d = 0;
   return ce?ce->getFunction(noTuples1, sizeOfTuple1,
                             noTuples2, sizeOfTuple2,
                             funType, sufficientMemory,
                             timeAtSuffMemory, timeAt16MB,
                             a,b,c,d)
            : false;
}


/*
Return maxAlgebraId

*/
int AlgebraManager::getMaxAlgebraId() const {
  return maxAlgebraId;
}
