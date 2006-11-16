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



September 2006, M. Spiekermann. Class implementations moved from "SecondoInterface.cpp"
into this file.


An overview about System tables is given in the file "SystemInfoRel.cpp".

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
   CmdTimesRel(const string& name) : SystemInfoRel(name) 
   {} 

   virtual ~CmdTimesRel() {}
   
   virtual void initSchema()
   { 
     addAttribute("CmdNr", "int");
     addAttribute("CmdStr", "text");
     addAttribute("ElapsedTime", "real");
     addAttribute("CpuTime", "real");
     addAttribute("CommitTime", "real");
     addAttribute("queryReal", "real");
     addAttribute("queryCPU", "real");
     addAttribute("outObjReal", "real");
     addAttribute("copyReal", "real");
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
   CmdCtrRel(const string& name) : SystemInfoRel(name) 
   {}
   virtual ~CmdCtrRel() {}
   
   virtual void initSchema()
   { 
     addAttribute("CtrNr", "int");
     addAttribute("CtrStr", "string");
     addAttribute("Value", "int");
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
   DerivedObjRel(const string& name) : SystemInfoRel(name, true) 
   {}
   virtual ~DerivedObjRel() {}
   
   virtual void initSchema()
   { 
     addAttribute("name", "string");
     addAttribute("value", "text");
     addAttribute("usedObjs", "text");
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
   CacheInfoRel(const string& name) : SystemInfoRel(name) 
   {}
   virtual ~CacheInfoRel() {}
   
   virtual void initSchema()
   { 
     addAttribute("CStatNr", "int");
     addAttribute("Bytes", "int");
     addAttribute("RegSize", "int");
     addAttribute("Hits", "int");
     addAttribute("Misses", "int");
     addAttribute("Pages_New", "int");
     addAttribute("Pages_In", "int");
     addAttribute("Pages_Out", "int");
     addAttribute("Pages_All", "int");
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
   FileInfoRel(const string& name) : SystemInfoRel(name) 
   {}
   virtual ~FileInfoRel() {}
   
   virtual void initSchema()
   { 
     addAttribute("FStatNr", "int");
     addAttribute("File", "text");
     addAttribute("PageSize", "int");
     addAttribute("Hits", "int");
     addAttribute("Misses", "int");
     addAttribute("Pages_New", "int");
     addAttribute("Pages_In", "int");
     addAttribute("Pages_Out", "int");
   } 
}; 

class TypeInfoTuple : public InfoTuple
{
   public:
   string type;
   int size;  
   string algebra;
   string signature;
   string typeListExample;
   string listRep;
   string valueListExample;
   string remark;
   
   TypeInfoTuple() {}
   virtual ~TypeInfoTuple() {

     type = "";
     size = 0;  
     signature = "";
     algebra = "";
     typeListExample = "";
     listRep = "";
     valueListExample = "";
     remark = "";
   } 

   virtual NList valueList() const
   {
     NList list;
     list.makeHead( NList().stringAtom(type) );
     list.append( NList().stringAtom(algebra) );
     list.append( NList().textAtom(signature) );
     list.append( NList().textAtom(typeListExample) );
     list.append( NList().textAtom(listRep) );
     list.append( NList().textAtom(valueListExample) );
     list.append( NList().textAtom(remark) );
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


class TypeInfoRel : public SystemInfoRel
{
   public:
   TypeInfoRel(const string& name) : SystemInfoRel(name) 
   {}
   virtual ~TypeInfoRel() {}
   
   virtual void initSchema()
   { 
     addAttribute("Type", "string");
     addAttribute("Algebra", "string");
     addAttribute("Signature", "text");
     addAttribute("TypeListExample", "text");
     addAttribute("ListRep", "text");
     addAttribute("ValueListExample", "text");
     addAttribute("Remark", "text");
     addAttribute("Size", "int");
   } 
}; 

class OperatorInfoTuple : public InfoTuple
{
   public:
   string name;
   string algebra;
   string signature;
   string syntax;
   string meaning;
   string example;
   string remark;
   
   OperatorInfoTuple() {}
   virtual ~OperatorInfoTuple() {

     name = "";
     algebra = "";
     signature = "";
     syntax = "";
     meaning = "";
     example = "";
     remark = "";
   } 

   virtual NList valueList() const
   {
     NList list;
     list.makeHead( NList().stringAtom(name) );
     list.append( NList().stringAtom(algebra) );
     list.append( NList().textAtom(signature) );
     list.append( NList().textAtom(syntax) );
     list.append( NList().textAtom(meaning) );
     list.append( NList().textAtom(example) );
     list.append( NList().textAtom(remark) );
     return list;
   } 
   
   virtual ostream& print(ostream& os) const
   {
      os << name << sep
         << algebra << endl; 
      return os;
   } 
};


class OperatorInfoRel : public SystemInfoRel
{
   public:
   OperatorInfoRel(const string& name) : SystemInfoRel(name) 
   {}
   virtual ~OperatorInfoRel() {}
   
   virtual void initSchema()
   { 
     addAttribute("Name", "string");
     addAttribute("Algebra", "string");
     addAttribute("Signature", "text");
     addAttribute("Syntax", "text");
     addAttribute("Meaning", "text");
     addAttribute("Example", "text");
     addAttribute("Remark", "text");
   } 
}; 


class OperatorUsageTuple : public InfoTuple
{
   public:
   string name;
   string algebra;
   int    calls;
   
   OperatorUsageTuple() {}
   virtual ~OperatorUsageTuple() {

     name = "";
     algebra = "";
     calls = 0;
   } 

   virtual NList valueList() const
   {
     NList list;
     list.makeHead( NList().stringAtom(name) );
     list.append( NList().stringAtom(algebra) );
     list.append( NList((int)calls) );
     return list;
   } 
   
   virtual ostream& print(ostream& os) const
   {
      os << name << sep
         << algebra << sep
         << calls << endl; 
      return os;
   } 
};


class OperatorUsageRel : public SystemInfoRel
{
   public:
   OperatorUsageRel(const string& name) : SystemInfoRel(name) 
   {}
   virtual ~OperatorUsageRel() {}
   
   virtual void initSchema()
   { 
     addAttribute("Algebra", "string");
     addAttribute("Operator", "string");
     addAttribute("Calls", "int");
   } 
}; 



#endif
