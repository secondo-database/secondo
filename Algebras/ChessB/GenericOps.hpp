#ifndef SECONDO_ALGEBRAS_CHESS_GENERIC_OPS_HPP
#define SECONDO_ALGEBRAS_CHESS_GENERIC_OPS_HPP

#include <functional>
#include <boost/lexical_cast.hpp>
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "ListStream.hpp"

struct even_op : std::unary_function< CcInt, bool > {
    bool operator () ( const CcInt& n ) { return n.GetValue() % 2 == 0; }
};

struct odd_op : std::unary_function< CcInt, bool > {
    bool operator () ( const CcInt& n ) { return n.GetValue() % 2 != 0; }
};

#include "NList.h"
#include "StreamIterator.hpp"

template< int N >
struct Ntuples_op : std::unary_function< StreamIterator<Tuple>, std::pair<bool, Tuple*> >
{
    Ntuples_op( const StreamIterator<Tuple>&, ListExpr type ) : type_(type){}

    std::pair<bool, Tuple*> operator()( const StreamIterator<Tuple>& stream )
    {
        StreamIterator<Tuple> s( stream );
        TupleType type( type_ );
        TupleType* tt = new TupleType(type_);
        Tuple* result = new Tuple( tt );
        tt->DeleteIfAllowed();
        bool result_valid = false;

        if ( s.valid() )
        {
            Tuple* first = *s;
            int dest = 0;
            for( int i = 0; i < first->GetNoAttributes(); ++i, ++dest )
                result->CopyAttribute( i, first, dest );

            int tuple_index = 1;
            for( ; tuple_index < N; ++tuple_index )
            {
                if ( (++s).valid() )
                {
                    Tuple* tuple = *s;
                    for( int i = 0; i < tuple->GetNoAttributes(); ++i, ++dest )
                        result->CopyAttribute( i, tuple, dest );
                    tuple->DeleteIfAllowed();
                }
                else
                    break;
            }
            if ( tuple_index == N )
                result_valid = true;
            else
            {
                //
                for( ; tuple_index < N; ++tuple_index )
                    for( int i = 0; i < first->GetNoAttributes(); ++i, ++dest )
                        result->CopyAttribute( i, first, dest );
            }
            first->DeleteIfAllowed();
        }
        return std::make_pair( result_valid, result );
    }

    static list_ostream type( ListExpr stream_le )
    {
        ListExpr e = nl->Second( nl->Second( nl->First( stream_le ) ) );

        std::vector< std::pair< std::string, std::string > > attrs;
        for(size_t i = 0, i_end = nl->ListLength(e); i < i_end; ++i)
        {
            ListExpr attr_def = nl->Nth( i + 1, e );
            std::string name = from_atom<std::string>( nl->First(attr_def) );
            std::string type = from_atom<std::string>( nl->Second(attr_def) );
            attrs.push_back( std::make_pair( name, type ) );
        }

        list_ostream new_def;
        for( int j = 0; j < N; ++j )
            for( size_t k = 0; k < attrs.size(); ++k )
                new_def << ( list_ostream()
                    << ChessBSymbol( attrs[k].first +
                        boost::lexical_cast<std::string>( j + 1 ) )
                    << ChessBSymbol( attrs[k].second ) );

        return new_def;
    }
private:
    ListExpr type_;
};

struct exists_op : std::unary_function< CcBool, bool >
{
    exists_op() : result_(false){}
    void operator()( const CcBool& arg ){ result_ |= arg.GetValue(); }
    bool result() const { return result_; }

private:
    bool result_;
};

struct forall_op : std::unary_function< CcBool, bool >
{
    forall_op() : result_(true){}
    void operator()( const CcBool& arg ){ result_ &= arg.GetValue(); }
    bool result() const { return result_; }

private:
    bool result_;
};

struct stddev_op : std::unary_function< CcReal, double >
{
    stddev_op() : sum_(0.0), square_(0.0), N_(0){}

    void operator()( const CcReal& a )
    {
        double arg = a.GetValue();
        sum_ += arg;
        square_ += arg * arg;
        ++N_;
    }

    bool result() const
    {
        return sqrt( (N_*square_ - sum_*sum_)/(N_*(N_- 1)) );
    }

private:
    double sum_;
    double square_;
    int N_;
};

#endif // SECONDO_ALGEBRAS_CHESS_GENERIC_OPS_HPP
