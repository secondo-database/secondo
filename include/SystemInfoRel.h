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

March 2006, M. Spiekermann

This file declares and implements a classes called ~SystemInfoRel~ and
~InfoTuple~.  It will be used to store tuples in memory. Hence they can be

regarded as a kind of memory relations which are not maintained by the system
catalog. A special operator in the relational algebra called ~feedinfo~
will create a stream of tuples, hence their data may be used by the relational
algebra operations. Currently, there are two info relations:

  * CmdTimes

  * CmdCounters

For example the query

----    query feedinfo["CmdTimes"] consume
----

will show all queries and their command times. But even without this operator
and the relation algebra the contents of the relations is available since it
will be dumped into files in CSV format which can be read in by a spreadsheet
program. 
   
*/

#ifndef CLASS_SYSINFOREL_H
#define CLASS_SYSINFOREL_H

#include <vector>
#include <iostream>
#include <map>

#include "NList.h"

// forward declaration
class Tuple;

using namespace std;

class InfoTuple
{
   
   public:
   const string sep;
   
   virtual Tuple* mkTuple() { return 0; }
   virtual ostream& print(ostream&) const = 0; 
   
   InfoTuple() : sep("|") {}
   virtual ~InfoTuple() {} 
  
};

ostream& operator<<(ostream&, const InfoTuple&); 

/*
Class ~SystemInfoRel~
   
*/

class SystemInfoRel 
{
  typedef vector<InfoTuple*> InfoTupVec;
  typedef InfoTupVec::iterator iterator;
 
  public:   
    typedef vector< pair<string, string> > RelSchema;
  
     InfoTupVec tuples;
     RelSchema* attrList; 
     const string name;
     const string logFile;
     const string sep;

  SystemInfoRel(const string& inName, RelSchema* attrs) :
    attrList(attrs), 
    name(inName),
    logFile(name + ".csv"),
    sep("|") 
  {
      // Write Headline
      ostream& clog = cmsg.file(logFile);
      RelSchema::iterator it = attrList->begin();
      while ( it != attrList->end() )
      { 
        clog << it->first;
        if (it+1 != attrList->end())
           clog << sep;
        it++;
      } 
      clog << endl;  
      
      // write configuration file
      ostream& cfg = cmsg.file(name+".cfg");  
      
      cfg << "# Generated file: Can be used together with "
          << "commands.csv by CVS2Secondo!" << endl
          << "Separator  |" << endl
          << "Object " << name << endl
          << "Scheme ";
     
      it = attrList->begin();
      while ( it != attrList->end() )
      { 
        cfg << it->first << " " << it->second;
        if (it+1 != attrList->end())
          cfg << " \\t ";
        it++;
      }   
      cfg << endl;

  }
  ~SystemInfoRel() 
  {
    iterator it = tuples.begin();
    while( it != tuples.end() )
    {
      delete *it;
      it++;
    } 
    if ( attrList)
      delete attrList;
    attrList = 0;
  }  

  NList resultType() 
  { 
     RelSchema::iterator it = attrList->begin();
     assert(it != attrList->end());
     
     NList type;
     NList pair( NList(it->first), NList(it->second) );
     type.makeHead( pair );
     it++;
      
     while ( it != attrList->end() )
     { 
       pair = NList( NList(it->first), NList(it->second) );
       type.append( pair );
       it++;
     }   
     
     type = NList( NList("tuple"), type); 
     //cout << type << endl;
     return type;
  } 
  
  const string& getName() { return name; }   
  
  void append(InfoTuple* t, bool dump) 
  {
    if (dump)
      cmsg.file(logFile) << *t << endl; 
    tuples.push_back(t); 
  }
 
  iterator begin() { return tuples.begin(); } 
  iterator end() { return tuples.end(); } 
  
  
};


class SystemTables {

   typedef map<string, SystemInfoRel*> Str2TableMap;
   typedef Str2TableMap::iterator iterator;
   
   private:
   
   Str2TableMap tables;
   static SystemTables* instance;
   
   SystemTables() {}
   
   public:
   ~SystemTables() 
   { 
     iterator it = tables.begin();
     while (it != tables.end())
     {
       delete it->second;
       it++;
     } 
   } 
   
   static SystemTables& getInstance() 
   { 
     if (instance) {
       return *instance;
     } else {
       instance = new SystemTables;
       return *instance;
     }  
   }
  
  void insert(SystemInfoRel* rel) 
  {
     const string& name = rel->getName(); 
     iterator it = tables.find(name);
     assert( it == tables.end() );
     tables[name] = rel;
  }
   
  SystemInfoRel* getInfoRel(const string& name)
  {
     iterator it = tables.find(name);
     if ( it == tables.end() )
       return 0;
     else
       return it->second;
  } 
  
}; 

#endif
