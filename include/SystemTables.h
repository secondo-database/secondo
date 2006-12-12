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
     addAttribute("CmdNr",       sym.INT()  );
     addAttribute("CmdStr",      sym.TEXT() );
     addAttribute("ElapsedTime", sym.REAL() );
     addAttribute("CpuTime",     sym.REAL() );
     addAttribute("CommitTime",  sym.REAL() );
     addAttribute("queryReal",   sym.REAL() );
     addAttribute("queryCPU",    sym.REAL() );
     addAttribute("outObjReal",  sym.REAL() );
     addAttribute("copyReal",    sym.REAL() );
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
     list.append( NList().intAtom(value) );
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
     addAttribute("CtrNr",  sym.INT()    );
     addAttribute("CtrStr", sym.STRING() );
     addAttribute("Value",  sym.INT()    );
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
     addAttribute("name",     sym.STRING() );
     addAttribute("value",    sym.TEXT()   );
     addAttribute("usedObjs", sym.TEXT()   );
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
     value.append( NList().intAtom(bytes) );
     value.append( NList().intAtom(regsize) );
     value.append( NList().intAtom(cache_hit) );
     value.append( NList().intAtom(cache_miss) );
     value.append( NList().intAtom(page_create) );
     value.append( NList().intAtom(page_in) );
     value.append( NList().intAtom(page_out) );
     value.append( NList().intAtom(pages) );
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
     addAttribute("CStatNr",   sym.INT() );
     addAttribute("Bytes",     sym.INT() );
     addAttribute("RegSize",   sym.INT() );
     addAttribute("Hits",      sym.INT() );
     addAttribute("Misses",    sym.INT() );
     addAttribute("Pages_New", sym.INT() );
     addAttribute("Pages_In",  sym.INT() );
     addAttribute("Pages_Out", sym.INT() );
     addAttribute("Pages_All", sym.INT() );
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
     value.append( NList().intAtom(pagesize) );
     value.append( NList().intAtom(cache_hit) );
     value.append( NList().intAtom(cache_miss) );
     value.append( NList().intAtom(page_create) );
     value.append( NList().intAtom(page_in) );
     value.append( NList().intAtom(page_out) );
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
     addAttribute("FStatNr",   sym.INT()  );
     addAttribute("File",      sym.TEXT() );
     addAttribute("PageSize",  sym.INT()  );
     addAttribute("Hits",      sym.INT()  );
     addAttribute("Misses",    sym.INT()  );
     addAttribute("Pages_New", sym.INT()  );
     addAttribute("Pages_In",  sym.INT()  );
     addAttribute("Pages_Out", sym.INT()  );
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
   
   TypeInfoTuple() {

     type = "";
     size = 0;  
     signature = "";
     algebra = "";
     typeListExample = "";
     listRep = "";
     valueListExample = "";
     remark = "";
   } 
   virtual ~TypeInfoTuple() {}

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
     list.append( NList().intAtom(size) );
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
     addAttribute("Type",             sym.STRING() );
     addAttribute("Algebra",          sym.STRING() );
     addAttribute("Signature",        sym.TEXT()   );
     addAttribute("TypeListExample",  sym.TEXT()   );
     addAttribute("ListRep",          sym.TEXT()   );
     addAttribute("ValueListExample", sym.TEXT()   );
     addAttribute("Remark",           sym.TEXT()   );
     addAttribute("Size",             sym.INT()    );
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
   string result;
   
   OperatorInfoTuple() {

     name = "";
     algebra = "";
     signature = "";
     syntax = "";
     meaning = "";
     example = "";
     result = "";
     remark = "";
   } 
   virtual ~OperatorInfoTuple() {}

   virtual NList valueList() const
   {
     NList list;
     list.makeHead( NList().stringAtom(name) );
     list.append( NList().stringAtom(algebra) );
     list.append( NList().textAtom(signature) );
     list.append( NList().textAtom(syntax) );
     list.append( NList().textAtom(meaning) );
     list.append( NList().textAtom(example) );
     list.append( NList().textAtom(result) );
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
     addAttribute("Name",      sym.STRING() );
     addAttribute("Algebra",   sym.STRING() );
     addAttribute("Signature", sym.TEXT()   );
     addAttribute("Syntax",    sym.TEXT()   );
     addAttribute("Meaning",   sym.TEXT()   );
     addAttribute("Example",   sym.TEXT()   );
     addAttribute("Result",    sym.TEXT()   );
     addAttribute("Remark",    sym.TEXT()   );
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
     list.append( NList().intAtom(calls) );
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
     addAttribute("Algebra",  sym.STRING() );
     addAttribute("Operator", sym.STRING() );
     addAttribute("Calls",    sym.INT()    );
   } 
}; 



#endif
