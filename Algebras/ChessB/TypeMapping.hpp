/*
Type map utilities

*/

#ifndef SECONDO_TYPE_MAPPING_HPP
#define SECONDO_TYPE_MAPPING_HPP

#include <boost/type_traits.hpp>
#include <string>
#include "NestedList.h"
#include "StandardTypes.h"
#include "FTextAlgebra.h"
#include "NList.h"
#include "ListStream.hpp"
#include "StreamIterator.hpp"

//-----------------------------------------------------------------------------
// Mapping result type
template< typename T > ListExpr map_result_type( T* )
    { return nl->SymbolAtom( boost::remove_pointer<T>::type::name() ); }
ListExpr map_result_type( bool* ){ return nl->SymbolAtom( "bool" ); }
ListExpr map_result_type( std::string* ){ return nl->SymbolAtom( "string" ); }
ListExpr map_result_type( int* ){ return nl->SymbolAtom( "int" ); }
ListExpr map_result_type( double* ){ return nl->SymbolAtom( "real" ); }
ListExpr map_result_type( CcBool* ){ return nl->SymbolAtom( "bool" ); }
ListExpr map_result_type( CcString* ){ return nl->SymbolAtom( "string" ); }
ListExpr map_result_type( CcInt* ){ return nl->SymbolAtom( "int" ); }
ListExpr map_result_type( CcReal* ){ return nl->SymbolAtom( "real" ); }

//-----------------------------------------------------------------------------
// Mapping result type for streams
template< typename OP >
ListExpr map_stream_result_type( ListExpr args, Tuple**, OP* )
{
    return list_ostream() << ChessBSymbol("tuple") << OP::type( args );
}

template< typename OP, typename T >
ListExpr map_stream_result_type( ListExpr args, T* t, OP* )
{
    return map_result_type( t );
}

//-----------------------------------------------------------------------------
// Checking argument type

// Following functions enable use of intrinsic C++ types in Secondo-operators
void check_argument( ListExpr arg, ListExpr /*root*/, CcInt* )
{
    if ( ! NList( arg ).isSymbol( "int" ) )
        throw std::runtime_error( "Expecting symbol \"int\"." );
}
void check_argument( ListExpr arg, ListExpr /*root*/, int* )
{
    if ( ! NList( arg ).isSymbol( "int" ) )
        throw std::runtime_error( "Expecting symbol \"int\"." );
}
void check_argument( ListExpr arg, ListExpr /*root*/, CcString* )
{
    if ( ! NList( arg ).isSymbol( "string" ) )
        throw std::runtime_error( "Expecting symbol \"string\"." );
}
void check_argument( ListExpr arg, ListExpr /*root*/, std::string* )
{
    if ( ! NList( arg ).isSymbol( "string" ) )
        throw std::runtime_error( "Expecting symbol \"string\"." );
}
void check_argument( ListExpr arg, ListExpr /*root*/, CcReal* )
{
    if ( ! NList( arg ).isSymbol( "real" ) )
        throw std::runtime_error( "Expecting symbol \"real\"." );
}
void check_argument( ListExpr arg, ListExpr /*root*/, double* )
{
    if ( ! NList( arg ).isSymbol( "real" ) )
        throw std::runtime_error( "Expecting symbol \"real\"." );
}
void check_argument( ListExpr arg, ListExpr /*root*/, CcBool* )
{
    if ( ! NList( arg ).isSymbol( "bool" ) )
        throw std::runtime_error( "Expecting symbol \"bool\"." );
}
void check_argument( ListExpr arg, ListExpr /*root*/, FText* )
{
    if ( ! NList( arg ).isSymbol( "text" ) )
        throw std::runtime_error( "Expecting symbol \"text\"." );
}
void check_argument( ListExpr arg, ListExpr /*root*/, bool* )
{
    if ( ! NList( arg ).isSymbol( "bool" ) )
        throw std::runtime_error( "Expecting symbol \"bool\"." );
}

// Default argument check funktion. Works with all Attribute-derived types.
template< typename ARG > void check_argument( ListExpr arg, ListExpr, ARG* )
{
    ListExpr error_info;
    if ( ! ARG::KindCheck( arg, error_info ) )
        throw std::runtime_error( "Kind check error" );
}

// For an unknown type there is obviously nothing to check
void check_argument( ListExpr, ListExpr, void* ) {}
void check_argument( ListExpr, ListExpr, void** ) {}

// type check for tuples.
void check_argument( ListExpr arg, ListExpr /*root*/, Tuple* )
{
    if ( nl->IsAtom( arg ) || 2 != nl->ListLength( arg ) )
        throw std::runtime_error( "Expect tuple definition");
    if ( ! NList( nl->First( arg ) ).isSymbol( "tuple" ) )
        throw std::runtime_error( "Expect tuple symbol" );

    ListExpr tuple_def_le = nl->Second( arg );
    if ( nl->IsAtom( tuple_def_le ) || 0 >= nl->ListLength( tuple_def_le ) )
        throw std::runtime_error( "Expect attribute list");
}

// type check for streams.
template< typename T >
void check_argument( ListExpr stream_le, ListExpr root, StreamIterator<T>* )
{
    if ( nl->IsAtom( stream_le ) || 2 != nl->ListLength( stream_le ) )
        throw std::runtime_error( "Expect list of length 2 as 1. argument");
    if ( ! NList( nl->First( stream_le ) ).isSymbol( "stream" ) )
        throw std::runtime_error( "Expect symbol stream as first argument" );

    check_argument( nl->Second( stream_le ), root, (T*) 0 );
}

/*
//type check for functions
template< typename A >
void check_argument( ListExpr stream_le, ListExpr root, Function<A, R>* )
{
    // function argument
    ListExpr funarg_le = nl->Second( nl->Second( args ) );
    check_argument( funarg_le, args, (A*)0 );

    // they must be equal
    if ( ! nl->Equal( nl->Second( nl->First(args) ), funarg_le ) )
      throw std::runtime_error("Function und operator arguments must be equal");

    // and function argument must be preceded by 'map'
    if ( ! NList(nl->First( nl->Second(args) )).isSymbol( "map" ) )
        throw std::runtime_error("Expect symbol map in function definition" );
}
*/

//-----------------------------------------------------------------------------
// Type mapping functions
template< typename O >
ListExpr unary_type_map( ListExpr args )
{
    if ( nl->IsAtom( args ) || 1 != nl->ListLength( args ) )
        return NList::typeError( "Expect list of length 1" );

    try
    {
        typedef typename O::argument_type A;
        typedef typename O::result_type R;

        check_argument( nl->First( args ), args, (A*)0 );
        return map_result_type( (R*)0 );
    }
    catch( const std::exception& e ) {
        return NList::typeError( e.what() );
    }
}

template< typename O >
ListExpr binary_type_map( ListExpr args )
{
    if ( nl->IsAtom( args ) || 2 != nl->ListLength( args ) )
        return NList::typeError( "Expect list of length 2" );

    try
    {
        typedef typename O::first_argument_type A1;
        typedef typename O::second_argument_type A2;
        typedef typename O::result_type R;

        check_argument( nl->First( args ), args, (A1*)0 );
        check_argument( nl->Second( args ), args, (A2*)0 );
        return map_result_type( (R*)0 );
    }
    catch( const std::exception& e ) {
        return NList::typeError( e.what() );
    }
}

template< typename O >
ListExpr ternary_type_map( ListExpr args )
{
    if ( nl->IsAtom( args ) || 3 != nl->ListLength( args ) )
        return NList::typeError( "Expect list of length 3" );

    try
    {
        typedef typename O::first_argument_type A1;
        typedef typename O::second_argument_type A2;
        typedef typename O::third_argument_type A3;
        typedef typename O::result_type R;

        check_argument( nl->First( args ), args, (A1*)0 );
        check_argument( nl->Second( args ), args, (A2*)0 );
        check_argument( nl->Third( args ), args, (A3*)0 );
        return map_result_type( (R*)0 );
    }
    catch( const std::exception& e ) {
        return NList::typeError( e.what() );
    }
}

//-----------------------------------------------------------------------------
// Type mapping functions returning stream
template< typename O >
ListExpr unary_stream_type_map( ListExpr args )
{
    if ( nl->IsAtom( args ) || 1 != nl->ListLength( args ) )
        return NList::typeError( "Expect list of length 1" );

    try
    {
        typedef typename O::argument_type A;
        typedef typename O::result_type R;
        typedef typename R::second_type T;

        check_argument( nl->First( args ), args, (A*)0 );
        return nl->TwoElemList( nl->SymbolAtom("stream"),
            map_stream_result_type( args, (T*)0, (O*)0 ) );
    }
    catch( const std::exception& e ) {
        return NList::typeError( e.what() );
    }
}

template< typename O >
ListExpr binary_stream_type_map( ListExpr args )
{
    if ( nl->IsAtom( args ) || 2 != nl->ListLength( args ) )
        return NList::typeError( "Expect list of length 2" );

    try
    {
        typedef typename O::first_argument_type A1;
        typedef typename O::second_argument_type A2;
        typedef typename O::result_type R;
        typedef typename R::second_type T;

        check_argument( nl->First( args ), args, (A1*)0 );
        check_argument( nl->Second( args ), args, (A2*)0 );

        return nl->TwoElemList( nl->SymbolAtom("stream"),
            map_stream_result_type( args, (T*)0, (O*)0 ) );
    }
    catch( const std::exception& e ) {
        return NList::typeError( e.what() );
    }
}

//-----------------------------------------------------------------------------
template< typename A, typename R >
ListExpr binary_aggregate_type_map( ListExpr args )
{
    if ( nl->IsAtom( args ) || 2 != nl->ListLength( args ) )
        return NList::typeError( "Expect list of length 2" );

    try
    {
        // stream value type
        check_argument( nl->First( args ), args, (StreamIterator<A>*)0 );

        // function argument
        ListExpr funarg_le = nl->Second( nl->Second( args ) );
        check_argument( funarg_le, args, (A*)0 );

        // they must be equal
        if ( ! nl->Equal( nl->Second( nl->First(args) ), funarg_le ) )
          throw std::runtime_error("Function und operator arguments must be equal");

        // and function argument must be preceded by 'map'
        if ( ! NList(nl->First( nl->Second(args) )).isSymbol( "map" ) )
            throw std::runtime_error("Expect symbol map in function definition" );

        return map_result_type( (R*)0 );
    }
    catch( const std::exception& e ) {
        return NList::typeError( e.what() );
    }
}

#endif // SECONDO_TYPE_MAPPING_HPP
