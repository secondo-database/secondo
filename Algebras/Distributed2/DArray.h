
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

#include <string>
#include "NestedList.h"
#include "RelationAlgebra.h"

#include "Dist2Helper.h"

namespace distributed2{

/*

2 Class ~DArrayElement~

This class represents information about a single worker of a DArray.

*/



class DArrayElement{
  public:
     DArrayElement(const std::string& _server, const int _port,
                    const int _num, const std::string& _config);

     DArrayElement(const DArrayElement& src);

     DArrayElement& operator=(const DArrayElement& src);


     ~DArrayElement();

      inline void setNum(const int num){
         this->num = num;
      }

     void set(const std::string& server, const int port, 
              const int num, const std::string& config);


     bool operator==(const DArrayElement& other) const;
     
     inline bool operator!=(const DArrayElement& other) const{
       return   !((*this) == other);
     }

     bool operator<(const DArrayElement& other) const;
     
     bool operator>(const DArrayElement& other) const;
     
     ListExpr toListExpr();

     bool readFrom(SmiRecord& valueRecord, size_t& offset);

     bool saveTo(SmiRecord& valueRecord, size_t& offset);

     void print(std::ostream& out)const;

     inline std::string getHost()const{ return server; }
     inline int getPort() const {return port; }
     inline std::string getConfig() const{ return config; }
     inline int getNum() const{ return num; }


     template<class H, class C>
     static DArrayElement* createFromTuple(Tuple* tuple, int num, 
                                   int hostPos, int portPos, int configPos){

         if(!tuple || (num < 0) ) {
            return 0;
         }

         H* CcHost = (H*) tuple->GetAttribute(hostPos);
         CcInt* CcPort = (CcInt*) tuple->GetAttribute(portPos);
         C* CcConfig = (C*) tuple->GetAttribute(configPos);

         if(!CcHost->IsDefined() || !CcPort->IsDefined() || 
            !CcConfig->IsDefined()){
             return 0;
         }
         std::string host = CcHost->GetValue();
         int port = CcPort->GetValue();
         std::string config = CcConfig->GetValue();
         if(port<=0){
            return 0;
         }
         return new DArrayElement(host,port,num,config);
     }


  private:
     std::string server;
     uint32_t port;
     uint32_t num;
     std::string config;
};

std::ostream& operator<<(std::ostream& out, const DArrayElement& elem);


bool InDArrayElement(ListExpr list, DArrayElement& result);

/*
3 Class ~DArray~

This class represents the Secondo type ~darray~. It just stores the information
about a connection to a remote server. The actual connections are stored within
the algebra instance.

*/

enum arrayType{DARRAY,DFARRAY,DFMATRIX};



class DArrayType{
 public:
   virtual arrayType getType() const = 0; 
   virtual ~DArrayType(){};
};


template<arrayType Type>
class DArrayT: public DArrayType{
  public:

/*
3.1 Constructors

The constructors create a darray from predefined values.

*/

     DArrayT(const std::vector<uint32_t>& _map, const std::string& _name);

     DArrayT(const size_t _size , const std::string& _name);

     DArrayT(const std::vector<uint32_t>& _map, const std::string& _name, 
               const std::vector<DArrayElement>& _worker);

     DArrayT(const size_t _size, const std::string& _name, 
               const std::vector<DArrayElement>& _worker);

     explicit DArrayT(int dummy) {} // only for cast function

     DArrayT(const DArrayT<Type>& src);

/*
3.2 Assignment Operator

*/
     template<arrayType T>
     DArrayT& operator=(const DArrayT<T>& src);

/*
3.3 Destructor

*/
     ~DArrayT();


/*
3.4 ~getWorkerNum~

This fucntion returns the worker that is responsible for
the given index. This operation cannot applied to a 
DFMATRIX.

*/
    uint32_t getWorkerNum(uint32_t index);

/*
3.5 ~getType~

Returns the template type.

*/
    arrayType getType() const;

/*
3.6 ~setSize~

Sets a new size. This operation is only allowed for
the DFMATRIX type.

*/

    void setSize(size_t newSize);

/*
3.6 ~set~

This sets the size, the name, and the worker for a 
darray. The map from index to workers is the
standard map.

*/
    void set(const size_t size, const std::string& name, 
              const std::vector<DArrayElement>& worker);


/*
3.7 ~equalMapping~

Checks whether the mappings from indexes to the workers
are equal for two darray types.

*/
     template<class AT>
     bool equalMapping(AT& a, bool ignoreSize );


/*
3.9 ~set~

Sets the mapping, the workers and the name for a darray.
The size is extracted from the mapping.

*/
    void set(const std::vector<uint32_t>& m, const std::string& name, 
              const std::vector<DArrayElement>& worker);

/*
3.10 ~IsDefined~

Checks whether this darray is in a defined state.

*/
     bool IsDefined();

/*
3.11 ~BasicType~

Returns the basic type of a  darray. The result depend on the
template type.

*/

     static const std::string BasicType();

/*
3.12 ~checkType~

Checks wether the argument is complete decsription of a darray.

*/
     static const bool checkType(const ListExpr list);

/*
3.13 Some Getters

*/
     size_t numOfWorkers() const;

     size_t getSize() const;
     
     DArrayElement getWorker(int i);

     std::string getName() const;


/*
3.14 Some setters

*/

     void makeUndefined();

     void setStdMap(size_t size);

     DArrayElement getWorkerForSlot(int i);

     size_t getWorkerIndexForSlot(int i);
     
     void setResponsible(size_t slot, size_t _worker);

     bool setName( const std::string& n);

/*
3.15 ~toListExpr~

Returns the list representation for this darray.

*/

     ListExpr toListExpr();


/*
3.16 ~readFrom~

Read a darray value from a list. If the list is not a valid
description, null is returned. The caller is responsible for 
deleting the return value, if the is one.

*/
     static DArrayT<Type>* readFrom(ListExpr list);

/*
3.17 ~open~

Reads the content of darray from a SmiRecord.

*/

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
     void print(std::ostream& out);



/*
3.21 ~equalWorker~

Checks whether the worker definitions are equal.

*/
      template<class TE>
      bool equalWorkers(const TE& a) const;


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
      template<class H, class C>
      static DArrayT<Type> createFromRel(Relation* rel, int size, 
                              std::string name,
                              int hostPos, int portPos, int configPos);



  friend class DArrayT<DARRAY>;
  friend class DArrayT<DFARRAY>;
  friend class DArrayT<DFMATRIX>;

  private:
    std::vector<DArrayElement> worker; // connection information
    std::vector<uint32_t> map;  // map from index to worker
    size_t  size; // corresponds with map size except map is empty
    std::string name;  // the basic name used on workers
    bool defined; // defined state of this array


/*
3.23  ~checkMap~

Checks whether the contained map is valid.

*/
   bool checkMap();

/*
3.24 ~isStdMap~

Checks whether the contained map is a standard map.

*/
   bool isStdMap();


/*
3.24 ~equalWorker~

Check for equaliness of workers.

*/
   bool equalWorker(const std::vector<DArrayElement>& w) const;

};


template<arrayType Type>
 template<class H, class C>
 DArrayT<Type> DArrayT<Type>::createFromRel(Relation* rel, int size,
                         std::string name, int hostPos, int portPos, int 
                         configPos){
     std::vector<uint32_t> m;
     DArrayT<Type> result(m,"");
     if(size<=0){
        result.defined = false;
        return result;
     }
     if(!stringutils::isIdent(name)){
        result.defined = false;
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



typedef DArrayT<DARRAY> DArray;
typedef DArrayT<DFARRAY> DFArray;
typedef DArrayT<DFMATRIX> DFMatrix;

} // end of namespace distributed2


#endif


