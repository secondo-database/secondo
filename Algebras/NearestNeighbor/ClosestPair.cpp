
/*
----
This file is part of SECONDO.

Copyright (C) 2014, University in Hagen, 
Faculty of Mathematic and  Computer Science,
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


#include "StandardTypes.h"
#include "RTreeAlgebra.h"
#include "RelationAlgebra.h"
#include "NestedList.h"
#include "RectangleAlgebra.h"
#include "Algebra.h"
#include "QueryProcessor.h"
#include "ListUtils.h"
#include "Stream.h"

#include <queue>

extern NestedList* nl;
extern QueryProcessor* qp;

/*
  Type mapping

*/
ListExpr closestPairsTM(ListExpr args){

   // arg 1 : rtree 1
   // arg 2 : relation 1
   // arg 3 : rtree 2
   // arg 4 : relation 2
   // arg 5 : k (integer) (how many pairs should be found)#

   string err = "expected: rtree x rel x rtree x rel x int";

   if(!nl->HasLength(args,5)){
      return listutils::typeError(err);
   }
   if(   !R_Tree<2,TupleId>::checkType(nl->First(args)) 
      || !Relation::checkType(nl->Second(args))
      || !R_Tree<2,TupleId>::checkType(nl->Third(args))
      || !Relation::checkType(nl->Fourth(args))
      || !CcInt::checkType(nl->Fifth(args))){
      return listutils::typeError(err);
   }
   ListExpr attrList = listutils::concat(
                          nl->Second(nl->Second(nl->Second(args))),
                          nl->Second(nl->Second(nl->Fourth(args))));
   if(!listutils::isAttrList(attrList)){
     return listutils::typeError("problem in concatenating attribute "
                                 "lists (may be name conflicts)");
   }  
   return nl->TwoElemList(
             listutils::basicSymbol<Stream<Tuple> >(),
             nl->TwoElemList(
                 listutils::basicSymbol<Tuple>(),
                  attrList));
}

/*
Heap Entry for pairs of node entries.

*/

class closestPairEntry{

  public:

    closestPairEntry(R_TreeLeafEntry<2,TupleId>& e1, int _level1,
                     R_TreeLeafEntry<2,TupleId>& e2, int _level2):
      id1(0), rect1(e1.box), tid1(e1.info), level1(_level1),
      id2(0), rect2(e2.box), tid2(e2.info), level2(_level2){
         computeDistances();
    }

    closestPairEntry(R_TreeLeafEntry<2,TupleId>& e1, int _level1,
                     R_TreeInternalEntry<2>& e2, int _level2):
      id1(0), rect1(e1.box), tid1(e1.info),level1(_level1),
      id2(e2.pointer), rect2(e2.box), tid2(0), level2(_level2) {
        computeDistances();
    }
    
    closestPairEntry(R_TreeInternalEntry<2>& e1, int _level1,
                     R_TreeInternalEntry<2>& e2, int _level2):
      id1(e1.pointer), rect1(e1.box), tid1(0), level1(_level1),
      id2(e2.pointer), rect2(e2.box), tid2(0), level2(_level2){
        computeDistances();
    }


    closestPairEntry(R_TreeInternalEntry<2>& e1, int _level1,
                     R_TreeLeafEntry<2,TupleId>& e2, int _level2):
      id1(e1.pointer), rect1(e1.box), tid1(0), level1(_level1),
      id2(0), rect2(e2.box), tid2(e2.info), level2(_level2){
        computeDistances();
    }

    closestPairEntry(closestPairEntry e,
                      R_TreeLeafEntry<2,TupleId>& e2, int _level2):
       id1(e.id1), rect1(e.rect1), tid1(e.tid1), level1(e.level1),
       id2(0), rect2(e2.box), tid2(e2.info), level2(_level2){
        computeDistances();
    }

    closestPairEntry(closestPairEntry e,
                      R_TreeInternalEntry<2>& e2, int _level2):
       id1(e.id1), rect1(e.rect1), tid1(e.tid1), level1(e.level1),
       id2(e2.pointer), rect2(e2.box), tid2(0), level2(_level2){
        computeDistances();
    }
    
    closestPairEntry( R_TreeLeafEntry<2,TupleId>& e1, int _level1,
                      closestPairEntry e):
       id1(0), rect1(e1.box), tid1(e1.info),level1(_level1),
       id2(e.id2), rect2(e.rect2), tid2(e.tid2), level2(e.level2){
        computeDistances();
    }
    
    closestPairEntry( R_TreeInternalEntry<2>& e1, int _level1,
                      closestPairEntry e):
       id1(e1.pointer), rect1(e1.box), tid1(0),level1(_level1),
       id2(e.id2), rect2(e.rect2), tid2(e.tid2), level2(e.level2){
        computeDistances();
    }

   
     

  
    double getMinDist()const {
      return minDist;
    }

    bool isFinal() const{
      return (tid1>0) && (tid2>0);
    }

    bool final1() const{
      return tid1>0;
    }
    
    bool final2() const{
      return tid2>0;
    }

    TupleId getTid1() const{
      return tid1;
    }
    TupleId getTid2() const{
      return tid2;
    }

    SmiRecordId getPointer1() const{
         return id1;
    }
    SmiRecordId getPointer2() const{
         return id2;
    }

    int getLevel1(){
       return level1;
    }

    int getLevel2(){
       return level2;
    }


  private:
    SmiRecordId id1;    // pointer to son
    Rectangle<2> rect1; // mbr
    TupleId tid1;       // pointer to relation
    int level1;
   
    SmiRecordId id2;
    Rectangle<2> rect2;
    TupleId tid2;
    int level2;

    double minDist;
    double minmaxdist;
    double maxmaxdist;


    void computeDistances(){
        minDist=rect1.Distance(rect2);
        maxmaxdist = rect1.QMaxMaxDistance(rect2);
        minmaxdist = rect2.QMinMaxDistance(rect2);
    }


    double dist(double* a1, double*a2, int dim){
       double res = 0;
       for(int i=0;i<dim;i++){
          double d = a1[i] - a2[i];
          res += d*d;
       }
       return res;
    }



};


/*
Comparison function

*/

class closestPairEntryLess{

  public:
     bool operator()(const closestPairEntry& e1,
                     const closestPairEntry& e2){
       return e1.getMinDist() > e2.getMinDist();
     }
};

/*
LocalInfo

*/

class ClosestPairLocalInfo{

  public:

     ClosestPairLocalInfo(R_Tree<2,TupleId>* _t1,
                          Relation* _r1,
                          R_Tree<2,TupleId>* _t2,  
                          Relation* _r2,
                          int _k,
                          ListExpr resType):
     t1(_t1), r1(_r1), t2(_t2), r2(_r2) , k(_k), count(0) {
     tt = new TupleType(resType);
     init();
   }

   Tuple* nextTuple(){
      if(count==k && k>=0) {
         return 0;
      }
      if(heap.empty()){
         return 0;
      }
      return produceNextTuple();
   }



   ~ClosestPairLocalInfo(){
        tt->DeleteIfAllowed();
   }

  private:
     R_Tree<2,TupleId>* t1;
     Relation* r1;
     R_Tree<2,TupleId>* t2;
     Relation* r2;
     int k;
     int count;
     TupleType* tt;
     priority_queue<closestPairEntry, 
                    vector<closestPairEntry> , 
                    closestPairEntryLess> heap;

    void init(){
       count = 0;
       while(!heap.empty()){
          heap.pop();
       }
       R_TreeNode<2,TupleId> root1 = t1->Root();
       R_TreeNode<2,TupleId> root2 = t2->Root();
       insertPair(root1,0, root2,0);
    }


    void insertPair(R_TreeNode<2,TupleId>& node1, int level1,
                    R_TreeNode<2,TupleId>& node2, int level2){

       if(node1.IsLeaf() && node2.IsLeaf()){
          for( int i1=0; i1 < node1.EntryCount(); i1++){
             R_TreeLeafEntry<2,TupleId> e1 = *(node1.GetLeafEntry(i1));
             for( int i2=0; i2<node2.EntryCount(); i2++){
                R_TreeLeafEntry<2,TupleId> e2 = *(node2.GetLeafEntry(i2));
                closestPairEntry entry(e1,level1+1,e2, level2+1);
                  if(!prune(entry)){ 
                      heap.push(entry);
                  }
               }
            }
         } 

         if(node1.IsLeaf() && !node2.IsLeaf()){
            for( int i1=0; i1 < node1.EntryCount(); i1++){
               R_TreeLeafEntry<2,TupleId> e1 = *(node1.GetLeafEntry(i1));
               for( int i2=0; i2<node2.EntryCount(); i2++){
                  R_TreeInternalEntry<2> e2 = *(node2.GetInternalEntry(i2));
                  closestPairEntry entry(e1,level1+1,e2,level2+1);
                  if(!prune(entry)){
                      heap.push(entry);
                  }
               }
            }
        }

        if(!node1.IsLeaf() && node2.IsLeaf()){
            for( int i1=0; i1 < node1.EntryCount(); i1++){
               R_TreeInternalEntry<2> e1 = *(node1.GetInternalEntry(i1));
               for( int i2=0; i2<node2.EntryCount(); i2++){
                  R_TreeLeafEntry<2,TupleId> e2 = *(node2.GetLeafEntry(i2));
                  closestPairEntry entry(e1,level1+1,e2,level2+1);
                  if(!prune(entry)){
                     heap.push(entry);
                  }
               }
            }
         } 

        if(!node1.IsLeaf() && !node2.IsLeaf()){
            for( int i1=0; i1 < node1.EntryCount(); i1++){
               R_TreeInternalEntry<2> e1 = *(node1.GetInternalEntry(i1));
               for( int i2=0; i2<node2.EntryCount(); i2++){
                  R_TreeInternalEntry<2> e2 = *(node2.GetInternalEntry(i2));
                  closestPairEntry entry(e1,level1+1,e2,level2+1);
                  if(!prune(entry)){
                      heap.push(entry);
                  }
               }
            }
        }

      }

      Tuple* produceNextTuple(){
        while(!heap.empty() && !heap.top().isFinal()){
           closestPairEntry e = heap.top();
           heap.pop();
           if(e.final1()){
               R_TreeNode<2,TupleId> n2 = getNode2(e);
               for(int i=0;i<n2.EntryCount();i++){
                  if(n2.IsLeaf()){
                     R_TreeLeafEntry<2,TupleId> 
                        e2 = (R_TreeLeafEntry<2,TupleId>&)n2[i];
                     closestPairEntry entry(e,e2,e.getLevel2()+1);
                     if(!prune(entry)){
                        heap.push(entry);
                     }
                  } else {
                     R_TreeInternalEntry<2> e2 = (R_TreeInternalEntry<2>&)n2[i];
                     closestPairEntry entry(e,e2,e.getLevel2()+1);
                     if(!prune(entry)){
                        heap.push(entry);
                     }
                  }
               }
           }  else if(e.final2()){
               R_TreeNode<2,TupleId> n1 = getNode1(e);
               for(int i=0;i<n1.EntryCount();i++){
                  if(n1.IsLeaf()){
                     R_TreeLeafEntry<2,TupleId> 
                          e1 = (R_TreeLeafEntry<2,TupleId>&)n1[i];
                     closestPairEntry entry(e1,e.getLevel1()+1, e);
                     if(!prune(entry)){
                        heap.push(entry);
                     }
                  } else {
                     R_TreeInternalEntry<2> e1 = (R_TreeInternalEntry<2>&)n1[i];
                     closestPairEntry entry(e1,e.getLevel2()+1, e);
                     if(!prune(entry)){
                        heap.push(entry);
                     }
                  }
               }
           } else { // both are not final
               R_TreeNode<2,TupleId> n1 = getNode1(e);
               R_TreeNode<2,TupleId> n2 = getNode2(e);
               insertPair(n1,e.getLevel1()+1,n2,e.getLevel2()+1);
           }


        }
        if(heap.empty()){ // no more entries
           return 0;
        }
        closestPairEntry e = heap.top();
        heap.pop();
        TupleId tid1 = e.getTid1();
        TupleId tid2 = e.getTid2();
        Tuple* t1 = r1->GetTuple(tid1,true);
        Tuple* t2 = r2->GetTuple(tid2,true);
      
        if(t1==0 || t2==0){
           cerr << "TupleId stored in rtree not found" << endl;
           cerr << " computing closest pair canceled" << endl;
           count = k;
           return 0; 
        }
        Tuple* res = new Tuple(tt);
        Concat(t1,t2,res);
        t1->DeleteIfAllowed();
        t2->DeleteIfAllowed();
        count++;
        return res; 

      }


      R_TreeNode<2,TupleId> getNode1(closestPairEntry& e){
          int level = e.getLevel1() +1;
          bool isLeaf = level == t1->Height();
          if(isLeaf){
              int minEntries = t1->MinLeafEntries();
              int maxEntries = t1->MaxLeafEntries();
              R_TreeNode<2,TupleId> node(isLeaf, minEntries,maxEntries);
              t1->GetNode(e.getPointer1(),node);
              return node;
          } else {
              int minEntries = t1->MinInternalEntries();
              int maxEntries = t1->MaxInternalEntries();
              R_TreeNode<2,TupleId> node(isLeaf, minEntries,maxEntries);
              t1->GetNode(e.getPointer1(),node);
              return node;
          }
      }


      R_TreeNode<2,TupleId> getNode2(closestPairEntry& e){
          int level = e.getLevel2() +1;
          bool isLeaf = level == t2->Height();
          if(isLeaf){
              int minEntries = t2->MinLeafEntries();
              int maxEntries = t2->MaxLeafEntries();
              R_TreeNode<2,TupleId> node(isLeaf, minEntries,maxEntries);
              t2->GetNode(e.getPointer2(),node);
              return node;
          } else {
              int minEntries = t2->MinInternalEntries();
              int maxEntries = t2->MaxInternalEntries();
              R_TreeNode<2,TupleId> node(isLeaf, minEntries,maxEntries);
              t2->GetNode(e.getPointer2(),node);
              return node;
          }
      }

    bool prune(const closestPairEntry& e1) const{
        if(k<0){  // all results required
          return false;
        }
        if(heap.size() < (size_t)(k - count)){ // not enough entries
          return false;
        } 
        // TODO: implement pruing stategies
        return false;
    }




  };


/*
Value Mapping

*/ 



  int closestPairsVM (Word* args, Word& result, int message,
               Word& local, Supplier s){

     
    ClosestPairLocalInfo* li = (ClosestPairLocalInfo*) local.addr;
    switch(message){
      case OPEN: {
          if(li) delete li;
          CcInt* k = (CcInt*) args[4].addr;
          if(k->IsDefined() ){
             local.addr = new ClosestPairLocalInfo(
                   (R_Tree<2,TupleId>*) args[0].addr,
                   (Relation*) args[1].addr,
                   (R_Tree<2,TupleId>*) args[2].addr,
                   (Relation*) args[3].addr,
                    k->GetValue(),
                    nl->Second(GetTupleResultType(s)));
          }
          }
          return 0;
      case REQUEST:
            result.addr=li?li->nextTuple():0;
            return result.addr?YIELD:CANCEL;
      case CLOSE:
           if(li){
               delete li;
               local.addr=0;
           }
    }
    return -1;
  }




