/* 

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Integration Hack

[TOC]

1 Introduction

This file is a dirty workaround to make some templates and other items 
available to ~MovingRegionAlgebra~, which are currently not located in an
include file. Modifacations of ~TemporalAlgebra~ and ~SpatialAlgebra~ have
been already requested so that this workaround can be removed in a while.

1 From TemporalAlgebra

*/

template <class Alpha>
int IntimeInst( Word* args, Word& result, int message, Word& local, Supplier s )
{
    if (MRA_DEBUG) cerr << "IntimeInst<Alpha>() called" << endl;

  result = qp->ResultStorage( s );
  Intime<Alpha>* i = (Intime<Alpha>*)args[0].addr;

  if( i->IsDefined() )
    ((Instant*)result.addr)->CopyFrom( &((Intime<Alpha>*)args[0].addr)->instant );
  else
    ((Instant*)result.addr)->SetDefined( false );

  return 0;
}

template <class Alpha>
int IntimeVal( Word* args, Word& result, int message, Word& local, Supplier s )
{
    if (MRA_DEBUG) cerr << "IntimeVal<Alpha>() called" << endl;

  result = qp->ResultStorage( s );
  Intime<Alpha>* i = (Intime<Alpha>*)args[0].addr;

  if( i->IsDefined() )
    ((Alpha*)result.addr)->CopyFrom( &((Intime<Alpha>*)args[0].addr)->value );
  else
    ((Alpha*)result.addr)->SetDefined( false );

  return 0;
}

template <class Mapping, class Alpha>
int MappingAtInstant( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Intime<Alpha>* pResult = (Intime<Alpha>*)result.addr;

  ((Mapping*)args[0].addr)->AtInstant( *((Instant*)args[1].addr), *pResult );

  return 0;
}

template <class Mapping, class Unit, class Alpha>
int MappingInitial( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((Mapping*)args[0].addr)->Initial( *((Intime<Alpha>*)result.addr) );
  return 0;
}

template <class Mapping, class Unit, class Alpha>
int MappingFinal( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((Mapping*)args[0].addr)->Final( *((Intime<Alpha>*)result.addr) );
  return 0;
}

template <class Mapping>
int MappingDefTime( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((Mapping*)args[0].addr)->DefTime( *(Periods*)result.addr );
  return 0;
}

template <class Mapping>
int MappingPresent_i( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Mapping *m = ((Mapping*)args[0].addr);
  Instant* inst = ((Instant*)args[1].addr);

  if( !inst->IsDefined() )
    ((CcBool *)result.addr)->Set( false, false );
  else if( m->Present( *inst ) )
    ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );

  return 0;
}

template <class Mapping>
int MappingPresent_p( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Mapping *m = ((Mapping*)args[0].addr);
  Periods* periods = ((Periods*)args[1].addr);

  if( periods->IsEmpty() )
    ((CcBool *)result.addr)->Set( false, false );
  else if( m->Present( *periods ) )
    ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );

  return 0;
}

/*
1 From TemporalAlgebra

*/

extern ListExpr OutRegion( ListExpr typeInfo, Word value );
