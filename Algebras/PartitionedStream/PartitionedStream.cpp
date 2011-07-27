/*
----
This file is part of SECONDO.

Copyright (C) 2004-2008, University in Hagen, Faculty Mathematics and Computer
Science, Database Systems for New Applications.

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

//paragraph [1] title: [{\Large \bf ]   [}]



[1] Partitioned Stream Algebra

January 2006 M. Spiekermann. Framework of the algebra

May 2006 M. Spiekermann. Implementation of ~pjoin2~ and cost functions

June 2006 M. Spiekermann. Corrections for bugs in the cardinality estimation. Implementation
of operator ~pjoin1~.

This algebra implements a type constructor ~ptuple~ which represents ~normal~ tuples
of the relational algebra or ~marker~ tuples which contain information about a bunch
of tuples. Since tuples are normally processed in a stream the markers defines partitions
of the stream.


Operations like ~puse~ and ~pjoin~ are implemented to support adaptive techniques
for query processing.


1 Preliminaries

1.1 Includes and global declarations

*/

#include <assert.h>
#include <iostream>
#include <sstream>
#include <queue>
#include <algorithm>

#undef TRACE_ON
//#define TRACE_ON 1
#include "LogMsg.h"

#include "CharTransform.h"
#include "StopWatch.h"
#include "Algebra.h"
#include "NList.h"
#include "QueryProcessor.h"
#include "SecondoSystem.h"
#include "Attribute.h"
#include "FunVector.h"
#include "CostFunction.h"

#include "SystemInfoRel.h"
#include "Environment.h"
#include "Symbols.h"

/*
Dependencies with other algebras: RelationAlgebra, StandardAlgebra

*/
#include "RelationAlgebra.h"
#include "StandardTypes.h"


extern NestedList* nl;
extern QueryProcessor *qp;

inline int
nextInt(const double d)
{
  return static_cast<int>( ceil(d) );
}

inline int
previousInt(const double d)
{
  return static_cast<int>( floor(d) );
}


// a system table which stores some internals about
// the adaptive processing

class PJoinTuple : public InfoTuple
{
   public:
   int id;
   string join;
   int arg1_est;
   int arg1_real;
   int arg1_err;
   int arg2_est;
   int arg2_real;
   int arg2_err;
   int result_est;
   int result_real;
   int result_err;
   float sel_est;
   float sel_real;
   int sel_err;
   int probe_result;
   float probe_seconds;
   int probe_arg1;
   int probe_arg2;
   int probe_cpuOps;
   string usedFunction;


   PJoinTuple() : join(""), usedFunction("") {
     id=0;
     arg1_est=0;
     arg1_real=0;
     arg1_err=0;
     arg2_est=0;
     arg2_real=0;
     arg2_err=0;
     result_est=0;
     result_real=0;
     result_err=0;
     sel_est=0;
     sel_real=0;
     sel_err=0;
     probe_result=0;
     probe_arg1=0;
     probe_arg2=0;
     probe_seconds=0.0;
     probe_cpuOps=0;
   }

   virtual ~PJoinTuple() {}

   virtual NList valueList() const
   {
     NList list;
     list.makeHead( NList().intAtom(id) );
     list.append( NList().stringAtom(join) );
     list.append( NList().intAtom(arg1_est) );
     list.append( NList().intAtom(arg1_real) );
     list.append( NList().intAtom(arg1_err) );
     list.append( NList().intAtom(arg2_est) );
     list.append( NList().intAtom(arg2_real) );
     list.append( NList().intAtom(arg2_err) );
     list.append( NList().intAtom(result_est) );
     list.append( NList().intAtom(result_real) );
     list.append( NList().intAtom(result_err) );
     list.append( NList().realAtom(sel_est) );
     list.append( NList().realAtom(sel_real) );
     list.append( NList().intAtom(sel_err) );
     list.append( NList().intAtom(probe_result) );
     list.append( NList().realAtom(probe_seconds) );
     list.append( NList().intAtom(probe_cpuOps) );
     list.append( NList().intAtom(probe_arg1) );
     list.append( NList().intAtom(probe_arg2) );
     list.append( NList().stringAtom(usedFunction) );
     return list;
   }

   virtual ostream& print(ostream& os) const
   {
      os << id << sep
         << join << sep
         << result_est << endl;
      return os;
   }
};



class CostTuple : public InfoTuple
{
   public:
   int id;
   int param_arg1_card;
   int param_arg2_card;
   int param_res_card;
   int param_arg1_pages;
   int param_arg2_pages;
   SEC_STD_REAL param_join_sel;

   string cost_name;
   int cost_write;
   int cost_read;
   int cost_cpu;
   SEC_STD_REAL cost_value;
   int real_write;
   int real_read;
   int real_cpu;
   SEC_STD_REAL real_runtime;

   CostTuple() {
   id=0;
   param_arg1_card = 0;
   param_arg2_card = 0;
   param_res_card = 0;
   param_arg1_pages = 0;
   param_arg2_pages = 0;
   param_join_sel = 0.0;
   cost_name = "";
   cost_write = 0;
   cost_read = 0;
   cost_cpu = 0;
   cost_value = 0;
   real_write = 0;
   real_read = 0;
   real_cpu = 0;
   real_runtime = 0;
   }

   CostTuple(const CostParams& cp, const CostResult& cr) {
   id=0;
   param_arg1_card = cp.cardA;
   param_arg2_card = cp.cardB;
   param_res_card = 0;
   param_arg1_pages = cp.pagesA;
   param_arg2_pages = cp.pagesB;
   param_join_sel = cp.sel;
   cost_name = "";
   cost_write = cr.write;
   cost_read = cr.read;
   cost_cpu = cr.cpu;
   cost_value = cr.value;
   real_write = 0;
   real_read = 0;
   real_cpu = 0;
   real_runtime = 0;
   }
   virtual ~CostTuple() {}

   virtual NList valueList() const
   {
     NList list;
     list.makeHead( NList().intAtom(id) );
     list.append( NList().intAtom(param_arg1_card) );
     list.append( NList().intAtom(param_arg2_card) );
     list.append( NList().intAtom(param_res_card) );
     list.append( NList().intAtom(param_arg1_pages) );
     list.append( NList().intAtom(param_arg2_pages) );
     list.append( NList().realAtom(param_join_sel) );
     list.append( NList().stringAtom(cost_name) );
     list.append( NList().intAtom(cost_write) );
     list.append( NList().intAtom(cost_read) );
     list.append( NList().intAtom(cost_cpu) );
     list.append( NList().realAtom(cost_value) );
     list.append( NList().intAtom(real_write) );
     list.append( NList().intAtom(real_read) );
     list.append( NList().intAtom(real_cpu) );
     list.append( NList().realAtom(real_runtime) );
     return list;
   }

   virtual ostream& print(ostream& os) const
   {
      os << id << sep
         << cost_name << sep
         << cost_value << endl;
      return os;
   }
};

class PJoinRel : public SystemInfoRel
{
   public:
   PJoinRel(const string& name) : SystemInfoRel(name)
   {}
   virtual ~PJoinRel() {}

   virtual void initSchema()
   {
     addAttribute("Id",    CcInt::BasicType()    );
     addAttribute("Join",  CcString::BasicType() );
     addAttribute("Arg1_guess", CcInt::BasicType() );
     addAttribute("Arg1_value", CcInt::BasicType() );
     addAttribute("Arg1_error", CcInt::BasicType() );
     addAttribute("Arg2_guess", CcInt::BasicType() );
     addAttribute("Arg2_value", CcInt::BasicType() );
     addAttribute("Arg2_error", CcInt::BasicType() );
     addAttribute("Result_guess", CcInt::BasicType() );
     addAttribute("Result_value", CcInt::BasicType() );
     addAttribute("Result_error", CcInt::BasicType() );
     addAttribute("Sel_guess", CcReal::BasicType() );
     addAttribute("Sel_value", CcReal::BasicType() );
     addAttribute("Sel_error", CcInt::BasicType() );
     addAttribute("Probe_Result", CcInt::BasicType() );
     addAttribute("Probe_Seconds", CcReal::BasicType() );
     addAttribute("Probe_CPU_Ops", CcInt::BasicType() );
     addAttribute("Probe_Arg1", CcInt::BasicType() );
     addAttribute("Probe_Arg2", CcInt::BasicType() );
     addAttribute("Used_Function", CcString::BasicType() );
   }
};


class CostRel : public SystemInfoRel
{
   public:
   CostRel(const string& name) : SystemInfoRel(name)
   {}
   virtual ~CostRel() {}

   virtual void initSchema()
   {
     addAttribute("Id",    CcInt::BasicType()    );
     addAttribute("Param_arg1_card",  CcInt::BasicType() );
     addAttribute("Param_arg2_card", CcInt::BasicType() );
     addAttribute("Param_res_card", CcInt::BasicType() );
     addAttribute("Param_arg1_pages", CcInt::BasicType() );
     addAttribute("Param_arg2_pages", CcInt::BasicType() );
     addAttribute("Param_join_sel", CcReal::BasicType() );
     addAttribute("Cost_name", CcString::BasicType() );
     addAttribute("Cost_write", CcInt::BasicType() );
     addAttribute("Cost_read", CcInt::BasicType() );
     addAttribute("Cost_cpu", CcInt::BasicType() );
     addAttribute("Cost_value", CcReal::BasicType() );
     addAttribute("Real_write", CcInt::BasicType() );
     addAttribute("Real_read",  CcInt::BasicType() );
     addAttribute("Real_cpu", CcInt::BasicType() );
     addAttribute("Real_runtime", CcReal::BasicType() );
   }
};

// the system relation instance pointer
PJoinRel* pjoinRel = 0;
CostRel* costRel = 0;


// extern InObject InRel;

ostream& operator<<(ostream& os, const CostResult& c)
{
  return c.print(os);
}

ostream& operator<<(ostream& os, const CostInfo& c)
{
  return c.print(os);
}

ostream& operator<<(ostream& os, const CostParams& c)
{
  return c.print(os);
}


/*

2 Data structures

*/


struct Marker
{
  int num;    // sequence number
  int parts;  // expected total number of partitions
  int tuples; // (requested) partition size

  Marker() : num(0), parts(0), tuples(0) {}
  Marker(const int n, const int p, const int t) :
   num(n), parts(p), tuples(t)
  {}

  inline void print(ostream& os) const
  {
    os << "(" << num << ", " << parts << ", [" << tuples << "])";
  }
};

ostream& operator<<(ostream& os, const Marker& m)
{
  m.print(os);
  return os;
}

struct PTuple
{
  const Marker* marker;
  Tuple* tuple;

  PTuple() : marker(0), tuple(0) {}
  PTuple(const Marker* m) : marker(m), tuple(0) {}
  PTuple(Tuple* t) : marker(0), tuple(t) {}
  ~PTuple()
  {
    if (marker)
      delete marker;
  }

  static const string BasicType() {return "ptuple";}

};


struct PartCtr
{
  int tuples; // number of tuples per partion
  int parts; // number of partitions
  int num;
  mutable int ctr;

  PartCtr() :  tuples(0), parts(0), num(0), ctr(0) {}
  PartCtr(const int pSize, const int card)
  {
    init(pSize, card);
  }

  void init(const int pSize, const int card)
  {
    num = 0;
    ctr = 0;

    adjustPartSize(pSize);
    computeParts(card);
    assert(tuples > 0);
  }

  void adjustPartSize(const int p) {
    tuples = max(abs(p), 2);
  }

  void computeParts(const int card) {
    parts = card /tuples;
  }


/*
After construction we have ctr == 0, hence the first call
will return true. After ~tuples~ calls the counter will be set
to ~tuples~.

*/
  inline bool resetIfNeeded()
  {
    if (ctr != 0) {
     ctr--;
     return false;
    }
    ctr = tuples;
    return true;
  }

  inline Marker* newMarker()
  {
    assert(ctr == tuples);
    num++;
    return new Marker( num-1, parts, tuples );
  }

};



/*
Below we define a class which helps to maintain stream in value
mappings.

*/

template<class T>
struct StreamBase
{
  typedef enum {opened, finished, closed} StreamState;

  private:
  const T& child() const {
    return static_cast<const T&>( *this );
  }

  public:
  mutable StreamState state;


  inline void open() {
    return child().open();
  }

  inline void close() {
    return child().close();
  }

  inline void* getNext() const
  {
    return child().getNext();
  }

  StreamBase() : state(closed) {}
  StreamBase(const StreamBase& rhs) : state(rhs.state) {}

};

struct StreamOpAddr : public StreamBase<StreamOpAddr>
{
  public:

  void* stream;

  StreamOpAddr( ) : StreamBase<StreamOpAddr>() {}

  StreamOpAddr( Supplier s ) : stream(s) {}

  StreamOpAddr( const StreamOpAddr& rhs ) :
    StreamBase<StreamOpAddr>(rhs),
    stream(rhs.stream)
  {}

  inline void open()
  {
    if (state == closed) {
      TRACE("StreamOpAddr()::open");
      qp->Open(stream);
      state = opened;
    }
  }

  inline void close()
  {
    if (state != closed) {
      TRACE("StreamOpAddr()::close");
      qp->Close(stream);
      state = closed;
    }
  }

/*
The next function gets a tuple of the input stream and returns
the tuple or a marker. At the first call a marker will be produced
This ensures that operators stream-downwards will always know the
requested partsize. Afterwards when the amount of ~tuples~ requests are made
a marker will be injected.

*/


  inline void* getNext() const
  {
    static Word element;
    if (state != finished) {
    qp->Request(stream, element);
    if( qp->Received(stream) ) {
      return element.addr;
    }
    else {
      state = finished;
      return 0;
    }
    }
    return 0;
  }

};

/*
The class below can take a relation but can be handled like a stream
of PTuple

*/

struct RelationAddr : public StreamBase<RelationAddr>
{

  private:
   GenericRelation* rel;
   GenericRelationIterator* rit;
   mutable PartCtr parts;

  public:

  RelationAddr( GenericRelation* r, const int tuples = 100 ) : rel(r), rit(0)
  {
    int card = r->GetNoTuples();
    parts.init(tuples, card);
  }

  ~RelationAddr() {}

  inline void open()
  {
    if (state == closed) {
      TRACE("RelationAddr()::open")
      rit = rel->MakeScan();
      state = opened;
    }
  }

  inline void close()
  {
    if (state != closed) {
      TRACE("RelationAddr()::close")
      delete rit;
      rit = 0;
      state = closed;
    }
  }

  inline void* getNext() const
  {
     if ( parts.resetIfNeeded() )
     {
       return new PTuple( parts.newMarker() );
     }
     else
     {
       Tuple* t = rit->GetNextTuple();
       if (t) {
         return new PTuple(t);
       } else {
          state = finished;
         return 0;
       }
     }
    return 0;
  }

};


/*
3 The Type Constructor ~ptuple~

Since type constructor ~ptuple~ will not be used to construct values from a
list representation it should never be called. Moreover, the ~create~, ~open~
~close~, ~clone~, etc. functions are useless since persistence of ~ptuple~
needs not to be supported.

3.1 Creation of the Type Constructor Instance

Many template functions are used for generic implementations. However,
most of them are not needed and should never be called. Defining ~open~
and ~save~ functions is only needed when the datatype should be used as
attribute type in relations. In and out functions are only interesting if
the object needs to be converted to or from a textual representation.
Data types like indexes don't need this.


*/




/*
4.1 Type mapping and value mapping functions of operator ~pfeed~

This operator makes the following type mapping

----
((rel(tuple(y))) int) -> (stream(ptuple(y)))
----

and creates a marker tuple after a number of tuples which is
specified in the second argument has been created.

*/



struct PartStreamMappings {

  static Symbols sym;

  PartStreamMappings() {}

  template<class T>
  inline static T* nextOfStream(const Word& w)
  {
    Word wTuple = SetWord(Address(0));
    qp->Request(w.addr, wTuple);
    if( qp->Received(w.addr) )
      return static_cast<T*>( wTuple.addr );
    else
      return 0;
  }

  template<class T>
  inline static T* nextOfStream2(const Supplier s)
  {
    Word wTuple = SetWord(Address(0));
    qp->Request(s, wTuple);
    if( qp->Received(s) )
      return static_cast<T*>( wTuple.addr );
    else
      return 0;
  }

  inline static Tuple* nextTuple(const Word& w)
  {
    return nextOfStream<Tuple>(w);
  }

  inline static PTuple* nextPTuple(const Word& w)
  {
    return nextOfStream<PTuple>(w);
  }

  inline static Tuple* nextTuple(const Supplier s)
  {
    return nextOfStream2<Tuple>(s);
  }

  inline static PTuple* nextPTuple(const Supplier s)
  {
    return nextOfStream2<PTuple>(s);
  }

  template<class T>
  inline static T* getArg(const Word& w)
  {
    return static_cast<T*>( w.addr );
  }

  inline static string expects( const string& s1,
                                const string& s2,
                                const string attr="..." )
  {
    return "(" + s1 + "(" + s2 + "(" + attr + ")))";
  }

/*
The functiion ~checkMap~ tests if a list represents a map of
~n~ arguments. It stores the ~n~ arguments and the result type
in the given vector reference. This can be used for subsequent
integrity checks for the arguments and result type.

*/
  static bool checkMap(const NList& l, Cardinal n, vector<NList>& sig)
  {
    Cardinal len = l.length();
    if ( len != (n+2) )
      return false;

    if ( !l.first().isSymbol( Symbol::MAP() ) )
      return false;

    sig.resize(n+1);
    for(Cardinal i = 0; i <= n; i++)
    {
      sig[i] = l.elem(2+i);
    }
    return true;
  }

  static bool checkRelTuple(const NList& l, NList& attrs)
  {
    return checkDepth3(l, Relation::BasicType(), Tuple::BasicType(), attrs);
  }

  static bool checkStreamTuple(const NList& l, NList& attrs)
  {
    return checkDepth3(l, Symbol::STREAM(), Tuple::BasicType(), attrs);
  }

  static bool checkStreamPTuple(const NList& l, NList& attrs)
  {
    return checkDepth3(l, Symbol::STREAM(), PTuple::BasicType(), attrs);
  }

  static NList makeStreamTuple(const NList& attrs)
  {
    NList tup( NList(Tuple::BasicType()), attrs );
    return NList( NList(Symbol::STREAM()), tup );
  }

  static NList makeStreamPTuple(const NList& attrs)
  {
    NList tup( NList(PTuple::BasicType()), attrs );
    return NList( NList(Symbol::STREAM()), tup );
  }

  static bool checkLength(const NList& l, const int len, string& err)
  {
    if ( !l.hasLength(len) )
    {
      stringstream s;
      s << "List length unequal " << len <<  ", " << err;
      err = s.str();
      return false;
    }
    return true;
  }

  static string argNotCorrect( const int n)
  {
    stringstream s;
    s <<  "Type of argument no. " << n << " not correct, ";
    return s.str();
  }

  static bool checkDepth3( const NList& l, const string& s1,
                           const string& s2, NList& attrs    )
  {

  if ( !l.hasLength(2) )
    return false;

  if ( !l.first().isSymbol(s1) )
    return false;

  NList s = l.second();
  if ( !s.hasLength(2) )
    return false;

  if ( !s.first().isSymbol(s2) )
    return false;

  if ( !s.second().isList() )
    return false;

  attrs = s.second();
  return true;
  }

static ListExpr pfeed_tm(ListExpr args)
{
  NList l(args);

  string e1 = expects(Relation::BasicType(),Tuple::BasicType());

  static const string err1 = "pfeed expects (" + e1 + " "
                                                + CcInt::BasicType() + ")!";

  if ( !l.hasLength(2) )
    return l.typeError(err1);

  NList attrs;
  if ( !checkRelTuple( l.first(), attrs ) )
    return l.typeError("First list element should be " + e1 + ".");

  if ( !l.second().isSymbol( CcInt::BasicType() ) )
    return l.typeError( "Second list element should be symbol '"
                        + CcInt::BasicType() + "'." );

  return makeStreamPTuple(attrs).listExpr();
}





  struct PartStreamInfo
  {
    RelationAddr relStream;

    PartStreamInfo( GenericRelation* r, const int partSize ) :
      relStream(r, partSize)
    {
      relStream.open();
    }
    ~PartStreamInfo()
    {
      relStream.close();
    }

    inline PTuple* nextPTuple() {
       return static_cast<PTuple*>( relStream.getNext() );
    }

  };


struct BufferedStreamInfo {

 private:
   TupleBuffer* tupBuf;
   PartStreamInfo* pi;

 public:
  BufferedStreamInfo(Word& stream, const int pSize) : pi(0)
  {
     qp->Open(stream.addr);
     tupBuf = new TupleBuffer();
     Tuple *t = nextTuple( stream );
     while (t != 0)
     {
       tupBuf->AppendTuple(t);
       t = nextTuple( stream );
     }
     qp->Close(stream.addr);
     GenericRelation* r = static_cast<GenericRelation*>( tupBuf );
     pi = new PartStreamInfo(r, pSize);
  }

  inline PTuple* nextPTuple() {
     return pi->nextPTuple();
  }

  ~BufferedStreamInfo()
  {
    delete tupBuf;
    delete pi;
  }

};

static int
pfeed_vm(Word* args, Word& result, int message,
         Word& local, Supplier s)
{
  // args[0]: Input Relation
  // args[1]: An integer defining the partition size

  static const string pre = "pfeed: ";
  PartStreamInfo* info = static_cast<PartStreamInfo*>( local.addr );

  switch (message)
  {
    case OPEN :
    {
      GenericRelation* r = getArg<GenericRelation>( args[0] );
      int partSize = StdTypes::GetInt( args[1] );

      //SHOW(partSize)
      //SHOW(parts)

      info = new PartStreamInfo( r, partSize );
      local.addr = info;
      return 0;
    }

    case REQUEST :
    {
      // the nextPTuple function returns a marker or a tuple
      result.addr = info->nextPTuple();
      if ( result.addr )
      {
        return YIELD;
      }
      else
      {
        return CANCEL;
      }
    }

    case CLOSE :
    {
      //TRACE(pre << "CLOSE received!")
      delete info;

      return 0;
    }

    default : { assert(false); }
  }
  return 0;
}


/*
4.2 Operator ~pdelete~

This operator does the following type mapping:

----
(stream(ptuple(y))) -> (stream(tuple(y)))
----

the value mapping removes all marker tuples fomr the input stream

*/


static ListExpr pdelete_tm(ListExpr args)
{
  NList l(args);

  string e1 = expects( Symbol::STREAM(), PTuple::BasicType() );

  static const string err1 = "pdelete expects " + e1 + "!";

  if ( !l.hasLength(1) )
    return l.typeError(err1);

  NList attrs;
  if ( !checkStreamPTuple( l.first(), attrs ) )
    return l.typeError(err1);

  return makeStreamTuple(attrs).listExpr();
}

static int pdelete_vm( Word* args, Word& result, int message,
                       Word& local, Supplier s)
{
  // args[0]: Input stream(ptuple(y)))
  static const string pre = "pdelete: ";

  switch (message)
  {
    case OPEN :
    {
      qp->Open( args[0].addr );
      return 0;
    }

    case REQUEST :
    {
      bool endOfStream = false;
      do
      {
        PTuple *pt = nextPTuple( args[0] );

        if (pt)
        {
           if (pt->marker) // marker tuple detected
           {
             //TRACE( pre << "Marker: " << *(pt->marker) )
             delete pt;
           }
           else // a normal tuple
           {
             //TRACE( pre << "Tuple: " << *(pt->tuple) )
             result.addr = pt->tuple;
             delete pt;
             return YIELD;
           }
        }
        else
        {
          endOfStream = true;
        }

      } while ( !endOfStream );
      return CANCEL;
    }

    case CLOSE :
    {
      //TRACE(pre << "CLOSE received")
      qp->Close( args[0].addr );
      return 0;
    }

    default : { assert(false); }
  }
  return 0;
}

/*
4.2 Operator ~pshow~

This operator does the following type mapping:

----
(stream(ptuple(y))) -> (stream(ptuple(y)))
----

the value mapping prints out all  markers of the input stream.

*/


static ListExpr pshow_tm(ListExpr args)
{
  NList l(args);

  string e1 = expects( Symbol::STREAM(), PTuple::BasicType() );

  static const string err1 = "pshow expects " + e1 + "!";

  if ( !l.hasLength(1) )
    return l.typeError(err1);

  NList attrs;
  if ( !checkStreamPTuple( l.first(), attrs ) )
    return l.typeError(err1);

  return l.first().listExpr();
}

static int pshow_vm( Word* args, Word& result, int message,
                     Word& local, Supplier s)
{
  // args[0]: Input stream(ptuple(y)))
  static const string pre = "pshow: ";
  static const bool showall = RTFlag::isActive("pshow:all");

  switch (message)
  {
    case OPEN :
    {
      qp->Open( args[0].addr );
      return 0;
    }

    case REQUEST :
    {
      PTuple *pt = nextPTuple( args[0] );
      if (pt)
      {
        if (pt->marker) // marker tuple detected
        {
          cerr << *(pt->marker) << endl;
        }
        else {
         if (showall)
          cerr << static_cast<void*>( pt->tuple ) << endl;
	}
        result.addr = pt;
        return YIELD;
      }
      else
      {
        result.addr = 0;
        return CANCEL;
      }
    }

    case CLOSE :
    {
      qp->Close( args[0].addr );
      return 0;
    }

    default : { assert(false); }
  }
  return 0;
}



/*
4.3 Type Mapping Operators ~PSTREAM1~ and ~PSTREAM2~

Thesr operators do the following type mappings:

----
(stream(ptuple(y)) ... ) -> (stream(tuple(y)))
((...) (stream(ptuple(y))) ... ) -> (stream(tuple(y)))
----

*/

static ListExpr PSTREAM1_tm(ListExpr args)
{
  NList l(args);

  //TRACE("PSTREAM1_tm:" << l.convertToString())
  string e1 = expects( Symbol::STREAM(), PTuple::BasicType() );

  static const string err1 = "PSTREAM1 expects (" + e1 + "...)!";

  if ( !(l.length() >= 1) )
    return l.typeError(err1);

  NList attrs;
  if ( !checkStreamPTuple( l.first(), attrs ) )
    return l.typeError(err1);

  return makeStreamTuple(attrs).listExpr();
}

static ListExpr PSTREAM2_tm(ListExpr args)
{
  NList l(args);

  string e1 = expects( Symbol::STREAM(), PTuple::BasicType() );

  static const string err1 = "PSTREAM2 expects ((...) " + e1 + "...)!";

  if ( !(l.length() >= 2) )
    return l.typeError(err1);

  NList attrs;
  if ( !checkStreamPTuple( l.second(), attrs) )
    return l.typeError(err1);

  return makeStreamTuple(attrs).listExpr();
}


/*
4.3 Operator ~puse~

This operator does the following type mapping:

----
(stream(ptuple(y)) (map (stream(tuple(y))) (stream(tuple(z)))) )
  -> (stream(ptuple(z)))
----

The value mapping passes over all marker tuples from
the input stream to the output stream. Therefore it uses internally a
marker queue which stores incoming markers until the first tuple of output
is produced. This is needed since calling function qp->Request() with the
parameter functions node of the query tree will block until a first tuple is
returned, but meanwhile there could be received some markers from the input
stream which need to be filtered out by the ~puse~ operator.

*/


static ListExpr puse_tm(ListExpr args)
{
  NList l(args);

  static const string e1 = expects( Symbol::STREAM(), PTuple::BasicType() );

  static string err1 = "Expecting input (" + e1 + ")!";

  if ( !checkLength( l, 2, err1 ) )
    return l.typeError( err1 );

  NList attrs;
  if ( !checkStreamPTuple( l.first(), attrs) )
    return l.typeError(err1);

  // Test the parameter function's signature
  static const string err2 = "Expecting as second argument a function "
                             "(map (stream(tuple(y))) (stream(tuple(z))))";
  vector<NList> sig;
  if ( !checkMap( l.second(), 1, sig ) )
    return l.typeError( argNotCorrect(2) + err2);

  NList attrs2;
  if ( !checkStreamTuple(sig[0], attrs2) )
    return l.typeError( "First argument of parameter function not correct!\n"
                        "Received " + sig[0].convertToString() + "." );

  if ( !(attrs == attrs2) )
    return l.typeError( "Tuple types do not match!\n"
                        "Received " + attrs2.convertToString() + "." );

  if ( !checkStreamTuple(sig[1], attrs2) )
    return l.typeError( "Result type of parameter function not correct!\n"
                        "Received " + sig[1].convertToString() + "." );

  return makeStreamPTuple(attrs2).listExpr();
}

struct MarkerQueue {

   MarkerQueue() : max(0), endOfStream(false) {}
   ~MarkerQueue()
   {
     assert( q.empty() );
     //cout << "Max. queue size: " << max << endl;
   }

   inline const Marker* removeFront()
   {
     const Marker* m = q.front();
     q.pop();
     //TRACE( "puse: remove Marker: " << *m )
     return m;
   }

   inline bool empty() const { return q.empty(); }

   inline void push(const Marker* m)
   {
     q.push(m);
     if (q.size() > max)
       max = q.size();
   }


   inline Tuple* getNextTuple(Word& fun) {

     if (endOfStream)
       return 0;

     Tuple* t = nextTuple(fun);
     if (!t)
       endOfStream = true;
     return t;
   };

   queue< const Marker* > q;
   size_t max;
   bool endOfStream;
};


static int puse_vm( Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  // args[0]: Input stream(ptuple(y)))
  // args[1]: map stream(tuple(y))) -> stream(tuple(z))


  static const string pre("puse: ");
  //Word funresult;
  ArgVectorPointer funargs;

  Word inStream = args[0];
  Word fun = args[1];

  MarkerQueue* m = static_cast<MarkerQueue*>( local.addr );

  switch ( message )
  {

    case OPEN: {

      //TRACE(pre << "Open received")

      // retrieve argument vector
      funargs = qp->Argument(fun.addr);

      // store the supplier of this operator at the last argument
      // position. This indicates that the argument is a stream.
      (*funargs)[MAXARG-1] = SetWord(s);

      // initalize local information. The first tuple of a nonempty
      // ptuple stream must be a marker tuple.
      m = new MarkerQueue();
      local.addr = m;

      // The parameter function's return type is a stream
      // hence we need to open it
      qp->Open(fun.addr);
      //TRACE("param function opened!")

      return 0;
    }
    case REQUEST: {

        //TRACE(pre << "Request received")
        bool ok = false;
        do {
        if ( !m->empty() ) // is a marker present?
        {
           result.addr = new PTuple( m->removeFront() );
           ok = true;
           break;
        }
        else
        {
          // return elements computed by the param function
          Tuple* t = m->getNextTuple(fun);

          // a side effect of nextTuple(fun) could be that new markers
          // are pushed to the marker queue

          if (t) // handle tuple pointer
          {
            result.addr = new PTuple(t);
            ok = true;
            break;
          }
          else
          {
            result.addr=0;
            ok = false;
          }
        }
        } while ( !m->empty() );

        if (ok) {
          assert(result.addr);
          return YIELD;
        }
        else {
          return CANCEL;
        }
    }
    case FUNMSG+OPEN: { // just open the input stream

      //TRACE(pre << "Message FUNMSG+OPEN received")
      qp->Open(inStream.addr);
      return 0;
    }

/*
The message below will map a ptuple of the input stream to a ~normal~ tuple
to be evaluated by the parameter function. If a marker is

*/

    case FUNMSG+REQUEST: {

      //TRACE(pre << "Message FUNMSG+REQUEST received")

      bool endOfStream = false;
      do
      {
        PTuple* pt = nextPTuple(inStream);
        if (pt)
        {
          if (pt->tuple) // return tuple to parameter function
          {
            result = SetWord( pt->tuple );
            return YIELD;
          }
          else // save marker in queue
          {
            //TRACE( pre << "save Marker: " << *(pt->marker) )
            m->push( pt->marker );
          }
          // don't delete pt, only operators consuming streams of PTuple
          // should do it.
        }
        else
        {
          endOfStream=true;
        }
      }
      while (!endOfStream);

      return CANCEL;
    }
    case FUNMSG+CLOSE: {

      // This message must be ignored since we will send a CLOSE message
      // to our input stream when requested by our parent node

      TRACE(pre << "Message FUNMSG+CLOSE received")
      return 0;
    }
    case CLOSE: {

      //TRACE(pre << "Message CLOSE received")
      // close the input stream
      qp->Close(inStream.addr);

      // send a close message to the parameter fun in order that it can be
      // propagated to its childs.
      qp->Close(fun.addr);

      delete m;
      return 0;
    }
    default: {

       cerr << pre << "Cannot handle message " << message << endl;
    }
  }
  return 0;

}

/*
4.3 Operator ~pjoin2~

This operator does the following type mapping:

----
  ( stream(ptuple(y1)) stream(ptuple(y2))
    ( ( map (stream(tuple(y1))) (stream(tuple(y2))) (stream(tuple(z))) )
      ... N repeats ... )
  )
  -> (stream(ptuple(z)))
----

During type mapping the names of the function identifier are appended as string
arguments. The names must correspond to a join method. For each join method
a cost function is present and will be used for determining the best function.

The value mapping is organized in two phases:

   1 Probe phase: First some input is used to do a probe join. This will give
estimations about the input and output sizes.
   2 Eval phase: The cost functions are used to determine the algorithm which
   computes the join for the whole inputs. Therfore the buffered inputs must be
   streamed again.

The implementation of the first phase is done partly in the OPEN and
FUNMSG+REQUEST messages. In the case of a REQUEST message the operator will return
a tuple computed by the chosen evaluation function.

After the first phase a marker tuple with a first size estimation will be returned.
Since it may happen that the output is much smaller than the inputs we need to maintain
a marker queue as in the implementation of the ~puse~ operator.

*/


static ListExpr pjoin2_tm(ListExpr args)
{
  NList l(args);

  static const string e1 = expects( Symbol::STREAM(), PTuple::BasicType(),"y1");
  static const string e2 = expects( Symbol::STREAM(), PTuple::BasicType(),"y2");

  static string err1 = "Expecting input ( "
                       + e1 + " " + e2
                       + " (list of join-expressions) ";

  if ( !checkLength( l, 3, err1 ) )
    return l.typeError( err1 );

  NList attrs1;
  if ( !checkStreamPTuple( l.first(), attrs1) )
    return l.typeError( argNotCorrect(1) + err1);

  NList attrs2;
  if ( !checkStreamPTuple( l.second(), attrs2) )
    return l.typeError( argNotCorrect(2) + err1);


  //if ( l.third().str() != Symbols::CcString::BasicType() )
  //  return l.typeError( argNotCorrect(3) + err1);

  // Test the parameter function's signature
  static const string
  err2 = "Expecting as third argument a list of functions of type "
         "(map (stream(tuple(y1))) (stream(tuple(y1))) (stream(tuple(z))))";

  NList joinMaps = l.third();
  NList lastResultAttrs;
  NList fNames;

  int joinMapsLength = joinMaps.length();
  if (joinMapsLength < 0)
    return l.typeError( "Empty list of functions! " + argNotCorrect(4) + err2);

  for (int i=1; i < (joinMapsLength+1); i++)
  {
    vector<NList> sig;

    NList funDesc = joinMaps.elem(i);
    NList fName = funDesc.first();
    if ( !fName.isSymbol() )
      return l.typeError( "symbol atom for function name expected!" );

    string funcStr = fName.str();
    string pre = "Function " + funcStr + ": ";


    if ( !checkMap( funDesc.second(), 2, sig ) )
      return l.typeError( argNotCorrect(3) + err2);

    NList leftAttrs;
    if ( !checkStreamTuple(sig[0], leftAttrs) )
      return l.typeError( pre + "First argument not correct!\n"
                          "Received " + sig[0].convertToString() + "." );

    NList rightAttrs;
    if ( !checkStreamTuple(sig[1], rightAttrs) )
      return l.typeError( pre + "Second argument not correct!\n"
                          "Received " + sig[1].convertToString() + "." );

    NList resultAttrs;
    if ( !checkStreamTuple(sig[2], resultAttrs) )
      return l.typeError( pre + "Result type not correct!\n"
                          "Received " + sig[2].convertToString() + "." );

    if ( i==1 ) // first time define reference mapping
    {
      lastResultAttrs =  resultAttrs;
      fNames.makeHead( fName.toStringAtom() );
    }
    else // compare with previous mapping
    {
      fNames.append( fName.toStringAtom() );
      if ( !(attrs1 == leftAttrs) )
        return l.typeError( pre +
                            "Tuple type of the first arg. does not match! \n"
                            "Received " + leftAttrs.convertToString() + "." );

      if ( !(attrs2 == rightAttrs) )
        return l.typeError( pre +
                            "Tuple type of the second arg. does not match! \n"
                            "Received " + rightAttrs.convertToString() + "." );

      if ( !(lastResultAttrs == resultAttrs) )
        return l.typeError( pre +
                            "Tuple type of the result does not match! \n"
                            "Received " + resultAttrs.convertToString()
                            + ". But the result type of the "
                            + "previous function was "
                            + lastResultAttrs.convertToString() );
    }


  }

  NList appendSym(Symbol::APPEND());
  NList resultType(appendSym, fNames, makeStreamPTuple(lastResultAttrs));
  return resultType.listExpr();
}

/*
Class ~PTupleBuffer~ captures the first part of a stream of tuples
into a tuple buffer and allows to us it again.

*/

template<class T>
struct PTupleBuffer
{
  private:
    const string pre;
    TupleBuffer* buf;
    GenericRelationIterator* rit;
    StreamBase<T>& is;
    bool scanBuffer;
    bool bufferOnly;
    bool endReached;
    bool markerRead;
    size_t maxSize;
    Marker lastMarker;
    int tuplesCurrentPart;
    int tuplesCompleteParts;
    int requestedTuples;

  public:
    PTupleBuffer(const string& id, StreamBase<T>& stream, const int size ) :
     pre(id+": "),
     rit(0),
     is(stream),
     scanBuffer(false),
     bufferOnly(true),
     endReached(false),
     markerRead(false),
     maxSize(size),
     tuplesCurrentPart(0),
     tuplesCompleteParts(0),
     requestedTuples(0)
    {
      buf = new TupleBuffer(size);
    }
    ~PTupleBuffer()
    {
      showInfo();
      delete buf;
    }

    void showInfo()
    {
      TRACE(pre << "Stored tuples " << getNoTuples())
      SHOW(tuplesCompleteParts)
      SHOW(tuplesCurrentPart)
    }


    TupleBuffer* getBufPtr() {
       return buf;
    }

    int getNoTuples(bool useCtrs = false, int ctrNum = 0) {
      if (!useCtrs)
        return buf->GetNoTuples();
      return qp->GetCounter(ctrNum+1);
    }

    int getNoTuplesOfCompleteParts() {
      return tuplesCompleteParts;
    }

    int getRequestedTuples() {
       return tuplesCurrentPart + tuplesCompleteParts;
    }

    // used by pjoin1 to correct the number of received tuples
    void setRequestedTuples(const int num) {
       tuplesCurrentPart = 0;
       tuplesCompleteParts = num;
    }

    int getTotalSize(bool useCtrs=false, int ctrNum = 0)
    {
      if (!useCtrs)
        return nextInt( buf->GetTotalSize() );

      double f = (1.0 * qp->GetCounter(ctrNum+1)) / qp->GetCounter(ctrNum);
      return nextInt(f * buf->GetTotalSize());
    }

    inline bool end() const {
      return endReached;
    }

    void setLastMarker(const Marker& m) {
      lastMarker = m;
    }

    const Marker& getLastMarker() {
      assert(markerRead);
      return lastMarker;
    }

/*
The ~getNext~ function will remove marker tuples and stores the tuples in
th ~TupleBuffer~ until a memory overflow is reached.

*/
    inline bool storeNextTuple() {

        // get next ptuple from input
        PTuple* pt = (PTuple*) is.getNext();
        if (pt)
        {
          if (pt->tuple) // save tuple
          {
            TRACE( pre << "Tuple received!" )
            buf->AppendTuple(pt->tuple);
            tuplesCurrentPart++;
            return true;
          }
          else // store last marker information
          {
            TRACE( pre << "Marker: " << *(pt->marker) )
            lastMarker = *(pt->marker);

            // delete the marker, since the join operator
            // will create new ones.
            // delete pt;
            tuplesCompleteParts += tuplesCurrentPart;
            tuplesCurrentPart = 0;
	    markerRead=true;
            return true;
          }
        }
	//cerr << pre << ": End Reached!" << endl;
        endReached = true;
        return false;
    }

    bool getNextTuple(Word& result)
    {
      // use the internal Buffer if requested
      if (scanBuffer)
      {
        result.addr = rit->GetNextTuple();
        if (!result.addr) {
          TRACE(pre << "Buffer usage finished!")
          scanBuffer = false;
          // release stored tuples
          //buf->Clear();
          if (bufferOnly)
           return false;
          else
           return getNextTuple(result);
        }
        return true;
      }
      else
      {
        if (!endReached)
        {
          do {
            // get next ptuple from input
            PTuple* pt = (PTuple*) is.getNext();
            if (pt)
            {
              if (pt->tuple) // return tuple to parameter function
              {
                tuplesCurrentPart++;
                result.addr = pt->tuple;
                return true;
              }
              else // store last marker information
              {
                //TRACE( pre << "Marker: " << *(pt->marker) )
                lastMarker = *(pt->marker);

                // delete the marker, since the join operator
                // will create new ones.
                // delete pt;
                tuplesCompleteParts += tuplesCurrentPart;
                tuplesCurrentPart = 0;
              }
            }
            else
            {
              TRACE(pre << "End of input stream")
              // indicate end of stream
              endReached = true;
              result.addr = 0;
            }
          } while (!endReached);
        }
      }
      return false;
    }

/*
The function ~estimateInputCard~ uses the last read marker Information and
the number of read tuples (only complete partitions)
to estimate the cardinality of the input stream

*/

    int getTuplesOfCompleteParts()
    {
      return max(tuplesCompleteParts, tuplesCurrentPart);
    }

    inline int readPartitions() { return lastMarker.num; }
    inline int partSize() { return lastMarker.tuples; }

    int maxInputCard() {
      assert(markerRead);
      return max(lastMarker.parts,1) * lastMarker.tuples;
    }

    int estimateInputCard(const bool useCtrs = false, int ctrNum = 0)
    {
      TRACE("Input cardinality estimation")
      SHOW(useCtrs)
      SHOW(ctrNum)
      SHOW(lastMarker.num)
      SHOW(lastMarker.parts)

      // expected tuples
      double expected = max(lastMarker.num,1) * lastMarker.tuples;

      // received tuples
      double received = getTuplesOfCompleteParts();

      if (endReached)
        received = getNoTuples();

      if (useCtrs)
      {
        // determine input selectivity by counters used in the
        // plan of the probe join.
        double Ctr1 = qp->GetCounter(ctrNum);
        double Ctr2 = qp->GetCounter(ctrNum+1);
        SHOW(Ctr1)
        SHOW(Ctr2)
        expected = max(Ctr1,1.0);
        received = max(Ctr2,1.0);
      }

      SHOW(expected)
      SHOW(received)

      // cardinality. If not a complete partition of tuples
      // was read in then the formula below may underestimate
      // the input card.
      double card = 0.0;

      if (lastMarker.parts != 0)
      {
        card  = lastMarker.tuples
                  * max(lastMarker.parts,1)
                      * (received / expected);
      }
      else
      {
        card = received;
      }
      SHOW(card)

      return static_cast<int>( ceil(card) );
    }


    void reset(const bool bufOnly = true)
    {
      rit = buf->MakeScan();
      scanBuffer = true;
      bufferOnly = bufOnly;
    }

    inline bool overFlow() {
       return getTotalSize() > maxSize;
    }

};


template<class StreamType>
  struct PJoinInfo {

    typedef enum { probe, eval } JoinState;

    // used to maintain the evaluation functions
    FunVector* evalFuns;

    // used to maintain the cost functions
    CostFunctions* costFuns;

    StreamOpAddr leftIs;
    StreamType rightIs;

    PTupleBuffer<StreamOpAddr>* leftBuf;
    PTupleBuffer<StreamType>* rightBuf;

    JoinState state;

    const int maxMem;

    // index between [0,n-1] pointing to the best evaluation function
    int bestPos;
    StreamOpAddr bestFun;

    // estimated cardinalites of input and output
    int leftCard;
    int rightCard;
    int resultCard;
    int leftAvgTupSize;
    int rightAvgTupSize;
    int resultTuples;

    float joinSel;
    bool isSelfJoin;
    bool useCtrs;
    bool doProbeJoin;

    int& instanceNum;
    int ctrNum;

    bool bufReset[2];

    PartCtr parts;

    PJoinTuple* info;

    PJoinInfo( StreamOpAddr left,
               StreamType right,
               int& instanceCtr,
               int num = 0,
               int mem = 1024*1024         ) :
      evalFuns(0),
      leftIs(left),
      rightIs(right),
      state(probe),
      maxMem(mem),
      bestPos(1),
      leftCard(0),
      rightCard(0),
      resultCard(0),
      leftAvgTupSize(0),
      rightAvgTupSize(0),
      resultTuples(0),
      joinSel(0.0),
      isSelfJoin(false),
      useCtrs(false),
      doProbeJoin(true),
      instanceNum(instanceCtr),
      ctrNum(num)
    {
      TRACE_FILE("pjoin.traces")

      info = new PJoinTuple();

      leftBuf = new PTupleBuffer<StreamOpAddr>( "left", leftIs, maxMem );
      rightBuf = new PTupleBuffer<StreamType>( "right", rightIs, maxMem );
      evalFuns = new FunVector();

      bufReset[0] = false;
      bufReset[1] = false;
    }
    ~PJoinInfo()
    {
      // close the input streams
      leftIs.close();
      rightIs.close();

      // send a close message to the parameter functions
      // in order that it can be propagated to its childs.
      bestFun.close();

      const int leftTuples = leftBuf->getRequestedTuples();
      const int rightTuples = rightBuf->getRequestedTuples();

      const int resultErr = relErr(resultCard, resultTuples);
      const int leftErr = relErr(leftCard, leftTuples);
      const int rightErr = relErr(rightCard, rightTuples);

      const float joinExact = (resultTuples / (max(leftTuples,1) * 1.0))
	                      / (max(rightTuples,1) * 1.0);
      const int joinSelErr = relErr( joinSel, joinExact);


      instanceNum++;
      info->id = instanceNum;

      info->arg1_est  = leftCard;
      info->arg1_real = leftTuples;
      info->arg1_err  = leftErr;

      info->arg2_est  = rightCard;
      info->arg2_real = rightTuples;
      info->arg2_err  = rightErr;

      info->result_est  = resultCard;
      info->result_real = resultTuples;
      info->result_err  = resultErr;

      info->sel_est  = joinSel;
      info->sel_real = joinExact;
      info->sel_err  = joinSelErr;

      FunInfo& f = evalFuns->get(bestPos);
      info->usedFunction = f.getName();


      pjoinRel->append(info, false);

      delete leftBuf;
      delete rightBuf;
      delete evalFuns;
      delete costFuns;
    }


    int relErr(const float a, const float b) {
      float c = b;
      if (b == 0.0) // avoid infinite values!
        c = 1.0;
      return static_cast<int>( ceil(((abs(a - b) * 1.0) / c) * 100.0) );
    }

    void setRequestedTuples() {
      rightBuf->setRequestedTuples( qp->GetCounter(ctrNum+2) );
    }


/*
Function ~computeCards~ will compute estimated cardinalities for the
input, output, and result stream.

*/
    void computeCards(const int resultTuples)
    {
       SHOW(resultTuples)
       SHOW(ctrNum)
       leftCard = leftBuf->estimateInputCard();
       rightCard = rightBuf->estimateInputCard(useCtrs, ctrNum);
       //SHOW(leftCard)
       //SHOW(rightCard)

       int leftRead = max( leftBuf->getNoTuples(), 1 );
       int rightRead = max( rightBuf->getNoTuples(useCtrs, ctrNum), 1);

       //avg tuple size
       leftAvgTupSize = nextInt( leftBuf->getTotalSize() / leftRead );
       rightAvgTupSize =
                   previousInt(
	                rightBuf->getTotalSize(useCtrs, ctrNum) / rightRead );

       // join selectivity
       if (resultTuples == 0)
         cerr << "Warning: probejoin returned no tuples!" << endl;

       /*
        * Hook for correction of the selectivity estimation
        * in the case of self joins. To be continued!
        *
       if (isSelfJoin)
       {
         float selfJoinTuples = (1.0 * leftRead * leftRead / leftCard);
       }
       */

       if ( (leftCard == 0) || (rightCard == 0) )
       {
         // in some cases we may have recognized that the input card is zero!
         joinSel = 0.0;
       }
       else
       {
         joinSel = (1.0 * max(resultTuples,1)) / (leftRead * rightRead);
       }
       SHOW(joinSel)

       // cardinality of the join result
       const float leftCardF = leftCard;
       const float rightCardF = rightCard;
       const float resultCardF = ceil(joinSel * leftCardF * rightCardF);
       resultCard = static_cast<int>( resultCardF );
       SHOW(resultCard)

    }

/*
The probe join will be stopped if

   * a memory overflow happens

   * more than ~n~ tuples (only complete parts) for each input are read

*/

    inline bool stopLoading(const int n=1000)
    {
      const int bufferSize = leftBuf->getTotalSize()
                                + rightBuf->getTotalSize();
      const bool overFlow = bufferSize > 2*maxMem;

      bool stop = false;
      if (overFlow)
      {
        TRACE("*** OVERFLOW! ***")
        stop = true;
      }
      else
      {
        int storedTuples = leftBuf->getNoTuples() + rightBuf->getNoTuples();
        stop = (storedTuples >= n);
        stop = ( stop || (leftBuf->end() && rightBuf->end()) );
      }

      if (stop)
      {
        TRACE("*** Enough tuples read ***")
        SHOW(bufferSize)
	// assign values to the ~info~ tuple
	info->probe_arg1 = leftBuf->getNoTuples();
	info->probe_arg2 = rightBuf->getNoTuples();
        // reset streams
        leftBuf->reset();
        leftBuf->showInfo();
        rightBuf->reset();
        rightBuf->showInfo();
      }
      return stop;
    }

    const long cpuOps() {
      const long cpu1 = Counter::getRef("CcInt::Compare");
      const long cpu2 = Counter::getRef("CcInt::Less");
      const long cpu3 = Counter::getRef("CcInt::Equal");
      const long cpu4 = Counter::getRef("CcInt::HashValue");
      return (cpu1 + cpu2 + cpu3 + cpu4);
    }

    // By default 0,05% of the cartesian product will
    // be used as input for the probe join
    int computeProbeSize( int est, double scaleP, int minVal, int maxVal  )
    {
      int ps = nextInt( est * scaleP );
      SHOW(ps)
      ps = min( max(ps, minVal), maxVal );

      SHOW(est)
      SHOW(ps)

      return ps;
    }

    void loadTupleBuffers() {

      TRACE("*** load TupleBuffers ***")


      int k=2;
      for(int i=0; i < k; i++) {
        leftBuf->storeNextTuple();
        rightBuf->storeNextTuple();
      }

      cerr << leftBuf->getLastMarker() << endl;;
      cerr << rightBuf->getLastMarker() << endl;
      int leftEst = leftBuf->maxInputCard();
      int rightEst = rightBuf->maxInputCard();

      // Define 0.05% of the cartesian product as sample size
      Environment& env = Environment::getInstance();
      double pScale = env.getFloat("SEC_pScale", 0.05);
      int pMinRead = env.getInt("SEC_pMinRead", 500);
      int pMaxRead = env.getInt("SEC_pMaxRead", 500);
      SHOW(pScale)
      SHOW(pMinRead)
      SHOW(pMaxRead)

      int leftTuples = computeProbeSize(leftEst, pScale, pMinRead, pMaxRead);
      int rightTuples = computeProbeSize(rightEst, pScale, pMinRead, pMaxRead);
      k = leftTuples + rightTuples;

      // adapt the sample to the currently estimated
      // input sizes
      while ( !stopLoading(k) )
      {
        if (leftBuf->getNoTuples() < leftTuples || rightBuf->end() )
          leftBuf->storeNextTuple();
        if (rightBuf->getNoTuples() < rightTuples || leftBuf->end() )
          rightBuf->storeNextTuple();

	static int ctr=50;
	ctr--;
	if (ctr == 0)
	{
	  ctr=50;
	  int leftEst = leftBuf->estimateInputCard();
	  int rightEst = rightBuf->estimateInputCard(useCtrs, ctrNum);
	  leftTuples = computeProbeSize(leftEst, pScale, pMinRead, pMaxRead);
	  rightTuples = computeProbeSize(rightEst, pScale, pMinRead, pMaxRead);
          k = leftTuples + rightTuples;
	}
      }
    }

    void showBufResetStates() {
      SHOW(bufReset[0]);
      SHOW(bufReset[1]);
    }

    bool resetBuffer(const int no) {

      assert( (no == 1) || (no == 2) );
      TRACE("resetBuffer: " << no)

      if ( bufReset[no-1] == false ) {
        if (no == 1)
          leftBuf->reset(false);
        else
          rightBuf->reset(false);

        bufReset[no-1] = true;
        return true;
      }
      return false;

    }

    void runProbeJoin()
    {
      StopWatch probeTimer;

      Supplier first = (evalFuns->get(0)).getSupplier();
      info->join = (evalFuns->get(0)).getName();

      // open both input streams
      // Recurses into the operator tree and calls
      // ~runProbeJoin~ for subordinated pjoins!
      leftIs.open();
      rightIs.open();


      loadTupleBuffers();
      // Loading the tuple buffers will cause
      // requests to subtrees. Hence the time
      // measurement can only start after the tuple
      // buffers are filled.
      probeTimer.start();
      cout << "* Start Probe Join *" << endl;

      qp->Open(first);

      Tuple* t = nextTuple(first);
      int probeReceived = 0;
      while( t )
      {
        probeReceived++;
        t = nextTuple(first);
      }
      qp->Close(first);

      cout << "* End Probe Join *" << probeTimer.diffTimes() << endl;
      TRACE( "\n*** Probe join finished! ***\n" )

      computeCards(probeReceived);
      computeBestFunction();

      // assgin values to the info tuple
      info->probe_cpuOps = cpuOps();
      info->probe_seconds = probeTimer.diffSecondsReal();
      info->probe_result = probeReceived;

      // open the chosen evaluation function
      bestFun.open();
      doProbeJoin=false;

    }


    inline void nextPTuple(Word& result)
    {
      static int call=0;
      call++;

      /*
      if (doProbeJoin) {
	cout << call << "-pjoin: runProbeJoin()" << endl;
        runProbeJoin();
      }*/

      if ( parts.resetIfNeeded() )
      {
        result.addr = new PTuple( parts.newMarker() );
      }
      else
      {
        Tuple* t =  (Tuple*) bestFun.getNext();
        if (t) {
          result.addr = new PTuple( t);
          resultTuples++;
        } else {
          result.addr = 0;
        }
      }
    }

    void computeBestFunction()
    {
      TRACE( "\n*** START Computing best function ***\n" )

      // initialize ouput partSize and expected parts.
      const int tuples = max( leftBuf->partSize(), rightBuf->partSize() );
      //const int numOfParts = max( resultCard / tuples, 1 );
      parts.init(tuples, resultCard);

      SHOW(parts.tuples)
      SHOW(parts.parts)
      //SHOW(numOfParts)

      CostParams cp( leftCard, leftAvgTupSize,
                     rightCard, rightAvgTupSize, joinSel );
      SHOW(cp)
      costFuns = new CostFunctions();

      bool useHint = false;
      size_t useIndex = 0;
      for (size_t i=1; i<evalFuns->size(); i++)
      {
	string name = (evalFuns->get(i)).getName();
	if ( hasPrefix("use_", name) ) {
	  removePrefix("use_", name);
	  useHint = true;
	  useIndex = i;
        }
	SHOW(name)
        bool ok = costFuns->append( name, i );
        assert(ok);
      }

      const CostInfo ci = costFuns->findBest(cp);
      SHOW(ci)
      bestPos = ci.cf->index;

      bool pAllowHints =
	       Environment::getInstance().getBool("SEC_pAllowHints", false);
      SHOW(pAllowHints)

      if (pAllowHints && useHint) {
        bestPos = useIndex;
        cout << "Using hint with index " << useIndex << endl;
      }
      assert( bestPos < (int)evalFuns->size() );

      FunInfo& f = evalFuns->get(bestPos);
      bestFun = StreamOpAddr(f.getSupplier());
      cout << "Using " << f.getName() << endl;

      TRACE( "\n*** END Computing best function ***\n" )
    }

  };


static int pjoin2_vm( Word* args, Word& result, int message,
                      Word& local, Supplier s)
{
  // args[0]: Input stream(ptuple(y1)))
  // args[1]: Input stream(ptuple(y2)))
    // args[2]: A string which defines if the join is a self join or not
  // args[2]: A list of map stream(tuple(y1)))x stream(tuple(y2))
  //          -> stream(tuple(z))
  // args[3]: A list of symbols for evaluation function names

  static const string pre("pjoin2: ");
  static int instanceCtr = 0;

  typedef PJoinInfo<StreamOpAddr> PJoin2_Info;

  PJoin2_Info* pj = static_cast<PJoin2_Info*>( local.addr );

  switch ( message )
  {

    case OPEN: {

      TRACE(pre << "Open received")

      // initialze local storage
      StreamOpAddr left(args[0].addr);
      StreamOpAddr right(args[1].addr);

      // A possible interface for triggering self join correction.
      //string joinType = StdTypes::GetString(args[2]);
      //SHOW(joinType)
      //bool isSelfJoin = joinType == "selfjoin";

      pj = new PJoin2_Info(left, right, instanceCtr);
      local.addr = pj;

      FunVector& evalFuns = *(pj->evalFuns);
      // load functions into funvector
      evalFuns.load(args[2], &args[3]);

      // save caller node in argument vectors
      for (size_t i=0; i < evalFuns.size(); i++ )
      {
        Supplier fun = evalFuns.get(i).getSupplier();
        qp->SetupStreamArg(fun, 1, s);
        qp->SetupStreamArg(fun, 2, s);
      }

      // open the output stream of the first function
      // and do the probe join
      pj->runProbeJoin();

      return 0;
    }
    case REQUEST: {

        //TRACE(pre << "Request received")
        // Do a request on the chosen evaluation function
        // a side effect of nextTuple(fun) could be that new markers
        // are read.
        pj->nextPTuple(result);
        //SHOW(result.addr)

        if (result.addr) // handle tuple pointer
          return YIELD;
        else
          return CANCEL;
    }
    case (1*FUNMSG)+OPEN:
    {
      // open the first (left) input stream
      TRACE(pre << "Message 1*FUNMSG+OPEN received")
      (pj->leftIs).open();
      return 0;
    }
    case (2*FUNMSG)+OPEN:
    {
      // open the second (right) input stream
      TRACE(pre << "Message 2*FUNMSG+OPEN received")
      (pj->rightIs).open();
      return 0;
    }


/*
The message below will map a ptuple of the input stream to a ~normal~ tuple
to be evaluated by the parameter function. If a marker is

*/

    case (1*FUNMSG)+REQUEST: {

      //TRACE(pre << "Message 1*FUNMSG+REQUEST received")

      // just collect all markers and store them in the queue
      pj->leftBuf->getNextTuple( result );

      if ( result.addr != 0)
        return YIELD;
      else
        return CANCEL;
    }

    case (2*FUNMSG)+REQUEST: {

      //TRACE(pre << "Message 2*FUNMSG+REQUEST received")

      // just collect all markers and store them in the queue
      pj->rightBuf->getNextTuple( result );

      if ( result.addr != 0)
        return YIELD;
      else
        return CANCEL;
    }

    case (1*FUNMSG)+CLOSE: {

      // This message must be ignored since we will send a CLOSE message
      // to our input stream when requested by our parent node

      TRACE(pre << "Message 1*FUNMSG+CLOSE received")
      pj->resetBuffer(1);
      return 0;
    }

    case (2*FUNMSG)+CLOSE: {

      // This message must be ignored since we will send a CLOSE message
      // to our input stream when requested by our parent node

      TRACE(pre << "Message 2*FUNMSG+CLOSE received")
      pj->resetBuffer(2);
      return 0;
    }


    case CLOSE: {

      TRACE(pre << "Message CLOSE received")

      // closing streams is done in the destructor of the JoinInfo instance
      delete pj;
      return 0;
    }
    default: {

       cerr << pre << "Cannot handle message " << message << endl;
    }
  }
  return 0;

}


/*
4.4 Operator ~pjoin1~

This operator does the following type mapping:

----
  ( stream(ptuple(y1)) rel(tuple(y2))
    ( ( map (stream(tuple(y1))) (rel(tuple(y2))) (stream(tuple(z))) )
      ... N repeats ... )
  )
  -> (stream(ptuple(z)))
----

During type mapping the names of the function identifier are appended as string
arguments. The names must correspond to a join method. For each join method
a cost function is present and will be used for determining the best function.
The value mapping is organized like ~pjoin1~.


*/


static ListExpr pjoin1_tm(ListExpr args)
{
  NList l(args);

  static const string e1 = expects( Symbol::STREAM(), PTuple::BasicType(),"y1");
  static const string e2 = expects( Relation::BasicType(),
                                    Tuple::BasicType(), "y2" );

  static string err1 = "Expecting input ("
                       + e1 + " " + e2
                       + " int (<list of functions>) ";

  if ( !checkLength( l, 4, err1 ) )
    return l.typeError( err1 );

  NList attrs1;
  if ( !checkStreamPTuple( l.first(), attrs1) )
    return l.typeError( argNotCorrect(1) + err1);

  NList attrs2;
  if ( !checkRelTuple( l.second(), attrs2) )
    return l.typeError( argNotCorrect(2) + err1);

  if ( !l.third().isSymbol( CcInt::BasicType() ) )
    return l.typeError( argNotCorrect(3) + err1);


  // Test the parameter function's signature
  static const string
  err2 = "Expecting as third argument a list of functions of type "
         "(map (stream(tuple(y1))) (rel(tuple(y1))) (stream(tuple(z))))";

  NList joinMaps = l.fourth();
  NList lastResultAttrs;
  NList fNames;

  int joinMapsLength = joinMaps.length();
  for (int i=1; i < (joinMapsLength+1); i++)
  {
    vector<NList> sig;

    NList funDesc = joinMaps.elem(i);
    NList fName = funDesc.first();
    if ( !fName.isSymbol() )
      return l.typeError( "symbol atom for function name expected!" );

    string funcStr = fName.str();
    string pre = "Function " + funcStr + ": ";


    if ( !checkMap( funDesc.second(), 2, sig ) )
      return l.typeError( argNotCorrect(3) + err2);

    NList leftAttrs;
    if ( !checkStreamTuple(sig[0], leftAttrs) )
      return l.typeError( pre + "First argument not correct!\n"
                          "Received " + sig[0].convertToString() + "." );

    NList rightAttrs;
    if ( !checkRelTuple(sig[1], rightAttrs) )
      return l.typeError( pre + "Second argument not correct!\n"
                          "Received " + sig[1].convertToString() + "." );

    NList resultAttrs;
    if ( !checkStreamTuple(sig[2], resultAttrs) )
      return l.typeError( pre + "Result type not correct!\n"
                          "Received " + sig[2].convertToString() + "." );

    if ( i==1 ) // first time define reference mapping
    {
      lastResultAttrs =  resultAttrs;
      fNames.makeHead( fName.toStringAtom() );
    }
    else // compare with previous mapping
    {
      fNames.append( fName.toStringAtom() );
      if ( !(attrs1 == leftAttrs) )
        return l.typeError( pre +
                            "Tuple type of the first arg. does not match! \n"
                            "Received " + leftAttrs.convertToString() + "." );

      if ( !(attrs2 == rightAttrs) )
        return l.typeError( pre +
                            "Tuple type of the second arg. does not match! \n"
                            "Received " + rightAttrs.convertToString() + "." );

      if ( !(lastResultAttrs == resultAttrs) )
        return l.typeError( pre +
                            "Tuple type of the result does not match! \n"
                            "Received " + resultAttrs.convertToString()
                            + ". But the result type of the "
                            + "previous function was "
                            + lastResultAttrs.convertToString() );
    }


  }

  NList appendSym(Symbol::APPEND());
  NList resultType(appendSym, fNames, makeStreamPTuple(lastResultAttrs));
  return resultType.listExpr();
}


static int pjoin1_vm( Word* args, Word& result, int message,
                      Word& local, Supplier s)
{
  // args[0]: Input stream(ptuple(y1)))
  // args[1]: Input rel(tuple(y2)))
  // args[2]: Ctr number
  // args[3]: A list of map stream(tuple(y1)))x rel(tuple(y2))
  //          -> stream(tuple(z))
  // args[4]: A list of symbols for evaluation function names

  static const string pre("pjoin1: ");
  static int instanceCtr = 0;

  typedef PJoinInfo<RelationAddr> PJoin1_Info;

  PJoin1_Info* pj = static_cast<PJoin1_Info*>( local.addr );

  switch ( message )
  {

    case OPEN: {

      TRACE(pre << "Open received")

      // initialze local storage
      StreamOpAddr left(args[0].addr);
      GenericRelation* r = getArg<GenericRelation>( args[1] );
      RelationAddr right(r);
      right.open();

      int ctrNum = StdTypes::GetInt(args[2]);
      pj = new PJoin1_Info(left, right, instanceCtr, ctrNum);
      local.addr = pj;

      FunVector& evalFuns = *(pj->evalFuns);
      // load functions into funvector
      evalFuns.load(args[3], &args[4]);

      // save caller node in argument vectors
      // and store the relation as second argument for
      // the parameter functions.
      for (size_t i=0; i < evalFuns.size(); i++ )
      {
        Supplier fun = evalFuns.get(i).getSupplier();
        qp->SetupStreamArg(fun, 1, s);

        ArgVectorPointer funargs = qp->Argument(fun);
        if (i == 0) {
          (*funargs)[1] = SetWord( pj->rightBuf->getBufPtr() );
        } else {
          (*funargs)[1] = SetWord(r);
        }
      }

      // open the output stream of the first function
      // and do the probe join
      pj->useCtrs = true;
      pj->runProbeJoin();


      return 0;
    }
    case REQUEST: {

        //TRACE(pre << "Request received")
        // Do a request on the chosen evaluation function
        // a side effect of nextTuple(fun) could be that new markers
        // are read.
        pj->nextPTuple(result);
        //SHOW(result.addr)

        if (result.addr) // handle tuple pointer
          return YIELD;
        else
          return CANCEL;
    }
    case (1*FUNMSG)+OPEN:
    {
      // open the first (left) input stream
      TRACE(pre << "Message 1*FUNMSG+OPEN received")
      (pj->leftIs).open();
      return 0;
    }


/*
The message below will map a ptuple of the input stream to a ~normal~ tuple
to be evaluated by the parameter function. If a marker is

*/
    case (1*FUNMSG)+REQUEST: {

      //TRACE(pre << "Message 1*FUNMSG+REQUEST received")

      // just collect all markers and store them in the queue
      pj->leftBuf->getNextTuple( result );

      if ( result.addr != 0)
        return YIELD;
      else
        return CANCEL;
    }


    case (1*FUNMSG)+CLOSE: {

      // This message must be ignored since we will send a CLOSE message
      // to our input stream when requested by our parent node

      TRACE(pre << "Message 1*FUNMSG+CLOSE received")
      pj->resetBuffer(1);
      return 0;
    }

    case CLOSE: {

      TRACE(pre << "Message CLOSE received")

      // correct the right PTupleBuffer's information about requested
      // tuples. Since the embedded executions plans will not request the
      // buffer (argument .. is a base relation) it will contain only the
      // number of probe tuples.
      pj->setRequestedTuples();

      // closing streams is done in the destructor of the JoinInfo instance
      delete pj;
      return 0;
    }
    default: {

       cerr << pre << "Cannot handle message " << message << endl;
    }
  }
  return 0;

}



static ListExpr pcreate_tm(ListExpr args)
{
  NList l(args);
  NList attrs;

  static const string e1 = expects( Symbol::STREAM(), Tuple::BasicType() );
  string err1 = "pcreate expects (" + e1 + "int)!";

    if ( !checkLength( l, 2, err1 ) )
      return l.typeError( err1 );

    if ( !checkStreamTuple( l.first(), attrs) )
      return l.typeError( argNotCorrect(1) + err1);

    if ( !l.second().isSymbol(CcInt::BasicType()) )
      return l.typeError( argNotCorrect(2) + err1);

  return makeStreamPTuple(attrs).listExpr();
}


static int pcreate_vm( Word* args, Word& result, int message,
                       Word& local, Supplier s )
{
  // args[0]: Input stream(tuple(y)))
  // args[1]: int
  static const string pre = "pcreate: ";
  BufferedStreamInfo* info = static_cast<BufferedStreamInfo*>( local.addr );

  switch (message)
  {
    case OPEN :
    {
      TRACE(pre << "OPEN")
      int partSize = StdTypes::GetInt(args[1]);
      info = new BufferedStreamInfo( args[0], partSize );
      local.addr = info;
      return 0;
    }

    case REQUEST :
    {
      // the nextPTuple function returns a marker or a tuple
      result.addr = info->nextPTuple();
      if ( result.addr )
      {
        return YIELD;
      }
      else
      {
        return CANCEL;
      }
    }

    case CLOSE :
    {
      //TRACE(pre << "CLOSE received!")
      delete info;

      return 0;
    }

    default : { assert(false); }
  }
}

static ListExpr pcreate2_tm(ListExpr args)
{
  NList l(args);

  static const string e1 = expects( Symbol::STREAM(), Tuple::BasicType() );
  static string err1 = "pcreate2 expects (" + e1 + "int int)!";

  if ( !checkLength(l, 3, err1) )
    return l.typeError( err1 );

  NList attrs;
  if ( !checkStreamTuple( l.first(), attrs) )
    return l.typeError( argNotCorrect(1) + err1);

  if ( !l.second().isSymbol(CcInt::BasicType()) )
    return l.typeError( argNotCorrect(2) + err1);

  if ( !l.third().isSymbol(CcInt::BasicType()) )
    return l.typeError( argNotCorrect(3) + err1);

  return makeStreamPTuple(attrs).listExpr();
}

static int pcreate2_vm( Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
  // args[0]: Input stream(ptuple(y)))
  // args[1]: int
  // args[2]: int
  return 0;
}


/*
4.4 The shuffling operators ~shuffle~ and ~memshuffle~

These operators are overloaded. Both have the same type mapping
hence they can use the same functions for type checking and evaluation
function selection.

*/


static ListExpr shuffleX_tm(ListExpr args, const string& op)
{
  NList l(args);

  static const string e1 = expects( Symbol::STREAM(), Tuple::BasicType() );
  static const string e2 = expects( Symbol::STREAM(), PTuple::BasicType() );
  string err1 = op + " expects (" + e1 + ") or (" + e2 + ")!";

  if ( !checkLength(l, 1, err1) )
    return l.typeError( err1 );

  NList attrs;
  if ( checkStreamTuple( l.first(), attrs) )
    return l.first().listExpr();

  if ( checkStreamPTuple( l.first(), attrs) )
    return l.first().listExpr();

  return l.typeError( argNotCorrect(1) + err1);
}

static ListExpr shuffle_tm(ListExpr args)
{
  return shuffleX_tm(args, "shuffle");
}

static ListExpr shuffle2_tm(ListExpr args)
{
  NList l(args);

  static const string e1 = expects( Symbol::STREAM(), Tuple::BasicType() );
  string err1 = "expecting (" + e1 + " x int)";

  if ( !checkLength(l, 2, err1) )
    return l.typeError( err1 );

  //cout << l.first() << endl;

  NList attrs;
  if ( checkStreamTuple( l.first(), attrs)
       && l.second().str() == CcInt::BasicType() )
  {
    return l.first().listExpr();
  }

  return l.typeError( argNotCorrect(1) + err1);
}


/*
The shuffle operator uses a tuple buffer of size $M$ and stores all incoming
tuples there. In the cas of an memory overflow it will work in the following
way

1 the tuples of the buffer are written in random order to disk

2 Fill the tuple buffer again. Hence the input relation is partitioned into
$N = StreamSize / M$ parts.

3 When the end of the input stream is reached read in tuples from each partition
$N$ in chunks of a logical page size $Lps$. In order to guarantee that all tuples
will be mixed we assume that $N * Lps = M$ which implies $StreamSize =
\frac{M^2}{Lps}$. For example, if $M=16MB$ and $Lps=32k$ we can randomize
relations with a size up to 8GB.

*/

struct ShuffleBuf {

  typedef vector<Tuple*> TupleBuf;

  TupleBuf buffer;
  size_t pos;      // pos is a positive value

  size_t M;
  size_t freeMem;
  int Lps;

  ShuffleBuf(int MaxMem, int LogicalPageSize) :
    pos(0),
    M(MaxMem),
    freeMem(MaxMem),
    Lps(LogicalPageSize)
  {}
  ~ShuffleBuf() {

     TupleBuf::const_iterator it = buffer.begin();
     while( it != buffer.end() ) {
       //(*it)->DecReference();
       (*it)->DeleteIfAllowed();
       it++;
     }
  }

  void shuffle() { random_shuffle(buffer.begin(), buffer.end()); }

  inline bool overFlow(const size_t tupleSize) {

     if ( freeMem < tupleSize )
       return true;

     freeMem -= tupleSize;
     return false;
  }

  // todo: implementation of the persistent case
  void handleOverFlow(Tuple* t) {

     cerr << "shuffle: A memory overflow happend!" << endl;
     buffer.push_back(t);

  }

  void append(Tuple* t)
  {
    t->IncReference();
//    if ( overFlow(t->GetExtSize()) )
//      handleOverFlow(t);
//    else
      buffer.push_back(t);
  }

  void open()
  {
    pos = buffer.size() + 1;
  }

  inline Tuple* getNext() {

    //cout << "pos: " << pos << endl;
    if ( pos == 1 )
      return 0;

    // Return values and start with buffer.size() - 1
    pos--;
    return buffer[pos-1];
  }
};

struct ShuffleBuf2 {

  vector<Tuple*> buffer;
  unsigned int size;

  ShuffleBuf2():size(0) {srand ( time(NULL) );}
  ~ShuffleBuf2() { }

  void append(Tuple* t)
  {
    buffer.push_back(t);
    ++size;
  }

  inline Tuple* getNext()
  {
    if ( size == 0)
      return 0;

    int i= rand() % size;
    Tuple* res= buffer[i];
    buffer[i]= buffer[size-1];
    --size;
    return res;
  }
};

  /*
struct ShuffleInfo {

  ShuffleInfo(int maxMem = 4 * 1024 * 1024, int maxTuples = 512)
    : persBuf(3/4 * maxMem),
      memBuf(maxMem),
      streamPos(0),
      pos(maxTuples/2),
      memTuples(maxTuples),
      skip(2),
      run(1),
      stepwidth(2),
      bufIter(0),
      memBufFinished(false),
      memCtr(0),
      persCtr(0)
  {}

  ~ShuffleInfo()
  {
    if ( bufIter != 0) {
      delete bufIter;
      bufIter = 0;
    }
  }

  void append(Tuple* t) {

    if ( (streamPos % skip) == 0 ) {
      // tuple will be selected for the	front part of the
      // output stream.

    if ( memBuf.GetNoTuples() == memTuples )
    {
      size_t p = nextReplacePos();
      if ( p >= memTuples )
      {
	// restart
        initReplacePos();
	p = nextReplacePos();
      }
        assert( p < memTuples );
        // store t in buffer
	t->IncReference();
        memBuf.SetTupleAtPos(p, t);
	assert( memBuf.InMemory() );
    }
    else
    {
      memBuf.AppendTuple(t);
    }

    } // end of if ( (streamPos % skip) == 0 )
    else
    {
      persCtr++;
      persBuf.AppendTuple(t);
    }
    streamPos++;
  }

  Tuple* getNext() {

    Tuple* result = 0;

    // return the tuples of the memory buffer
    if ( memBufFinished == false )
    {
      if ( bufIter == 0) {
        bufIter = memBuf.MakeScan();
      }
      memCtr++;
      result = bufIter->GetNextTuple();

      if (result == 0) {
        cerr << endl;
        cerr << "streamPos" << streamPos << endl;
        cerr << "memBuf: " << memBuf.GetNoTuples() << endl;
        cerr << "memCtr: " << memCtr << endl;
        cerr << "persBuf: " << persBuf.GetNoTuples() << endl;
        cerr << "persCtr: " << persCtr << endl;
        memBufFinished = true;
	delete bufIter;
	bufIter = 0;
      }
    }

    // return the tuples of the persistent buffer
    if ( memBufFinished == true)
    {
      if ( bufIter == 0) {
        bufIter = persBuf.MakeScan();
      }
      result = bufIter->GetNextTuple();
    }

    return result;
  }

  inline void initReplacePos()
  {
    skip = max((size_t)2, streamPos / memTuples);
    run++;
    pos = memTuples / 2;

    // compress buffer: replace i by 2i. The replaced positions
    // are transferred to persBuf.

    cerr << endl << "compressing buffer" << endl;
    for(size_t i=1; i < memTuples/2; i++)
    {
	// transfer pos 2*i-1 to persBuf
	Tuple* t = memBuf.GetTupleAtPos(2*i-1);
	assert(t != 0);
	t->DecReference();
	persCtr++;
	persBuf.AppendTuple(t);
    }

    for(size_t i=1; i < memTuples/2; i++)
    {
	// override pos i
	Tuple* t = memBuf.GetTupleAtPos(2*i);
	assert(t != 0);
	memBuf.SetTupleAtPos(i, t);
	memBuf.SetTupleAtPos(2*i, 0);
    }

    bool trace = false;

    if (trace) {
    for(size_t i=0; i < memTuples/2; i++) {
	Tuple* t = memBuf.GetTupleAtPos(i);
        cerr << i << ": ";
	if (t == 0)
          cerr << "null pointer!";
        else
	  cerr << *t;
	cerr << endl;
    }
    }

    cerr << endl;

    cerr << endl
	 << "run: " << run
	 << " streamPos: " << streamPos
	 << " bufindex: " << pos
	 << " skip: " << skip
	 << endl;
  }

  inline size_t nextReplacePos() {

    cerr << "p" << pos << ", ";
    size_t res = pos;
    pos++;
    return res;
  }

  TupleBuffer persBuf;
  TupleBuffer memBuf;
  size_t streamPos;
  size_t pos;
  size_t memTuples;
  size_t skip;
  size_t run;
  size_t stepwidth;

  GenericRelationIterator* bufIter;
  bool memBufFinished;
  size_t memCtr;
  size_t persCtr;

};
  */

/*
Operator ~shuffle~

*/

struct ShuffleInfo {

  ShuffleInfo(int maxMem = 4 * 1024 * 1024, int maxTuples = 100000 )
    : persBuf(maxMem),
      memBuf(maxMem),
      memTuples(500),
      skip(maxTuples/500),
      streamPos(0),
      bufIter(0),
      memBufFinished(false),
      trace(false)
  {}

  ~ShuffleInfo()
  {
    if ( bufIter != 0) {
      delete bufIter;
      bufIter = 0;
    }
  }

  void append(Tuple* t) {

    if ( (streamPos % skip) == 0 )
    {
      // tuple will be selected for the	front part of the
      // output stream.
      memBuf.AppendTuple(t);

    } // end of if ( (streamPos % skip) == 0 )
    else
    {
      persBuf.AppendTuple(t);
    }
    streamPos++;
  }

  Tuple* getNext() {

    Tuple* result = 0;

    // return the tuples of the memory buffer
    if ( memBufFinished == false )
    {
      if ( bufIter == 0) {
        bufIter = memBuf.MakeScan();
      }
      result = bufIter->GetNextTuple();

      if (result == 0) {

	if (trace) {
          cerr << endl;
          cerr << "streamPos: " << streamPos << endl;
          cerr << "memBuf   : " << memBuf.GetNoTuples() << endl;
          cerr << "persBuf  : " << persBuf.GetNoTuples() << endl;
        }
        memBufFinished = true;
	delete bufIter;
	bufIter = 0;
      }
    }

    // return the tuples of the persistent buffer
    if ( memBufFinished == true)
    {
      if ( bufIter == 0) {
        bufIter = persBuf.MakeScan();
      }
      result = bufIter->GetNextTuple();
    }

    return result;
  }


  TupleBuffer persBuf;
  TupleBuffer memBuf;
  size_t memTuples;
  size_t skip;
  size_t streamPos;

  GenericRelationIterator* bufIter;
  bool memBufFinished;
  bool trace;

};





static int shuffle2_vm( Word* args,
                        Word& result, int message,
                        Word& local, Supplier s    )
{
  //args[0]: stream(tuple(y))
  //args[1]: int

  static const string pre = "shuffle2: ";

  ShuffleInfo* info = static_cast<ShuffleInfo*>( local.addr );
  Word stream = args[0];
  int numTuples = StdTypes::GetInt( args[1] );

  switch (message)
  {
    case OPEN :
    {
      info = new ShuffleInfo(4 * 1024 * 1024, numTuples);
      qp->Open(stream.addr);
      Tuple* t = nextTuple(stream);
      while( t != 0 )
      {
        info->append(t);
        t = nextTuple(stream);
      }
      local.addr = info;

      return 0;
    }

    case REQUEST :
    {
      Tuple* t = info->getNext();
      if ( t != 0 )
      {
        result.addr = t;
        return YIELD;
      }
      else
      {
        result.addr = 0;
        return CANCEL;
      }
    }

    case CLOSE :
    {
      qp->Close(stream.addr);
      delete info;

      return 0;
    }

    default : { assert(false); }
  }
  return 0;
}


/*
Operator ~shuffle~

*/


struct ShuffleInfoRAND : public ShuffleInfo {

  ShuffleInfoRAND() : ShuffleInfo(),
		      lastTuple(0)
  {}

  ~ShuffleInfoRAND()
  {}


  void append(Tuple* t)
  {
    size_t i = 0;
    bool replaced = false;
    Tuple* v = rtBuf.ReplacedByRandom(t, i, replaced);

    if ( replaced )
    {
      // v was replaced by t
      if (v != 0) {
        persBuf.AppendTuple(v);
        v->DeleteIfAllowed();
      }
    }
    else
    {
      assert(v == 0);
      // v == 0, and t was not stored in buffer
      persBuf.AppendTuple(t);
    }
  }

  inline size_t freeBytes() const {
    return persBuf.FreeBytes();
  }

/*
Copy all tuples of the random buffer into the memory buffer of
the parent class.

*/
  void finish()
  {
    rtBuf.copy2TupleBuf( memBuf );
  }

  Tuple* lastTuple;
  // Will be used to store the last tuple which could not be hold
  // in memory. Refer to shuffle3_vm.

  private:
    RandomTBuf rtBuf;

};

static int shuffle_vm( Word* args,
                        Word& result, int message,
                        Word& local, Supplier s    )
{
  //args[0]: stream(tuple(y))

  static const string pre = "shuffle: ";

  ShuffleInfoRAND* info = static_cast<ShuffleInfoRAND*>( local.addr );
  Word stream = args[0];

  switch (message)
  {
    case OPEN :
    {
      info = new ShuffleInfoRAND();
      qp->Open(stream.addr);
      Tuple* t = nextTuple(stream);
      while( t != 0 )
      {
        info->append(t);
        t = nextTuple(stream);
      }
      info->finish();
      local.addr = info;

      return 0;
    }

    case REQUEST :
    {
      Tuple* t = info->getNext();
      if ( t != 0 )
      {
        result.addr = t;
        return YIELD;
      }
      else
      {
        result.addr = 0;
        return CANCEL;
      }
    }

    case CLOSE :
    {
      qp->Close(stream.addr);
      delete info;

      return 0;
    }

    default : { assert(false); }
  }
  return 0;
}

static int shuffle3_vm( Word* args,
                        Word& result, int message,
                        Word& local, Supplier s    )
{
  //args[0]: stream(tuple(y))

  static const string pre = "shuffle: ";

  ShuffleInfoRAND* info = static_cast<ShuffleInfoRAND*>( local.addr );
  Word stream = args[0];

  switch (message)
  {
    case OPEN :
    {
      info = new ShuffleInfoRAND();
      qp->Open(stream.addr);
      Tuple* t = nextTuple(stream);
      size_t ctr = 0;
      while( t != 0 )
      {
	// check if the tuple fits into the buffer
	if ( (size_t)t->GetExtSize() < info->freeBytes() ) {
          info->append(t);
          t = nextTuple(stream);
	  ctr++;
	} else {
	  // store last tuple for the next REQUEST message
	  info->lastTuple = t;
	  break;
        }
      }
      cout << "ctr = " << ctr << endl;
      info->finish();
      local.addr = info;

      return 0;
    }

    case REQUEST :
    {
      Tuple* t = info->getNext();
      if ( t != 0 )
      {
	// iterate over the memory buffer
        result.addr = t;
        return YIELD;
      }
      else
      {
	// continue iterating the input stream
	t = info->lastTuple;
        if (t != 0) {
          info->lastTuple = 0;
	} else {
	  t = nextTuple(stream);
	}

        if ( t != 0)
	{
          result.addr = t;
          return YIELD;
        }
	else
	{
          result.addr = 0;
          return CANCEL;
        }
      }
    }

    case CLOSE :
    {
      qp->Close(stream.addr);
      delete info;

      return 0;
    }

    default : { assert(false); }
  }
  return 0;
}


static ListExpr memshuffle_tm(ListExpr args)
{
  return shuffleX_tm(args, "memshuffle");
}

static int memshuffle_vm( Word* args,
                       Word& result, int message,
                       Word& local, Supplier s    )
{
  //args[0]: Input stream(ptuple(y))  or stream(tuple(y))

  static const string pre = "memshuffle: ";

  ShuffleBuf* info = static_cast<ShuffleBuf*>( local.addr );

  Word stream = args[0];

  switch (message)
  {
    case OPEN :
    {
      const int M = 40 * 4096 * 4096;
      const int Lps = 32 * 1024;

      info = new ShuffleBuf(M, Lps);
      qp->Open(stream.addr);
      Tuple* t = nextTuple(stream);
      while( t != 0 )
      {
        info->append(t);
        t = nextTuple(stream);
      }
      info->shuffle();
      info->open();
      local.addr = info;

      return 0;
    }

    case REQUEST :
    {
      Tuple* t = info->getNext();
      if ( t != 0 )
      {
        result.addr = t;
        return YIELD;
      }
      else
      {
        result.addr = 0;
        return CANCEL;
      }
    }

    case CLOSE :
    {
      qp->Close(stream.addr);
      delete info;

      return 0;
    }

    default : { assert(false); }
  }
  return 0;
}

static ListExpr memshuffle2_tm(ListExpr args)
{
  return shuffleX_tm(args, "memshuffle2");
}

static int memshuffle2_vm( Word* args,
                       Word& result, int message,
                       Word& local, Supplier s    )
{
  //args[0]: Input stream(ptuple(y))  or stream(tuple(y))

  static const string pre = "memshuffle2: ";
  ShuffleBuf2* info = static_cast<ShuffleBuf2*>( local.addr );

  Word stream = args[0];

  switch (message)
  {
    case OPEN :
    {
      info = new ShuffleBuf2();
      qp->Open(stream.addr);
      Tuple* t = nextTuple(stream);
      while( t != 0 )
      {
        info->append(t);
        t = nextTuple(stream);
      }
      local.addr = info;
      return 0;
    }

    case REQUEST :
    {
      Tuple* t = info->getNext();
      if ( t != 0 )
      {
        result.addr = t;
        return YIELD;
      }
      else
      {
        result.addr = 0;
        return CANCEL;
      }
    }

    case CLOSE :
    {
      qp->Close(stream.addr);
      delete info;

      return 0;
    }

    default : { assert(false); }
  }
  return 0;
}

static ListExpr runtime_tm(ListExpr list)
{
  NList args(list);

  if (!args.hasLength(7))
    return NList::typeError("Expecting 7 arguments");

  if ( !args.elem(1).isSymbol(CcString::BasicType()) )
    return NList::typeError("Expecting a string as first argument");
  if ( !args.elem(2).isSymbol(CcInt::BasicType()) )
    return NList::typeError("Expecting an int as second argument");
  if ( !args.elem(3).isSymbol(CcInt::BasicType()) )
    return NList::typeError("Expecting an int as third argument");
  if ( !args.elem(4).isSymbol(CcReal::BasicType()) )
    return NList::typeError("Expecting a real as forth argument");
  if ( !args.elem(5).isSymbol(CcReal::BasicType()) )
    return NList::typeError("Expecting a real as fifth argument");
  if ( !args.elem(6).isSymbol(CcReal::BasicType()) )
    return NList::typeError("Expecting a real as sixth argument");
  if ( !args.elem(7).isSymbol(CcInt::BasicType()) )
   return NList::typeError("Expecting an int as seventh argument");

  return NList(CcInt::BasicType()).listExpr();
}

static int runtime_vm( Word* args, Word& result, int message,
                       Word& local, Supplier s)
{
  // args[0]: Input stream(ptuple(y))  or stream(tuple(y))
  result = qp->ResultStorage(s);
  Word w;

  qp->Request(args[0].addr, w);
  string name = StdTypes::GetString(w);
  qp->Request(args[1].addr, w);
  int c1 = StdTypes::GetInt(w);
  qp->Request(args[2].addr, w);
  int c2 = StdTypes::GetInt(w);
  qp->Request(args[3].addr, w);
  int s1 = (int)floor( StdTypes::GetReal(w) + 0.5 );
  qp->Request(args[4].addr, w);
  int s2 = (int)floor( StdTypes::GetReal(w) + 0.5 );
  qp->Request(args[5].addr, w);
  SEC_STD_REAL sel = StdTypes::GetReal(w);

  SHOW(name)
  SHOW(c1)
  SHOW(s1)
  CostParams p(c1, s1, c2, s2, sel);
  cout << p << endl;
  StopWatch t;

  // evaluate the last argument
  t.start();
  qp->Request(args[6].addr, w);
  double rt = t.diffSecondsReal();
  cout << rt << endl;

  CostFunctions* costFuns = new CostFunctions();
  costFuns->append( name, 1 );
  const CostInfo ci = costFuns->findBest(p);
  SHOW(ci)

  // set result value
  CcInt* funRes = static_cast<CcInt*>( w.addr);
  CcInt* res = static_cast<CcInt*>( result.addr );
  res->CopyFrom(funRes);

  // store values in system table
  static int id = 0;
  id++;

  CostTuple* cost = new CostTuple(p, ci.costs);
  cost->cost_name = name;
  cost->id = id;
  cost->param_res_card = res->GetIntval();
  cost->real_runtime = rt;
  cost->real_write = Counter::getRef( Symbol::CTR_TBUF_PAGES_W() );
  cost->real_read = Counter::getRef( Symbol::CTR_TBUF_PAGES_R() );
  long& intHash = Counter::getRef(Symbol::CTR_INT_HASH());
  long& intLess  = Counter::getRef(Symbol::CTR_INT_LESS());
  long& intEqual = Counter::getRef(Symbol::CTR_INT_EQUAL());
  long& intCompare = Counter::getRef(Symbol::CTR_INT_COMPARE());
  cost->real_cpu = intHash + intLess + intEqual + intCompare;

  costRel->append(cost, false);
  return 0;
}

}; // end of PartStreamMappings

class SIG {

  public:
  SIG( const string& p1,
       const string& p2,
       const string& p3 = "",
       const string& p4 = "",
       const string& p5 = "",
       const string& p6 = "",
       const string& p7 = "",
       const string& p8 = "") {

    map.push_back(p1);
    map.push_back(p2);
    if (p3 != "")
      map.push_back(p3);
    if (p4 != "")
      map.push_back(p4);
    if (p5 != "")
      map.push_back(p5);
    if (p6 != "")
      map.push_back(p6);
    if (p7 != "")
      map.push_back(p7);
    if (p8 != "")
      map.push_back(p8);
  }

  string str() {

    string res="";
    size_t args = map.size();
    assert(args >= 2);

    res=map[0];
    for(size_t i = 1; i <= args-2; i++) {
      res += " x " + map[i];
    }
    res += " -> " + map[args-1];
    return res;
  }

  private:
  vector<string> map;

};

struct runtimeInfo : OperatorInfo {

  runtimeInfo()
  {
    name      = "runtime";
    signature = SIG(CcString::BasicType(), CcInt::BasicType(),
                    CcInt::BasicType(),
                    CcReal::BasicType(),CcReal::BasicType(),CcReal::BasicType(),
                    "(map ... -> int)", CcInt::BasicType()).str();
    syntax    = "runtime(s,t1,t2,s1,s2,sel,f)";
    meaning   = "Determines the runtime for a join given by function f and "
                "applies the cost function named by s with the parameters: "
		"t1,t2,s1,s2 (input cardinalities and avg. tuple sizes), "
		"sel (join selectivity)";
  }
};


struct sortmergejoinrInfo : OperatorInfo {

  sortmergejoinrInfo(const string& _name)
  {
    name      = _name;

    signature = "stream(tuple(...)) x stream(tuple(...)) -> stream(tuple(...))";
    syntax    = "_ _ sortmergejoin_r[an, bm]";
    meaning   = "Computes a join by sorting the inputs but returns the result "
                "R = S1 u S2 with a random subset S1 and a sorted subset S2. "
                "The variant _r does this by generating all output tuples "
                "two times, the variant _r2 does this in a more sophisticated "
                "way with less and variant _r3 outputs only the random "
                "sample S1";

    supportsProgress = true;
  }
};


extern int
sortmergejoinr_vm( Word* args, Word& result,
                   int message, Word& local, Supplier s );

extern int
sortmergejoinr2_vm( Word* args, Word& result,
                    int message, Word& local, Supplier s );

extern int
sortmergejoinr3_vm( Word* args, Word& result,
                    int message, Word& local, Supplier s );


template<bool, int> extern ListExpr
JoinTypeMap(ListExpr args);


// Defining static member sym
Symbols PartStreamMappings::sym;


/*
4 Partitioned Stream Algebra

*/

class PartStreamAlgebra : public Algebra
{
  typedef PartStreamMappings psm;

  public:
    PartStreamAlgebra() : Algebra()
    {

      //assert(false);

      ConstructorInfo ci;
      ci.name      =    PTuple::BasicType();
      ci.signature =    "(ident x DATA)+ -> PTUPLE";
      ci.typeExample =  PTuple::BasicType();
      ci.listRep =      "Not supported! ";
      ci.valueExample = " - ";
      ci.remarks =      "Objects are only used in streams!";

      ConstructorFunctions<PTuple> ptf;
      TypeConstructor* ptuple = new TypeConstructor( ci, ptf );

      ptuple->AssociateKind( Kind::PTUPLE() );
      AddTypeConstructor( ptuple,true );

/*
4 Operators

*/

      Operator* op = 0;
      OperatorInfo oi;

        oi.name =      "pfeed";
        oi.signature = "rel(tuple(y) -> stream(ptuple(y))";
        oi.syntax =    "_ pfeed[int k]";
        oi.meaning =   "Creates a stream containing marker tuples. "
                       "Every max(|k|,2) tuples a new marker tuple will "
                       "be inserted.";

      op = new Operator(
        oi,
        psm::pfeed_vm,
        psm::pfeed_tm
      );

      AddOperator( op,true );

        oi.name =      "pdelete";
        oi.signature = "stream(ptuple(y) -> stream(tuple(y))";
        oi.syntax =    "_ pdelete";
        oi.meaning =   "Removes marker tuples from a stream";

      op = new Operator(
        oi,
        psm::pdelete_vm,
        psm::pdelete_tm
      );

      AddOperator( op,true );

        oi.name =      "PSTREAM1";
        oi.signature = "((stream(ptuple(y)) ...) -> stream(tuple(y))";
        oi.syntax =    "Not available";
        oi.meaning =   "Type mapping operator";

      op = new Operator(
        oi,
        0,
        psm::PSTREAM1_tm
      );

      AddOperator( op,true );

        oi.name =      "PSTREAM2";
        oi.signature = "((...) (stream(ptuple(y)) ...) -> stream(tuple(y))";
        oi.syntax =    "Not available";
        oi.meaning =   "Type mapping operator";

      op = new Operator(
        oi,
        0,
        psm::PSTREAM2_tm
      );

      AddOperator( op,true );

        oi.name =      "puse";
        oi.signature = "stream(ptuple(y) x ( stream(tuple(y)) "
                       "->  stream(tuple(y)) ) -> stream(tuple(y))";
        oi.syntax =    "_ puse[ _ ]";
        oi.meaning =   "Hides the marker tuples for the parameter function "
                       "and inserts them again into the result stream";

      op = new Operator(
        oi,
        psm::puse_vm,
        psm::puse_tm
      );

      AddOperator( op,true );

        oi.name =      "pjoin1";
        oi.signature = "( stream(ptuple(y1)) rel(tuple(y2)) "
                       "( ( map (stream(tuple(y1))) (rel(tuple(y2))) "
                       "(stream(tuple(z))) ) ... N repeats ... ))"
                       "-> (stream(ptuple(z))).";
        oi.syntax =    "_ _ pjoin1[ f1: expr1, f2: expr2, ... ]";
        oi.meaning =   "Implements the adaptive join for two relations "
                       "given as streams. Function f1 will be used for the"
                       "probe join. Each function must have one of the names"
                       "symj (symmjoin), smj (sortmergejoin), hj (hashjoin), "
                       "ilj (index-loop-join).";
        oi.meaning =   "Implements the adaptive join for a tuple stream "
                       "and a base relation.";

      op = new Operator(
        oi,
        psm::pjoin1_vm,
        psm::pjoin1_tm
      );

      AddOperator( op,true );

        oi.name =      "pjoin2";
        oi.signature = "( stream(ptuple(y1)) stream(ptuple(y2)) "
                       "( ( map (stream(tuple(y1))) (stream(tuple(y2))) "
                       "(stream(tuple(z))) ) ... N repeats ... ))"
                       "-> (stream(ptuple(z))).";
        oi.syntax =    "_ _ pjoin2[ f1: expr1, f2: expr2, ... ]";
        oi.meaning =   "Implements the adaptive join for two relations "
                       "given as streams. Function f1 will be used for the"
                       "probe join. Each function must have one of the names"
                       "symj (symmjoin), smj (sortmergejoin), hj (hashjoin).";

      op = new Operator(
        oi,
        psm::pjoin2_vm,
        psm::pjoin2_tm
      );


      AddOperator( op,true );

        oi.name =      "pcreate";
        oi.signature = "stream(tuple(y)) x int -> stream(ptuple(y))";
        oi.syntax =    "_ pcreate[ _ ]";
        oi.meaning =   "Consumes a stream and creates a partitioned "
                       "stream like pfeed";

      op = new Operator(
        oi,
        psm::pcreate_vm,
        psm::pcreate_tm
      );

      AddOperator( op,true );


        oi.name =      "pcreate2";
        oi.signature = "stream(tuple(y)) x int x int -> stream(ptuple(y))";
        oi.syntax =    "_ pcreate2[ _, _ ]";
        oi.meaning =   "Creates a partitioned stream but gets the inputs "
                       "stream size as third parameter. This is useful if "
                       "the input stream is returned "
                       "by an index structure and the size is known.";

      op = new Operator(
        oi,
        psm::pcreate2_vm,
        psm::pcreate2_tm
      );

      AddOperator( op,true );

        oi.name =      "shuffle";
        oi.signature = "stream(tuple(y)) -> stream(tuple(y))";
        oi.syntax =    "_ shuffle";
        oi.meaning =   "Randomizes its input: Materializes a stream "
                       "returns the first 500 tuples in random order.";

      op = new Operator(
        oi,
        psm::shuffle_vm,
        psm::shuffle_tm
      );

      AddOperator( op,true );

        oi.name =      "shuffle2";
        oi.signature = "stream(tuple(y)) x int -> stream(tuple(y))";
        oi.syntax =    "_ shuffle2[ n ]";
        oi.meaning =   "Randomizes its input: Every n/500-th tuple is passed "
		       "directly to the output (non-blocking). The other "
		       "tuples are buffered or materialized if necessary and "
		       "returned afterwards.";

      op = new Operator(
        oi,
        psm::shuffle2_vm,
        psm::shuffle2_tm
      );

      AddOperator( op,true );


        oi.name =      "shuffle3";
        oi.signature = "stream(tuple(y)) -> stream(tuple(y))";
        oi.syntax =    "_ shuffle3";
        oi.meaning =   "Randomizes its input: Tuples up to the maximum allowed "
		       "memory per operator are read into a buffer. Out of "
		       "this a random sample of 500 tuples is drawn and "
		       "returned first. Afterwards all remaining tuples of "
		       "the buffer and stream are returned.";

      op = new Operator(
        oi,
        psm::shuffle3_vm,
        psm::shuffle_tm
      );

      AddOperator( op,true );


        oi.name =      "memshuffle";
        oi.signature = "stream(ptuple(y)) -> stream(ptuple(y)), "
                       "stream(tuple(y)) -> stream(tuple(y))";
        oi.syntax =    "_ memshuffle";
        oi.meaning =   "Overloaded operator which shuffles tuples in a "
                       "memory buffer and outputs them in random order. "
                       "For large input streams the randomness might be "
                       "insufficient.";

      op = new Operator(
        oi,
        psm::memshuffle_vm,
        psm::memshuffle_tm
      );

      AddOperator( op,true );

      oi.name =      "memshuffle2";
      oi.signature = "stream(ptuple(y)) -> stream(ptuple(y)), "
                     "stream(tuple(y)) -> stream(tuple(y))";
      oi.syntax =    "_ memshuffle2";
      oi.meaning =   "Shuffles a stream of tuples in a "
                     "memory buffer and outputs them in random order. ";

    op = new Operator(
      oi,
      psm::memshuffle2_vm,
      psm::memshuffle2_tm
    );

    AddOperator( op,true );

        oi.name =      "pshow";
        oi.signature = "stream(ptuple(y)) -> stream(ptuple(y))";
        oi.syntax =    "_ pshow";
        oi.meaning =   "Display the marker tuples' information.";

      op = new Operator(
        oi,
        psm::pshow_vm,
        psm::pshow_tm
      );

      AddOperator( op,true );

      op = new Operator( runtimeInfo(), psm::runtime_vm, psm::runtime_tm );
      AddOperator( op,true );
      op->SetRequestsArguments();


      AddOperator( sortmergejoinrInfo("sortmergejoin_r"),
                   sortmergejoinr_vm,
                   JoinTypeMap<false, 1> );

      AddOperator( sortmergejoinrInfo("sortmergejoin_r2"),
                   sortmergejoinr2_vm,
                   JoinTypeMap<false, 1> );

      AddOperator( sortmergejoinrInfo("sortmergejoin_r3"),
                   sortmergejoinr3_vm,
                   JoinTypeMap<false, 1> );

    }

    // We don't care about the deletion of Algebra and TypeConstructor
    // instances since they will be destroyed when the process terminates.
    ~PartStreamAlgebra() {};

};


/*
5 Initialization

*/

extern "C"
Algebra*
InitializePartitionedStreamAlgebra(NestedList *nlRef, QueryProcessor *qpRef)
{
  nl = nlRef;
  qp = qpRef;

  pjoinRel = new PJoinRel("SEC2PJOIN");
  costRel = new CostRel("SEC2PJOINCOST");
  SystemTables& st = SystemTables::getInstance();
  st.insert(pjoinRel);
  st.insert(costRel);

  return (new PartStreamAlgebra());
}


