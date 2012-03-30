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

July 2004, M. Spiekermann. Counters for type constructors and operators were
introduced to assert correct vector indexes and avoid segmentation faults.

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

January 2006, M. Spiekermann. New constructors for operators and type constructors
added.

*/

using namespace std;

#include "AlgebraManager.h"
#include "Algebra.h"
#include "NestedList.h"
#include "SecondoSystem.h"
#include "Symbols.h"

#include <fstream>

NestedList *nl;
QueryProcessor *qp;
AlgebraManager *am;


OperatorInfo::OperatorInfo( const string& opName, const string& specStr)
{
  assert(nl);
  ListExpr spec = nl->Empty();
  nl->ReadFromString(specStr, spec);
  NList list(spec);
  if ( !list.hasLength(2) ) {
    cout << "Operator: " << opName << endl;
    cout << "specStr: " << specStr << endl;
    cout << "Assuming a list of length 2!" << endl;
    assert(false);
  }
  list = list.second();

  name = opName;
  signature = "";
  syntax = "";
  meaning = "";
  example = "";
  remark ="";

  if (list.length() >= 1)
  signature = list.elem(1).str();
  if (list.length() >= 2)
  syntax = list.elem(2).str();
  if (list.length() >= 3)
  meaning = list.elem(3).str();
  if (list.length() >= 4)
  example = list.elem(4).str();
  if (list.length() >= 5)
  remark = list.elem(5).str();
}


const string
OperatorInfo::str() const {

  const string S("<text>");
  const string E("</text--->");
  const string headStr = "(\"Signature\" \"Syntax\" \"Meaning\" \"Example\")";

  string spec = "(" + headStr + "("
                + S + signature + E
                + S + syntax + E
                + S + meaning + E
                + S + example + E + "))";

  return spec;
}


const ListExpr
OperatorInfo::list() const {

  assert(nl);
  ListExpr spec = nl->Empty();
  nl->ReadFromString(str(), spec);
  return spec;
}


void
OperatorInfo::appendSignature(const string& sig) {

  signature += ", " + sig;
}

ostream& operator<<(ostream& o, const OperatorInfo& oi){
   return oi.Print(o);
}

/* Member functions of class Operator: */

Operator::Operator( const string& nm,
                    const string& specStr,
                    const int noF,
                    ValueMapping vms[],
                    SelectFunction sf,
                    TypeMapping tm,
                    CreateCostEstimation* createCE)
{
  name           = nm;
  specString     = specStr;
  numOfFunctions = noF;
  selectFunc     = sf;
  valueMap       = new ValueMapping[numOfFunctions];
  calls          = new unsigned int[numOfFunctions];
  createCostEstimation = new CreateCostEstimation[numOfFunctions];
  costEstimation = new CostEstimation*[numOfFunctions];
  typeMap        = tm;
  supportsProgress = false;
  requestsArgs   = false;
  usesArgsInTypeMapping = false;
  usesMemory     = false;
  

  for ( int i = 0; i < numOfFunctions; i++ ){
    calls[i] = 0;
    AddValueMapping( i, vms[i] );
    if(createCE){
       createCostEstimation[i] = createCE[i];
       costEstimation[i] = createCE[i]?createCE[i]():0;
    } else {
       createCostEstimation[i] = 0;
       costEstimation[i] = 0;
    }
  }
}

Operator::Operator( const string& nm,
                    const string& specStr,
                    ValueMapping vm,
                    SelectFunction sf,
                    TypeMapping tm,
                    CreateCostEstimation createCE )
{
  name           = nm;
  specString     = specStr;
  selectFunc     = sf;
  if(vm){
     numOfFunctions = 1;
     valueMap       = new ValueMapping[1];
     calls          = new unsigned int[1];
     createCostEstimation = new CreateCostEstimation[1];
     costEstimation = new CostEstimation*[1];
     AddValueMapping( 0, vm );
     calls[0] = 0;
     if(createCE){
       createCostEstimation[0] = createCE;
       costEstimation[0] = createCE?createCE():0;
     } else {
       createCostEstimation[0] = 0;
       costEstimation[0] = 0;
     }
  } else {
     valueMap = 0;
     calls = 0;
     numOfFunctions = 0;
     createCostEstimation = 0;
     costEstimation = 0;
  }
  typeMap        = tm;
  supportsProgress = false;
  requestsArgs   = false;
  usesArgsInTypeMapping = false;
  usesMemory     = false;
}

Operator::Operator( const OperatorInfo& oi,
                    ValueMapping vm,
                    TypeMapping tm,
                    CreateCostEstimation createCE )
{
  // define member attributes
  name           = oi.name;
  specString     = oi.str();
  spec           = oi;


  if(vm){
     numOfFunctions = 1;
     valueMap       = new ValueMapping[1];
     calls          = new unsigned int[1];
     createCostEstimation = new CreateCostEstimation[1];
     costEstimation = new CostEstimation*[1];
     AddValueMapping( 0, vm );
     calls[0] = 0;
     if(createCE){
        createCostEstimation[0] = createCE;
        costEstimation[0] = createCE?createCE():0;
     } else {
         createCostEstimation[0] = 0;
         costEstimation[0] = 0;
     }
  } else {
     valueMap = 0;
     numOfFunctions = 0;
     calls = 0;
     createCostEstimation = 0;
     costEstimation = 0;
  }

  selectFunc     = SimpleSelect;
  typeMap        = tm;
  supportsProgress = oi.supportsProgress ? true : false;
  requestsArgs   = oi.requestsArgs ? true : false;
  usesArgsInTypeMapping = oi.usesArgsInTypeMapping ? true : false;
  usesMemory     = oi.usesMemory;

}

Operator::Operator( const OperatorInfo& oi,
                    ValueMapping vms[],
                    SelectFunction sf,
                    TypeMapping tm,
                    CreateCostEstimation* createCE )
{
  int max = 0;
  while ( vms[max] != 0 ) { max++; }

  // define member attributes
  name           = oi.name;
  specString     = oi.str();
  spec           = oi;
  numOfFunctions = max;
  selectFunc     = sf;
  valueMap       = new ValueMapping[max];
  calls          = new unsigned int[max];
  createCostEstimation = new CreateCostEstimation[max];
  costEstimation = new CostEstimation*[max];
  typeMap        = tm;
  supportsProgress = oi.supportsProgress ? true : false;
  requestsArgs   = oi.requestsArgs ? true : false;
  usesArgsInTypeMapping = oi.usesArgsInTypeMapping ? true : false;
  usesMemory     = oi.usesMemory;

  for ( int i = 0; i < max; i++ ) {
    //cout << "Adding " << i << endl;
    //cout << (void*) vms[i] << endl;
    AddValueMapping( i, vms[i] );
    calls[i] = 0;
    if(createCE){
       createCostEstimation[i] = createCE[i];
       costEstimation[i] = createCE[i]?createCE[i]():0;   
    } else {
       createCostEstimation[i] = 0;
       costEstimation[i]=0;
    }
  }
}


bool
Operator::AddValueMapping( const int index, ValueMapping f )
{
  if ( index < numOfFunctions && index >= 0 )
  {
    valueMap[index] = f;
    return (true);
  }
  else
  {
    return (false);
  }
}


ostream& Operator::Print(ostream& o) const{
   o << "Operator [ "<<  name  << ", "
     <<  specString << ", "
     <<  spec << ", "
     << "numOfFunctions = "   <<  numOfFunctions << ", "
     << "supportsProgress ="  <<  supportsProgress << ", "
     << "requestsArgs ="  <<     requestsArgs << ", "
	 << "usesArgsInTypeMapping ="  <<     usesArgsInTypeMapping
     << "]";
   return o;
}

ostream& operator<<(ostream& o, const Operator& op){
  return op.Print(o);
}




/* Member functions of Class TypeConstructor */
bool
TypeConstructor::DefaultOpen( SmiRecord& valueRecord,
                              size_t& offset,
                              const ListExpr typeInfo,
                              Word& value )
{
  ListExpr valueList = 0;
  string valueString;
  int valueLength;

  ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );
  bool correct;
  valueRecord.Read( &valueLength, sizeof( valueLength ), offset );
  offset += sizeof( valueLength );
  char* buffer = new char[valueLength];
  valueRecord.Read( buffer, valueLength, offset );
  offset += valueLength;
  valueString.assign( buffer, valueLength );
  delete []buffer;
  nl->ReadFromString( valueString, valueList );
  value = RestoreFromList( nl->First(typeInfo),
                           nl->First(valueList),
                           1, errorInfo, correct  );
  if ( errorInfo != 0 )
  {
    nl->Destroy( errorInfo );
  }
  nl->Destroy( valueList );
  return (true);
}

bool
TypeConstructor::DefaultSave( SmiRecord& valueRecord,
                              size_t& offset,
                              const ListExpr typeInfo,
                              Word& value )
{
  ListExpr valueList;
  string valueString;
  int valueLength;

  valueList = SaveToList( nl->First(typeInfo), value );
  valueList = nl->OneElemList( valueList );
  nl->WriteToString( valueString, valueList );
  valueLength = valueString.length();
  valueRecord.Write( &valueLength, sizeof( valueLength ), offset );
  offset += sizeof( valueLength );
  valueRecord.Write( valueString.data(), valueString.length(), offset );
  offset += valueString.length();

  nl->Destroy( valueList );
  return (true);
}

Word
TypeConstructor::DummyCreate( const ListExpr typeInfo )
{
  return (SetWord( Address( 0 ) ));
}

void
TypeConstructor::DummyDelete( const ListExpr typeInfo, Word& w )
{
}

void
TypeConstructor::DummyClose( const ListExpr typeInfo, Word& w )
{
}

Word
TypeConstructor::DummyClone( const ListExpr typeInfo, const Word& w )
{
  return (SetWord( Address( 0 ) ));
}

int
TypeConstructor::DummySizeOf()
{
  return (0);
}

TypeConstructor::TypeConstructor( const string& nm,
                                  TypeProperty prop,
                                  OutObject out,
                                  InObject in,
                                  OutObject saveToList,
                                  InObject restoreFromList,
                                  ObjectCreation create,
                                  ObjectDeletion del,
                                  ObjectOpen open,
                                  ObjectSave save,
                                  ObjectClose close,
                                  ObjectClone clone,
                                  ObjectCast ca,
                                  ObjectSizeof sizeOf,
                                  TypeCheckFunction tcf )
{
  name                 = nm;
  propFunc             = prop;
  outFunc              = out;
  inFunc               = in;
  saveToListFunc       = saveToList;
  restoreFromListFunc  = restoreFromList;
  createFunc           = create;
  deleteFunc           = del;
  openFunc             = open;
  saveFunc             = save;
  closeFunc            = close;
  cloneFunc            = clone;
  castFunc             = ca;
  sizeofFunc           = sizeOf;
  typeCheckFunc        = tcf;
}

TypeConstructor::~TypeConstructor()
{
}

void
TypeConstructor::AssociateKind( const string& kindName )
{
  if ( kindName.length() > 0 )
  {
    kinds.push_back( kindName );
  }
}

ListExpr
TypeConstructor::Property()
{
  if (propFunc)
   return (*propFunc)();
  return Property(conInfo);
}


ListExpr
TypeConstructor::Property(const ConstructorInfo& ci)
{
  return ci.list();
}


ListExpr
TypeConstructor::Out( ListExpr type, Word value )
{
  return ((*outFunc)( type, value ));
}

Word
TypeConstructor::In( const ListExpr typeInfo, const ListExpr value,
                     const int errorPos, ListExpr& errorInfo, bool& correct )
{
  return ((*inFunc)( typeInfo, value, errorPos, errorInfo, correct ));
}

ListExpr
TypeConstructor::SaveToList( ListExpr type, Word value )
{
  if( saveToListFunc != 0 )
    return ((*saveToListFunc)( type, value ));
  else
    return ((*outFunc)( type, value ));
}

Word
TypeConstructor::RestoreFromList( const ListExpr typeInfo,
                                  const ListExpr value,
                                  const int errorPos,
                                  ListExpr& errorInfo, bool& correct )
{
  if( restoreFromListFunc != 0 )
    return ((*restoreFromListFunc)( typeInfo, value,
                                    errorPos, errorInfo, correct ));
  else
    return ((*inFunc)( typeInfo, value, errorPos, errorInfo, correct ));
}

Word
TypeConstructor::Create( const ListExpr typeInfo )
{
  return (*createFunc)( typeInfo );
}

Word
TypeConstructor::Create( const NList& typeInfo )
{
  return (*createFunc)( typeInfo.listExpr() );
}


void
TypeConstructor::Delete( const ListExpr typeInfo, Word& w )
{
  (*deleteFunc)( typeInfo, w );
}

bool
TypeConstructor::Open( SmiRecord& valueRecord,
                       size_t& offset,
                       const ListExpr typeInfo,
                       Word& value )
{
  if ( openFunc != 0 )
  {
    return ((*openFunc)( valueRecord, offset, typeInfo, value ));
  }
  else
  {
    return (DefaultOpen( valueRecord, offset, typeInfo, value ));
  }
}

bool
TypeConstructor::Save( SmiRecord& valueRecord,
                       size_t& offset,
                       const ListExpr typeInfo,
                       Word& value )
{
  if ( saveFunc != 0 )
  {
    return ((*saveFunc)( valueRecord, offset, typeInfo, value ));
  }
  else
  {
    return (DefaultSave( valueRecord, offset, typeInfo, value ));
  }
}

void
TypeConstructor::Close( const ListExpr typeInfo, Word& w )
{
  (*closeFunc)( typeInfo, w );
}

Word
TypeConstructor::Clone( const ListExpr typeInfo, const Word& w )
{
  return (*cloneFunc)( typeInfo, w );
}

int
TypeConstructor::SizeOf()
{
  return (*sizeofFunc)();
}

bool
TypeConstructor::MemberOf(const string& k)
{
  vector<string>::iterator it = find( kinds.begin(), kinds.end(), k );
  if (it != kinds.end()) {
    return true;
  }
  else {
    return false;
  }
}

/*
This function initializes some properties which are (currently) only
of interst for types derived from class attribute. In a future design it
would be nice to have base classes for types and operators as well. But
for the moment only the import subcase of data types used in relations
have it.

As a work around it is convenient to add new feature by defining virtual
functions in the Attribute class with a default implementation. This avoids
to introduce new functions for all types when only needed in some classes.

*/

void
TypeConstructor::initKindDataProperties()
{
   SecondoCatalog& ctlg = *SecondoSystem::GetCatalog();

   if (propFunc) {
     NList p = NList( (*propFunc)() );
     //cerr << p << endl;
     conInfo = ConstructorInfo( p.second() );
     conInfo.name = name;
   }

   serializedFixSize = -1;
   numOfFlobs = -2;
   storageType = Attribute::Unspecified;

   if ( MemberOf(Kind::DATA()) ) {

      //cout << "** TC **  " << Name()
      //     << " <-- " << conInfo.typeExample << endl;
      ListExpr type = nl->Empty();
      nl->ReadFromString(conInfo.typeExample, type);

      // to do: better error handling
      ListExpr numType = ctlg.NumericType(type);
      Word w = Create( numType );

      if (w.addr == 0) {
        cerr << "** TC Error ** Could not create an instance for "
             << Name() << " using type list " << NList(type) << endl;
        numOfFlobs = -2;
      }

      Attribute* attr =  static_cast<Attribute*>(w.addr);

      if (attr != 0) {
        serializedFixSize = attr->SerializedSize();
        numOfFlobs = attr->NumOfFLOBs();
        storageType =  attr->GetStorageType();
        delete attr;
      }
    }
}

string TypeConstructor::Storage2Str()
{
  switch ( storageType ) {
    case Attribute::Default: {
      return "Memoryblock-fix-core";
      break;
    }
    case Attribute::Core: {
      return "Serialize-fix-core";
      break;
    }
    case Attribute::Extension: {
      return "Serialize-variable-extension";
      break;
    }
    default: { return "unspecified"; };
  }
}




/* Member functions of Class Algebra: */
Algebra::Algebra() : tcsNum(0), opsNum(0)
{
}

Algebra::~Algebra()
{
  // delete dynamicly created operators
  for(unsigned int i=0;i<ops.size();i++){
     if(opdel[i] && ops[i]){
         delete ops[i];
         ops[i] = 0;
     }
  }
  // delete type contructors
  for(unsigned int i=0;i<tcs.size();i++){
     if(tcdel[i] && tcs[i]){
         delete tcs[i];
         tcs[i] = 0;
     }
  }

}

void
Algebra::AddTypeConstructor( TypeConstructor* tc,
                             const bool nonstatic /* = false */ )
{
  tcs.push_back( tc );
  tcdel.push_back(nonstatic);
  tcsNum++;

}

Operator*
Algebra::AddOperator( Operator* op, const bool nonstatic /* = false */)
{
  ops.push_back( op );
  opdel.push_back(nonstatic);
  opsNum++;
  return op;
}

Operator*
Algebra::AddOperator( OperatorInfo oi, ValueMapping vm, TypeMapping tm )
{
  Operator* newOp = new Operator(oi, vm, tm);
  AddOperator(newOp, true);
  return newOp;
}

Operator*
Algebra::AddOperator( OperatorInfo oi, ValueMapping vms[],
		      SelectFunction sf, TypeMapping tm   )
{
  Operator* newOp = new Operator(oi, vms, sf, tm);
  AddOperator(newOp, true);
  return newOp;
}

