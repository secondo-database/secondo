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

#include "../MainMemory2/MPointer.h"
#include "../MainMemory2/Mem.h"
#include "../Relation-C++/RelationAlgebra.h"
#include "../FText/FTextAlgebra.h"

#include <string>
#include <Utils.h>
using namespace std;

//------------------------------------------------------------------
namespace pgraph
{

   int debugLevel = 0;

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
bool firstUpper(const string& word) 
{ 
      return word.size() && std::isupper(word[0]); 
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
    else cout << "Error in executing  query: " << querystring << endl;


  return i;
}

//-----------------------------------------------------------------------------

Relation* QueryRelation(string relname) 
{
   ListExpr s;
   return QueryRelation(relname, s);
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

Relation* QueryRelation(string relname, ListExpr &reltype) 
{
   // query Street feed addcounter[Gid,1] consume
  string querystring = "(consume (feed " + relname + "))";
  //string querystring = "(consume (addcounter (feed  "+relname+") Gid 1))";
  
   Word resultWord;
   
    Word queryResult;
    std::string typeString = "";
    std::string errorString = "";
    bool correct;
    bool evaluable;
    bool defined;
    bool isFunction;

    // use the queryprocessor for executing the expression
   ListExpr queryList;
    nl->ReadFromString( querystring, queryList );

    qp->ExecuteQuery(queryList, queryResult, 
                    typeString, errorString, correct, 
                    evaluable, defined, isFunction);


   if ( QueryProcessor::ExecuteQuery(querystring, resultWord ) )
   {
      nl->ReadFromString(typeString, reltype);
      //cout << "type: " << typeString << endl;
      
      return ( Relation * ) resultWord.addr;    
   }
   else cout << "Error in executing  query: " << querystring << endl;

  return NULL;
}

} // namespace pgraph
