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



#include "semaphore.h"

#include "fsrel.h"
#include "DArray.h"
#include "frel.h"
#include <iostream>
#include <vector>
#include <list>

#include "SecondoInterface.h"
#include "SecondoInterfaceCS.h"
#include "FileSystem.h"
#include "Algebra.h"
#include "NestedList.h"
#include "StandardTypes.h"
#include "FTextAlgebra.h"
#include "ListUtils.h"
#include "StringUtils.h"
#include "RelationAlgebra.h"
#include "Stream.h"
#include "NList.h"
#include "ArrayAlgebra.h"
#include "SocketIO.h"
#include "StopWatch.h"

#include "Bash.h"
#include "DebugWriter.h"


#include "FileRelations.h"
#include "FileAttribute.h"



  // use boost for thread handling

#include <boost/thread.hpp>
#include <boost/date_time.hpp>


extern DebugWriter dwriter;

extern boost::mutex nlparsemtx;


using namespace std;
using namespace distributed2;

namespace distributed3 {


class Distributed3Algebra: public Algebra{

  public:

/*
1.1 Constructor defined at the end of this file

*/
     Distributed3Algebra();


/*
1.2 Destructor

*/
     ~Distributed3Algebra();


};

Distributed3Algebra* algInstance;



/*
2.1.1 Class ~taskElement~

This class represents the Elements of Code.

*/
class TaskElement{
 public:
    TaskElement( Word* args, Word& result, Supplier s): pos(0){
//      tt = new TupleType(nl->Second(GetTupleResultType(s)));
      
      dep = 0;
      a1Name = ((DArray*) args[0].addr)->getName();
      a2Name = ((DArray*) args[1].addr)->getName();
      part1 = 0;
      part2 = 0;
      funQuery = ((CcString*) args[4].addr)->GetValue();
      max = min(((DArray*) args[0].addr)->getSize(), 
                ((DArray*) args[1].addr)->getSize());

      resName = ((DArray*) result.addr)->getName();
      resPart = 0;
    }

    TaskElement( size_t _pos, size_t _dep, string _a1Name,string _a2Name,
               size_t  _part1,size_t  _part2,string _funQuery,
               string _resArrayName,size_t  _resPart){
      pos =_pos;
      dep = _dep;
      a1Name = _a1Name;
      a2Name = _a2Name;
      part1 = _part1;
      part2 = _part2;
      funQuery = _funQuery; 
      resName = _resArrayName;
      resPart = _resPart;
    }
    
    
    TaskElement(ListExpr list){
      if(!nl->HasLength(list,9)){
        return;
      }
      ListExpr e1 = nl->First(list);
      pos = nl->IntValue(e1);
      list = nl->Rest(list);

      ListExpr e2 = nl->First(list);
      dep = nl->IntValue(e2);
      list = nl->Rest(list);

      ListExpr e3 = nl->First(list);
      a1Name = nl->StringValue(e3);
      list = nl->Rest(list);

      ListExpr e4 = nl->First(list);
      a2Name = nl->StringValue(e4);
      list = nl->Rest(list);

      ListExpr e5 = nl->First(list);
      part1 = nl->IntValue(e5);
      list = nl->Rest(list);

      ListExpr e6 = nl->First(list);
      part2 = nl->IntValue(e6);
      list = nl->Rest(list);

      ListExpr e7 = nl->First(list);
      funQuery = nl->StringValue(e7);
      list = nl->Rest(list);

      ListExpr e8 = nl->First(list);
      resName = nl->StringValue(e8);
      list = nl->Rest(list);

      ListExpr e9 = nl->First(list);
      resPart = nl->IntValue(e9);
      list = nl->Rest(list);
    }

    
    ListExpr toListExpr(){
      ListExpr expr1 = nl->SixElemList(
        nl->StringAtom(a2Name),
        nl->IntAtom(part1),
        nl->IntAtom(part2),
        nl->StringAtom(funQuery),
        nl->StringAtom(resName),
        nl->IntAtom(resPart)
      );
      
      ListExpr expr2 = nl->Cons(nl->StringAtom(a1Name),expr1);
      ListExpr expr3 = nl->Cons(nl->IntAtom(dep),expr2);
      ListExpr expr = nl->Cons(nl->IntAtom(pos),expr3);
      return expr;
    }


    ~TaskElement(){
    }
    
    Tuple* getTuple(TupleType* tt){
      Tuple* res = new Tuple(tt);
      res->PutAttribute(0, new CcInt(true,pos));
      res->PutAttribute(1, new CcInt(true,dep));
      res->PutAttribute(2, new CcString(true,a1Name));
      res->PutAttribute(3, new CcString(true,a2Name));
      res->PutAttribute(4, new CcInt(true,part1));
      res->PutAttribute(5, new CcInt(true,part2));
      res->PutAttribute(6, new CcString(true,funQuery));
      res->PutAttribute(7, new CcString(true,resName));
      res->PutAttribute(8, new CcInt(true,resPart));
      return res;
    }

    bool readFrom(SmiRecord& valueRecord, size_t& offset){
        if(!readVar(pos,valueRecord,offset)){
            return false;
        }
        if(!readVar(dep,valueRecord,offset)){
           return false;
        }
        if(!readVar(a1Name,valueRecord,offset)){
           return false;
        }
        if(!readVar(a2Name,valueRecord,offset)){
           return false;
        }
        if(!readVar(part1,valueRecord,offset)){
           return false;
        }
        if(!readVar(part2,valueRecord,offset)){
           return false;
        }
        if(!readVar(funQuery,valueRecord,offset)){
           return false;
        }
        if(!readVar(resName,valueRecord,offset)){
           return false;
        }
        if(!readVar(resPart,valueRecord,offset)){
           return false;
        }
        return true;
     }


    bool saveTo(SmiRecord& valueRecord, size_t& offset){
        if(!writeVar(pos,valueRecord,offset)){
            return false;
        }
        if(!writeVar(dep,valueRecord,offset)){
           return false;
        }
        if(!writeVar(a1Name,valueRecord,offset)){
           return false;
        }
        if(!writeVar(a2Name,valueRecord,offset)){
           return false;
        }
        if(!writeVar(part1,valueRecord,offset)){
           return false;
        }
        if(!writeVar(part2,valueRecord,offset)){
           return false;
        }
        if(!writeVar(funQuery,valueRecord,offset)){
           return false;
        }
        if(!writeVar(resName,valueRecord,offset)){
           return false;
        }
        if(!writeVar(resPart,valueRecord,offset)){
           return false;
        }
         return true; 
     }
     
    void print(ostream& out)const{
      out << "( Task: " << pos << ", Dep : " << dep 
        << ", Arg1 : " << a1Name << ", Arg2 : " << a2Name
        << ", Part1 : " << part1 << ", Part2 : " << part2
        << ", Query : " << funQuery 
        << ", Res : " << resName << ", ResPart : " << resPart
        << ")";
    }

    size_t GetPos(){
      return pos;
    }
    
    size_t GetDep(){
      return dep;
    }
    
    string GetA1Name(){
      return a1Name;
    }
    
    string GetA2Name(){
      return a2Name;
    }
    
    size_t  GetPart1(){
      return part1;
    }
    
    size_t  GetPart2(){
      return part2;
    }
    
    string GetFunQuery(){
      return funQuery;
    }
    
    string GetResName(){
      return resName;
    }
    
    size_t GetResPart(){
      return resPart;
    }
     
     
 private:
    size_t  max;
    TupleType* tt;
    
    size_t pos;
    size_t dep;
    string a1Name;
    string a2Name;
    size_t  part1;
    size_t  part2;
    string funQuery;
    string resName;
    size_t  resPart;

};



/*
2.1.2 Class ~Code~

This class represents the Secondo type ~code~. It just stores the list of tasks.

*/

class Code{
  public:
     Code(const vector<TaskElement*>& _tasks): 
           tasks(_tasks) {
        defined = true;
     }

     explicit Code() {} // only for cast function


     Code& operator=(const Code& src) {
        this->tasks = src.tasks;
        this->defined = src.defined;
        return *this;
     }     
 
     ~Code() {
         size_t s = tasks.size();
         for(size_t i=0;i<s;i++){
           delete tasks[i];
         }
         tasks.clear();
     }

    void set(const vector<TaskElement*>& tasks){
        if(tasks.size() ==0){ // invalid
           makeUndefined(); 
           return;
        }
        defined = true;
        this->tasks = tasks;
     }

    void add(const vector<TaskElement*> tasks){
        if(tasks.size() ==0){ // invalid
           makeUndefined(); 
           return;
        }
        for(size_t i=0;i<tasks.size();i++){
          this->tasks.push_back(tasks[i]);
        }
     }

     bool IsDefined(){
        return defined;
     }

     static const string BasicType() { 
       return "code";
     }

     static const bool checkType(const ListExpr list){
      return listutils::isSymbol(list, BasicType());
     }

     size_t numOfTasks() const{
       return getSize();
     }
     
     size_t getSize() const{
       return (size_t)tasks.size();
     }
 
     TaskElement* getTask(size_t i){
        if(i<0 || i>= tasks.size()){
           assert(false);
        }
        return tasks[i];
     } 

     ListExpr toListExpr(){
       if(!defined){
         return listutils::getUndefined();
       }

       ListExpr tl;
       if(tasks.empty()){
         tl =  nl->TheEmptyList();
       } else {
         int index = tasks.size() - 1;
         tl = nl->OneElemList(tasks[index]->toListExpr());
         while(index > 0){
           index--; 
           tl = nl->Cons(tasks[index]->toListExpr(), tl);
         }
       }
       
       ListExpr listExpr = nl->ThreeElemList(nl->SymbolAtom( BasicType()), 
                                nl->IntAtom(tasks.size()), 
                                tl); 
       return listExpr; 
     }


     static Code* readFrom(ListExpr list){
        if(listutils::isSymbolUndefined(list)){
           vector<TaskElement*> tl;
           return new Code(tl);
        }
        if(!nl->HasLength(list,3)){
           return 0;
        }
        ListExpr Tasks = nl->Second(list);

        vector<TaskElement*> tl;
        while(!nl->IsEmpty(Tasks)){
           TaskElement* elem = new TaskElement(nl->First(Tasks));
           tl.push_back(elem);
           Tasks = nl->Rest(Tasks);
        }
        Code* result = new Code(tl);
        result->defined = true;
        return result;
     }
     
     static bool open(SmiRecord& valueRecord, size_t& offset, 
                      const ListExpr typeInfo, Word& result){
        bool defined;
        result.addr = 0;
        if(!readVar<bool>(defined,valueRecord,offset)){
           return false;
        } 
        if(!defined){
          vector<TaskElement*> tasks;
          result.addr = new Code(tasks);
          return true;
        }
        // object in smirecord is defined, read size
        size_t s;
        if(!readVar<size_t>(s,valueRecord,offset)){
           return false;
        }

        // read  vector
        vector<TaskElement*> tasks;
        Code* res= new Code(tasks);

        // append tasks
        for(size_t i=0; i< s; i++){
           size_t s = 0;
           TaskElement*  task = new TaskElement(s);
           if(!task->readFrom(valueRecord, offset)){
               delete res;
               return false;
           }
           res->tasks.push_back(task);
        }
        res->defined = true;
        result.addr = res;
        return true;
     }

     static bool save(SmiRecord& valueRecord, size_t& offset,
                      const ListExpr typeInfo, Word& value) {
       
         Code* c = (Code*) value.addr;
         // defined flag
         if(!writeVar(c->defined,valueRecord,offset)){
           return false;
         }
         if(!c->defined){
            return true;
         }
         // size
         size_t s = c->getSize();
         if(!writeVar(s,valueRecord,offset)){
           return false;
         }
         // tasks
         for(size_t i=0;i<s;i++){
           if(!c->tasks[i]->saveTo(valueRecord,offset)){
             return false;
           }
         }
         return true; 
     }


     void print(ostream& out){
       if(!defined){
          out << "undefined";
          return;
       }

       out << ", size : " << tasks.size()
           << " tasks : [" ;
       for(size_t i =0;i<tasks.size();i++){
          if(i>0) out << ", ";
          tasks[i]->print(out);
       }
       out << "]";
     }

     void makeUndefined(){
        tasks.clear();
        defined = false;
     }
     
     size_t GetStartIndexOfLastResult(){
      size_t lastIndex = tasks.size() - 1;
      TaskElement* lastElement = tasks[lastIndex];
       for(size_t i =lastIndex - 1; i+1 != 0;i--){
          if(tasks[i]->GetResName() == lastElement->GetA1Name() ||
            tasks[i]->GetResName() == lastElement->GetA2Name()){
            return i + 1;
          }
       }
       return 0;
     }
     
     void BeginIteration(){
       iter = 0;
     }

     TaskElement* GetNext(){
       if(iter < tasks.size()){
         return tasks[iter++];
       }
       return 0;
     }

  private:
    vector<TaskElement*> tasks; // the Tasks information
    bool defined; // defined state of this Code Element
    size_t  iter; // the actual index for an interatition
};


/*
2.1.2.1 Property function

*/
ListExpr CodeProperty(){
   return nl->TwoElemList(
            nl->FourElemList(
                 nl->StringAtom("Signature"),
                 nl->StringAtom("Example Type List"), 
                 nl->StringAtom("List Rep"),
                 nl->StringAtom("Example List")),
            nl->FourElemList(
                 nl->StringAtom(" -> SIMPLE"),
                 nl->StringAtom(" (code(darray <basictype>))"),
                 nl->TextAtom("(size ( t1 t2  ...)) where "
                     "t_i =(pos dep arg1 arg2 part1 part2 query res resPart)"),
                 nl->TextAtom("( 1(1 0 'R' 'S' 0 0 " 
                     "'.feed{r} ..feed{s} hashjoin[R_a, S_b] count' 'T' 0)")));
}

/*
2.1.2.2 IN function

*/

Word InCode(const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct){

   Word res((void*)0);
   res.addr = Code::readFrom(instance);
   correct = res.addr!=0;
   return res;
}

/*

2.1.2.3 Out function

*/
ListExpr OutCode(ListExpr typeInfo, Word value){
   Code* c = (Code*) value.addr;
   return c->toListExpr();
}

/*

2.1.2.4 Create function

*/
Word CreateCode(const ListExpr typeInfo){
  Word w;
  vector<TaskElement*> t;
  w.addr = new Code(t);
  return w;
}

/*

2.1.2.4 Delete function

*/
void DeleteCode(const ListExpr typeInfo, Word& w){
  Code* c = (Code*) w.addr;
  delete c;
  w.addr = 0;
}

/*

2.1.2.4 Close function

*/
void CloseCode(const ListExpr typeInfo, Word& w){
  Code* c = (Code*) w.addr;
  delete c;
  w.addr = 0;
}

/*

2.1.2.4 Clone function

*/
Word CloneCode(const ListExpr typeInfo, const Word& w){
    Code* c = (Code*) w.addr;
    Word res;
    res.addr = new Code(*c);
    return res;
}

void* CastCode(void* addr){
   const vector<TaskElement*> ts;
   return (new (addr) Code(ts));   
}

bool CodeTypeCheck(ListExpr type, ListExpr& errorInfo){
    return Code::checkType(type);
}


int SizeOfCode(){
  return 42; // a magic number
}


TypeConstructor CodeTC(
  Code::BasicType(),
  CodeProperty,
  OutCode, InCode,
  0,0,
  CreateCode, DeleteCode,
  Code::open, Code::save,
  CloseCode, CloneCode,
  CastCode,
  SizeOfCode,
  CodeTypeCheck );


/*
1.15 Operator ~dloop3~

This operator transforms a function that will be performed 
over all entries of a DArray into a Code object.

1.15.1 Type Mapping

The signature is
darray(X) x string x (X->Y) -> code(darray(Y))
or
Code(darray(X)) x string x (X->Y) -> code(darray(Y))

*/

ListExpr dloop3TM(ListExpr args){
   string err ="darray(X) x string x fun: X -> Y   or "
       "code(darray(X)) x string x fun: X -> Y   expected";

  if(!nl->HasLength(args,3) ){
    return listutils::typeError(err + "(wrong number of args)");
  }
  
  ListExpr temp = args;
  while(!nl->IsEmpty(temp)){
    if(!nl->HasLength(nl->First(temp),2)){
        return listutils::typeError("internal Error");
     }
     temp = nl->Rest(temp);
  }

  ListExpr darray = nl->First(args);
  ListExpr fun;
  
  if(!CcString::checkType(nl->First(nl->Second(args)))){
     return listutils::typeError("Second arg not of type string");
  }
  
  fun = nl->Third(args);
  ListExpr funType = nl->First(fun);
  ListExpr arrayType = nl->First(darray);

  if(!DArray::checkType(arrayType) && !Code::checkType(arrayType)){
    return listutils::typeError(err + ": first arg not a darray or code");
  }

  if(!listutils::isMap<1>(funType)){
    return listutils::typeError(err + ": last arg is not a function");
  }
  if(DArray::checkType(arrayType)){
    if(!nl->Equal(nl->Second(arrayType), nl->Second(funType))){
      return listutils::typeError("type mismatch between darray and "
                                  "function arg");
    }
  }
   
  ListExpr funquery = nl->Second(fun);
  
  ListExpr funargs = nl->Second(funquery);

  ListExpr dat;
  if(DArray::checkType(nl->First(nl->First(args)))){
    dat = nl->Second(arrayType);
  }else{
    dat = nl->Second(funType);
  }

  ListExpr rfunargs = nl->TwoElemList(
                        nl->First(funargs),
                        dat);
  ListExpr rfun = nl->ThreeElemList(
                        nl->First(funquery),
                        rfunargs,
                        nl->Third(funquery));   

  if(DArray::checkType(nl->First(nl->First(args))) 
    && CcString::checkType(nl->First(nl->Second(args)))
    && listutils::isMap<1>(nl->First(nl->Third(args)))){
 
    return nl->ThreeElemList(
               nl->SymbolAtom(Symbols::APPEND()),
               nl->OneElemList(nl->TextAtom(nl->ToString(rfun))),
               NList(Code::BasicType()).listExpr());
  }

  if(Code::checkType(nl->First(nl->First(args))) 
    && CcString::checkType(nl->First(nl->Second(args)))
    && listutils::isMap<1>(nl->First(nl->Third(args)))){
    
    return nl->ThreeElemList(
               nl->SymbolAtom(Symbols::APPEND()),
               nl->OneElemList(nl->TextAtom(nl->ToString(rfun))),
               NList(Code::BasicType()).listExpr());
  }
  return listutils::typeError(err);
}

/*
1.15.2 Selection Function

*/
int loop3Select( ListExpr args )
{
  if(DArray::checkType(nl->First(args))){
    return 0;
  }
  return 1;
}

/*
1.15.3 Value Mapping Function

*/


int dloop3VM_Array(Word* args, Word& result, int message,
           Word& local, Supplier s ){
   string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
   DArray* array = (DArray*) args[0].addr;
   result = qp->ResultStorage(s);
   Code* res = (Code*) result.addr;
   
   if(! array->IsDefined()){
      res->makeUndefined();
      return 0;
   }  
   string a1Name = array->getName();

   string a2Name = "";
   size_t  part2 = 0;

   string resName;
   CcString* name = (CcString*) args[1].addr;
   if(name->IsDefined() && !(name->GetValue().length()==0)){
      resName = name->GetValue();
   }

   string fun = ((FText*) args[3].addr)->GetValue();
   
   vector<TaskElement*> tasks;
   int pos = 0;
   TaskElement* element;
   for(size_t i=0; i < array->getSize(); i++){
      pos++;
      element = new TaskElement(pos,0,a1Name,a2Name,i+1,part2,fun,resName,i+1);
      tasks.push_back(element);
   }
   res->set(tasks);
   return 0; 
}

int dloop3VM_Code(Word* args, Word& result, int message,
           Word& local, Supplier s ){
   string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
   Code* code = (Code*) args[0].addr;
   result = qp->ResultStorage(s);
   Code* res = (Code*) result.addr;

   if(!code->IsDefined()){
      res->makeUndefined();
      return 0;
   }  

   string a2Name = "";

   CcString* name = (CcString*) args[1].addr;
   string resName;
   if(name->IsDefined() && !(name->GetValue().length()==0)){
      resName = name->GetValue();
   }
   if(!stringutils::isIdent(resName)){
    res->makeUndefined();
    return 0;
   }
   
   string fun = ((CcString*) args[3].addr)->GetValue();
   
   vector<TaskElement*> tasks;
   TaskElement* e;
   TaskElement* element;

   size_t size = code->getSize();
   for(size_t i = 0; i < size; i++){
     e = code->getTask(i);
     element = new TaskElement( e->GetPos(),e->GetDep(),e->GetA1Name(),
                          e->GetA2Name(),e->GetPart1(),e->GetPart2(),
                          e->GetFunQuery(),e->GetResName(),e->GetResPart());
     tasks.push_back(element);
   }

   size_t pos = size;
   for(size_t i = code->GetStartIndexOfLastResult(); i < size;i++){
     pos++;
     e = code->getTask(i);
     element = new TaskElement( pos,i+1,e->GetResName(),a2Name,
                          e->GetResPart(),0,fun,resName,e->GetResPart());
     tasks.push_back(element);
   }
   res->set(tasks);
   return 0; 
}

ValueMapping dloop3VM[] = {
  dloop3VM_Array,
  dloop3VM_Code
  
};

/*
1.15.4 Specification

*/
OperatorSpec dloop3Spec(
     " {darray(X), code(darray(X))} x string x  (X->Y) -> code(darray(y))",
     " _ dloop3[_,_]",
     "Generates a Code Object that performs a function "
     " on each element of a darray instance."
     "The string argument specifies the name of the result. If the name"
     " is undefined or an empty string, a name is generated automatically.",
     "query da2 dloop3[\"da3\", . toTasks"
     );

/*
1.15.5 Operator instance

*/
Operator dloop3Op(
  "dloop3",
  dloop3Spec.getStr(),
  2,
  dloop3VM,
  loop3Select,
  dloop3TM
);


/*
1.16 Operator ~DARRAYELEM~ , ~DARRAYELEM2~

This operators checks whether a parameter of a function is an
element of a DARRAY


*/

ListExpr
DARRAYELEMfromCodeTM( ListExpr args )
{

  if(!nl->HasMinLength(args,1)){
    return listutils::typeError("at least one argument required");
  }
  ListExpr first = nl->First(args);
  if(!Code::checkType(first)){
    return listutils::typeError("code expected");
  }
  return first;
}

OperatorSpec DARRAYELEMfromCodeSpec(
     "code(X) -> X ",
     "DARRAYELEM(_)",
     "Type Mapping Operator. Extract the type of a code.",
     "query c2 dloop[\"da3\", . count]"
     );

Operator DARRAYELEMfromCodeOp (
      "DARRAYELEM",
      DARRAYELEMfromCodeSpec.getStr(),
      0,   
      Operator::SimpleSelect,
      DARRAYELEMfromCodeTM );


ListExpr
DARRAYELEM2fromCodeTM( ListExpr args )
{
  if(!nl->HasMinLength(args,2)){
    return listutils::typeError("two arguments required");
  }
  ListExpr second = nl->Second(args);
  if(!Code::checkType(second)){
    return listutils::typeError("code expected");
  }
  return second;
}

OperatorSpec DARRAYELEM2fromCodeSpec(
     "T x code(Y) x ... -> Y ",
     "DARRAYELEM2(_)",
     "Type Mapping Operator. Extract the type of a darray.",
     "query c2 c3 dloop[\"da3\", .. count"
     );

Operator DARRAYELEM2fromCodeOp (
      "DARRAYELEM2",
      DARRAYELEM2fromCodeSpec.getStr(),
      0,   
      Operator::SimpleSelect,
      DARRAYELEM2fromCodeTM );

/*
1.16 Operator totasks

The totasks operator exports all the tasks of a code object into a stream.

1.16.1 Type Mapping

This operator has a code object as arguments. The output is a stream of tasks.

*/
ListExpr  toTasksTM(ListExpr args){
  string err = "Code object  expected";

  if (nl->ListLength(args) != 1)
    return listutils::typeError(err);

  if(!Code::checkType(nl->First(args))){
    return listutils::typeError(err);    
  }

   //The mapping of the  stream(tuple)
  ListExpr attr9 = nl->TwoElemList( nl->SymbolAtom("ResPart"),
                                     nl->SymbolAtom(CcInt::BasicType()));
  ListExpr attrList = nl->OneElemList(attr9);

  ListExpr attr8 = nl->TwoElemList( nl->SymbolAtom("Res"),
                                      nl->SymbolAtom(CcString::BasicType()));
  attrList = nl->Cons(attr8, attrList);

  ListExpr attr7 = nl->TwoElemList( nl->SymbolAtom("Query"),
                                    nl->SymbolAtom(CcString::BasicType()));
  attrList = nl->Cons(attr7, attrList);
  
  ListExpr attr6 = nl->TwoElemList( nl->SymbolAtom("Part2"),
                                     nl->SymbolAtom(CcInt::BasicType()));
  attrList = nl->Cons(attr6, attrList);
  
  ListExpr attr5 = nl->TwoElemList( nl->SymbolAtom("Part1"),
                                     nl->SymbolAtom(CcInt::BasicType()));
  attrList = nl->Cons(attr5, attrList);
  
  ListExpr attr4 = nl->TwoElemList( nl->SymbolAtom("Arg2"),
                                      nl->SymbolAtom(CcString::BasicType()));
  attrList = nl->Cons(attr4, attrList);
  
  ListExpr attr3 = nl->TwoElemList( nl->SymbolAtom("Arg1"),
                                      nl->SymbolAtom(CcString::BasicType()));
  attrList = nl->Cons(attr3, attrList);
  
  ListExpr attr2 = nl->TwoElemList( nl->SymbolAtom("Dep"),
                                     nl->SymbolAtom(CcInt::BasicType()));
  attrList = nl->Cons(attr2, attrList);
  
  ListExpr attr1 = nl->TwoElemList( nl->SymbolAtom("Task"),
                                     nl->SymbolAtom(CcInt::BasicType()));
  attrList = nl->Cons(attr1, attrList);

  ListExpr returnList = nl->TwoElemList(
             listutils::basicSymbol<Stream<Tuple> >(),
              nl->TwoElemList(
                 listutils::basicSymbol<Tuple>(),
                  attrList));  
  return returnList;
}


/*
1.16.2 Value Mapping

*/

int toTasksVM( Word* args, Word& result, int message,
                  Word& local, Supplier s ){
  TupleType* tt = new TupleType(nl->Second(GetTupleResultType(s)));
  Code* c = (Code*)args[0].addr;
  switch (message) {
    case OPEN: {
      c->BeginIteration();
      return 0;
    }
    case REQUEST: {
      TaskElement* task = c->GetNext();
      if(task != 0){
        result.addr = task->getTuple(tt);
      }else{
        result.addr = 0;
      }
      return result.addr ? YIELD : CANCEL;
    }
    case CLOSE: {
      return 0;
    }
  }
  return 0;
}

/*
1.16.3 Specification

*/
OperatorSpec toTasksSpec(
     "Code(darray(X)) -> stream(tuble)",
     "_ toTasks",
     "The elements of a Code object are exported in a stream. ",
     "query c toTasks"
); 


/*
1.16.4 Operator instance


*/
Operator toTasksOp (
    "toTasks",                    //name
    toTasksSpec.getStr(),         //specification
    toTasksVM,                    //value mapping 
    Operator::SimpleSelect,        //trivial selection function
    toTasksTM                     //type mapping
);



/*
1.16 Operator schedule

The schedule operator reads all the tasks to be performed into an internal
data structure. It then distributes tasks to servers. When a server informs 
the scheduler that a task is finished, the scheduler selects another task 
and assigns it to this server.
The scheduler may also listen to heartbeat or progress messages of servers.
The list of tasks described by the incoming tuple stream may contain dependencies
so that a certain task can only be started when a specified other task has been 
finished. In this way the overlapping processing of successive operations 
can be controlled.

1.16.1 Type Mapping

This operator has 2 arguments. Input is a stream of tuples describing tasks
to be performed by Secondo servers. 
The second argument to schedule is the set of available Secondo servers.

*/
ListExpr  scheduleTM(ListExpr args){
  string lenErr = "2 parameters expected";
  string newErr = "stream(tuple) x stream(tuple) expected";
  string mapErr = "It's not a map ";

  if (nl->ListLength(args) != 2)
    return listutils::typeError(lenErr);

  ListExpr stream1 = nl->First(args);
  ListExpr stream2 = nl->Second(args);

  if (!listutils::isTupleStream(stream1))
    return listutils::typeError(newErr);

  if (!listutils::isTupleStream(stream2))
    return listutils::typeError(newErr);

//Stream  1:  
//The Tasklist, that should be scheduled  
//That could be the output of the dloop2schedule operator. 
  string att;
  int index;
  ListExpr type;
  ListExpr attrList = nl->Second(nl->Second(stream1));

  att = "Task";
  index = listutils::findAttribute(attrList, att, type);
  if(!index){
    return listutils::typeError("Attribute " + att + 
                                " in the first streem not found");
  }
  if(!CcInt::checkType(type)){
     return listutils::typeError("Attribute " + att + 
                                 " not of type int");
  }  

  att = "Dep";
  index = listutils::findAttribute(attrList, att, type);
  if(!index){
    return listutils::typeError("Attribute " + att + 
                                " in the first streem not found");
  }
  if(!CcInt::checkType(type)){
     return listutils::typeError("Attribute " + att + 
                                 " not of type int");
  }  

  att = "Arg1";
  index = listutils::findAttribute(attrList, att, type);
  if(!index){
    return listutils::typeError("Attribute " + att + 
                                " in the first streem not found");
  }
  if(!CcString::checkType(type) && !FText::checkType(type)){
     return listutils::typeError("Attribute " + att + 
                                 " not of type string");
  }  

  att = "Arg2";
  index = listutils::findAttribute(attrList, att, type);
  if(!index){
    return listutils::typeError("Attribute " + att + 
                                " in the first streem not found");
  }
  if(!CcString::checkType(type) && !FText::checkType(type)){
     return listutils::typeError("Attribute " + att + 
                                 " not of type string");
  }  

  att = "Part1";
  index = listutils::findAttribute(attrList, att, type);
  if(!index){
    return listutils::typeError("Attribute " + att + 
                                " in the first streem not found");
  }
  if(!CcInt::checkType(type)){
     return listutils::typeError("Attribute " + att + 
                                 " not of type int");
  }  

  att = "Part2";
  index = listutils::findAttribute(attrList, att, type);
  if(!index){
    return listutils::typeError("Attribute " + att + 
                                " in the first streem not found");
  }
  if(!CcInt::checkType(type)){
     return listutils::typeError("Attribute " + att + 
                                 " not of type int");
  }  

  att = "Query";
  index = listutils::findAttribute(attrList, att, type);
  if(!index){
    return listutils::typeError("Attribute " + att + 
                                " in the first streem not found");
  }
  if(!CcString::checkType(type) && !FText::checkType(type)){
     return listutils::typeError("Attribute " + att + 
                                 " not of type string");
  }  

  att = "Res";
  index = listutils::findAttribute(attrList, att, type);
  if(!index){
    return listutils::typeError("Attribute " + att + 
                                " in the first streem not found");
  }
  if(!CcString::checkType(type) && !FText::checkType(type)){
     return listutils::typeError("Attribute " + att + 
                                 " not of type string");
  }  

  att = "ResPart";
  index = listutils::findAttribute(attrList, att, type);
  if(!index){
    return listutils::typeError("Attribute " + att + 
                                " in the first streem not found");
  }
  if(!CcInt::checkType(type)){
     return listutils::typeError("Attribute " + att + 
                                 " not of type int");
  }  
  
//Stream2:
//The set of avaiable Secondo server.
//That could be the output of the checkConnection operator. 

  attrList = nl->Second(nl->Second(stream2));
  att = "Id";
  index = listutils::findAttribute(attrList, att, type);
  if(!index){
    return listutils::typeError("Attribute " + att + 
                                " in the second streem not found");
  }
  if(!CcInt::checkType(type)){
     return listutils::typeError("Attribute " + att + 
                                 " not of type int");
  }  

  att = "Host";
  index = listutils::findAttribute(attrList, att, type);
  if(!index){
    return listutils::typeError("Attribute " + att + 
                                " in the second streem not found");
  }
  if(!CcString::checkType(type) && !FText::checkType(type)){
     return listutils::typeError("Attribute " + att + 
                                 " not of type string");
  }  

  att = "Port";
  index = listutils::findAttribute(attrList, att, type);
  if(!index){
    return listutils::typeError("Attribute " + att + 
                                " in the second streem not found");
  }
  if(!CcInt::checkType(type)){
     return listutils::typeError("Attribute " + att + 
                                 " not of type string");
  }  

  att = "ConfigFile";
  index = listutils::findAttribute(attrList, att, type);
  if(!index){
    return listutils::typeError("Attribute " + att + 
                                " in the second streem not found");
  }
  if(!CcString::checkType(type) && !FText::checkType(type)){
     return listutils::typeError("Attribute " + att + 
                                 " not of type string");
  }  

  att = "OK";
  index = listutils::findAttribute(attrList, att, type);
  if(!index){
    return listutils::typeError("Attribute " + att + 
                                " in the second streem not found");
  }
  if(!CcBool::checkType(type) && !FText::checkType(type)){
     return listutils::typeError("Attribute " + att + 
                                 " not of type bool");
  }  

  att = "PID";
  index = listutils::findAttribute(attrList, att, type);
  if(!index){
    return listutils::typeError("Attribute " + att + 
                                " in the second streem not found");
  }
  if(!CcInt::checkType(type)){
     return listutils::typeError("Attribute " + att + 
                                 " not of type int");
  }  
  
  //returns 0 ?
  return nl->SymbolAtom(CcInt::BasicType());
}


/*
1.16.2 Value Mapping

*/



int scheduleVM( Word* args, Word& result, int message,
                  Word& local, Supplier s ){

  qp->Open(args[0].addr); // open the argument stream

  //Straem 1
  Stream<Tuple> streamTasks(args[0]);
  streamTasks.open();

  Tuple* nextTask = streamTasks.request();
  int countTasks =0;
  while(nextTask!=0){
    countTasks++;
    nextTask->DeleteIfAllowed();
    nextTask = streamTasks.request();
  }
  streamTasks.close();

  Stream<Tuple> streamServer(args[0]);
  streamServer.open();

  //Stream 2
  Tuple* nextServer = streamServer.request();
  int countServer =0;
  while(nextServer!=0){
    countServer++;
    nextServer->DeleteIfAllowed();
    nextServer = streamServer.request();
  }
  streamServer.close();
  return 0;
}

/*
1.16.3 Specification

*/
OperatorSpec scheduleSpec(
     "stream(tuble) x stream(tuble) -> darray(Z)",
     "_ _ schedule",
     "The first stream contains the tasks, the second the Server. "
     "The name of the resulting darray is specifyed with the tasks.", 
     "query st1 st2 schedule "
); 


/*
1.16.4 Operator instance


*/
Operator scheduleOp (
    "schedule",                    //name
    scheduleSpec.getStr(),         //specification
    scheduleVM,                    //value mapping 
    Operator::SimpleSelect,        //trivial selection function
    scheduleTM                     //type mapping
);



/*
3 Implementation of the Algebra

*/
Distributed3Algebra::Distributed3Algebra(){

   AddTypeConstructor(&CodeTC);
   CodeTC.AssociateKind(Kind::SIMPLE());
   

   AddOperator(&dloop3Op);
   dloop3Op.SetUsesArgsInTypeMapping();

   AddOperator(&toTasksOp);

   AddOperator(&DARRAYELEMfromCodeOp);
   AddOperator(&DARRAYELEM2fromCodeOp);
   
   //AddOperator(&scheduleOp);

}


Distributed3Algebra::~Distributed3Algebra(){
}


} // end of namespace distributed3

extern "C"
Algebra*
   InitializeDistributed3Algebra( NestedList* nlRef,
                             QueryProcessor* qpRef,
                             AlgebraManager* amRef ) {

   distributed3::algInstance = new distributed3::Distributed3Algebra();
   //distributed3::showCommands = false;   
   return distributed3::algInstance;
}


