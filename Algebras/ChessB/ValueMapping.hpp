/*
Template Functions hiding the Secondo Interface for Value Mappings

*/

#ifndef SECONDO_VALUE_MAPPING_HPP
#define SECONDO_VALUE_MAPPING_HPP

#include <boost/assert.hpp>
#include "AlgebraTypes.h"
#include "StandardTypes.h"
#include "QueryProcessor.h"
#include "RelationAlgebra.h"
#include "Attribute.h"
#include "StreamIterator.hpp"

//-----------------------------------------------------------------------------
// Result value conversion to secondo type
Attribute* result_value( bool b ){ return new CcBool( true, b ); }
Attribute* result_value( std::string s ){ return new CcString( true, s ); }
Attribute* result_value( int i ){ return new CcInt( true, i ); }
Attribute* result_value( double d ){ return new CcReal( true, d ); }
Tuple* result_value( Tuple* t ) { return t; }
template< typename T > Attribute* result_value( T attr )
    { return static_cast<Attribute*>( attr ); }



void setResultValue(void* resAddr, const bool b){
  (static_cast<CcBool*>(resAddr))->Set(true, b); 
} 
void setResultValue(void* resAddr, const std::string& s){
  (static_cast<CcString*>(resAddr))->Set(true,s);
}
void setResultValue(void* resAddr, const int i){
  (static_cast<CcInt*>(resAddr))->Set(true, i); 
} 
void setResultValue(void* resAddr, const double d){
  (static_cast<CcReal*>(resAddr))->Set(true, d); 
}

void setResultValue(void* resAddr, const Attribute* a){
   (static_cast<Attribute*>(resAddr))->CopyFrom(a);
}

void setResultValue(void*& resAddr, Tuple* t){
  resAddr = t;
} 

template<typename T> void setResultValue(void* resAddr, T attr){
  (static_cast<T>(resAddr))->CopyFrom(attr);
}


void delPointer(int i) { }
void delPointer(std::string s){}
void delPointer(double d){ }
void delPointer(bool b){ }

void delPointer(Attribute*& a){
   a->DeleteIfAllowed(); 
   a=0;
}

void delPointer(Tuple*& t){
   t->DeleteIfAllowed();
   t=0;
}

template<typename T>
void delPointer(T*& p){ 
   p->DeleteIfAllowed(); 
   p = 0;
}





void* on_open_arg( void* arg, void* ){ return arg; }
void* on_request_arg( void* arg, void* ){ return arg; }

// the next argument conversion is needed for forall and exists
void* on_open_arg( void* arg, void** ){ return arg; }
void* on_request_arg( void* arg, void** ){ return arg; }

// default behavior, cast from Attibute to an Attibute-derived type
template< typename T > T& on_open_arg( void* arg, T* ){
    return *static_cast<T*>( arg );
}
template< typename T > T& on_request_arg( void* arg, T* ){
    return *static_cast<T*>( arg );
}

template< typename T > void on_close_arg( void* arg, T* ){}


// initialize StreamIterator
template< typename T >
StreamIterator<T> on_open_arg( void* arg, StreamIterator<T>* )
{
    qp->Open( arg );
    return StreamIterator<T>(0);
}
template< typename T >
StreamIterator<T> on_request_arg( void* arg, StreamIterator<T>* )
{
    return StreamIterator<T>( arg );
}
template< typename T > void on_close_arg( void* arg, StreamIterator<T>* )
{
    qp->Close( arg );
}

//-----------------------------------------------------------------------------
#include "QueryProcessor.h"

template< typename ARG, typename R >
struct Function
{
    Function( void* arg ) : handle_(arg){}

    R operator()( const ARG& arg ) const
    {
        BOOST_ASSERT( handle_ != 0 && "Function handle is null!" );

        Word value;
        ( *qp->Argument(handle_) )[0].addr = arg;
        qp->Request( handle_, value );
        return static_cast<R*>( value.addr );
    }

private:
    void* handle_;
};

//-----------------------------------------------------------------------------
// initialize Function
template< typename A, typename R >
Function< A, R > on_open_arg( void* arg, Function< A, R >* ) {
    return Function< A, R >( arg );
}
template< typename A, typename R >
Function< A, R > on_request_arg( void* arg, Function< A, R >* ) {
    return Function< A, R >( arg );
}
template< typename A, typename R >
void on_close_arg( void* arg, Function< A, R >* ){}

//-----------------------------------------------------------------------------
// Value mapping functions
template< typename O >
int unary_value_map( Word* args, Word& result, int msg, Word&, Supplier s )
{
    typedef typename O::argument_type A;
    typedef typename O::result_type R;

    result = qp->ResultStorage( s );
    on_open_arg( args[0].addr, (A*)0 );
//    if ( static_cast<Attribute*>( args[0].addr )->IsDefined() )
//    {
        try
        {
            // result.addr = result_value( O()
            //    ( on_request_arg( args[0].addr, (A*)0 ) ) );
            R res = O()(on_request_arg(args[0].addr, (A*)0));
            setResultValue(result.addr, res);
            delPointer(res);
            return 0;
        }
        catch( const std::exception& e ) {
            std::cerr << e.what() << endl;
        }
//    }
    on_close_arg( args[0].addr, (A*)0 );
//    static_cast<Attribute*>(result.addr)->SetDefined( false );
    return 0;
}

template< typename O >
int binary_value_map( Word* args, Word& result, int, Word&, Supplier s )
{
    typedef typename O::first_argument_type A1;
    typedef typename O::second_argument_type A2;
    typedef typename O::result_type R;

    result = qp->ResultStorage( s );
    on_open_arg( args[0].addr, (A1*)0 );
    on_open_arg( args[1].addr, (A2*)0 );
//    if ( static_cast<Attribute*>( args[0].addr )->IsDefined() &&
//         static_cast<Attribute*>( args[1].addr )->IsDefined() )
//    {
        try
        {
           R res = O()(on_request_arg(args[0].addr, (A1*)0) ,
                       on_request_arg(args[1].addr, (A2*)0));
           setResultValue(result.addr, res);
           delPointer(res);
  
//            result.addr = result_value( O()
//                ( on_request_arg( args[0].addr, (A1*)0 ),
//                  on_request_arg( args[1].addr, (A2*)0 ) ) );
            return 0;
        }
        catch( const std::exception& e ) {
            std::cerr << e.what() << endl;
        }
//    }
    on_close_arg( args[0].addr, (A1*)0 );
    on_close_arg( args[1].addr, (A2*)0 );
//    static_cast<Attribute*>(result.addr)->SetDefined( false );
    return 0;
}

template< typename O >
int ternary_value_map( Word* args, Word& result, int, Word&, Supplier s )
{
    typedef typename O::first_argument_type A1;
    typedef typename O::second_argument_type A2;
    typedef typename O::third_argument_type A3;
    typedef typename O::result_type R;

    result = qp->ResultStorage( s );
    on_open_arg( args[0].addr, (A1*)0 );
    on_open_arg( args[1].addr, (A2*)0 );
    on_open_arg( args[2].addr, (A3*)0 );
//    if ( static_cast<Attribute*>( args[0].addr )->IsDefined() &&
//         static_cast<Attribute*>( args[1].addr )->IsDefined() &&
//         static_cast<Attribute*>( args[2].addr )->IsDefined() )
//    {
        try
        {
            R res  =  O()
                ( on_request_arg( args[0].addr, (A1*)0 ),
                  on_request_arg( args[1].addr, (A2*)0 ),
                  on_request_arg( args[2].addr, (A3*)0 ) );
            setResultValue(result.addr, res);
            delPointer(res);
            return 0;
        }
        catch( const std::exception& e ) {
            std::cerr << e.what() << endl;
        }
//    }
    on_close_arg( args[0].addr, (A1*)0 );
    on_close_arg( args[1].addr, (A2*)0 );
    on_close_arg( args[2].addr, (A3*)0 );
//    static_cast<Attribute*>(result.addr)->SetDefined( false );
    return 0;
}

//-----------------------------------------------------------------------------
// Stream producing value mapping functions
template< typename O >
int unary_stream_value_map( Word* args, Word& result,
                            int message, Word& local, Supplier s )
{
    typedef typename O::argument_type A;
    typedef typename O::result_type R;

    if ( OPEN == message )
    {
        try {
            local.addr = new O( on_open_arg( args[0].addr, (A*)0 ),
                                nl->Second( GetTupleResultType(s) ) );
        }
        catch( std::exception const& e ) {
            std::cerr << e.what() << endl;
            local.addr = 0;
        }
    }
    else if ( CLOSE == message )
    {
        on_close_arg( args[0].addr, (A*)0 );
        if ( local.addr )
        {
            delete static_cast< O* >( local.addr );
            local.addr = 0;
        }
    }
    else if ( REQUEST == message )
    {
        if ( local.addr )
        {
            try
            {
                O* op = static_cast< O* >( local.addr );
                R r = (*op)( on_request_arg( args[0].addr, (A*)0 ) );
                if ( r.first )
                {
                    result = qp->ResultStorage( s );
                    result.addr = result_value( r.second );
                    return YIELD;
                } else {
                  delPointer(r.second);
                }
            }
            catch( const std::exception& e ){
                std::cerr << e.what() << endl;
            }
        }
        return CANCEL;
    }
    return 0;
}

template< typename O >
int binary_stream_value_map( Word* args, Word& result,
                             int message, Word& local, Supplier s )
{
    typedef typename O::first_argument_type A1;
    typedef typename O::second_argument_type A2;
    typedef typename O::result_type R;

    if ( OPEN == message )
    {
        try
        {
            local.addr = new O( on_open_arg( args[0].addr, (A1*)0 ),
                                on_open_arg( args[1].addr, (A2*)0 ),
                                nl->Second( GetTupleResultType(s) ) );
        }
        catch( std::exception const& e ) {
            std::cerr << e.what() << endl;
            local.addr = 0;
        }
    }
    else if ( CLOSE == message )
    {
        on_close_arg( args[0].addr, (A1*)0 );
        on_close_arg( args[1].addr, (A2*)0 );
        if ( local.addr )
        {
            delete static_cast< O* >( local.addr );
            local.addr = 0;
        }

    }
    else if ( REQUEST == message )
    {
        if ( local.addr )
        {
            try
            {
                O* op = static_cast< O* >( local.addr );
                R r = (*op)( on_request_arg( args[0].addr, (A1*)0 ),
                             on_request_arg( args[1].addr, (A2*)0 ) );
                if ( r.first )
                {
                    result = qp->ResultStorage( s );
                    result.addr = result_value( r.second );
                    return YIELD;
                }
            }
            catch( const std::exception& e ){
                std::cerr << e.what() << endl;
            }
        }
        return CANCEL;
    }
    return 0;
}

//-----------------------------------------------------------------------------
// Stream consuming aggregate value mapping functions
template< typename A, typename OP >
struct unary_aggregate
    : std::unary_function< StreamIterator< A >, typename OP::result_type >
{
    typedef typename OP::result_type R;

    R operator()( const StreamIterator< A >& stream )
    {
        OP op;
        for( StreamIterator< A > s(stream); s.valid(); ++s )
            op( on_request_arg( *s, (typename OP::argument_type*)0 ) );
        return op.result();
    }
};

template< typename A, typename OP >
struct binary_aggregate : std::binary_function< StreamIterator< A >,
                                 Function< A*, typename OP::result_type >,
                                 typename OP::result_type >
{
    typedef typename OP::result_type R;

    R operator()( const StreamIterator< A >& stream,
                  const Function< A*, R >& fun )
    {
        OP op;
        for( StreamIterator<A> s(stream); s.valid(); ++s )
            op( fun(*s) );
        return op.result();
    }
};

#endif // SECONDO_VALUE_MAPPING_HPP
