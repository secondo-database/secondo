/*

STPatternAlgebra.h

Created on: Jan 6, 2009
Author: m.attia

*/

#ifndef STPATTERNALGEBRA_H_
#define STPATTERNALGEBRA_H_
#include "Algebra.h"

#include "NestedList.h"

#include "QueryProcessor.h"

#include "StandardTypes.h"
#include "LogMsg.h"
#include "NList.h"
#include "RelationAlgebra.h"
#include "TemporalAlgebra.h"
#include <map>
using namespace datetime;
typedef DateTime Instant;
extern NestedList *nl;
extern QueryProcessor* qp;


namespace STP {
enum SimpleConnectors {
  aabb=1,
  bbaa=2,
  aa_bb=4,
  bb_aa=8,
  abab=16,
  baba=32,
  baab=64,
  abba=128,
  a_bab=256,
  a_bba=512,
  baa_b=1024,
  aba_b=2048,
  a_ba_b=4096,
  a_abb=8192,
  a_a_bb=16384,
  ba_ab=32768,
  bb_a_a=65536,
  bba_a=131072,
  b_baa=262144,
  b_b_aa=524288,
  ab_ba=1048576,
  aa_b_b=2097152,
  aab_b=4194304,
  a_ab_b=8388608,
  a_a_b_b=16777216,
  b_ba_a=33554432
};

string StrSimpleConnectors[]= {"aabb"  , "bbaa"  ,  "aa.bb"  ,  "bb.aa"  ,
  "abab"  ,  "baba"  ,  "baab"  ,  "abba"  ,  "a.bab"  ,  "a.bba"  ,
  "baa.b"  ,  "aba.b"  ,  "a.ba.b"  ,  "a.abb"  ,  "a.a.bb"  ,
  "ba.ab"  ,  "bb.a.a"  ,  "bba.a"  ,  "b.baa"  ,  "b.b.aa"  ,
  "ab.ba"  ,  "aa.b.b"  ,  "aab.b"  ,  "a.ab.b"  ,  "a.a.b.b"  ,
  "b.ba.a"  };

class STVector
{
private:
  inline int Count(int vec);
  inline int Str2Simple(string s);
  inline ListExpr Vector2List();
public:

  int v;
  int count;
  STVector():v(0), count(0){}
  STVector(int vec):v(vec), count(Count(vec)){};
  STVector(STVector* vec):v(vec->v), count(vec->count){};
  ~STVector(){};

  bool Add(string simple);
  
  bool Add(int simple);
  
  //Secondo framework support
  static Word In( const ListExpr typeInfo, const ListExpr instance,
              const int errorPos, ListExpr& errorInfo, bool& correct );
  static ListExpr Out( ListExpr typeInfo, Word value );
  static Word Create( const ListExpr typeInfo );
  static void Delete( const ListExpr typeInfo, Word& w );
  static bool Open( SmiRecord& valueRecord, size_t& offset, 
      const ListExpr typeInfo, Word& value );
  static bool Save( SmiRecord& valueRecord, size_t& offset, 
                    const ListExpr typeInfo, Word& value );
  static void Close( const ListExpr typeInfo, Word& w );
  static Word Clone( const ListExpr typeInfo, const Word& w );
  static int SizeOfObj();
  static ListExpr Property();
  static bool KindCheck( ListExpr type, ListExpr& errorInfo );
};


class CSP  {
}csp;

class STPattern
{
  
  
};

} // namespace STPattern



#endif /* STPATTERNALGEBRA_H_ */
