/*
----
This file is part of SECONDO.

Copyright (C) 2015,
Faculty of Mathematics and Computer Science,
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


//[$][\$]

*/

#include <string>
#include "Dist2Helper.h"
#include "SecondoSMI.h"
#include "Algebras/FText/FTextAlgebra.h"

using namespace std;

namespace distributed2{

template<>
bool readVar<string>(string& value, SmiRecord& record, size_t& offset){
  size_t len;
  if(!readVar<size_t>(len,record,offset)){
    return false;
  }
  //assert(len<=48);

  if(len==0){
     value = "";
     return true;
  }
  char* cstr = new char[len];
  bool res = record.Read(cstr, len, offset) == len;
  offset += len;
  value.assign(cstr,len);
  delete[] cstr;
  return res;
}

template<>
bool writeVar<string>(
  const string& value, SmiRecord& record, size_t& offset){

  // write the size of the sting
  size_t len = value.length();
  if(!writeVar<size_t>(len,record,offset)){
     return false;
  }
  if(len==0){
     return true;
  }
  if(record.Write(value.c_str(),len, offset)!=len){
     return false;
  }
  offset+= len;
  return true;
}


DebugWriter dwriter;

void showCommand(SecondoInterfaceCS* src,
                 const std::string& host,
                 const int port,
                 const std::string& cmd,
                 bool start,
                 bool showCommands)
{
    // boost::mutex showCommandMtx; //TODO FIXME
    if (showCommands)
    {
        dwriter.write(
                showCommands,
                std::cout,
                src,
                src->getPid(),
                "= " + host + ":" + stringutils::int2str(port) + ":"
                        + (start ? "start " : "finish ") + cmd);
    }

}

/*
Some Helper functions.

*/

std::string getUDRelType(ListExpr r){

  assert(Relation::checkType(r) || frel::checkType(r));
  ListExpr attrList = nl->Second(nl->Second(r));
  std::string rt = nl->SymbolValue(nl->First(r));
  std::string res = rt+"(tuple([";
  bool first = true;
  while(!nl->IsEmpty(attrList)){
    if(!first){
      res += ", ";
    } else {
      first = false;
    }
    ListExpr attr = nl->First(attrList);
    attrList = nl->Rest(attrList);
    res += nl->SymbolValue(nl->First(attr));
    res += " : " + nl->ToString(nl->Second(attr));
  }
  res +="]))";
  return res;
}

/*
1.0.1 ~rewriteQuery~

This function replaces occurences of [$]<Ident> within the string orig
by corresponding const expressions. If one Ident does not represent
a valid object name, the result will be false and the string is
unchanged.

*/

std::string rewriteRelType(ListExpr relType){
  if(!Relation::checkType(relType)){
     return nl->ToString(relType);
  }

  ListExpr attrList = nl->Second(nl->Second(relType));

  std::stringstream ss;
  ss << "rel(tuple([";
  bool first = true;
  while(!nl->IsEmpty(attrList)){
     if(!first){
       ss << ", ";
     } else {
        first = false;
     }
     ListExpr attr = nl->First(attrList);
     attrList = nl->Rest(attrList);
     ss << nl->SymbolValue(nl->First(attr));
     ss << " : ";
     ss << nl->ToString(nl->Second(attr));
  }
  ss << "]))";
  return ss.str();
}

/*
The next function returns a constant expression for an
database object.

*/

bool getConstEx(const std::string& objName, std::string& result){

   SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
   if(!ctlg->IsObjectName(objName)){
       return false;
   }
   result = " [const " + rewriteRelType(ctlg->GetObjectTypeExpr(objName))
          + " value " + nl->ToString(ctlg->GetObjectValue(objName)) + "] ";

   return true;
}


template<class T>
void print(std::vector<T>& v, std::ostream& out){
  for(size_t i=0;i<v.size();i++){
     out << v[i] << " ";
  }
}



/*
rewrites a query. Replaces dollar signs by numbers.

*/
bool rewriteQuery(const std::string& orig, std::string& result){

  std::stringstream out;
  size_t pos = 0;
  size_t f = orig.find_first_of("$", pos);
  std::map<std::string,std::string> used;
  while(f!=std::string::npos){
     size_t f2 = f +1;
     if(f2 < orig.length()){
        if(stringutils::isLetter(orig[f2])){
           if(f>pos){
              out << orig.substr(pos , f - pos);
           }
           std::stringstream ident;
           ident << orig[f2];
           f2++;
           while(f2<orig.length() && (stringutils::isLetter(orig[f2]) ||
                 stringutils::isDigit(orig[f2] || (orig[f2]=='_')))){
              ident  << orig[f2];
              f2++;
           }
           std::string constEx;
           if(used.find(ident.str())!=used.end()){
             constEx = used[ident.str()];
           }  else {
              if(!getConstEx(ident.str(),constEx)){
                  result =  orig;
                  return false;
              }
           }
           out << constEx;
           pos = f2;
        } else if(orig[f2]=='$'){ // masked dollar
            out <<  orig.substr(pos,f2-pos);
            pos = f2+1;
        } else { // not a indent after $
            out <<  orig.substr(pos,f2+1-pos);
            pos = f2;
           pos++;
        }
     } else { // end of
       out << orig.substr(pos,std::string::npos);
       pos = f2;
     }
     f = orig.find_first_of("$", pos);
  }
   out << orig.substr(pos,std::string::npos);
   result = out.str();
  return true;
}

bool isWorkerRelDesc(ListExpr rel, ListExpr& positions, ListExpr& types,
                     std::string& errMsg){

  if(!Relation::checkType(rel)){
     errMsg = " not a relation";
     return false;
  }
  ListExpr attrList = nl->Second(nl->Second(rel));

  ListExpr htype;

  int hostPos = listutils::findAttribute(attrList,"Host",htype);
  if(!hostPos){
     errMsg = "Attribute Host not present in relation";
     return false;
  }
  if(!CcString::checkType(htype) && !FText::checkType(htype)){
     errMsg = "Attribute Host not of type text or string";
     return false;
  }
  hostPos--;

  ListExpr ptype;
  int portPos = listutils::findAttribute(attrList,"Port",ptype);
  if(!portPos){
    errMsg = "Attribute Port not present in relation";
    return false;
  }
  if(!CcInt::checkType(ptype)){
     errMsg = "Attribute Port not of type int";
     return false;
  }
  portPos--;
  ListExpr ctype;
  int configPos = listutils::findAttribute(attrList, "Config", ctype);
  if(!configPos){
    errMsg = "Attrribute Config not present in relation";
    return false;
  }
  if(!CcString::checkType(ctype) && !FText::checkType(ctype)){
     errMsg = "Attribute Config not of type text or string";
     return false;
  }
  configPos--;
  positions = nl->ThreeElemList(
               nl->IntAtom(hostPos),
               nl->IntAtom(portPos),
               nl->IntAtom(configPos));
  types = nl->ThreeElemList(htype, ptype,ctype);

  return true;
}


}



