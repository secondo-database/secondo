
/*
----
This file is part of SECONDO.

Copyright (C) 2017, University in Hagen, 
Faculty of Mathematics and  Computer Science,
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


#ifndef RobustSetOpsImpl_H
#define RobustSetOpsImpl_H

#include "RobustSetOps.h"
#include "RegionCreator.h"
#include "Tools/Flob/DbArray.h"

namespace robust{

bool overlaps(const HalfSegment& hs1, const HalfSegment& hs2);

void processParallel(const HalfSegment& hsline,
                     const HalfSegment& hsreg,
                     std::vector<std::pair<double,bool> >& res);

bool computeSplitPoint(const HalfSegment& hs1, 
                       const HalfSegment& hs2,
                       double& delta1,
                       double& delta2);

template<template<typename T>class Array>
void addSplitPoint( const HalfSegment hsline, 
                    const HalfSegment& hsreg , 
                    std::vector<std::pair<double, bool> >& res){
   double lx1 = hsline.GetDomPoint().GetX();
   double ly1 = hsline.GetDomPoint().GetY();
   double lx2 = hsline.GetSecPoint().GetX();
   double ly2 = hsline.GetSecPoint().GetY();

   double rx1 = hsreg.GetDomPoint().GetX();
   double ry1 = hsreg.GetDomPoint().GetY();
   double rx2 = hsreg.GetSecPoint().GetX();
   double ry2 = hsreg.GetSecPoint().GetY();

   double u = lx2-lx1;
   double v = rx1-rx2;
   double w = lx1-rx1;
   double x = ly2-ly1;
   double y = ry1-ry2; 
   double z = ly1-ry1;

   double k = y*u-v*x;


   //cout << "called addSplitPoint" << endl;
   //cout << "hsline = " << hsline.getLineString() << endl;
   //cout << "hsreg = " << hsreg.getLineString() << endl;


   //cout << "k = " << k << endl; 

   if(k==0){
      //cout << "parallel segments found " << endl;
      processParallel(hsline,hsreg,res);
      return; 
   }

   double delta2 = (w*x-z*u) / k;

   //cout << "delta2 = " << delta2 << endl;
   double delta1;
   if(std::abs(u) > std::abs(x)){
      delta1 = -1*((w+delta2*v)/u);
   } else {
      delta1 = -1*((z+delta2*y)/x);
   }   

   //cout << "delta1 = " << delta1 << endl;

   if(delta1<0 || delta2 < 0){  // intersection point outside segments
        return;
   }   
   if(delta1>1 || delta2 > 1){  // intersection point outside splitpart
      return;
   }   

   if(AlmostEqual(delta1,0.0)){ // intersection point is the dominating point
     // no split -> ignore
     return;  
   }
   if(AlmostEqual(delta1,1.0)){
      return;
   }
   bool leftBehindDelta = !RegionCreator<Array>::isRight(hsreg.GetDomPoint(), 
                                                  hsreg.GetSecPoint(), 
                                                  hsline.GetSecPoint()); 
   bool insideLeft = hsreg.insideLeft();

   //cout << "insert delta " << delta1 << endl;
   res.push_back(std::pair<double,bool>(delta1,leftBehindDelta == insideLeft));
}


/*
Compare function.

*/
bool splitpointless(const std::pair<double,bool>& a, 
                    const std::pair<double,bool>& b);


/*
Returns the point relative on hs. delta must be in [0,1] to return a 
Point on the halfsegment.

*/

Point atDelta(const HalfSegment& hs, const double& delta);


/*
Evaluates the splitpoints and creates the halfsegment
parts inside the region.

*/
template<template<typename T>class Array>
void insertLineParts(HalfSegment& hs, 
                     std::vector<std::pair<double,bool> > splitpoints, 
                     const RegionT<Array>& region,
                     LineT<Array>& result,
                     int& edgeno){


    if(splitpoints.empty()){ // segment completely inside or 
      // cout << "No intersection, no problems" << endl;
       // completely outside the region
       // because, we ignore 'splitpoint' at the endpoints, we 
       // have to use an inner point of the halfsegment for 
       // checking the containedness
      // cout << "no SPlitPoints " << endl;
      // cout << "check " << hs.middlePoint() << " for containedness" << endl;

      // if(contains(region,hs.middlePoint())){
       if(region.InInterior(hs.middlePoint())){ 
         // cout << "Region contains point" << endl;
          hs.attr.edgeno = edgeno;
          result += hs;
          hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
          result += hs;
          edgeno++; 
       } else {
          // cout << "point is not contained" << endl;
       }
       //cout << "finsihed" << endl;
       return;
    }



    // halfsegment was split by splitpoints
    // step 1: sort splitpoints by delta value
    sort(splitpoints.begin(), splitpoints.end(),splitpointless);
    // if not present, 
    if(!AlmostEqual(splitpoints[0].first,0.0)){
       splitpoints.insert(splitpoints.begin(),
                          std::pair<double,bool>(0.0,!splitpoints[0].second));
    } 
    if(!AlmostEqual(splitpoints.back().first,1.0)){
       splitpoints.push_back(
           std::pair<double,bool>(1.0,!splitpoints.back().second));
    }



    //cout << " go to complex case" << endl;
    // now, process splitpoints having equal delta values
    // within such a sequence, all splits can have the same direction
    // in this case, we keep only one of the group
    // if there are different split directions, we remove all splits at
    // this position
    std::vector<std::pair<double,bool> > splitpoints2;
    size_t pos = 0;
    while(pos <  splitpoints.size()){
      std::pair<double,bool> first = splitpoints[pos];
      bool differ = false;
      size_t pos2 = pos + 1;
      bool done = pos2 >= splitpoints.size();
      while(!done){
         if(!AlmostEqual(first.first,splitpoints[pos2].first)){
            done = true;
         } else {
            differ = first.second != splitpoints[pos2].second;
            pos2++;
         }
      }
      if(!differ){
        splitpoints2.push_back(first);
      } 
      pos = pos2;
    }



    //cout << "part 1 finished" << endl;
    
    if(splitpoints2.empty()){ // segment completely inside or 
       // TODO: ensure, that the used point is not a former
       // splitpoint   
       if(contains(region,hs.middlePoint())){
          hs.attr.edgeno = edgeno;
          result += hs;
          hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
          result += hs;
          edgeno++; 
       } 
       return;
    }

    //cout << "splits are present " << endl;


    pos = 0;
    while(pos < splitpoints2.size()){
        //cout << "start while at pos " << pos << endl;
        while(pos < (splitpoints2.size())-1 && !splitpoints2[pos].second){
            pos++;
        }
        //cout << "start found at position " << pos << endl;

        if(pos < splitpoints2.size()-1){ // found starting fragment
 
           Point p1 = atDelta(hs,splitpoints2[pos].first);

           while(pos < splitpoints2.size() && splitpoints2[pos].second){
              pos++;
           }
           //cout << "found end part at " << pos << endl;
           double s = 1.0;
           if(pos<splitpoints2.size()){
              s = splitpoints2[pos].first;
              pos++;
           } 
           Point p2 = atDelta(hs,s);
           if(!AlmostEqual(p1,p2)){
               // create and insert halfsegment
               HalfSegment hs(true,p1,p2);
               hs.attr.edgeno = edgeno;
               result += hs;
               hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
               result += hs;
               edgeno++;
           }
        } else {
           pos++;
        }
    }
    //cout << "all fisnihed" << endl;

 }


/*
 Intersection between region and line.


*/
template<template<typename T1>class Array1,
         template<typename T2>class Array2,
         template<typename T3>class Array3>
void intersection(const RegionT<Array1>& region, 
                  const LineT<Array2>& line, 
                  LineT<Array3>& result){

   result.Clear();
   if(!region.IsDefined() || ! line.IsDefined()){
      result.SetDefined(false);
      return;
   }
   result.SetDefined(true);

   if(region.IsEmpty() || line.IsEmpty()){ 
      // interscetion with empty set is empty
      return;
   }

   if(!region.BoundingBox().Intersects(line.BoundingBox())){
       // non-intersecting bounding boxes
       return; 
   }

   result.Resize(line.Size());
   
   // 1st. Build an RTree and insert the halfsegments of the region

   //cout << "build r-tree" << endl;

   mmrtree::RtreeT<2,int> tree(4,8);
   HalfSegment hs;
   for(int i=0;i<region.Size();i++){
     region.Get(i,hs);
     if(hs.IsLeftDomPoint()){
       tree.insert(hs.BoundingBox(),i);
     }  
   }
   //cout << "R-tree built" << endl;
 
   result.StartBulkLoad();
   int edgeno = 0;
   for(int i=0;i<line.Size();i++){
      line.Get(i,hs);
      if(hs.IsLeftDomPoint()){
          std::vector<std::pair<double, bool> > splitPoints;
          mmrtree::RtreeT<2,int>::iterator* it = tree.find(hs.BoundingBox());
          HalfSegment hs2;
          int const* pos;
          while(  (pos = it->next()) != 0){
             region.Get(*pos,hs2);
             addSplitPoint<Array1>(hs,hs2,splitPoints);
          }
          delete it;
          insertLineParts(hs,splitPoints, region, result, edgeno);
      } 
   }
   result.EndBulkLoad(true,false); 
}


enum splitKind  {normalSplit, startCommon, endCommon};


class splitPointComp{
  public:
  bool operator()(const std::pair<double,splitKind>& p1, 
                  const std::pair<double,splitKind>& p2){
    if(p1.first<p2.first){
      return true;
    }
    if(p1.first>p2.first){
       return false;
    }
    return p1.second < p2.second;  
  }

};


std::string splitkind(splitKind i);


template<template<typename T> class Array>
bool insertSegment(const HalfSegment& hs, 
                   const double pos1, const double pos2, 
                   Array<HalfSegment>& result, int& edgeno){

    Point p1 = atDelta(hs,pos1);
    Point p2 = atDelta(hs,pos2);
    if(AlmostEqual(p1,p2)){
       // too short for building a halfsegment
       return false;
    }
    HalfSegment res(true,p1,p2);
    res.attr.edgeno = edgeno;
    result.Append(res);
    res.SetLeftDomPoint(!res.IsLeftDomPoint());
    result.Append(res);
    edgeno++;
    return true;
}


template<template<typename T> class Array>
void insertSegmentParts(HalfSegment& hs, 
                      std::vector< std::pair<double, splitKind> >& splitpoints, 
                      Array<HalfSegment>& result, 
                      int& edgeno){


   //cout << "insertsegmentparts called for " << hs.SimpleString() << endl;

   if(splitpoints.size()==0){ // keep original Segment
       //cout << "no split, insert hs" << endl;
       hs.attr.edgeno = edgeno;
       result.Append(hs);
       hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
       result.Append(hs);
       edgeno++;
       return;
   }
   splitPointComp cmp;
   sort(splitpoints.begin(), splitpoints.end(),cmp);
   if((splitpoints[0].first!=0.0) || (splitpoints[0].second!=normalSplit) ){
      splitpoints.insert(splitpoints.begin(), 
                         std::pair<double,splitKind>(0.0,normalSplit));
   }
   if((splitpoints.back().first!=1.0) || 
      (splitpoints.back().second!=normalSplit)){
      splitpoints.push_back(std::pair<double,splitKind>(1.0,normalSplit));
   }
   double lastPos = splitpoints[0].first;
   splitKind lastKind = splitpoints[0].second;
   int numCommonParts = 0;
   for(size_t i=1;i<splitpoints.size();i++){
      double currentPos = splitpoints[i].first;
      splitKind currentKind = splitpoints[i].second;
      switch(currentKind){
        case normalSplit: {
             if(lastKind==normalSplit){
                // otherwise, this plit is within an already processed part
                bool ok = insertSegment(hs,lastPos,currentPos,result,edgeno);
                if(ok){
                   lastPos = currentPos;
                }  
             } 
             break;
        }
        case startCommon : {
           if(lastKind==normalSplit){
             assert(numCommonParts==0);
             insertSegment(hs,lastPos,currentPos, result, edgeno);
             lastPos = currentPos;
             lastKind = startCommon;
           }
           numCommonParts++;
           break;
        }
        case endCommon : {
           assert(numCommonParts>0);
           numCommonParts--;
           if(numCommonParts==0){
              lastPos = currentPos;
              lastKind = normalSplit;
           }
           break;
        }
        default : assert(false);
      }
   } 
}


bool commonPart(const HalfSegment& hs1, const HalfSegment& hs2,
                double& delta1, double& delta2);

void realminizeParallel(const HalfSegment& hs1, const HalfSegment& hs2,
                        std::vector<std::pair<double,splitKind> >& splitPoints);

void realminize(HalfSegment& hs1, HalfSegment& hs2, const bool secondFirst, 
                std::vector<std::pair<double,splitKind> >& splitpoints);


template<template<typename T1> class Array1,
         template<typename T2>class Array2>
void realminize(const Array1<HalfSegment>& src, 
                      Array2<HalfSegment>& result){
    if(src.Size()==0){
      result.clean();
      return;
    }
    // the result size is at least the size of src
    result.resize(src.Size());

    // step1: insert halfSegments into an mmrtree
    mmrtree::RtreeT<2,int> tree(4,8);
    HalfSegment hs;
    for(int i=0;i<src.Size();i++){
      src.Get(i,hs);
      if(hs.IsLeftDomPoint()){
         tree.insert(hs.BoundingBox(),i);
      }  
    }
    // step 2 process halfsegments
    int edgeno = 0;
    for(int i=0;i<src.Size();i++){
       src.Get(i,hs);
       if(hs.IsLeftDomPoint()){
          //cout << "process Segment" << hs.SimpleString() << endl;

          mmrtree::RtreeT<2,int>::iterator* it = tree.find(hs.BoundingBox());
          HalfSegment hs2;
          int const* pos;
          std::vector<std::pair<double,splitKind> > splitPoints;
          while( (pos = it->next()) != 0){
              if((*pos)!=i){
                 src.Get(*pos,hs2);
                 //cout << "Test with" << hs2.SimpleString() << endl;
                 realminize(hs,hs2, (*pos) < i , splitPoints);   
              }
          }
          delete it;
          insertSegmentParts(hs,splitPoints,result, edgeno); 
       }
    }
}


void crossings(const HalfSegment& hs1, 
               const HalfSegment& hs2,
               std::set<Point,ApproxPointLess>& candidates,
               std::set<Point,ApproxPointLess>& falseHitsCandidates);


template<template<typename T1>class Array1,
         template<typename T2>class Array2,
         template<typename T3>class Array3>
void crossings(const LineT<Array1>& l1, 
               const LineT<Array2>& l2,
               PointsT<Array3>& result) {
    if(!l1.IsDefined() || !l2.IsDefined()){
       result.SetDefined(false);
       return;
    }
    result.Clear();
    result.SetDefined(true);

    //build an r-tree from the first line

    // step1: insert halfSegments into an mmrtree
    mmrtree::RtreeT<2,int> tree(4,8);
    HalfSegment hs;
    for(int i=0;i<l1.Size();i++){
      l1.Get(i,hs);
      if(hs.IsLeftDomPoint()){
         tree.insert(hs.BoundingBox(),i);
      }  
    }

    // step 2 process halfsegments of l2
    std::set<Point,ApproxPointLess> candidates;
    std::set<Point,ApproxPointLess> falseHitCandidates;

    for(int i=0;i<l2.Size();i++){
       l2.Get(i,hs);
       if(hs.IsLeftDomPoint()){
          mmrtree::RtreeT<2,int>::iterator* it = tree.find(hs.BoundingBox());
          HalfSegment hs2;
          int const* pos;
          while( (pos = it->next()) != 0){
             l1.Get(*pos,hs2);
             crossings(hs,hs2,candidates, falseHitCandidates);   
          }
          delete it;
       }
    }

    result.StartBulkLoad();
    std::set<Point,ApproxPointLess>::iterator it;
  
    for(it = candidates.begin();it!=candidates.end();it++){
       if(falseHitCandidates.find(*it) == falseHitCandidates.end()){
          result += *it;
       }
    } 
   
    result.EndBulkLoad();

}

bool onSameLine(const HalfSegment& hs1, const HalfSegment& hs2);

/*
Realmchecker implemenation.

*/

template<template<typename T>class Array>
 bool RealmChecker<Array>::isRealm(const HalfSegment& hs1, 
                            const HalfSegment& hs2, 
                            const bool print){
    if(onSameLine(hs1,hs2)){
       bool ok =  !overlaps(hs1,hs2);
       if(!ok && print){
          cout << "found overlapping segments " << endl;
          cout << "S1 : " << hs1.getLineString() << endl;
          cout << "S2 : " << hs2.getLineString() << endl;
       }
       return ok;
    } else {

       Point dp1 = hs1.GetDomPoint();
       Point sp1 = hs1.GetSecPoint();
       Point dp2 = hs2.GetDomPoint();
       Point sp2 = hs2.GetSecPoint();
       if(AlmostEqual(dp1,dp2) || AlmostEqual(dp1,sp2) ||
           AlmostEqual(sp1,dp2) || AlmostEqual(sp1,sp2) ){
         return  true;
       }


       double delta1, delta2;
       bool intersection = computeSplitPoint(hs1,hs2,delta1,delta2);
       if(!intersection){ // parallel segments
          return true;
       }
       bool ok = true;
       if( (0 < delta1) && (delta1 < 1)){
          if( (delta2>=0) && (delta2<=1)){
              
              ok =  false;
          }
       }
       if( (0 < delta2) && (delta2 < 1)){
          if( (delta1>=0) && (delta1<=1)){
             ok = false;
          }
       }
       if(!ok && print){
         cout << "found crossing segments" << endl;
         cout << "S1 : " << hs1.getLineString() << endl;
         cout << "S2 : " << hs2.getLineString() << endl;
         cout << "delta1 = " << delta1 << endl;
         cout << "delta2 = " << delta2 << endl;
       }
       return ok;
    }
    

 }

template<template<typename T>class Array>
RealmChecker<Array>::RealmChecker(const Array<HalfSegment>* _hss):
    hss(_hss), tree(4,8), pos(0), it(0) {

    ListExpr numTupleType = 
            SecondoSystem::GetCatalog()->NumericType(getTupleType());
    tt = new TupleType(numTupleType);
    HalfSegment hs1;
    for(int i=0;i<hss->Size();i++){
      hss->Get(i,hs1);
      if(hs1.IsLeftDomPoint()){
         tree.insert(hs1.BoundingBox(),i);
      }  
    }
}


template<template<typename T>class Array>
RealmChecker<Array>::RealmChecker(const Array<HalfSegment>* _hss,
                                  TupleType* _tt):
    hss(_hss), tree(4,8), pos(0), it(0) {

    tt = _tt;
    tt->IncReference();
    HalfSegment hs1;
    for(int i=0;i<hss->Size();i++){
      hss->Get(i,hs1);
      if(hs1.IsLeftDomPoint()){
         tree.insert(hs1.BoundingBox(),i);
      }  
    }
}


template<template<typename T>class Array>
void RealmChecker<Array>::reset(){
   if(it){
     delete it;
     it = 0;
   }
   pos = 0;
}


template<template<typename T>class Array>
RealmChecker<Array>::~RealmChecker(){
    hss = 0;
    if(it){
      delete it; 
    }
    tt->DeleteIfAllowed();
}


template<template<typename T>class Array>
bool RealmChecker<Array>::checkRealm(){
    reset();
    Tuple* t = nextTuple(true);
    if(t){
       t->DeleteIfAllowed();
       return false;
    } else {
       return true;
    }
}


template<template<typename T>class Array>
Tuple* RealmChecker<Array>::nextTuple(const bool print /* = false */ ){
  while(pos < hss->Size()){
      while(!it){
        hss->Get(pos,currentHs);
        if(currentHs.IsLeftDomPoint()){ 
            it = tree.find(currentHs.BoundingBox());
        } else {
          pos++;
          if(pos>=hss->Size()){
             return 0;
          }
        }
      }
      int const* itpos;
      bool ok;
      HalfSegment hs2;
      while( (itpos = it->next()) != 0) {
         if(*itpos != this->pos){ 
             hss->Get(*itpos,hs2);
             ok = isRealm(currentHs,hs2, print);
             if(!ok){
                return createTuple(this->pos, *itpos, currentHs, hs2); 
             }
         }
      } 
      delete it;
      it = 0;
      pos++; 
   }
   return 0;
}


template<template<typename T>class Array>
Tuple* RealmChecker<Array>::createTuple(const int pos1, const int pos2, 
                   const HalfSegment& hs1, const HalfSegment& hs2) const{

   Tuple* res = new Tuple(tt);
   res->PutAttribute(0, new CcInt(true,pos1));
   res->PutAttribute(1, new CcInt(true, hs1.attr.partnerno));
   res->PutAttribute(2, getLine(hs1));
   res->PutAttribute(3, new CcInt(true,pos2));
   res->PutAttribute(4, new CcInt(true, hs2.attr.partnerno));
   res->PutAttribute(5, getLine(hs2));
   return res;
}


template<template<typename T>class Array>
LineT<Array>* RealmChecker<Array>::getLine( HalfSegment hs){
   LineT<Array>* result = new LineT<Array>(2);
   result->StartBulkLoad();
   hs.attr.edgeno = 0;
   hs.SetLeftDomPoint(true);
   (*result) += hs;
   hs.SetLeftDomPoint(false);
   (*result) += hs;
   result->EndBulkLoad();
   return result;
}


template<template<typename T>class Array>
ListExpr RealmChecker<Array>::getTupleType(){

  ListExpr attr = nl->TwoElemList(
                       nl->SymbolAtom("No1"),
                       listutils::basicSymbol<CcInt>());

  ListExpr res = nl->OneElemList( attr);
  ListExpr last = res;

  attr = nl->TwoElemList(
                       nl->SymbolAtom("Partner1"),
                       listutils::basicSymbol<CcInt>());
  last = nl->Append(last, attr);
  attr = nl->TwoElemList(
                       nl->SymbolAtom("Segment1"),
                       listutils::basicSymbol<LineT<DbArray> >());
  last = nl->Append(last, attr);

  attr = nl->TwoElemList(
                       nl->SymbolAtom("No2"),
                       listutils::basicSymbol<CcInt>());
  last = nl->Append(last, attr);
  attr = nl->TwoElemList(
                       nl->SymbolAtom("Partner2"),
                       listutils::basicSymbol<CcInt>());
  last = nl->Append(last, attr);
  attr = nl->TwoElemList(
                       nl->SymbolAtom("Segment2"),
                       listutils::basicSymbol<LineT<DbArray> >());
  last = nl->Append(last, attr);
  return  nl->TwoElemList(listutils::basicSymbol<Tuple>(),res); 
}


template<template<typename T1>class Array1,
         template<typename T2>class Array2,
         template<typename T3>class Array3>
void intersection(const LineT<Array1>& l1, 
                  const LineT<Array2>& l2,  
                  LineT<Array3>& result){

   if(!l1.IsDefined() || !l1.IsDefined()){
      result.SetDefined(false);
      return;
   }
   if(l1.Size() == 0){
      result.CopyFrom(&l2);
      return;
   }
   if(l2.Size() == 0){
      result.CopyFrom(&l1);
      return;
   }
   if(!l1.BoundingBox().Intersects(l2.BoundingBox())){
      result.Clear();
      return;
   }
   // overlapping bounding boxes, we have to compute 
   // insert l1 into an r-tree
   mmrtree::RtreeT<2,int> tree(4,8);
   HalfSegment hs;
   for(int i=0;i<l1.Size();i++){
      l1.Get(i,hs);
      if(hs.IsLeftDomPoint()){
        tree.insert(hs.BoundingBox(),i);
      }
   }
   // iterate over l2, look for common parts in tree
   result.Clear();
   result.SetDefined(true);
   result.StartBulkLoad();
   std::vector<std::pair<double,bool> > v;
   int edgeno = 0;
   for(int i=0;i<l2.Size();i++){
      l2.Get(i,hs);
      if(hs.IsLeftDomPoint()){
        mmrtree::RtreeT<2,int>::iterator* it=0;
        it = tree.find(hs.BoundingBox());
        int  const* pos;
        HalfSegment hs2;
        while( (pos = it->next()) != 0){
           l1.Get(*pos,hs2);
           if(onSameLine(hs,hs2)){
              processParallel(hs,hs2,v);
              if(!v.empty()){
                double d1 = v[0].first;
                double d2 = v[1].first;
                v.clear();
                HalfSegment hsr(true,atDelta(hs,d1), atDelta(hs,d2));
                hsr.attr.edgeno = edgeno;
                result += hsr;
                hsr.SetLeftDomPoint(!hsr.IsLeftDomPoint());
                result += hsr;
                edgeno++;       
              }
           } 
        } 
        delete it;
     }
  }
  result.EndBulkLoad( true,false); // sort, no realminize
}


template<template<typename T1>class Array1,
         template<typename T2>class Array2,
         template<typename T3>class Array3>
void intersection(const LineT<Array1>& l, 
                  const RegionT<Array2>& r, 
                  LineT<Array3>& result){
   intersection(r,l,result);
}





} // end of namespace robust


#endif
