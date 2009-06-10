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
  
  bool ApplyVector(Interval<CcReal>& p1, Interval<CcReal>& p2);
  
  bool ApplySimple(Interval<CcReal>& p1, Interval<CcReal>& p2, 
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


class CSP  
{
public:
  vector< vector<Interval<CcReal> > > SA;
  vector<Supplier> Agenda;
  map<string, int> VarAliasMap;
  vector< vector<Supplier> >ConstraintGraph;
  int count;
  int iterator;
  Interval<CcReal> nullInterval;
  vector<int> assignedVars;
  
  
  CSP():count(0),iterator(-1), 
  nullInterval(CcReal(true,0.0),CcReal(true,0.0), true,true)
  {}
  
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
  
  bool Solve()
  {
    bool debugme=true;
    int varIndex;
    Word Value;
    vector<Interval<CcReal> > domain(0);
    while( (varIndex= PickVariable()) != -1)
    {
      qp->Request(Agenda[varIndex], Value);
      Agenda[varIndex]=0;
      MBool2Vec((MBool*)Value.addr, domain);
      if(domain.size()==0) return false;
      if(Extend(varIndex, domain)!= 0) return false;
      if(SA.size()==0) return false;
      if(debugme)
        Print();
    }
    return true;
  }
  
  void IntervalInstant2IntervalCcReal(const Interval<Instant>& in, 
      Interval<CcReal>& out)
  {
    out.start.Set(in.start.IsDefined(), in.start.ToDouble());
    out.end.Set(in.end.IsDefined(), in.end.ToDouble());
  }
  
  
  int MBool2Vec(const MBool* mb, vector<Interval<CcReal> >& vec)
  {
    const UBool* unit;
    vec.clear();
    Interval<CcReal> elem(CcReal(true,0.0),CcReal(true,0.0),true,true);
    for(int i=0; i<mb->GetNoComponents(); i++)
    {
      mb->Get(i, unit);
      if( ((CcBool)unit->constValue).GetValue())
      {
        IntervalInstant2IntervalCcReal(unit->timeInterval, elem);
        vec.push_back(elem);
      }
    }
    return 0;
  }
  
  int Extend(int index, vector<Interval<CcReal> >& domain )
  {
    vector<Interval<CcReal> > sa(count);
    
    if(SA.size() == 0)
    {
      for(int i=0; i<count; i++)
        sa[i].CopyFrom(nullInterval);
      for(unsigned int i=0; i<domain.size(); i++)
      {
        
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
        sa[index]= domain[j];
        if(IsSupported(sa,index))
          SA.push_back(sa);
      }
      SA.erase(SA.begin());
    }
    return 0;
  }
  
  bool IsSupported(vector<Interval<CcReal> > sa, int index)
  {
    
    bool supported=false; 
    for(unsigned int i=0; i<assignedVars.size()-1; i++)
    {
      for(unsigned int j=0; j<assignedVars.size(); j++)
      {
        if(i== (unsigned int)index || j == (unsigned int)index )
        {
          if(ConstraintGraph[assignedVars[i]][assignedVars[j]] != 0)
          {
            supported= CheckConstraint(sa[assignedVars[i]], 
                sa[assignedVars[j]], 
                ConstraintGraph[assignedVars[i]][assignedVars[j]]);
            if(!supported) return false;
          }

          if(ConstraintGraph[assignedVars[j]][assignedVars[i]] != 0)
          {
            supported= CheckConstraint(sa[assignedVars[j]], 
                sa[assignedVars[i]], 
                ConstraintGraph[assignedVars[j]][assignedVars[i]]);
            if(!supported) return false;
          }
        }
      }
    }
    return true;
  }
  
  bool CheckConstraint(Interval<CcReal>& p1, Interval<CcReal>& p2, 
      Supplier constraint)
  {
    Word Value;
    qp->Request(constraint,Value);
    STVector* vec= (STVector*) Value.addr;
    return vec->ApplyVector(p1, p2);
  }
  
  int PickVariable()
  {
    bool debugme=true;
    vector<int> vars(0);
    vector<double> numconstraints(0);
    double cnt=0;
    int index=-1;
    for(unsigned int i=0;i<Agenda.size();i++)
    {
      if(Agenda[i] != 0)
      {
        vars.push_back(i);
        cnt=0;
        for(unsigned int r=0; r< ConstraintGraph.size(); r++)
        {
          for(unsigned int c=0; c< ConstraintGraph[r].size(); c++)
          {
            if( r == i && ConstraintGraph[r][c] != 0)
            {
              cnt+=0.5;
              for(unsigned int v=0; v< assignedVars.size(); v++)
              {
                if(c == (unsigned int)assignedVars[v]) cnt+=0.5;
              }
            }
            
            if( c == i && ConstraintGraph[r][c] != 0)
            {
              cnt+=0.5;
              for(unsigned int v=0; v< assignedVars.size(); v++)
              {
                if(r == (unsigned int)assignedVars[v]) cnt+=0.5;
              }
            }
          }
        }
        numconstraints.push_back(cnt);
      }
    }
    double max=-1;
    
    for(unsigned int i=0; i<numconstraints.size(); i++)
    {
      if(numconstraints[i]>max){ max=numconstraints[i]; index=vars[i];}
    }
    if(debugme)
    {
      for(unsigned int i=0; i<numconstraints.size(); i++)
        cout<< "\nConnectivity of variable " <<vars[i] <<"  = "
        << numconstraints[i];
      cout<<endl<< "Picking variable "<< index<<endl;
      cout.flush();
    }
    if(index== -1) return -1; 
    assignedVars.push_back(index);
    return index;
  }

  
  bool MoveNext()
  {
    if(iterator < (signed int)SA.size()-1)
      iterator++;
    else
      return false;
    return true;
  }
  
  bool GetStart(string alias, Instant& result)
  {
    map<string, int>::iterator it;

    it=VarAliasMap.find(alias);
    if(it== VarAliasMap.end()) return false;
    
    int index=(*it).second;
    result.ReadFrom(SA[iterator][index].start.GetRealval());
    return true;
  }
  
  bool GetEnd(string alias, Instant& result)
  {
    map<string, int>::iterator it;

    it=VarAliasMap.find(alias);
    if(it== VarAliasMap.end()) return false;

    int index=(*it).second;
    result.ReadFrom(SA[iterator][index].end.GetRealval());
    return true;
  }
  
  void Print()
  {
    cout<< "\n==========================\nSA.size() = "<< SA.size()<< endl;
    for(unsigned int i=0; i< SA.size(); i++)
    {
      for(unsigned int j=0; j<SA[i].size(); j++)
      {
        cout<<(SA[i][j].start.GetRealval()-3444)*24*60*60<< "\t "<<
        (SA[i][j].end.GetRealval()-3444)*24*60*60 <<" | ";
      }
      cout<<endl;
    }
    cout.flush();
  }
  int Clear()
  {
    SA.clear();
    Agenda.clear();
    ConstraintGraph.clear();
    VarAliasMap.clear();
    assignedVars.clear();
    count=0;
    iterator=-1;
    return 0;
  }
}csp;


} // namespace STPattern



#endif /* STPATTERNALGEBRA_H_ */
