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
enum SimpleConnector {
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
  
  bool ApplyVector(Interval<Instant>& p1, Interval<Instant>& p2);
  
  bool ApplySimple(Interval<Instant>& p1, Interval<Instant>& p2, 
      int simple);
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
public:
  vector< vector<Interval<Instant> > > SA;
  vector<Supplier> Agenda;
  map<string, int> VarAliasMap;
  vector< vector<Supplier> >ConstraintGraph;
  int count;
  Interval<Instant> nullInterval;
  
  CSP():count(0),
  nullInterval(Instant(0,0,instanttype), Instant(0,0,instanttype),true,true){}
  
  int AddVariable(string alias, Supplier handle)
  {
    Agenda.push_back(handle);
    VarAliasMap[alias]=count;
    count++;
    ConstraintGraph.resize(count);
    for(int i=0; i<count; i++)
      ConstraintGraph[i].resize(count);
    return 0;
  }
  
  int AddConstraint(string alias1, string alias2, Supplier handle)
  {
    int index1=-1, index2=-1;
    try{
      index1= VarAliasMap[alias1];
      index2= VarAliasMap[alias2];
      if(index1==index2)
        throw;
    }
    catch(...)
    {
      return -1;
    }
    ConstraintGraph[index1][index2]= handle;
    return 0;
  }
  
  int MBool2Vec(const MBool* mb, vector<Interval<Instant> >& vec)
  {
    const UBool* unit;
    vec.clear();
    for(int i=0; i<mb->GetNoComponents(); i++)
    {
      mb->Get(i, unit);
      if( ((CcBool)unit->constValue).GetValue())
        vec.push_back(unit->timeInterval);
    }
    return 0;
  }
  
  int Extend(int index, vector<Interval<Instant> >& domain )
  {
    vector<Interval<Instant> > sa(count);
    for(int i=0; i<count; i++)
      sa[i]= nullInterval;
    if(SA.size() == 0)
    {
      for(unsigned int i=0; i<domain.size(); i++)
      {
        sa.clear();
        sa[index]= domain[i];
        SA.push_back(sa);
      }
      return 0;
    }
    
    unsigned int SASize= SA.size();
    for(unsigned int i=0; i<SASize; i++)
    {
      sa= SA[0];
      for(unsigned int j=0; j< domain.size(); j++)
      {
        sa[index]= domain[i];
        if(IsSupported(sa))
          SA.push_back(sa);
      }
      SA.erase(SA.begin());
    }
    return 0;
  }
  
  bool IsSupported(vector<Interval<Instant> > sa)
  {
    vector<int> assignedVars;
    bool supported=false;
    for(unsigned int i=0; i<sa.size(); i++)
    {
      if(sa[i] != nullInterval)
        assignedVars.push_back(i);
    }
    
    for(unsigned int i=0; i<assignedVars.size()-1; i++)
    {
      for(unsigned int j=0; j<assignedVars.size(); j++)
      {
        if(ConstraintGraph[assignedVars[i]][assignedVars[j]] != 0)
        {
          supported= CheckConstraint(sa[assignedVars[i]], sa[assignedVars[j]], 
              ConstraintGraph[assignedVars[i]][assignedVars[j]]);
          if(!supported) return false;
        }
        
        if(ConstraintGraph[assignedVars[j]][assignedVars[i]] != 0)
        {
          supported= CheckConstraint(sa[assignedVars[j]], sa[assignedVars[i]], 
              ConstraintGraph[assignedVars[j]][assignedVars[i]]);
          if(!supported) return false;
        }
      }
    }
    return true;
  }
  
  bool CheckConstraint(Interval<Instant>& p1, Interval<Instant>& p2, 
      Supplier constraint)
  {
    Word Value;
    qp->Request(constraint,Value);
    STVector* vec= (STVector*) Value.addr;
    return vec->ApplyVector(p1, p2);
  }
    
}csp;

class STPattern
{
  
  
};

} // namespace STPattern



#endif /* STPATTERNALGEBRA_H_ */
