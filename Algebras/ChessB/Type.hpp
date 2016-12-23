#ifndef SECONDO_TYPE_HPP
#define SECONDO_TYPE_HPP

#include "TypeConstructor.h"
#include "AlgebraTypes.h"

//-----------------------------------------------------------------------------
// Type constructor builder
enum undef_t { UNDEF, DEFINED };


template < typename T >
struct MyFunctions : ConstructorFunctions< T >
{
  typedef ConstructorFunctions< T > F; 	
  MyFunctions() : F() 
  {	
     F::out = Out;
     F::in = In;
     F::cast = Cast;
     F::kindCheck = T::KindCheck;
  }

  static void* Cast( void* addr )
  {
    return new (addr) T;
  }

  static Word In( const ListExpr, const ListExpr instance,
                    const int, ListExpr&, bool& correct )
  {
        if ( nl->IsAtom( instance ) &&
             nl->AtomType( instance ) == SymbolType &&
             nl->SymbolValue( instance ) == "undef" )
        {
            correct = true;
            return SetWord( new T( UNDEF ) );
        }

        try
        {
            T t = T::In( instance );
            correct = true;
            return SetWord( new T( t ) );
        }
        catch( const std::exception& e )
        {
            cmsg.inFunError( e.what() );
            correct = false;
        }
        return SetWord( new T( UNDEF ) );
  }

  static ListExpr Out( ListExpr d, Word value )
  {
        const T& attr = *static_cast< const T* >( value.addr );
        if ( ! attr.IsDefined() )
            return nl->SymbolAtom( "undef" );

        return T::Out( attr );
  }

};

template< typename T >
struct Type : TypeConstructor
{
    Type( const ConstructorInfo& ci )
        : TypeConstructor( ci, MyFunctions< T >() )
    {
        AssociateKind( "DATA" );
    }
 };

#endif // SECONDO_TYPE_HPP
