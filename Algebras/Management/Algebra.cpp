#include "AlgebraManager.h"
#include "Algebra.h"
#include "NestedList.h"

using namespace std;

/* Member functions of class Operator: */

Word
Operator::DummyModel( ArgVector, Supplier )
{
  return (Word( Address( 0 ) ));
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
                    CostMapping cm = Operator::DummyCost )
{
  name           = nm;
  specString     = spec;
  specification  = 0;
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
                    CostMapping cm = Operator::DummyCost )
{
  name           = nm;
  specString     = spec;
  specification  = 0;
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

Word
TypeConstructor::DummyInModel( ListExpr typeExpr, ListExpr list, int objNo )
{
  return (Word( Address( 0 ) ));
}

ListExpr
TypeConstructor::DummyOutModel( ListExpr typeExpr, Word model )
{
  return (0);
}

Word
TypeConstructor::DummyValueToModel( ListExpr typeExpr, Word value )
{
  return (Word( Address( 0 ) ));
}

Word
TypeConstructor::DummyValueListToModel( const ListExpr typeExpr,
                                        const ListExpr valueList,
                                        const int errorPos,
                                        ListExpr& errorInfo,
                                        bool& correct )
{
  return (Word( Address( 0 ) ));
}

TypeConstructor::TypeConstructor( const string& nm,
                                  TypeProperty prop,
                                  OutObject out,
                                  InObject in,
                                  ObjectCreation create,
                                  ObjectDeletion del,
                                  ObjectCast ca,
                                  TypeCheckFunction tcf,
                                  InModelFunction inm,
                                  OutModelFunction outm,
                                  ValueToModelFunction vtm,
                                  ValueListToModelFunction vltm )
{
  name                 = nm;
  property             = 0;
  propFunc             = prop;
  outFunc              = out;
  inFunc               = in;
  createFunc           = create;
  deleteFunc           = del;
  castFunc             = ca;
  typeCheckFunc        = tcf;
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
  if ( property == 0 )
  {
    property = (*propFunc)();
  }
  return (property);
}

ListExpr
TypeConstructor::Out( ListExpr type, Word value )
{
  return ((*outFunc)( type, value ));
}

Word
TypeConstructor::In( const ListExpr type, const ListExpr value,
                     const int errorPos, ListExpr& errorInfo, bool& correct )
{
  return ((*inFunc)( type, value, errorPos, errorInfo, correct ));
}

Word
TypeConstructor::Create( int size )
{
  return ((*createFunc)( size ));
}

void
TypeConstructor::Delete( Word& w )
{
  (*deleteFunc)( w );
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

