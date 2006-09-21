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

September 2006 Markus Spiekermann

*/

#ifndef SECONDO_SYSTABLES_H
#define SECONDO_SYSTABLES_H

#include "NList.h"
#include "SystemInfoRel.h"

class CmdTimes : public InfoTuple 
{
   int nr;
   string cmdStr;
   double elapsedTime;
   double cpuTime;
   double commitTime;
   double queryReal;
   double queryCPU;
   double outObjReal;
   double copyReal;

   public: 
   CmdTimes( int num, 
             const string& cmd, 
             double realT, 
             double cpuT, 
             double commitT, 
             double qRT,
             double qCT,
             double outRT,
             double cpRT ) :
     nr(num),
     cmdStr(cmd),
     elapsedTime(realT),
     cpuTime(cpuT),
     commitTime(commitT),
     queryReal(qRT),
     queryCPU(qCT),
     outObjReal(outRT),
     copyReal(cpRT)
   {
   }
   virtual ~CmdTimes() {}
  
   virtual NList valueList() const
   {
     NList value;
     value.makeHead( NList(nr) );
     value.append( NList().textAtom(cmdStr) );
     value.append( NList(elapsedTime) );
     value.append( NList(cpuTime) );
     value.append( NList(commitTime) );
     value.append( NList(queryReal) );
     value.append( NList(queryCPU) );
     value.append( NList(outObjReal) );
     value.append( NList(copyReal) );
     return value;
   } 
   
   virtual ostream& print(ostream& os) const
   {
      os << nr << sep << cmdStr << sep << elapsedTime << sep << cpuTime;
      return os;
   } 

}; 


class CmdTimesRel : public SystemInfoRel 
{
   public:
   CmdTimesRel(const string& name) : SystemInfoRel(name, initSchema()) 
   {} 

   ~CmdTimesRel() {}
   
   private:
   RelSchema* initSchema()
   { 
     RelSchema*  attrList = new RelSchema();
     attrList->push_back( make_pair("CmdNr", "int") );
     attrList->push_back( make_pair("CmdStr", "text") );
     attrList->push_back( make_pair("ElapsedTime", "real") );
     attrList->push_back( make_pair("CpuTime", "real") );
     attrList->push_back( make_pair("CommitTime", "real") );
     attrList->push_back( make_pair("queryReal", "real") );
     attrList->push_back( make_pair("queryCPU", "real") );
     attrList->push_back( make_pair("outObjReal", "real") );
     attrList->push_back( make_pair("copyReal", "real") );
     return attrList;  
   } 

}; 

class CmdCtr : public InfoTuple 
{
   int nr;
   string ctrStr;
   long value;

   public: 
   CmdCtr(int num, const string& cmd, long ctrVal) :
     nr(num),
     ctrStr(cmd),
     value(ctrVal)
   {}
   virtual ~CmdCtr() {}
   
   virtual NList valueList() const
   {
     NList list;
     list.makeHead( NList(nr) );
     list.append( NList().stringAtom(ctrStr) );
     list.append( NList((int) value) );
     return list;
   } 

   
   virtual ostream& print(ostream& os) const
   {
      os << nr << sep << ctrStr << sep << value;
      return os;
   } 
}; 


class CmdCtrRel : public SystemInfoRel 
{
   public:
   CmdCtrRel(const string& name) : SystemInfoRel(name, initSchema()) 
   {}
   virtual ~CmdCtrRel() {}
   
   private:
   RelSchema* initSchema()
   { 
     RelSchema* attrList = new RelSchema();
     attrList->push_back( make_pair("CtrNr", "int") );
     attrList->push_back( make_pair("CtrStr", "string") );
     attrList->push_back( make_pair("Value", "int") );
     return attrList;
   } 
}; 

class DerivedObjInfo : public InfoTuple 
{
   string name;
   string value;
   string usedObjs;

   public: 
   DerivedObjInfo(const string& n, const string& v, const string&u) :
     name(n),
     value(v),
     usedObjs(u)
   {}
   virtual ~DerivedObjInfo() {}
   
   virtual NList valueList() const
   {
     NList list;
     list.makeHead( NList().stringAtom(name) );
     list.append( NList().textAtom(value) );
     list.append( NList().textAtom(usedObjs) );
     return list;
   } 

   
   virtual ostream& print(ostream& os) const
   {
      os << name << sep << value << sep << usedObjs;
      return os;
   } 
}; 



class DerivedObjRel : public SystemInfoRel 
{
   public:
   DerivedObjRel(const string& name) : SystemInfoRel(name, initSchema(), true) 
   {}
   virtual ~DerivedObjRel() {}
   
   private:
   RelSchema* initSchema()
   { 
     RelSchema* attrList = new RelSchema();
     attrList->push_back( make_pair("name", "string") );
     attrList->push_back( make_pair("value", "text") );
     attrList->push_back( make_pair("usedObjs", "text") );
     return attrList;
   } 
}; 


class CacheInfoTuple : public InfoTuple, public CacheInfo
{
   public:
   CacheInfoTuple() {}
   virtual ~CacheInfoTuple() {} 

   virtual NList valueList() const
   {
     NList value;
     value.makeHead( NList(cstatNr) );
     value.append( NList((int)bytes) );
     value.append( NList((int)regsize) );
     value.append( NList((int)cache_hit) );
     value.append( NList((int)cache_miss) );
     value.append( NList((int)page_create) );
     value.append( NList((int)page_in) );
     value.append( NList((int)page_out) );
     value.append( NList((int)pages) );
     return value;
   } 
   
   virtual ostream& print(ostream& os) const
   {
      os << cstatNr << sep
         << bytes << sep 
         << regsize << sep 
         << cache_hit << sep 
         << cache_miss << sep
         << page_create << sep
         << page_in << sep
         << page_out << sep
         << pages << endl; 
      return os;
   } 
};

class CacheInfoRel : public SystemInfoRel 
{
   public:
   CacheInfoRel(const string& name) : SystemInfoRel(name, initSchema()) 
   {}
   virtual ~CacheInfoRel() {}
   
   private:
   RelSchema* initSchema()
   { 
     RelSchema* attrList = new RelSchema();
     attrList->push_back( make_pair("CStatNr", "int") );
     attrList->push_back( make_pair("Bytes", "int") );
     attrList->push_back( make_pair("RegSize", "int") );
     attrList->push_back( make_pair("Hits", "int") );
     attrList->push_back( make_pair("Misses", "int") );
     attrList->push_back( make_pair("Pages_New", "int") );
     attrList->push_back( make_pair("Pages_In", "int") );
     attrList->push_back( make_pair("Pages_Out", "int") );
     attrList->push_back( make_pair("Pages_All", "int") );
     return attrList;
   } 
}; 

class FileInfoTuple : public InfoTuple, public FileInfo
{
   public:
   FileInfoTuple(FileInfo* fstat) 
   {
     fstatNr = fstat->fstatNr;
     file_name = fstat->file_name;
     pagesize = fstat->pagesize;
     cache_hit = fstat->cache_hit;
     cache_miss = fstat->cache_miss;
     page_create = fstat->page_create;
     page_in = fstat->page_in;
     page_out = fstat->page_out;
   }
   virtual ~FileInfoTuple() {} 

   virtual NList valueList() const
   {
     NList value;
     value.makeHead( NList(fstatNr) );
     value.append( NList().textAtom(file_name) );
     value.append( NList((int)pagesize) );
     value.append( NList((int)cache_hit) );
     value.append( NList((int)cache_miss) );
     value.append( NList((int)page_create) );
     value.append( NList((int)page_in) );
     value.append( NList((int)page_out) );
     return value;
   } 
   
   virtual ostream& print(ostream& os) const
   {
      os << fstatNr << sep
         << file_name << sep 
         << pagesize << sep 
         << cache_hit << sep 
         << cache_miss << sep
         << page_create << sep
         << page_in << sep
         << page_out << endl; 
      return os;
   } 
};


class FileInfoRel : public SystemInfoRel
{
   public:
   FileInfoRel(const string& name) : SystemInfoRel(name, initSchema()) 
   {}
   virtual ~FileInfoRel() {}
   
   private:
   RelSchema* initSchema()
   { 
     RelSchema* attrList = new RelSchema();
     attrList->push_back( make_pair("FStatNr", "int") );
     attrList->push_back( make_pair("File", "text") );
     attrList->push_back( make_pair("PageSize", "int") );
     attrList->push_back( make_pair("Hits", "int") );
     attrList->push_back( make_pair("Misses", "int") );
     attrList->push_back( make_pair("Pages_New", "int") );
     attrList->push_back( make_pair("Pages_In", "int") );
     attrList->push_back( make_pair("Pages_Out", "int") );
     return attrList;
   } 
}; 

class SizeInfoTuple : public InfoTuple
{
   string type;
   int size;
   
   public:
   SizeInfoTuple(const string& Type, int Size) 
   {
     type = Type;
     size = Size;
   }
   virtual ~SizeInfoTuple() {} 

   virtual NList valueList() const
   {
     NList list;
     list.makeHead( NList().stringAtom(type) );
     list.append( NList((int) size) );
     return list;
   } 
   
   virtual ostream& print(ostream& os) const
   {
      os << type << sep
         << size << endl; 
      return os;
   } 
};


class SizeInfoRel : public SystemInfoRel
{
   public:
   SizeInfoRel(const string& name) : SystemInfoRel(name, initSchema()) 
   {}
   virtual ~SizeInfoRel() {}
   
   private:
   RelSchema* initSchema()
   { 
     RelSchema* attrList = new RelSchema();
     attrList->push_back( make_pair("Type", "string") );
     attrList->push_back( make_pair("Size", "int") );
     return attrList;
   } 
}; 

#endif
