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

//paragraph [1] title: [{\Large \bf ]   [}]



[1] Partitioned Stream Algebra

January 2006 M. Spiekermann 

This algebra implements a type constructor ~ptuple~ which represents ~normal~ tuples
of the relational algebra or ~marker~ tuples which contain information about a bunch
of tuples. Since tuples are normally processed in a stream the markers defines partitions
of the stream.


Operations like ~puse~ and ~pjoin~ are implemented to support adaptive techniques
for query processing.


1 Preliminaries

1.1 Includes and global declarations

*/


#include <iostream>
#include <queue>


#include "Algebra.h"
#include "NList.h"
#include "QueryProcessor.h"
#include "SecondoSystem.h"
#include "Attribute.h"
#include "FunVector.h"
#include "CostFunction.h"

#include "SystemInfoRel.h"


/*
Dependencies with other algebras: RelationAlgebra, StandardAlgebra
   
*/
#include "RelationAlgebra.h" 
#include "StandardTypes.h"

#include "LogMsg.h"

extern NestedList* nl;
extern QueryProcessor *qp;


//extern InObject InRel;

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
};


/* 
Below we define a class which helps to maintain stream in value
mappings.

*/

struct QPStream
{
 
  typedef enum {opened, finished, closed} StreamState;

  public:
  QPStream() : state(closed), stream(0) {} 
    
  QPStream( Word is ) : state(closed), stream(is.addr) {} 
  
  QPStream( Supplier s ) : state(closed), stream(s) {} 
   
  QPStream( QPStream& rhs ) : state(rhs.state), stream(rhs.stream) {} 
    
  inline void open() 
  {
    if (state == closed) {
      qp->Open(stream);
      state = opened;
    }   
  }
    
  inline void close() 
  {
    if (state != closed) {
      qp->Close(stream);
      state = closed;
    }  
  }
  
  inline void* getStream() { return stream; }
  
  inline void* getNext() 
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
  
  private: 
   StreamState state;
   void* stream;
   
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
    Word wTuple; 
    qp->Request(w.addr, wTuple);
    if( qp->Received(w.addr) )
      return static_cast<T*>( wTuple.addr );
    else
      return 0;
  }
 
  template<class T>
  inline static T* nextOfStream2(const Supplier s)
  {
    Word wTuple; 
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

  
  inline static int getInt(const Word& w) 
  {
    static Word wArgVal;
    qp->Request(w.addr, wArgVal);
    return static_cast<CcInt*>( wArgVal.addr )->GetIntval();
  } 
  
  inline static GenericRelation* getRelation(const Word& w) 
  {
    static Word wArgVal;
    qp->Request(w.addr, wArgVal);
    return static_cast<GenericRelation*>( wArgVal.addr );
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

    if ( !l.first().isSymbol( sym.map ) )
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
    return checkDepth3(l, sym.rel, sym.tuple, attrs);
  }
  
  static bool checkStreamTuple(const NList& l, NList& attrs)
  {
    return checkDepth3(l, sym.stream, sym.tuple, attrs);
  }

  static bool checkStreamPTuple(const NList& l, NList& attrs)
  {
    return checkDepth3(l, sym.stream, sym.ptuple, attrs);
  }
  
  static NList makeStreamTuple(const NList& attrs)
  {	  
    NList tup( NList(sym.tuple), attrs );
    return NList( NList(sym.stream), tup );
  }
  
  static NList makeStreamPTuple(const NList& attrs)
  {	  
    NList tup( NList(sym.ptuple), attrs );
    return NList( NList(sym.stream), tup );
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

  string e1 = expects(sym.rel,sym.tuple);
  
  static const string err1 = "pfeed expects (" + e1 + " " + sym.integer + ")!";
  
  if ( !l.hasLength(2) )
    return l.typeError(err1);
 
  NList attrs;
  if ( !checkRelTuple( l.first(), attrs ) )
    return l.typeError("First list element should be " + e1 + ".");
  
  if ( !l.second().isSymbol( sym.integer ) )
    return l.typeError( "Second list element should be symbol '" 
                        + sym.integer + "'." );
 
  return makeStreamPTuple(attrs).listExpr();
}

  struct PartStreamInfo 
  { 
    GenericRelationIterator* rit;
    int ctr;
    int tuples;
    int num;
    int parts;

    PartStreamInfo() : rit(0), ctr(0), tuples(0), num(0), parts(0) {} 
    PartStreamInfo(GenericRelationIterator* r, const int t, const int p) : 
      rit(r), 
      ctr(0), 
      tuples(t), 
      num(0), 
      parts(p) 
    { 
     assert(tuples > 0);
    }
    ~PartStreamInfo() { delete rit; }
      

/* 
The next function gets a tuple of the input stream and returns
the tuple or a marker. At the first call a marker will be produced
This ensures that operators stream-downwards will always know the
requested partsize. Afterwards when the amount of ~tuples~ requests are made
a marker will be injected.

*/
    inline PTuple* nextPTuple()
    {
       if ( resetIfNeeded() )
       { 
         return new PTuple( newMarker() ); 
       }
       else
       {
         Tuple* t = rit->GetNextTuple(); 
         if (t)
           return new PTuple(t);
         else
           return 0;
       }	 
    }	    


/*
After construction we have ctr == 0, hence the first call
will return true.
   
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



static int
pfeed_vm(Word* args, Word& result, int message, 
         Word& local, Supplier s)
{
  // args[0]: Input Relation
  // args[1]: An integer defining the partition size
 
  static const string pre = "pfeed: ";
  GenericRelation* r = 0;
  Word argRelation = SetWord(0);
 

  PartStreamInfo* info = static_cast<PartStreamInfo*>( local.addr );
  
  switch (message)
  {
    case OPEN :
    { 
      r = getRelation( args[0] );
      int partSize = max( abs(getInt(args[1])), 2 );
      int parts = r->GetNoTuples() / partSize;

      //SHOW(partSize)
      //SHOW(parts) 
      
      info = new PartStreamInfo( r->MakeScan(), partSize, parts );
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
  
  string e1 = expects( sym.stream, sym.ptuple );

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
             delete pt;
             result.addr = pt->tuple;
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
  string e1 = expects( sym.stream, sym.ptuple );

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
  
  string e1 = expects( sym.stream, sym.ptuple );

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
  
  static const string e1 = expects( sym.stream, sym.ptuple );

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
   
   queue<const Marker*> q;
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

      // save caller node in argument vector
      funargs = qp->Argument(fun.addr);

      // store the supplier of this operator as and argument
      (*funargs)[MAXARG-1] = SetWord(s);
  
      // initalize local information
      local.addr = new MarkerQueue();
      
      // The parameter function's return type is a stream
      // hence we need to open it
      qp->Open(fun.addr);
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

      //TRACE(pre << "Message FUNMSG+CLOSE received")
      return 0;
    }
    case CLOSE: {

      //TRACE(pre << "Message CLOSE received")
      // close the input stream 
      qp->Close(inStream.addr);
      
      // send a close message to the parameter fun in order that it can be
      // propageted to its childs.
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
  
  //cout << "pjoin2_tm --------------------->" << endl;
  //cout << l << endl;
  //cout << "<---------------------" << endl;
  
  static const string e1 = expects( sym.stream, sym.ptuple, "y1" );
  static const string e2 = expects( sym.stream, sym.ptuple, "y2" );

  static string err1 = "Expecting input (" 
                       + e1 + " " + e2
                       + " (list of join-expressions) ";
                       //+ " (list of cost-functions) )!";
  
  if ( !checkLength( l, 3, err1 ) )
    return l.typeError( err1 );
  
  NList attrs1;
  if ( !checkStreamPTuple( l.first(), attrs1) )
    return l.typeError( argNotCorrect(1) + err1);
  
  NList attrs2;
  if ( !checkStreamPTuple( l.second(), attrs2) )
    return l.typeError( argNotCorrect(2) + err1);

  
  // Test the parameter function's signature 
  static const string 
  err2 = "Expecting as third argument a list of functions of type "
         "(map (stream(tuple(y1))) (stream(tuple(y1))) (stream(tuple(z))))";
  
  NList joinMaps = l.third();
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

  /*
  // check cost functions  
  NList costMaps = l.fourth();
  //NList lastResultAttrs;
  NList cfNames;

  if (! costMaps.length() == (joinMaps.length() + 1) )
   return l.typeError( "Number of cost functions not correct! " + err1 );
  */
    
  NList appendSym("APPEND");
  NList resultType(appendSym, fNames, makeStreamPTuple(lastResultAttrs));
  return resultType.listExpr();
}

/*
Class ~PTupleBuffer~ captures the first part of a stream of tuples
into a tuple buffer and allows to us it again.

*/

struct PTupleBuffer
{

  private: 
    const string pre;
    TupleBuffer* buf;
    GenericRelationIterator* rit;
    void* is;
    bool scanBuffer;
    bool bufferOnly;
    bool endReached;
    size_t maxSize;
    Marker lastMarker;
    int tuplesCurrentPart;
    int tuplesCompleteParts;
    int requestedTuples;
    
  public:
    PTupleBuffer(const string& id, Word stream, const int size ) :
     pre(id+": "),
     rit(0),
     is(stream.addr),
     scanBuffer(false),
     bufferOnly(true),
     endReached(false),
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

    int getNoTuples() { 
      return buf->GetNoTuples(); 
    }
    
    inline int getNoTuplesOfCompleteParts() { 
      return tuplesCompleteParts; 
    }
    
    inline int getRequestedTuples() {
       return tuplesCurrentPart + tuplesCompleteParts;
    }   

    inline bool end() const { 
      return endReached;
    }
    
/*
The ~getNext~ function will remove marker tuples and stores the tuples in
th ~TupleBuffer~ until a memory overflow is reached. 
   
*/   
    inline bool storeNextTuple() {
    
        // get next ptuple from input 
        PTuple* pt = nextPTuple( is );
        if (pt) 
        {
          if (pt->tuple) // return tuple to parameter function
          { 
            //TRACE(pre << "Tuple received!")
            buf->AppendTuple(pt->tuple);
            tuplesCurrentPart++;
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
            return true;
          }  
        }
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
            PTuple* pt = nextPTuple( is );
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
    
    int estimateInputCard() 
    {
      SHOW(lastMarker.num)
     
      // expected tuples
      int expected = max(lastMarker.num,1) * lastMarker.tuples;
      SHOW(expected)
      
      // received tuples
      int received = getTuplesOfCompleteParts(); 
      SHOW(received) 
      
      // cardinality. If not a complete partition of tuples 
      // was read in then the formula below may underestimate
      // the input card. 
      int card = (lastMarker.tuples * lastMarker.parts * received) / expected;
     
      return card;
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

    inline double getTotalSize() { 
      return buf->GetTotalSize(); 
    }

}; 


static int pjoin2_vm( Word* args, Word& result, int message, 
                      Word& local, Supplier s)
{
  // args[0]: Input stream(ptuple(y1))) 
  // args[1]: Input stream(ptuple(y2))) 
  // args[2]: A list of map stream(tuple(y1)))x stream(tuple(y2)) 
  //          -> stream(tuple(z))
  // args[3]: A list of symbols for evaluation function names

  static const string pre("pjoin2: ");

  typedef enum { probe, eval } JoinState;
 

  typedef PartStreamMappings psm;
  
  struct JoinInfo :  public PartStreamInfo {
   
    // used to maintain the evaluation functions 
    FunVector* evalFuns;

    // used to maintain the cost functions 
    CostFunctions* costFuns; 

    MarkerQueue* mLeft;
    MarkerQueue* mRight;

    QPStream leftIs;
    QPStream rightIs;
    
    PTupleBuffer* leftBuf;
    PTupleBuffer* rightBuf;
    
    JoinState state;

    int maxMem;
    
    // index between [0,n-1] pointing to the best evaluation function
    int bestPos;
    QPStream bestFun;
    
    // estimated cardinalites of input and output
    int leftCard;
    int rightCard;
    int resultCard;
    int leftAvgTupSize;
    int rightAvgTupSize;
    int resultTuples;
    
    float joinSel;
    
    long probeJoinCPU;
    
    bool bufReset[2];
    
    JoinInfo(Word left, Word right, const int mem = 1024*1024) : 
      PartStreamInfo(),
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
      probeJoinCPU(0)  
    {
      TRACE_FILE("pjoin2.traces")

      leftBuf = new PTupleBuffer( "left", left, maxMem );
      rightBuf = new PTupleBuffer( "right", right, maxMem );
      evalFuns = new FunVector();

      bufReset[0] = false;
      bufReset[1] = false;
    } 
    ~JoinInfo()
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
 
      Counter::getRef("PSA::pjoin2:Card_result") = resultTuples;
      Counter::getRef("PSA::pjoin2:Card_result_err") = resultErr; 
       
      Counter::getRef("PSA::pjoin2:Card_left") = leftTuples; 
      Counter::getRef("PSA::pjoin2:Card_left_err") = leftErr;
      
      Counter::getRef("PSA::pjoin2:Card_right") = rightTuples;
      Counter::getRef("PSA::pjoin2:Card_right_err") = rightErr;
     
      Counter::getRef("PSA:Usedjoin:CPU_Ops") = cpuOps() - probeJoinCPU; 
      
      delete leftBuf;
      delete rightBuf;
      delete evalFuns;
      delete costFuns;
    } 

     
    const int relErr(const int a, const int b) {
      return static_cast<int>( ceil(((abs(a - b) * 1.0) / b) * 100) );
    } 
    
    
/*
Function ~computeCards~ will compute estimates for the
input, output and result stream
   
*/     
    void computeCards(const int resultTuples)
    {
       leftCard = leftBuf->estimateInputCard();
       rightCard = rightBuf->estimateInputCard();
       //SHOW(leftCard)
       //SHOW(rightCard)

       int leftRead = leftBuf->getNoTuples();
       int rightRead = rightBuf->getNoTuples();
      
       //avg tuple size
       leftAvgTupSize = static_cast<int>( leftBuf->getTotalSize() / leftRead );
       rightAvgTupSize = 
          static_cast<int>( rightBuf->getTotalSize() / rightRead );
       
       // join selectivity
       joinSel = (1.0 * max(resultTuples,1)) / (leftRead * rightRead);
       SHOW(joinSel)
       
       // cardinality of the join result
       const float leftCardF = leftCard;
       const float rightCardF = rightCard;
       const float resultCardF = ceil(joinSel * leftCardF * rightCardF);
       resultCard = static_cast<int>( resultCardF );
       SHOW(resultCard)

       Counter::getRef("PSA::pjoin2:Card_left_est") = leftCard;
       Counter::getRef("PSA::pjoin2:Card_right_est") = rightCard;
       Counter::getRef("PSA::pjoin2:Card_result_est") = resultCard;
    } 

/*
The probe join will be stopped if 

   * a memory overflow happens

   * more than ~n~ tuples (only complete parts) for each input are read

*/
  
    inline bool stopLoading() 
    { 
      const double bufferSize = leftBuf->getTotalSize() 
                                + rightBuf->getTotalSize();
      const bool overFlow = bufferSize > 2*maxMem;
      const int n = 500;
      
      bool stop = false;
      if (overFlow) 
      {
        TRACE("*** OVERFLOW! ***")
        stop = true;
      }
      else
      {
        int storedTuples = leftBuf->getNoTuplesOfCompleteParts()
                             + rightBuf->getNoTuplesOfCompleteParts();
        stop = (storedTuples > 2*n);
        stop = ( stop || (leftBuf->end() && rightBuf->end()) );
      }

      if (stop)
      { 
        TRACE("*** Enough tuples read ***")
        SHOW(bufferSize) 
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

    void loadTupleBuffers() {

      TRACE("*** load TupleBuffers ***")
      while ( !stopLoading() ) 
      {
        leftBuf->storeNextTuple();
        rightBuf->storeNextTuple(); 
      } 
    } 

    bool resetBuffer(const int no) {

      assert( (no == 1) || (no == 2) );
      
      if ( (no == 1) && !bufReset[1] ) {
        leftBuf->reset(false);
        bufReset[1] = true;
        return true;
      }  
      
      if ( (no == 2) && !bufReset[2] ) {
        rightBuf->reset(false);
        bufReset[2] = true;
        return true;
      }
      return false;  
       
    }
     
    void runProbeJoin() 
    { 
     
      Supplier first = (evalFuns->get(0)).getSupplier();
      qp->Open(first);
      
      loadTupleBuffers();

      Tuple* t = psm::nextTuple(first);
      int probeReceived = 0;
      while( t ) 
      {
        probeReceived++;
        t = psm::nextTuple(first);
      } 
      SHOW(probeReceived);
      qp->Close(first);
      TRACE( "\n*** Probe join finished! ***\n" )

      probeJoinCPU = Counter::getRef("PSA:Probejoin:CPU_Ops") = cpuOps(); 
        
      computeCards(probeReceived);
      computeBestFunction();
      bestFun.open();
      
    }

    
    inline void nextPTuple(Word& result)
    {
      //SHOW(bestFun.getStream())
      if ( resetIfNeeded() )
      {
        result.addr = new PTuple( newMarker() );
      } 
      else
      {
        Tuple* t =  psm::nextTuple(bestFun.getStream());
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
      tuples = max( leftBuf->partSize(), rightBuf->partSize() );
      parts = max( resultCard / tuples, 1 ); 

      SHOW(tuples)
      SHOW(parts) 
     
      CostParams cp( leftCard, leftAvgTupSize, 
                     rightCard, rightAvgTupSize, joinSel );
      SHOW(cp)
      costFuns = new CostFunctions();
       
      for (size_t i=1; i<evalFuns->size(); i++) 
      {
        bool ok = costFuns->append( (evalFuns->get(i)).getName(), i );
        assert(ok);
      } 
      
      const CostInfo ci = costFuns->findBest(cp, true);
      SHOW(ci)
      bestPos = ci.cf->index;
      const string& functionName = ci.cf->name;
      
      assert( bestPos < (int)evalFuns->size() );
      FunInfo& f = evalFuns->get(bestPos);
      bestFun = QPStream(f.getSupplier());
      cout << "Using " << functionName << endl;

      TRACE( "\n*** END Computing best function ***\n" )
    } 
    
  };
 
   
  JoinInfo* pj = static_cast<JoinInfo*>( local.addr );

  Word leftStream = args[0];
  Word rightStream = args[1];

  //SHOW( result.addr );

  switch ( message )
  {

    case OPEN: {

      TRACE(pre << "Open received")

      // initialze local storage
      pj = new JoinInfo(leftStream, rightStream); 
      local.addr = pj;
    
      FunVector& evalFuns = *(pj->evalFuns); 
      // load functions into funvector 
      evalFuns.load(args[2], &args[3], true);

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




static ListExpr pcreate_tm(ListExpr args)
{
  NList l(args);
  NList attrs;
  
  static const string& e1 = expects( sym.stream, sym.tuple );
  string err1 = "pcreate expects (" + e1 + "int)!";

    if ( !checkLength( l, 2, err1 ) )
      return l.typeError( err1 );

    if ( !checkStreamTuple( l.first(), attrs) )
      return l.typeError( argNotCorrect(1) + err1);

    if ( !l.second().isSymbol(sym.integer) )
      return l.typeError( argNotCorrect(2) + err1);
  
  return makeStreamPTuple(attrs).listExpr();
}

static int pcreate_vm( Word* args, Word& result, int message, 
                    Word& local, Supplier s)
{
  // args[0]: Input stream(ptuple(y)))  
  // args[1]: int  
  return 0;
}   

static ListExpr pcreate2_tm(ListExpr args)
{
  NList l(args);
  
  static const string& e1 = expects( sym.stream, sym.tuple );
  static string err1 = "pcreate2 expects (" + e1 + "int int)!";
  
  if ( !checkLength(l, 3, err1) )
    return l.typeError( err1 );
  
  NList attrs;
  if ( !checkStreamTuple( l.first(), attrs) )
    return l.typeError( argNotCorrect(1) + err1);
  
  if ( !l.second().isSymbol(sym.integer) )
    return l.typeError( argNotCorrect(2) + err1);
  
  if ( !l.third().isSymbol(sym.integer) )
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


static ListExpr shuffle2_tm(ListExpr args, const string& op)
{
  NList l(args);
  
  static const string& e1 = expects( sym.stream, sym.tuple );
  static const string& e2 = expects( sym.stream, sym.ptuple );
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
  return shuffle2_tm(args, "shuffle");
}

static int shuffle_vm( Word* args, Word& result, int message, 
                    Word& local, Supplier s)
{
  // args[0]: Input stream(ptuple(y))  or stream(tuple(y)) 
  return 0;
}   


static ListExpr memshuffle_tm(ListExpr args)
{
  return shuffle2_tm(args, "memshuffle");
}

static int memshuffle_vm( Word* args, Word& result, int message, 
                    Word& local, Supplier s)
{
  // args[0]: Input stream(ptuple(y))  or stream(tuple(y)) 
  return 0;
}   


}; // end of PartStreamMappings


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

      ConstructorInfo ci;
      ci.name      =    "ptuple";
      ci.signature =    "(ident x DATA)+ -> PTUPLE";
      ci.typeExample =  "ptuple";
      ci.listRep =      "Not supported! ";
      ci.valueExample = " - ";
      ci.remarks =      "Objects are only used in streams!";

      ConstructorFunctions<PTuple> ptf;
      TypeConstructor* ptuple = new TypeConstructor( ci, ptf );

      ptuple->AssociateKind( "PTUPLE" );
      AddTypeConstructor( ptuple );

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
        oi.example =   "plz pfeed[500] pdelete count"; 
      
      op = new Operator(
        oi,  
	psm::pfeed_vm,    
	psm::pfeed_tm     
      );

      AddOperator( op );
      
        oi.name =      "pdelete";
        oi.signature = "stream(ptuple(y) -> stream(tuple(y))";
        oi.syntax =    "_ pdelete";
        oi.meaning =   "Removes marker tuples from a stream";
        oi.example =   "plz pfeed[500] pdelete count"; 
      
      op = new Operator(
        oi,  
	psm::pdelete_vm,    
	psm::pdelete_tm     
      );

      AddOperator( op );
      
        oi.name =      "PSTREAM1";
        oi.signature = "((stream(ptuple(y)) ...) -> stream(tuple(y))";
        oi.syntax =    "Not available";
        oi.meaning =   "Type mapping operator";
        oi.example =   "plz pfeed[500] puse[fun (s: PSTREAM1) s head[5]] "
                       "pdelete count"; 
      
      op = new Operator(
        oi,  
	0,    
        psm::PSTREAM1_tm     
      );

      AddOperator( op );
      
        oi.name =      "PSTREAM2";
        oi.signature = "((...) (stream(ptuple(y)) ...) -> stream(tuple(y))";
        oi.syntax =    "Not available";
        oi.meaning =   "Type mapping operator";
        oi.example =   "plz pfeed[500] plz pfeed[100] "
                       "pjoin2[fun (s: PSTREAM1, t PSTREAM2) ..."; 
      
      op = new Operator(
        oi,  
	0,    
	psm::PSTREAM2_tm     
      );

      AddOperator( op );

        oi.name =      "puse";
        oi.signature = "stream(ptuple(y) x ( stream(tuple(y)) "
	               "->  stream(tuple(y)) ) -> stream(tuple(y))";
        oi.syntax =    "_ puse[ _ ]";
        oi.meaning =   "Hides the marker tuples for the parameter function "
                       "and inserts them again into the result stream";
        oi.example =   "plz pfeed[500] puse[. filter[.PLZ = 44227]] "
                       "pdelete count"; 
      
      op = new Operator(
        oi,  
	psm::puse_vm,    
	psm::puse_tm     
      );
 
     AddOperator( op );

        oi.name =      "pjoin2";
        oi.signature = "stream(ptuple(y) x ( stream(tuple(y)) "
	               "->  stream(tuple(y)) ) -> stream(tuple(y))";
        oi.syntax =    "_ puse[ _ ]";
        oi.meaning =   "Implements the adaptive join for two relations "
                       "given as streams.";
        oi.example =   "plz pfeed[500] plz pfeed[500] "
                       " pjoin2[ . .. {arg2} symjoin[.PLZ = .PLZ_arg2]] "
                       "pdelete count"; 
      
      op = new Operator(
        oi,  
	psm::pjoin2_vm,    
	psm::pjoin2_tm     
      );

      
      AddOperator( op );
      
        oi.name =      "pcreate";
        oi.signature = "stream(tuple(y)) x int -> stream(ptuple(y))";
        oi.syntax =    "_ pcreate[ _ ]";
        oi.meaning =   "Consumes a stream and creates a partitioned "
                       "stream like pfeed";
        oi.example =   "plz feed pcreate[100] pdelete count"; 
      
      op = new Operator(
        oi,  
	psm::pcreate_vm,    
	psm::pcreate_tm     
      );
      
      AddOperator( op );

      
        oi.name =      "pcreate2";
        oi.signature = "stream(tuple(y)) x int x int -> stream(ptuple(y))";
        oi.syntax =    "_ pcreate2[ _, _ ]";
        oi.meaning =   "Creates a partitioned stream but gets the inputs "
                       "stream size as third parameter. This is useful if "
                       "the input stream is returned "
                       "by an index structure and the size is known.";
        oi.example =   "plz feed pcreate[100, 42167] pdelete count"; 
      
      op = new Operator(
        oi,  
	psm::pcreate2_vm,    
	psm::pcreate2_tm     
      );
      
      AddOperator( op );

        oi.name =      "shuffle";
        oi.signature = "stream(ptuple(y)) -> stream(ptuple(y)), "
                       "stream(tuple(y)) -> stream(tuple(y))";
        oi.syntax =    "_ shuffle";
        oi.meaning =   "Overloaded operator which materializes a stream "
                       "and produces output tuples in random order.";
        oi.example =   "plz feed pshuffle count"; 
      
      op = new Operator(
        oi,  
	psm::shuffle_vm,    
	psm::shuffle_tm     
      );
      
      AddOperator( op );

        oi.name =      "memshuffle";
        oi.signature = "stream(ptuple(y)) -> stream(ptuple(y)), "
                       "stream(tuple(y)) -> stream(tuple(y))";
        oi.syntax =    "_ memshuffle";
        oi.meaning =   "Overloaded operator which shuffles tuples in a "
                       "memory buffer and outputs them in random order. "
                       "For large input streams the randomness might be "
                       "insufficient.";
        oi.example =   "plz feed memshuffle count"; 
      
      op = new Operator(
        oi,  
	psm::memshuffle_vm,    
	psm::memshuffle_tm     
      );
      
      AddOperator( op );

    }

    // We don't care about the deletion of Algebra and TypeConstructor
    // instances since they will be destroyed when the process terminates.
    ~PartStreamAlgebra() {};

};

PartStreamAlgebra partStreamAlgebra;

/*
5 Initialization

*/

extern "C"
Algebra*
InitializePartitionedStreamAlgebra(NestedList *nlRef, QueryProcessor *qpRef)
{
  nl = nlRef;
  qp = qpRef;
  return (&partStreamAlgebra);
}


