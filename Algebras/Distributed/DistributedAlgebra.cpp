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
Foundation, Inc., 59 Temple Place, Suite 330, 
Boston, MA  02111-1307  USA
----

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[_] [\_]
//[&] [\&]
//[x] [\ensuremath{\times}]
//[->] [\ensuremath{\rightarrow}]
//[>] [\ensuremath{>}]
//[<] [\ensuremath{<}]
//[ast] [\ensuremath{\ast}]

*/

/*
[1] DistributedAlgebra

April 2012 Thomas Achmann

November 2010 Tobias Timmerscheidt

This algebra implements a distributed array. This type of array
keeps its element on remote servers, called worker. Upon creation
of the array all elements are transfered to the respective workers.
The list of workers must be specified in terms of a relation in any
operator that gives back a darray.
Operations on the darray-elements are carried out on the remote machines.



1. Preliminaries

1.1 Includes

*/

#include "Algebra.h"
#include "NestedList.h"
#include "NList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "SecondoCatalog.h"
#include "RelationAlgebra.h"
#include "TypeMapUtils.h"
#include "Remote.h"
#include "DSecondoMonitorComm.h"
#include "DServer.h"
#include "DServerManager.h"
#include "DServerCmdCopy.h"
#include "DServerCmdDelete.h"
#include "DServerCmdExecute.h"
#include "DServerCmdRead.h"
#include "DServerCmdReadRel.h"
#include "DServerCmdWrite.h"
#include "DServerCmdWriteRel.h"
#include "DServerCmdShuffleRec.h"
#include "DServerCmdShuffleSend.h"
#include "DServerCmdShuffleMultipleConn.h"
#include "DServerShuffleSender.h"
#include "DServerShuffleReceiver.h"
#include "DServerCmdCallBackComm.h"
#include "zthread/ThreadedExecutor.h"
#include "zthread/Mutex.h"
#include "../FText/FTextAlgebra.h"
#include "../Array/ArrayAlgebra.h"
#include "DistributedAlgebra.h"
#include "DBAccessGuard.h"
#include "StringUtils.h"
#include "Symbols.h"
#include "Stream.h"
#include "ThreadedMemoryCntr.h"
#include <sys/timeb.h>

using namespace std;
using namespace mappings;

extern NestedList* nl;
extern QueryProcessor *qp;


/*

3 Auxiliary Functions

*/

//Uses Function from ArrayAlgebra
void extractIds(const ListExpr,int&,int&);


//Converts int to string
string toString_ul(unsigned long i)
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
  struct timeb t1;
  ftime(&t1);
  unsigned long tm = (t1.time * 1000) + t1.millitm;
  string t = toString_ul(tm) + int2Str(number);
  return t;
}

//Converts a pair (algID typID) to the corresponding
//type name (used by converType)
ListExpr convertSingleType( ListExpr type)
{
   if(nl->ListLength(type) != 2) 
     return nl->SymbolAtom("ERROR");
   if(!nl->IsAtom(nl->First(type)) || !nl->IsAtom(nl->Second(type)))
     return nl->SymbolAtom("ERROR");

   int algID, typID;

   extractIds(type,algID,typID);

   SecondoCatalog* sc = SecondoSystem::GetCatalog();
   if(algID < 0 || typID < 0) return nl->SymbolAtom("ERROR");

   return nl->SymbolAtom(sc->GetTypeName(algID,typID));
}


//Converts a numerical type to its text representation
ListExpr convertType( ListExpr type, bool isOneElList = false)
{
   ListExpr result,result2;

     //Is it not a type expression but an attribute name?
   if(nl->ListLength(type) < 2)
   {
      if(nl->IsAtom(type) || nl->IsEmpty(type)) 
        {
          return type;
        }
      //Only one element that is not atomic
      else 
        {
          result = convertType(nl->First(type), true);
          if (isOneElList)
            result = nl -> OneElemList(result);
          return result;
        }
   }

     //Single type expression
   if(nl->ListLength(type) == 2 &&
         nl->IsAtom(nl->First(type)) &&
         nl->IsAtom(nl->Second(type)))
     {
       result = convertSingleType(type);
       return result;
     }

   //It's a list with more than three elements, proceed recursively
   result = convertType(nl->First(type), false);
   result2 = convertType(nl->Rest(type), false);
   
   if(nl->ListLength(type) == 2)
     {
       return nl->TwoElemList(result,result2);
     }
   else
     {
       return nl->Cons(result,result2);
     }
}

static bool RunCmdPopen(const string& inCmd,
                         string &outResult)
{
  //cout << "RunCmdPopen:" << inCmd << endl;

  bool ret_val = true;
  FILE *fs;
  char qBuf[1024];
  memset(qBuf, '\0', sizeof(qBuf));
  fs = popen(inCmd.c_str(), "r");

  if (fs == NULL)
    {
      perror(("ERROR: Cannot start Command:" + inCmd).c_str());
      ret_val = false;
    }
  else if (fgets(qBuf, sizeof(qBuf), fs) != NULL)
    {
      outResult = string(qBuf);
      pclose(fs);
    }
  else
    {
      pclose(fs);
    }

  //cout << "Popen Res:" << ret_val << outResult << endl;

  return ret_val;
}

static bool RunCmdSSH(const string& inHost,
                      const string& inCmd,
                      string &outResult)
{
  string cmd;
  bool retval = false;
  if (inHost != "localhost" &&
      inHost != "127.0.0.")
    {
      string tmpCmd = stringutils::replaceAll(inCmd,"$","\\$");
      cmd = "ssh " + inHost + " \"bash -c '" + tmpCmd + "'\"";  
    }
  else
    {
      cmd = "bash -c 'cd ${HOME}; " + inCmd + "'";
    }
     
  retval = RunCmdPopen(cmd, outResult);
  
  return retval;
}

/*

4. Type Constructor ~DArray~

4.2 Implementation of basic functions

*/

//Definition of static variable
int DArray::no = 0;

//Creates an undefined DArray
DArray::DArray(bool isDefined)
  : m_name()
{
  //cout << "C-TOR1" << endl;
   SetDefined(isDefined);
   setNoRelType();
   m_size=0;
   alg_id=0;
   typ_id=0;
   no++; 
   m_serverlist = nl -> TheEmptyList();
   m_serverManager = NULL;
   //   m_watch.start();
   //cout << "DArray::DArray():" << no << endl;
}

//Creates a defined DArray
//A DArray is defined by its name, type, size and serverlist
//It can be defined while all its elements are undefined
DArray::DArray(ListExpr inType, 
               const string& inName, int s, ListExpr inServerlist)
  : m_name(inName)
{
  //cout << "C-TOR2" << endl;
   m_type = inType;

   extractIds( m_type, alg_id, typ_id);

   SecondoCatalog* sc = SecondoSystem::GetCatalog();
   if(sc->GetTypeName(alg_id,typ_id) == Relation::BasicType())
     setRelType();
   else
     setNoRelType();


   m_size = s;

   m_elements = vector<Word>(getSize());
   m_present = vector<bool>(getSize(), false);

   m_serverlist = inServerlist;
   m_serverManager = 
     new DServerManager(m_serverlist, m_name,m_type,getSize());

   no++;

   if (!(m_serverManager -> isOk()))
     SetUndefined();
   else
     SetDefined(true);
   //cout << "DArray::DArray(...):" << no << endl;
}



DArray::~DArray()
{
  //cout << "DArray::~DArray:" << no << endl;
  //cout <<"Time:" << m_watch.diffTimes() << endl;
  assert(no > 0);
  no--;
  //if(IsDefined())
   {
     for(int i=0;i< getSize();i++)
      {
         //Elements that are present on the master are deleted
         //note that this deletes NOT the Secondo-objects on the 
         //workers.
         //they need to be deleted seperately which can be done by
         //the remove-function
        if(m_present[i])
          {
            //cout << "DEL EL: " << i << endl;
            
            (am->DeleteObj(alg_id,typ_id))(m_type,m_elements[i]);
          }
      }
      m_present.clear();
      m_elements.clear();
   }

  delete m_serverManager;
  m_serverManager = NULL;
}

void DArray::remove()
{
  //cout << "DArray::remove()" << endl;
  if(getServerManager() != NULL )
   {

     DServerCmdDeleteParam delParam;

     ZThread::ThreadedExecutor exec;

     bool ret_val = runCommandThreaded<DServerCmdDelete, 
                                        DServerCmdDeleteParam>(exec,
                                                               delParam);

     const int nrOfWorkers = getServerManager()->getNoOfUsedWorkers(getSize());
    
     for(int i= 0;i<nrOfWorkers;i++)
       {
         if (getServerManager()->
             getServerByID(i) -> 
             hasError())
           {
             getServerManager() -> 
               setErrorText(string("Error Worker " +
                                   int2Str( i ) + ": " +
                                   getServerManager()->getServerByID(i)
                                   -> getErrorText()));
             cerr << "ERROR: Deleting DArray!" << endl;
             cerr << getServerManager() -> getErrorText() << endl;
             SetUndefined();
             return;
           }
       } 
      
     if (!ret_val)
       {
         cerr << "ERROR: Could not delete data from Worker:" << endl;
       }
       
   }
  //cout << "DArray::remove() ... done" << endl;
}


const string&  
DArray::getHostNameByIndex(int inIdx)
{
  DServer* server = getServerManager()->getServerByIndex(inIdx);
  return server -> getServerHostName();
}


void DArray::refresh(int i)
{ 
  bool ret_val = true;

  if(m_present[i])
    (am->DeleteObj(alg_id,typ_id))(m_type,m_elements[i]);

  if(isRelType())
    {
      m_elements[i].addr = 
        (am->CreateObj(alg_id,typ_id))(m_type).addr;

      DServerCmdReadRelParam readRelParam(&m_elements, 
                                          &m_present,
                                          nl -> Second(getType()));
 
      ret_val = runCommand<DServerCmdReadRel, 
                           DServerCmdReadRelParam>(readRelParam, i);

    }
  else
    {
      DServerCmdReadParam readParam(&m_elements, &m_present, getType());

      ret_val = runCommand<DServerCmdRead, 
                           DServerCmdReadParam>(readParam, i);
    }

  DServer* server = getServerManager() -> getServerByIndex(i);
  if (!ret_val || server -> hasError())
    {
      cerr << "ERROR: Could not get data from Worker:" << endl;
      cerr << server -> getErrorText() << endl;
      m_present[i] = false;
    }
     
}

void DArray::refresh()
{
  //cout << "DArray::refresh S:" << getSize() 
  //     << " (Rel:" << isRelType() << ")" << endl
  //     << " T:" << nl -> ToString(getType()) << endl;
 
   ZThread::ThreadedExecutor exec;

   //Elements are deleted if they were present
   //If the darray has a relation-type new relations must be created
   assert(m_elements.size() == (unsigned int)getSize());
  
   for (int i = 0; i <  getSize(); ++i)
     { 
       if(m_present[i])
         {
           (am->DeleteObj(alg_id,typ_id))(m_type,m_elements[i]);
         }
       
       if(isRelType())
         {
           m_elements[i].addr = 
             (am->CreateObj(alg_id,typ_id))(m_type).addr;
         }
       
     }
   
   bool success = true;

   if (isRelType())
     {
        DServerCmdReadRelParam readRelParam(&m_elements, 
                                            &m_present,
                                            nl -> Second(getType()));
 
        success = runCommandThreaded<DServerCmdReadRel, 
                                DServerCmdReadRelParam>(exec, 
                                                        readRelParam);
     
     }
   else
     {
       DServerCmdReadParam readParam(&m_elements,
                                     &m_present, getType());

       success = runCommandThreaded<DServerCmdRead, 
                                    DServerCmdReadParam>(exec, 
                                                         readParam);
     }
   
     //All elements are present now
   //cout << "DArray::refresh ...done" << endl;
}

void DArray::refresh(TFQ tfqOut, ThreadedMemoryCounter *inMemCntr)
{
  if (!(getServerManager() -> checkServers(true)))
    {
      SetUndefined();
      return;
    }

   ZThread::ThreadedExecutor exec;

   //Elements are deleted if they were present
   //If the darray has a relation-type new relations must be created
   assert(m_elements.size() == (unsigned int)getSize());
   assert(isRelType());

   DServerCmdReadRelParam readRelParam(tfqOut, inMemCntr,
                                          nl -> Second(getType()));
   
   bool success = 
     runCommandThreaded<DServerCmdReadRel, 
                        DServerCmdReadRelParam>(exec, 
                                                readRelParam);
     
   //tfqRefreshDone();

   //dummy element to release collector thread
   tfqOut -> put(NULL);
}

bool DArray::initialize(ListExpr inType,
                        const string& inName, int s,
                        ListExpr inServerlist,
                        const vector<Word> &n_elem)
{
  //cout << "INIT2" << endl;
  // initializes an undefined array, all 
  // elements are present on the master
  SetDefined(true);
   m_type = inType;

   extractIds( m_type , alg_id, typ_id);

   SecondoCatalog* sc = SecondoSystem::GetCatalog();

   //check if type is relation-type
   if(sc->GetTypeName(alg_id,typ_id) == Relation::BasicType())
     {
       setRelType();
     }
   else
     {
       setNoRelType();
     }

   m_name = inName;
   m_size = s;
   m_serverlist = inServerlist;

   //create DServerManager, and
   //thereby also the DServer (worker connections)
   m_serverManager = 
     new DServerManager(m_serverlist, m_name, m_type, getSize());

   //Set the elements-array
   m_elements = n_elem;
   m_present = vector<bool>(getSize(), true); // all true

   const int max_servers = 
     getServerManager()->getNoOfUsedWorkers(getSize());

   // the elements to the respective workers
   ZThread::ThreadedExecutor exec;

   bool success = true;

   if(!isRelType())
     {
      
       DServerCmdWriteParam writeParam(&m_elements);

       success = runCommandThreaded<DServerCmdWrite, 
                                    DServerCmdWriteParam>(exec, 
                                                          writeParam);
     }
   else
     {
       DServerCmdWriteRelParam writeRelParam(&m_elements);

       success = runCommandThreaded<DServerCmdWriteRel, 
                                    DServerCmdWriteRelParam>(exec, 
                                                             writeRelParam);
     }
   for(int i= 0;i<max_servers;i++)
     {
       if (getServerManager()->
             getServerByID(i) -> 
               hasError())
         {
           getServerManager() -> 
             setErrorText(string("Error Worker " +
                                 int2Str( i ) + ": " +
                                 getServerManager()->getServerByID(i)
                                 -> getErrorText()));
           cerr << "ERROR: Initialize DArray!" << endl;
           cerr << getServerManager() -> getErrorText() << endl;
           SetUndefined();
           return false;
         }
     }
  
   //cout <<"Init1:" << m_watch.diffTimes() << endl;
   return true;
}

bool DArray::initialize(ListExpr inType, 
                        const string& inName,
                        int s,
                        ListExpr inServerlist)
{
  //cout << "INIT1" << endl;
   //initializes an undefined, no elements are given
   //all elements must already exist on the workers
  if (nl -> IsEmpty(inServerlist))
    {
      SetDefined(false);
      return false;
    }
   SetDefined(true);
   m_type = inType;

   extractIds( m_type , alg_id, typ_id);

   SecondoCatalog* sc = SecondoSystem::GetCatalog();

   //ceck whether array of relation?
   if(sc->GetTypeName(alg_id,typ_id) == Relation::BasicType())
     {
       setRelType(); 
     }
   else
     setNoRelType();

   m_name = inName;
   m_size = s;

   //elements-array is empty, no elements are present on the master
   m_elements = vector<Word>(getSize());
   m_present = vector<bool>(getSize(), false); // all false!;

   //creates DServerManager, which creates the
   //DServer-objects for all workers
   m_serverlist = inServerlist;
   m_serverManager = 
     new DServerManager(m_serverlist, m_name,m_type,getSize());
   //cout <<"Init2:" << m_watch.diffTimes() << endl;
   return true;
}


const Word& DArray::get(int i)
{
   //returns an element of the elements-array
  if(IsDefined() && i >=0 && i < getSize())
    {
      if (!m_present[i])
        refresh(i);
      
      if (!m_present[i])
        {
          SetUndefined();
          cerr << "ERROR: Could not retrieve Element " << i << endl;
         
          return DServer::ms_emptyWord;
        }

      return m_elements[i];
    }
  // else:

  cerr << "ERROR: ";
  if (!IsDefined())
    cerr << "Array is not defined!!";
  else if ( i < 0 || i >= getSize())
    cerr << "Index out of scope!!";
  else
    cerr << "Element is not present!!";
  
  cerr << " (Index: " << i << ")" << endl;
  return DServer::ms_emptyWord;
  
}

void DArray::set(Word n_elem, int i)
{
  //sets an element of the array
  //the element is subsequently written to the corresponding worker
  vector<int> l;
  l.push_back(i);

  if(IsDefined())
    {
      m_elements[i].addr = n_elem.addr;
      bool success = false;

      if(!isRelType())
        {
          DServerCmdWriteParam writeParam(&m_elements);

          success = runCommand<DServerCmdWrite, 
                                DServerCmdWriteParam>(writeParam, i);
        }
      else
        {
          DServerCmdWriteRelParam writeRelParam(&m_elements);

          success = runCommand<DServerCmdWriteRel, 
                                DServerCmdWriteRelParam>(writeRelParam, i);
        }

      if (!success)
        {
          cerr << "ERROR: Could not send data to Worker:" << endl;
          cerr << getServerManager()->getServerByIndex(i) -> getErrorText() 
               << endl;
          m_present[i] = false;
          SetUndefined();
          return;
        }

    }

  //This element is now present on the master
  m_present[i] = true; //true
}




/*

4.3 In and Out functions

*/

Word DArray::In( const ListExpr inTypeInfo, const ListExpr instance,
                        const int errorPos, ListExpr& errorInfo,
                        bool& correct )
{
  //cout << "DArray::In" << endl;
   Word e;
   int algID, typID;

   extractIds(nl->Second(inTypeInfo),algID,typID);
   DArray* a = new DArray(nl->Second(inTypeInfo),
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
         (nl->Second(inTypeInfo),
          element,errorPos,errorInfo,correct));

      a->set(e,i);
      i++;
   }
   while(!nl->IsEmpty(listOfElements) && correct);

   if(correct)
   {
     //cout << " -> OK" << endl;
      return SetWord(a);
   }
   a -> remove();
   delete a;

   //cout << "-> Error!" << endl;
   correct=false;
   return SetWord(Address(0));

}


ListExpr DArray::Out( ListExpr inTypeInfo, Word value )
{
  //cout << "DArray::Out" << endl;
   DArray* a = (DArray*)value.addr;

   if(a == NULL || !(a->IsDefined()))
     {
       //cout << " -> ERROR!" << endl;
       return nl->SymbolAtom(Symbol::UNDEFINED());
     }
   if( a -> getServerManager() == NULL ||
       !(a -> getServerManager() -> checkServers(true)))
     {
       return nl->SymbolAtom(Symbol::UNDEFINED());
     }
   ListExpr list;
   ListExpr last;
   ListExpr element;
   list = nl->OneElemList(a->getServerList());
   last=list;

   a->refresh();

   if(a->IsDefined())
     {
       for(int i = 0; i<a->getSize();i++)
       {
         Word org = (Word)a->get(i);
         
         if(a->IsDefined())
           {
             element = ((am->OutObj(a->getAlgID(),a->getTypID()))
                        (nl->Second(inTypeInfo),
                         org));

             last=nl->Append(last,element);
           }
         else
           {
             assert(0);
             return nl->SymbolAtom(Symbol::UNDEFINED());
           }
       } 
     }

   return list;
}


/*

4.4 Persistent Storage Functions

*/


Word DArray::Create( const ListExpr inTypeInfo )
{
  //cout << "DArray::Create" << endl;
  return SetWord(new DArray());
}


//Note: The Delete-function deletes the 
//entire DArray, therefore the remote
//obejcts are delete and thereafter the local 
//data structure. The Close-function only 
//removes the local data structure

void DArray::Delete( const ListExpr inTypeInfo, Word& w )
{
  //cout << "DArray::Delete" << endl;
  DArray *da = (DArray*)w.addr;
  if (da != NULL)
    {
      ((DArray*)w.addr)->remove();
      delete (DArray*)w.addr;
    }
   w.addr = 0;
   //cout << " -> OK" << endl;
}

void DArray::Close( const ListExpr inTypeInfo, Word& w )
{
  //cout << "DArray::Close" << endl;
  DArray *da = (DArray*)w.addr;
  if (da != 0)
    {
      delete (DArray*)w.addr;
    }
   w.addr = 0;
   //cout << " -> OK" << endl;
}

//A new DArray is created locally with the 
//same type, size and serverlist. It is given a 
//new name and all the elements of the old 
//array are copied on the workers
Word DArray::Clone( const ListExpr inTypeInfo, const Word& w )
{
  //cout << "DArray::Clone" << endl;
   DArray* alt = (DArray*)w.addr;
   DArray* neu;

   neu = new DArray(nl->Second(inTypeInfo),
                getArrayName(DArray::no),
                alt->getSize(),
                alt->getServerList());

   DServerCmdCopyParam param (neu -> getName(), -1 );

   ZThread::ThreadedExecutor poolEx;
   if (alt -> runCommandThreaded <DServerCmdCopy, 
                                  DServerCmdCopyParam> (poolEx, param) )
     {
       //cout << " -> OK" << endl;
       return SetWord(neu);
     }
   else
     {
       if (alt -> hasError())
         {
           cerr << "ERROR:" << alt -> getErrorText() << endl;
         }
       else
         cerr << "ERROR: Cannot copy darray!" << endl;
     }

   return SetWord(new DArray (false) );
    
}


bool DArray::Open( SmiRecord& valueRecord ,
               size_t& offset ,
               const ListExpr inTypeInfo ,
               Word& value )
{
  //cout << "DArray::Open" << endl;
   char* buffer;
   string name, type, server;
   int length;
   int size;
   int da_revision;
   bool is_defined = true;

   //Size of the array is read
   valueRecord.Read(&da_revision,sizeof(int),offset);
   offset+=sizeof(int);

   if (da_revision == -1)
     {
       //cout << "READING NEW FORMAT!" << endl;
       // this is new fromat as of March 2012
       // format March 2012: size: -1

       // reading defined / not defined
       int tmp = 0;
       valueRecord.Read(&tmp,sizeof(int),offset);
       offset+=sizeof(int);
       
       if (tmp == 0)
         is_defined = false;

       //Size of the array is read
       valueRecord.Read(&size,sizeof(int),offset);
       offset+=sizeof(int);

     }
   else
     { 
       //cout << "READING OLD FORMAT!" << endl;
       // old format:
       size = da_revision;
     }

   //type-expression (string) is read
   valueRecord.Read(&length, sizeof(length), offset);
   offset += sizeof(length);
   buffer = new char[length];
   valueRecord.Read(buffer, length, offset);
   offset += length;
   type.assign(buffer, length);
   delete [] buffer;

   //Workerlist is read
   valueRecord.Read(&length, sizeof(length), offset);
   offset += sizeof(length);
   buffer = new char[length];
   valueRecord.Read(buffer, length, offset);
   offset += length;
   server.assign(buffer, length);
   delete [] buffer;

   //name is read
   valueRecord.Read(&length, sizeof(length), offset);
   offset += sizeof(length);
   buffer = new char[length];
   valueRecord.Read(buffer, length, offset);
   offset += length;
   name.assign(buffer, length);
   delete [] buffer;

   ListExpr typeList;
   nl->ReadFromString( type, typeList);

   ListExpr serverlist = nl -> TheEmptyList();
   DArray *da = NULL;
   if (server.length() > 0 && server != "()")
     {
       nl->ReadFromString(server,serverlist);
       da = new DArray(typeList,name,size,serverlist);
     }
   else
     da = new DArray(false);

   if (!is_defined)
     da -> SetUndefined();

   value.addr = ((Word)da).addr;

   //cout << " -> OK" << endl;

   return true;
}

bool DArray::Save( SmiRecord& valueRecord ,
               size_t& offset ,
               const ListExpr inTypeInfo ,
               Word& value )
{
   int length;
   DArray*  darray = ((DArray*)value.addr);
   assert (darray != NULL);

   // This is using a new fromat as of March 2012.
   // Format March 2012: revision = -1
   // all new formats must have negative numbers!
   // since zero value and positive values are used 
   // by the original format
   int da_revision = -1;

   //current format revision is saved
   valueRecord.Write(&da_revision, sizeof(int),offset);
   offset+=sizeof(int);

   //defined / not defined is saved
   int defined  = darray->IsDefined() ? 1 : 0;
   valueRecord.Write(&defined, sizeof(int),offset);
   offset+=sizeof(int);

   //Size of the array is saved
   int size = darray->getSize();
   valueRecord.Write(&size, sizeof(int),offset);
   offset+=sizeof(int);

   //element-type of the array is saved
   string type;
   nl->WriteToString( type, nl->Second(inTypeInfo) );
   length = type.length();
   valueRecord.Write( &length, sizeof(length), offset);
   offset += sizeof(length);
   valueRecord.Write ( type.data(), length, offset);
   offset += length;


   //Workerlist of the array is saved
   string server;
   nl->WriteToString( server, darray->getServerList());
   length = server.length();
   valueRecord.Write( &length, sizeof(length), offset);
   offset += sizeof(length);
   valueRecord.Write ( server.data(), length, offset);
   offset += length;

   //Name of the array is saved
   string name = darray->getName();
   length = name.length();
   valueRecord.Write( &length, sizeof(length), offset);
   offset += sizeof(length);
   valueRecord.Write ( name.data(), length, offset);
   offset += length;

   return true;
}

/*

4.5 Kind Checking

*/

bool DArray::KindCheck( ListExpr inType, ListExpr& errorInfo )
{
   if(nl->ListLength(inType) == 2)
   {
      if(nl->IsEqual(nl->First(inType),DArray::BasicType()))
      {

         SecondoCatalog* sc = SecondoSystem::GetCatalog();

         if(sc->KindCorrect(nl->Second(inType),errorInfo))
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

bool 
DArray::hasError() const
{
  if (getServerManager() != NULL)
    {
      for(int i =0; i < getSize();i++)
        if (getServerManager()->getServerByIndex(i) -> hasError())
          return true;
    }
  return false;
}

string 
DArray::getErrorText() const
{
  if (getServerManager() != NULL)
    {
      for(int i =0; i < getSize();i++)
        if (getServerManager()->getServerByIndex(i) -> hasError())
          return getServerManager()->
            getServerByIndex(i) -> getErrorText();
    }
  return string();
}

bool DArray::destroyAnyChilds()
{
  bool ret_val = true;
  if (getServerManager() != NULL)
    for(int k = 0; k < getServerManager() -> getNoOfAllWorkers(); k++)
      if (getServerManager() ->getServerByID(k) != NULL)
        getServerManager() ->getServerByID(k)->DestroyChilds();

  return ret_val;
}

bool 
DArray::multiplyWorkers(vector<DServer*>* outServerList,
                        bool startChilds)
{ 
  bool ret_val = true;

  DServerManager* man = getServerManager();
  DServer* server = 0;

  if (man == NULL || !(man -> isOk()))
    {
      cerr << "Cannot start workers!" << endl;
      ret_val = false;
    }
  else
    {
      int server_no = man -> getNoOfMultiWorkers(getSize());
      int rel_server = man -> getRelativeNrOfChildsPerWorker(getSize());
      int workerWithMoreChilds = 
        man -> getNrOfWorkersWithMoreChilds(getSize());
      if (startChilds)
        {
          try
            {

              ZThread::ThreadedExecutor threadEx;
              cout << "Multiplying worker connections... " << endl;

       
              for(int i = 0; i<server_no;i++)
                {
                  int childCnt = rel_server;
                  if (i < workerWithMoreChilds)
                    childCnt ++;
                  server = man->getServerByID(i);
                  threadEx.execute(new DServerMultiplyer(server,
                                                         childCnt));
                }
              threadEx.wait();
            }
          catch(ZThread::Synchronization_Exception& e)
            {
              cerr << "Could not multiply  DServers!" << endl;
              cerr << e.what() << endl;
              ret_val = false;
            }
        }

      
      for(int i = 0; ret_val && i < getSize(); i++)
        {
          server = man->getServerByIndex(i);
          int child = man->getMultipleServerIndex(i);

          if(child > -1)
            {
              assert((unsigned int)child < 
                     server->getChilds().size());
              server = (server->getChilds())[child];
            }

          if (!(server -> checkServer(true)) ||
              server -> hasError())
            {
              cerr << "ERROR: Multiplying darrays" << endl;
              cerr << server -> getErrorText() << endl;
              ret_val = false;
            }

          if (ret_val && outServerList != NULL)
            (*outServerList)[i] = server;
        }
    }
  
  if (!ret_val)
    {
      destroyAnyChilds();

      if (outServerList != NULL)
        outServerList -> clear();
    }

  return ret_val;
}

template <class T, class P>
bool DArray::runCommand(const P& inParam,
                        int inServerIndex)
{
  bool ret_val = true;

  if (inServerIndex < 0)
    {
      return false;
    }

  DServer* worker = getServerManager()->getServerByIndex(inServerIndex);
  DServerCmd *cmd = 
    new T ();
      
  cmd -> setWorker(worker);
  cmd -> setIndex(inServerIndex);
  cmd -> setParam<P>(&inParam);
  
  cmd -> run();
  
  if (worker -> hasError())
    {
      ret_val = false;
    }

  delete cmd;
   
  return ret_val;
}

template <class T, class P>
bool DArray::runCommandThreaded(ZThread::ThreadedExecutor& inExec,
                                const P& inParam,
                                bool inWaitForThreadToEnd,
                                bool startChilds)
{
  // this command can run in 2 modes:
  // a) run a command for each worker 
  //    (worker can contain multiple indexes)
                  
  // b) run a command for each child
  //    (each index is starte as a separate thread)

  bool ret_val = true;

  const int arraySize = getSize();
  const int workerSize = getServerManager()->getNoOfUsedWorkers(arraySize);

  int runSize = 0;

  bool runForChilds = inParam.useChilds();

  vector<DServer*> serverList;

  if (runForChilds)
    {
      serverList.resize(arraySize, NULL);
      runSize = arraySize;
      
      if (!(multiplyWorkers(&serverList, startChilds)))
        {
          cerr << "ERROR: Could not multiply workers!" << endl;
          return false;
        }
    }
  else
    {
      serverList.resize(workerSize);
      runSize = workerSize;
      for (int ds = 0; ds < workerSize; ++ds)
        serverList[ds] = getServerManager()->getServerByID(ds);
    }

  DServer* worker = NULL;
  for(int i =0; ret_val && i < runSize; i++)
    {
      worker = serverList[i];

      if (worker != NULL)
        {
          // cmd ist destroyed automatically at end of thread
          // no need to delete it!
          DServerCmd *cmd = new T ();
          cmd -> setWorker(worker);
          cmd -> setIndex(i);
          if (!runForChilds)
            cmd -> setAllIndex(getServerManager()->getIndexList(i));

          cmd -> setParam<P>(&inParam);
          
          //cout << "RUN CMD THREADED " << i << cmd -> getInfo() << endl;
          inExec.execute(cmd);
          
          if (worker -> hasError())
            {
              cerr << "ERROR: Worker has errors!" << endl;
              cerr << worker ->  getErrorText() << endl;
              ret_val = false;
            }
        }
      else
        {
          cerr << "ERROR: Worker not defined!" << endl;
          ret_val = false;
        }
   }
  
  if (inWaitForThreadToEnd)
    {
      //cout << "RUN CMD THREADED: wait for finshed" << endl;
      inExec.wait();
      if (runForChilds && startChilds)
        destroyAnyChilds();
      //cout << " ... done";
    }
  return ret_val;
}
/*

4.6 Type Constructor

*/

struct darrayInfo : ConstructorInfo
{

   darrayInfo()
   {

      name         = DArray::BasicType();
      signature    = "typeconstructor -> ARRAY" ;
      typeExample  = "darray int";
      listRep      =  "(a1 a2 a3)";
      valueExample = "(4 12 2 8)";
      remarks = "A darray keeps all its element on remote systems";
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

5.1 Operator makeDarray

Typemap (rel() t t t...) -> darray t
Creates a new DArray. The first parameter must be a workerlist of
format rel(tuple([Server: string, Port: int]))
The other elements in the list form the elements of the array

*/

static ListExpr makeDarrayTypeMap( ListExpr args )
{
   //test whether workerlist-format is correct
  if (nl -> IsEmpty(args))
    return NList::typeError( "Empty input! Expecting \
       ((rel(tuple([Server: string, Port: int]))) t t ... t)" );

  NList workers (nl->First(args));

   // check worker rel
   if (!workers.isList() || 
       !listutils::isRelDescription(workers.listExpr()) ||
       !workers.
         second().
           second().
             first().
                second().isSymbol(CcString::BasicType()) ||
       !workers.
          second().
            second().
              second().
                second().isSymbol(CcInt::BasicType()))
     {
       return 
         NList::typeError
         ("Worker relation not in the correct format (string, int)");
     }
   //all other types must be the same
   args = nl->Rest(args);
   if (nl -> IsEmpty(args))
     return NList::typeError( "Only workers given! Expecting \
       ((rel(tuple([Server: string, Port: int]))) t t ... t)" );

   ListExpr first = nl->First(args);
   ListExpr rest = nl->Rest(args);

   while(!(nl->IsEmpty(rest)))
   {
      if(!nl->Equal(nl->First(rest),first))
         return nl->SymbolAtom(Symbol::TYPEERROR());

      rest = nl->Rest(rest);
   }

   return 
     nl->TwoElemList(
          nl->SymbolAtom(DArray::BasicType()),nl->First(args));

}

static int
makeDarrayfun( Word* args, Word& result,
             int message, Word& local, Supplier s )
{
   //determine element type
   SecondoCatalog* sc = SecondoSystem::GetCatalog();

   ListExpr type = qp->GetType(s);
   ListExpr typeOfElement = sc->NumericType(nl->Second(type));


   int algID, typID;
   extractIds( typeOfElement, algID, typID);

   //determine size of the array
   int size = qp->GetNoSons(s)-1;

   vector<Word> cloned(size);

   //Objects need to be cloned to be persistent after the query ends
   for(int i = 0;i<size;i++)
     {
      cloned[i] = 
        (am->CloneObj(algID,typID))(typeOfElement,args[i+1]);
     }

   //Generate serverlist as ListExpr from Relation
   GenericRelation* r = (GenericRelation*)args[0].addr;
   GenericRelationIterator* rit = r->MakeScan();
   ListExpr reltype;
   nl->
     ReadFromString("(rel (tuple ((Server string) (Port int))))",
                    reltype);
   ListExpr serverlist = Relation::Out(reltype,rit);

   result = qp->ResultStorage(s);

   bool rc = 
     ((DArray*)result.addr)->initialize(typeOfElement,
                                        getArrayName(DArray::no),
                                        size,serverlist,
                                        cloned);
   if (!rc)
     {
       cerr << "ERROR: Could not initialize DArray!" << endl;
       ((DArray*)result.addr)->SetUndefined();
     }

   return 0;
}

const string makeDarraySpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>(rel(tuple([Server: string, Port: int])) t t ...)"
       " -> darray t</text---><text>makeDarray ( _, _ )</text--->"
       "<text>Returns a distributed Array"
       " containing x element</text--->"
       "<text>query makeDarray(server_rel,1,2,3)</text---> ))";

Operator makeDarray(
         "makeDarray",
         makeDarraySpec,
         makeDarrayfun,
         Operator::SimpleSelect,
         makeDarrayTypeMap);

/*

5.2 Operator get

((darray t) int) -> t

Returns an element of a DArray.

*/

static ListExpr getTypeMap( ListExpr args )
{
   //There must be 2 parameters
   if(nl->ListLength(args) == 2)
   {
      ListExpr arg1 = nl->First(args);
      ListExpr arg2 = nl->Second(args);

      //The first one needs to a DArray and the second int
      if(!nl->IsAtom(arg1) &&
         nl->IsEqual(nl->First(arg1),DArray::BasicType()) &&
         nl->IsEqual(arg2,CcInt::BasicType()))
      {
         //The return-type of get is the element-type of the array
         ListExpr resulttype = nl->Second(arg1);
         return resulttype;
      }
   }

   return nl->SymbolAtom(Symbol::TYPEERROR());
}

static int getFun( Word* args,
                   Word& result,
                   int message,
                   Word& local,
                   Supplier s)
{
   DArray* array = ((DArray*)args[0].addr);
   result =  qp->ResultStorage(s);
   if ( array == 0 ||
        !(array -> IsDefined()) ||
        !(array -> getServerManager() -> checkServers(false)))
     {
       cerr << "ERROR: DArray is not defined correctly!" << endl;
       ((Attribute *)result.addr) -> SetDefined(false);
       return 0;
     }

   CcInt* index = ((CcInt*)args[1].addr);

   int i = index->GetIntval();

   int n = array -> getSize();

   if (i < 0 || i >= n)
     {
       cerr << "ERROR: invalid array index!" << endl;
       ((Attribute *)result.addr) -> SetDefined(false);
       return 0;
     }

   //Determine type
   ListExpr resultType = array->getType();

   int algID,typID;
   extractIds(resultType,algID,typID);

   //retrieve element from worker
   array->refresh(i);
   //copy the element


   if (array -> isRelType())
     {
       result.addr = ((Word)array->get(i)).addr;
     }
   else
     {
       Word org = (Word)array->get(i);
       /*
       Word cloned = 
         (am->CloneObj(algID,typID))
         (resultType,org);
      
       result.addr = cloned.addr;
       */  
       result.addr = org.addr;
     }

   return 0;
}

const string getSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>((darray t) int) -> t</text--->"
       "<text>get ( _, _ )</text--->"
       "<text>Returns an element from a distributed Array</text--->"
       "<text>query get(makeDarray(server_rel,1,2,3),1)"
       "</text---> ))";

Operator getA(
         "get",
         getSpec,
         getFun,
         Operator::SimpleSelect,
         getTypeMap);


/*

5.3 Operator put

((darray t), t, int) -> (darray t)

*/

static ListExpr putTypeMap( ListExpr args )
{
   if(nl->ListLength(args) == 3)
   {
      ListExpr arg1 = nl->First(args);
      ListExpr arg2 = nl->Second(args);
      ListExpr arg3 = nl->Third(args);

      //The first argument needs to be a darray, the second 
      //one of the same type as the array-elements and the 
      //third one int
      if(!nl->IsAtom(arg1) && 
         nl->IsEqual(nl->First(arg1), DArray::BasicType())
         && nl->Equal(nl->Second(arg1),arg2)
         && nl->IsEqual(arg3,CcInt::BasicType()))
      {
        return arg1;
      }
   }

   return nl->SymbolAtom(Symbol::TYPEERROR());
}


static int putFun( Word* args,
               Word& result,
               int message,
               Word& local,
               Supplier s)
{
  DArray* array_alt = ((DArray*)args[0].addr);
  Word element = args[1];
  int idx = ((CcInt*)args[2].addr)->GetIntval();

  //new array is initialized
  result = qp->ResultStorage(s);
  DArray* da = (DArray*)result.addr;
  
  if (array_alt == NULL ||
      !(array_alt -> IsDefined()))
    {
      cerr << "ERROR: DArray object is not defined!" << endl;
      da -> SetUndefined();
      return 0;
    }
  
  //new elements needs to be copied
  Word elem_n = ((am->CloneObj(array_alt->getAlgID(),
                               array_alt->getTypID()))
                 (array_alt->getType(),element));

  // check, if valid object
  if (da != NULL && 
      // check, if valid index
      idx >= 0 && idx < array_alt -> getSize())
    {
      da->initialize(array_alt->getType(),
                     getArrayName(DArray::no),
                     array_alt->getSize(),
                     array_alt->getServerList());

      ZThread::ThreadedExecutor poolEx;
      DServerCmdCopyParam param (da -> getName(), idx );
  
      if (array_alt -> 
          runCommandThreaded <DServerCmdCopy,  
                              DServerCmdCopyParam> (poolEx, param) )
        { 
          //The substituted element is set
          da -> set(elem_n, idx);

          // all is ok
          return 0;
        }
      else
        {
          if (array_alt -> hasError())
            {
              cerr << "ERROR:" 
                   << array_alt -> getErrorText() << endl;
            }
          else
            cerr << "ERROR: Operator put - unknown error!" << endl;
        }

    }
  else
    {
      // error msg
      if (da == NULL || !(da -> IsDefined()))
        cerr << "Error: undefined darray object!" << endl;
      else if (idx < 0 || idx >= array_alt -> getSize())
        cerr << "Error: invalid index: 0 <= "
             << idx << " < " << array_alt -> getSize() 
             << " is not a valid index!" << endl;
      else
        cerr << "Error in put function!" << endl;
    }
  // error: invalidate result
  da -> SetUndefined();
  return 0;
}

const string putSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>((darray t) t int) -> darray t</text--->"
       "<text>put ( _, _, _ )</text--->"
       "<text>Returns a distributed array where"
       "one element is altered</text--->"
   "<text>query put(makeDarray(server_rel,1,2,3),2,2)</text---> ))";

Operator putA(
         "put",
         putSpec,
         putFun,
         Operator::SimpleSelect,
         putTypeMap);

/*

5.4 Operator send

Internal Usage for Data Transfer between Master and Worker

*/

static ListExpr sendTypeMap( ListExpr args )
{
   //Always return int
   //Hostname and Port are appended for the value-mapping

   ListExpr ret = 
     nl->ThreeElemList(
      nl->SymbolAtom(Symbol::APPEND()),
      nl->TwoElemList(nl->StringAtom(nl->ToString(nl->First(args))),
                  nl->StringAtom(nl->ToString(nl->Second(args)))),
      nl->SymbolAtom(CcInt::BasicType()));
  return ret;
}

static int sendFun( Word* args,
                Word& result,
                int message,
                Word& local,
                Supplier s)
{
  result = qp->ResultStorage(s);

   //retrieve Hostname and Port
   string host = 
     (string)(char*)((CcString*)args[3].addr)->GetStringval();
   string port = 
     (string)(char*)((CcString*)args[4].addr)->GetStringval();

   host = stringutils::replaceAll(host,"_",".");
   host = stringutils::replaceAll(host,"h","");
   port = stringutils::replaceAll(port,"p","");
   
   //Connect to master
   DServerCmdCallBackCommunication callBack(host, port);
   if (!callBack.createGlobalSocket())
     {
       // error on stream
       ((CcInt*)result.addr)->Set(0);
       ((Attribute*)result.addr)->SetDefined(false);
       return 0;
     }

   string line;
   if (!callBack.getTextFromCallBack("TYPE", line))
     {
       ((CcInt*)result.addr)->Set(0);
       ((Attribute*)result.addr)->SetDefined(false);
       callBack.closeCallBackCommunication();
       return 0;
     }
   ListExpr type;
   nl->ReadFromString(line,type);
       
   int algID,typID;
   extractIds(type,algID,typID);
         
   Attribute* attr =static_cast<Attribute*>
     ((am->Cast(algID,typID))(args[2].addr));
   if (attr == NULL || !(attr -> IsDefined()))
     {
       cerr << "ERROR SENDING:";
       if (attr == NULL)
         cerr << "empty attribute";
       else
         cerr << "attribute not defined";
       cerr << "!" << endl;
       callBack.sendTagToCallBack("UNDEFINED");
       callBack.closeCallBackCommunication();
       return 0;
     }
   if (!callBack.sendTagToCallBack("DATA"))
     {
       ((CcInt*)result.addr)->Set(0);
       ((Attribute*)result.addr)->SetDefined(false);
       callBack.closeCallBackCommunication();
       return 0;
     }
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
       
   if (!callBack.sendTextToCallBack("SIZE", size))
     {
       ((CcInt*)result.addr)->Set(0);
       ((Attribute*)result.addr)->SetDefined(false);
       callBack.closeCallBackCommunication();
       return 0;
     }

   if (!callBack.Write(buffer,size))
     {
       ((CcInt*)result.addr)->Set(0);
       ((Attribute*)result.addr)->SetDefined(false);
       callBack.closeCallBackCommunication();
       return 0;
     }
       
   delete buffer;
       
   TypeConstructor* t = am->GetTC(algID,typID);
   Attribute* a = NULL;
   if(t->NumOfFLOBs() > 0 )
     a = static_cast<Attribute*>
       ((am->Cast(algID,typID))(args[2].addr));
       
   Flob::clearCaches();
   for(int i = 0; i < t->NumOfFLOBs(); i++)
     {
       Flob* f = a->GetFLOB(i);
           
       SmiSize si = f->getSize();
       int n_blocks = si / 1024 + 1;
       char* buf = new char[n_blocks*1024];
       memset(buf,0,1024*n_blocks);
           
       f->read(buf,si,0);
           
       if (!callBack.sendTagToCallBack("FLOB"))
         {
           ((CcInt*)result.addr)->Set(0);
           ((Attribute*)result.addr)->SetDefined(false);
           callBack.closeCallBackCommunication();
           return 0;
         }
       if (!callBack.sendTextToCallBack("FSIZE", si))
         {
           ((CcInt*)result.addr)->Set(0);
           ((Attribute*)result.addr)->SetDefined(false);
           callBack.closeCallBackCommunication();
           return 0;
         }

       for(int j = 0; j<n_blocks;j++)
         {
           if (! callBack.Write(buf+j*1024,1024))
             {
               ((CcInt*)result.addr)->Set(0);
               ((Attribute*)result.addr)->SetDefined(false);
               callBack.closeCallBackCommunication();
               return 0;
             }
         }
           
       delete buf;
     }
       
   if (!callBack.sendTagToCallBack("CLOSE"))
     {
       ((CcInt*)result.addr)->Set(0);
       ((Attribute*)result.addr)->SetDefined(false);
       callBack.closeCallBackCommunication();
       return 0;
     }
     
   if (!callBack.getTagFromCallBack("FINISH"))
     {
       ((CcInt*)result.addr)->Set(0);
       ((Attribute*)result.addr)->SetDefined(false);
       callBack.closeCallBackCommunication();
       return 0;
     }

   ((CcInt*)result.addr)->Set(1);

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
5.5 Operator receive

Internal Usage for Data Transfer between Master and Worker

*/

static ListExpr receiveTypeMap( ListExpr args )
{
   string host = nl->ToString(nl->First(args));
   string port = nl->ToString(nl->Second(args));
   string line;

   host = stringutils::replaceAll(host,"_",".");
   host = stringutils::replaceAll(host,"h","");
   port = stringutils::replaceAll(port,"p","");
 
   DServerCmdCallBackCommunication* callBack = 
     new DServerCmdCallBackCommunication (host, port);

   if (!(callBack -> createGlobalSocket()))
     {
       ErrorReporter::ReportError("Could not connect to Server!\n" +
                                  callBack -> getErrorText());
       delete callBack;
       return nl->SymbolAtom(Symbol::TYPEERROR());
     }

   if (!(callBack -> getTextFromCallBack("TYPE", line)))
     {
       ErrorReporter::ReportError("Received invalid token!\n" + 
                                  callBack -> getErrorText());
       delete callBack;
       return nl->SymbolAtom(Symbol::TYPEERROR());
     }

   ListExpr type;
   DBAccessGuard::getInstance() -> NL_ReadFromString(line,type);

   if (!(callBack -> sendTagToCallBack("CLOSE")))
     {
       ErrorReporter::ReportError("Could not send token!\n" + 
                                  callBack -> getErrorText());
       delete callBack;
       return nl->SymbolAtom(Symbol::TYPEERROR());
     }

   delete callBack;

   return
     nl->ThreeElemList(
        nl->SymbolAtom(Symbol::APPEND()),
          nl->TwoElemList(
                nl->StringAtom(nl->ToString(nl->First(args))),
                nl->StringAtom(nl->ToString(nl->Second(args)))),
          convertType(type));

}

static int receiveFun( Word* args,
                   Word& result,
                   int message,
                   Word& local,
                   Supplier s)
{
   string host = 
     (string)(char*)((CcString*)args[2].addr)->GetStringval();
   string port = 
     (string)(char*)((CcString*)args[3].addr)->GetStringval();
   string line;

   host = stringutils::replaceAll(host,"_",".");
   host = stringutils::replaceAll(host,"h","");
   port = stringutils::replaceAll(port,"p","");

   result = qp->ResultStorage(s);

   DServerCmdCallBackCommunication *recCallBack = 
     new DServerCmdCallBackCommunication(host, port
#ifdef RECEIVE_FUN_DEBUG
                                         , "REC_VM"
#endif
                                         );
   if(!(recCallBack -> createGlobalSocket()))
     {
       delete recCallBack;
       ((Attribute*) result.addr) -> SetDefined(false);
       return 0;
     }
   
   if (!(recCallBack -> getTextFromCallBack("TYPE", line)))
     {
       delete recCallBack;
       ((Attribute*) result.addr) -> SetDefined(false);
       return 0;
     }
   
   ListExpr type;
   size_t size =0;
   int algID,typID;
   DBAccessGuard::getInstance() -> NL_ReadFromString(line,type);
   
   extractIds(type,algID,typID);
   
   if (!(recCallBack -> sendTagToCallBack("GOTTYPE")))
     {
       delete recCallBack;
       ((Attribute*) result.addr) -> SetDefined(false);
       return 0;
     }
   
   if (!(recCallBack -> getTextFromCallBack("SIZE", line)))
     {
       delete recCallBack;
       ((Attribute*) result.addr) -> SetDefined(false);
       return 0;
     }
   
   size = atoi(line.data());
   
   
   char* buffer = new char[size];
   
   if (!(recCallBack ->Read(buffer,size)))
     {
       
       delete recCallBack;
       ((Attribute*) result.addr) -> SetDefined(false);
       return 0;
     }
     
   SmiRecordFile recF(false,0);
   SmiRecord rec;
   SmiRecordId recID;
     
   recF.Open("receiveop");
   recF.AppendRecord(recID,rec);
   size_t s0 = 0;
     
   rec.Write(buffer,size,0);
   Word w;
   am->OpenObj(algID,typID,rec,s0,type,w);
     
   result.addr = w.addr;
   rec.Truncate(3);
   recF.DeleteRecord(recID);
   recF.Close();
     
   bool noFlobError = true;
   int flobs = 0;
     
   while (noFlobError &&
          recCallBack -> getTagFromCallBackTF("FLOB", "CLOSE", noFlobError))
     {
         
       if (!(recCallBack -> getTextFromCallBack("FLOBSIZE", line)))
         {
           delete recCallBack;
           ((Attribute*) result.addr) -> SetDefined(false);
           return 0;
         }
         
       if (noFlobError)
         {
           SmiSize si = atoi(line.data());
             
           int n_blocks = si / 1024 + 1;
           char* buf = new char[n_blocks*1024];
           memset(buf,0,1024*n_blocks);
             
             
           for(int i = 0; i< n_blocks; i++)
             if (!(recCallBack -> Read(buf+1024*i,1024)))
               noFlobError = false;
             
           if (noFlobError)
             {
               Attribute* a = static_cast<Attribute*>
                 ((am->Cast(algID,typID))(result.addr));
                 
                 
               Flob*  f = a->GetFLOB(flobs);
               f->write(buf,si,0);
                 
               delete buf;
                 
               if (!(recCallBack -> sendTagToCallBack("GOTFLOB")))
                 {
                   delete recCallBack;
                   ((Attribute*) result.addr) -> SetDefined(false);
                   return 0;
                 }
             }
           else
             {
               delete buf;
             }
         } // if (noFlobError)
         
       if (!noFlobError)
         {
           recCallBack -> sendTagToCallBack("ERROR");
         }
       flobs ++;
     } // while "FLOB" ...
                  
   Flob::clearCaches();
     
   if (!(recCallBack -> sendTagToCallBack("FINISH")))
     {
       ((Attribute*) result.addr) -> SetDefined(false);
     }
     
    
   delete recCallBack;
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

5.6 Operator d\_receive\_rel

*/

static ListExpr receiverelTypeMap( ListExpr args )
{
   string host = nl->ToString(nl->First(args));
   string port = nl->ToString(nl->Second(args));
   string line;

   host = stringutils::replaceAll(host,"_",".");
   host = stringutils::replaceAll(host,"h","");
   port = stringutils::replaceAll(port,"p","");

   DServerCmdCallBackCommunication* callBack = 
     new DServerCmdCallBackCommunication (host, port);

   if (!(callBack -> createGlobalSocket()))
     {
       ErrorReporter::ReportError("Could not connect to Server!\n" +
                                  callBack -> getErrorText());
       delete callBack;
       return nl->SymbolAtom(Symbol::TYPEERROR());
     }

   if (!(callBack -> getTextFromCallBack("TYPE", line)))
     {
       ErrorReporter::ReportError("Received invalid token!\n" + 
                                  callBack -> getErrorText());
       delete callBack;
       return nl->SymbolAtom(Symbol::TYPEERROR());
     }

   ListExpr type;
   DBAccessGuard::getInstance() -> NL_ReadFromString(line,type);

   if (!(callBack -> sendTagToCallBack("CLOSE")))
     {
       ErrorReporter::ReportError("Could not send token!\n" + 
                                  callBack -> getErrorText());
       delete callBack;
       return nl->SymbolAtom(Symbol::TYPEERROR());
     }

   delete callBack;

#ifdef RECEIVE_REL_MAP_DEBUG
   cout <<  "receiverelTypeMap - done " << endl;
#endif
   
   return nl->ThreeElemList(
          nl->SymbolAtom(Symbol::APPEND()),
          nl->TwoElemList(
              nl->StringAtom(nl->ToString(nl->First(args))),
              nl->StringAtom(nl->ToString(nl->Second(args)))),
          convertType(type));
}

class RecContainer 
  : public DServerCmdCallBackCommunication::ReadTupleContainer
{
public:
  RecContainer(TupleType *inTT,
               GenericRelation* inRel,
               int inDelAttrIdx) 
    : DServerCmdCallBackCommunication::ReadTupleContainer()
    , m_saveTupleType(inTT)
    , m_rel(inRel)
    , m_delAttrIndex(inDelAttrIdx) {}

  virtual ~RecContainer() {}

  bool storeTuple(Tuple *t) const
  {
    Tuple* saveTuple = new Tuple(m_saveTupleType);
    int j = 0;
    for(int i = 0; i < t->GetNoAttributes(); i++)
      {
        if(i != m_delAttrIndex)
          saveTuple->CopyAttribute(i,t,j++);
      }

    DBAccessGuard::getInstance() -> REL_AppendTuple(m_rel,saveTuple);
    DBAccessGuard::getInstance() -> T_DeleteIfAllowed(saveTuple);
    return true;
  }
  
private:
  TupleType *m_saveTupleType;
  GenericRelation* m_rel;
  int m_delAttrIndex;

};

static int receiverelFun( Word* args,
                                    Word& result,
                                    int message,
                                    Word& local,
                                    Supplier s)
{
#ifdef RECEIVE_REL_FUN_DEBUG
      cout <<  " receiverelFun - start" << endl;
#endif
   string host = 
     (string)(char*)((CcString*)args[2].addr)->GetStringval();
   string port = 
     (string)(char*)((CcString*)args[3].addr)->GetStringval();

   ListExpr resultType, sendType;

   string line;

   host = stringutils::replaceAll(host,"_",".");
   host = stringutils::replaceAll(host,"h","");
   port = stringutils::replaceAll(port,"p","");

   result = qp->ResultStorage(s);

   DServerCmdCallBackCommunication *recCallBack = 
     new DServerCmdCallBackCommunication(host, port
#ifdef RECEIVE_REL_FUN_DEBUG
                                         , "REL_REC_VM"
#endif
                                   );

   if(!(recCallBack -> createGlobalSocket()))
     {
       delete recCallBack;
       ((Attribute*) result.addr) -> SetDefined(false);
       return 0;
     }

   if (!(recCallBack -> getTextFromCallBack("TYPE", line)))
     {
       delete recCallBack;
       ((Attribute*) result.addr) -> SetDefined(false);
       return 0;
     }
   DBAccessGuard::getInstance() -> NL_ReadFromString(line,resultType);
   resultType = nl->Second(resultType);

   if (!(recCallBack -> getTextFromCallBack("INTYPE", line)))
     {
       delete recCallBack;
       ((Attribute*) result.addr) -> SetDefined(false);
       return 0;
     }

   if (line == "EMPTY")
     {
       sendType = resultType;
     }
   else
     {
       DBAccessGuard::getInstance() -> NL_ReadFromString(line,sendType);
     }

   if (!(recCallBack -> getTextFromCallBack("DELIDX", line)))
     {
       delete recCallBack;
       ((Attribute*) result.addr) -> SetDefined(false);
       return 0;
     }
 
   int darrIndex = -1;

   if (line != "EMPTY")
     {
       darrIndex = atoi(line.data());
     }
 
   if (!(recCallBack -> sendTagToCallBack("GOTALL")))
     {
       delete recCallBack;
       ((Attribute*) result.addr) -> SetDefined(false);
       return 0;
     }
   
   TupleType* sendTT = DBAccess::getInstance() -> TT_New(sendType);
   TupleType* tupleType = DBAccess::getInstance() -> TT_New(resultType);
   GenericRelation* myRel = (Relation*)result.addr;

   RecContainer recContainer(tupleType, myRel, darrIndex);

   bool noTupleError = true;
   bool runIt = true;
   while (runIt)
     {
       runIt = recCallBack -> 
         getTagFromCallBackTF("TUPLE", "CLOSE", noTupleError);

       if (runIt)
         {
           if (!( recCallBack -> readTupleFromCallBack(sendTT, &recContainer)))
             noTupleError = false;
          
           if (noTupleError)
             {
               if (!(recCallBack -> sendTagToCallBack("OK")))
                 {
                   delete recCallBack;
                   ((Attribute*) result.addr) -> SetDefined(false);
                   return 0;
                 }
             }
           else
             {
               if (!(recCallBack -> sendTagToCallBack("ERROR")))
                 {
                   delete recCallBack;
                   ((Attribute*) result.addr) -> SetDefined(false);
                   return 0;
                 }
               runIt = false;
             }
         } // if (runIt)
     }// while (runIT)
   
   if (!(recCallBack -> sendTagToCallBack("FINISH")))
     {
       ((Attribute*) result.addr) -> SetDefined(false);
     }
     
   DBAccess::getInstance() -> TT_DeleteIfAllowed(tupleType);
   DBAccess::getInstance() -> TT_DeleteIfAllowed(sendTT);

   delete recCallBack;
   return 0;
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


5.7 Operator d\_send\_rel

Internal Usage for Data Transfer between Master and Worker

*/


static ListExpr sendrelTypeMap( ListExpr args )
{
   return nl->ThreeElemList(
                  nl->SymbolAtom(Symbol::APPEND()),
                  nl->TwoElemList(
                  nl->StringAtom(nl->ToString(nl->First(args))),
                  nl->StringAtom(nl->ToString(nl->Second(args)))),
                  nl->SymbolAtom(CcInt::BasicType()));

}

static int sendrelFun( Word* args,
                   Word& result,
                   int message,
                   Word& local,
                   Supplier s)
{

   string host =
     (string)(char*)((CcString*)args[3].addr)->GetStringval();
   string port =
     (string)(char*)((CcString*)args[4].addr)->GetStringval();
   string line;

   host = stringutils::replaceAll(host,"_",".");
   host = stringutils::replaceAll(host,"h","");
   port = stringutils::replaceAll(port,"p","");
 
   result = qp->ResultStorage(s);

   //Connect to master
   DServerCmdCallBackCommunication callBack(host, port);
   if (!callBack.createGlobalSocket())
     {
       // error on stream
       ((CcInt*)result.addr)->Set(0);
       ((Attribute*)result.addr)->SetDefined(false);
       return 0;
     }
   
   GenericRelation* rel = (Relation*)args[2].addr;
   GenericRelationIterator* iter = rel->MakeScan();
   Tuple* t;
   bool runIt = true;
   bool noError = true;
   while(runIt && (t=iter->GetNextTuple()) != 0)
     {
       if (!callBack.sendTagToCallBack("NEXTTUPLE"))
         {
           ((CcInt*)result.addr)->Set(0);
           ((Attribute*)result.addr)->SetDefined(false);
           return 0;
         }
       if (!callBack.writeTupleToCallBack(t))
         {
           ((CcInt*)result.addr)->Set(0);
           ((Attribute*)result.addr)->SetDefined(false);
           return 0;
         }

       runIt = callBack.getTagFromCallBackTF("OK", "ERROR", noError);

       if (!noError  || !runIt)
         {
           ((CcInt*)result.addr)->Set(0);
           ((Attribute*)result.addr)->SetDefined(false);
           return 0;
         }
     }
        
   if (!callBack.sendTagToCallBack("CLOSE"))
     {
       ((CcInt*)result.addr)->Set(0);
       ((Attribute*)result.addr)->SetDefined(false);
       return 0;
     }
   delete iter;

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

5.8 Operator d\_receive\_shuffle

*/

static ListExpr receiveShuffleTypeMap( ListExpr args )
{
  //NList myAargs (args);
                  
#ifdef RECEIVE_SHUFFLE_MAP_DEBUG
  cout << "ReceiveShuffleTM:"
       << myAargs.convertToString() << endl;
#endif

   string host = nl->ToString(nl->First(args));
   string port = nl->ToString(nl->Second(args));
   string line;

   host = stringutils::replaceAll(host,"_",".");
   host = stringutils::replaceAll(host,"h","");
   port = stringutils::replaceAll(port,"p","");

   DServerCmdCallBackCommunication* callBack =
     new DServerCmdCallBackCommunication(host, 
                                         port
#ifdef RECEIVE_SHUFFLE_MAP_DEBUG
                                         ,"SHUFFLE_REC_TM"
#endif
                                         );

   if (!(callBack -> createGlobalSocket()))
     {
       ErrorReporter::ReportError("Could not connect to Server!\n" +
                                  callBack -> getErrorText());
       delete callBack;
       return nl->SymbolAtom(Symbol::TYPEERROR());
     }

   if (!(callBack -> getTextFromCallBack("TYPE", line)))
     {
       ErrorReporter::ReportError("Received invalid token!\n" + 
                                  callBack -> getErrorText());
       delete callBack;
       return nl->SymbolAtom(Symbol::TYPEERROR());
     }

   ListExpr type;
   DBAccessGuard::getInstance() -> NL_ReadFromString(line,type);

   if (!(callBack -> sendTagToCallBack("CLOSE")))
     {
       ErrorReporter::ReportError("Could not send token!\n" + 
                                  callBack -> getErrorText());
       delete callBack;
       return nl->SymbolAtom(Symbol::TYPEERROR());
     }

   delete callBack;

#ifdef RECEIVE_SHUFFLE_MAP_DEBUG
   cout <<  "receive shuffle TM - done " << endl;
#endif

   ListExpr typeLE = convertType(type);

   ListExpr res =  nl->ThreeElemList(
                nl->SymbolAtom(Symbol::APPEND()),
                  nl->TwoElemList(
                   nl->StringAtom(nl->ToString(nl->First(args))),
                   nl->StringAtom(nl->ToString(nl->Second(args)))),
                typeLE);

   //cout << nl -> ToString(res) << endl;
   return res;
}

static int receiveShuffleFun( Word* args,
                                    Word& result,
                                    int message,
                                    Word& local,
                                    Supplier s)
{
#ifdef RECEIVE_SHUFFLE_FUN_DEBUG
      cout <<  " receiveShuffleFun - start" << endl;
#endif
   string host = 
     (string)(char*)((CcString*)args[2].addr)->GetStringval();
   string port =
     (string)(char*)((CcString*)args[3].addr)->GetStringval();


   //cout << "REC-SHUFFLE on " << host << ":" << port << endl;

   SecondoCatalog* sc = SecondoSystem::GetCatalog();
   ListExpr resultType = nl->Second(qp->GetType(s));
   resultType = sc->NumericType(resultType);
   
   result = qp->ResultStorage(s);

   string line;

   host = stringutils::replaceAll(host,"_",".");
   host = stringutils::replaceAll(host,"h","");
   port = stringutils::replaceAll(port,"p","");

   // create callBack w/ worker
   // this is done in DServerCmdShuffleRec

   DServerCmdCallBackCommunication *dscCallBack = 
     new DServerCmdCallBackCommunication(host, port
#ifdef RECEIVE_SHUFFLE_FUN_DEBUG
                                         , "SHUFFLERECVM"
#endif
                                         );

   if(dscCallBack -> createGlobalSocket())
     {

       //cout << "REC-SHUFFLE on " << host << ":" << port 
       //   << " - connected" << endl; 
       // now connected to the DServerCmdShuffleMultipleConn
       if (!(dscCallBack -> getTagFromCallBack("STARTMULTIPLYCONN")))
         {
           cerr << "ERROR:" << dscCallBack -> getErrorText() << endl;
           delete dscCallBack;
           ((Attribute*) result.addr) -> SetDefined(false);
           return 0;
         }
      
       if (!(dscCallBack -> sendTagToCallBack("OK")))
         {
           cerr << "ERROR:" << dscCallBack -> getErrorText() << endl;
           delete dscCallBack;
           ((Attribute*) result.addr) -> SetDefined(false);
           return 0;
         }

       // get the number of workers, which will be sending
       // data
       if (!(dscCallBack -> getTextFromCallBack("SRCWSIZE", line)))
         {
           cerr << "ERROR:" << dscCallBack -> getErrorText() << endl;
           delete dscCallBack;
           ((Attribute*) result.addr) -> SetDefined(false);
           return 0;
         }
      
       int srcSize = atoi(line.data());
      
       vector<string> srcHost(srcSize);
       vector<int> srcToPort(srcSize);
       int expectedstate = 0; // 0 - host, 1 - fromPort, 2 - toPort
       unsigned long srcCnt = 0;
       bool runIt = true;
       bool noError = true;
       string errMsg;

       do{  
         if (expectedstate == 0)
           { 
             if (!(dscCallBack -> 
                      getTextFromCallBack("SRCWHOST", line)))
               {
                 cerr << "ERROR:" 
                      << dscCallBack -> getErrorText() << endl;
                 delete dscCallBack;
                 ((Attribute*) result.addr) -> SetDefined(false);
                 return 0;
               }
             srcHost[srcCnt] = line;
             expectedstate = 1;
           }
         else if (expectedstate == 1)
           { 
             if (!(dscCallBack -> 
                      getTextFromCallBack("SRCWTPORT", line)))
               {
                 cerr << "ERROR:" 
                      << dscCallBack -> getErrorText() << endl;
                 delete dscCallBack;
                 ((Attribute*) result.addr) -> SetDefined(false);
                 return 0;
               }
             srcToPort[srcCnt] = atoi(line.data());
             srcCnt ++;
             expectedstate = 0;
           }

         if (!(dscCallBack -> sendTagToCallBack("OK")))
           {
             cerr << "ERROR:" 
                  << dscCallBack -> getErrorText() << endl;
             delete dscCallBack;
             ((Attribute*) result.addr) -> SetDefined(false);
             return 0;
           }
        
         runIt = 
           dscCallBack -> getTagFromCallBackTF("NEXT", "DONE", 
                                               noError);
        
         if (!noError)
           { 
             cerr << "ERROR:" 
                  << dscCallBack -> getErrorText() << endl;
             delete dscCallBack;
             ((Attribute*) result.addr) -> SetDefined(false);
             return 0;
           }

       } while (runIt);

       //cout << "REC-SHUFFLE on " << host << ":" << port 
       //           << " - got hosts: << " << srcCnt 
       // << " - emit ready" << endl; 
       //for(int i = 0; i < srcSize; i++)
       // cout << "REC-SHUFFLE on " << host << ":" << port 
       //      << " - " << srcHost[i] << ":" << srcToPort[i] << endl;

       if (!(dscCallBack -> sendTagToCallBack("READY")))
         {
           cerr << "ERROR:" 
                << dscCallBack -> getErrorText() << endl;
           delete dscCallBack;
           ((Attribute*) result.addr) -> SetDefined(false);
           return 0;
         }
 
       // setup CommandQueue for each server;
       vector<DServerShuffleReceiver*> serverCommand(srcSize);
  
       GenericRelation* rel = (Relation*)result.addr;

       //cout << "REC-SHUFFLE on " << host << ":" << port 
       //           << " - starting receivers" << endl; 
       try
         {
           ZThread::ThreadedExecutor poolEx;
          
           for(int i = 0; i < srcSize; i++)
             {
               serverCommand[i] = 
                 new DServerShuffleReceiver(srcHost[i],
                                            int2Str(srcToPort[i]),
                                            rel,
                                            resultType);
              
               poolEx.execute(serverCommand[i]);
             }

           //dscCallBack -> sendTagToCallBack("RUNNING");
         
           poolEx.wait();
         }
       catch(ZThread::Synchronization_Exception& e)
         {
           cerr << "Could not distribute data!" << endl;
           cerr << e.what() << endl;
           ((Attribute*) result.addr) -> SetDefined(false);
           return 0;
         }

       //cout << "REC-SHUFFLE on " << host << ":" << port 
       //           << " - sending DONE" << endl; 

       if (!noError)
         { 
           cout << "RECEIVER on SENDING ERROR!" << endl;

           if (!(dscCallBack -> sendTagToCallBack("ERROR")))
             {
               cerr << "ERROR:" 
                    << dscCallBack -> getErrorText() << endl;
               delete dscCallBack;
               ((Attribute*) result.addr) -> SetDefined(false);
               return 0;
             }
           string errorMsg ="Shuffle: error receiving tuples!";
           if (!errMsg.empty())
             errorMsg += "Reason: " + errMsg;

           if (!(dscCallBack -> 
                    sendTextToCallBack("ERRORDESC", errorMsg)))
             {
               cerr << "ERROR:" 
                    << dscCallBack -> getErrorText() << endl;
               delete dscCallBack;
               ((Attribute*) result.addr) -> SetDefined(false);
               return 0;
             }
         }
       else
         {
           if (!(dscCallBack -> sendTagToCallBack("DONE")))
             {
               delete dscCallBack;
               ((Attribute*) result.addr) -> SetDefined(false);
               return 0;
             }
         }

       //dscCallBack -> forceCloseSavedCommunication();
       delete dscCallBack;

     }
   else //   if(dscCallBack -> createGlobalSocket())
     {
       cerr << "ERROR: Could not connect receivers and senders!" 
            <<endl;
     }

   return 0;

} //static int receiveShuffleFun(...)

const string receiveShuffleSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>-</text--->"
       "<text>-</text--->"
       "<text>Internal Usage by DistributedAlgebra</text--->"
       "<text>-</text---> ))";

Operator receiveShuffle(
         "d_receive_shuffle",
         receiveShuffleSpec,
         receiveShuffleFun,
         Operator::SimpleSelect,
         receiveShuffleTypeMap);

/*


5.7 Operator d\_send\_shuffle

Internal Usage for Data Transfer between Master and Worker
for the shuffle operator

*/


static ListExpr sendShuffleTypeMap( ListExpr inArgs )
{
  NList args (inArgs);
  
#ifdef SEND_SHUFFLE_MAP_DEBUG
  cout << "HERE!: SendShuffleTM:" << args.convertToString() 
       << endl;    
#endif      

  ListExpr errLE = nl->SymbolAtom(Symbol::TYPEERROR());

  if (args.length() != 4)
    {
      ErrorReporter::ReportError
        ("Wrong number of arguments for d_shuffle_send operator!");
      return errLE;
    }
  NList mapDesc = args.second();

  if (mapDesc.third() != NList(CcInt::BasicType()))
    {
      ErrorReporter::ReportError
        ("Function return type must be of type <int>!!");
      return errLE;
    }
    
   ListExpr ret = nl->ThreeElemList(
                  nl->SymbolAtom(Symbol::APPEND()),
                  nl->TwoElemList(
                     nl->StringAtom(args.third().convertToString()),
                     nl->StringAtom(args.fourth().convertToString())
                                  ),
                  nl->SymbolAtom(CcInt::BasicType()));
                  
#ifdef SEND_SHUFFLE_MAP_DEBUG
   cout << "SendShuffleTM Done:" 
        << NList(ret).convertToString() << endl;    
#endif      
   return ret;

}

static int sendShuffleFun( Word* args,
                           Word& result,
                           int message,
                           Word& local,
                           Supplier s)
{
   string host =
     (string)(char*)((CcString*)args[4].addr)->GetStringval();
   string port =
     (string)(char*)((CcString*)args[5].addr)->GetStringval();
   string line;

   bool invalidIndex = false;
   host = stringutils::replaceAll(host,"_",".");
   host = stringutils::replaceAll(host,"h","");
   port = stringutils::replaceAll(port,"p","");

 DServerCmdCallBackCommunication *dscCallBack = 
     new DServerCmdCallBackCommunication(host, port
#ifdef SEND_SHUFFLE_FUN_DEBUG
                                         , "SHUFFLESEND"
#endif
                                         );

   result = qp->ResultStorage(s);

   // cout << "SENDER " 
   //<< host << ":" << port << " connecting!" << endl;

   if(dscCallBack -> createGlobalSocket())
   {
      // now connected to the DServerCmdShuffleMultipleConn
      if (!(dscCallBack -> getTagFromCallBack("STARTMULTIPLYCONN")))
          {
            cerr << "ERROR:" 
                 << dscCallBack -> getErrorText() << endl;
            delete dscCallBack;
            ((Attribute *) result.addr) -> SetDefined(false);
            return 0;
          }

      if (!(dscCallBack -> sendTagToCallBack("OK")))
        {
          cerr << "ERROR2" << dscCallBack -> getErrorText() << endl;
          delete dscCallBack;
          ((Attribute *) result.addr) -> SetDefined(false);
          return 0;
        }
        
      // get the number of workers, which will be receiving
      // data
      if (!(dscCallBack -> getTextFromCallBack("SRCWSIZE", line)))
        {
          cerr << "ERROR3" << dscCallBack -> getErrorText() << endl;
          delete dscCallBack;
          ((Attribute *) result.addr) -> SetDefined(false);
          return 0;
        }
      
      int srcSize = atoi(line.data());

      vector<string> destHost(srcSize);
      vector<int> destToPort(srcSize);
      int expectedstate = 0; // 0 - host, 1 - toPort
      unsigned long destCnt = 0;
      bool runIt = true;
      bool noError = true;
      string errMsg;

      do{  
        if (expectedstate == 0)
          { 
            if (!(dscCallBack -> 
                    getTextFromCallBack("SRCWHOST", line)))
              {
                cerr << "ERROR:" 
                     << dscCallBack -> getErrorText() << endl;
                delete dscCallBack;
                ((Attribute *) result.addr) -> SetDefined(false);
                return 0;
              }
            destHost[destCnt] = line;
            expectedstate = 1;
          }
        else if (expectedstate == 1)
          { 
             if (!(dscCallBack -> 
                      getTextFromCallBack("SRCWTPORT", line)))
              {
                cerr << "ERROR:" 
                     << dscCallBack -> getErrorText() << endl;
                delete dscCallBack;
                ((Attribute *) result.addr) -> SetDefined(false);
                return 0;
              }
            destToPort[destCnt] = atoi(line.data());
            destCnt ++;
            expectedstate = 0;
          }

        if (!(dscCallBack -> sendTagToCallBack("OK")))
          {
            cerr << "ERROR:" 
                 << dscCallBack -> getErrorText() << endl;
            delete dscCallBack;
            ((Attribute *) result.addr) -> SetDefined(false);
            return 0;
          }
        
        runIt = 
          dscCallBack -> 
              getTagFromCallBackTF("NEXT", "DONE", noError);
        if (!noError)
          {
            cerr << "ERROR:" 
                 << dscCallBack -> getErrorText() << endl;
            delete dscCallBack;
            ((Attribute *) result.addr) -> SetDefined(false);
            return 0;
          }

      } while (runIt);

      
      if (!(dscCallBack -> sendTagToCallBack("READY")))
        {
          cerr << "ERRORA:" 
               << dscCallBack -> getErrorText() << endl;
          delete dscCallBack;
          ((Attribute *) result.addr) -> SetDefined(false);
          return 0;
        }

      // now sending tuples
      ThreadedMemoryCounter memCntr (qp->GetMemorySize(s) * 1024 * 1024);
      
      Word current = SetWord( Address (0) );
      
      Stream<Tuple> inTupleStream(args[0]);
      inTupleStream.open();
      
      Tuple* tuple1;
      // setup CommandQueue for each server;
      vector<DServerShuffleSender*> serverCommand(srcSize); 
      
      try
        {
          //ZThread::PoolExecutor poolEx(2);
          ZThread::ThreadedExecutor poolEx;
                    
          for(int i = 0; i < srcSize; i++)
            {
              serverCommand[i] = 
                new DServerShuffleSender(destHost[i],
                                         int2Str(destToPort[i]), 
                                         &memCntr);
              
              poolEx.execute(serverCommand[i]);
            }

          Word value;
          ArgVectorPointer funargs;

          while( !invalidIndex &&
                 (tuple1 = 
                  DBAccess::getInstance() -> 
                         TS_Request(inTupleStream)) != 0)
            {
              // ArrayIndex
              int arrIndex = 0;
             
              funargs = qp->Argument(args[1].addr);
              ((*funargs)[0]).setAddr(tuple1);
              qp->Request(args[1].addr, value);
              if ( ((CcInt*)value.addr)->IsDefined())
                arrIndex = ((CcInt*)value.addr)->GetIntval();

              if (arrIndex >= srcSize ||
                  arrIndex < 0)
                {
                  errMsg = "INVALID INDEX (" + 
                    int2Str(arrIndex) + ")";
                  noError = false;
                  invalidIndex = true;
                }
              else
                {
                  memCntr.request(tuple1 -> GetSize());
                  
                  serverCommand[arrIndex] -> AppendTuple(tuple1);
                }
              //number ++;
            } // while (...)
          
          for(int i = 0; i < srcSize; i++)
            {
              //cout << "DONE: " << i << endl;
              serverCommand[i] -> done();
            }
          
          poolEx.wait();

          inTupleStream.close();
          
        }
      catch(ZThread::Synchronization_Exception& e)
        {
          cerr << "Could not distribute data!" << endl;
          cerr << e.what() << endl;
          ((Attribute *) result.addr) -> SetDefined(false);
          return 0;
        }
      
      if (!noError)
        { 
          if (!(dscCallBack -> sendTagToCallBack("ERROR")))
            {
              cerr << "ERROR:" 
                   << dscCallBack -> getErrorText() << endl;
              delete dscCallBack;
              ((Attribute *) result.addr) -> SetDefined(false);
              return 0;
            }
          string errorMsg ="Shuffle: error sending tuples!";
          if (!errMsg.empty())
            errorMsg += "Reason: " + errMsg;

          if (!(dscCallBack -> 
                  sendTextToCallBack("ERRORDESC", errorMsg)))
            {
              cerr << "ERROR:" 
                   << dscCallBack -> getErrorText() << endl;
              delete dscCallBack;
              ((Attribute *) result.addr) -> SetDefined(false);
              return 0;
            }
        }
      else
        {
          if (!(dscCallBack -> sendTagToCallBack("DONE")))
            {
              cerr << "ERROR:"
                   << dscCallBack -> getErrorText() << endl;
              delete dscCallBack;
              ((Attribute *) result.addr) -> SetDefined(false);
              return 0;
            }
        }

      delete dscCallBack;
   }

   ((CcInt*)result.addr)->Set(1);

   return 0;

   }

const string sendShuffleSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>-</text--->"
       "<text>-</text--->"
       "<text>Internal Usage by DistributedAlgebra</text--->"
       "<text>-</text---> ))";

Operator sendShuffle(
         "d_send_shuffle",
         sendShuffleSpec,
         sendShuffleFun,
         Operator::SimpleSelect,
         sendShuffleTypeMap);


/*


5.8 Operator distribute

*/


static ListExpr distributeTypeMap( ListExpr inargs )
{
   NList args(inargs);
   if( args.length() == 4)
   {
      NList stream_desc = args.first();
      ListExpr attr_desc = args.second().listExpr();

      NList workers = args.fourth();

      // check worker rel
      if (!workers.isList() || 
          !listutils::isRelDescription(workers.listExpr()) ||
          !workers.
            second().
              second().
                first().
                  second().isSymbol(CcString::BasicType()) ||
          !workers.
            second().
              second().
                second().
                  second().isSymbol(CcInt::BasicType()))
        {
          return 
            args.typeError
            ("workers must be of type\
(rel(tuple([Server:string, Port:int]))) )");
        }

      if( stream_desc.isList() && 
          stream_desc.first().isSymbol(Symbol::STREAM())
            && (stream_desc.length() == 2)
            && (nl->AtomType(attr_desc) == SymbolType))
      {
         ListExpr tuple_desc = stream_desc.second().listExpr();
         string attr_name = nl->SymbolValue(attr_desc);

         if(nl->IsEqual(nl->First(tuple_desc),Tuple::BasicType()) &&
            nl->ListLength(tuple_desc) == 2)
         {
            ListExpr attrL = nl->Second(tuple_desc);

            if(IsTupleDescription(attrL))
            {
               int attrIndex;
               ListExpr attrType;

               attrIndex = FindAttribute(attrL,attr_name,attrType);

               if(nl->ListLength(attrL > 1) && attrIndex > 0
                  && nl->IsEqual(attrType,CcInt::BasicType()))
               {
                  ListExpr attrL2 = nl -> TheEmptyList();
                  ListExpr last = nl -> TheEmptyList();

                  while(!nl->IsEmpty(attrL))
                  {
                     ListExpr attr = nl->First(attrL);

                     if(nl->SymbolValue(nl->First(attr)) != 
                        attr_name)
                     {
                        if(nl->IsEmpty(attrL2))
                        {
                           attrL2 = nl->OneElemList(attr);
                           last = attrL2;
                        }
                        else
                           last = nl->Append(last,attr);
                     }

                     attrL = nl->Rest(attrL);
                  }
                  return 
                    nl->ThreeElemList(
                        nl->SymbolAtom(Symbol::APPEND()),
                        nl->TwoElemList(nl->IntAtom(attrIndex),
                          NList(NList(tuple_desc).convertToString(),
                                true, true).listExpr()),
                        nl->TwoElemList(
                          nl->SymbolAtom(DArray::BasicType()),
                          nl->TwoElemList(
                              nl->SymbolAtom(Relation::BasicType()),
                              nl->TwoElemList(
                                 nl->SymbolAtom(Tuple::BasicType()),
                                 attrL2))));
               }
            }
         }
      }
   }

   return args.typeError("input is not (stream(tuple(y))) x ...");
}

static int
distributeFun (Word* args, Word& result, 
               int message, Word& local, 
               Supplier s)
{
   int size = ((CcInt*)(args[2].addr))->GetIntval();

   GenericRelation* r = (GenericRelation*)args[3].addr;
   GenericRelationIterator* rit = r->MakeScan();
   ListExpr reltype;
   nl->ReadFromString("(rel (tuple ((Server string) (Port int))))",
                      reltype);
   ListExpr serverlist = Relation::Out(reltype,rit);

   int attrIndex = ((CcInt*)(args[4].addr))->GetIntval() - 1;
   string attrIndexStr (int2Str(attrIndex));
   
   string sendTType = ((FText*)args[5].addr)->GetValue();

   SecondoCatalog* sc = SecondoSystem::GetCatalog();

   ListExpr sendTypeNum;
   nl->ReadFromString(sendTType,sendTypeNum);
   sendTypeNum = sc->NumericType(sendTypeNum);
   sendTType = nl -> ToString(sendTypeNum);

   ListExpr restype = nl->Second(qp->GetType(s));
   restype = sc->NumericType(restype);
   DArray* array = (DArray*)(qp->ResultStorage(s)).addr;

   array->initialize(restype,getArrayName(DArray::no),
                     size, serverlist);
   
   // setup tuple queue for each server;
   ThreadedMemoryCounter memCntr (qp->GetMemorySize(s) * 1024 * 1024);
   vector<DServerMultiCommand*> serverCommand(size); 
 

   for(int i = 0; i < size; i++)
     {
       serverCommand[i] = 
         new DServerMultiCommand(&memCntr);

     }

   DServerCmdWriteRelParam writeRelParam(&serverCommand,
                                         sendTType,
                                         attrIndexStr);
   bool success = true;                          
   try
     {
       ZThread::ThreadedExecutor poolEx;
       success = array -> runCommandThreaded<DServerCmdWriteRel,
                                DServerCmdWriteRelParam>(poolEx,
                                                         writeRelParam,
                                                         false); 
       if (success)
         {
           Stream<Tuple> inTupleStream(args[0]);
           inTupleStream.open();
   
           Tuple* tuple1;

           cout << "Reading Data ..." << endl;

           while( (tuple1 = 
                   DBAccess::getInstance() -> 
                   TS_Request(inTupleStream)) != 0)
             {
               DBAccessGuard::getInstance() -> T_IncReference(tuple1);
               // ArrayIndex
               int arrIndex = 
                 ((CcInt*)(tuple1->
                           GetAttribute(attrIndex)))->GetIntval();
       
               assert (arrIndex >= 0);
       
               arrIndex = arrIndex % size;
       
               memCntr.request(tuple1 -> GetSize());
       
               serverCommand[arrIndex] -> AppendTuple(tuple1);
               DBAccessGuard::getInstance() -> T_DeleteIfAllowed(tuple1);

             } // while (...)

           inTupleStream.close();

           cout << "Reading Data done ..." << endl;
         }
       for(int i = 0; i < size; i++)
         {
           serverCommand[i] -> done();
         }
       
       poolEx.wait();

     }
   catch(ZThread::Synchronization_Exception& e)
     {
       cerr << "Could not distribute data!" << endl;
       cerr << e.what() << endl;
       array -> SetUndefined();
       result = SetWord(array);
       return 0;
     }

   if (!success)
     { 
       cerr << "Could not distribute data!" << endl;
       array -> SetUndefined();
       result = SetWord(array);
       return 0;
     }
   cout << "Closing connections ..." << endl;
   array -> destroyAnyChilds();


   for(int i = 0; i < size; i++)
     delete serverCommand[i];
     

   result.addr = array;
   return 0;

}





const string distributeSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
"( <text>((stream (tuple ((x1 t1) ... (xn tn)))) xi int (rel(tuple("
      "[Server:string, Port: int]))) ) -> "
     "(darray (rel (tuple ((x1 t1) ... (xi-1 ti-1)"
     "(xi+1 ti+1) ... (xn tn)))))</text--->"
      "<text>_ ddistribute [ _ , _ , _]</text--->"
      "<text>Distributes a stream of tuples"
     "into a darray of relations.</text--->"
      "<text>plz feed ddistribute [pkg,3,server_rel]</text--->))";

Operator distributeA (
      "ddistribute",
      distributeSpec,
      distributeFun,
      Operator::SimpleSelect,
      distributeTypeMap );


/*


5.8 Operator shuffle

*/

template<int dim>
static ListExpr shuffleTypeMap( ListExpr inargs )
{
  ListExpr errRes = nl->SymbolAtom(Symbol::TYPEERROR());

  NList m_args(inargs);
  NList params;

   if(m_args.length() == dim + 1)
    {

      NList mapdesc = m_args.elem(2);
      
      if (mapdesc.first().length() != 3 )
        {
          ErrorReporter::ReportError("Invalid function description");
          return  errRes;
        }
      if (mapdesc.first().first() != NList(Symbol::MAP()))
        {
          ErrorReporter::ReportError(
                     "First argument must be a function!");
          return  errRes;  
        }
      if (mapdesc.first().elem(3) == NList(Symbol::TYPEERROR()))
        { 
          ErrorReporter::ReportError("Function contains an error!");
          return  errRes;
        }
      if (mapdesc.first().elem(3) != NList(CcInt::BasicType()))
        {
          ErrorReporter::ReportError
            ("Function return value must be of type <int>!");
          return  errRes;
        }

      NList funcExpr = mapdesc.second().third();

      params = NList();

      params.append(NList(NList(NList(funcExpr.
                                      convertToString(),
                                      true, true))));

      NList darraydesc = m_args.first().elem(1);

      // check for correct types
      if (darraydesc.length() != 2)
        {
          ErrorReporter::ReportError(
                  "First object must be of type <darray>!");
          return  errRes;
        }

      if(!darraydesc.first().isSymbol(DArray::BasicType()))
        {
          
          ErrorReporter::ReportError(
                  "First object must be of type <darray>!");
          return  errRes;
        }

      if (dim == 3)
        {
          NList workers = m_args.fourth().first();
          
          // check worker rel
          if (!workers.isList() || 
              !listutils::isRelDescription(workers.listExpr()) ||
              !workers.
                second().
                  second().
                    first().
                      second().isSymbol(CcString::BasicType()) ||
              !workers.
                second().
                  second().
                    second().
                      second().isSymbol(CcInt::BasicType()))
            {
              return 
                m_args.typeError
                ("workers must be of type\
                      (rel(tuple([Server:string, Port:int]))) )");
             }
        }

      NList res =
        NList(NList(Symbols::APPEND()),
              NList(params),

              darraydesc);

      return res.listExpr();
    }
   
   return errRes;
}

// Arguments (for dim == 3):
// 0 - source DArray
// 1 - index function (unused)
// 2 - destination size (optional)
// 3 - worker relation (optional)
// appended args:
// 4 - index function (string)

template<int dim>
static int
shuffleFun (Word* args, Word& result, 
            int message, Word& local, Supplier s)
{
  DArray* sourceArray = (DArray *)(args[0].addr); 
  DArray* destArray = (DArray*)(qp->ResultStorage(s)).addr;

  // check source DArray
  if (sourceArray == 0 ||
      sourceArray -> getServerManager() == 0 ||
      !(sourceArray -> getServerManager() -> isOk()) )
    {
      cerr << "ERROR: DArray not initialized corretly!"  << endl;
      destArray -> SetDefined(false);
      result.addr = destArray;
      return 0;
    }

  if (!(sourceArray -> IsDefined()))
    {
      cerr << "Undefined DArray!" << endl;
      destArray -> SetDefined(false);
      result.addr = destArray;
      return 0;
    }

  // setup size of source and destination DArray
  const int srcSize = sourceArray -> getSize();
  int destSize = -1;

  if (dim > 1)
    destSize = ((CcInt*)(args[2].addr))->GetIntval();
  else
    // destination size was not set
    destSize = srcSize;
  
  if (destSize < 1)
    {
      cerr << "Undefined DArray size!" << endl;
      destArray -> SetDefined(false);
      result.addr = destArray;
      return 0;
    }
  

  if (destSize  > 36 ||
      srcSize > 36 )
    {
      cerr << "DArray size may not be larger than 36!" << endl;
      destArray -> SetUndefined();
      result.addr = destArray;
      return 0;
    }
  ListExpr serverlist = nl -> TheEmptyList();

  if (dim == 3) // worker relation is given; read it!
    {
      GenericRelation* r = (GenericRelation*)args[3].addr;
      GenericRelationIterator* rit = r->MakeScan();
      ListExpr reltype;
      nl->
        ReadFromString("(rel (tuple ((Server string) (Port int))))",
                       reltype);
      serverlist = Relation::Out(reltype,rit);
    }
  else
    {
      serverlist = sourceArray -> getServerList();
    }

  if (nl -> IsEmpty(serverlist))
    {
      cerr << "ERROR: No workers defined!" << endl;
      destArray -> SetDefined(false);
      result.addr = destArray;
      return 0;
    }

  string sendFunc = ((FText*)args[dim + 1].addr)->GetValue();

  SecondoCatalog* sc = SecondoSystem::GetCatalog();

  ListExpr restype = nl->Second(qp->GetType(s));
  restype = sc->NumericType(restype);

   destArray->initialize(restype,getArrayName(DArray::no),
                         destSize, serverlist);

   if (!(destArray ->  getServerManager() -> isOk() ))
    {
      cerr << "ERROR: DArray not initialized correctly!" << endl;
      cerr << destArray ->  getServerManager() -> getErrorText() 
           << endl;
      destArray -> SetDefined(false);
      result.addr = destArray;
      return 0;
    }

  if (!(destArray -> IsDefined()))
    {
      cerr << "Undefined DArray!" << endl;
      destArray -> SetDefined(false);
      result.addr = destArray;
      return 0;
    }

   // mapping for source <--> destination
   vector<vector<string> > sourceWorker (destSize);
   vector<vector<int> > sourceToDestWorkerIdx (destSize);
   vector<vector<int> > sourceToDestFromPort (destSize);

   // mapping for destination <--> source
   vector<vector<string> >destWorker (srcSize);
   vector<vector<int> > destToSourceToPort (srcSize);

   int basePort = 1900; 
   // init vecotrs of inner loop
   for (unsigned long destWorkerSize = 0; 
        destWorkerSize < (unsigned long) destSize; ++destWorkerSize)
     {
       sourceWorker[destWorkerSize] = vector<string>  (srcSize);
       sourceToDestFromPort[destWorkerSize] = vector<int> (srcSize);
     }

   for (unsigned long srcWorkerSize = 0; 
        srcWorkerSize < (unsigned long) srcSize; ++srcWorkerSize)
     {
       destWorker[srcWorkerSize] = 
         vector<string>(destSize);
       destToSourceToPort[srcWorkerSize] = 
         vector<int> (destSize);
       for (unsigned long destWorkerSize = 0; 
            destWorkerSize < (unsigned long) destSize;
            ++destWorkerSize)
         {
           string worker = 
             sourceArray -> getHostNameByIndex(srcWorkerSize);
           int fromPort = basePort++;
           
           // compile information for the destination worker
           // source -> destination
           // first index: destination worker idx
           // second index: source worker idx
           // source host
           sourceWorker[destWorkerSize][srcWorkerSize] = worker;
           // source port
           sourceToDestFromPort[destWorkerSize][srcWorkerSize] = 
             fromPort;

           // compile information for the source worker
           // destination -> source
           // first index: source worker idx
           // second index: destination worker idx
           // destination host
           destWorker[srcWorkerSize][destWorkerSize] = worker;

           // source port (same as for destination worker
           destToSourceToPort[srcWorkerSize][destWorkerSize] = 
             fromPort;
         }
     }

   // initiate source workers
   if (!(sourceArray -> getServerManager() -> isOk()))
     {
       //Close additional connections
       destArray -> destroyAnyChilds();
       sourceArray -> destroyAnyChilds();
       destArray -> SetDefined(false);
       result.addr = destArray;
       return 0;
     }
  
   const int srcBasePort = 1700;
   const int destBasePort = 1800;

   bool abort = false;
   try
     {
       ZThread::ThreadedExecutor poolEx2;
      
       DServerCmdShuffleRecParam destParams (destBasePort);

       if (!(destArray -> 
                runCommandThreaded <DServerCmdShuffleRec,
                                    DServerCmdShuffleRecParam>
             ( poolEx2, // Executor
               destParams,  // Parameters
               false)))  // NOT wait for thread to end
         {
           abort = true;
         }

       if (!abort)
         {
           DServerCmdShuffleSendParam sourceParams 
             (sendFunc, // sender function
              srcBasePort); // base port number

           if (!(sourceArray -> 
                 runCommandThreaded <DServerCmdShuffleSend,
                                     DServerCmdShuffleSendParam>
                 ( poolEx2, // Executor
                   sourceParams,  // Parameters
                   false)))  // NOT wait for thread to end
             {
               abort = true;
             }
         }

       //cout << "DSHUFFLE - Sender and Reciver started" << endl;

       // create connectoions betweem source and destination servers;
       // and start shuffleing!
       if (! abort)
         {
           try
             {
               ZThread::ThreadedExecutor poolEx4;
               DServerCmdShuffleMultiConnParam srcMulitParams
                 (DServerCmdShuffleMultiConnParam::DSC_SMC_P_SENDER, 
                  srcBasePort, destWorker, destToSourceToPort);
           
               //cout << "  START Source Multiplyer" << endl;
               if (!(sourceArray -> 
                     runCommandThreaded<DServerCmdShuffleMultiConn,
                                    DServerCmdShuffleMultiConnParam>
                     ( poolEx4, // Executor
                       srcMulitParams,  // Parameters
                       false,  // NOT wait for thread to end
                       false))) // Do Not multiply workers again
                 {
                   abort = true;
                 }

               //cout << "  START dest Multiplyer" << endl;
               DServerCmdShuffleMultiConnParam destMulitParams
              (DServerCmdShuffleMultiConnParam::DSC_SMC_P_RECEIVER,
                  destBasePort, sourceWorker, sourceToDestFromPort);
           
               if (!(destArray -> 
                     runCommandThreaded <DServerCmdShuffleMultiConn,
                                   DServerCmdShuffleMultiConnParam>
                     ( poolEx4, // Executor
                       destMulitParams,  // Parameters
                       false, // NOT wait for thread to end
                       false))) // Do Not multiply workers again
                 {
                   abort = true;
                 }

               poolEx4.wait();
             }
           catch(ZThread::Synchronization_Exception& e)
             {
               abort = true;
             }
           
         }


       if (abort)
         {
           //Close additional connections
           destArray -> destroyAnyChilds();
           sourceArray -> destroyAnyChilds();
           destArray -> SetDefined(false);
           result.addr = destArray;
           return 0;
         }

       poolEx2.wait(); // command threads
     }
   catch(ZThread::Synchronization_Exception& e)
     {
     }

   DServerManager* destMan = destArray->getServerManager();
   DServerManager* sourceMan = sourceArray->getServerManager();
   
   
   for(int i = 0; 
       (i < (sourceMan -> getNoOfUsedWorkers(sourceArray->getSize()))) && 
         !abort; i++)
     {
       DServer* w = sourceMan -> getServerByID(i);

       if (w -> hasError())
         {
           cerr << "ERROR: DShuffle encountered errors:\n"
                << endl;
           cerr << w -> getErrorText() << endl;
           abort = true;
         }
     }

   for(int i = 0;
       (i < (destMan -> getNoOfUsedWorkers(destArray->getSize()))) && 
         !abort; i++)
     {
       DServer* w = destMan -> getServerByID(i);
       if (w -> hasError())
         {
           cerr << "ERROR: DShuffle encountered errors:\n"
                << endl;
           cerr << w -> getErrorText() << endl;
           abort = true;
         }
     }

   int dest_server_no = destMan -> getNoOfMultiWorkers(destSize);
   int src_server_no = sourceMan -> getNoOfMultiWorkers(srcSize);

   if (abort)
     {
       cout << "ERROR! CLEAN IT UP!" << endl;

       //Close additional connections
       for(int k = 0; k<dest_server_no;k++)
         destArray ->getServerManager()->
           getServerByID(k)->DestroyChilds();
   
       //Close additional connections
       for(int k = 0; k<src_server_no;k++)
         sourceMan->getServerByID(k)->DestroyChilds();
       
       destArray -> SetDefined(false);
       result.addr = destArray;
       return 0;
     }
   
   cout << "DShuffle finished! " << endl;

   //Close additional connections
   destArray -> destroyAnyChilds();
   sourceArray -> destroyAnyChilds();
   
   result.addr = destArray;

   return 0;
}

const string shuffleSpec3 =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>((darray t) ((map t int)  int (rel(tuple("
      "[Server:string, Port: int]))) ) -> darray t</text>"
      "<text>_ dshuffle [fun, _ , _]</text--->"
      "<text>Redistributes a distirbuted array.</text--->"
"<text>query darr dschuffle [randint(3), 3,server_rel]</text--->))";

Operator shuffle3 (
      "dshuffle",
      shuffleSpec3,
      shuffleFun<3>,
      Operator::SimpleSelect,
      shuffleTypeMap<3> );

const string shuffleSpec2 =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>((darray t) ((map t int)  int )) -> darray t</text>"
      "<text>_ dshuffle2 [fun, _ , _]</text--->"
      "<text>Redistributes a distirbuted array.</text--->"
      "<text>query darr dschuffle2 [randint(3), 3]</text--->))";

Operator shuffle2 (
      "dshuffle2",
      shuffleSpec2,
      shuffleFun<2>,
      Operator::SimpleSelect,
      shuffleTypeMap<2> );

const string shuffleSpec1 =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>((darray t) ((map t int))) -> darray t</text>"
      "<text>_ dshuffle1 [fun, _ , _]</text--->"
      "<text>Redistributes a distirbuted array.</text--->"
      "<text>query darr dschuffle1 [randint(3)]</text--->))";

Operator shuffle1 (
      "dshuffle1",
      shuffleSpec1,
      shuffleFun<1>,
      Operator::SimpleSelect,
      shuffleTypeMap<1> );

/*

5.9 Operator loop

*/
template< int dim>
static ListExpr loopTypeMap(ListExpr args)
{
  
  //cout << "LoopTM " << dim << " :" << nl -> ToString(args) << " -> ";
  ListExpr errRes = nl->SymbolAtom(Symbol::TYPEERROR());

  NList m_args(args);
  NList params;

   if(m_args.length() == dim + 1)
    {
      NList mapdesc = m_args.elem(dim + 1);

      if (mapdesc.first().length() != dim + 2 )
        {
          return  errRes;
        }

      if (mapdesc.first().first() != NList(Symbol::MAP()))
        {
          return  errRes;
        }
      if (mapdesc.first().elem(dim+2) == NList(Symbol::TYPEERROR()))
        {
          return  errRes;
        }

      params = NList(dim).enclose();

      params.append(NList(NList(NList(mapdesc.
                                      second().
                                      elem(dim + 2).
                                      convertToString(),
                                      true, true))));

      for (unsigned int i = 0; i < dim; ++i)
        {
          NList darraydesc = m_args.elem(i + 1);

          // check for correct types
          if (darraydesc.length() != 2)
            {
              return  errRes;
            }
          if(darraydesc.first() == (DArray::BasicType()))
            {
              return  errRes;
            }

          //check, if darray and map are equal
          if (darraydesc.first().second() != 
              mapdesc.first().elem(i + 2))
            {
              return  errRes;
            }
          params.append(NList
                        (NList
                         (mapdesc.second().
                          elem(i + 2).first().
                             convertToString(), true)));
        }

      NList res =
        NList(NList(Symbols::APPEND()),
              params,

              NList(NList(DArray::BasicType()),
                    mapdesc.first().elem(dim + 2)));

      //cout << res.convertToString() << endl;
      return res.listExpr();
    }

   return errRes;
}

template< int dim>
static int loopValueMap
(Word* args, Word& result, int message, Word& local, Supplier s)
{
   result = qp->ResultStorage(s);

   SecondoCatalog* sc = SecondoSystem::GetCatalog();
   ListExpr type = sc->NumericType(nl->Second((qp->GetType(s))));
   string command = ((FText*)args[dim + 2].addr)->GetValue();

   ZThread::ThreadedExecutor exec;
   DServer* server;

   int size = 0;
   ListExpr serverList;
   vector<string> from;

   string rpl = "!";
   for (int i = 0; i < dim; i ++)
     {
       DArray* alt = (DArray*)args[i].addr;
       if (alt == NULL ||
           !(alt -> IsDefined()))
         {
           cerr << "ERROR: Input DArray is not defined!" << endl;
           ((DArray *) result.addr) -> SetDefined(false);
           return 0;
         }
       if (size == 0)
         {
           size = alt -> getSize();
         }
       
       size = min(size, alt -> getSize());

       serverList = alt->getServerList();

       if (nl -> IsEmpty(serverList))
         {
           cerr << "ERROR: Input DArray worker list is empty!"
                << endl;
           ((DArray *) result.addr) -> SetDefined(false);
           return 0;
         }


       from.push_back(alt->getName());
       // TODO: need to compare server lists!

       string elementname = 
         ((CcString*)args[i+ dim + 3].addr) -> GetValue();
       command = stringutils::replaceAll(command, elementname, rpl);
       rpl += "!";
     }

   //cout << "CMD:" << command << endl;

   string name = getArrayName(DArray::no);

   DArray* neu = ((DArray*)(result.addr));
   neu ->initialize(type,
                    name,
                    size,
                    serverList);

   ZThread::ThreadedExecutor poolEx;
      
   DServerCmdExecuteParam execParam(command,
                                    from);

   
   bool containsError = false;

   if (neu -> runCommandThreaded<DServerCmdExecute,
                                 DServerCmdExecuteParam>(poolEx,
                                                         execParam))
     {
       // all ok
       for (int i = 0; 
            i < neu -> getServerManager() -> 
              getNoOfUsedWorkers(neu -> getSize()); ++i)
         {
           DServer *w = neu -> getServerManager() -> getServerByID(i);
           if (w -> hasError())
             {
               cerr << "ERROR: Worker " << i << " reports:" << endl
                    << w -> getErrorText() << endl;
               containsError = true;
             }
     
         }
     }
   else
     {
       containsError = true;
       cerr << "ERROR: loop command reported an error" << endl;
     }
   
                                    
   if (containsError)
     {
       ((DArray*)(result.addr)) -> SetUndefined();
     }

   return 0;

}

const string loopSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>((darray t) (map t u)) -> (darray u)</text--->"
      "<text>_ dloopa [ fun ]</text--->"
      "<text>Evaluates each element with a function, "
      "that needs to be given"
      "as paremeter function </text--->"
      "<text>query plz_a20 dloop[. count]</text--->))";
struct loopaSpec : OperatorInfo {
  loopaSpec() : OperatorInfo() {
    name = "dloopa";
    signature = "((darray t) (darray u) (map t u r)) -> (darray r)";
    syntax = "_ _ dloopa [ fun ]";
    meaning =
      "Evaluates each element of each darray with a function, "
      "that needs to be given as paremeter function";
  }
};


Operator loopA (
      "dloop",
      loopSpec,
      loopValueMap<1>,
      Operator::SimpleSelect,
      loopTypeMap <1>);

Operator dloopA (loopaSpec(),
                 loopValueMap<2>,
                 loopTypeMap <2>);

/*

5.10.1 Type Operator DELEMENT
(derived from Operator ELEMENT of the ArrayAlgebra)

*/

ListExpr delementTypeMap( ListExpr args )
{
  //cout << "DelTM:" << nl -> ToString(args) << " -> ";
   if(nl->ListLength(args) >= 1)
   {
      ListExpr first = nl->First(args);
      if (nl->ListLength(first) == 2)
      {
         if (nl->IsEqual(nl->First(first), DArray::BasicType()))
         {
            ListExpr res =  nl->Second(first);
            //cout << nl -> ToString(res) << endl;
            return res;
         }
      }
   }
   return nl->SymbolAtom(Symbol::TYPEERROR());
}

const string DELEMENTSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Remarks\" )"
    "( <text>((array t) ... ) -> t</text--->"
      "<text>type operator</text--->"
      "<text>Extracts the type of the elements "
      "from a darray type given "
      "as the first argument.</text--->"
      "<text>not for use with sos-syntax</text---> ))";

Operator dElementA (
      "DELEMENT",
      DELEMENTSpec,
      0,
      Operator::SimpleSelect,
      delementTypeMap );

/*

5.10.2 Type Operator DELEMENT2

*/

ListExpr delement2TypeMap( ListExpr args )
{
  //cout << "Del2TM:" << nl -> ToString(args) << " -> ";
   if(nl->ListLength(args) >= 2)
   {
      ListExpr second = nl->Second(args);
      if (nl->ListLength(second) == 2)
      {
         if (nl->IsEqual(nl->First(second), DArray::BasicType()))
         {
            ListExpr res = nl->Second(second);
            cout << nl -> ToString(res) << endl;
            return res;
         }
      }
   }
   return nl->SymbolAtom(Symbol::TYPEERROR());
}

const string DELEMENT2Spec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Remarks\" )"
    "( <text>((array t) ... ) -> t</text--->"
      "<text>type operator</text--->"
      "<text>Extracts the type of the elements "
      "from a darray type given "
      "as the second argument.</text--->"
      "<text>not for use with sos-syntax</text---> ))";

Operator dElementA2 (
      "DELEMENT2",
      DELEMENT2Spec,
      0,
      Operator::SimpleSelect,
      delement2TypeMap );

/*

5.10.2 Type Operator DRELATION

*/
ListExpr dRelTypeMap( ListExpr inArgs )
{
  NList args (inArgs);
  //cout << "DRELTM:" << args.convertToString() << " -> ";
  args = args.first();
  
  if(args.length() >= 2)
    {
   
      if (args.first().isSymbol(DArray::BasicType()))
        {
          NList second = args.second();
         
          if (second.length() == 2 &&
              second.first().isSymbol(Relation::BasicType()))
            {
              NList res = second.second();
              //cout << res.convertToString() << endl;
              return res.listExpr();
            }
        }
    }

  ErrorReporter::ReportError(
            "Only DArrays of type Relation allowed here!");
   return nl->SymbolAtom(Symbol::TYPEERROR());
}

const string DRELSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Remarks\" )"
    "( <text>DRELATION</text--->"
      "<text>type operator</text--->"
      "<text>Internal operator for darray. "
      "Is replaced by current index</text--->"
      "<text>not for use with sos-syntax</text---> ))";

Operator drel (
      "DRELATION",
      DRELSpec,
      0,
      Operator::SimpleSelect,
      dRelTypeMap );
/*

5.10.2 Type Operator d[_]idx

*/

ListExpr dindexTypeMap( ListExpr inArgs )
{
  NList args (inArgs);
  //cout << "dindex:" << args.convertToString() << " -> ";
  if(args.length() == 0)
   {
     NList result = NList(NList(CcInt::BasicType()));
     //cout << result.convertToString() << endl;
     return result.listExpr(); 
   }
   return nl->SymbolAtom(Symbol::TYPEERROR());
}


static int dindexValueMap
(Word* args, Word& result, int message, Word& local, Supplier s)
{
   result = qp->ResultStorage(s);
   ((CcInt*)result.addr)->Set(1);
   return 0;
}

const string DINDEXSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Remarks\" )"
    "( <text>d_idx() -> int</text--->"
      "<text>type operator</text--->"
      "<text>Internal operator for darray. "
      "Is replaced by current index</text--->"
      "<text>not for use with sos-syntax</text---> ))";

Operator dIndex (
      "d_idx",
      DINDEXSpec,
      dindexValueMap,
      Operator::SimpleSelect,
      dindexTypeMap );

/*
5.11 Operator ~dtie~

The operator calculates a single "value" of an
darray by evaluating the elements of an darray
with a given function from left to right, e.g.

dtie ( (a1, a2, ... , an), + ) = a1 + a2 + ... + an

The formal specification of type mapping is:

---- ((darray t) (map t t t)) -> t
----

*/
static ListExpr
dtieTypeMap( ListExpr args )
{
  //cout << "DTieTM:" <<  nl -> ToString(args) << " -> ";
  if (nl->ListLength(args) == 2)
  {
    ListExpr arrayDesc = nl->First(args);
    ListExpr mapDesc = nl->Second(args);

    if ((nl->ListLength(arrayDesc) == 2)
        && (nl->ListLength(mapDesc) == 4))
    {
      if (nl->IsEqual(nl->First(arrayDesc), DArray::BasicType())
          && nl->IsEqual(nl->First(mapDesc), Symbol::MAP()))
      {
        ListExpr elementDesc = nl->Second(arrayDesc);

        if (nl->Equal(elementDesc, nl->Second(mapDesc))
            && nl->Equal(elementDesc, nl->Third(mapDesc))
            && nl->Equal(elementDesc, nl->Fourth(mapDesc)))
        {
          //cout<< "DTie ResultType:"
          // << nl -> ToString(elementDesc) << endl;
          return elementDesc;
        }
      }
    }
  }

  return nl->SymbolAtom(Symbol::TYPEERROR());
}

static int
dtieFun( Word* args, Word& result, int message, 
         Word& local, Supplier s )
{
  result = qp->ResultStorage(s);

  DArray* array = ((DArray*)args[0].addr);

  if (!array || !(array -> IsDefined()))
    {
      cerr << "ERROR: Input DArray is not defined correctly!" << endl;
      ((Attribute *)result.addr) -> SetDefined(false);
      return 0;
    }

  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  ArgVectorPointer funargs = qp->Argument(args[1].addr);
  Word funresult;

  ListExpr typeOfElement = sc->NumericType(qp->GetType(s));

  int algebraId;
  int typeId;
  extractIds(typeOfElement, algebraId, typeId);

  const int n = array->getSize();

  if (n == 0)
    {
      cerr << "ERROR: DArray is empty !" << endl;
      result =  qp->ResultStorage(s); 
      ((Attribute *)result.addr) -> SetDefined(false);
      return 0;
    }

  array->refresh();

  if ( !(array -> IsDefined()))
    {
      cerr << "ERROR: Could not transfer data of input DArray !" << endl;
      result = qp->ResultStorage(s); 
      ((Attribute *)result.addr) -> SetDefined(false);
      return 0;
    }

  //copy the first element
      Word partResult =
    (am->CloneObj(algebraId, typeId))(typeOfElement,
                                      (Word)array->get(0));

      
  Attribute* a = static_cast<Attribute*>
    ((am->Cast(algebraId,typeId))(partResult.addr));
     
  if (a == NULL  ||
      !(a -> IsDefined()))
    {
      cerr << "ERROR: Partial result is not defined1!" << endl;
      result = qp->ResultStorage(s); 
      ((Attribute *)result.addr) -> SetDefined(false);
      return 0;
    }
      
 
  for (int i=1; i<n; i++) 
    {
      Word ielem =
        (am->CloneObj(algebraId, typeId))(typeOfElement,
                                          (Word)array->get(i));
     
      a = static_cast<Attribute*>
        ((am->Cast(algebraId,typeId))(ielem.addr));
      
      if ( a == NULL  ||
          !(a -> IsDefined()))
        {
          cerr << "ERROR: Partial result is not defined2!" << endl;
          result = qp->ResultStorage(s); 
          ((Attribute *)result.addr) -> SetDefined(false);
          return 0;
        }
                                          
    //copy the next element

    (*funargs)[0] = partResult;
    (*funargs)[1] = ielem;

    // calculate the intermediate result;
    qp->Request(args[1].addr, funresult);

    if (funresult.addr != partResult.addr) {

      // delete the previous intermediate result
      (am->DeleteObj(algebraId, typeId))
        (typeOfElement, partResult);

      // assign the next intermediate result
      partResult =
      Array::genericClone(algebraId, typeId, 
                          typeOfElement, funresult);
    }
    // delete the current element
    (am->DeleteObj(algebraId, typeId))(typeOfElement,ielem);
  }
  // In the next statement the (by the Query Processor) 
  // provided place forthe result is not used in order 
  // to be flexible with regard to the result type.

  result.addr = partResult.addr;

  return 0;
}

const string dtieSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>((darray t) (map t t u)) -> u</text--->"
      "<text>_ tie [ fun ]</text--->"
      "<text>Calculates the \"value\" of an darray "
      "evaluating the elements of "
   "the darray with a given function from left to right.</text--->"
      "<text>query ai tie[fun(i:int,l:int)(i+l)]</text---> ))";

Operator dtie(
      "dtie",
      dtieSpec,
      dtieFun,
      Operator::SimpleSelect,
      dtieTypeMap );


/*
5.12 Operator ~dsummarize~

The operator ~dsummarize~ provides a stream of tuples from a 
darray of relations. For this purpose, the operator scans 
all relations beginning with the first relation of the array.

The formal specification of type mapping is:

---- ((darray (rel t))) -> (stream t)

     at which t is of the type tuple
----

Note that the operator ~dsummarize~ is not exactly inverse
to the operator ~ddistribute~ because the indexy of the relation
is not appended to the attributes of the outgoing tuples. If the
darray has been constructed by the operator ~ddistribute~ the 
order of the resulting stream in most cases does not correspond
to the order of the input stream of the operator ~ddistribute~.

*/
static ListExpr
dsummarizeTypeMap( ListExpr args )
{
  //cout << "dsummarizeTM:" << nl -> ToString(args) << endl;
  if (nl->ListLength(args) == 1)
  {
    ListExpr arrayDesc = nl->First(args);

    if (nl->ListLength(arrayDesc) == 2
        && nl->IsEqual(nl->First(arrayDesc), DArray::BasicType()))
    {
      ListExpr relDesc = nl->Second(arrayDesc);

      if (nl->ListLength(relDesc) == 2
          && nl->IsEqual(nl->First(relDesc), Relation::BasicType()))
      {
        ListExpr tupleDesc = nl->Second(relDesc);
        if (nl->IsEqual(nl->First(tupleDesc), Tuple::BasicType()))
        {
          ListExpr ret =  nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                                          nl->Second(relDesc));
          //out << nl -> ToString(ret) << endl;
          return ret;
        }
      }
    }
  }

  ErrorReporter::ReportError(
     "dsummarize: Input type darray( rel( tuple(...))) expected!");
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

class DArrayIteratorRefreshThread : public ZThread::Runnable
{
 
public:
  DArray *d;
  TFQ tfqOut;
  ThreadedMemoryCounter *memCntr;

  DArrayIteratorRefreshThread() 
  : ZThread::Runnable()  {}

  void run()
  {
    d -> refresh(tfqOut, memCntr);
    
  }
  
  ~DArrayIteratorRefreshThread() { }
};

static int
dsummarizeFun( Word* args, Word& result, int message, 
               Word& local, Supplier s )
{
  //cout << "dsummarizeFun" << endl;
  struct DArrayIterator
  {
    private:
    DArray *da;
    TFQ m_tfqOut;
    ZThread::ThreadedExecutor m_exe;
    bool m_error;
    ThreadedMemoryCounter m_memCntr;

    public:
    DArrayIterator(DArray* d, size_t allowed_mem_size) 
  : da(d)
  , m_tfqOut(new TupleFifoQueue())
  , m_memCntr(allowed_mem_size)
    {
      assert (da != NULL);
      m_error = false;

      if ( d == 0 ||
           !(d -> IsDefined()) ||
           !(d -> getServerManager() -> checkServers(false)))

        {
          //cout << "DSFI ERROR!" << endl;
          m_error = true;
        }
      else
        {
          DArrayIteratorRefreshThread *darrRefresh = 
            new DArrayIteratorRefreshThread();
          //cout << "DSFI init!" << endl;

          darrRefresh -> d = d;
          darrRefresh -> tfqOut = m_tfqOut;

          // see below, why this is NULL
          darrRefresh -> memCntr = NULL; // &m_memCntr

          //start collector thread
          m_exe.execute(darrRefresh);

          // all Done!;

          // need to first store all the data on the master
          // this is really bad, since we cannot use threads
          // for streams nor memory allocation checking.
          // the reason is, that post commands (eg. count)
          // are not implemented threadsave!

          m_exe.wait(); 
          // all Done!;
        }
    }

    ~DArrayIterator()
    {
      delete m_tfqOut;
    }

    Tuple* getNextTuple()
    {
      if (m_error)
        return 0;

      //cout << "NEXT TUPLE:"  << endl;
      // no threads are running, no need for DBAccessGuard
      Tuple* t = m_tfqOut -> get();

      //
      // we cannot  check for allocated memory!
      //if (t != NULL)
      //        {
      //          cout << "TFQ GET:" << t -> GetSize() << endl;
      //          m_memCntr.put_back(t -> GetSize());
      //        }
      
      return t;
    }
     
    bool hasError() const { return m_error; }
    };
 
  DArrayIterator* dait = 0;
  dait = (DArrayIterator*)local.addr;
   
  switch (message) {
    case OPEN : {
      //cout << "OPEN" << endl;
      DArray *da = (DArray*)args[0].addr;
      assert (da != NULL);

      dait = 
        new DArrayIterator(da, qp->GetMemorySize(s) * 1024 * 1024);
      local.addr = dait;
      return 0;
    }
    case REQUEST : {

      //cout << "RQUEST" << endl;
      if (dait -> hasError())
        {
          //cout << " -> send Cancel1" << endl;
          result.setAddr(0);
          return CANCEL;
        }

      Tuple* t = dait->getNextTuple();
      
      if (t != 0) {
        result = SetWord(t);
        //DBAccessGuard::getInstance() -> T_DeleteIfAllowed(t);
        return YIELD;
      }
      //cout << "RQUEST DONE" << endl;
      //cout << " -> send Cancel2" << endl;
      result.setAddr(0);
      return CANCEL;
    }
    case CLOSE : {
      //cout << "CLOSE" << endl;
      if(local.addr)
      {
        dait = (DArrayIterator*)local.addr;
        delete dait;
        local.setAddr(0);
      }      
      qp->Close(args[0].addr);
      return 0;
    }
    default : {
      return 0;
    }
  }
  return 0;
}

const string dsummarizeSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>((darray (rel t))) -> (stream t)</text--->"
      "<text>_ dsummarize</text--->"
 "<text>Produces a stream of the tuples from all relations in the "
      "darray.</text--->"
      "<text>query prel dsummarize consume</text---> ))";

Operator dsummarize (
      "dsummarize",
      dsummarizeSpec,
      dsummarizeFun,
      Operator::SimpleSelect,
      dsummarizeTypeMap );

/*
5.14 Operator ~checkworkers~

The operator ~ceckworkers~ takes a relation of 
type host:string, port:int and checks, if there 
exists a running instance of SecondoMonitor at
the specified host, listening at the specified port
It also checks, if the database distributed is available.

*/
static bool 
sendCommandToWorker(Socket *inServer,
                    const string inCmd,
                    const string inExpectedResponse,
                    string &outMsg)
{
  if(inServer == 0 || !inServer->IsOk())
    {      
      outMsg = "Cannot connect to worker";
      if (inServer != 0)
        {
          outMsg += ":" + inServer -> GetErrorText();
        }

      return false;
    }

  iostream &iosock = inServer -> GetSocketStream();

  // check db distributed available
  if (!iosock.good())
    {
      outMsg = "(5) Communication is blocked! Restart Worker";
      return false;
    }

  // cout << "OUT:" << "<Secondo>" << endl << "1" << endl 
  //       << inCmd << endl 
  //      << "</Secondo>" << endl;

  iosock << "<Secondo>" << endl << "1" << endl 
         << inCmd << endl 
         << "</Secondo>" << endl;

   if (!iosock.good())
    {
      outMsg = "(6) Communication is blocked! Restart Worker";
      return false;
    }

   string line;
  getline( iosock, line );
  //cout << "GOT:" << line << endl;

  bool foundResult = inExpectedResponse.empty()? true : false;
  bool gotError = false;
  stringstream allLines;
  if(line=="<SecondoResponse>")
    {
      do
        {
          if (!iosock.good())
            {
              outMsg = 
                "(7) Communication is blocked! Restart Worker";
              return false;
            }

          getline( iosock, line );
          if (line.find("</SecondoResponse>") == string::npos)
            allLines << line << endl;


          if(line.find("ERROR") != string::npos ||
             line.find("Error") != string::npos ||
             line.find("error") != string::npos)
            { 
              //cout << "  -> GOT ERROR!";
              gotError = true;
            }
          else if (line.find(inExpectedResponse) != string::npos)
            {
              //cout << "  -> GOT IT!";
              foundResult = true;
            }
                        
        }
      while(line.find("</SecondoResponse>") == string::npos);
    }
  else 
    outMsg = "Unexpected response from worker";
  
  if (gotError)
    {
      outMsg = allLines.str();
      //cerr << "GOT ERROR:" << outMsg;
      foundResult = false;
    }
  return foundResult;
}

static bool 
openConnection(Socket *inServer,
               string &outMsg)
{ 
  string line;
  if(inServer == 0 || !inServer->IsOk())
    {      
      outMsg = "Cannot connect to worker";
      if (inServer != 0)
        {
          outMsg += ":" + inServer -> GetErrorText();
        }

      return false;
    }

  iostream &ioSock = inServer->GetSocketStream();
      
  if (!inServer -> IsOk())
    {
      outMsg = "Cannot access worker socket"; 
      return false;
    }
  do
    {
      if (!ioSock.good())
        {
          outMsg = "(1):";
          switch (ioSock.rdstate())
           {
             default:
             outMsg += " nospecified";
             break;
           }
          outMsg += " Communication is blocked! Restart Worker:";
          return false;
        }
      
      getline( ioSock, line );
      
    } while (line.empty());
      
  bool startupOK = true;

  if(line=="<SecondoOk/>")
    {
      if (!ioSock.good())
        {
          outMsg = "(2) Communication is blocked! Restart Worker";
          return false;
        }
      ioSock << "<Connect>" << endl << endl 
             << endl << "</Connect>" << endl;
      if (!ioSock.good())
        {
          outMsg = "(3) Communication is blocked! Restart Worker";
          return false;
        }
      getline( ioSock, line );
          
      if( line == "<SecondoIntro>")
        {
          do
            {
              if (!ioSock.good())
                {
                  outMsg =
                    "(4) Communication is blocked! Restart Worker";
                  return false;
                }
              getline( ioSock, line);
              
            }  while(line != "</SecondoIntro>");
            
              
        }
      else 
        startupOK = false;
          
    }
  else 
    startupOK = false;
   
  if (!startupOK)
    {
      outMsg = "Unexpected response from worker";
      return false;
    }

  if (!(inServer -> IsOk()))
    { 
      outMsg = "Cannot Connect to Worker";
      return false;
    } // if (!(inServer -> IsOk()))

  return startupOK;
}

static void closeConnection(Socket *ioServer)
{
  iostream &ioSock = ioServer -> GetSocketStream();
  ioSock << "<Disconnect/>" << endl;
  ioServer->Close();
  
}

static bool
checkWorkerRunning(const string &host, int port,  
                   const string &cmd, string &msg)
{
  cout << "checking worker on " << host << ":" << port << endl;
  // check worker running
  string line;

  Socket* server = Socket::Connect( host, int2Str(port), 
                                    Socket::SockGlobalDomain,
                                    5,
                                    1);

  if (server == 0)
    {
      msg = "Unable to open connection to Worker!";
      return false;
    }
  if (!openConnection(server, msg))
    {
      if (server != 0)
        {
          server->Close();
          delete server;
        }
      return false;
    }

  if (!sendCommandToWorker(server, 
                           "open database distributed",
                           "", 
                           msg)) 
    {
      if (server != 0)
        {
          server->Close();
          delete server;
        }
      return false;
    }

  if (!sendCommandToWorker(server, 
                           cmd,
                           "", 
                           msg)) 
    {
      if (server != 0)
        {
          server->Close();
          delete server;
        } 
      msg = 
        "Database \"distributed\" in use";
      return false;
    }

  closeConnection(server);
  delete server;
  server=0;
  return true;
}

static ListExpr
checkWorkersTypeMap( ListExpr args )
{
  //cout << "CheckWorkersTM" << nl -> ToString(args) << " -> ";
  NList myargs = NList(args).first();

  if (myargs.hasLength(2) && myargs.first().isSymbol(sym.STREAM()))
    {
      NList tupTypeL( myargs.second().second());

      if (tupTypeL.length() >= 2)
        { 
          if (!tupTypeL.
                first().
                  second().
                    isSymbol(CcString::BasicType()))
            {
              return 
                NList::typeError(
                       "First attribute must be the server name");
            } 
          if (!tupTypeL.
                second().
                  second().
                    isSymbol(CcInt::BasicType()))
            {
              return 
                NList::typeError(
                       "Second attribute must be the port number");
            }
          //The return-type of checkWorkers is 
          // a stream of workers each appended w/ a status msg;
          NList app (NList("Status"), NList(CcString::BasicType())); 

          NList tuple = 
            NList(tupTypeL.first(), tupTypeL.second(), app);

          NList result = NList().tupleStreamOf(tuple);
          //cout << result.convertToString() << endl;
          return result.listExpr(); 
        }
      return 
        NList::typeError("Tuple stream has not enough attributes");
    }
  
  return 
    NList::typeError(
       "Expecting a stream of tuples(Server:string, port:int)");
}

struct CwLocalInfo
{
  TupleType *resTupleType;
  vector<pair<string, int> > hostport;
  string cmd;

  CwLocalInfo(ListExpr inTT){ resTupleType = new TupleType(inTT); }
  ~CwLocalInfo() { resTupleType -> DeleteIfAllowed(); }
};

static int
checkWorkersFun (Word* args, Word& result, 
                 int message, Word& local, Supplier s)
{
  CwLocalInfo *localInfo;
  Tuple *curTuple;
  Tuple *resTuple;
  Word cTupleWord;

  switch (message)
    {
    case OPEN:
      {
        cout << "Checking workers" << endl;
        qp->Open(args[0].addr);
      
        ListExpr resultTupleNL = GetTupleResultType(s);

        localInfo = new CwLocalInfo(nl->Second(resultTupleNL));
      
        localInfo -> cmd = "let test1 = \"test\"";

        local.setAddr(localInfo);
        return 0;
      }
  
    case REQUEST:
      {
        if (local.addr == 0)
          return CANCEL;

        localInfo = (CwLocalInfo*)local.addr;

     
        qp->Request(args[0].addr, cTupleWord);
        if (qp->Received(args[0].addr))
          {
            curTuple = (Tuple*)cTupleWord.addr;
            string host =
              ((CcString*)(curTuple->GetAttribute(0)))->GetValue();
            int port =
              ((CcInt*)(curTuple->GetAttribute(1)))->GetValue();
            curTuple -> DeleteIfAllowed();
            string msg = "OK"; 
            
            bool retVal = checkWorkerRunning(host, port, 
                                             localInfo -> cmd, msg);
            if (!retVal)
              {
                msg = "ERROR: " + msg + "!";
              }
     
            resTuple = new Tuple(localInfo->resTupleType);
            resTuple->PutAttribute(0, new CcString(true, host));
            resTuple->PutAttribute(1, new CcInt(true, port));
            resTuple->PutAttribute(2, new CcString(true, msg));
        
            localInfo -> 
              hostport.push_back(
                               make_pair<string, int>(host, port));
        
            result.setAddr(resTuple);
            return YIELD;
          }
        else
          return CANCEL;
      }
    case CLOSE:
      {
        if (local.addr == 0)
          return CANCEL;

        localInfo = (CwLocalInfo*)local.addr;
        localInfo -> cmd = "delete test1";
    
        while( !localInfo -> hostport.empty() ) 
          {
            string msg = "OK";
            checkWorkerRunning(localInfo -> hostport.back().first, 
                               localInfo -> hostport.back().second, 
                               localInfo -> cmd, msg);
            
            localInfo -> hostport.pop_back();
          }
        
        delete localInfo;
        local.setAddr(0);
        qp->Close(args[0].addr);
        return 0;
      }
    }
 
  return 0;
}

const string checkWorkersSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "(<text>((stream (tuple ((Server string) (Port int))))) ->  "
    "((stream (tuple ((Server string) "
    "(Port int) (Status string)))))</text--->"
      "<text>_ check_workers</text--->"
      "<text>checks workers, if running and database 'distributed'"
      "exists</text--->"
      "<text>query workers check_workers</text---> ))";

Operator checkWorkers (
      "check_workers",
      checkWorkersSpec,
      checkWorkersFun,
      Operator::SimpleSelect,
      checkWorkersTypeMap );

/*
5.15 Operator ~startup~

The operator ~startup~ takes  host, port, configfile, createDistrDBFlag,
user, password, bindir, and starts a SecondoServer at the specified location.
It returns true, if server start was successful 

*/

static bool createRemoteDistrDB(string host, int port)
{
  bool retVal = true;

  cout << " -> Creating Remote DB 'distributed' "
       << "on host " << host << " port:" << port << endl;

  DSecondoMonitorCommunication secondoMonitor( host, int2Str(port));
  
  if (retVal &&
      !secondoMonitor.openConnection())
    {
      cerr << "ERROR:" << secondoMonitor.getErrorText() << endl;
      retVal = false;
    }

  if (retVal &&
      !secondoMonitor.sendSecondoCmdToWorkerSOS("delete database distributed")) 
    {
      cerr << "ERROR:";
      if (secondoMonitor.hasCmdError())
        cerr << secondoMonitor.getCmdErrorText();
      else
        cerr << secondoMonitor.getErrorText();
      cerr << endl;

      retVal = false;
    }

  if (retVal &&
      !secondoMonitor.sendSecondoCmdToWorkerSOS("create database distributed")) 
    {
      cerr << "ERROR:";
      if (secondoMonitor.hasCmdError())
        cerr << secondoMonitor.getCmdErrorText();
      else
        cerr << secondoMonitor.getErrorText();
      cerr << endl;

      retVal = false;
    }
  
  if (retVal)
    cout << "  ... successful";
  else
    cout << "  ... failed";
  cout << endl;

  return retVal;
}

static ListExpr
startupTypeMap( ListExpr args )
{
  NList myargs (args);
  //cout << "StartupTM:" << myargs.convertToString() << " -> ";
  if (myargs.length() >= 2  && myargs.length() <= 7)
    {
      if (!myargs.first().isSymbol(CcString::BasicType()))
        {
          return NList::typeError(
                    "First parameter must be the server name");
        } 
      if (!myargs.second().isSymbol(CcInt::BasicType()))
        {
          return NList::typeError(
                   "Second parameter must be the port number");
        }
      if (!myargs.third().isSymbol(CcString::BasicType()))
        {
          return 
            NList::typeError
   ("Third parameter must be the name of the SecondoConfigFile");
        }

      NList result = NList(NList(CcBool::BasicType()));
      //cout << result.convertToString() << endl;
      return result.listExpr(); 
    }
  
  return 
    NList::typeError
    ("Expecting at least server name, port and configuration file");
}

static int
startupFun (Word* args, Word& result, int message, 
            Word& local, Supplier s)
{ 
  result = qp->ResultStorage(s); 
  int args_cnt = qp->GetNoSons(s);
  bool ret_val = true;

  string host = ((CcString*)args[0].addr)->GetValue();
  int port = ((CcInt*)args[1].addr)->GetValue();
  string secConf = ((CcString*)args[2].addr)->GetValue();
  bool createDB = false;
  
  // checking optional parameters
  // don't enter breaks here!
  switch (args_cnt)
    {
    case 5:
      // something to do ...
    case 4:
      createDB = ((CcBool*)args[3].addr)->GetValue();
    }

  if (host.empty())
    {
      cerr << "ERROR: Please specify the host" << endl;
      ret_val = false;
    }

  else if (secConf.empty())
    {
      cerr << "ERROR: Please specify the SecondoConfig file" << endl;
      ret_val = false;
    }
  
  if (ret_val)
    {
      cout << "Starting: SecondoMonitor on host "
           << host << ":" << port
           << " with " << secConf << endl;

      string lckfile = "/tmp/SM_" + int2Str(port) + ".lck";
      string devnull = "< /dev/null > /dev/null 2>&1";

      string exportConf = 
        " export SECONDO_CONFIG=${HOME}/secondo/bin/" + 
        secConf + ";";

      //string secondoBinDir = 
      // " export DISTR_SECONDO_BINDIR=/opt/psec/achmann-secondo/bin/;";

      string lckfileexist = "if [ -r " + lckfile +
        " ]; then echo 0; else echo 1; fi;";
      string cddir = ". .bashrc; cd secondo/bin; ";
      string cmd = 
        cddir + 
        lckfileexist + 
        exportConf + 
        //secondoBinDir +
        " ./StartMonitor.remote " + devnull + " & ";
 
      string retVal;
      bool success = RunCmdSSH(host, cmd, retVal);
      if (!success ||
          retVal.empty() || retVal[0] != '1')
        {
          cerr << "ERROR: Lock file '" + lckfile + "' exists!" 
               << endl;
          ret_val = false;
        }
      else
        {
          string waitForLockFileCnt = "CNT=0 ;";
          string waitForLockFileWhile = " until ";
          string lckFileExistCond = "[ -e " + lckfile + " ]";
          string waitForLockFileWhileCond =
            "[ ${CNT} -gt 10 ] || " + lckFileExistCond + ";";
          string waitForLockFileDo = "do sleep 1;";
          string waitForLockFileWhileDone = "let CNT=CNT+1; done; ";
          string retValCond = " if " + lckFileExistCond + 
            "; then echo 1; else echo 0; fi;";


          cmd = 
            waitForLockFileCnt + 
            waitForLockFileWhile + 
            waitForLockFileWhileCond + 
            waitForLockFileDo +
            waitForLockFileWhileDone +
            retValCond; 

          success = RunCmdSSH(host, cmd, retVal);
          if (!success ||
              retVal.empty() || retVal[0] != '1')
            {
              cerr << "ERROR: Lock file '" + lckfile + "' not created!" 
                   << endl;
              cout << "RET:" << retVal << endl;
              ret_val = false;
            }

          if (//ret_val && 
              createDB)
            {
              createRemoteDistrDB(host, port);
            }
        }
    }
  cout << endl;
  ((CcBool*) (result.addr))->Set(true, ret_val);

  return 0;
} 
 
const string startupSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "(<text>startup (Server:string , Port:int,"
    "SecondoConf:string, CreateDistrDB: bool) "
    "->  bool</text--->"
      "<text>startup (_, _, _)</text--->"
      "<text>Starts a worker on a given host. On that host there "
      "must be a directory ${HOME}/secondo/bin. In this directory "
      "there must exists the StartMonitor.remote script. The given "
      "SecondoConfig.ini file must exist there, too, with "
      "a matching SecondoPort number."
      "If the optional parameter CreateDistrDB is set to true, the"
      "database 'distributed' will be restored</text--->"
      "<text>startup (\"localhost\", 1234, "
      "\"SecondoConfig.ini\" )</text---> ))";

Operator startUp (
      "startup",
      startupSpec,
      startupFun,
      Operator::SimpleSelect,
      startupTypeMap );

/*
5.16 Operator ~shutdown~

The operator ~shutdown~ takes  host and port 
and stops a SecondoServer at the
specified location.
It returns true, if server was stopped successfully.

*/

static ListExpr
shutdownTypeMap( ListExpr args )
{
  NList myargs (args);
  //cout << "ShutdownTM:" <<  myargs.convertToString() << " -> ";
  if (myargs.length() >= 2 )
    {
      if (!myargs.first().isSymbol(CcString::BasicType()))
        {
          return 
            NList::typeError(
                 "First parameter must be the server name");
        } 
      if (!myargs.second().isSymbol(CcInt::BasicType()))
        {
          return 
            NList::typeError(
                  "Second parameter must be the port number");
        }

      NList result = NList(NList(CcBool::BasicType()));
      //cout << result.convertToString() << endl;
      return result.listExpr(); 
    }
  
  return 
    NList::typeError("Expecting at least server name and port");
}

static int
shutdownFun (Word* args, Word& result, 
             int message, Word& local, Supplier s)
{  
  result = qp->ResultStorage(s);
  bool ret_val = true;

  string host = ((CcString*)args[0].addr)->GetValue();
  int port = ((CcInt*)args[1].addr)->GetValue();

  // retrieving pid for SecondoMonitor
  // stored in /tmp/SM_<port>.lck on that host
  string killPid; // store pid of SecondoMonitor
  string lckfile = "/tmp/SM_" + int2Str(port) + ".lck";
  string cat_cmd = "cat " + lckfile;
  bool success = RunCmdSSH(host, cat_cmd, killPid);

  if (!success || killPid.empty())
    {
      cerr << "ERROR: No lock file '" + lckfile + "' found!" << endl;
      ret_val = false; 
    }

  if (ret_val)
    {
      // now killing that process w/ kill -SIGTERM <pid>
      // and removing the lockfile /tmp/SM_<port>.lck
      killPid =  stringutils::replaceAll(killPid,"\n","");
      
      string kill_cmd = "kill -SIGTERM " + killPid + ";";
      string rm_cmd = " /bin/rm " + lckfile + ";";
      
      string cmd = "if [ ! -r "+ lckfile + 
        " ]; then echo 0; else " + 
        kill_cmd + rm_cmd + " echo 1; fi";
      
      string retVal;
      success = RunCmdSSH(host, cmd, retVal);

      if (!success || retVal.empty() || retVal[0] != '1')
        {
          cerr << "ERROR: No lock file '" + lckfile + 
            "' found!" << endl;
          ret_val = false;
        }
      else
        {
          cout << "Stopped SecondoMonitor on "
               << host << ":" << port << endl;
        }
    }
    
  ((CcBool*) (result.addr))->Set(true, ret_val);
  return 0;
}
 
const string shutdownSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "(<text>shutdown (Server:string , Port:int) ->  bool</text--->"
      "<text>shutdown (_, _)</text--->"
      "<text>kills worker on a host given in input </text--->"
      "<text>shutdown(\"localhost\", 1234)</text---> ))";

Operator shutDown (
      "shutdown",
      shutdownSpec,
      shutdownFun,
      Operator::SimpleSelect,
      shutdownTypeMap );

/*

6 Creating the Algebra

*/



class DistributedAlgebra : public Algebra
{
   public:
      DistributedAlgebra() : Algebra()
      {
         AddTypeConstructor( &darrayTC );
         darrayTC.AssociateKind(Kind::ARRAY());
         AddOperator( &makeDarray );
         AddOperator( &getA );
         AddOperator( &putA );
         AddOperator( &sendA );
         AddOperator( &receiveA);
         AddOperator( &receiverelA);
         AddOperator( &sendrelA);
         AddOperator( &shuffle3); 
         shuffle3.SetUsesArgsInTypeMapping();
         AddOperator( &shuffle2); 
         shuffle2.SetUsesArgsInTypeMapping();
         AddOperator( &shuffle1);
         shuffle1.SetUsesArgsInTypeMapping();
         AddOperator( &receiveShuffle);
         AddOperator( &sendShuffle);
         sendShuffle.SetUsesMemory();
         AddOperator( &distributeA);
         distributeA.SetUsesMemory();
         AddOperator( &loopA); loopA.SetUsesArgsInTypeMapping();
         AddOperator( &dloopA); dloopA.SetUsesArgsInTypeMapping();
         AddOperator( &dElementA);
         AddOperator( &dElementA2);
         AddOperator( &drel);
         AddOperator( &dIndex);
         AddOperator( &dtie);
         AddOperator( &dsummarize );
         dsummarize.SetUsesMemory();
         AddOperator( &checkWorkers );
         AddOperator( &startUp );
         AddOperator( &shutDown );
      }
      ~DistributedAlgebra() {}
};



/*

7 Initialization

*/

extern "C"
Algebra*
InitializeDistributedAlgebra(NestedList *nlRef, 
                             QueryProcessor *qpRef)
{
  nl = nlRef;
  qp = qpRef;
  return new DistributedAlgebra();
}


