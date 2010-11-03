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

This algebra implements a distributed array. This type of array keeps its element on remote servers, called worker. Upon creation of the array all elements
are transfered to the respective workers. The list of workers must be specified in terms of a relation in any operator that gives back a darray.
Operation on the darray-elements are carried out on the remote machines.



1. Preliminaries

1.1 Includes

*/

#include "Algebra.h"
#include "NestedList.h"
#include "NList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "RelationAlgebra.h"
#include "TypeMapUtils.h"
#include "Remote.h"
#include "zthread/ThreadedExecutor.h"
#include "../FText/FTextAlgebra.h"

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

//Converts a pair (algID typID) to the corresponding 
//type name (used by converType)
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


//Converts a numerical type to its text representation
ListExpr convertType( ListExpr type )
{
   ListExpr result,result2; 
     
     //Is it not a type expression but an attribute name?
   if(nl->ListLength(type) < 2)
   {
      if(nl->IsAtom(type) || nl->IsEmpty(type)) return type;
      //Only one element that is not atomic
      else return convertType(nl->First(type));
   }
     
     //Single type expression
   if(nl->ListLength(type) == 2 &&
         nl->IsAtom(nl->First(type)) &&
         nl->IsAtom(nl->Second(type)))
            
         return convertSingleType(type);
     
   //It's a list with more than three elements, proceed recursively
   result = convertType(nl->First(type));
   result2 = convertType(nl->Rest(type));
   if(nl->ListLength(type) == 2)
   return nl->TwoElemList(result,result2);
   else   
      return nl->Cons(result,result2);
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
         
   //Returns the content of elements[int]
   Word get(int);
   //Sets elements[int] and sends the object to the respective worker
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
         
   //Retrieves the element int/all elements from the worker
   //refresh must be called before calling get()
   void refresh(int);
   void refresh();
         
   //Deletes all the remote elements on the workers
   void remove();
         
   //Persistens Storage functions for the type constructor
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
         
         
   //Static no of existing DArray-Instances, used for naming
   static int no;
         
   bool isRelType() {return isRelation;}
         
   private:
              
   //Sends the relation in elements[index] to the respective worker
   void WriteRelation(int index);
         
   //Is the DArray defined (posseses a name, size, serverlist, type?!)
   bool defined;
   //Is a certain element present on the master?
   bool* present;
   bool isRelation;
   int size;
   int alg_id;
   int typ_id;
   string name;
   ListExpr type;
         
   ListExpr serverlist;
         
   DServerManager* manager;
         
   Word* elements;
         
};

/*

2.2 Implementation of basic functions

*/

//Definition of static variable
int DArray::no = 0;

//Creates an undefined DArray
DArray::DArray()
{
   defined = false;
   isRelation = false;
   size=0;
   alg_id=0;
   typ_id=0;
   name="";
   no++;
   present = 0;
}

//Creates a defined DArray
//A DArray is defined by its name, type, size and serverlist
//It can be defined while all its elements are undefined 
DArray::DArray(ListExpr n_type, string n, int s, ListExpr n_serverlist)
{
   defined = true;
         
   type = n_type;
         
         
         
   extractIds( type, alg_id, typ_id);
     
   SecondoCatalog* sc = SecondoSystem::GetCatalog();
   if(sc->GetTypeName(alg_id,typ_id) == "rel") 
      isRelation = true;
   else 
      isRelation = false;
     
          
   name = n;
         
   size = s;
                  
   elements = new Word[size];
   present = new bool[size];
   //Since no elements are given, none can be present
     
   for(int i = 0; i<size; i++) 
      present[i] = false;
         
   serverlist = n_serverlist;
   manager = new DServerManager(serverlist, name,type,size);
         
   no++;
         
}



DArray::~DArray()
{
   no--;
   if(defined)
   {
         
      for(int i=0;i<size;i++)
      {
         //Elements that are present on the master are deleted
         //note that this deletes NOT the Secondo-objects on the workers
         //they need to be deleted seperately which can be done by
         //the remove-function
         if(present[i])
               (am->DeleteObj(alg_id,typ_id))(type,elements[i]);
      }
      delete elements;
         
      delete manager;
      delete present;
   }
}

void DArray::remove()
{
         
   if(defined)
   {
      ZThread::ThreadedExecutor exec;
      for(int i = 0;i<manager->getNoOfServers();i++)
      {
         DServer* server = manager->getServerbyID(i);
         server->setCmd("delete",manager->getIndexList(i),elements);
         DServerExecutor* server_ex = new DServerExecutor(server);
         exec.execute(server_ex);
      }
         exec.wait();
   }
                  
}

void DArray::refresh(int i)
{
         
   DServer* server = manager->getServerByIndex(i);
   list<int>* l = new list<int>;
   l->push_front(i);
   
   if(isRelation)
   {
      
      if(present[i]) 
         (am->DeleteObj(alg_id,typ_id))(type,elements[i]);
      
      elements[i].addr = (am->CreateObj(alg_id,typ_id))(type).addr;
      server->setCmd("read_rel",l,elements);
      server->run();
   }
   
   else
   {
      server->setCmd("read",l,elements);
      server->run();
   }
   
   present[i] = true;
}


void DArray::refresh()
{
   ZThread::ThreadedExecutor exec;
   DServerExecutor* server_ex;
     
   //Elements are deleted if they were present
   //If the darray has a relation-type new relations must be created
   for(int i=0;i<size;i++)
   {
      if(present[i])
         (am->DeleteObj(alg_id,typ_id))(type,elements[i]);
      
      if(isRelation) 
         elements[i].addr = (am->CreateObj(alg_id,typ_id))(type).addr;
   }
     
     
   for(int i = 0; i < manager->getNoOfServers(); i++)
   {
      DServer* server = manager->getServerbyID(i);
      if(isRelation)
      {
         server->setCmd("read_rel",manager->getIndexList(i),elements);
      }
      else
      {
         server->setCmd("read",manager->getIndexList(i),elements);
      }
          
      server_ex = new DServerExecutor(server);
      exec.execute(server_ex);
   }
   
    exec.wait();
    
     //All elements are present now
    for(int i=0;i<size;i++) present[i] = true;
}


void DArray::initialize(ListExpr n_type, string n, int s,
                                  ListExpr n_serverlist, Word* n_elem)
{
   defined = true;
         
   type = n_type;
         
   extractIds( type , alg_id, typ_id);
     
   SecondoCatalog* sc = SecondoSystem::GetCatalog();
   
   if(sc->GetTypeName(alg_id,typ_id) == "rel") 
      isRelation = true;
   else 
      isRelation = false;
     
   name = n;
         
   size = s;
         
   serverlist = n_serverlist;
        
   manager = new DServerManager(serverlist, name,type,size);
         
   elements = n_elem;
   present = new bool[size];
   
   for(int i = 0; i<size; i++)
      present[i] = true;

      
   ZThread::ThreadedExecutor exec;
     
   if(!isRelation)
   {
         
      for(int i = 0; i<manager->getNoOfServers();i++)
      {

         DServer* server = manager->getServerbyID(i);
         server->setCmd("write",manager->getIndexList(i),elements);
         DServerExecutor* server_exec = new DServerExecutor(server);
         exec.execute(server_exec);
      }
   }
   else
      for(int i= 0;i<manager->getNoOfServers();i++)
         {
            RelationWriter* write = new RelationWriter
               (manager->getServerbyID(i),elements,manager->getIndexList(i));
            exec.execute(write);
         }
     
     exec.wait();
       
}


void DArray::initialize(ListExpr n_type, string n, int s, ListExpr n_serverlist)
{
   defined = true;
         
   type = n_type;
         
   extractIds( type , alg_id, typ_id);
     
   SecondoCatalog* sc = SecondoSystem::GetCatalog();
   
   if(sc->GetTypeName(alg_id,typ_id) == "rel") 
      isRelation = true;
   else 
      isRelation = false;
     
   name = n;
         
   size = s;
         
   elements = new Word[size];
   present = new bool[size];
   for(int i = 0; i<size; i++) 
      present[i] = false;
         
   serverlist = n_serverlist;
   manager = new DServerManager(serverlist, name,type,size);
}


Word DArray::get(int i) 
{ 
   if(defined && present[i]) 
   {
      return elements[i];
   } 
   else 
   { 
      cout << "Error: Array nicht definiert!!"; return new Word(); 
   } 
}

void DArray::set(Word n_elem, int i) 
{ 
   list<int>* l = new list<int>;
   l->push_front(i);
   
   if(defined) 
   {
      elements[i].addr = n_elem.addr;
              
      if(!isRelation)
      {
         DServer* server = manager->getServerByIndex(i);
         server->setCmd("write",l,elements);
         server->run();
      }
      else
         WriteRelation(i);
                  
      present[i] = true;
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


void DArray::WriteRelation(int index)
{
   GenericRelation* rel = (Relation*)elements[index].addr;
   GenericRelationIterator* iter = rel->MakeScan();
     
   DServer* worker = manager->getServerByIndex(index);
     
   Tuple* t;
   
   list<int>* l = new list<int>;
   l->push_front(index);
     
     
   Word* word = new Word[1];

   t = iter->GetNextTuple();
   word[0].addr = t;
   worker->setCmd("open_write_rel",l,word);
   worker->run();
     
   while(t != 0)
   {
          
      word[0].addr = t;
      worker->setCmd("write_rel",0,word);
      worker->run();
      t->DeleteIfAllowed();
      t = iter->GetNextTuple();
   }
     
   worker->setCmd("close_write_rel",0,0);
   worker->run();
     
     
   delete iter;
     

}
          


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
                                          nl->ListLength(instance)-1,
                                          nl->First(instance));
   
   ListExpr listOfElements = nl->Rest(instance);
   ListExpr element;
   int i = 0;
         
   do
   {
      element = nl->First(listOfElements);
      listOfElements = nl->Rest(listOfElements);
      e = ((am->InObj(algID,typID))
               (nl->Second(typeInfo),element,errorPos,errorInfo,correct));
      
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
   list = nl->OneElemList(a->getServerList());
   last=list;
   a->refresh();
         
   if(a->isDefined())
      for(int i = 0; i<a->getSize();i++)
      {
         element = ((am->OutObj(a->getAlgID(),a->getTypID()))
                              (nl->Second(typeInfo),a->get(i)));
         last=nl->Append(last,element);
      }
                           
   else 
   {
      cout << "Fehler! DArray nicht definiert oder Relation";
      ListExpr err = nl->StringAtom("RELATION, KEINE AUSGABE");
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
                  
         
//Note: The Delete-function deletes the entire DArray, therefore the remote
//obejcts are delete and thereafter the local data structure. The Close-
//function only removes the local data structure

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

//A new DArray is created locally with the same type, size and serverlist
//It is given a new name and all the elements of the old array are copied
//on the workers
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
      
      list<int>* l = new list<int>;
      l->push_front(i);
      
      string to = neu->getName();
      Word* w = new Word[0];
      w[0].addr = &to;

      alt->getServerManager()->getServerByIndex(i)
                  ->setCmd("copy",l,w);
      
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

struct darrayInfo : ConstructorInfo 
{

   darrayInfo()
   {

      name         = "darray";
      signature    = "typeconstructor -> ARRAY" ;
      typeExample  = "darray int";
      listRep      =  "(a1 a2 a3)";
      valueExample = "(4 12 2 8)";
      remarks      = "A darray keeps all its element on remote systems";
   }
};


struct darrayFunctions : ConstructorFunctions<DArray> 
{

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

static ListExpr makeDarrayTypeMap( ListExpr args )
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

   ListExpr type = qp->GetType(s);
   ListExpr typeOfElement = sc->NumericType(nl->Second(type));
          

   int algID, typID;
   extractIds( typeOfElement, algID, typID);
         
    int size = qp->GetNoSons(s)-1;
        
   Word* cloned = new Word[size];
       
   //Objects need to be cloned to be persistent after the query ends
   for(int i = 0;i<size;i++)
   {                  
      cloned[i] = (am->CloneObj(algID,typID))(typeOfElement,args[i+1]);
   }
     
   //Generate serverlist as ListExpr from Relation
   GenericRelation* r = (GenericRelation*)args[0].addr;
   GenericRelationIterator* rit = r->MakeScan();
   ListExpr reltype;
   nl->ReadFromString("(rel (tuple ((Server string) (Port int))))",reltype);
   ListExpr serverlist = Relation::Out(reltype,rit);

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
       "<text>query makeDarray(server_rel,1,2,3)</text---> ))";

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
         
   SecondoCatalog* sc = SecondoSystem::GetCatalog();
   ListExpr resultType = qp->GetType(s);

   if (nl->ListLength(resultType) > 1) 
   {
      if (nl->IsEqual(nl->First(resultType), "map")) 
      {
         while (nl->ListLength(resultType) > 1) 
            resultType = nl->Rest(resultType);
        
         resultType = nl->First(resultType);
      }
   }
    
   resultType = sc->NumericType(resultType);
    
   int algID,typID; 
   extractIds(resultType,algID,typID);
    
   array->refresh(i);
   Word cloned = (am->CloneObj(algID,typID))(resultType,(Word)array->get(i));

   result = qp->ResultStorage(s);
   result.addr = cloned.addr;
    
   return 0;
}
         
const string getSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>darray t -> t</text--->"
       "<text>get ( _, _ )</text--->"
       "<text>Returns an element from a distributed Array</text--->"
       "<text>query get(makeDarray(server_rel,1,2,3),1)</text---> ))";

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
      Word* w = new Word[0];
      string to = ((DArray*)result.addr)->getName();
      w[0].addr = &to;
      
      if(j!=i)
      {
         ListExpr arg = nl->TwoElemList(nl->StringAtom(((DArray*)result.addr)
                                                               ->getName()),
                                                         nl->IntAtom(j));
         
         list<int>* l = new list<int>;
         l->push_front(j);
         
         
         array_alt->getServerManager()->getServerByIndex(j)->setCmd(
                                                   "copy",
                                                   l,
                                                   w);
         
         array_alt->getServerManager()->getServerByIndex(j)->run();
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
       "<text>query put(makeDarray(server_rel,1,2,3),2,2)</text---> ))";

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
          
          TypeConstructor* t = am->GetTC(algID,typID);
          Attribute* a;
          if(t->NumOfFLOBs() > 0 ) 
         a = static_cast<Attribute*>
            ((am->Cast(algID,typID))(args[2].addr));
               for(int i = 0; i < t->NumOfFLOBs(); i++)
         {
         Flob* f = a->GetFLOB(i);
         
         SmiSize si = f->getSize();
         int n_blocks = si / 1024 + 1;
         char* buf = new char[n_blocks*1024];
         memset(buf,0,1024*n_blocks);
         
         f->read(buf,si,0);
         
         iosock << "<FLOB>" << endl 
            << "<SIZE>" << endl << si << endl 
            << "</SIZE>" << endl;
         for(int j = 0; j<n_blocks;j++)
            master->Write(buf+j*1024,1024);
         iosock << "</FLOB>" << endl;
    delete buf;
         }
      
               iosock << "<CLOSE>" << endl;
         
                 
                   getline(iosock,line);
                   if(line!="<FINISH>") cout << "FEHLER";

                    result = qp->ResultStorage(s);

                    ((CcInt*)result.addr)->Set(0);

                    
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
     
     int algID, typID; extractIds(type,algID,typID);
      ;
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
                         Word w;
                         result = qp->ResultStorage(s);
                         am->OpenObj(algID,typID,rec,s0,type,w); 
      
                         result.addr = w.addr;
                         rec.Truncate(3);
                         recF.DeleteRecord(recID);
                         recF.Close();
          getline(iosock,line);
         
         
         int flobs = 0;    
         while(line=="<FLOB>")
         {
            getline(iosock,line);
            if(line!="<SIZE>") cout << "ERROR";
            getline(iosock,line);
            SmiSize si = atoi(line.data());
            getline(iosock,line);
            if(line!="</SIZE>") cout << "ERROR";
            
            int n_blocks = si / 1024 + 1;
            char* buf = new char[n_blocks*1024];
            memset(buf,0,1024*n_blocks);
            for(int i = 0; i< n_blocks; i++)
               iosock.read(buf+1024*i,1024);
            
            
            Attribute* a = static_cast<Attribute*>
               ((am->Cast(algID,typID))(result.addr));
            
            
            Flob*  f = a->GetFLOB(flobs);
            f->write(buf,si,0);
            
            delete buf;
            
            getline(iosock,line);
            if(line!="</FLOB>") cout << "ERROR";
            
            getline(iosock,line);
            flobs++;
            //receive FLOB
         }
         
         if(line!="<CLOSE>") cout << "ERROR";
                         
                         iosock << "<FINISH>" << endl;

                         
                    }}
                    
               }
               master->Close();delete master;master=0;
              
               return 0;
          }
     
     }
      cout << "FEHLERHAFTE AUSFÜHRUNG" << endl;
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
     
4.7 Operator d\_receive\_rel
     
*/

static ListExpr receiverelTypeMap( ListExpr args )
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
     
     int algID, typID; extractIds(type,algID,typID);
          
     cout << "Ende-Typemapping-Receive-Rel" << endl;
     cout << "Numerischer Typ !" << nl->ToString(type) << "!" << endl;
     cout << "Ergebnistyp !" << nl->ToString(convertType(type)) << "! " << endl;
      return nl->ThreeElemList(
          nl->SymbolAtom("APPEND"),
          nl->TwoElemList(nl->StringAtom(nl->ToString(nl->First(args))),
                              nl->StringAtom(nl->ToString(nl->Second(args)))),
                              //nl->StringAtom(nl->ToString(type))),
                              convertType(type));

}

static int receiverelFun( Word* args, 
                                    Word& result, 
                                    int message, 
                                    Word& local, 
                                    Supplier s)
{
     string host = (string)(char*)((CcString*)args[2].addr)->GetStringval();
     string port = (string)(char*)((CcString*)args[3].addr)->GetStringval();
     
     ListExpr resultType; 
     /*nl->ReadFromString((string)(char*)((CcString*)args[4].addr)
          ->GetStringval(),resultType);
     
     
     resultType = nl->Second(resultType);
     
     if(nl->ListLength(nl->First(nl->Second(resultType))) != 2)
          resultType = nl->TwoElemList(nl->First(resultType),
                                                  nl->OneElemList(
                                                  nl->Second(resultType)));
     
     cout << "Tuple-Typ-Input: " << nl->ToString(resultType) << endl;
     
     TupleType* tupleType = new TupleType(resultType);
     */

     string line;
     cout << "Beginn Value-Mapping-Receive-Rel" << endl;
     host = replaceAll(host,"_",".");
     host = replaceAll(host,"h","");
     port = replaceAll(port,"p","");

     Socket* master = Socket::Connect(host,port,Socket::SockGlobalDomain);
     
     result = qp->ResultStorage(s);
     
     GenericRelation* rel = (Relation*)result.addr;
     
     if(master!=0 && master->IsOk())
     {

          iostream& iosock = master->GetSocketStream();
          
          //iosock.read((char*)tupleType,sizeof(TupleType));
          
          string line;
          getline(iosock, line);
          
          if(line == "<TYPE>")
          {
               getline(iosock,line);
               nl->ReadFromString(line,resultType);
               resultType = nl->Second(resultType);
               
               cout << "ResultType: " << nl->ToString(resultType) << endl;
               
               TupleType* tupleType = new TupleType(resultType);
               getline(iosock,line);getline(iosock,line);
          char* buffer;
          while(line == "<TUPLE>")
          {
               getline(iosock,line);
               size_t size = atoi(line.data());
               
               int num_blocks = (size / 1024) + 1;
               getline(iosock,line);
               
               iosock << "<OK>" << endl << toString_d(num_blocks) 
               << endl << "</OK>" << endl;
               
               buffer = new char[1024*num_blocks];
               memset(buffer,0,1024*num_blocks);
               for(int i = 0; i<num_blocks; i++)
                    master->Read(buffer+i*1024,1024); 
                              
               Tuple* t = new Tuple(tupleType);
               
               
               t->ReadFromBin(buffer+sizeof(int),size);
               
               rel->AppendTuple(t);
               
                t->DeleteIfAllowed();
               
               getline(iosock,line);
          }
          
          if(line=="<CLOSE>")
          {
               iosock << "<FINISH>" << endl;
               master->Close(); delete master; master=0;
               return 0;
          }
          else
          {
               cout << "Fehlerhaftes Kommando Relation: " << line << endl;
               return 1;
          }}
     }
     
     else cout << "Fehler bei Verbindungsaufbau Relation";
     
     return 1;
     
}

const string receiverelSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>-</text--->"
       "<text>-</text--->"
       "<text>Internal Usage by DistributedAlgebra</text--->"
       "<text>-</text---> ))";

Operator receiverelA(
         "d_receive_rel",
         receiverelSpec,
         receiverelFun,
         Operator::SimpleSelect,
         receiverelTypeMap);


/*


4.8 Operator d\_send\_rel

Internal Usage for Data Transfer between Master and Worker

*/


static ListExpr sendrelTypeMap( ListExpr args )
{
     return nl->ThreeElemList(
                    nl->SymbolAtom("APPEND"),
                    nl->TwoElemList(nl->StringAtom(
                                               nl->ToString(nl->First(args))),
                                              nl->StringAtom(nl->ToString(
                                                  nl->Second(args)))),
                    nl->SymbolAtom("int"));

}

static int sendrelFun( Word* args, 
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
     
     char* buffer;
     if(master!=0 && master->IsOk())
     {

          iostream& iosock = master->GetSocketStream();
          GenericRelation* rel = (Relation*)args[2].addr;
          GenericRelationIterator* iter = rel->MakeScan();
          Tuple* t;
          
          while((t=iter->GetNextTuple()) != 0)
          {
               size_t cS,eS,fS;
               size_t size = t->GetBlockSize(cS,eS,fS);
               
               iosock << "<TUPLE>" << endl << toString_d(size) 
               << endl << "</TUPLE>" << endl;
               
               int num_blocks = (size / 1024) + 1;
               delete buffer;
               buffer = new char[num_blocks*1024];
               memset(buffer,0,num_blocks*1024);
               
               t->WriteToBin(buffer,cS,eS,fS);
               
               for(int i =0; i< num_blocks; i++)
                    iosock.write(buffer+i*1024,1024);
               
               t->DeleteIfAllowed();
          }
          
          iosock << "<CLOSE>" << endl;
          
          master->Close(); delete master; master=0;
          
          delete iter;
          
          result = qp->ResultStorage(s);
          ((CcInt*)result.addr)->Set(0);
          
          return 0;
     }
     
     result = qp->ResultStorage(s);
     ((CcInt*)result.addr)->Set(1);
     
     return 0;
     
}

const string sendrelSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>-</text--->"
       "<text>-</text--->"
       "<text>Internal Usage by DistributedAlgebra</text--->"
       "<text>-</text---> ))";

Operator sendrelA(
         "d_send_rel",
         sendrelSpec,
         sendrelFun,
         Operator::SimpleSelect,
         sendrelTypeMap);
          
          

/*


4.9 Operator distribute

*/


static ListExpr distributeTypeMap( ListExpr inargs )
{
     NList args(inargs);
     if( args.length() == 4)
     {
          NList stream_desc = args.first();
          ListExpr attr_desc = args.second().listExpr();
          if( stream_desc.isList() && stream_desc.first().isSymbol("stream")
               && (stream_desc.length() == 2)
               && (nl->AtomType(attr_desc) == SymbolType))
          {
               ListExpr tuple_desc = stream_desc.second().listExpr();
               string attr_name = nl->SymbolValue(attr_desc);
               
               if(nl->IsEqual(nl->First(tuple_desc),"tuple") &&
                    nl->ListLength(tuple_desc) == 2)
               {
                    ListExpr attrL = nl->Second(tuple_desc);
                    
                    if(IsTupleDescription(attrL))
                    {
                         int attrIndex;
                         ListExpr attrType;
                         
                         attrIndex = FindAttribute(attrL,attr_name,attrType);
                         
                         if(nl->ListLength(attrL > 1) && attrIndex > 0
                              && nl->IsEqual(attrType,"int"))
                         {
                              ListExpr attrL2 = nl->TheEmptyList();
                              ListExpr last;
                              
                              while(!nl->IsEmpty(attrL))
                              {
                                   ListExpr attr = nl->First(attrL);
                                   
                                   if(nl->SymbolValue(nl->First(attr)) 
                                        != attr_name)
                                   {
                                        if(nl->IsEmpty(attrL2)){
                                             attrL2 = nl->OneElemList(attr);
                                             last = attrL2;}
                                        else
                                             last = nl->Append(last,attr);
                                   }
                                   
                                   attrL = nl->Rest(attrL);
                              }
                              return nl->ThreeElemList(
                         nl->SymbolAtom("APPEND"),
                         nl->OneElemList(nl->IntAtom(attrIndex)),
                         nl->TwoElemList(
                           nl->SymbolAtom("darray"),
                           nl->TwoElemList(
                             nl->SymbolAtom("rel"),
                             nl->TwoElemList(nl->SymbolAtom("tuple"),
                                             attrL2))));
                         }
                    }
               }
          }
     }
     
     return args.typeError("input is not (stream(tuple(y))) x ...");
}

static int
distributeFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
     int size = ((CcInt*)(args[2].addr))->GetIntval();
     
     GenericRelation* r = (GenericRelation*)args[3].addr;
        GenericRelationIterator* rit = r->MakeScan();
         ListExpr reltype;
         nl->ReadFromString("(rel (tuple ((Server string) (Port int))))",
                                         reltype);
     ListExpr serverlist = Relation::Out(reltype,rit);
     
     int attrIndex = ((CcInt*)(args[4].addr))->GetIntval() - 1;
     
     SecondoCatalog* sc = SecondoSystem::GetCatalog();
     ListExpr restype = nl->Second(qp->GetType(s));
     restype = sc->NumericType(restype);

     
     DArray* array = (DArray*)(qp->ResultStorage(s)).addr;
     array->initialize(restype,getArrayName(DArray::no),
                                   size, serverlist);
     DServerManager* man = array->getServerManager();
     DServer* server = 0;
          
     int server_no = man->getNoOfServers();
     int rel_server = (size / server_no);
     
     for(int i = 0; i<server_no;i++)
     {
          server = man->getServerbyID(i);
          server->Multiply(rel_server);
     }

     
     for(int i = 0; i < size; i++)
     {
          server = man->getServerByIndex(i);
          int child = man->getMultipleServerIndex(i);
          if(child > -1)
               server = (server->getChilds())[child];
          
          list<int>* l = new list<int>;
          l->push_front(i);
          
          server->setCmd("open_write_rel",l,0);
          server->run();
     }
     

     int number = 0;               
     
     ListExpr tupleType = nl->Second(restype);
     Word current = SetWord( Address (0) );
     
     qp->Open(args[0].addr);
     qp->Request(args[0].addr,current);
     
     ZThread::ThreadedExecutor ex;
     
     while(qp->Received(args[0].addr))
     {
          Tuple* tuple1 = (Tuple*)current.addr;
          Tuple* tuple2 = new Tuple(tupleType);

          
          int j = 0;
          for(int i = 0; i < tuple1->GetNoAttributes(); i++)
          {
               if(i != attrIndex)
                    tuple2->CopyAttribute(i,tuple1,j++);
          }

          int index = ((CcInt*)(tuple1->GetAttribute(attrIndex)))->GetIntval();
          tuple1->DeleteIfAllowed();
          
          index = index % size;
          int child = man->getMultipleServerIndex(index);
          server = man->getServerByIndex(index);
          if(child > -1) server = (server->getChilds())[child];
          
          //current = SetWord(tuple2);
          Word* w = new Word(1);
          w[0] = SetWord(tuple2);tuple2->IncReference();

          while(server->status != 0) ZThread::Thread::yield();
          server->status = 1;
          server->setCmd("write_rel",0,w);
          DServerExecutor* exec = new DServerExecutor(server);
          ex.execute(exec);
          //server->run();

          tuple2->DeleteIfAllowed();
          
          qp->Request(args[0].addr,current);
          
          number++; cout << toString_d(number) << " Tuple verarbeitet" << endl;
     }
     
     ex.wait();     
     
     for(int i = 0; i < size; i++)
     {
          server = man->getServerByIndex(i);
          int child = man->getMultipleServerIndex(i);
          if(child > -1)
               server = (server->getChilds())[child];
          
          server->setCmd("close_write_rel",0,0);
          server->run();
     }
     
     for(int i = 0; i<server_no;i++)
     {
          server = man->getServerbyID(i);
          server->DestroyChilds();
     }
     
     
     result.addr = array;
     return 0;
        
     
}
     
     
     
     

const string distributeSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>((stream (tuple ((x1 t1) ... (xn tn)))) xi j) -> "
     "(darray (rel (tuple ((x1 t1) ... (xi-1 ti-1)" 
     "(xi+1 ti+1) ... (xn tn)))))</text--->"
      "<text>_ ddistribute [ _ , _ ]</text--->"
      "<text>Distributes a stream of tuples" 
     "into a darray of relations.</text--->"
      "<text>let prel = plz feed distribute [pkg]</text---> ))";

Operator distributeA (
      "ddistribute",
      distributeSpec,
      distributeFun,
      Operator::SimpleSelect,
      distributeTypeMap );

/*

4.10 Operator loop

*/

static ListExpr loopTypeMap(ListExpr args)
{
     if(nl->ListLength(args) == 3)
     {
          ListExpr array = nl->First(args);
          ListExpr map = nl->Second(args);
          
          if(nl->ListLength(array) == 2 &&
               nl->ListLength(map) == 3 &&
               nl->IsEqual(nl->Third(args),"text"))
          {
               if(nl->IsEqual(nl->First(array),"darray") &&
                    nl->IsEqual(nl->First(map),"map") &&
                    !nl->IsEqual(nl->Third(map),"typeerror"))
               {
                    if(nl->Equal(nl->Second(array),nl->Second(map)))
                         return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
                    nl->TwoElemList(nl->SymbolAtom("darray"),
                                                        nl->Third(map)),
                                nl->TwoElemList(nl->SymbolAtom("darray"),
                                                        nl->Third(map)));
               }
          }
     }
     
     return nl->SymbolAtom("typeerror");
}

static int loopValueMap
(Word* args, Word& result, int message, Word& local, Supplier s)
{
     DArray* alt = (DArray*)args[0].addr;
     
     result = qp->ResultStorage(s);
     
     SecondoCatalog* sc = SecondoSystem::GetCatalog();
     ListExpr type = sc->NumericType(nl->Second((qp->GetType(s))));
     ((DArray*)(result.addr))->initialize(type,
                                                getArrayName(DArray::no),
                                               alt->getSize(),
                                               alt->getServerList());
     
     string command = ((FText*)args[2].addr)->GetValue();
     
     ZThread::ThreadedExecutor exec;DServer* server;
     DServerExecutor* ex;
     Word* w = new Word[2];
     string to = ((DArray*)(result.addr))->getName();
     w[0].addr = &to;
     w[1].addr = &command;
     for(int i=0; i < alt->getServerManager()->getNoOfServers(); i++)
     {
          server = alt->getServerManager()->getServerbyID(i);
          /*ListExpr param = nl->TwoElemList(nl->TwoElemList(
                                        nl->StringAtom(((DArray*)(result.addr))
                                                  ->getName()),
                                        nl->StringAtom(command)),
                                   alt->getServerManager()->getIndexList(i));*/
          server->setCmd("execute",alt->getServerManager()->getIndexList(i),w);
          
          ex = new DServerExecutor(server);
          exec.execute(ex);
     }
     
     exec.wait();
     
     return 0;
     
}

const string loopSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>((stream (tuple ((x1 t1) ... (xn tn)))) xi j) -> "
     "(darray (rel (tuple ((x1 t1) ... (xi-1 ti-1)" 
     "(xi+1 ti+1) ... (xn tn)))))</text--->"
      "<text>_ ddistribute [ _ , _ ]</text--->"
      "<text>Distributes a stream of tuples" 
     "into a darray of relations.</text--->"
      "<text>let prel = plz feed distribute [pkg]</text---> ))";


Operator loopA (
      "dloop",
      loopSpec,
      loopValueMap,
      Operator::SimpleSelect,
      loopTypeMap );

/*

4.11 Type Operator DELEMENT (derived from Operator ELEMENT of the ArrayAlgebra

*/

ListExpr delementTypeMap( ListExpr args )
{
  if(nl->ListLength(args) >= 1)
  {
    ListExpr first = nl->First(args);
    if (nl->ListLength(first) == 2)
    {
      if (nl->IsEqual(nl->First(first), "darray")) {
        return nl->Second(first);
      }
    }
  }
  return nl->SymbolAtom("typeerror");
}

const string DELEMENTSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Remarks\" )"
    "( <text>((array t) ... ) -> t</text--->"
      "<text>type operator</text--->"
      "<text>Extracts the type of the elements from a darray type given "
      "as the first argument.</text--->"
      "<text>not for use with sos-syntax</text---> ))";

Operator dElementA (
      "DELEMENT",
      DELEMENTSpec,
      0,
      Operator::SimpleSelect,
      delementTypeMap );


/* 

5 Creating the Algebra 

*/



class DistributedAlgebra : public Algebra
{
  public:
    DistributedAlgebra() : Algebra()
    {
             
             AddTypeConstructor( &darrayTC );
             darrayTC.AssociateKind("ARRAY");
             AddOperator( &makeDarray );
             AddOperator( &getA );
             AddOperator( &putA );
             AddOperator( &sendA );
             AddOperator( &receiveA);
             AddOperator( &receiverelA);
             AddOperator( &sendrelA);
             AddOperator( &distributeA);
             AddOperator( &loopA);
          AddOperator( &dElementA);
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


