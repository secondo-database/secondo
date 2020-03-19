/*
----
This file is part of SECONDO.

Copyright (C) 2012, University in Hagen
Faculty of Mathematic and Computer Science,
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

*/

#include "MPointer.h"
#include "Mem.h"
#include "../Relation-C++/RelationAlgebra.h"
#include "../FText/FTextAlgebra.h"
#include "Stream.h"

#include "../MainMemory2/MainMemoryExt.h"
#include "MPointer.h"
#include "Mem.h"
#include "MemoryObject.h"
#include "MemCatalog.h"

#include <string>
#include <Utils.h>
using namespace std;

//------------------------------------------------------------------
namespace pgraph
{

   int debugLevel = 0;   
       // The default loglevel - the first step in the typemapping functions
       // will be quiet until the point where the log option is processed.
       // this makes it perhaps necesssary to adjust the default value here.

   void setDebugLevel(int level) { debugLevel=level; }

//------------------------------------------------------------------
   string getDBname()
   {
      SecondoSystem *sys = SecondoSystem::GetInstance();
      return sys->GetDatabaseName();
   }

   //--------------------------------------------------------------------------
   bool DefSave(OutObject outfunc, SmiRecord &valueRecord,
             size_t &offset, const ListExpr typeInfo,
             Word &value)
   {
      ListExpr valueList;
      string valueString;
      int valueLength;

      valueList = outfunc(nl->First(typeInfo), value);
      valueList = nl->OneElemList(valueList);
      nl->WriteToString(valueString, valueList);
      valueLength = valueString.length();
      valueRecord.Write(&valueLength, sizeof(valueLength), offset);
      offset += sizeof(valueLength);
      valueRecord.Write(valueString.data(), valueString.length(), offset);
      offset += valueString.length();

      nl->Destroy(valueList);
      return (true);
   }

   //--------------------------------------------------------------------------
   bool DefOpen(InObject infunc, SmiRecord &valueRecord,
             size_t &offset, const ListExpr typeInfo,
             Word &value)
   {
      ListExpr valueList = 0;
      string valueString;
      int valueLength;

      ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERRORS"));
      bool correct;
      valueRecord.Read(&valueLength, sizeof(valueLength), offset);
      offset += sizeof(valueLength);
      char *buffer = new char[valueLength];
      valueRecord.Read(buffer, valueLength, offset);
      offset += valueLength;
      valueString.assign(buffer, valueLength);
      delete[] buffer;
      nl->ReadFromString(valueString, valueList);
      value = infunc(nl->First(typeInfo),
          nl->First(valueList),
          1, errorInfo, correct);
      if (errorInfo != 0)
      {
         nl->Destroy(errorInfo);
      }
      nl->Destroy(valueList);
      return (true);
   }

//----------------------------------------------------------------------------
void ReplaceStringInPlace(std::string& subject, const std::string& search,
                          const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
}

//----------------------------------------------------------------------------
ListExpr GetTupleDefFromObject(string name)
{
    return nl->Empty();
}

//----------------------------------------------------------------------------
// distance to the next non-atom entry 
// -1: not sub list found
int GetNextListIndex(ListExpr list)
{
    // find node (list type)
    int max=nl->ListLength(list);
    int i=1;
    while (i<=max)
    {
      ListExpr item=nl->Nth(i,list);
      if (!nl->IsAtom(item)) 
         return i-1;
      i++;
    }
    return -1;
}

//----------------------------------------------------------------------------
bool FirstUpper(const string& word) 
{ 
      return word.size() && std::isupper(word[0]); 
}

//-----------------------------------------------------------------------------
ListExpr ParseString(string s)
{
   ListExpr list = 0;
   nl->ReadFromString(s,list);
   return list;
}

//-----------------------------------------------------------------------------
string GetArgValue(ListExpr args, int pos)
{
   NList list(args);
   return list.elem(pos).second().str();
  //return nl->Text2String(nl->Second(nl->Nth(pos, args))); 
  // above failed as it will not respect different string types
}

//-----------------------------------------------------------------------------
ListExpr GetArg_FTEXT_AS_LIST(Address a)
{
   FText *text = static_cast<FText*>( a);
   ListExpr list = 0;
   nl->ReadFromString(text->GetValue(), list);
   return list;
}

//-----------------------------------------------------------------------------
string GetArg_FTEXT_AS_STRING(Address a)
{
   if (a==NULL) return "";
   FText *text = static_cast<FText*>( a);
   if (text==NULL) return "";
   return text->GetValue();
}

//-----------------------------------------------------------------------------
string GetArg_STRING(Address a)
{
   if (a==NULL) return "";
   CcString *text = static_cast<CcString*>( a);
   if (text==NULL) return "";
   return text->GetValue();
}

//-----------------------------------------------------------------------------
int  GetArgCount(ListExpr args)
{
   return nl->ListLength(args);
}

//-----------------------------------------------------------------------------
void CheckArgCount(ListExpr args, int min, int max)
{
   if ( nl->ListLength(args)<min || nl->ListLength(args)>max )  
   {
      if (min==max)
         throw PGraphException("Expecting "+ to_string(min)+" arguments ");
      else
         throw PGraphException("Expecting "+ to_string(min)+"-"+
                 to_string(max)+" arguments ");
   }
}

//-----------------------------------------------------------------------------
void CheckArgType(ListExpr args, int pos, string typename_)
{
   if ( nl->ToString(nl->First(nl->Nth(pos,args))) != typename_ ) 
      throw PGraphException("Argument #"+to_string(pos)+" has to be of type "+
          typename_);
}

//-----------------------------------------------------------------------------
void CheckArgTypeIsTupleStream(ListExpr args, int pos)
{
   if (!Stream<Tuple>::checkType( nl->First(nl->Nth(pos,args)) )) 
      throw PGraphException("Argument #"+to_string(pos)+
          " has to be a tuple stream");
}

//-----------------------------------------------------------------------------
int QueryRelationCount(string relname) 
{
  string querystring = "(count (feed " + relname + "))";
  
  Word resultWord;
  int i=-1;
  if ( QueryProcessor::ExecuteQuery(querystring, resultWord) )
   {
      
      i=((CcInt*)resultWord.addr)->GetIntval();
      cout << " count: "<< i<< "\n";
      ((CcInt*)resultWord.addr)->DeleteIfAllowed();
    }
    else
       throw PGraphException("Error in executing  query: "+querystring);


  return i;
}


//-----------------------------------------------------------------------------
//
bool queryValueDouble(string cmd, double &val){
   Word res;
   bool success = QueryProcessor::ExecuteQuery(cmd,res);
   if(!success){
     return false;
   }
   CcReal* s = (CcReal*) res.addr;
   if(!s->IsDefined()){
      s->DeleteIfAllowed();
      return false;
   }
   val = s->GetValue();
   s->DeleteIfAllowed();
   return true;
}

//-----------------------------------------------------------------------------
double round(double x, int n) {
   int d = 0;
   if((x * pow(10, n + 1)) - (floor(x * pow(10, n))) > 4) d = 1;
   x = (floor(x * pow(10, n)) + d) / pow(10, n);
   return x;
   }

//-----------------------------------------------------------------------------
mm2algebra::MemoryRelObject* QueryMemRelation(string relname) 
{
   mm2algebra::MemCatalog *memCatalog = 
       mm2algebra::MemCatalog::getInstance();

   if (memCatalog->isObject(relname)) {
      mm2algebra::MemoryObject *mo=memCatalog->getMMObject(relname);
      //string s=mo->getObjectTypeExpr();
      return (mm2algebra::MemoryRelObject*) mo;
   }
   
   throw PGraphException("Error reading relation : "+relname);
}

//-----------------------------------------------------------------------------
Relation* OpenRelation(string relname) 
{
   ListExpr s;
   return OpenRelation(relname, s);
}

//-----------------------------------------------------------------------------
Relation* OpenRelation(string relname, ListExpr &reltype) 
{
   SecondoCatalog* catalog = SecondoSystem::GetCatalog();
   reltype=catalog->GetObjectTypeExpr(relname);
  
   if (catalog->IsObjectName(relname)) {
      bool defined;
      Word word;
      catalog->GetObject(relname, word, defined);
      return (Relation*) word.addr;
   }
   throw PGraphException("Error reading relation : "+relname);
}

//-----------------------------------------------------------------------------
Relation*  ExecuteQuery(string query) 
{
  
   Word resultWord;
   
   Word queryResult;
   std::string typeString = "";
   std::string errorString = "";
   bool correct;
   bool evaluable;
   bool defined;
   bool isFunction;
   ListExpr q;
   nl->ReadFromString(query,q);
   if (qp->ExecuteQuery(q, queryResult, 
                    typeString, errorString, correct, 
                    evaluable, defined, isFunction))
   {
      //nl->ReadFromString(typeString, reltype);
      return ( Relation* ) queryResult.addr;    
   }
   throw PGraphException("Error executing query : "+query);
}

//-----------------------------------------------------------------------------
//TODO move to utils
void DoLet(string objName, string  commandText)
// see QueryProcessor/SecondointerfaceTTY.cpp
{
   ListExpr valueExpr = nl->TheEmptyList();
  if ( !nl->ReadFromString( commandText, valueExpr ) )
      throw PGraphException("[DoLet] Parsing commandtext");


   QueryProcessor& qp = *SecondoSystem::GetQueryProcessor();
   SecondoCatalog& ctlg = *SecondoSystem::GetCatalog();
  
   NestedList& nl = *SecondoSystem::GetNestedList();
   bool correct = false;
   bool evaluable = false;
   bool defined = false;
   bool isFunction = false;
   ListExpr resultType = nl.TheEmptyList();
   OpTree tree = 0;
   Word result = SetWord( Address(0) );

   try {
   qp.Construct( valueExpr, correct, evaluable, defined,
                      isFunction, tree, resultType );

        if ( evaluable || isFunction )
        {
          //cout << "DoLet eval or function\n";
          string typeName = "";
          ctlg.CreateObject(objName, typeName, resultType, 0);
        }
        if ( evaluable )
        {
          qp.EvalP( tree, result, 1 );

          if( IsRootObject( tree ) && !IsConstantObject( tree ) )
          {
            //cout << "DoLet root\n";
            ctlg.CloneObject( objName, result );
            qp.Destroy( tree, true );
          }
          else
          {
            //cout << "DoLet nonroot\n";
            ctlg.UpdateObject( objName, result );
            qp.Destroy( tree, false );
          }
        }
        else if ( isFunction ) // abstraction or function object
        {
          //cout << "DoLet isfunction\n";
          if ( nl.IsAtom( valueExpr ) )  // function object
          {
             ListExpr functionList = ctlg.GetObjectValue(
                                            nl.SymbolValue( valueExpr ) );
             ctlg.UpdateObject( objName, SetWord( functionList ) );
          }
          else
          {
             ctlg.UpdateObject( objName, SetWord( valueExpr ) );
          }
          qp.Destroy( tree, true );
        }
        //cout << "DoLet x\n";
      } catch (SI_Error err) {
        // errorCode = err;
        throw PGraphException("SI_Error DoLet");
         qp.Destroy( tree, true );
      }  
}


} // namespace pgraph
