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

1 Overview

This file declares and implements a classes called ~SystemInfoRel~ and
~InfoTuple~.  It will be used to store tuples in memory. Hence they can be
regarded as a kind of memory or virtual relations which are recognized by the
system catalog. If they are used inside a query, a list representation of them
will be generated and a relation object will be created on the fly using the
~In-Function~ of type constructor ~rel~. But even without the relation algebra
the contents of the relations is available since it will be dumped into files in
CSV format which can be read in by a spreadsheet program. Hence there will be no
compile time or run time dependency to the relational algebra module.


Currently there are two system relations (implemented in SecondoInterface.cpp):

  * SEC\_COMMANDS

  * SEC\_COUNTERS

For example the query

----    query SEC_COMMANDS feed consume
----

will show all queries and their command times.

*/

#ifndef CLASS_SYSINFOREL_H
#define CLASS_SYSINFOREL_H

#include <vector>
#include <iostream>
#include <map>

#include "NList.h"

using namespace std;

/*
2 Class ~InfoTuple~
 
This class defines 2 virtual functions which need to be implemented by
its subclasses. An instance of ~InfoTuple~ represents a tuple and can be
appended to an instance of class ~SystemInfoRel~.

*/


class InfoTuple
{
   
   public:
   const string sep;
   
   virtual NList valueList() const = 0;
   virtual ostream& print(ostream&) const = 0; 
   
   InfoTuple() : sep("|") {}
   virtual ~InfoTuple() {} 
  
};

ostream& operator<<(ostream&, const InfoTuple&); 

/*
3 Class ~SystemInfoRel~
 
Reserved object identifiers for the SECONDO System can be added
in the Constructor of this class. As a convention all system reserverd 
identifiers should be prefixed with "SEC\_".

*/


class SystemInfoRel 
{

  typedef vector<InfoTuple*> InfoTupVec;
  typedef InfoTupVec::const_iterator iterator;

  public:   
  typedef vector< pair<string, string> > RelSchema;
  
  InfoTupVec tuples;
  RelSchema* attrList; 
  const string name;
  const string logFile;
  const string sep;
  const bool isPersistent;
     
  SystemInfoRel( const string& inName, 
                 RelSchema* attrs, 
                 const bool persistent=false ) :
    attrList(attrs), 
    name(inName),
    logFile(name + ".csv"),
    sep("|"),
    isPersistent(persistent) 
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

  NList relSchema() const
  { 
     RelSchema::iterator it = attrList->begin();
     assert(it != attrList->end());
     
     NList types;
     NList pair( NList(it->first), NList(it->second) );
     types.makeHead( pair );
     it++;
      
     while ( it != attrList->end() )
     { 
       pair = NList( NList(it->first), NList(it->second) );
       types.append( pair );
       it++;
     }   
     
     NList relSchema = NList( NList("rel"), NList(NList("tuple"), types) ); 
     //cout << "Schema: " << relSchema << endl;
     return relSchema;
  } 

  NList relValues() const
  {  
    //SHOW(tuples.size())
    iterator it = begin();
    NList values;
    values.makeHead( (*it)->valueList() );
    it++;
      
    while( it != end() )
    {
       values.append( (*it)->valueList() );
       it++;
    } 
    //cout << "Values: " << values << endl;
    return values;
  }
  
  const string& getName() const { return name; }   
  
  void append(InfoTuple* t, bool dump) 
  {
    if (dump)
      cmsg.file(logFile) << *t << endl; 
    tuples.push_back(t); 
  }
 
  iterator begin() const { return tuples.begin(); } 
  iterator end()   const { return tuples.end(); } 
 
};


class SystemTables {

   public:
   typedef map<string, const SystemInfoRel*> Str2TableMap;
   typedef Str2TableMap::const_iterator iterator;
   
   
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
       cout << "Creating SystemTable instance!" << endl;
       instance = new SystemTables;
       return *instance;
     }  
   }
  
  void insert(SystemInfoRel* rel) 
  {
     const string& name = rel->getName();
     assert( tables.find(name) == tables.end() );

     // register new system table
     string type="virtual";
     if (rel->isPersistent)
       type="persistent";
     cout << "  registering " << type << " system table " << name << endl;
     tables[name] = /*(const SystemInfoRel*)*/ rel;
  }
   
  const SystemInfoRel* getInfoRel(const string& name) const
  {
     iterator it = tables.find(name);
     if ( it == tables.end() ) {
       //cout << "table " << name << " not found!" << endl;
       return 0;
     }  
     else
       return it->second;
  } 
 
  iterator begin() const { return tables.begin(); }
  iterator end()   const { return tables.end(); }
   
  private:
  
  Str2TableMap tables;
  static SystemTables* instance;
  
  SystemTables() {}

}; 

#endif
