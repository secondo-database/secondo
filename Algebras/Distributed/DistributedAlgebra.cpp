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
*/


/*
[1] DistributedAlgebra

March 2010 Tobias Timmerscheidt

This algebra implements an array, that holds all its elements at remote servers. Currently only one server can be used.
The address and port of the server must be set at lines 188, 238 and 261 of this file.



1. Preliminaries

1.1 Includes

*/

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "RelationAlgebra.h"

#include "TypeMapUtils.h"
#include "Symbols.h"

#include "Remote.h"

#include <sstream>

using namespace std;
using namespace symbols;
using namespace mappings;

extern NestedList* nl;
extern QueryProcessor *qp;


/*

1.2 Auxiliary Functions

*/

//Uses Function from ArrayAlgebra
void extractIds(const ListExpr,int&,int&);


//Converts int to string
string toString_d(int i)
{
         std::string s;
         std::stringstream out;
         out << i;
         s = out.str();
         return s;
}


//Creates an unique identifier for a new distributed array
string getArrayName(int number)
{
         string t = toString_d(time(0)) + toString_d(number);
         return t;
}

ListExpr convertSingleType( ListExpr type)
{
     if(nl->ListLength(type) != 2) return nl->SymbolAtom("ERROR");
     if(!nl->IsAtom(nl->First(type)) || !nl->IsAtom(nl->Second(type)))
               return nl->SymbolAtom("ERROR");
     
     int algID, typID;
     
     extractIds(type,algID,typID);
     
     SecondoCatalog* sc = SecondoSystem::GetCatalog();
     if(algID < 0 || typID < 0) return nl->SymbolAtom("ERROR");
     
     return nl->SymbolAtom(sc->GetTypeName(algID,typID));
}

ListExpr convertType( ListExpr type )
{
     ListExpr result,result2; 
   
     if(nl->ListLength(type) < 2)
          if(nl->IsAtom(type) || nl->IsEmpty(type)) return type;
          else return convertType(nl->First(type));
     
     if(nl->ListLength(type) == 2 &&
          nl->IsAtom(nl->First(type)) &&
          nl->IsAtom(nl->Second(type)))
                    return convertSingleType(type);
     
     result = convertType(nl->First(type));
     result2 = convertType(nl->Rest(type)); 
     //result = nl->Append(result, convertType(nl->Rest(type)));
    
     return nl->TwoElemList(result,result2);
}
     


/*


2. Type Constructor ~DArray~

2.1 Data Structure - Class ~DArray~

*/


class DArray
{
         public:
         DArray();
         DArray(ListExpr, string,int,ListExpr);
         
         ~DArray();
         
         void initialize(ListExpr, string, int,ListExpr,Word*);
         void initialize(ListExpr, string,int,ListExpr);
         Word get(int);
         void set(Word,int);
         
         int getSize();
         int getAlgID();
         int getTypID();
         DServerManager* getServerManager();
         ListExpr getServerList();
         Word* getElements();
         bool isDefined();
         string getName();
         ListExpr getType();
         
         void refresh(int);
         
         void remove();
         
         static Word In( const ListExpr typeInfo , const ListExpr instance ,
                                    const int errorPos , ListExpr& errorInfo ,
                                    bool& correct );
         static ListExpr Out( ListExpr typeInfo , Word value );
         static Word Create( const ListExpr typeInfo );
         static void Delete( const ListExpr typeInfo , Word& w );
         static void Close( const ListExpr typeInfo, Word& w );
         static Word Clone( const ListExpr typeInfo , const Word& w );
         static bool KindCheck( ListExpr type , ListExpr& errorInfo );
         static int SizeOfObj();
         static bool Open( SmiRecord& valueRecord ,
                                  size_t& offset , const ListExpr typeInfo,
                                             Word& value );
         static bool Save( SmiRecord& valueRecord , size_t& offset ,
                                     const ListExpr typeInfo , Word& value );
         
         
         
         static int no;
         
         
         private:
         bool defined;
         int size;
         int alg_id;
         int typ_id;
         string name;
         ListExpr type;
         
         ListExpr serverlist;
         
         DServerManager* manager;
         //DServer* server;
         Word* elements;
         
};

/*

2.2 Implementation of basic functions

*/

int DArray::no = 0;

DArray::DArray()
{
         defined = false;
         size=0;
         alg_id=0;
         typ_id=0;
         name="";
         no++;
}

DArray::DArray(ListExpr n_type, string n, int s, ListExpr n_serverlist)
{
         defined = true;
         
         type = n_type;
         
         
         
         extractIds( type, alg_id, typ_id);
         name = n;
         
         size = s;
                  
         elements = new Word[size];
         
         serverlist = n_serverlist;
         manager = new DServerManager(serverlist, name,type,size);
         //server = manager->getServerbyID(0);
         //server = new DServer("192.168.2.3",1234,name,type);         
         //Server must be specified in the
//         code for now (see also DArray::initialize below)
         
         no++;
         
}



DArray::~DArray()
{
         no--;
         if(defined){
         
         delete elements;
         
         /*if(server != 0) server->Terminate();
         delete server;*/
         delete manager;}
}

void DArray::remove()
{
         
         if(defined)
         for(int i = 0;i<size;i++)
         {
                  DServer* server = manager->getServerByIndex(i);
                  server->setCmd("delete",nl->IntAtom(i),elements);
                  server->run();
         }
                  
}

void DArray::refresh(int i)
{
         //cout << "Refreshing Element" << toString_d(i);
         DServer* server = manager->getServerByIndex(i);
         server->setCmd("read",nl->IntAtom(i),elements);
         server->run();
}


void DArray::initialize(ListExpr n_type, string n, int s,
                                  ListExpr n_serverlist, Word* n_elem)
{
         defined = true;
         
         type = n_type;
         
         extractIds( type , alg_id, typ_id);
         name = n;
         
         size = s;
         
         serverlist = n_serverlist;
        
         manager = new DServerManager(serverlist, name,type,size);
         //server = manager->getServerbyID(0);
         elements = n_elem;
         //server = new DServer("192.168.2.3",1234,name,type);
         //Server must be specified in the code
//         for now (see also Constructor above)
         
         /*for(int i = 0; i<size; i++)
         {
                  DServer* server = manager->getServerByIndex(i);
                  server->setCmd("write",nl->IntAtom(i),elements);
                  server->run();
         }*/
         for(int i = 0; i<manager->getNoOfServers();i++)
         {
                 DServer* server = manager->getServerbyID(i);
                 server->setCmd("write",manager->getIndexList(i),elements);
                 server->run();
         }
}


void DArray::initialize(ListExpr n_type, string n, int s, ListExpr n_serverlist)
{
         defined = true;
         
         type = n_type;
         
         extractIds( type , alg_id, typ_id);
         name = n;
         
         size = s;
         
         elements = new Word[size];
         
         serverlist = n_serverlist;
         manager = new DServerManager(serverlist, name,type,size);
         //server = manager->getServerbyID(0);
         /*server = new DServer(
         "192.168.2.3",1234,name,type);                  //see above*/
         
}


Word DArray::get(int i) 
{ 
         if(defined) 
         {
                  refresh(i); 
                 return elements[i];
         } 
         else 
         { 
                  cout << "Error: Array nicht definiert!!"; return new Word(); 
         } 
}

void DArray::set(Word n_elem, int i) 
{ 
         if(defined) 
         {
                  DServer* server = manager->getServerByIndex(i);
                  elements[i] = n_elem;
                  server->setCmd("write",nl->IntAtom(i),elements);
                  server->run();
         }
}

         
int DArray::getAlgID() { return alg_id; }

int DArray::getTypID() { return typ_id; }

ListExpr DArray::getType() { return type; }

ListExpr DArray::getServerList() { return serverlist; }

string DArray::getName() { return name; }

bool DArray::isDefined() { return defined; }

int DArray::getSize() { return size; }

DServerManager* DArray::getServerManager() {return manager;}

Word* DArray::getElements() {return elements;}


/*

2.3 In and Out functions

*/

Word DArray::In( const ListExpr typeInfo, const ListExpr instance,
                        const int errorPos, ListExpr& errorInfo,
                        bool& correct )
{
         
         Word e;
         int algID, typID;
         
         extractIds(nl->Second(typeInfo),algID,typID);
         DArray* a = new DArray(nl->Second(typeInfo),
                                                      getArrayName(DArray::no), 
                                                      nl->ListLength(instance),
                                                      nl->First(instance));
         ListExpr listOfElements = nl->Rest(instance);
         ListExpr element;
         int i = 0;
         
         do
         {
                  element = nl->First(listOfElements);
                  listOfElements = nl->Rest(listOfElements);
                  e = ((am->InObj(algID,typID))
                           (nl->Second(typeInfo),element,
                           errorPos,errorInfo,correct));
                  a->set(e,i);
                  i++;
         }
         while(!nl->IsEmpty(listOfElements) && correct);
         
         if(correct)
         {
                  return SetWord(a);
         }
         
         correct=false;
         return SetWord(Address(0));
         
         
}

ListExpr DArray::Out( ListExpr typeInfo, Word value )
{
         DArray* a = (DArray*)value.addr;
         
         ListExpr list;
         ListExpr last;
         ListExpr element;
         list = nl->OneElemList(a->getServerList());last=list;
         
         if(a->isDefined())
                  for(int i = 0; i<a->getSize();i++)
                  {
                           element = ((am->OutObj(a->getAlgID(),a->getTypID()))
                                             (nl->Second(typeInfo),a->get(i)));
                           /*if(i==0) {list=nl->OneElemList(element);last=list;}
                           else*/         last=nl->Append(last,element);
                  }
                           
         else 
         {
                  cout << "Fehler! DArray nicht definiert";
                  ListExpr err;
                  return err;
         }
         
         return list;
}


/*

2.4 Persistent Storage Functions

*/


Word DArray::Create( const ListExpr typeInfo )
{
         
         return SetWord(new DArray());
}
                  
         
         
void DArray::Delete( const ListExpr typeInfo, Word& w )
{
         ((DArray*)w.addr)->remove();
         delete (DArray*)w.addr;
         w.addr = 0;
}

void DArray::Close( const ListExpr typeInfo, Word& w )
{
         delete (DArray*)w.addr;
         w.addr = 0;
}

Word DArray::Clone( const ListExpr typeInfo, const Word& w )
{
         DArray* alt = (DArray*)w.addr;
         DArray* neu;
         
         neu = new DArray(nl->Second(typeInfo),
                                             getArrayName(DArray::no),
                                             alt->getSize(),
                                             alt->getServerList());
                  
         for(int i =0;i<alt->getSize();i++)
         {
                  ListExpr arg = nl->TwoElemList(nl->StringAtom(neu->getName()),
                                                           nl->IntAtom(i));
                  alt->getServerManager()->getServerByIndex(i)
                        ->setCmd("copy",arg,neu->getElements());
                  alt->getServerManager()->getServerByIndex(i)->run();
         }
         

         return SetWord(neu);
}


bool DArray::Open( SmiRecord& valueRecord , 
                                    size_t& offset , 
                                    const ListExpr typeInfo , 
                                    Word& value )
{
         char* buffer;
         string name, type, server;
         int length;
         int size;
         
         valueRecord.Read(&size,sizeof(int),offset);
         offset+=sizeof(int);
         
         valueRecord.Read(&length, sizeof(length), offset);
         offset += sizeof(length);
         buffer = new char[length];
         valueRecord.Read(buffer, length, offset);
         offset += length;
         type.assign(buffer, length);
         delete buffer;
        
         valueRecord.Read(&length, sizeof(length), offset);
         offset += sizeof(length);
         buffer = new char[length];
         valueRecord.Read(buffer, length, offset);
         offset += length;
         server.assign(buffer, length);
         delete buffer;
         
         valueRecord.Read(&length, sizeof(length), offset);
         offset += sizeof(length);
         buffer = new char[length];
         valueRecord.Read(buffer, length, offset);
         offset += length;
         name.assign(buffer, length);
         delete buffer;
         
         ListExpr typeList;
         nl->ReadFromString( type, typeList);
         
         ListExpr serverlist;
         nl->ReadFromString(server,serverlist);
         
                  
         value.addr = ((Word)new DArray(typeList,name,size,serverlist)).addr;
         return true;
}

bool DArray::Save( SmiRecord& valueRecord ,
                                    size_t& offset , 
                                    const ListExpr typeInfo , 
                                    Word& value )
{
         int length;
         int size = ((DArray*)value.addr)->getSize();
         
         valueRecord.Write(&size, sizeof(int),offset);
         offset+=sizeof(int);
        
         string type;
         nl->WriteToString( type, nl->Second(typeInfo) );
         length = type.length();
         valueRecord.Write( &length, sizeof(length), offset);
         offset += sizeof(length);
         valueRecord.Write ( type.data(), length, offset);
         offset += length;
        
         string server;
         nl->WriteToString( server, ((DArray*)value.addr)->getServerList());
         length = server.length();
         valueRecord.Write( &length, sizeof(length), offset);
         offset += sizeof(length);
         valueRecord.Write ( server.data(), length, offset);
         offset += length;
         
         string name = ((DArray*)value.addr)->getName();
         length = name.length();
         
         valueRecord.Write( &length, sizeof(length), offset);
         offset += sizeof(length);
         valueRecord.Write ( name.data(), length, offset);
         offset += length;
         
         
         
         return true;
}

/*

2.5 Kind Checking

*/

bool DArray::KindCheck( ListExpr type, ListExpr& errorInfo )
{
         if(nl->ListLength(type) == 2)
         {
                  if(nl->IsEqual(nl->First(type),"darray"))
                  {
                  
                           SecondoCatalog* sc = SecondoSystem::GetCatalog();
                           if(sc->KindCorrect(nl->Second(type),errorInfo))
                           {
                                    return true;
                           }
                  }
         }
         
         return false;
         
}
         
         

int DArray::SizeOfObj()
{
         return sizeof(DArray);
}


/*

2.6 Type Constructor

*/

struct darrayInfo : ConstructorInfo {

  darrayInfo() {

    name         = "darray";
    signature    = "typeconstructor -> ARRAY" ;
    typeExample  = "darray int";
    listRep      =  "(a1 a2 a3)";
    valueExample = "(4 12 2 8)";
    remarks      = "";
  }
};


struct darrayFunctions : ConstructorFunctions<DArray> {

  darrayFunctions()
  {

         create = DArray::Create;
         in = DArray::In;
         out = DArray::Out;
         close = DArray::Close;
         deletion = DArray::Delete;
         clone = DArray::Clone;
         kindCheck = DArray::KindCheck;
         open = DArray::Open;
         save = DArray::Save;
 
  }
};

darrayInfo dai;
darrayFunctions daf;
TypeConstructor darrayTC( dai, daf );









                  

/*

4.1 Operator makeDarray

Typemap (t t t...) -> darray t

*/

static ListExpr
makeDarrayTypeMap( ListExpr args )
{
         ListExpr slist = nl->First(args);
         if(nl->ToString(slist) != "(rel (tuple ((Server string) (Port int))))")
                 return nl->SymbolAtom("typeerror");
         args = nl->Rest(args);
         ListExpr first = nl->First(args);
         ListExpr rest = nl->Rest(args);
         while(!(nl->IsEmpty(rest)))
         {
                  if(!nl->Equal(nl->First(rest),first))
                           return nl->SymbolAtom("typeerror");
                  rest = nl->Rest(rest);
         }
         return nl->TwoElemList(nl->SymbolAtom("darray"),nl->First(args));
}

static int
makeDarrayfun( Word* args, Word& result, int message, Word& local, Supplier s )
{
         SecondoCatalog* sc = SecondoSystem::GetCatalog();         
         //3 lines copied from ArrayAlgebra.cpp

         ListExpr type = qp->GetType(s);
         ListExpr typeOfElement = sc->NumericType(nl->Second(type));

         int algID, typID;
         extractIds( typeOfElement, algID, typID);
         
         int size = qp->GetNoSons(s)-1;
         
         Word* cloned = new Word[size];
         
         for(int i = 0;i<size;i++)
         {                  
             cloned[i] = (am->CloneObj(algID,typID))(typeOfElement,args[i+1]);
         }
         
         GenericRelation* r = (GenericRelation*)args[0].addr;
        GenericRelationIterator* rit = r->MakeScan();
         ListExpr reltype;
         nl->ReadFromString("(rel (tuple ((Server string) (Port int))))",
                                         reltype);
         ListExpr serverlist = Relation::Out(reltype,rit);
        /*ListExpr serverlist = nl->TwoElemList((nl->TwoElemList
                                        (nl->StringAtom("192.168.2.3"),
                                                       nl->IntAtom(1234))),
                                        (nl->TwoElemList(
                                                nl->StringAtom("127.0.0.1"),
                                                nl->IntAtom(1234))));*/
         result = qp->ResultStorage(s);
         ((DArray*)result.addr)->initialize(typeOfElement, 
                                        getArrayName(DArray::no),
                                                        size,serverlist,
                                                                 cloned);
         
         
         return 0;
}

const string makeDarraySpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>t -> darray t</text--->"
       "<text>makeDarray ( _, _ )</text--->"
       "<text>Returns a distributed Array containing x element</text--->"
       "<text>query makeDarray(1,2,3)</text---> ))";

Operator makeDarray(
         "makeDarray",
         makeDarraySpec,
         makeDarrayfun,
         Operator::SimpleSelect,
         makeDarrayTypeMap);

/* 

4.2 Operator get

((darray t) int) -> t

*/

static ListExpr getTypeMap( ListExpr args )
{
         if(nl->ListLength(args) == 2)
         {
                  ListExpr arg1 = nl->First(args);
                  ListExpr arg2 = nl->Second(args);
                  
                  if(!nl->IsAtom(arg1) && 
                           nl->IsEqual(nl->First(arg1),"darray") &&
                           nl->IsEqual(arg2,"int"))
                  {
                           ListExpr resulttype = nl->Second(arg1);
                           return resulttype;
                  }
         }
         
         return nl->SymbolAtom("typeerror");
}

static int getFun( Word* args, 
                                    Word& result, 
                                    int message, 
                                    Word& local, 
                                    Supplier s)
{
         DArray* array = ((DArray*)args[0].addr);
         CcInt* index = ((CcInt*)args[1].addr);
         
         int i = index->GetIntval();
         
         // Following lines from ArrayAlgebra.cpp
         SecondoCatalog* sc = SecondoSystem::GetCatalog();
         ListExpr resultType = qp->GetType(s);

         if (nl->ListLength(resultType) > 1) {
      if (nl->IsEqual(nl->First(resultType), "map")) {

        // In case of a mapping only the type of the resulting object of
        // the mapping is relevant.

        while (nl->ListLength(resultType) > 1) {
          resultType = nl->Rest(resultType);
        }
        resultType = nl->First(resultType);
      }
    }
    resultType = sc->NumericType(resultType);
    
    int algID,typID; extractIds(resultType,algID,typID);
    Word cloned = (am->CloneObj(algID,typID))(resultType,(Word)array->get(i));

    //code from ArrayAlgebra.cpp ends here
    result = qp->ResultStorage(s);
    result.addr = cloned.addr;
    return 0;
}
         
const string getSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>darray t -> t</text--->"
       "<text>get ( _, _ )</text--->"
       "<text>Returns an element from a distributed Array</text--->"
       "<text>query get(makeDarray(1,2,3),1)</text---> ))";

Operator getA(
         "get",
         getSpec,
         getFun,
         Operator::SimpleSelect,
         getTypeMap);


/*

4.3 Operator put

((darray t)<, t, int) -> (darray t)

*/

static ListExpr putTypeMap( ListExpr args )
{
         if(nl->ListLength(args) == 3)
         {
                  ListExpr arg1 = nl->First(args);
                  ListExpr arg2 = nl->Second(args);
                  ListExpr arg3 = nl->Third(args);
                  
                 if(!nl->IsAtom(arg1) && nl->IsEqual(nl->First(arg1),"darray")
                     && nl->Equal(nl->Second(arg1),arg2)  
                     && nl->IsEqual(arg3,"int"))
                  {
                           return arg1;
                  }
         }
         
         return nl->SymbolAtom("typeerror");
}

static int putFun( Word* args,
                                    Word& result,
                                    int message,
                                    Word& local,
                                    Supplier s)
{
         DArray* array_alt = ((DArray*)args[0].addr);
         Word element = args[1];
         int i = ((CcInt*)args[2].addr)->GetIntval();

         
         Word elem_n = ((am->CloneObj(array_alt->getAlgID(),
                                    array_alt->getTypID()))
                                    (array_alt->getType(),element));

         result = qp->ResultStorage(s);
         ((DArray*)result.addr)->initialize(array_alt->getType(),
                                                               getArrayName
                                                                  (DArray::no),
                                                      array_alt->getSize(),
                                                array_alt->getServerList());
         
         for(int j = 0; j<array_alt->getSize();j++)
         {
                  if(j!=i)
                  {
                           ListExpr arg = nl->TwoElemList
                                                      (nl->StringAtom(
                                                     ((DArray*)result.addr)
                                                               ->getName()),
                                                               nl->IntAtom(j));
                           array_alt->getServerManager()->getServerByIndex(j)
                                                        ->setCmd("copy",
                                           arg,array_alt->getElements());
                           array_alt->getServerManager()->getServerByIndex(j)
                                                        ->run();
                  }
                  else
                  {
                           ((DArray*)result.addr)->set(elem_n,j);
                  }
         }
         
         return 0;
}

const string putSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>darray t -> darray t</text--->"
       "<text>put ( _, _, _ )</text--->"
"<text>Returns a distributed array where one element is altered</text--->"
       "<text>query put(makeDarray(1,2,3),2,2)</text---> ))";

Operator putA(
         "put",
         putSpec,
         putFun,
         Operator::SimpleSelect,
         putTypeMap);

/* 

4.5 Operator send

Internal Usage for Data Transfer between Master and Worker

*/

static ListExpr sendTypeMap( ListExpr args )
{
     /*if(nl->IsEqual(nl->First(args),"Symbol"))
             if(nl->IsEqual(nl->Second(args),"Symbol"))
                  return nl->SymbolAtom("int");
     {cout << "Fehler Typemapping"; return nl->SymbolAtom("typeerror");}*/
      return nl->ThreeElemList(
                    nl->SymbolAtom("APPEND"),
                    nl->TwoElemList(nl->StringAtom(
                                               nl->ToString(nl->First(args))),
                                              nl->StringAtom(nl->ToString(
                                                  nl->Second(args)))),
                    nl->SymbolAtom("int"));

}

static int sendFun( Word* args, 
                                    Word& result, 
                                    int message, 
                                    Word& local, 
                                    Supplier s)
{

     string host = (string)(char*)((CcString*)args[3].addr)->GetStringval();
     string port = (string)(char*)((CcString*)args[4].addr)->GetStringval();
     string line;

     host = replaceAll(host,"_",".");
     host = replaceAll(host,"h","");
     port = replaceAll(port,"p","");

     Socket* master = Socket::Connect(host,port,Socket::SockGlobalDomain);
     
     
     
     if(master!=0 && master->IsOk())
     {

          iostream& iosock = master->GetSocketStream();
          getline(iosock,line);
          
          if(line=="<TYPE>")
          {
               getline(iosock,line);
               ListExpr type;
               nl->ReadFromString(line,type);

               getline(iosock,line);
               if(line=="</TYPE>")
               {
                    int algID,typID;
                    extractIds(type,algID,typID);
                    
                    SmiRecordFile recF(false,0);
                    SmiRecord rec;
                    SmiRecordId recID;
                    
                    recF.Open("sendop");
                    recF.AppendRecord(recID,rec);
                    size_t size = 0;
                    
                    am->SaveObj(algID,typID,rec,size,type,args[2]);
                    char* buffer = new char[size];
                    
                    rec.Read(buffer,size,0);
                    rec.Truncate(3);
                    recF.DeleteRecord(recID);
                    recF.Close();

                    iosock << "<SIZE>" << endl << size << endl
                         << "</SIZE>" << endl;

                    master->Write(buffer,size);

                    result = qp->ResultStorage(s);

                    ((CcInt*)result.addr)->Set(0);
                    
                    //recF.Remove();

                    
               }
               master->Close();delete master;master=0;
              
               return 0;
          }
          result = qp->ResultStorage(s);
          ((CcInt*)result.addr)->Set(1);
     }
                    
     return 0;               
		
                    
     }
         
const string sendSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>-</text--->"
       "<text>-</text--->"
       "<text>Internal Usage by DistributedAlgebra</text--->"
       "<text>-</text---> ))";

Operator sendA(
         "sendD",
         sendSpec,
         sendFun,
         Operator::SimpleSelect,
         sendTypeMap);


/* 

4.6 Operator receive

Internal Usage for Data Transfer between Master and Worker

*/

static ListExpr receiveTypeMap( ListExpr args )
{
     string host = nl->ToString(nl->First(args));
     string port = nl->ToString(nl->Second(args));
     string line;
     
     host = replaceAll(host,"_",".");
     host = replaceAll(host,"h","");
     port = replaceAll(port,"p","");

     Socket* master = Socket::Connect(host,port,Socket::SockGlobalDomain);
     
     if(master==0 || !master->IsOk()) 
          {cout << "FEHLER bei Typemapping";
               return nl->SymbolAtom("typeerror");}
     
     iostream& iosock = master->GetSocketStream();
     
     getline(iosock,line);
     if(line!= "<TYPE>") return nl->SymbolAtom("typeerror");
     getline(iosock,line);
     ListExpr type;
     nl->ReadFromString(line,type);
     getline(iosock,line);
     if(line!= "</TYPE>") return nl->SymbolAtom("typeerror");
     
     iosock << "<CLOSE>" << endl;
     
     master->Close(); delete master; master=0;
     
     SecondoCatalog* sc = SecondoSystem::GetCatalog();
     int algID, typID; extractIds(type,algID,typID);

      return nl->ThreeElemList(
          nl->SymbolAtom("APPEND"),
          nl->TwoElemList(nl->StringAtom(nl->ToString(nl->First(args))),
                              nl->StringAtom(nl->ToString(nl->Second(args)))),
                              convertType(type));

}

static int receiveFun( Word* args, 
                                    Word& result, 
                                    int message, 
                                    Word& local, 
                                    Supplier s)
{

     string host = (string)(char*)((CcString*)args[2].addr)->GetStringval();
     string port = (string)(char*)((CcString*)args[3].addr)->GetStringval();
     string line;

     host = replaceAll(host,"_",".");
     host = replaceAll(host,"h","");
     port = replaceAll(port,"p","");

     Socket* master = Socket::Connect(host,port,Socket::SockGlobalDomain);
     
     
     
     if(master!=0 && master->IsOk())
     {

          iostream& iosock = master->GetSocketStream();
          iosock << "LOS" << endl;
          getline(iosock,line);
          
          if(line=="<TYPE>")
          {
               getline(iosock,line);
               ListExpr type;
               nl->ReadFromString(line,type);

               getline(iosock,line);
               if(line=="</TYPE>")
               {
                    int algID,typID;
                    extractIds(type,algID,typID);
                    size_t size =0;
                    
                    getline(iosock,line);
                    if(line=="<SIZE>")
                    {
                    getline(iosock,line);
                         
                     size = atoi(line.data());

                    getline(iosock,line);
                    if(line=="</SIZE>")
                    {
                         
                         char* buffer = new char[size]; 
                         iosock.read(buffer,size);


                         SmiRecordFile recF(false,0);
                         SmiRecord rec;
                         SmiRecordId recID;
                    
                         recF.Open("receiveop");
                         recF.AppendRecord(recID,rec);
                         size_t s0 = 0;
                         
                         rec.Write(buffer,size,0);
                         
                         result = qp->ResultStorage(s);
                         am->OpenObj(algID,typID,rec,s0,type,result); 

                         rec.Truncate(3);
                         recF.DeleteRecord(recID);
                         recF.Close();
                         

                    }}
                    
               }
               master->Close();delete master;master=0;
              
               return 0;
          }
     
     }
      
     result = qp->ResultStorage(s);
     ((CcInt*)(result.addr))->Set(true,3);
     return 0;               
		
                    
     }
         
const string receiveSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>-</text--->"
       "<text>-</text--->"
       "<text>Internal Usage by DistributedAlgebra</text--->"
       "<text>-</text---> ))";

Operator receiveA(
         "receiveD",
         receiveSpec,
         receiveFun,
         Operator::SimpleSelect,
         receiveTypeMap);
/* 

5 Creating the Algebra 

*/



class DistributedAlgebra : public Algebra
{
  public:
    DistributedAlgebra() : Algebra()
    {
             darrayTC.AssociateKind("ARRAY");
             AddTypeConstructor( &darrayTC );
             
             AddOperator( &makeDarray );
             AddOperator( &getA );
             AddOperator( &putA );
             AddOperator( &sendA );
             AddOperator( &receiveA);
    }
    ~DistributedAlgebra() {}
};



/*

6 Initialization

*/

extern "C"
Algebra*
InitializeDistributedAlgebra(NestedList *nlRef, QueryProcessor *qpRef)
{
  nl = nlRef;
  qp = qpRef;
  return new DistributedAlgebra();
}


