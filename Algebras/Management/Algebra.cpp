using namespace std;

#include "AlgebraManager.h"
#include "Algebra.h"
#include "NestedList.h"
#include "SecondoSystem.h"


/* Member functions of class Operator: */

Word
Operator::DummyModel( ArgVector, Supplier )
{
  return (SetWord( Address( 0 ) ));
}

ListExpr
Operator::DummyCost( ListExpr )
{
  return (0);
}

Operator::Operator( const string& nm,
                    const string& spec,
                    const int noF,
                    ValueMapping vms[],
                    ModelMapping mms[],
                    SelectFunction sf,
                    TypeMapping tm,
                    CostMapping cm /* = Operator::DummyCost */ )
{
  name           = nm;
  specString     = spec;
  numOfFunctions = noF;
  selectFunc     = sf;
  valueMap       = new ValueMapping[numOfFunctions];
  modelMap       = new ModelMapping[numOfFunctions];
  typeMap        = tm;
  costMap        = cm;
  
  for ( int i = 0; i < numOfFunctions; i++ )
  {
    AddValueMapping( i, vms[i] );
    AddModelMapping( i, mms[i] );
  }
}

Operator::Operator( const string& nm,
                    const string& spec,
                    ValueMapping vm,
                    ModelMapping mm,
                    SelectFunction sf,
                    TypeMapping tm,
                    CostMapping cm /* = Operator::DummyCost */ )
{
  name           = nm;
  specString     = spec;
  numOfFunctions = 1;
  selectFunc     = sf;
  valueMap       = new ValueMapping[1];
  modelMap       = new ModelMapping[1];
  typeMap        = tm;
  costMap        = cm;

  AddValueMapping( 0, vm );
  AddModelMapping( 0, mm );
}

Operator::~Operator()
{
  delete[] valueMap;
  delete[] modelMap;
}

int
Operator::Select( ListExpr le )
{
  return ((*selectFunc)( le ));
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

bool
Operator::AddModelMapping( const int index, ModelMapping f )
{
  if ( index < numOfFunctions && index >= 0 )
  {
    modelMap[index] = f;
    return (true);
  }
  else
  {
    return (false);
  }
}

string
Operator::Specification()
{
  return (specString);
}

int
Operator::CallValueMapping( const int index, ArgVector args, Word& result, 
                            int message, Word& local, Supplier sup )
{
  return (*valueMap[index])( args, result, message, local, sup );
}

ListExpr
Operator::CallTypeMapping( ListExpr argList )
{
  return ((*typeMap)( argList ));
}

Word
Operator::CallModelMapping( int index, ArgVector argv, Supplier sup )
{
  return ((*modelMap[index])( argv, sup ));
}

ListExpr
Operator::CallCostMapping( ListExpr argList )
{
  return ((*costMap)( argList ));
}

/* Member functions of Class TypeConstructor */

bool
TypeConstructor::DefaultOpen( SmiRecord& valueRecord,
                              const ListExpr typeInfo,
                              Word& value )
{
  NestedList* nl = SecondoSystem::GetNestedList();
  ListExpr valueList = 0;
  string valueString;
  int valueLength;
  
  ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );
  bool correct;
  valueRecord.Read( &valueLength, sizeof( valueLength ), 0 );
  char* buffer = new char[valueLength];
  valueRecord.Read( buffer, valueLength, sizeof( valueLength ) );
  valueString.assign( buffer, valueLength );
  delete []buffer;
  nl->ReadFromString( valueString, valueList );
  value = In( nl->First(typeInfo), nl->First( valueList ), 1, errorInfo, correct );
  if ( errorInfo != 0 )
  {
    nl->Destroy( errorInfo );
  }
  nl->Destroy( valueList );
  return (true);
}

bool
TypeConstructor::DefaultSave( SmiRecord& valueRecord,
                              const ListExpr typeInfo,
                              Word& value )
{
  NestedList* nl = SecondoSystem::GetNestedList();
  ListExpr valueList;
  string valueString;
  int valueLength;

  valueList = Out( nl->First(typeInfo), value );
  valueList = nl->OneElemList( valueList );
  nl->WriteToString( valueString, valueList );
  valueLength = valueString.length();
  valueRecord.Write( &valueLength, sizeof( valueLength ), 0 );
  valueRecord.Write( valueString.data(), valueString.length(), sizeof( valueLength ) );

  nl->Destroy( valueList );
  return (true);
}

bool
TypeConstructor::DefaultPersistModel( const PersistDirection dir,
                                      SmiRecord& modelRecord,
                                      const ListExpr typeExpr,
                                      Word& model )
{
  NestedList* nl = SecondoSystem::GetNestedList();
  ListExpr modelList = 0;
  string modelString;
  int modelLength;
  switch ( dir )
  {
    case ReadFrom:
    {
      modelRecord.Read( &modelLength, sizeof( modelLength ), 0 );
      char* buffer = new char[modelLength];
      modelRecord.Read( buffer, modelLength, sizeof( modelLength ) );
      modelString.assign( buffer, modelLength );
      delete []buffer;
      nl->ReadFromString( modelString, modelList );
      model = InModel( typeExpr, modelList, 1 );
    }
    break;
    case WriteTo:
    {
      modelList = OutModel( typeExpr, model );
      nl->WriteToString( modelString, modelList );
      modelLength = modelString.length();
      modelRecord.Write( &modelLength, sizeof( modelLength ), 0 );
      modelRecord.Write( modelString.data(), modelString.length(), sizeof( modelLength ) );
    }
    break;
    case DeleteFrom:
    {
      // Nothing needs to be done in this case
    }
    break;
  }
  nl->Destroy( modelList );
  return (true);
}

bool
TypeConstructor::DummyPersistModel( const PersistDirection dir,
                                    SmiRecord& modelRecord,
                                    const ListExpr typeExpr,
                                    Word& model )
{
  return (true);
}

Word
TypeConstructor::DummyInModel( ListExpr typeExpr, ListExpr list, int objNo )
{
  return (SetWord( Address( 0 ) ));
}

ListExpr
TypeConstructor::DummyOutModel( ListExpr typeExpr, Word model )
{
  return (0);
}

Word
TypeConstructor::DummyValueToModel( ListExpr typeExpr, Word value )
{
  return (SetWord( Address( 0 ) ));
}

Word
TypeConstructor::DummyValueListToModel( const ListExpr typeExpr,
                                        const ListExpr valueList,
                                        const int errorPos,
                                        ListExpr& errorInfo,
                                        bool& correct )
{
  return (SetWord( Address( 0 ) ));
}

TypeConstructor::TypeConstructor( const string& nm,
                                  TypeProperty prop,
                                  OutObject out,
                                  InObject in,
                                  ObjectCreation create,
                                  ObjectDeletion del,
                                  ObjectOpen open,
                                  ObjectSave save,
                                  ObjectClose close,
                                  ObjectClone clone,
                                  ObjectCast ca,
                                  TypeCheckFunction tcf,
                                  PersistFunction pmf,
                                  InModelFunction inm,
                                  OutModelFunction outm,
                                  ValueToModelFunction vtm,
                                  ValueListToModelFunction vltm )
{
  name                 = nm;
  propFunc             = prop;
  outFunc              = out;
  inFunc               = in;
  createFunc           = create;
  deleteFunc           = del;
  openFunc             = open;
  saveFunc             = save;
  closeFunc            = close;
  cloneFunc            = clone;
  castFunc             = ca;
  typeCheckFunc        = tcf;
  persistModelFunc     = pmf;
  inModelFunc          = inm;
  outModelFunc         = outm;
  valueToModelFunc     = vtm;
  valueListToModelFunc = vltm;
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
  ListExpr property = (*propFunc)();
  return (property);
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

Word
TypeConstructor::Create( const ListExpr typeInfo )
{
  return ((*createFunc)( typeInfo ));
}

void
TypeConstructor::Delete( Word& w )
{
  (*deleteFunc)( w );
}

bool
TypeConstructor::Open( SmiRecord& valueRecord,
                       const ListExpr typeInfo,
                       Word& value )
{
  if ( openFunc != 0 )
  {
    return ((*openFunc)( valueRecord, typeInfo, value ));
  }
  else
  {
    return (DefaultOpen( valueRecord, typeInfo, value ));
  }
}

bool
TypeConstructor::Save( SmiRecord& valueRecord,
                       const ListExpr typeInfo,
                       Word& value )
{
  if ( saveFunc != 0 )
  {
    return ((*saveFunc)( valueRecord, typeInfo, value ));
  }
  else
  {
    return (DefaultSave( valueRecord, typeInfo, value ));
  }
}

void
TypeConstructor::Close( Word& w )
{
  (*closeFunc)( w );
}

Word
TypeConstructor::Clone( const Word& w )
{
  return (*cloneFunc)( w );
}

bool
TypeConstructor::PersistModel( PersistDirection dir,
                               SmiRecord& modelRecord,
                               const ListExpr typeExpr,
                               Word& model )
{
  if ( persistModelFunc != 0 )
  {
    return ((*persistModelFunc)( dir, modelRecord, typeExpr, model ));
  }
  else
  {
    return (DefaultPersistModel( dir, modelRecord, typeExpr, model ));
  }
}

Word
TypeConstructor::InModel( ListExpr type, ListExpr list, int objNo )
{
  return ((*inModelFunc)( type, list, objNo ));
}

ListExpr
TypeConstructor::OutModel( ListExpr type, Word model )
{
  return ((*outModelFunc)( type, model ));
}

Word
TypeConstructor::ValueToModel( ListExpr type, Word value )
{
  return ((*valueToModelFunc)( type, value ));
}


/* Member functions of Class Algebra: */

Algebra::Algebra()
{
}

Algebra::~Algebra()
{
}

void
Algebra::AddTypeConstructor( TypeConstructor* tc )
{
  tcs.push_back( tc );
}

void
Algebra::AddOperator( Operator* op )
{
  ops.push_back( op );
}

