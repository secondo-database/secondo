/*
----
This file is part of SECONDO.

Copyright (C) 2009, University in Hagen
Faculty of Mathematic and Computer Science,
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

*/


#include "TBTree.h"

#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "RelationAlgebra.h"
#include "FTextAlgebra.h"


extern NestedList* nl;
extern QueryProcessor* qp;



namespace tbtree {

/*
1 Type Contructor


*/


ListExpr TBTreeProperty(){
  return nl->TwoElemList(
             nl->FiveElemList(
                 nl->StringAtom("Signature"),
                 nl->StringAtom("Example Type List"),
                 nl->StringAtom("List Rep"),
                 nl->StringAtom("Example List"),
                 nl->StringAtom("Remarks")),
             nl->FiveElemList(
                nl->TextAtom(" -> SIMPLE"),
                nl->TextAtom(" (tbtree attrlist ai aj)"),
                nl->TextAtom("(tbtree attrlist idname unitname)"),
                nl->TextAtom("(rbtree ((id int) (up  upoint)) id up)"),
                nl->TextAtom(" alpha state"))
         );
}


bool CheckTBTree(ListExpr type, ListExpr& ErrorInfo){

   if(nl->ListLength(type)!=4){
      ErrorReporter::ReportError("invalid list length");
      return false;
   }
   ListExpr tbtree = nl->First(type);
   ListExpr attrList = nl->Second(type);
   ListExpr idName = nl->Third(type);
   ListExpr upName = nl->Fourth(type);

   if(!nl->IsEqual(tbtree,"tbtree")){
      ErrorReporter::ReportError("symbol tbtree not found");
      return false;
   }
   if(!listutils::isAttrList(attrList)){
      ErrorReporter::ReportError("not an attrlist");
      return false;
   }
   
   if(nl->AtomType(idName)!=SymbolType || nl->AtomType(upName)!=SymbolType ){
      ErrorReporter::ReportError("invalid attribute names");
      return false;
   }

   string idname = nl->SymbolValue(idName);
   string upname = nl->SymbolValue(upName);

   ListExpr dummy;
   if(!listutils::findAttribute(attrList, idname, dummy)){
      ErrorReporter::ReportError("idname not in attrlist");
      return false;
   }
   if(!listutils::findAttribute(attrList, upname, dummy)){
      ErrorReporter::ReportError("upname not in attrlist");
      return false;
   }
   return true;
}


ListExpr OutTBTree(ListExpr typeInfo, Word value){
   TBTree* t = static_cast<TBTree*>(value.addr);
   return nl->TextAtom(t->toString());
}

Word InTBTree( ListExpr typeInfo, ListExpr value,
               int errorPos, ListExpr& errorInfo, bool& correct ) {
   correct = false;
   return SetWord(Address(0));
}

Word CreateTBTree( const ListExpr typeInfo ){
   return new TBTree(4000);
}

void DeleteTBTree( const ListExpr typeInfo, Word& w ){
   TBTree* tree = static_cast<TBTree*>(w.addr);
   tree->deleteFile();
   delete tree;
}

bool OpenTBTree( SmiRecord& valueRecord,
                 size_t& offset,
                 const ListExpr typeInfo,
                 Word& value ){
  SmiFileId fileid;
  valueRecord.Read( &fileid, sizeof( SmiFileId ), offset );
  offset += sizeof( SmiFileId );
  SmiRecordId rid;
  valueRecord.Read( &rid, sizeof( SmiRecordId ), offset );
  offset += sizeof( SmiRecordId );
  TBTree* tree = new TBTree(fileid, rid);
  value.setAddr(tree); 
  return true;
}

void CloseTBTree( const ListExpr typeInfo, Word& w ){
   TBTree* tree = static_cast<TBTree*>(w.addr);
   delete tree;
   w.setAddr(0);
}

bool SaveTBTree( SmiRecord& valueRecord,
                 size_t& offset,
                 const ListExpr typeInfo,
                 Word& value ){
   TBTree* tree = static_cast<TBTree*>(value.addr);
   SmiFileId fileId = tree->getFileId();
   valueRecord.Write( &fileId, sizeof( SmiFileId ), offset );
   offset += sizeof( SmiFileId );
   SmiRecordId headerId = tree->getHeaderId();
   valueRecord.Write(&headerId, sizeof(SmiRecordId), offset);
   offset += sizeof( SmiRecordId );
   return true;
}


Word CloneTBTree( const ListExpr typeInfo, const Word& w ){
   return SetWord( Address(0) );
}

void* CastTBTree( void* addr) {
   return ( 0 );
}

int SizeOfTBTree() {
  return 0;
}




TypeConstructor tbtreetc( "tbtree",
                         TBTreeProperty,
                         OutTBTree,
                         InTBTree,
                         0,
                         0,
                         CreateTBTree,
                         DeleteTBTree,
                         OpenTBTree,
                         SaveTBTree,
                         CloseTBTree,
                         CloneTBTree,
                         CastTBTree,
                         SizeOfTBTree,
                         CheckTBTree );


/*
2 Operators

*/

/*
2.1 ~createtbtree~

2.1.1 Type Mapping


   rel(tuple(... (int id) ... (up upoint) ...) x id x up -> tbtree(...)


*/

ListExpr createtbtreeTM(ListExpr args){
  
  if(nl->ListLength(args)!=3){
    ErrorReporter::ReportError("wong number of arguments (3 required)");
    return nl->TypeError();
  }
  ListExpr rel  = nl->First(args);
  ListExpr id = nl->Second(args);
  ListExpr up = nl->Third(args); 

  if(!listutils::isRelDescription(rel)){
    ErrorReporter::ReportError("first argument must be an tuple rel");
    return nl->TypeError();
  }

  if(nl->AtomType(id)!=SymbolType || nl->AtomType(up)!=SymbolType){
    ErrorReporter::ReportError("second and third argument must be an "
                               "attribute name ");
    return nl->TypeError();
  }

  ListExpr type;
  ListExpr attrList = nl->Second(nl->Second(rel));
  string ids = nl->SymbolValue(id);
  int index1;
  if(!(index1=listutils::findAttribute(attrList,ids, type))){
    ErrorReporter::ReportError("attribute " +ids  +
                               "not an attribute of the relation ");
    return nl->TypeError();
  }
  if(!nl->IsEqual(type,"int")){
    ErrorReporter::ReportError("attribute " +ids  +
                               " has to be an int");
    return nl->TypeError();
  }
  
  string ups = nl->SymbolValue(up);
  int index2;
  if(!(index2=listutils::findAttribute(attrList,ups, type))){
    ErrorReporter::ReportError("attribute " +ups  +
                               "not an attribute of the relation ");
    return nl->TypeError();
  }
  if(!nl->IsEqual(type,"upoint")){
    ErrorReporter::ReportError("attribute " +ups  +
                               " has to be a upoint");
    return nl->TypeError();
  }

  ListExpr tree =  nl->FourElemList( 
                        nl->SymbolAtom("tbtree"),
                        attrList,
                        id,
                        up);
   return nl->ThreeElemList(
                nl->SymbolAtom("APPEND"),
                nl->TwoElemList(
                     nl->IntAtom(index1),
                     nl->IntAtom(index2)),
                tree);
}


/*
2.1.2 Specification

*/
const string createtbtreeSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Comment\" ) "
    "(<text>rel(tuple(...(id int)...(up upoint)...) x "
    "id x up -> tbtree(...)</text--->"
    "<text> _ createtbtree[_ _]</text--->"
    "<text>creates a tbtree from the elements in the relation </text--->"
    "<text>query UnitTrains createtbtree[Id, UTrip] </text--->"
    "<text>The elements in the relation must be in temporal"
    " order for a single trip</text--->"
    ") )";

/*
2.1.3 Value Mapping

*/
int createtbtreeVM(Word* args, Word& result, int message,
                   Word& local, Supplier s){

   result = qp->ResultStorage(s);
   TBTree* tree = static_cast<TBTree*>(result.addr);
   Relation* rel = static_cast<Relation*>(args[0].addr);
   GenericRelationIterator* iter = rel->MakeScan();
   int index1 = (static_cast<CcInt*>(args[3].addr))->GetIntval() - 1; 
   int index2 = (static_cast<CcInt*>(args[4].addr))->GetIntval() - 1;
   Tuple* tuple; 
   while((tuple = iter->GetNextTuple())){
     CcInt* id  = static_cast<CcInt*>(tuple->GetAttribute(index1));
     UPoint* up = static_cast<UPoint*>(tuple->GetAttribute(index2));
     TupleId tid = tuple->GetTupleId();
     if(id->IsDefined() && up->IsDefined() && tid){
        tree->insert(*up,id->GetIntval(), tid);
     } else {
        std::cerr << "undefined id, up or invalid tuple id" << endl;
     }
     tuple->DeleteIfAllowed(); 
   }
   delete iter; 
   return 0;   
}

/*
2.1.4 Operator instance

*/
Operator createtbtree (
         "createtbtree",            // name
          createtbtreeSpec,          // specification
          createtbtreeVM,           // value mapping
          Operator::SimpleSelect, // trivial selection function
          createtbtreeTM);        


/*
2.2  Info operators

2.2.1 Type mapping

*/

ListExpr TBTree2Int(ListExpr args){
   if(nl->ListLength(args)!=1){
     ErrorReporter::ReportError("invalid number of arguments");
     return nl->TypeError();
   }
   ListExpr errorInfo = listutils::emptyErrorInfo();
   if(CheckTBTree(nl->First(args), errorInfo)){
      return nl->SymbolAtom("int");
   }
   ErrorReporter::ReportError("TBTree expected");
   return nl->TypeError();
}


/*
2.2.2 Specifications

*/
const string tbentriesSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Comment\" ) "
    "(<text>tbtree(...) -> int </text--->"
    "<text> tbentries(_) </text--->"
    "<text>returns the number of entries stored in the tb tree </text--->"
    "<text>query tbentries(UnitTrains createtbtree[Id, UTrip]) </text--->"
    "<text></text--->"
    ") )";

const string tbnodesSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Comment\" ) "
    "(<text>tbtree(...) -> int </text--->"
    "<text> tbnodes(_) </text--->"
    "<text>returns the number of nodes stored in the tb tree </text--->"
    "<text>query tbnodes(UnitTrains createtbtree[Id, UTrip]) </text--->"
    "<text></text--->"
    ") )";

const string tbleafnodesSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Comment\" ) "
    "(<text>tbtree(...) -> int </text--->"
    "<text> tbleafnodes(_) </text--->"
    "<text>returns the number of leaf nodes stored in the tb tree </text--->"
    "<text>query tbleafnodes(UnitTrains createtbtree[Id, UTrip]) </text--->"
    "<text></text--->"
    ") )";


const string tblevelSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Comment\" ) "
    "(<text>tbtree(...) -> int </text--->"
    "<text> tblevel(_) </text--->"
    "<text>returns the level of the tb tree </text--->"
    "<text>query level(UnitTrains createtbtree[Id, UTrip]) </text--->"
    "<text></text--->"
    ") )";

/*
2.2.3 Value Mappings

*/


int tbentriesVM(Word* args, Word& result, int message,
                   Word& local, Supplier s){
  TBTree* t = static_cast<TBTree*>(args[0].addr);
  result = qp->ResultStorage(s);
  CcInt* r = static_cast<CcInt*>(result.addr);
  r->Set(true, t->entryCount());
  return 0;
}

int tbnodesVM(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  TBTree* t = static_cast<TBTree*>(args[0].addr);
  result = qp->ResultStorage(s);
  CcInt* r = static_cast<CcInt*>(result.addr);
  r->Set(true, t->nodeCount());
  return 0;
}

int tbleafnodesVM(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  TBTree* t = static_cast<TBTree*>(args[0].addr);
  result = qp->ResultStorage(s);
  CcInt* r = static_cast<CcInt*>(result.addr);
  r->Set(true, t->leafnodeCount());
  return 0;
}


int tblevelVM(Word* args, Word& result, int message,
                 Word& local, Supplier s){
  TBTree* t = static_cast<TBTree*>(args[0].addr);
  result = qp->ResultStorage(s);
  CcInt* r = static_cast<CcInt*>(result.addr);
  r->Set(true, t->getHeight());
  return 0;
}

/*
2.2.4 Operator definitions

*/


Operator tbentries (
         "tbentries",            // name
          tbentriesSpec,          // specification
          tbentriesVM,           // value mapping
          Operator::SimpleSelect, // trivial selection function
          TBTree2Int);        

Operator tbnodes (
       "tbnodes",            // name
        tbnodesSpec,          // specification
        tbnodesVM,           // value mapping
        Operator::SimpleSelect, // trivial selection function
        TBTree2Int);        

Operator tbleafnodes (
       "tbleafnodes",            // name
        tbleafnodesSpec,          // specification
        tbleafnodesVM,           // value mapping
        Operator::SimpleSelect, // trivial selection function
        TBTree2Int);        

Operator tblevel (
       "tblevel",            // name
        tblevelSpec,          // specification
        tblevelVM,           // value mapping
        Operator::SimpleSelect, // trivial selection function
        TBTree2Int);       

/*
2.4 getnodes


2.4.1 Auxiliary class

*/
template<unsigned  Dim>
class AllSelector{
  public:
    bool operator()(BasicNode<Dim> * n) const{
       return true;
    }
};


/*
2.4.2 Type Mapping

*/

ListExpr getnodesTM(ListExpr args){
   if(nl->ListLength(args)!=1){
     ErrorReporter::ReportError("invalid number of arguments");
     return nl->TypeError();
   }
   ListExpr errorInfo = listutils::emptyErrorInfo();
   if(CheckTBTree(nl->First(args), errorInfo)){
      return nl->TwoElemList(
                    nl->SymbolAtom("stream"),
                    nl->TwoElemList(
                        nl->SymbolAtom("tuple"),
                        nl->SixElemList(
                            nl->TwoElemList(
                                nl->SymbolAtom("Id"),
                                nl->SymbolAtom("int")),
                            nl->TwoElemList(
                                nl->SymbolAtom("ParentId"),
                                nl->SymbolAtom("int")),
                            nl->TwoElemList(
                                nl->SymbolAtom("Level"),
                                nl->SymbolAtom("int")),
                            nl->TwoElemList(
                                nl->SymbolAtom("IsLeaf"),
                                nl->SymbolAtom("bool")),
                            nl->TwoElemList(
                                nl->SymbolAtom("Entries"),
                                nl->SymbolAtom("int")),
                            nl->TwoElemList(
                                nl->SymbolAtom("Box"),
                                nl->SymbolAtom("rect3")))));
   }
   ErrorReporter::ReportError("TBTree expected");
   return nl->TypeError();
}

/*
2.4.2 Specification

*/


const string getnodesSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Comment\" ) "
    "(<text>tbtree(...) -> stream(tuple(...)) </text--->"
    "<text> getnodes(_) </text--->"
    "<text>returns the nodes contained in the tree </text--->"
    "<text>query getnodes(UnitTrains createtbtree[Id, UTrip]) count</text--->"
    "<text></text--->"
    ") )";


/*
2.4.3 Value Mapping

2.4.3.1 LocalInfo

*/

class GetnodesLocalInfo{
 public:
  GetnodesLocalInfo(TBTree* t, 
                    const ListExpr tupleType,
                    const AllSelector<3>& s,
                    const NoPruner<InnerNode<3, InnerInfo> >& p): ds(t,s,p){
    tt = new TupleType(tupleType);
  }
  ~GetnodesLocalInfo(){
     tt->DeleteIfAllowed();
     ds.finish();
  }

  DepthSearch<TBTree, 
                      InnerNode<3, InnerInfo>, 
                      3, AllSelector<3> ,
                      NoPruner<InnerNode<3, InnerInfo> >
                     > ds;
  TupleType* tt;

};

/*
2.4.3.1 Value Mapping function 

*/

int getnodesVM(Word* args, Word& result, int message,
                   Word& local, Supplier s){

   switch (message){
      case OPEN:{
        if(local.addr){
          delete static_cast<GetnodesLocalInfo*>(local.addr);
        }
        AllSelector<3> as;
         TBTree* t = static_cast<TBTree*>(args[0].addr);
         NoPruner<InnerNode<3, InnerInfo> > p;
         local.addr = new GetnodesLocalInfo(t, 
                          nl->Second(GetTupleResultType(s)), as,p );
         return 0;   
      }

      case REQUEST: {
         if(!local.addr){
           return CANCEL;
         }
         GetnodesLocalInfo* li = static_cast<GetnodesLocalInfo*>(local.addr);
         IteratorElement<3> next;
         if(!li->ds.next(next)){
            return CANCEL;
         }
         Tuple* res = new Tuple(li->tt);
         res->PutAttribute(0, new CcInt(true, next.getOwnId()));
         res->PutAttribute(1, new CcInt(true, next.getParentId())); 
         res->PutAttribute(2, new CcInt(true, next.getLevel()));
         res->PutAttribute(3, new CcBool(true, next.getNode()->isLeaf()));
         res->PutAttribute(4, new CcInt(true, next.getNode()->entryCount()));
         res->PutAttribute(5, new Rectangle<3>(next.getNode()->getBox()));
         next.deleteNode();
         result.setAddr(res);
         return YIELD;
      }

      case CLOSE: {
        if(local.addr){
          delete static_cast<GetnodesLocalInfo*>(local.addr);
          local.setAddr(0);
        }
        return 0;
      }
      default: assert(false);
               return 0;
   }

}

/*
2.4.4 Operator Instance

*/



Operator getnodes (
       "getnodes",            // name
        getnodesSpec,          // specification
        getnodesVM,           // value mapping
        Operator::SimpleSelect, // trivial selection function
        getnodesTM);       


/*
2.5. Operator getFileInfo

2.5.1 Type Mapping


*/
ListExpr TBTree2Text(ListExpr args){
   if(nl->ListLength(args)!=1){
     ErrorReporter::ReportError("invalid number of arguments");
     return nl->TypeError();
   }
   ListExpr errorInfo = listutils::emptyErrorInfo();
   if(CheckTBTree(nl->First(args), errorInfo)){
      return nl->SymbolAtom("text");
   }
   ErrorReporter::ReportError("TBTree expected");
   return nl->TypeError();
}

/*
2.5.2 Specification

*/

const string getFileInfoSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Comment\" ) "
    "(<text>tbtree(...) -> text </text--->"
    "<text> getFileIndo(_) </text--->"
    "<text> returns information about the underlying file </text--->"
    "<text>query getFileInfo(UnitTrains createtbtree[Id, UTrip]) </text--->"
    "<text></text--->"
    ") )";

/*
2.5.2 Value Mapping

*/
int getFileInfoVM(Word* args, Word& result, int message,
                        Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  FText* restext = static_cast<FText*>(result.addr);
  TBTree*  tree = (TBTree*)(args[0].addr);
  SmiStatResultType resVector(0);
  if ( (tree != 0) && tree->getFileStats(resVector) ){
    string resString = "[[\n";
    for(SmiStatResultType::iterator i = resVector.begin();
        i != resVector.end(); ){
      resString += "\t[['" + i->first + "'],['" + i->second + "']]";
      if(++i != resVector.end()){
        resString += ",\n";
      } else {
        resString += "\n";
      }
    }
    resString += "]]";
    restext->Set(true, resString);
  } else {
    restext->Set(false,"");
  }
  return 0;
};

/*
2.5.2 Operator Instance

*/
Operator getFileInfo (
       "getFileInfo",            // name
        getFileInfoSpec,          // specification
        getFileInfoVM,           // value mapping
        Operator::SimpleSelect, // trivial selection function
        TBTree2Text);       
 

/*
2.6 getEntries


2.4.1 Auxiliary class

*/
template<unsigned  Dim>
class LeafSelector{
  public:
    bool operator()(BasicNode<Dim> * n) const{
       return n->isLeaf();
    }
};


/*
2.6.2 Type Mapping

*/

ListExpr getentriesTM(ListExpr args){
   if(nl->ListLength(args)!=1){
     ErrorReporter::ReportError("invalid number of arguments");
     return nl->TypeError();
   }
   ListExpr errorInfo = listutils::emptyErrorInfo();
   if(CheckTBTree(nl->First(args), errorInfo)){
      return nl->TwoElemList(
                    nl->SymbolAtom("stream"),
                    nl->TwoElemList(
                        nl->SymbolAtom("tuple"),
                        nl->ThreeElemList(
                            nl->TwoElemList(
                                nl->SymbolAtom("TupleId"),
                                nl->SymbolAtom("int")),
                            nl->TwoElemList(
                                nl->SymbolAtom("TrjId"),
                                nl->SymbolAtom("int")),
                            nl->TwoElemList(
                                nl->SymbolAtom("Box"),
                                nl->SymbolAtom("rect3")))));
   }
   ErrorReporter::ReportError("TBTree expected");
   return nl->TypeError();
}

/*
2.6.2 Specification

*/


const string getentriesSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Comment\" ) "
    "(<text>tbtree(...) -> stream(tuple(...)) </text--->"
    "<text> getentries(_) </text--->"
    "<text>returns the entries contained in the tree </text--->"
    "<text>query getentries(UnitTrains createtbtree[Id, UTrip]) count</text--->"
    "<text></text--->"
    ") )";


/*
2.6.3 Value Mapping

2.6.3.1 LocalInfo

*/

class GetentriesLocalInfo{
 public:
  GetentriesLocalInfo(TBTree* t, 
                    const ListExpr tupleType,
                    const LeafSelector<3>& s,
                    const NoPruner<InnerNode<3, InnerInfo> >& p): 
    ds(t,s,p), node(0), pos(0){

    tt = new TupleType(tupleType);
  }
  ~GetentriesLocalInfo(){
     tt->DeleteIfAllowed();
     ds.finish();
     if(node){
         delete node;
     }
  }

  Tuple* next(){
     if(!node){
        getNextNode();
     } else if(pos==node->entryCount()){
        getNextNode();
     }
     if(!node){
        return 0;
     } else {
         const Entry<3, TBLeafInfo>* e = node->getEntry(pos);
         pos++;
         Tuple* res = new Tuple(tt);
         res->PutAttribute(0, new CcInt(true, e->getInfo().getTupleId()));
         res->PutAttribute(1, new CcInt(true, node->getTrjId())); 
         res->PutAttribute(2, new Rectangle<3>(e->getBox()));
         return res; 
     }
  } 

  private:
    void getNextNode(){
       if(node){
           delete node;
           node = 0;
       }
       pos = 0;
       IteratorElement<3> next;
       if(ds.next(next)){
          assert(next.getNode()->isLeaf());
          node = dynamic_cast<TBLeafNode<3, 
                       TBLeafInfo>*>(next.getNode());
          pos = 0; 
        }
    }


  DepthSearch<TBTree, 
                      InnerNode<3, InnerInfo>, 
                      3, LeafSelector<3>,
                      NoPruner<InnerNode<3, InnerInfo> >
                     > ds;
  TupleType* tt;
  TBLeafNode<3, TBLeafInfo>* node;
  int pos;

};

/*
2.6.3.1 Value Mapping function 

*/

int getentriesVM(Word* args, Word& result, int message,
                   Word& local, Supplier s){

   switch (message){
      case OPEN:{
        if(local.addr){
          delete static_cast<GetentriesLocalInfo*>(local.addr);
        }
        LeafSelector<3> as;
         TBTree* t = static_cast<TBTree*>(args[0].addr);
         NoPruner<InnerNode<3, InnerInfo> > p;
         local.addr = new GetentriesLocalInfo(t, 
                          nl->Second(GetTupleResultType(s)), as, p);
         return 0;   
      }

      case REQUEST: {
         if(!local.addr){
           return CANCEL;
         }
         GetentriesLocalInfo* li = 
               static_cast<GetentriesLocalInfo*>(local.addr);
         Tuple* res = li->next();
         result.addr = res;
         return res?YIELD: CANCEL;
      }

      case CLOSE: {
        if(local.addr){
          delete static_cast<GetentriesLocalInfo*>(local.addr);
          local.setAddr(0);
        }
        return 0;
      }
      default: assert(false);
               return 0;
   }

}

/*
2.6.4 Operator Instance

*/



Operator getentries (
       "getentries",            // name
        getentriesSpec,          // specification
        getentriesVM,           // value mapping
        Operator::SimpleSelect, // trivial selection function
        getentriesTM);       



/*
2.7 windowIntersectsS


2.7.1 Type Mapping

*/

ListExpr windowintersectsSTM(ListExpr args){
   if(nl->ListLength(args)!=2){
     ErrorReporter::ReportError("invalid number of arguments");
     return nl->TypeError();
   }
   if(!nl->IsEqual(nl->Second(args), "rect") &&
      !nl->IsEqual(nl->Second(args), "rect3")){
     ErrorReporter::ReportError("second argument must be a rectangle");
     return nl->TypeError(); 
   }
   ListExpr errorInfo = listutils::emptyErrorInfo();
   if(CheckTBTree(nl->First(args), errorInfo)){
      return nl->TwoElemList(
                    nl->SymbolAtom("stream"),
                    nl->TwoElemList(
                       nl->SymbolAtom("tuple"),
                       nl->OneElemList(
                            nl->TwoElemList(
                              nl->SymbolAtom("id"),
                              nl->SymbolAtom("tid")))));

   }
   ErrorReporter::ReportError("TBTree expected as first arg");
   return nl->TypeError();
}

/*
2.6.2 Specification

*/


const string windowintersectsSSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Comment\" ) "
    "(<text>tbtree(...) x rect | rect3 -> stream(tuple((id tid))) </text--->"
    "<text> _ windowintersectsS[_]</text--->"
    "<text>Returns the ids of the entries intersecting w </text--->"
    "<text>query (UnitTrains createtbtree[Id, UTrip]) "
          "windowinterscetsS[b] count</text--->"
    "<text></text--->"
    ") )";


/*
2.7.3 Value Mapping

2.7.3.1 LocalInfo

*/

class WindowintersectsSLocalInfo{
 public:
  WindowintersectsSLocalInfo(TBTree* t, 
                    const LeafSelector<3>& s,
                    const IntersectsPruner<3, InnerNode<3, InnerInfo> >& p,
                    const ListExpr ttlist,
                    Relation* rel1 = 0) 
                    : ds(t,s,p), node(0), pos(0), rel(rel1) {
    tt = new TupleType(ttlist);
    rect = p.getBox();
  }
  ~WindowintersectsSLocalInfo(){
     if(tt){
       tt->DeleteIfAllowed();
     }
     ds.finish();
     if(node){
         delete node;
     }
  }

  Tuple* next(){
     assert(rel==0);
     if(!node){
        getNextNode();
     } 
     while(node){
       while(pos<node->entryCount()){
          const Entry<3, TBLeafInfo>* e = node->getEntry(pos);     
          pos++;
          if(rect.Intersects(e->getBox())){
             Tuple* res = new Tuple(tt);
             res->PutAttribute(0, new TupleIdentifier(true,
                                  e->getInfo().getTupleId()));
             return res;
          }
       } 
       getNextNode();
     }
     return 0; 
  } 

  Tuple* nextTuple(){
     assert(rel);
     if(!node){
        getNextNode();
     } 
     while(node){
       while(pos<node->entryCount()){
          const Entry<3, TBLeafInfo>* e = node->getEntry(pos);     
          pos++;
          if(rect.Intersects(e->getBox())){
             Tuple* res = rel->GetTuple(e->getInfo().getTupleId());
             return res;
          }
       } 
       getNextNode();
     }
     return 0; 
  } 



  private:
    void getNextNode(){
       if(node){
           delete node;
           node = 0;
       }
       pos = 0;
       IteratorElement<3> next;
       if(ds.next(next)){
          assert(next.getNode()->isLeaf());
          node = dynamic_cast<TBLeafNode<3, 
                       TBLeafInfo>*>(next.getNode());
        }
    }


  DepthSearch<TBTree, 
                      InnerNode<3, InnerInfo>, 
                      3, LeafSelector<3>,
                      IntersectsPruner<3, InnerNode<3, InnerInfo> >
                     > ds;
  TBLeafNode<3, TBLeafInfo>* node;
  int pos;
  Relation* rel; 
  Rectangle<3> rect;
  TupleType* tt;

};

/*
2.7.3.2 Value Mapping function 

*/

int windowintersectsS_3dVM(Word* args, Word& result, int message,
                   Word& local, Supplier s){

   switch (message){
      case OPEN:{
        if(local.addr){
          delete static_cast<WindowintersectsSLocalInfo*>(local.addr);
          local.addr=0;
        }
        Rectangle<3>* rect3 = static_cast<Rectangle<3>*>(args[1].addr);
        if(!rect3->IsDefined()){
           return 0;
        }
        LeafSelector<3> as;
        TBTree* t = static_cast<TBTree*>(args[0].addr);
        IntersectsPruner<3, InnerNode<3, InnerInfo> > p(*rect3);
        local.addr = new WindowintersectsSLocalInfo(t, as, p, 
                         nl->Second(GetTupleResultType(s)));
        return 0;   
      }

      case REQUEST: {
         if(!local.addr){
           return CANCEL;
         }
         WindowintersectsSLocalInfo* li = 
               static_cast<WindowintersectsSLocalInfo*>(local.addr);
         Tuple* res = li->next();
         result.addr = res;
         return res?YIELD: CANCEL;
      }

      case CLOSE: {
        if(local.addr){
          delete static_cast<WindowintersectsSLocalInfo*>(local.addr);
          local.setAddr(0);
        }
        return 0;
      }
      default: assert(false);
               return 0;
   }
}

int windowintersectsS_2dVM(Word* args, Word& result, int message,
                   Word& local, Supplier s){

   switch (message){
      case OPEN:{
        if(local.addr){
          delete static_cast<WindowintersectsSLocalInfo*>(local.addr);
          local.addr=0;
        }
        Rectangle<2>* rect = static_cast<Rectangle<2>*>(args[1].addr);
        if(!rect->IsDefined()){
           return 0;
        }
        LeafSelector<3> as;
        TBTree* t = static_cast<TBTree*>(args[0].addr);
        Rectangle<3> box = t->getBox();
        if(!box.IsDefined()){
          return 0;
        } 
        double min[3];
        double max[3];
        min[0] = rect->MinD(0);
        min[1] = rect->MinD(1);
        min[2] = box.MinD(2);
        max[0] = rect->MaxD(0);
        max[1] = rect->MaxD(1);
        max[2] = box.MaxD(2);
        Rectangle<3> rect3(true, min, max); 

        IntersectsPruner<3, InnerNode<3, InnerInfo> > p(rect3);
        local.addr = new WindowintersectsSLocalInfo(t,  as, p, 
                                      nl->Second(GetTupleResultType(s)));
        return 0;   
      }

      case REQUEST: {
         if(!local.addr){
           return CANCEL;
         }
         WindowintersectsSLocalInfo* li = 
               static_cast<WindowintersectsSLocalInfo*>(local.addr);
         Tuple* res = li->next();
         result.addr = res;
         return res?YIELD: CANCEL;
      }

      case CLOSE: {
        if(local.addr){
          delete static_cast<WindowintersectsSLocalInfo*>(local.addr);
          local.setAddr(0);
        }
        return 0;
      }
      default: assert(false);
               return 0;
   }
}

/*
2.7.4 Selection Function

*/

int windowintersectsSSelect(ListExpr args){
  if(nl->IsEqual(nl->Second(args),"rect")){
    return 0;
  } else {
    return 1;
  }
}

/*
2.7.5 Value Mapping Array

*/
ValueMapping windowintersectsSVM[] = {
     windowintersectsS_2dVM,
     windowintersectsS_3dVM};


/*
2.7.6 Operator Instance

*/

Operator windowintersectsS (
       "windowintersectsS",            // name
        windowintersectsSSpec,          // specification
        2,
        windowintersectsSVM,           // value mapping
        windowintersectsSSelect, 
        windowintersectsSTM);       



/*
2.8 Operator getBox


2.8.1 Type mapping

*/
ListExpr getBoxTM(ListExpr args){

   if(nl->ListLength(args)!=1){
     ErrorReporter::ReportError("invalid number of arguments");
     return nl->TypeError();
   }
   ListExpr errorInfo = listutils::emptyErrorInfo();
   if(CheckTBTree(nl->First(args), errorInfo)){
      return nl->SymbolAtom("rect3");
   }
   ErrorReporter::ReportError("TBTree expected");
   return nl->TypeError();
}

/*
2.8.2 Specification

*/
const string getBoxSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Comment\" ) "
    "(<text>tbtree(...) -> rect3 </text--->"
    "<text> getBox(_) </text--->"
    "<text>returns bounding box of the tree </text--->"
    "<text>query getBox(UnitTrains createtbtree[Id, UTrip]) </text--->"
    "<text></text--->"
    ") )";

/*
2.8.3 Value Mapping

*/

int getBoxVM(Word* args, Word& result, int message,
                   Word& local, Supplier s){
  TBTree* t = static_cast<TBTree*>(args[0].addr);
  result = qp->ResultStorage(s);
  Rectangle<3>* r = static_cast<Rectangle<3>*>(result.addr);
  *r = t->getBox();
  return 0;
}

/*
2.8.4 Operator definitions

*/


Operator getBox (
         "getBox",            // name
          getBoxSpec,          // specification
          getBoxVM,           // value mapping
          Operator::SimpleSelect, // trivial selection function
          getBoxTM);        


/*
2.9 windowIntersects


2.9.1 Type Mapping

*/

ListExpr windowintersectsTM(ListExpr args){
   if(nl->ListLength(args)!=3){
     ErrorReporter::ReportError("invalid number of arguments");
     return nl->TypeError();
   }
   if(!nl->IsEqual(nl->Third(args), "rect") &&
      !nl->IsEqual(nl->Third(args), "rect3")){
     ErrorReporter::ReportError("second argument must be a rectangle");
     return nl->TypeError(); 
   }
   ListExpr errorInfo = listutils::emptyErrorInfo();
   if(CheckTBTree(nl->First(args), errorInfo) &&
      listutils::isRelDescription(nl->Second(args))){

      ListExpr tbal  = nl->Second(nl->First(args));
      ListExpr relal = nl->Second(nl->Second(nl->Second(args)));
      if(!nl->Equal(tbal, relal)){
         ErrorReporter::ReportError("types of tbtree and relation differ");
         return nl->TypeError();
      }

      return nl->TwoElemList(
                    nl->SymbolAtom("stream"),
                    nl->TwoElemList(
                       nl->SymbolAtom("tuple"),
                       relal));
   }
   ErrorReporter::ReportError("tbtree x rel x rect | rect3 expected");
   return nl->TypeError();
}

/*
2.9.2 Specification

*/


const string windowintersectsSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Comment\" ) "
    "(<text>tbtree(...) x rel x rect | rect3 -> stream(tuple((...))) </text--->"
    "<text> _ _ windowintersects[_]</text--->"
    "<text>returns the tuple indexed by the tree"
           " intersecting the rectangle</text--->"
    "<text>query  (UnitTrains createtbtree[Id, UTrip]) "
          "UnitTrains windowintersect[b] count</text--->"
    "<text></text--->"
    ") )";


/*
2.9.3 Value Mapping functions 

*/

int windowintersects_3dVM(Word* args, Word& result, int message,
                   Word& local, Supplier s){

   switch (message){
      case OPEN:{
        if(local.addr){
          delete static_cast<WindowintersectsSLocalInfo*>(local.addr);
          local.addr=0;
        }

        Relation* rel = static_cast<Relation*>(args[1].addr);
        Rectangle<3>* rect3 = static_cast<Rectangle<3>*>(args[2].addr);
        if(!rect3->IsDefined()){
           return 0;
        }
        LeafSelector<3> as;
        TBTree* t = static_cast<TBTree*>(args[0].addr);
        IntersectsPruner<3, InnerNode<3, InnerInfo> > p(*rect3);
      
        local.addr = new WindowintersectsSLocalInfo(t, as, p, 
                         nl->Second(GetTupleResultType(s)), 
                         rel);
        return 0;   
      }

      case REQUEST: {
         if(!local.addr){
           return CANCEL;
         }
         WindowintersectsSLocalInfo* li = 
               static_cast<WindowintersectsSLocalInfo*>(local.addr);
         Tuple* res = li->nextTuple();
         result.addr = res;
         return res?YIELD: CANCEL;
      }

      case CLOSE: {
        if(local.addr){
          delete static_cast<WindowintersectsSLocalInfo*>(local.addr);
          local.setAddr(0);
        }
        return 0;
      }
      default: assert(false);
               return 0;
   }
}

int windowintersects_2dVM(Word* args, Word& result, int message,
                   Word& local, Supplier s){

   switch (message){
      case OPEN:{
        if(local.addr){
          delete static_cast<WindowintersectsSLocalInfo*>(local.addr);
          local.addr=0;
        }
        Relation* rel = static_cast<Relation*>(args[1].addr);
        Rectangle<2>* rect = static_cast<Rectangle<2>*>(args[2].addr);
        if(!rect->IsDefined()){
           return 0;
        }
        LeafSelector<3> as;
        TBTree* t = static_cast<TBTree*>(args[0].addr);
        Rectangle<3> box = t->getBox();
        if(!box.IsDefined()){
          return 0;
        } 
        double min[3];
        double max[3];
        min[0] = rect->MinD(0);
        min[1] = rect->MinD(1);
        min[2] = box.MinD(2);
        max[0] = rect->MaxD(0);
        max[1] = rect->MaxD(1);
        max[2] = box.MaxD(2);
        Rectangle<3> rect3(true, min, max); 

        IntersectsPruner<3, InnerNode<3, InnerInfo> > p(rect3);
        local.addr = new WindowintersectsSLocalInfo(t,  as, p, 
                                      nl->Second(GetTupleResultType(s)),
                                      rel);
        return 0;   
      }

      case REQUEST: {
         if(!local.addr){
           return CANCEL;
         }
         WindowintersectsSLocalInfo* li = 
               static_cast<WindowintersectsSLocalInfo*>(local.addr);
         Tuple* res = li->nextTuple();
         result.addr = res;
         return res?YIELD: CANCEL;
      }

      case CLOSE: {
        if(local.addr){
          delete static_cast<WindowintersectsSLocalInfo*>(local.addr);
          local.setAddr(0);
        }
        return 0;
      }
      default: assert(false);
               return 0;
   }
}

/*
2.7.4 Selection Function

*/

int windowintersectsSelect(ListExpr args){
  if(nl->IsEqual(nl->Third(args),"rect")){
    return 0;
  } else {
    return 1;
  }
}

/*
2.7.5 Value Mapping Array

*/
ValueMapping windowintersectsVM[] = {
     windowintersects_2dVM,
     windowintersects_3dVM};


/*
2.7.6 Operator Instance

*/

Operator windowintersects (
       "windowintersects",            // name
        windowintersectsSpec,          // specification
        2,
        windowintersectsVM,           // value mapping
        windowintersectsSelect, 
        windowintersectsTM);       



} // end of namespace tbtree
/*
3 Algebra Creation

*/

class TBTreeAlgebra : public Algebra {
  public:
   TBTreeAlgebra() : Algebra() {
     AddTypeConstructor( &tbtree::tbtreetc );

     AddOperator(&tbtree::createtbtree);
     AddOperator(&tbtree::tbentries);
     AddOperator(&tbtree::tbnodes);
     AddOperator(&tbtree::tbleafnodes);
     AddOperator(&tbtree::tblevel);
     AddOperator(&tbtree::getnodes);
     AddOperator(&tbtree::getFileInfo);
     AddOperator(&tbtree::getentries);
     AddOperator(&tbtree::windowintersectsS);
     AddOperator(&tbtree::getBox);
     AddOperator(&tbtree::windowintersects);

   }
};


extern "C"
Algebra*
  InitializeTBTreeAlgebra( NestedList* nlRef, QueryProcessor* qpRef ) {
     nl = nlRef;
     qp = qpRef;
     return (new TBTreeAlgebra);
}

