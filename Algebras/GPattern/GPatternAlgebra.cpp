/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Source File of the Spatiotemporal Group Pattern Algebra

JAN, 2010 Mahmoud Sakr

[TOC]

1 Overview

2 Defines and includes

*/

#include "GPatternAlgebra.h"
using namespace mset;

namespace GPattern{

/*
3 Classes

*/


int GPatternSolver::PickVariable()
{
  bool debugme=false;
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
          if( r == i && ConstraintGraph[r][c].size() != 0)
          {
            cnt+= ConstraintGraph[r][c].size() * 0.5;
            for(unsigned int v=0; v< assignedVars.size(); v++)
            {
              if(c == (unsigned int)assignedVars[v]) 
                cnt+=0.5 * ConstraintGraph[r][c].size();
            }  

          }

          if( c == i && ConstraintGraph[r][c].size() != 0)
          {
            cnt+=0.5 * ConstraintGraph[r][c].size();
            for(unsigned int v=0; v< assignedVars.size(); v++)
            {
              if(r == (unsigned int)assignedVars[v]) 
                cnt+=0.5 * ConstraintGraph[r][c].size();
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

bool GPatternSolver::MoveNext()
{
  if(iterator < (signed int)SA.size()-1)
    iterator++;
  else
    return false;
  return true;
}


bool GPatternSolver::GetStart(string alias, Instant& result)
{
  map<string, int>::iterator it;

  it=VarAliasMap.find(alias);
  if(it== VarAliasMap.end()) return false;

  int index=(*it).second;
  result.ReadFrom(SA[iterator][index].first.start.GetRealval());
  return true;
}

bool GPatternSolver::GetEnd(string alias, Instant& result)
{
  map<string, int>::iterator it;

  it=VarAliasMap.find(alias);
  if(it== VarAliasMap.end()) return false;

  int index=(*it).second;
  result.ReadFrom(SA[iterator][index].first.end.GetRealval());
  return true;
}

ostream& GPatternSolver::Print(ostream &os)
{
  os<< "\n==========================\nSA.size() = "<< SA.size()<< endl;
  Instant tmp(instanttype);
  for(unsigned int i=0; i< SA.size(); i++)
  {
    os<< "Tuple number:" << i;
    for(unsigned int j=0; j<SA[i].size(); j++)
    {
      if(Agenda[j] !=0) continue;
      os<< "Attribute number:" << j;
      SA[i][j].second->Print(os);
//      tmp.ReadFrom(SA[i][j].first.start.GetRealval());
//      tmp.Print(os);
//      os<< "\t ";
//      tmp.ReadFrom(SA[i][j].first.end.GetRealval());
//      tmp.Print(os);
//      os<<"\t{";
//      for(set<int>::iterator itr= SA[i][j].second.begin(); 
//        itr!= SA[i][j].second.end(); ++itr)
//      {
//        os<< *itr << ", ";
//      }
//      os<<"} |";
    }
    os<<endl;
  }
  return os;
}

int GPatternSolver::Clear()
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

void GPatternSolver::IntervalInstant2IntervalCcReal(
    const Interval<Instant>& in, Interval<CcReal>& out)
{
  bool debugme=false;
  out.start.Set(in.start.IsDefined(), in.start.ToDouble());
  out.end.Set(in.end.IsDefined(), in.end.ToDouble());
  out.lc= in.lc;
  out.rc= in.rc;
  if(debugme)
  {
    in.Print(cerr);
    out.Print(cerr);
  }
  
}

/*
Extend

*/

bool GPatternSolver::Extend(int varIndex)
{
  bool debugme= false;
  vector< pair< Interval<CcReal>, MSet* > > sa(count);
  qp->Open(Agenda[varIndex]);
  Interval<CcReal> deftime;
  Interval<Instant> period;
  Periods periods(0);
  set<int> val;
  USetRef unit;
  Word Value;

  if(SA.size() == 0) //This is the first varirable to be evaluated
  {
    for(int i=0; i<count; i++)
      sa[i].first.CopyFrom(nullInterval);

    qp->Request(Agenda[varIndex], Value);
    while(qp->Received(Agenda[varIndex]))
    {
      MSet* res = static_cast<MSet*>(Value.addr)->Clone();
      ToDelete.push_back(res);
      if(res->IsDefined())
      {
        if(debugme) res->Print(cerr);
        res->DefTime(periods);
        periods.Get(0, period);
        IntervalInstant2IntervalCcReal(period, deftime);
        sa[varIndex].first= deftime;
        sa[varIndex].second= res;
        SA.push_back(sa);
      }
      qp->Request(Agenda[varIndex], Value);
    }
    qp->Close(Agenda[varIndex]); 
  }
  else
  {
    vector< pair< Interval<CcReal>, MSet* > > stream;
    pair< Interval<CcReal>, MSet* > elem;
    qp->Request(Agenda[varIndex], Value);
    while(qp->Received(Agenda[varIndex]))
    {
      MSet* res = static_cast<MSet*>(Value.addr)->Clone() ;
      ToDelete.push_back(res);
      if(res->IsDefined())
      {
        res->DefTime(periods);
        periods.Get(0, period);
        IntervalInstant2IntervalCcReal(period, elem.first);
        elem.second= res;
        stream.push_back(elem);
      }
      qp->Request(Agenda[varIndex], Value);
    }
    // SA has already entries 
    unsigned int SASize= SA.size();
    for(unsigned int i=0; i<SASize; i++)
    {
      sa= SA[0];
      
      for(unsigned int j=0; j<stream.size(); ++j)
      {
        sa[varIndex]= stream[j];
        if(IsSupported(sa, varIndex))
          SA.push_back(sa);
      } 
      SA.erase(SA.begin());
    }
  }
  if(debugme) Print(cerr);
  return true;
}

bool GPatternSolver::IsSupported(
    vector< pair<Interval<CcReal>, MSet* > >& sa, int index)
{
  bool supported=false; 
  for(unsigned int i=0; i<assignedVars.size()-1; i++)
  {
    for(unsigned int j=0; j<assignedVars.size(); j++)
    {
      if(i== (unsigned int)index || j == (unsigned int)index )
      {
        if(ConstraintGraph[assignedVars[i]][assignedVars[j]].size() != 0)
        {
          supported= CheckConstraint(sa[assignedVars[i]].first, 
              sa[assignedVars[j]].first, 
              ConstraintGraph[assignedVars[i]][assignedVars[j]]);
          if(!supported) return false;
        }

        if(ConstraintGraph[assignedVars[j]][assignedVars[i]].size() != 0)
        {
          supported= CheckConstraint(sa[assignedVars[j]].first, 
              sa[assignedVars[i]].first, 
              ConstraintGraph[assignedVars[j]][assignedVars[i]]);
          if(!supported) return false;
        }
      }
    }
  }
  return supported;
}

/*
CheckCopnstraint

*/

bool GPatternSolver::CheckConstraint(Interval<CcReal>& p1, 
    Interval<CcReal>& p2, vector<Supplier> constraint)
{
  bool debugme=false;
  Word Value;
  bool satisfied=false;
  for(unsigned int i=0;i< constraint.size(); i++)
  {
    if(debugme)
    {
      cout<< "\nChecking constraint "<< qp->GetType(constraint[i])<<endl;
      cout.flush();
    }
    qp->Request(constraint[i],Value);
    STP::STVector* vec= (STP::STVector*) Value.addr;
    satisfied= vec->ApplyVector(p1, p2);
    if(!satisfied) return false;
  }
  return true; 
}

int GPatternSolver::AddConstraint(string alias1, string alias2, Supplier handle)
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
  ConstraintGraph[index1][index2].push_back(handle);
  return 0;
}

int GPatternSolver::AddVariable(string alias, Supplier handle)
{
  Agenda.push_back(handle);
  VarAliasMap[alias]=count;
  count++;
  ConstraintGraph.resize(count);
  for(int i=0; i<count; i++)
    ConstraintGraph[i].resize(count);
  return 0;
}

bool GPatternSolver::Solve()
{
  int varIndex;
  while( (varIndex= PickVariable()) != -1)
  {
    if(! Extend(varIndex)) return false;
    if(SA.size() == 0)  return false;
    Agenda[varIndex]=0;
  }
  if(SA.size() == 0)
    return false;
  return true;
}

void GPatternSolver::WriteTuple(Tuple* tuple)
{
  vector< pair< Interval<CcReal>, MSet* > > sa= SA[iterator];
  USet uset(true);
  Instant instant(instanttype);
  MSet mset(0);
  for(unsigned int i=0; i<sa.size(); ++i)
  {
//    MSet* mset= new MSet(0);
//    uset.constValue.Clear();
//    instant.ReadFrom(sa[i].first.start.GetValue());
//    uset.timeInterval.start= instant;
//    instant.ReadFrom(sa[i].first.end.GetValue());
//    uset.timeInterval.end= instant;
//    uset.timeInterval.lc= sa[i].first.lc;
//    uset.timeInterval.rc= sa[i].first.rc;
//    for(set<int>::iterator it=sa[i].second.begin(); 
//       it != sa[i].second.end(); ++it)
//      uset.constValue.Insert(*it);
//    uset.SetDefined(true);
//    uset.constValue.SetDefined(true);
//    assert(uset.IsValid());
//    mset->Add(uset);
    tuple->PutAttribute(i, sa[i].second->Clone());
  }
}

/*
4 Algebra Types and Operators 

*/


ListExpr RowColTM(ListExpr args)
{
  bool debugme= false;

  string argstr;

  if(debugme)
  {
    cout<<endl<< nl->ToString(args)<<endl;
    cout.flush();
  }
  ListExpr errorInfo;
  nl->WriteToString(argstr, args);
  CHECK_COND(nl->ListLength(args) == 1 &&
      nl->IsAtom(nl->First(args)) && 
      am->CheckKind("TEMPORAL", nl->First(args), errorInfo),
      "Operators row/col expect one argument of kind TEMPORAL \n but got: " 
      + argstr + ".");

  if(debugme)
  {
    cout<<endl<<endl<<"Operator row/col accepted the input";
    cout.flush();
  }
  return nl->First(args);
}


ListExpr CrossPatternTM(ListExpr args)
{
  bool debugme= false;

  string argstr;

  if(debugme)
  {
    cout<<endl<< nl->ToString(args)<<endl;
    cout.flush();
  }

  nl->WriteToString(argstr, args);
  CHECK_COND(nl->ListLength(args) == 5,
      "Operator crosspattern expects 5 arguments \n but got: " 
      + argstr + ".");
   
  ListExpr tuple =  nl->First(args),
  liftedPred = nl->Second(args),  
  count  =   nl->Third(args),
  quantifier = nl->Fourth(args),
  graphConnectivity= nl->Fifth(args);    

  nl->WriteToString(argstr, tuple);
    CHECK_COND( listutils::isTupleDescription(tuple) ,
          "Operator crosspattern expects a tuple as the first argument. "
          "\nBut got: " + argstr + ".");
  
  nl->WriteToString(argstr, liftedPred);
  CHECK_COND( nl->IsAtom(liftedPred) && 
    nl->SymbolValue(liftedPred) == "mbool",
        "Operator crosspattern expects an mbool as the second argument. "
        "\nBut got: " + argstr + ".");

  nl->WriteToString(argstr, count);
  CHECK_COND( nl->IsAtom(count) && 
    nl->SymbolValue(count) == "int",
        "Operator crosspattern expects an int as the third argument. "
        "\nBut got: " + argstr + ".");
  
  nl->WriteToString(argstr, quantifier);
  CHECK_COND( nl->IsAtom(quantifier) && 
    (nl->SymbolValue(quantifier) == "exactly" || 
       nl->SymbolValue(quantifier) == "atleast"),
     "Operator crosspattern expects a quantifier (exactly or atleast) as the "
     "fourth argument. \nBut got: " + argstr + ".");
  
  nl->WriteToString(argstr, graphConnectivity);
    CHECK_COND( nl->IsAtom(graphConnectivity) && 
      (nl->SymbolValue(graphConnectivity) == "partiallyconnected" || 
         nl->SymbolValue(graphConnectivity) == "fullyconnected"),
      "Operator crosspattern expects a quantifier (partiallyconnected or "
      "fullyconnected) as the fifth argument. \nBut got: " + argstr + ".");
    
  ListExpr result = nl->SymbolAtom("mflock");
  if(debugme)
  {
    cout<<endl<<endl<<"Operator crosspattern accepted the input";
    cout.flush();
  }
  return result;
}

ListExpr GPatternTM(ListExpr args)
{
  bool debugme= false;

  string argstr;

  if(debugme)
  {
    cout<<endl<< nl->ToString(args)<<endl;
    cout.flush();
  }

  nl->WriteToString(argstr, args);
  if(nl->ListLength(args) != 5)
  {
    ErrorReporter::ReportError( 
        "Operator gpattern expects 5 arguments \n but got: " + argstr + ".");
    return nl->SymbolAtom("typeerror");
  }
      
  
  ListExpr tuple =  nl->First(args),
  liftedPred = nl->Second(args),
  duration = nl->Third(args),
  count  =   nl->Fourth(args),
  quantifier = nl->Fifth(args);
  
  nl->WriteToString(argstr, liftedPred);
  if( nl->IsAtom(liftedPred) || nl->ListLength(liftedPred) != 3 || 
      nl->SymbolValue(nl->First(liftedPred)) != "map" ||
      nl->SymbolValue(nl->Third(liftedPred)) != "mbool")
  {
    ErrorReporter::ReportError(
        "Operator gpattern expects an mbool as the second argument. "
        "\nBut got: " + argstr + ".");
    return nl->SymbolAtom("typeerror");
  }
        
  nl->WriteToString(argstr, duration);
  if( !nl->IsAtom(duration) || nl->SymbolValue(duration) != "duration")
  {
    ErrorReporter::ReportError(
        "Operator gpattern expects a duration as the third argument. "
        "\nBut got: " + argstr + ".");
    return nl->SymbolAtom("typeerror");
  }
          
  nl->WriteToString(argstr, count);
  if( !nl->IsAtom(count) || nl->SymbolValue(count) != "int")
  {
    ErrorReporter::ReportError(
        "Operator gpattern expects an int as the fourth argument. "
        "\nBut got: " + argstr + ".");
    return nl->SymbolAtom("typeerror");
  }
        
  nl->WriteToString(argstr, quantifier);
  if( !nl->IsAtom(quantifier) || 
    (nl->SymbolValue(quantifier) != "exactly" && 
       nl->SymbolValue(quantifier) != "atleast"))
  {
    ErrorReporter::ReportError(
        "Operator gpattern expects a quantifier (exactly or atleast) as the "
        "fifth argument. \nBut got: " + argstr + ".");
    return nl->SymbolAtom("typeerror");
  }

  nl->WriteToString(argstr, tuple);
  if(!listutils::isTupleDescription(tuple) && !listutils::isTupleStream(tuple))
  {
    ErrorReporter::ReportError(
       "Operator gpattern expects a tuple or a stream(tuple) as the first "
       "argument. \nBut got: " + argstr + ".");
    return nl->SymbolAtom("typeerror");
  }
  
  ListExpr result = 
    nl->TwoElemList(nl->SymbolAtom("stream"), nl->SymbolAtom("mset"));
  if(debugme)
  {
    cout<<endl<<endl<<"Operator gpattern accepted the input";
    cout.flush();
  }
  return result;
}

/*
Type map ReportPattern

*/

ListExpr ReportPatternTM(ListExpr args)
{
  bool debugme= false;

  string argstr;

  if(debugme)
  {
    cout<<endl<< nl->ToString(args)<<endl;
    cout.flush();
  }

  nl->WriteToString(argstr, args);
  CHECK_COND(nl->ListLength(args) == 4,
      "Operator reportpattern expects 4 arguments \n but got: " 
      + argstr + ".");
  
  ListExpr streamExpr = nl->First(args),   //stream(tuple(int, mx))
  NamedPatternoidList  = nl->Second(args),  //named list of patternoids
  ConstraintList = nl->Third(args),    //TConstraint list
  BoolCondition  = nl->Fourth(args);    

  nl->WriteToString(argstr, streamExpr);
  CHECK_COND( listutils::isTupleStream(streamExpr) ,
      "Operator reportpattern expects stream(tuple(X)) as first argument."
      "\nBut got: " + argstr + ".");
  
  ListExpr errorInfo;
  ListExpr tuple = nl->Second(nl->Second(streamExpr));
  nl->WriteToString(argstr, streamExpr);
  CHECK_COND( nl->ListLength(tuple) == 2 &&
    nl->IsAtom     (nl->Second(nl->First (tuple))) && 
    nl->SymbolValue(nl->Second(nl->First (tuple)))== "int" &&
    nl->IsAtom     (nl->Second(nl->Second(tuple))) && 
    am->CheckKind("TEMPORAL", nl->Second(nl->Second(tuple)), errorInfo),  
        "Operator reportpattern expects stream(tuple(int mx)) as first "
        "argument.\nBut got: " + argstr + ".");
  
  nl->WriteToString(argstr, NamedPatternoidList);
  CHECK_COND( ! nl->IsAtom(NamedPatternoidList) ,
      "Operator  reportpattern expects as second argument a "
      "list of aliased patternoid reporting functions.\n"
      "But got: '" + argstr + "'.\n" );
  
  ListExpr NamedPatternoidListRest = NamedPatternoidList;
  ListExpr NamedPatternoid;
  vector<ListExpr> aliases;
  vector<ListExpr>::iterator it; 
  while( !nl->IsEmpty(NamedPatternoidListRest) )
  {
    NamedPatternoid = nl->First(NamedPatternoidListRest);
    NamedPatternoidListRest = nl->Rest(NamedPatternoidListRest);
    nl->WriteToString(argstr, NamedPatternoid);

    CHECK_COND
    ((nl->ListLength(NamedPatternoid) == 2 &&
        nl->IsAtom(nl->First(NamedPatternoid))&&
        listutils::isMap<1>(nl->Second(NamedPatternoid))&&
        listutils::isDATAStream(nl->Third(nl->Second(NamedPatternoid)))&&
        nl->IsAtom((nl->Second(nl->Third(nl->Second(NamedPatternoid)))))&&
        nl->SymbolValue((nl->Second(nl->Third(nl->Second(NamedPatternoid)))))
        == "mset"),
        "Operator reportpattern expects a list of aliased patternoid "
        "reporting operators. \nBut got: " + argstr + ".");
    aliases.push_back(nl->First(NamedPatternoid));
  }

  nl->WriteToString(argstr, ConstraintList);
  ListExpr ConstraintListRest = ConstraintList;
  ListExpr STConstraint;
  while( !nl->IsEmpty(ConstraintListRest) )
  {
    STConstraint = nl->First(ConstraintListRest);
    ConstraintListRest = nl->Rest(ConstraintListRest);

    CHECK_COND((nl->IsAtom(STConstraint)&&
        nl->SymbolValue(STConstraint)== "bool"),
        "Operator reportpattern expects a list of temporal connectors. "
        "\nBut got: " + argstr + ".");
  }

  nl->WriteToString(argstr, BoolCondition);
  CHECK_COND( nl->IsAtom(BoolCondition) && 
    nl->SymbolValue(BoolCondition) == "bool",
        "Operator reportpattern expects a boolean condition. "
        "\nBut got: " + argstr + ".");
  
  
  it= aliases.begin();
  ListExpr attr = nl->TwoElemList(*it, nl->SymbolAtom("mset"));
  ListExpr attrList = nl->OneElemList(attr);
  ListExpr lastlistn = attrList;
  ++it;
  while (it != aliases.end())
  {
    attr = nl->TwoElemList(*it, nl->SymbolAtom("mset"));
    lastlistn = nl->Append(lastlistn, attr);
    ++it;
  }
    
  ListExpr result = 
    nl->TwoElemList(nl->SymbolAtom("stream"),
        nl->TwoElemList(nl->SymbolAtom("tuple"), attrList));
  if(debugme)
  {
    cout<<endl<<endl<<"Operator reportpattern accepted the input";
    cout<< "return type is "<< nl->ToString(result);
    cout.flush();
  }
  return result;
}

ListExpr EmptyMSetTM(ListExpr args)
{
  string argstr;
  nl->WriteToString(argstr, args);
  CHECK_COND(nl->IsEmpty(args),
      "Operator emptymset expects zero paramenters.\n but got "+ argstr+ ".");
  ListExpr result = nl->SymbolAtom("mset");
  return result;
}

ListExpr MBool2MSetTM(ListExpr args)
{
  string argstr;
  nl->WriteToString(argstr, args);
  CHECK_COND(nl->ListLength(args) == 2 &&
    nl->IsAtom(nl->First(args)) &&  
    nl->SymbolValue(nl->First(args))== "mbool" && 
    nl->IsAtom(nl->Second(args)) &&  
    nl->SymbolValue(nl->Second(args))== "int",
      "Operator mbool2mset expects (mbool int)\n but got "+ argstr+ ".");
  ListExpr result = nl->SymbolAtom("mset");
  return result;
}

ListExpr UnionMSetTM(ListExpr args)
{
  string argstr;
  nl->WriteToString(argstr, args);
  CHECK_COND(nl->ListLength(args) == 2 &&
    nl->IsAtom(nl->First(args)) &&  
    nl->SymbolValue(nl->First(args))== "mset" &&
    nl->IsAtom(nl->Second(args)) &&  
    nl->SymbolValue(nl->Second(args))== "mset",
      "Operator union expects (mset mset)\n but got "+ argstr+ ".");
  ListExpr result = nl->SymbolAtom("mset");
  return result;
}

ListExpr Union2MSetTM(ListExpr args)
{
  string argstr;
  nl->WriteToString(argstr, args);
  CHECK_COND(nl->ListLength(args) == 2 &&
    nl->IsAtom(nl->First(args)) &&  
    nl->SymbolValue(nl->First(args))== "mset" &&
    nl->IsAtom(nl->Second(args)) &&  
    nl->SymbolValue(nl->Second(args))== "mset",
      "Operator union expects (mset mset)\n but got "+ argstr+ ".");
  ListExpr result = nl->SymbolAtom("mset");
  return result;
}

ListExpr CardinalityMSetTM(ListExpr args)
{
  string argstr;
  nl->WriteToString(argstr, args);
  CHECK_COND( nl->ListLength(args)==1 &&  nl->IsAtom(nl->First(args)) &&  
    nl->SymbolValue(nl->First(args))== "mset" ,
      "Operator cardinality expects mset\n but got "+ argstr+ ".");
  ListExpr result = nl->SymbolAtom("mint");
  return result;
}

/*
TConstraint

*/

ListExpr TConstraintTM(ListExpr args)
{
  bool debugme= false;

  string argstr;

  if(debugme)
  {
    cout<<endl<< nl->ToString(args)<<endl;
    cout.flush();
  }

  ListExpr alias1 = nl->First(args),   
  alias2  = nl->Second(args),      
  temporalconnector = nl->Third(args); 

  nl->WriteToString(argstr, alias1);
  CHECK_COND(( nl->IsAtom(alias1)&&
      nl->SymbolValue(alias1)== "string"),
      "Operator tconstraint expects a predicate label as first "
      "argument.\n But got '" + argstr + "'.");

  nl->WriteToString(argstr, alias2);
  CHECK_COND(( nl->IsAtom(alias2)&&
      nl->SymbolValue(alias2)== "string"),
      "Operator tconstraint: expects a predicate label as second "
      "argument.\n But got '" + argstr + "'.");

  nl->WriteToString(argstr, temporalconnector);
  CHECK_COND(( nl->IsAtom(temporalconnector)&&
      nl->SymbolValue(temporalconnector)== "stvector"),
      "Operator tconstraint: expects a temporal connector as third "
      "argument.\n But got '" + argstr + "'.");

  ListExpr result = nl->SymbolAtom("bool");
  if(debugme)
  {
    cout<<endl<<endl<<"Operator tconstraint accepted the input";
    cout.flush();
  }
  return result;
}

//operators
template <int Alfa>
ListExpr CreateAlfaSetTM(ListExpr args)
{
  bool debugme= false;
  //type names according to secondo type mapping
  char* thetypes[]={"int", "real", "string", "bool"}; 
  string alfa(thetypes[Alfa]);
  string argstr;

  if(debugme)
  {
    cout<<endl<< nl->ToString(args)<<endl;
    cout.flush();
  }

  nl->WriteToString(argstr, args);
  CHECK_COND(nl->ListLength(args) == 2,
      "Operator stream2set expects 2 arguments \n but got: " 
      + argstr + ".");
  
  ListExpr streamExpr = nl->First(args);   //stream(tuple(DATA))

  nl->WriteToString(argstr, nl->First(args));
  CHECK_COND( listutils::isTupleStream(streamExpr) ,
      "Operator stream2set expects stream(tuple(.)) as first "
      "argument.\nBut got: " + argstr + ".");
  
  nl->WriteToString(argstr, nl->Second(args));
  CHECK_COND( nl->IsAtom(nl->Second(args)) ,
        "Operator stream2set expects as second argument an "
        "attribute name.\nBut got: '" + argstr + "'.\n" );

  nl->WriteToString(argstr, args);
  ListExpr attrType;
  int attrIndex= 
    listutils::findAttribute(nl->Second(nl->Second(streamExpr)), 
        nl->ToString(nl->Second(args)), 
        attrType);
  
  
  CHECK_COND( attrIndex != 0,
        "Operator  stream2set expects as second argument an "
        "attribute name that belongs to the first argument.\n"
        "But got: '" + argstr + "'.\n" );
  
  CHECK_COND( nl->IsEqual(attrType, alfa),
        "Operator  stream2set expects a attribute of type " + alfa +
        ".\n But got: '" + argstr + "'.\n" );
  
  ListExpr res= nl->ThreeElemList(nl->SymbolAtom("APPEND"), 
      nl->TwoElemList(nl->IntAtom(attrIndex), attrType), 
      nl->SymbolAtom("intset")); 
  if(debugme)
  {
    cout<<endl<<endl<<"Operator create" + alfa + "set accepted the input";
    cout.flush();
  }
  return res;
}

ListExpr MSet2MRegionTM(ListExpr args)
{
  string msg= nl->ToString(args);
  CHECK_COND( nl->ListLength(args) == 3 ,
      "Operator mset2mregion expects 3 arguments.\nBut got: " + msg + ".");
  
  msg= nl->ToString(nl->First(args));
  CHECK_COND( listutils::isTupleStream(nl->First(args)) ,
      "Operator mset2mregion expects stream(tuple(X)) as first argument."
      "\nBut got: " + msg + ".");

  msg= nl->ToString(nl->Second(args));
  CHECK_COND( nl->IsAtom(nl->Second(args)) && 
      nl->SymbolValue(nl->Second(args)) == "mset",
      "Operator mset2mregion expects an mset as second argument."
      "\nBut got: " + msg + ".");
  
  msg= nl->ToString(nl->Third(args));  
  CHECK_COND( nl->IsAtom(nl->Third(args)) && 
      nl->SymbolValue(nl->Third(args))== "duration",  
          "Operator mset2mregion expects duration as third "
          "argument.\nBut got: " + msg + ".");
  
  ListExpr tuple1 = nl->Second(nl->Second(nl->First(args)));
  msg= nl->ToString(tuple1);
  CHECK_COND( nl->ListLength(tuple1) == 2 &&
    nl->IsAtom     (nl->Second(nl->First (tuple1))) && 
    nl->SymbolValue(nl->Second(nl->First (tuple1)))== "int" &&
    nl->IsAtom     (nl->Second(nl->Second(tuple1))) && 
    nl->SymbolValue(nl->Second(nl->Second(tuple1)))== "mpoint",  
        "Operator mset2mregion expects stream(tuple(int mpoint)) as first "
        "argument.\nBut got: stream(tuple(" + msg + ")).");
  
  return nl->SymbolAtom("movingregion");
}

ListExpr ConvexHullTM(ListExpr args)
{
  string msg= nl->ToString(args);
  CHECK_COND( nl->ListLength(args) == 2 ,
      "Operator convexhull expects 2 arguments.\nBut got: " + msg + ".");
  
  msg= nl->ToString(nl->First(args));
  ListExpr strm= nl->First(args);
  CHECK_COND( nl->ListLength(strm) == 2 &&
      nl->IsAtom(nl->First(strm)) && 
      nl->SymbolValue(nl->First(strm)) == "stream" &&
      nl->IsAtom(nl->Second(strm)) && 
      nl->SymbolValue(nl->Second(strm)) == "mpoint",
      "Operator convexhull expects stream(mpoint) as first argument."
      "\nBut got: " + msg + ".");

  msg= nl->ToString(nl->Second(args));
  CHECK_COND( nl->IsAtom(nl->Second(args)) && 
      nl->SymbolValue(nl->Second(args)) == "instant",
      "Operator convexhull expects an instant as second argument."
      "\nBut got: " + msg + ".");
  
  return nl->SymbolAtom("region");
}

ListExpr MSet2MPointsTM(ListExpr args)
{
  string msg= nl->ToString(args);
  CHECK_COND( nl->ListLength(args) == 3 ,
      "Operator mset2mpoints expects 3 arguments.\nBut got: " + msg + ".");
  
  msg= nl->ToString(nl->First(args));
  CHECK_COND( listutils::isTupleStream(nl->First(args)) ,
      "Operator mset2mpoints expects stream(tuple(X)) as first argument."
      "\nBut got: " + msg + ".");

  msg= nl->ToString(nl->Second(args));
  CHECK_COND( nl->IsAtom(nl->Second(args)) && 
      nl->SymbolValue(nl->Second(args)) == "mset",
      "Operator mset2mpoints expects an mset as second argument."
      "\nBut got: " + msg + ".");

  msg= nl->ToString(nl->Third(args));
  CHECK_COND( nl->IsAtom(nl->Third(args)) && 
      nl->SymbolValue(nl->Third(args)) == "bool",
      "Operator mset2mpoints expects a bool as third argument."
      "\nBut got: " + msg + ".");
    
  ListExpr tuple1 = nl->Second(nl->Second(nl->First(args)));
  msg= nl->ToString(tuple1);
  CHECK_COND( nl->ListLength(tuple1) == 2 &&
    nl->IsAtom     (nl->Second(nl->First (tuple1))) && 
    nl->SymbolValue(nl->Second(nl->First (tuple1)))== "int" &&
    nl->IsAtom     (nl->Second(nl->Second(tuple1))) && 
    nl->SymbolValue(nl->Second(nl->Second(tuple1)))== "mpoint",  
        "Operator mset2mpoints expects stream(tuple(int mpoint)) as first "
        "argument.\nBut got: stream(tuple(" + msg + ")).");
  
  return nl->TwoElemList(nl->SymbolAtom("stream"), nl->SymbolAtom("mpoint"));
}

//ListExpr StartEndTM(ListExpr args)
//{
//  bool debugme=false;
//  string argstr;
//  if(debugme)
//  {
//    cout<<endl<< nl->ToString(args) <<endl;
//    cout<< nl->ListLength(args)  << ".."<< nl->IsAtom(nl->First(args))<<
//    ".."<< nl->SymbolValue(nl->First(args));
//    cout.flush();
//  }
//  nl->WriteToString(argstr, args);
//  CHECK_COND(nl->ListLength(args) == 1 &&
//      nl->IsAtom(nl->First(args)) &&
//      nl->SymbolValue(nl->First(args))== "string",
//      "Operator start/end expects a string symbol "
//      "but got." + argstr);
//  return nl->SymbolAtom("instant");
//}

/*
Value map ReportPattern

*/

int 
ReportPatternVM(Word* args, Word& result,int message, Word& local, Supplier s)
{
  bool debugme=false;
  if(debugme)
  {
    cout<<" Inside ReportPatternVM\n";
    cout<<" Message = " <<message<<endl;
    //    qp->ListOfTree(s, cout);
    cout.flush();
  }

  switch( message )
  {
  case OPEN: // Construct and Solve the CSP, then store it in the "local" 
  { 
    Supplier stream, namedpatternoidlist, namedpatternoid,alias, patternoid, 
    constraintlist, filter, constraint, alias1, alias2, tvector;
    Word Value;
    string aliasstr, alias1str, alias2str;
    int noofpatternoids, noofconstraints;

    stream = args[0].addr;
    namedpatternoidlist = args[1].addr;
    constraintlist= args[2].addr;
    filter= args[3].addr;

    noofpatternoids= qp->GetNoSons(namedpatternoidlist);
    noofconstraints= qp->GetNoSons(constraintlist);

    GPSolver.Clear();
    GPSolver.TheStream= stream;
    for(int i=0; i< noofpatternoids; i++)
    {
      namedpatternoid= qp->GetSupplierSon(namedpatternoidlist, i);
      alias= qp->GetSupplierSon(namedpatternoid, 0);
      patternoid = qp->GetSupplierSon(namedpatternoid, 1);
      aliasstr= nl->ToString(qp->GetType(alias));
      GPSolver.AddVariable(aliasstr,patternoid);
    }

    for(int i=0; i< noofconstraints; i++)
    {
      constraint = qp->GetSupplierSon(constraintlist, i);
      alias1= qp->GetSupplierSon(constraint, 0);
      alias2= qp->GetSupplierSon(constraint, 1);
      tvector= qp->GetSupplierSon(constraint, 2);

      qp->Request(alias1, Value);
      alias1str= ((CcString*) Value.addr)->GetValue();
      qp->Request(alias2, Value);
      alias2str= ((CcString*) Value.addr)->GetValue();
      GPSolver.AddConstraint(alias1str,alias2str, tvector);
    }

    GPSolver.Solve();
    return 0;
  }
  case REQUEST: { // return the next stream element
   bool Part2=false;
   Supplier filter= args[3].addr;
   Word Value;
   while(!Part2 && GPSolver.MoveNext())
   {
     qp->Request(filter, Value);
     Part2= ((CcBool*)Value.addr)->GetValue();
   }
   if(Part2)
   {
     result = qp->ResultStorage( s );
     Tuple* tuple= 
       new Tuple(static_cast<Tuple*>(result.addr)->GetTupleType() );
     GPSolver.WriteTuple(tuple);
     result.setAddr(tuple);
     return YIELD;
   }
   result.addr = 0;
   return CANCEL;
  }
  case CLOSE: { // free the local storage

    GPSolver.Clear();
    local.addr = 0;
  }

  return 0;
  }
  return 0;
}


int TConstraintVM 
(Word* args, Word& result, int message, Word& local, Supplier s)
{
  cerr<< "\nThe operator tconstraint is a facked operator. It may only be "
      "called within the reportpattern operator.";
  assert(0); //this function should never be invoked.
  return 0;
}

int GPatternNestedVM 
(Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool debugme= false;
  switch( message )
  {
  case OPEN: // Iterate over all tuple and compute the stream(mset) result 
  { 
    Word t, value;
    Tuple* tup;
    MBool* mbool;
    CcInt* id;
    ArgVectorPointer funargs;
    CompressedInMemMSet accumlator;
    InMemMSet accumlator2;
    funargs = qp->Argument(s);
    qp->Open(GPSolver.TheStream);
    qp->Request(GPSolver.TheStream, t);
    while (qp->Received(GPSolver.TheStream))
    {
      tup = static_cast<Tuple*>(t.addr);
      id= static_cast<CcInt*>(tup->GetAttribute(0));
      (*funargs)[0] = tup;
      qp->Request(args[1].addr, value);
      mbool = static_cast<MBool*>(value.addr);
      if (mbool->IsDefined())
      {
        accumlator.Buffer(*mbool, id->GetIntval());
//        accumlator2.Union(*mbool, id->GetIntval());
      }

      tup->DeleteIfAllowed();
      qp->Request(GPSolver.TheStream, t);
    }
    accumlator.ConstructFromBuffer();
    if(debugme)
    {
      MSet tmp1(0), tmp2(0);
      accumlator.WriteToMSet(tmp1);
      accumlator2.WriteToMSet(tmp2);
      if(tmp1 != tmp2)
      {
        tmp1.Print(cerr);
        tmp2.Print(cerr);
      }
      tmp1.Print(cerr);
    }
    qp->Close(GPSolver.TheStream);

    qp->Request(args[2].addr, value);
    Instant di( *static_cast<Instant*>(value.addr));
    double d= di.ToDouble()* day2min;
    qp->Request(args[3].addr, value);
    int n= static_cast<CcInt*>(value.addr)->GetIntval();
    string qts= nl->ToString(qp->GetType(args[4].addr));
    GPattern::quantifier q= (qts=="exactly")? 
      GPattern::exactly : GPattern::atleast;
    
    bool changed= true;
    while(changed && accumlator.GetNoComponents() > 0)
    {
      accumlator.RemoveSmallUnits(n);
//      accumlator2.RemoveSmallUnits(n);
      if(debugme)
      {
        MSet tmp1(0), tmp2(0);
        accumlator.WriteToMSet(tmp1);
        accumlator2.WriteToMSet(tmp2);
        if(tmp1 != tmp2)
        {
          tmp1.Print(cerr);
          tmp2.Print(cerr);
        }
      }
      changed=accumlator.RemoveShortElemParts(d);
//      accumlator2.RemoveShortElemParts(d);
      if(debugme)
      {
        MSet tmp1(0), tmp2(0);
        accumlator.WriteToMSet(tmp1);
        accumlator2.WriteToMSet(tmp2);
        if(tmp1 != tmp2)
        {
          tmp1.Print(cerr);
          tmp2.Print(cerr);
        }
      }
    }

    if(debugme)
    {
      MSet tmp1(0);
      accumlator.WriteToMSet(tmp1);
      tmp1.Print(cerr);
    }
       
    vector<InMemMSet>* resStream = new vector<InMemMSet>();
    list<CompressedInMemUSet>::iterator begin= 
      accumlator.units.begin(), end, tmp;
    //cast the CompressedInMemMSet into an InMemMSet
    begin != accumlator.units.end();
    while(begin != accumlator.units.end())
    {
      end= accumlator.GetPeriodEndUnit(begin);
      if(debugme)
      {
        (*begin).Print(cerr);
        (*end).Print(cerr);
      }
      if(q == GPattern::atleast)
      {
        InMemMSet* mset= new InMemMSet();
        tmp= end;
        ++tmp;
        accumlator.WriteToInMemMSet(*mset, begin, tmp);
        if(debugme)
        {
          MSet tmp1(0);
          mset->WriteToMSet(tmp1);
          tmp1.Print(cerr);
        }        
        resStream->push_back(*mset);
      }
      else
      {
        InMemMSet* mset= new InMemMSet();
        tmp= end;
        ++tmp;
        accumlator.WriteToInMemMSet(*mset, begin, tmp);
        if(debugme)
        {
          MSet tmp1(0);
          mset->WriteToMSet(tmp1);
          tmp1.Print(cerr);
        } 
        
        list<InMemUSet>::iterator e= mset->units.end();
        --e;
        GPatternHelper::ComputeAddSubSets(*mset, mset->units.begin(), e, 
            n, d, resStream);
      }

      begin= ++end;
    }
    local= SetWord(resStream);
    return 0;
  }
  case REQUEST: { // return the next stream element
    vector<InMemMSet>* resStreams= static_cast<vector<InMemMSet>*>(local.addr); 
    if ( resStreams->size() != 0)
    {
      MSet* res= new MSet(0);
      (*resStreams->begin()).WriteToMSet(*res);
      resStreams->erase(resStreams->begin());
      result= SetWord(res);  
      return YIELD;
    }
    else
    {
      // you should always set the result to null
      // before you return a CANCEL
      result.addr = 0;
      return CANCEL;
    }
  }
  case CLOSE: { // free the local storage
    vector<InMemMSet>* resStream= static_cast<vector<InMemMSet>* >(local.addr);
    resStream->clear();
    delete resStream;
    local.addr = 0;
  }

  return 0;
  }
  return 0;
}


int GPatternVM 
(Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool debugme= false;
  switch( message )
  {
  case OPEN: // Iterate over all tuple and compute the stream(mset) result 
  { 
    Word t, value;
    Tuple* tup;
    MBool* mbool;
    CcInt* id;
    ArgVectorPointer funargs;
    CompressedInMemMSet accumlator;
    Supplier TheStream = args[0].addr, liftedPred = args[1].addr;
    funargs = qp->Argument(liftedPred);
    qp->Open(TheStream);
    qp->Request(TheStream, t);
    while (qp->Received(TheStream))
    {
      tup = static_cast<Tuple*>(t.addr);
      id= static_cast<CcInt*>(tup->GetAttribute(0));
      (*funargs)[0] = tup;
      qp->Request(args[1].addr, value);
      mbool = static_cast<MBool*>(value.addr);
      if (mbool->IsDefined())
        accumlator.Buffer(*mbool, id->GetIntval());
      tup->DeleteIfAllowed();
      qp->Request(TheStream, t);
    }
    accumlator.ConstructFromBuffer();
    if(debugme)
    {
      MSet tmp1(0);
      accumlator.WriteToMSet(tmp1);
      tmp1.Print(cerr);
    }
    qp->Close(TheStream);

    qp->Request(args[2].addr, value);
    Instant di( *static_cast<Instant*>(value.addr));
    double d= di.ToDouble()* day2min;
    qp->Request(args[3].addr, value);
    int n= static_cast<CcInt*>(value.addr)->GetIntval();
    string qts= nl->ToString(qp->GetType(args[4].addr));
    GPattern::quantifier q= (qts=="exactly")? 
      GPattern::exactly : GPattern::atleast;
    
    bool changed= true;
    while(changed && accumlator.GetNoComponents() > 0)
    {
      accumlator.RemoveSmallUnits(n);
      if(debugme)
      {
        MSet tmp1(0);
        accumlator.WriteToMSet(tmp1);
        tmp1.Print(cerr);
      }
      changed=accumlator.RemoveShortElemParts(d);
      if(debugme)
      {
        MSet tmp1(0);
        accumlator.WriteToMSet(tmp1);
        tmp1.Print(cerr);
      }
    }

    if(debugme)
    {
      MSet tmp1(0);
      accumlator.WriteToMSet(tmp1);
      tmp1.Print(cerr);
    }  
    vector<InMemMSet>* resStream = new vector<InMemMSet>();
    list<CompressedInMemUSet>::iterator begin= 
      accumlator.units.begin(), end, tmp;
    //cast the CompressedInMemMSet into an InMemMSet
    begin != accumlator.units.end();
    while(begin != accumlator.units.end())
    {
      end= accumlator.GetPeriodEndUnit(begin);
      if(debugme)
      {
        (*begin).Print(cerr);
        (*end).Print(cerr);
      }
      if(q == GPattern::atleast)
      {
        InMemMSet* mset= new InMemMSet();
        tmp= end;
        ++tmp;
        accumlator.WriteToInMemMSet(*mset, begin, tmp);
        if(debugme)
        {
          MSet tmp1(0);
          mset->WriteToMSet(tmp1);
          tmp1.Print(cerr);
        }        
        resStream->push_back(*mset);
      }
      else
      {
        InMemMSet* mset= new InMemMSet();
        tmp= end;
        ++tmp;
        accumlator.WriteToInMemMSet(*mset, begin, tmp);
        if(debugme)
        {
          MSet tmp1(0);
          mset->WriteToMSet(tmp1);
          tmp1.Print(cerr);
        } 
        
        list<InMemUSet>::iterator e= mset->units.end();
        --e;
        GPatternHelper::ComputeAddSubSets(*mset, mset->units.begin(), e, 
            n, d, resStream);
      }
      begin= ++end;
    }
    local= SetWord(resStream);
    return 0;
  }
  case REQUEST: { // return the next stream element
    vector<InMemMSet>* resStreams= static_cast<vector<InMemMSet>*>(local.addr); 
    if ( resStreams->size() != 0)
    {
      MSet* res= new MSet(0);
      (*resStreams->begin()).WriteToMSet(*res);
      resStreams->erase(resStreams->begin());
      result= SetWord(res);  
      return YIELD;
    }
    else
    {
      // you should always set the result to null
      // before you return a CANCEL
      result.addr = 0;
      return CANCEL;
    }
  }
  case CLOSE: { // free the local storage
    vector<InMemMSet>* resStream= static_cast<vector<InMemMSet>* >(local.addr);
    resStream->clear();
    delete resStream;
    local.addr = 0;
  }

  return 0;
  }
  return 0;
}

//int GPatternVM 
//(Word* args, Word& result, int message, Word& local, Supplier s)
//{
//  bool debugme= true;
//  switch( message )
//  {
//  case OPEN: // Iterate over all tuple and compute the stream(mset) result 
//  { 
//    Word t, value;
//    Tuple* tup;
//    MBool* mbool;
//    CcInt* id;
//    ArgVectorPointer funargs;
//    InMemMSet accumlator;
//    CompressedInMemMSet c_accumlator;
//    funargs = qp->Argument(s);
//    qp->Open(GPSolver.TheStream);
//    qp->Request(GPSolver.TheStream, t);
//    while (qp->Received(GPSolver.TheStream))
//    {
//      tup = static_cast<Tuple*>(t.addr);
//      id= static_cast<CcInt*>(tup->GetAttribute(0));
//      (*funargs)[0] = tup;
//      qp->Request(args[1].addr, value);
//      mbool = static_cast<MBool*>(value.addr);
//      if (mbool->IsDefined())
//      {
//        accumlator.Union(*mbool, id->GetIntval());
//        c_accumlator.Buffer(*mbool, id->GetIntval());
//      }
//      tup->DeleteIfAllowed();
//      qp->Request(GPSolver.TheStream, t);
//    }
//    c_accumlator.ConstructFromBuffer();
////    if(debugme)
////    {
////      accumlator.Print(cerr);
////      MSet tmp(0);
////      c_accumlator.WriteToMSet(tmp);
////      tmp.Print(cerr);
////    }
//    qp->Close(GPSolver.TheStream);
//
//    qp->Request(args[2].addr, value);
//    Instant di( *static_cast<Instant*>(value.addr));
//    double d= di.ToDouble()* day2min;
//    qp->Request(args[3].addr, value);
//    int n= static_cast<CcInt*>(value.addr)->GetIntval();
//    string qts= nl->ToString(qp->GetType(args[4].addr));
//    GPattern::quantifier q= (qts=="exactly")? 
//        GPattern::exactly : GPattern::atleast;
////    if(debugme)
////      cerr<< qts;
//    
//    bool changed= true;
//    while(changed && accumlator.GetNoComponents() > 0)
//    {
//      accumlator.RemoveSmallUnits(n);
//      c_accumlator.RemoveSmallUnits(n);
////      if(debugme)
////      {
////        MSet tmp1(0), tmp2(0);
////        accumlator.WriteToMSet(tmp1);
////        c_accumlator.WriteToMSet(tmp2);
////        if(tmp1.Compare(&tmp2) != 0)
////        {
////          tmp1.Print(cerr);
////          tmp2.Print(cerr);
////        }
////      }
//      changed= accumlator.RemoveShortElemParts(d);
//      c_accumlator.RemoveShortElemParts(d);
//    }
//
//    if(debugme)
//    {
//      MSet tmp1(0), tmp2(0);
//      accumlator.WriteToMSet(tmp1);
//      c_accumlator.WriteToMSet(tmp2);
//      if(tmp1.Compare(&tmp2) != 0)
//      {
//        tmp1.Print(cerr);
//        tmp2.Print(cerr);
//      }
//    }
//    vector<InMemMSet>* resStream = new vector<InMemMSet>();
//    list<InMemUSet>::iterator begin= accumlator.units.begin(), end;
//    
//    vector<InMemMSet>* cresStream = new vector<InMemMSet>();
//    list<CompressedInMemUSet>::iterator cbegin= 
//      c_accumlator.units.begin(), cend, ctmp;
//    //cast the CompressedInMemMSet into an InMemMSet
//    cbegin != c_accumlator.units.end();
//    while(begin != accumlator.units.end())
//    {
//      end= accumlator.GetPeriodEndUnit(begin);
//      cend= c_accumlator.GetPeriodEndUnit(cbegin);
//      if(debugme)
//      {
//        (*begin).Print(cerr);
//        (*end).Print(cerr);
//        (*cbegin).Print(cerr);
//        (*cend).Print(cerr);
//      }
//      if(q == GPattern::atleast)
//      {
//        InMemMSet* mset= new InMemMSet(accumlator, begin, end);
//        InMemMSet* cmset= new InMemMSet();
//        c_accumlator.WriteToInMemMSet(*cmset, cbegin, cend);
//        if(debugme)
//        {
//          MSet tmp1(0), tmp2(0);
//          mset->WriteToMSet(tmp1);
//          cmset->WriteToMSet(tmp2);
//          if(tmp1.Compare(&tmp2) != 0)
//          {
//            tmp1.Print(cerr);
//            tmp2.Print(cerr);
//          }
//        }        
//        resStream->push_back(*mset);
//        cresStream->push_back(*cmset);
//      }
//      else
//      {
//        InMemMSet* cmset= new InMemMSet();
//        ctmp= cend;
//        ++ctmp;
//        c_accumlator.WriteToInMemMSet(*cmset, cbegin, ctmp);
//        if(debugme)
//        {
//          MSet tmp1(0), tmp2(0);
//          list<InMemUSet>::iterator k= end;
//          ++k;
//          accumlator.WriteToMSet(tmp1, begin, k);
//          cmset->WriteToMSet(tmp2);
//          if(tmp1.Compare(&tmp2) != 0)
//          {
//            tmp1.Print(cerr);
//            tmp2.Print(cerr);
//          }
//        } 
//        GPatternHelper::ComputeAddSubSets(accumlator, begin, end, 
//            n, d, resStream);
//        list<InMemUSet>::iterator e= cmset->units.end();
//        --e;
//        GPatternHelper::ComputeAddSubSets(*cmset, cmset->units.begin(), e, 
//            n, d, cresStream);
//      }
//
//      begin= ++end;
//      cbegin = ++cend;
//    }
//    pair<vector<InMemMSet>*, vector<InMemMSet>* >* r= new 
//      pair<vector<InMemMSet>*, vector<InMemMSet>* >(resStream, cresStream);
//    local= SetWord(r);
//    return 0;
//  }
//  case REQUEST: { // return the next stream element
//    pair<vector<InMemMSet>*, vector<InMemMSet>* >* resStreams= 
//    static_cast<pair<vector<InMemMSet>*, vector<InMemMSet>* >* >(local.addr); 
//    if ( resStreams->first->size() != 0)
//    {
//      MSet* res= new MSet(0);
//      (*resStreams->first->begin()).WriteToMSet(*res);
//      resStreams->first->erase(resStreams->first->begin());
//      result= SetWord(res);
//      if(debugme)
//      {
//        MSet tmp1(0);
//        (*(resStreams->second->begin())).WriteToMSet(tmp1);
//        if(tmp1.Compare(res) != 0)
//        {
//          tmp1.Print(cerr);
//          res->Print(cerr);
//        }
//        resStreams->second->erase(resStreams->second->begin());
//      }  
//      return YIELD;
//    }
//    else
//    {
//      // you should always set the result to null
//      // before you return a CANCEL
//      result.addr = 0;
//      return CANCEL;
//    }
//  }
//  case CLOSE: { // free the local storage
//  vector<InMemMSet>* resStream= static_cast<vector<InMemMSet>* >(local.addr);
//    resStream->clear();
//    delete resStream;
//    local.addr = 0;
//  }
//
//  return 0;
//  }
//  return 0;
//}

//int GPatternVM 
//(Word* args, Word& result, int message, Word& local, Supplier s)
//{
//  bool debugme= true;
//  switch( message )
//  {
//  case OPEN: // Iterate over all tuple and compute the stream(mset) result 
//  { 
//    Word t, value;
//    Tuple* tup;
//    MBool* mbool;
//    CcInt* id;
//    ArgVectorPointer funargs;
//    InMemMSet accumlator;
////    CompressedInMemMSet c_accumlator;
//    funargs = qp->Argument(s);
//    qp->Open(GPSolver.TheStream);
//    qp->Request(GPSolver.TheStream, t);
//    while (qp->Received(GPSolver.TheStream))
//    {
//      tup = static_cast<Tuple*>(t.addr);
//      id= static_cast<CcInt*>(tup->GetAttribute(0));
//      (*funargs)[0] = tup;
//      qp->Request(args[1].addr, value);
//      mbool = static_cast<MBool*>(value.addr);
//      if (mbool->IsDefined())
//      {
//        accumlator.Union(*mbool, id->GetIntval());
////        c_accumlator.Buffer(*mbool, id->GetIntval());
//      }
//      tup->DeleteIfAllowed();
//      qp->Request(GPSolver.TheStream, t);
//    }
////    c_accumlator.ConstructFromBuffer();
////    if(debugme)
////    {
////      accumlator.Print(cerr);
////      MSet tmp(0);
////      c_accumlator.WriteToMSet(tmp);
////      tmp.Print(cerr);
////    }
//    qp->Close(GPSolver.TheStream);
//
//    qp->Request(args[2].addr, value);
//    Instant di( *static_cast<Instant*>(value.addr));
//    double d= di.ToDouble()* day2min;
//    qp->Request(args[3].addr, value);
//    int n= static_cast<CcInt*>(value.addr)->GetIntval();
//    string qts= nl->ToString(qp->GetType(args[4].addr));
//    GPattern::quantifier q= (qts=="exactly")? 
//        GPattern::exactly : GPattern::atleast;
////    if(debugme)
////      cerr<< qts;
//    
//    bool changed= true;
//    while(changed && accumlator.GetNoComponents() > 0)
//    {
//      accumlator.RemoveSmallUnits(n);
////      c_accumlator.RemoveSmallUnits(n);
////      if(debugme)
////      {
////        MSet tmp1(0), tmp2(0);
////        accumlator.WriteToMSet(tmp1);
////        c_accumlator.WriteToMSet(tmp2);
////        if(tmp1.Compare(&tmp2) != 0)
////        {
////          tmp1.Print(cerr);
////          tmp2.Print(cerr);
////        }
////      }
//      changed= accumlator.RemoveShortElemParts(d);
////      c_accumlator.RemoveShortElemParts(d);
//    }
//
//    if(debugme)
//    {
//      MSet tmp1(0), tmp2(0);
//      accumlator.WriteToMSet(tmp1);
////      c_accumlator.WriteToMSet(tmp2);
//      if(tmp1.Compare(&tmp2) != 0)
//      {
//        tmp1.Print(cerr);
//        tmp2.Print(cerr);
//      }
//    }
//    vector<InMemMSet>* resStream = new vector<InMemMSet>();
//    list<InMemUSet>::iterator begin= accumlator.units.begin(), end;
//    
//    vector<InMemMSet>* cresStream = new vector<InMemMSet>();
////    list<CompressedInMemUSet>::iterator cbegin= 
////      c_accumlator.units.begin(), cend, ctmp;
//    //cast the CompressedInMemMSet into an InMemMSet
////    cbegin != c_accumlator.units.end();
//    while(begin != accumlator.units.end())
//    {
//      end= accumlator.GetPeriodEndUnit(begin);
////      cend= c_accumlator.GetPeriodEndUnit(cbegin);
//      if(debugme)
//      {
//        (*begin).Print(cerr);
//        (*end).Print(cerr);
////        (*cbegin).Print(cerr);
////        (*cend).Print(cerr);
//      }
//      if(q == GPattern::atleast)
//      {
//        InMemMSet* mset= new InMemMSet(accumlator, begin, end);
////        InMemMSet* cmset= new InMemMSet();
////        c_accumlator.WriteToInMemMSet(*cmset, cbegin, cend);
//        if(debugme)
//        {
//          MSet tmp1(0), tmp2(0);
//          mset->WriteToMSet(tmp1);
////          cmset->WriteToMSet(tmp2);
//          if(tmp1.Compare(&tmp2) != 0)
//          {
//            tmp1.Print(cerr);
//            tmp2.Print(cerr);
//          }
//        }        
//        resStream->push_back(*mset);
////        cresStream->push_back(*cmset);
//      }
//      else
//      {
////        InMemMSet* cmset= new InMemMSet();
////        ctmp= cend;
////        ++ctmp;
////        c_accumlator.WriteToInMemMSet(*cmset, cbegin, ctmp);
////        if(debugme)
////        {
////          MSet tmp1(0), tmp2(0);
////          list<InMemUSet>::iterator k= end;
////          ++k;
////          accumlator.WriteToMSet(tmp1, begin, k);
////          cmset->WriteToMSet(tmp2);
////          if(tmp1.Compare(&tmp2) != 0)
////          {
////            tmp1.Print(cerr);
////            tmp2.Print(cerr);
////          }
////        } 
//        GPatternHelper::ComputeAddSubSets(accumlator, begin, end, 
//            n, d, resStream);
////        list<InMemUSet>::iterator e= cmset->units.end();
////        --e;
////        GPatternHelper::ComputeAddSubSets(*cmset, cmset->units.begin(), e, 
////            n, d, cresStream);
//      }
//
//      begin= ++end;
////      cbegin = ++cend;
//    }
////    pair<vector<InMemMSet>*, vector<InMemMSet>* >* r= new 
////      pair<vector<InMemMSet>*, vector<InMemMSet>* >(resStream, cresStream);
//    pair<vector<InMemMSet>*, vector<InMemMSet>* >* r= new 
//      pair<vector<InMemMSet>*, vector<InMemMSet>* >(resStream, 0);  
//    local= SetWord(r);
//    return 0;
//  }
//  case REQUEST: { // return the next stream element
//    pair<vector<InMemMSet>*, vector<InMemMSet>* >* resStreams= 
//   static_cast<pair<vector<InMemMSet>*, vector<InMemMSet>* >* >(local.addr); 
//    if ( resStreams->first->size() != 0)
//    {
//      MSet* res= new MSet(0);
//      (*resStreams->first->begin()).WriteToMSet(*res);
//      resStreams->first->erase(resStreams->first->begin());
//      result= SetWord(res);
//      if(debugme)
//      {
//        MSet tmp1(0);
////        (*(resStreams->second->begin())).WriteToMSet(tmp1);
//        if(tmp1.Compare(res) != 0)
//        {
////          tmp1.Print(cerr);
//          res->Print(cerr);
//        }
//        //resStreams->second->erase(resStreams->second->begin());
//      }  
//      return YIELD;
//    }
//    else
//    {
//      // you should always set the result to null
//      // before you return a CANCEL
//      result.addr = 0;
//      return CANCEL;
//    }
//  }
//  case CLOSE: { // free the local storage
//    pair<vector<InMemMSet>*, vector<InMemMSet>* >* resStreams= 
//   static_cast<pair<vector<InMemMSet>*, vector<InMemMSet>* >* >(local.addr); 
//   resStreams->first->clear();
//    delete resStreams;
//    local.addr = 0;
//  }
//
//  return 0;
//  }
//  return 0;
//}

/*
Value map CrossPattern

*/

int CrossPatternVM 
(Word* args, Word& result, int message, Word& local, Supplier s)
{
//  bool debugme= false;
//  typedef std::pair<int, int> EdgeKey;
//  switch( message )
//  {
//  case OPEN: // Iterate over all tuple and compute the stream(mset) result 
//  { 
//    Word t, value;
//    vector<Tuple*> tups;
//    unsigned int row=0, col=0, cnt=0;
//    EdgeKey key;
//    MBool* mbool;
//    map<EdgeKey, int> EdgeKey2int;
//    map<int, EdgeKey> int2EdgeKey;
//    CcInt* idrow, idcol;
//    ArgVectorPointer funargs;
//    InMemMSet accumlator;
//    funargs = qp->Argument(s);
//    qp->Request(args[2].addr, value); // isCommutativeLiftedPredicate
//    bool isCommutativeLiftedPred= 
//      static_cast<CcBool*>(value.addr)->GetBoolval();
//      qp->Open(GPSolver.TheStream);
//      qp->Request(GPSolver.TheStream, t);
//      while (qp->Received(GPSolver.TheStream))
//      {
//        tups.push_back = static_cast<Tuple*>(t.addr);
//        row= tups.size() - 1;
//        for(col= tups.begin(); col < row; ++col)
//        {
//          SetCurRow(tups[row]);
//          SetCurCol(tupe[col]);
//          qp->Request(args[1].addr, value);  // The lifted predicate
//          mbool = static_cast<MBool*>(value.addr);
//          mapkey.first= row ; mapkey.second= col;
//          //matrix[mapkey] = mbool;
//          if(mbool->IsDefined())
//          {
//            int2EdgeKey[cnt]= mapkey;
//            EdgeKey2int[key]= cnt;
//            accumlator.Union(*mbool, cnt);
//            ++cnt;
//          }
//          if(!isCommutativeLiftedPred)
//          {
//            SetCurRow(tups[col]);
//            SetCurCol(tupe[row]);
//            qp->Request(args[1].addr, value); // The lifted predicate
//            mbool = static_cast<MBool*>(value.addr);
//            mapkey.first= col ; mapkey.second= row;
//            //matrix[mapkey] = mbool;
//            if(mbool->IsDefined())
//            {
//              int2EdgeKey[cnt]= mapkey;
//              EdgeKey2int[key]= cnt;
//              accumlator.Union(*mbool, cnt);
//              ++cnt;
//            }          
//          }
//        }
//        qp->Request(GPSolver.TheStream, t);
//      }
//      if(debugme)
//        accumlator.Print(cerr);
//      qp->Close(GPSolver.TheStream);
//      qp->Request(args[3].addr, value); // the minduration
//      Instant di( *static_cast<Instant*>(value.addr));
//      double d= di.ToDouble()* day2min;
//      qp->Request(args[4].addr, value); // min group count
//      int n= static_cast<CcInt*>(value.addr)->GetIntval();
//      string qts= nl->ToString(qp->GetType(args[5].addr)); // quantifier
//      GPattern::quantifier q= (qts=="exactly")? 
//          GPattern::exactly : GPattern::atleast;
//      string subGraphType= nl->ToString(qp->GetType(args[6].addr)); 
//
//      bool changed= true;
//      while(changed && accumlator.GetNoComponents() > 0)
//      {
//        accumlator.RemoveShortElemParts(d);
//        changed= accumlator.RemoveSmallUnits(n);
//      }
//
//      if(debugme)
//        accumlator.Print(cerr);
//      vector<InMemMSet>* resStream = new vector<InMemMSet>();
//      list<InMemUSet>::iterator begin= accumlator.units.begin(), end;
//      while(begin != accumlator.units.end())
//      {
//        end= accumlator.GetPeriodEndUnit(begin);
//        if(debugme)
//        {
//          (*begin).Print(cerr);
//          (*end).Print(cerr);
//        }
//        GPatternHelper::ComputeAddSubGraphs(accumlator, begin, end, 
//            n, d, q, subGraphType, resStream);
//
//        begin= ++end;
//      }
//      local= SetWord(resStream);
//      return 0;
//  }
//  case REQUEST: { // return the next stream element
//  vector<InMemMSet>* resStream= static_cast<vector<InMemMSet>* >(local.addr); 
//    if ( resStream->size() != 0)
//    {
//      result = qp->ResultStorage( s );
//      MSet* res= static_cast<MSet*> (result.addr);
//      res->Clear();
//      (*resStream->begin()).WriteToMSet(*res);
//      resStream->erase(resStream->begin());
//      return YIELD;
//    }
//    else
//    {
//      // you should always set the result to null
//      // before you return a CANCEL
//      result.addr = 0;
//      return CANCEL;
//    }
//  }
//  case CLOSE: { // free the local storage
//  vector<InMemMSet>* resStream= static_cast<vector<InMemMSet>* >(local.addr);
//    resStream->clear();
//    delete resStream;
//    local.addr = 0;
//  }
//
//  return 0;
//  }
  return 0;
}

int EmptyMSetVM 
(Word* args, Word& result, int message, Word& local, Supplier s)
{
  result= qp->ResultStorage(s);
  static_cast<MSet*>(result.addr)->Clear();
  static_cast<MSet*>(result.addr)->SetDefined(true);
  return 0;
}

int MBool2MSetVM 
(Word* args, Word& result, int message, Word& local, Supplier s)
{
  result= qp->ResultStorage(s);
  MSet* res= static_cast<MSet*>(result.addr);
  MBool * arg1= static_cast<MBool*>(args[0].addr);
  CcInt * arg2= static_cast<CcInt*>(args[1].addr);
  res->MBool2MSet(*arg1, arg2->GetIntval());
  return 0;
}

int UnionMSetVM 
(Word* args, Word& result, int message, Word& local, Supplier s)
{
  result= qp->ResultStorage(s);
  MSet* res= static_cast<MSet*>(result.addr);
  MSet* op1= static_cast<MSet*>(args[0].addr);
  MSet* op2= static_cast<MSet*>(args[1].addr);
  op1->LiftedUnion(*op2, *res);
  return 0;
}

int Union2MSetVM 
(Word* args, Word& result, int message, Word& local, Supplier s)
{
  result= qp->ResultStorage(s);
  MSet* res= static_cast<MSet*>(result.addr);
  MSet* op1= static_cast<MSet*>(args[0].addr);
  MSet* op2= static_cast<MSet*>(args[1].addr);
  op1->LiftedUnion2(*op2, *res);
  return 0;
}

int CardinalityMSetVM 
(Word* args, Word& result, int message, Word& local, Supplier s)
{
  result= qp->ResultStorage(s);
  MInt* res= static_cast<MInt*>(result.addr);
  MSet* arg= static_cast<MSet*>(args[0].addr);
  arg->LiftedCount(*res);
  return 0;
}


template <bool Row>
int RowColVM 
(Word* args, Word& result, int message, Word& local, Supplier s)
{
  return 0;
}

template <class Alfa>
int CreateAlfaSetVM 
(Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool debugme= false;
  Word t, value;
  Tuple* tup;
  IntSet* res=0;
  int elem;
  
  int attrIndex= static_cast<CcInt*>(args[2].addr)->GetIntval();
  //string attrType= static_cast<CcString*>(args[3].addr)->GetValue();
  result = qp->ResultStorage( s );
  res= static_cast< IntSet* > (result.addr);
  res->Clear();
  qp->Open(args[0].addr);
  qp->Request(args[0].addr, t);
  while (qp->Received(args[0].addr))
  {
    tup = (Tuple*)t.addr;
    elem= dynamic_cast< CcInt* > (tup->GetAttribute(attrIndex-1))->GetIntval();
    res->Insert(elem);
    qp->Request(args[0].addr, t);
    res->Print(cerr);
  }
  qp->Close(args[0].addr);
  return 0;
}

void 
MPointsSample(Instant& curTime, vector<MPoint*>& sourceMPoints, Points& res,
    bool checkDefined)
{
  bool debugme=false;
  if(debugme)
    curTime.Print(cerr);
  res.Clear();
  Point curPoint(0, 0);
  Intime<Point> pointIntime(curTime, curPoint);
  for(unsigned int i=0; i<sourceMPoints.size(); ++i)
  {
    sourceMPoints[i]->AtInstant(curTime, pointIntime);
//    if(checkDefined)
      assert(pointIntime.IsDefined());
    if(debugme)
      pointIntime.Print(cerr);
    res += pointIntime.value;
  }
}

void 
MPointsSample(Instant& curTime, set<int>& idset, 
    map<int, MPoint*>& sourceMPoints, Points& res, bool checkDefined)
{
  bool debugme=false;
  if(debugme)
    curTime.Print(cerr);
  res.Clear();
  Point curPoint(0, 0);
  Intime<Point> pointIntime(curTime, curPoint);
  set<int>::iterator id= idset.begin();
  for(; id != idset.end(); ++id)
  {
    sourceMPoints[*id]->AtInstant(curTime, pointIntime);
//    if(checkDefined)
//      assert(pointIntime.IsDefined());
    if(debugme)
      pointIntime.Print(cerr);
    if(pointIntime.IsDefined()) res += pointIntime.value;
  }
}


void AppendMRegionPart(set<int>& idset, map<int, MPoint*>& sourceMPoints, 
    Interval<Instant>& unitBoundary,
    Instant& samplingDuration,
    MRegion& res)
{
  bool debugme=false;
    vector<double> weigths(4);
    weigths[0] = 0.7;            // AreaWeight
    weigths[1] = 0.7;            // OverlapWeight
    weigths[2] = 0.5;            // HausdorffWeight
    weigths[3] = 1.0;            // LinearWeight
  
    Instant curTime(unitBoundary.start);
    Interval<Instant> 
      unitInterval(curTime, unitBoundary.end, unitBoundary.lc, false);
    
    Points ps(0);
    Region* reg1=new Region(0), *reg2=new Region(0), *regswap;
    RegionForInterpolation *reginter1, *reginter2, *reginterswap;
    Match *sm;
    mLineRep *lines;
    URegion *resUnit;
    bool firstIteration= true;
    
    MPointsSample(curTime, idset, sourceMPoints, ps, true);
    GrahamScan::convexHull(&ps,reg1);
    reginter1=new RegionInterpol::RegionForInterpolation(reg1);
    curTime+= samplingDuration;
    while(curTime < unitBoundary.end)
    {
      if(!firstIteration)
        unitInterval.lc= true;
      firstIteration= false;
      MPointsSample(curTime, idset, sourceMPoints, ps, true);
      GrahamScan::convexHull(&ps, reg2);
      unitInterval.end= curTime;
      reginter2=new RegionInterpol::RegionForInterpolation(reg2);
      sm=new OptimalMatch(reginter1, reginter2, weigths);
      lines=new mLineRep(sm);    
      resUnit= new URegion(lines->getTriangles(), unitInterval);
      if(debugme)
        unitInterval.Print(cerr);
      res.AddURegion(*resUnit);
/*
Copying the right part of this URegion to the left part of the next URegion

*/
      
      unitInterval.start= unitInterval.end;
      regswap= reg1;
      reg1= reg2;
      reg2= regswap;
      reginterswap= reginter1;
      reginter1= reginter2;
/*
Garbage collection

*/      
      delete resUnit;
      delete lines;
      delete reginterswap;
      curTime+= samplingDuration;
    }
/*
Adding the last instant in the unit

*/  
    Instant endI(unitBoundary.end);
    if(!unitBoundary.rc)
    {
      Instant milli(0, 1, durationtype);
      endI -= milli;
    } 
    MPointsSample(endI, idset, sourceMPoints, ps, false);
    unitInterval.rc= unitBoundary.rc;
    GrahamScan::convexHull(&ps, reg2);
    unitInterval.end= unitBoundary.end;
    reginter2=new RegionInterpol::RegionForInterpolation(reg2);
    sm=new OptimalMatch(reginter1, reginter2, weigths);
    lines=new mLineRep(sm);    
    resUnit= new URegion(lines->getTriangles(), unitInterval);
    if(debugme)
      unitInterval.Print(cerr);
    res.AddURegion(*resUnit);
    delete resUnit;
    delete lines;
    delete reg1;
    delete reg2;
    delete reginter1;
    delete reginter2;
}

/*
Value map MSet2MRegion

*/

int 
MSet2MRegionVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool debugme=false;
  result = qp->ResultStorage(s);
  MRegion* res = static_cast<MRegion*>( result.addr);
  res->Clear();
  Word Value;
  Supplier arg0= qp->GetSon(s,0);
  Supplier arg1= qp->GetSon(s,1);
  Supplier arg2= qp->GetSon(s,2);
  
  qp->Request(arg1, Value);
  MSet* mset= static_cast<MSet*>(Value.addr);
  if(debugme)
    mset->Print(cerr);

  
  if(!mset->IsDefined() || mset->GetNoComponents()==0)
  {
    res->SetDefined(false);
    return 0;
  }
  
  qp->Request(arg2, Value);
  Instant* d= static_cast<Instant*>(Value.addr);
  USetRef usetref;
  USet uset(true);
  set<int> idset;
  set<int>::iterator setIt;
  for(int i=0; i<mset->GetNoComponents(); ++i)
  {
    mset->Get(i, usetref);
    usetref.GetUnit(mset->data, uset);
    if(uset.constValue.Count()!= 0)
    {
      for(int k=0; k< uset.constValue.Count(); ++k)
        idset.insert(uset.constValue[k]);
    }
  }
  
  map<int, MPoint*> mpoints;
  vector<Tuple*> tuplesToDelete(0);
  qp->Open(arg0);
  qp->Request(arg0, Value);
  while(qp->Received(arg0) &&  (mpoints.size() < idset.size()))
  {
    Tuple* tuple= static_cast<Tuple*>(Value.addr);
    int id= dynamic_cast<CcInt*>(tuple->GetAttribute(0))->GetIntval();
    setIt= idset.find(id);
    if(setIt != idset.end())
    {
      MPoint* elem=dynamic_cast<MPoint*>(
          dynamic_cast<MPoint*>(tuple->GetAttribute(1)));
      mpoints[id]= elem;
      tuplesToDelete.push_back(tuple);
    }
    else
      tuple->DeleteIfAllowed();
      
    qp->Request(arg0, Value);
  }
  qp->Close(arg0);  
  if(mpoints.size() != idset.size())
  {
    res->SetDefined(false);
    cerr<<"mset2mregion: not all ids in the mset are "
    "found in the mpoint stream\n";
    for(unsigned int k=0; k<tuplesToDelete.size(); ++k)
      tuplesToDelete[k]->DeleteIfAllowed();
    return 0;
  }
  
  for(int i=0; i<mset->GetNoComponents(); ++i)
  {
    mset->Get(i, usetref);
    usetref.GetUnit(mset->data, uset);
    if(uset.constValue.Count()!= 0)
    {
      idset.clear();
      for(int k=0; k< uset.constValue.Count(); ++k)
        idset.insert(uset.constValue[k]);
      AppendMRegionPart(idset, mpoints, usetref.timeInterval, *d , *res);
    }
  }
  for(unsigned int k=0; k<tuplesToDelete.size(); ++k)
    tuplesToDelete[k]->DeleteIfAllowed();
  return 0;
}

//int 
//MSet2MRegionVM(Word* args, Word& result, int message, Word& local, Supplier s)
//{
//  bool debugme=false;
//  result = qp->ResultStorage(s);
//  MRegion* res = static_cast<MRegion*>( result.addr);
//  res->Clear();
//  Word Value;
//  Supplier arg0= qp->GetSon(s,0);
//  Supplier arg1= qp->GetSon(s,1);
//  Supplier arg2= qp->GetSon(s,2);
//  
//  qp->Request(arg1, Value);
//  MSet* mset= static_cast<MSet*>(Value.addr);
//  if(debugme)
//    mset->Print(cerr);
//
//  
//  if(!mset->IsDefined() || mset->GetNoComponents()==0)
//  {
//    res->SetDefined(false);
//    return 0;
//  }
//  
//  qp->Request(arg2, Value);
//  Instant* d= static_cast<Instant*>(Value.addr);
//  USetRef usetref;
//  USet uset(true);
//  set<int> idset;
//  set<int>::iterator setIt;
//  for(int i=0; i<mset->GetNoComponents(); ++i)
//  {
//    mset->Get(i, usetref);
//    usetref.GetUnit(mset->data, uset);
//    if(uset.constValue.Count()!= 0)
//    {
//      idset.clear();
//      for(int k=0; k< uset.constValue.Count(); ++k)
//        idset.insert(uset.constValue[k]);
//      vector<MPoint*> mpoints(0);
//      qp->Open(arg0);
//      qp->Request(arg0, Value);
//      while(qp->Received(arg0))
//      {
//        Tuple* tuple= static_cast<Tuple*>(Value.addr);
//        int id= dynamic_cast<CcInt*>(tuple->GetAttribute(0))->GetIntval();
//        setIt= idset.find(id);
//        if(setIt != idset.end())
//        {
//          MPoint* elem=dynamic_cast<MPoint*>( 
//            dynamic_cast<MPoint*>(tuple->GetAttribute(1))->Clone());
//          mpoints.push_back(elem);
//        }
//        tuple->DeleteIfAllowed();
//        qp->Request(arg0, Value);
//      }
//      if(mpoints.size() != idset.size())
//      {
//        res->SetDefined(false);
//        cerr<<"mset2mregion: not all ids in the mset are "
//        "found in the mpoint stream\n";
//        return 0;
//      }
//      qp->Close(arg0);
//      AppendMRegionPart(mpoints, usetref.timeInterval, *d , *res);
//      for(unsigned int k=0; k<mpoints.size(); ++k)
//        delete mpoints[k];
//    }
//  }
//  return 0;
//}

/*
Value map MSet2MPoints

*/

int 
MSet2MPointsVM(Word* args, Word& result, int message, Word& local, Supplier s)
{

  bool debugme=false;

  switch( message )
  {
  case OPEN: { // initialize the local storag
    map<int, Periods*>* msetElems= new map<int, Periods*>();      
    Word Value;
    Supplier arg1= qp->GetSon(s,1);
    qp->Request(arg1, Value);
    MSet* mset= static_cast<MSet*>(Value.addr);
    int nocomponents= mset->GetNoComponents();
    if(!mset->IsDefined() || nocomponents==0)
    {
      local=SetWord(msetElems);
      return 0;
    }
    Supplier arg2= qp->GetSon(s,2);
    qp->Request(arg2, Value);    
    bool restrictMPoints= static_cast<CcBool*>(Value.addr)->GetBoolval();  
    
    int nounitcomponents=0, idFromMSet=0;
    map<int, Periods*>::iterator msetElemsIt;
    
    USetRef usetref;
    USet uset(true);
    for(int i=0; i< nocomponents; ++i)
    {
      mset->Get(i, usetref);
      usetref.GetUnit(mset->data, uset);
      nounitcomponents= uset.constValue.Count();
      for(int e= 0; e<nounitcomponents; ++e)
      {
        idFromMSet= uset.constValue[e];
        msetElemsIt= msetElems->find(idFromMSet);
        if(msetElemsIt == msetElems->end())
        {
          Periods* periods= new Periods(1);
          periods->Add(uset.timeInterval);
          (*msetElems)[idFromMSet]= periods;
        }
        else
        {
          if(!restrictMPoints)
            continue;        
          (*msetElemsIt).second->MergeAdd(uset.timeInterval);
        }
      }
    }    
    local=SetWord(msetElems);
    qp->Open( args[0].addr );
    return 0;
  }
  case REQUEST: { // return the next stream element
    map<int, Periods*>* msetElems= static_cast<map<int, Periods*>*>(local.addr);
    if(msetElems->size() == 0)
    {
      result.addr = 0;
      return CANCEL;
    }
    //result = qp->ResultStorage(s);
    Word Value;
    map<int, Periods*>::iterator msetElemsIt; 
    Supplier arg2= qp->GetSon(s,2);
    qp->Request(arg2, Value);  
    bool restrictMPoints= static_cast<CcBool*>(Value.addr)->GetBoolval();  
    
    qp->Request(args[0].addr, Value);
    while(qp->Received(args[0].addr))
    {
      Tuple* tuple= static_cast<Tuple*>(Value.addr);
      int idFromTuple= 
        dynamic_cast<CcInt*>(tuple->GetAttribute(0))->GetIntval();
      msetElemsIt= msetElems->find(idFromTuple);
      if(msetElemsIt != msetElems->end())
      {
        MPoint* res= new MPoint(0);
        if(restrictMPoints)
        {
          MPoint* tmp = dynamic_cast<MPoint*>(tuple->GetAttribute(1));
          tmp->AtPeriods(*((*msetElemsIt).second), *res);
        }
        else
        {
          MPoint* tmp = dynamic_cast<MPoint*>(tuple->GetAttribute(1));
          res->CopyFrom(tmp);
        }
        msetElems->erase(msetElemsIt);
        tuple->DeleteIfAllowed();
        result= SetWord(res);
        return YIELD;
      }
      tuple->DeleteIfAllowed();
      qp->Request(args[0].addr, Value);
    }
    cerr<<"operator mset2mpoints: some mset elements are not found in "
        "the stream";
    return FAILURE;
  }
  case CLOSE: { // free the local storage

    map<int, Periods*>* msetElems= static_cast<map<int, Periods*>*>(local.addr);
    map<int, Periods*>::iterator msetElemsIt= msetElems->begin();
    for(; msetElemsIt != msetElems->end(); ++msetElemsIt)
      delete (*msetElemsIt).second;
    delete msetElems;
    result.addr=0;  
    return 0;
  }
  return 0;
  }
  return 0;
}

int 
ConvexHullVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool debugme= false;
  Word Value;
  MPoint* mp=0;
  Instant* instant=0;
  vector<MPoint*> stream(0);
  result = qp->ResultStorage(s);
  
  qp->Open(args[0].addr);
  qp->Request(args[0].addr,Value);
  while (qp->Received(args[0].addr))
  {
    mp = static_cast<MPoint*>(Value.addr);
    stream.push_back(mp);
    qp->Request(args[0].addr, Value);
  }
  Supplier arg1= qp->GetSon(s,1);
  qp->Request(arg1, Value);
  instant= static_cast<Instant*>(Value.addr);
  
  Points ps(0);
  MPointsSample(*instant, stream, ps, true);
  Region* reg=static_cast<Region*>(result.addr);
  reg->Clear();
  GrahamScan::convexHull(&ps, reg);
  qp->Close(args[0].addr);
  return 0;
}

//template <bool leftbound> int StartEndVM
//(Word* args, Word& result, int message, Word& local, Supplier s)
//{
//  bool debugme= false;
//  Interval<Instant> interval;
//  Word input;
//  string lbl;
//
//  //qp->Request(args[0].addr, input);
//  lbl= ((CcString*)args[0].addr)->GetValue();
//  if(debugme)
//  {
//    cout<<endl<<"Accessing the value of label "<<lbl;
//    cout.flush();
//  }
//
//  Instant res(0,0,instanttype);
//  bool found=false;
//  if(leftbound)
//    found=csp.GetStart(lbl, res);
//  else
//    found=csp.GetEnd(lbl, res);
//
//  if(debugme)
//  {
//    cout<<endl<<"Value is "; if(found) cout<<"found"; else cout<<"Not found";
//    res.Print(cout);
//    cout.flush();
//  }
//
//  result = qp->ResultStorage( s );
//  ((Instant*)result.addr)->CopyFrom(&res);
//  return 0;
//}

/*
Operator properties

*/

const string RowColSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>tuple(x) X namedFunlist X constraintList X bool -> bool</text--->"
  "<text>_ reportpattern[ namedFunlist;  constraintList; bool ]</text--->"
  "<text>The operator implements the Extended Spatiotemporal Pattern "
  "Predicate.</text--->"
  "<text>query Trains feed filter[. reportpattern[a: .Trip inside msnow, "
  "b: distance(.Trip, mehringdamm)<10.0, c: speed(.Trip)>8.0 ;  "
  "stconstraint(\"a\",\"b\",vec(\"aabb\")),  "
  "stconstraint(\"b\",\"c\",vec(\"bbaa\"));  (end(\"b\") - start(\"a\")) < "
  "[const duration value (1 0)] ]] count  </text--->"
  ") )";

const string CrossPatternSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>tuple(x) X namedFunlist X constraintList X bool -> bool</text--->"
  "<text>_ reportpattern[ namedFunlist;  constraintList; bool ]</text--->"
  "<text>The operator implements the Extended Spatiotemporal Pattern "
  "Predicate.</text--->"
  "<text>query Trains feed filter[. reportpattern[a: .Trip inside msnow, "
  "b: distance(.Trip, mehringdamm)<10.0, c: speed(.Trip)>8.0 ;  "
  "stconstraint(\"a\",\"b\",vec(\"aabb\")),  "
  "stconstraint(\"b\",\"c\",vec(\"bbaa\"));  (end(\"b\") - start(\"a\")) < "
  "[const duration value (1 0)] ]] count  </text--->"
  ") )";

const string GPatternSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>tuple(x) X namedFunlist X constraintList X bool -> bool</text--->"
  "<text>_ reportpattern[ namedFunlist;  constraintList; bool ]</text--->"
  "<text>The operator implements the Extended Spatiotemporal Pattern "
  "Predicate.</text--->"
  "<text>query Trains feed filter[. reportpattern[a: .Trip inside msnow, "
  "b: distance(.Trip, mehringdamm)<10.0, c: speed(.Trip)>8.0 ;  "
  "stconstraint(\"a\",\"b\",vec(\"aabb\")),  "
  "stconstraint(\"b\",\"c\",vec(\"bbaa\"));  (end(\"b\") - start(\"a\")) < "
  "[const duration value (1 0)] ]] count  </text--->"
  ") )";

const string ReportPatternSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>tuple(x) X namedFunlist X constraintList X bool -> bool</text--->"
  "<text>_ reportpattern[ namedFunlist;  constraintList; bool ]</text--->"
  "<text>The operator implements the Extended Spatiotemporal Pattern "
  "Predicate.</text--->"
  "<text>query Trains feed filter[. reportpattern[a: .Trip inside msnow, "
  "b: distance(.Trip, mehringdamm)<10.0, c: speed(.Trip)>8.0 ;  "
  "stconstraint(\"a\",\"b\",vec(\"aabb\")),  "
  "stconstraint(\"b\",\"c\",vec(\"bbaa\"));  (end(\"b\") - start(\"a\")) < "
  "[const duration value (1 0)] ]] count  </text--->"
  ") )";

const string TConstraintSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>string X string X stvector -> bool</text--->"
  "<text>_ stconstraint( string, string, vec(_))</text--->"
  "<text>The operator is used only within the stpattern and reportpattern "
  "operators. It is used to express a spatiotemporal constraint. The operator "
  "doesn't have a value mapping function because it is evaluated within the "
  "stpattern. It should never be called elsewhere."
  "</text--->"
  "<text>query Trains feed filter[. stpattern[a: .Trip inside msnow,"
  "b: distance(.Trip, mehringdamm)<10.0, c: speed(.Trip)>8.0 ;"
  "stconstraint(\"a\",\"b\",vec(\"aabb\")), "
  "stconstraint(\"b\",\"c\",vec(\"bbaa\"));"
  "(end(\"b\") - start(\"a\")) < "
  "[const duration value (1 0)] ]] count </text--->"
  ") )";

const string CreateIntSetSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>stream(tuple(.)) X symbol -> elemset</text--->"
  "<text>_ intstream2set(_)</text--->"
  "<text>The operator collects the values of a stream int attribute into "
  "a set.</text--->"
  "<text>query ten feed intstream2set[no] consume</text--->"
  ") )";

const string EmptyMSetSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text> -> mset</text--->"
  "<text>_ emptymset()</text--->"
  "<text>The operator creates an empty mset (i.e. an mset with zero units)."
  "</text--->"
  "<text>query emptymset()"
  "</text--->"
  ") )";

const string MBool2MSetSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text> mbool x int -> mset</text--->"
  "<text>_ mbool2mset(_)</text--->"
  "<text>The operator creates an mset representation for the given mbool. "
  "For every true unit in the mbool argument, a corresponding uset is added "
  "to the result. The set within the uset will have one element, the int "
  "argument. The operator is used within the GPattern operator."
  "</text--->"
  "<text>query speed(train7)>15 mbool2mset(7) nocomponents"
  "</text--->"
  ") )";

const string UnionMSetSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text> mset x mset -> mset</text--->"
  "<text>_ union _</text--->"
  "<text>The operator is a lifted union for msets."
  "</text--->"
  "<text>query speed(train7)>15 mbool2mset(7) union emptymset() nocomponents"
  "</text--->"
  ") )";

const string Union2MSetSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text> mset x mset -> mset</text--->"
  "<text>_ union2 _</text--->"
  "<text>The operator is a lifted union for msets. Unlike union, this operator "
  "treats undefined intervals as empty msets."
  "</text--->"
  "<text>query speed(train7)>15 mbool2mset(7) union emptymset() nocomponents"
  "</text--->"
  ") )";

const string CardinalityMSetSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text> mset -> mint</text--->"
  "<text>_ cardinality</text--->"
  "<text>The operator is the lifted count/cardinality for the msets."
  "</text--->"
  "<text>query speed(train7)>15 mbool2mset(7) cardinality max"
  "</text--->"
  ") )";

const string MSet2MPointsSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text> mset -> mint</text--->"
  "<text>_ cardinality</text--->"
  "<text>The operator is the lifted count/cardinality for the msets."
  "</text--->"
  "<text>query speed(train7)>15 mbool2mset(7) cardinality max"
  "</text--->"
  ") )";

const string ConvexHullSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text> mset -> mint</text--->"
  "<text>_ cardinality</text--->"
  "<text>The operator is the lifted count/cardinality for the msets."
  "</text--->"
  "<text>query speed(train7)>15 mbool2mset(7) cardinality max"
  "</text--->"
  ") )";


const string MSet2MRegionSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text> stream(tuple(int mpoint)) x mset x duration -> movingregion"
  "</text--->"
  "<text>Trains feed addcounter[Cnt, 1] project[Cnt, Trip] Flocks feed "
  "mflocks2mregions[create_duration(0, 10000)]</text--->"
  "<text>Creates mving region representation for the mflocks. The resulting "
  "mregions are the interpolation of the convex hull regions taken at time "
  "intervals of duration at most.</text--->"
  "<text>query Trains feed addcounter[Cnt, 1] project[Cnt, Trip] Flocks feed "
  "mflocks2mregions[create_duration(0, 10000)] consume</text--->"
  ") )";


int GPatternSelect(ListExpr args){
   ListExpr arg = nl->First(args);
   if(listutils::isTupleDescription(arg))
     return 0;
   else if (listutils::isTupleStream(arg))
     return 1;
   return -1; // should never occur
}

ValueMapping GPatternVMmap[] = { GPatternNestedVM,
                                 GPatternVM };
//const string StartEndSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
//  "\"Example\" ) "
//  "( <text>string -> instant</text--->"
//  "<text>start( _ )/ end(_)</text--->"
//  "<text>Are used only within an extended spatiotemporal pattern predicate. "
// "They return the start and end time instants for a predicate in the SA list."
//  "These operators should never be called unless within the reportpattern "
//  "operator."
//  "</text--->"
//  "<text> query Trains feed filter[. reportpattern[a: .Trip inside msnow, "
//  "b:distance(.Trip, mehringdamm)<10.0 ; "
//  "stconstraint(\"a\",\"b\", vec(\"aabb\", \"aab.b\")) ; "
//  "(start(\"b\")-end(\"a\"))< [const duration vecalue(0 1200000)] ] ] "
//  "count </text--->"
//  ") )";

Operator row (
    "row",    //name
    RowColSpec,     //specification
    RowColVM<true>,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    RowColTM        //type mapping
);

Operator col (
    "col",    //name
    RowColSpec,     //specification
    RowColVM<false>,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    RowColTM        //type mapping
);

Operator crosspattern (
    "crosspattern",    //name
    CrossPatternSpec,     //specification
    CrossPatternVM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    CrossPatternTM        //type mapping
);

Operator gpattern (
    "gpattern",    //name
    GPatternSpec,     //specification
    2,
    GPatternVMmap,       //value mapping
    GPatternSelect,
    GPatternTM        //type mapping
);

Operator reportpattern (
    "reportpattern",    //name
    ReportPatternSpec,     //specification
    ReportPatternVM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    ReportPatternTM        //type mapping
);

Operator tconstraint (
    "tconstraint",    //name
    TConstraintSpec,     //specification
    TConstraintVM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    TConstraintTM        //type mapping
);

Operator emptymset (
    "emptymset",    //name
    EmptyMSetSpec,     //specification
    EmptyMSetVM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    EmptyMSetTM        //type mapping
);

Operator mbool2mset (
    "mbool2mset",    //name
    MBool2MSetSpec,     //specification
    MBool2MSetVM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    MBool2MSetTM        //type mapping
);

Operator unionmset (
    "union",    //name
    UnionMSetSpec,     //specification
    UnionMSetVM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    UnionMSetTM        //type mapping
);

Operator union2mset (
    "union2",    //name
    Union2MSetSpec,     //specification
    Union2MSetVM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    Union2MSetTM        //type mapping
);

Operator cardinalitymset (
    "cardinality",    //name
    CardinalityMSetSpec,     //specification
    CardinalityMSetVM,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    CardinalityMSetTM        //type mapping
);

Operator intstream2set (
    "intstream2set",    //name
    CreateIntSetSpec,     //specification
    CreateAlfaSetVM<CcInt>,       //value mapping
    Operator::SimpleSelect, //trivial selection function
    CreateAlfaSetTM<0>        //type mapping
);


Operator mset2mregion (
    "mset2mregion",               // name
    MSet2MRegionSpec,             // specification
    MSet2MRegionVM,                 // value mapping
    Operator::SimpleSelect, // trivial selection function
    MSet2MRegionTM          // type mapping
);

Operator mset2mpoints (
    "mset2mpoints",               // name
    MSet2MPointsSpec,             // specification
    MSet2MPointsVM,                 // value mapping
    Operator::SimpleSelect, // trivial selection function
    MSet2MPointsTM          // type mapping
);

Operator convexhull (
    "convexhull2",               // name
    ConvexHullSpec,             // specification
    ConvexHullVM,                 // value mapping
    Operator::SimpleSelect, // trivial selection function
    ConvexHullTM          // type mapping
);

//Operator start (
//    "start",    //name
//    StartEndSpec,     //specification
//    StartEndVM<true>,       //value mapping
//    Operator::SimpleSelect, //trivial selection function
//    StartEndTM        //type mapping
//);
//
//Operator end (
//    "end",    //name
//    StartEndSpec,     //specification
//    StartEndVM<false>,       //value mapping
//    Operator::SimpleSelect, //trivial selection function
//    StartEndTM        //type mapping
//);

class GPatternAlgebra : public Algebra
{
public:
  GPatternAlgebra() : Algebra()
  {

    AddTypeConstructor( &mset::intSetTC );
    AddTypeConstructor( &mset::usetTC );
    AddTypeConstructor( &mset::msetTC );
    
    mset::intSetTC.AssociateKind( "DATA" );
    mset::usetTC.AssociateKind("TEMPORAL" );
    mset::usetTC.AssociateKind( "DATA" );
    mset::msetTC.AssociateKind("TEMPORAL" );
    mset::msetTC.AssociateKind( "DATA" );
/*
The spattern and reportpattern operators are registered as lazy variables.

*/
    reportpattern.SetRequestsArguments();
    gpattern.SetRequestsArguments();
    crosspattern.SetRequestsArguments();
    
    AddOperator(&GPattern::tconstraint);
    AddOperator(&GPattern::reportpattern);
    AddOperator(&GPattern::emptymset);
    AddOperator(&GPattern::gpattern);
    AddOperator(&GPattern::crosspattern);
    AddOperator(&GPattern::row);
    AddOperator(&GPattern::col);
    AddOperator(&intstream2set);
    AddOperator(&emptymset);
    AddOperator(&mbool2mset);
    AddOperator(&unionmset);
    AddOperator(&union2mset);
    AddOperator(&cardinalitymset);
    AddOperator(&mset2mregion);
    AddOperator(&mset2mpoints);
    AddOperator(&convexhull);
//    AddOperator(&STP::start);
//    AddOperator(&STP::end);

  }
  ~GPatternAlgebra() {};
};

};

/*
5 Initialization

*/



extern "C"
Algebra*
InitializeGPatternAlgebra( NestedList* nlRef,
    QueryProcessor* qpRef )
    {
  // The C++ scope-operator :: must be used to qualify the full name
  return new GPattern::GPatternAlgebra;
    }
