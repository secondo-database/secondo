/*
----
This file is part of SECONDO.

Copyright (C) 2015,
Faculty of Mathematics and Computer Science,
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


//[$][\$]

*/

#ifndef DARRAY_H
#define DARRAY_H

#include "DArrayElement.h"
#include "ArrayTypes.h"
#include <string>
#include <boost/thread/recursive_mutex.hpp>
#include "NestedList.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "SecondoSMI.h"

#include "Dist2Helper.h"

namespace distributed2{

/*
3 Class ~DArray~

This class represents the Secondo type ~darray~. It just stores the information
about a connection to a remote server. The actual connections are stored within
the algebra instance.

*/


std::string getName(const arrayType  a);
ListExpr wrapType(const arrayType  a, ListExpr subType);



class DistTypeBase{
 public:
   DistTypeBase(const std::vector<DArrayElement>& worker,
              const std::string& _name);
               
   DistTypeBase( const std::string& _name);

   DistTypeBase( const DistTypeBase& src);

   // DistTypeBase() {}
   explicit DistTypeBase(const int __attribute__((unused)) dummy) {}
     
   DistTypeBase& operator=(const DistTypeBase& src);

   virtual ~DistTypeBase(){}


   virtual arrayType getType() const = 0; 
   bool IsDefined() const{ return defined; }


/*
3.6 ~set~

This sets the size, the name, and the worker for a 
darray. The map from index to workers is the
standard map.

*/
   virtual void set(const std::string& name, 
                    const std::vector<DArrayElement>& worker);


     size_t numOfWorkers() const;

     virtual size_t getSize() const = 0;
     
     DArrayElement getWorker(int i);

     const std::vector<DArrayElement>& getWorker(){
        return worker;
     }

     std::string getName() const;
     bool setName( const std::string& n);


     virtual void makeUndefined();
     bool equalWorkers(const DistTypeBase&  a) const;

     const std::vector<DArrayElement>& getWorkers() const{
       return worker;
     }     


 protected:
    std::vector<DArrayElement> worker; // connection information
    std::string name;  // the basic name used on workers
    bool defined; // defined state of this array


/*
3.24 ~equalWorker~

Check for equaliness of workers.

*/
   bool equalWorkers(const std::vector<DArrayElement>& w) const;

};


class SDArray : public DistTypeBase {
  public:

   SDArray(const std::vector<DArrayElement>& worker,
           const std::string& _name): DistTypeBase(worker,_name){}
               
   SDArray( const std::string& _name): DistTypeBase(_name){}

   SDArray( const DistTypeBase& src):DistTypeBase(src){}

   explicit SDArray(const int __attribute__((unused)) dummy)
      :DistTypeBase(dummy) {}
     
   DistTypeBase& operator=(const DistTypeBase& src);

     

     
     virtual size_t getSize() const {
       return worker.size();
     }
   
     virtual arrayType getType() const{
        return SDARRAY;
     }

  const DArrayElement& getWorkerForSlot(int i){
    return worker[i];
  }

  std::string getObjectNameForSlot(int i) const{
     return name; 
  }


     inline static std::string BasicType(){
         return "sdarray";
     } 

     inline static bool checkType(ListExpr list){
        return nl->HasLength(list,2) &&
               listutils::isSymbol(nl->First(list),BasicType());
     }
  
     static ListExpr Property();

     static Word IN( const ListExpr typeInfo, const ListExpr instance,
      const int errorPos, ListExpr& errorInfo, bool& correct );

     static ListExpr Out( ListExpr typeInfo, Word value ) ;

     static Word Create( const ListExpr typeInfo ) {
        SDArray* res = new SDArray("");
        res->makeUndefined();
        return SetWord(res);
     }

     static  void Delete( const ListExpr typeInfo, Word& w ){
        SDArray* victim = (SDArray*) w.addr;
        delete victim;
        w.addr = 0;
     }

    static  bool Open( SmiRecord& valueRecord,
                 size_t& offset, const ListExpr typeInfo,
                 Word& value );

    static  bool Save( SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& value );

    static  void Close( const ListExpr typeInfo, Word& w ){
        SDArray* victim = (SDArray*) w.addr;
        delete victim;
        w.addr = 0;
    }
    
    static Word Clone( const ListExpr typeInfo, const Word& w ) {
        SDArray* a = (SDArray*) w.addr;
        return SetWord(new SDArray(*a));
    }

    static void*  Cast( void* addr ) {
      return (new (addr) SDArray(23));
    }
    
    static bool TypeCheck(ListExpr type, ListExpr& errorInfo){
       return checkType(type);
    }

    static int SizeOf(){
      return 85; // size depends on an object, not clear what to return here
    }

    static ListExpr wrapType(ListExpr subtype){
      return nl->TwoElemList( nl->SymbolAtom(BasicType()), subtype);
    }

  private:
    // privacy is unknown to this class

};



class DArrayBase: public DistTypeBase{
  public:

/*
3.1 Constructors

The constructors create a darray from predefined values.

*/

     DArrayBase(const std::vector<uint32_t>& _map, const std::string& _name);

     DArrayBase(const size_t _size , const std::string& _name);

     DArrayBase(const std::vector<uint32_t>& _map, const std::string& _name, 
               const std::vector<DArrayElement>& _worker);

     DArrayBase(const size_t _size, const std::string& _name, 
               const std::vector<DArrayElement>& _worker);

     explicit DArrayBase(int dummy):DistTypeBase(dummy) {}
     // only for cast function

     DArrayBase(const DArrayBase& src);

/*
3.2 Assignment Operator

*/
     DArrayBase& operator=(const DArrayBase& src);

     
     void copyFrom(const DArrayBase& src){
        *this = src;
     }

     void copyFrom(const DistTypeBase& src){
         DistTypeBase::operator=(src);
         setStdMap(src.getSize());
     }
 

/*
3.3 Destructor

*/
     virtual ~DArrayBase() {}


/*
3.4 ~getWorkerNum~

This fucntion returns the worker that is responsible for
the given index. 

*/
    uint32_t getWorkerNum(uint32_t index);



/*
3.5 ~getType~

*/
    arrayType getType() const = 0;

   
    const std::vector<uint32_t> getMap()const{
       return map;
    }
   

    size_t getSize() const;


/*
3.7 ~equalMapping~

Checks whether the mappings from indexes to the workers
are equal for two darray types.

*/
     bool equalMapping(DistTypeBase& a, bool ignoreSize )const;


/*
3.8 ~set~

Sets size, name and workers. Set to a standard map.

*/
     void set(const std::string& name, 
                    const std::vector<DArrayElement>& worker);

     virtual void set(const size_t size, const std::string& name, 
                    const std::vector<DArrayElement>& worker);
/*
3.9 ~set~

Sets the mapping, the workers and the name for a darray.
The size is extracted from the mapping.

*/
    void set(const std::vector<uint32_t>& m, const std::string& name, 
              const std::vector<DArrayElement>& worker);



/*
3.14 Some setters

*/

     virtual void makeUndefined();

     void setStdMap(size_t size);

     DArrayElement getWorkerForSlot(int i);

     size_t getWorkerIndexForSlot(int i);
     
     void setResponsible(size_t slot, size_t _worker);


/*
3.15 ~toListExpr~

Returns the list representation for this darray.

*/

     ListExpr toListExpr() const;


/*
3.16 ~readFrom~

Read a darray value from a list. If the list is not a valid
description, null is returned. The caller is responsible for 
deleting the return value, if the is one.

*/
     template<class R>
     static R* readFrom(ListExpr list);

/*
3.17 ~open~

Reads the content of darray from a SmiRecord.

*/
     template<class R>
     static bool open(SmiRecord& valueRecord, size_t& offset, 
                      const ListExpr typeInfo, Word& result);


/*
3.18 ~save~

Saves a darray to an SmiRecord.

*/
     static bool save(SmiRecord& valueRecord, size_t& offset,
                      const ListExpr typeInfo, Word& value);

/*
3.19 ~createStdMap~

Returns a vector representing the standard mapping from index to
worker.

*/
     static std::vector<uint32_t> createStdMap(const uint32_t size, 
                                          const int numWorkers);

/*
3.20 ~print~

Writes the content to an output stream.

*/
     void print(std::ostream& out)const;


/*
3.21 ~getObjectNameForSlot~

Returns the name of the remote object

*/
  std::string getObjectNameForSlot(const size_t slot) const{
     std::stringstream ss;
     ss << name << "_" << slot;
     return ss.str();
  }

/*
3.22 ~getFilePath~

*/
  std::string getFilePath(const std::string& home, const std::string& dbname,
                          const size_t slot){
    std::stringstream ss;
    ss << home << "/dfarrays/" << dbname << "/" << name << "/" 
       << name << "_" << slot << ".bin";
    ss.flush();
    return ss.str();
  }

  std::string getFilePath(const size_t slot){
     std::string home = SmiEnvironment::GetSecondoHome();
     std::string db = SecondoSystem::GetInstance()->GetDatabaseName();
     return getFilePath(home,db,slot);
  }

/*
3.22 ~getFilePath~

*/
  std::string getPath(const std::string& home, const std::string& dbname){
    std::stringstream ss;
    ss << home << "/dfarrays/" << dbname << "/" << name << "/"; 
    return ss.str();
  }



/*
3.22 ~createFromRel ~

Reads the content of a darray from a relation defining 
the workers. The name and the size are explicitely given.
The relation must have at least 3 attributes. The attribute 
at position hostPos must be of type H (CcString or FText) and
describes the host of the worker. At potPos, a ~CcInt~ describes 
the port of the SecondoMonitor. At position configPos, an attribute
of type C (CcString of FText) describes the configuration file
for connecting with the worker. 

*/
      template<class H, class C, class R>
      static R createFromRel(Relation* rel, int size, 
                              std::string name,
                              int hostPos, int portPos, int configPos);

      bool equalMapping(DArrayBase& a, bool ignoreSize ) const;

  protected:
    std::vector<uint32_t> map;  // map from index to worker
    mutable boost::recursive_mutex mapmtx;


/*
3.23  ~checkMap~

Checks whether the contained map is valid.

*/
   bool checkMap();

/*
3.24 ~isStdMap~

Checks whether the contained map is a standard map.

*/
   bool isStdMap() const;



};



std::ostream& operator<<(std::ostream& o, const DArrayBase& a);



 template<class H, class C, class R>
 R DArrayBase::createFromRel(Relation* rel, int size,
                         std::string name, int hostPos, int portPos, int 
                         configPos){
     std::vector<uint32_t> m;
     R result(m,"");
     if(size<=0){
        result.makeUndefined();
        return result;
     }


     if(!stringutils::isIdent(name)){
        result.makeUndefined();
        return result;
     }

     result.defined = true;
     result.name = name;

     GenericRelationIterator* it = rel->MakeScan();
     Tuple* tuple;
     while((tuple = it->GetNextTuple())){
        DArrayElement* elem = 
               DArrayElement::createFromTuple<H,C>(tuple,
               result.worker.size(),hostPos, 
               portPos, configPos);
        tuple->DeleteIfAllowed();
        if(elem){
           result.worker.push_back(*elem);
           delete elem;
        }
     } 
     delete it;
     result.setStdMap(size);

     return result;

}


template<arrayType type>
class DArrayT: public DArrayBase{
 public: 
 
   DArrayT(const std::vector<uint32_t>&v, const std::string& name):
       DArrayBase(v,name) {} 

   DArrayT(const int dummy):DArrayBase(dummy) {}

   DArrayT(const DArrayBase& src) : DArrayBase(src) {}
   
   DArrayT& operator=(const DArrayBase& src){
      DArrayBase::operator=(src);
      return *this;
   }

  static const std::string BasicType(){
     return distributed2::getName(type);;
  }

  static ListExpr wrapType(ListExpr subtype){
     return distributed2::wrapType(type, subtype); 
  }


  static DArrayT* readFrom(ListExpr list){
    return DArrayBase::readFrom<DArrayT<type> >(list);
  }


  arrayType getType()const{ return type; }

  static bool checkType(const ListExpr list);

};

typedef DArrayT<DARRAY> DArray;
typedef DArrayT<DFARRAY> DFArray;



class DFMatrix: public DistTypeBase{
   public: 
     DFMatrix(const size_t _size, const std::string& _name);
     DFMatrix(const size_t _size, const std::string& _name, 
              const std::vector<DArrayElement>& _worker); 
   
     
     explicit DFMatrix(int dummy):DistTypeBase(dummy) {} 
     // only for cast function

     void setSize(size_t newSize);
     static const std::string BasicType();

     static bool open(SmiRecord& valueRecord, size_t& offset,     
                 const ListExpr typeInfo, Word& result);

     virtual arrayType getType() const {
        return DFMATRIX;
     } 

     static bool save(SmiRecord& valueRecord, size_t& offset,
                      const ListExpr typeInfo, Word& value);

     static bool checkType(ListExpr e);

     
     ListExpr toListExpr() const;
     static DFMatrix* readFrom(ListExpr list);

     size_t getSize() const{
       return size; 
     }

     void copyFrom(const DFMatrix& M){
        *this = M;
     }

     void copyFrom(const DArrayBase& A){
         DistTypeBase::operator=(A);
         size = A.getSize();
     }


   private:
      size_t  size; 

}; 


} // end of namespace distributed2


#endif


