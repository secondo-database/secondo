/*
----
This file is part of SECONDO.

Copyright (C) 2010, University in Hagen, 
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


1 RegionTools


This file contains functions handling with regions.


*/

#include "SpatialAlgebra.h"
#include "RegionTools.h"
#include "../TopOps/TopOpsAlgebra.h"
#include <vector>




/*
~reverseCycle~

Changes teh direction of a cycle.

*/

void reverseCycle(vector<Point>& cycle){

  for(unsigned int i=0;i<cycle.size()/2; i++){
     Point tmp = cycle[i];
     cycle[i] = cycle[cycle.size()-(i+1)];
     cycle[cycle.size()-(i+1)] = tmp;
  }
}



/*
1.2 ~getDir~

Determines the direction of a cycle. If the cycle is in clockwise order, the
return value is true. If the cycle is directed counter clockwise, the result 
will
be false. 


*/

bool getDir(const vector<Point>& vp){
  // determine the direction of cycle
  int min = 0;
  for(unsigned int i=1;i<vp.size();i++){
    if(vp[i] < vp[min]){
       min = i;
    }
  }

  bool cw;
  int s = vp.size();
  if(AlmostEqual(vp[0],vp[vp.size()-1])){
    s--;
  }

  Point a = vp[ (min - 1 + s ) % s ];
  Point p = vp[min];
  Point b = vp[ (min+1) % s];
  if(AlmostEqual(a.GetX(),p.GetX())){ // a -> p vertical
    if(a.GetY()>p.GetY()){
       cw = false;
    } else {
       cw = true;
    }
  } else if(AlmostEqual(p.GetX(), b.GetX())){ //p -> b vertical
    if(p.GetY()>b.GetY()){
       cw = false;
    } else {
       cw = true;
    }
  } else { // both segments are non-vertical
    double m_p_a = (a.GetY() - p.GetY()) / (a.GetX() - p.GetX());
    double m_p_b = (b.GetY() - p.GetY()) / (b.GetX() - p.GetX());
    if(m_p_a > m_p_b){
        cw = false;
    } else {
        cw = true;
    }
  }
  return cw;
}



/*
~separateCycles~

Finds simple subcycles within ~path~ and inserts each of them into
~cycles~.

*/
void separateCycles(const vector<Point>& path, vector <vector<Point> >& cycles){

  if(path.size()<4){ // path too short for a polyon
    return;
  }

  if(!AlmostEqual(path[0], path[path.size()-1])){
    cout << "Ignore Dead end" << endl;
    return;
  }

  set<Point> visitedPoints;
  vector<Point> cycle;

  for(unsigned int i=0;i<path.size(); i++){
    Point p = path[i];
    if(visitedPoints.find(p)!=visitedPoints.end()){ // subpath found
      vector<Point> subpath;
      subpath.clear();
      subpath.push_back(p);
      int pos = cycle.size()-1;
      while(pos>=0 && !AlmostEqual(cycle[pos],p)){
         subpath.push_back(cycle[pos]);
         visitedPoints.erase(cycle[pos]);
         pos--;
      }
      if(pos<0){
        cerr << "internal error during searching a subpath" << endl;
        return;
      } else {
        subpath.push_back(p); // close path;
        if(subpath.size()>3){
          cycles.push_back(subpath);
        }
        cycle.erase(cycle.begin()+(pos+1), cycle.end());
      }
    } else {
      cycle.push_back(p);
      visitedPoints.insert(p);
    }
  }
  if(cycle.size()>3){
    cycles.push_back(cycle);
  }
}



/*
~getUnusedExtension~

Returns the position of an unused extension of the segemnt at position pos.
If no such segment exist, -1 is returned.

*/

int getUnusedExtension(const DbArray<HalfSegment>& segs,
                        const int pos,
                        const bool* used){
      HalfSegment hs;
      segs.Get(pos,hs);
      Point dp = hs.GetDomPoint();
      double dpx = dp.GetX();
      bool done = false;
      int pos2=pos-1;
      while(pos2>=0 && !done){
        if(used[pos2]){
          pos2--;
        } else {
          segs.Get(pos2,hs);
          if(AlmostEqual(dp,hs.GetDomPoint())){
             return pos2;
          } else {
             double dpx2 = hs.GetDomPoint().GetX();
             if(AlmostEqual(dpx,dpx2)){
               pos2--;
             } else { // outside the X-Range
               done = true;
             }
          }
        }
      }
      pos2 = pos+1;
      while(pos2<segs.Size() ){
         if(used[pos2]){
             pos2++;
         }else {
            segs.Get(pos2,hs);
            if(AlmostEqual(dp,hs.GetDomPoint())){
              return pos2;
            } else {
              double dpx2 = hs.GetDomPoint().GetX();
              if(AlmostEqual(dpx,dpx2)){
                pos2++;
              }else{
                return -1;
              }
            }
         }
      }
      return -1;
}

/*
~SetPartnerno~

Sets the partner for the halfsegments if the edegno is set correct.

*/
static void SetPartnerNo(DbArray<HalfSegment>& segs){
  if(segs.Size()==0){
     return;
  }
  int TMP[(segs.Size()+1)/2];
  HalfSegment hs1;
  HalfSegment hs2;
  for(int i=0; i<segs.Size(); i++){
     segs.Get(i,&hs1);
     if(hs1.IsLeftDomPoint()){
       TMP[hs1.attr.edgeno] = i;
     } else {
       int leftpos = TMP[hs1.attr.edgeno];
       HalfSegment right = hs1;
       right.attr.partnerno = leftpos;
       segs.Get(leftpos,hs2);
       HalfSegment left = hs2;
       left.attr.partnerno = i;
       segs.Put(i,right);
       segs.Put(leftpos,left);
     }
  }
}



 void addRegion(vector<pair<Region*, bool> > & regs, vector<Point>& cycle){
   bool isFace = getDir(cycle);
   if(cycle.size()<2){
     return;
   }

   if(!AlmostEqual(cycle[0],cycle[cycle.size()-1])){ // cycle not closed

   }

    if(cycle.size() < 4){
       return;
    }

    Line* hss = new Line(cycle.size()*2);

    hss->StartBulkLoad();
    for(size_t i = 0; i<cycle.size()-1; i++){
       HalfSegment hs(true, cycle[i],cycle[i+1]);
       hs.attr.edgeno = i;
       (*hss) += hs;
       hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
       (*hss) += hs; 
    }
    hss->EndBulkLoad();
    Region* res= new Region(cycle.size()*2);

    hss->Transform(*res);
    hss->DeleteIfAllowed();
    regs.push_back(pair<Region*,bool>(res, isFace));
}


 

 Region*  buildRegion2( vector< vector<Point> >& cycles){


   //cout << "buildRegion2  called wth " << cycles.size() << " cycles " << endl;

   Line* hss = new Line(80);

   hss->StartBulkLoad();
   int edgeno = 0;
   for(size_t c = 0; c<cycles.size(); c++){
       for(size_t i=0;i<cycles[c].size()-1; i++){
          HalfSegment hs(true, cycles[c][i],cycles[c][i+1]);
          hs.attr.edgeno = edgeno;
          (*hss) += hs;
          hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
          (*hss) += hs; 
          edgeno++;
       }
    }


    hss->EndBulkLoad();

    

    Region* res= new Region(hss->Size());

    hss->Transform(*res);


    hss->DeleteIfAllowed();
    return res;
}

/*
Adds a single cycle region to regs if cycle is valid.

*/
void addRegion2(vector<pair<Region*, bool> >& regs, vector<Point>& cycle){

  if(cycle.size()<4){ // at least 3 points
    cerr << "Cycle with less than 3 different points detected" << endl;
    return;
  }
  bool isFace = getDir(cycle);

  // create a DBArray of halfsegments representing this cycle
  DbArray<HalfSegment> segments1(0);
  
  for(unsigned int i=0;i<cycle.size()-1;i++){
    Point p1 = cycle[i];
    Point p2 = cycle[i+1];
    Point lp(0.0,0.0);
    Point rp(0.0,0.0);
    if(p1<p2){
       lp = p1;
       rp = p2;
    } else {
       lp = p2;
       rp = p1;
    }

    HalfSegment hs1(true,lp,rp);

    hs1.attr.edgeno = i;
    hs1.attr.faceno = 0;
    hs1.attr.cycleno =0;
    hs1.attr.coverageno = 0;
    hs1.attr.insideAbove = false;
    hs1.attr.partnerno = -1;

    HalfSegment hs2 = hs1;
    hs2.SetLeftDomPoint(false);

    segments1.Append(hs1);
    segments1.Append(hs2);
  }

  segments1.Sort(HalfSegmentCompare);

  // split the segments at crossings and overlappings
  DbArray<HalfSegment>* segments = Split(segments1);


  SetPartnerNo(*segments);


  bool used[segments->Size()];
  for(int i=0;i<segments->Size();i++){
    used[i] = false;
  }

  // try to find cycles

  vector< vector<Point> > cycles; // corrected (simple) cycles

  vector<Point> path;     // current path
  set<Point>  points;     // index for points in path
  bool subcycle;          // a multiple point within the path?
  for(int i=0; i< segments->Size(); i++){
    if(!used[i]){ // start of a new path found
      int pos = i;
      path.clear();
      points.clear();
      bool done = false;
      subcycle = false;
      while(!done){
        HalfSegment hs1;
        segments->Get(pos,hs1);
        Point dp = hs1.GetDomPoint();
        Point ndp = hs1.GetSecPoint();
        int partner = hs1.attr.partnerno;
        path.push_back(dp);
        points.insert(dp);
        used[pos] = true;
        used[partner] = true;
        if(points.find(ndp)!=points.end()){ // (sub) cycle found
          if(AlmostEqual(path[0],ndp)){ // cycle closed
             path.push_back(ndp);
             done = true;
          } else { // subcycle found
             subcycle = true;
          }
        }
        if(!done){
          // no cycle, try to extend
          int nb = getUnusedExtension(*segments,partner,used);
          if(nb>=0){ // extension found, continue
            pos = nb;
          } else { // dead end found, track back
            cout << " ----> DEAD END FOUND <--- " << endl;
            done = true; // should never occur
          }
        }
      }
      if(subcycle){
        separateCycles(path,cycles);
      } else if( (path.size()>3 ) && AlmostEqual(path[0],path[path.size()-1])){
        vector<Point> cycle = path;
        cycles.push_back(cycle);
      } else {
        cout << "remove invalid path of lengthh " << path.size() << endl;
      }
    }// new path found
  } // for
  delete segments;

  // build the region from the corrected cycles
  Region* result = 0;
  for(unsigned int i = 0; i< cycles.size();i++){
    vector<Point> cycle = cycles[i];
    bool cw = getDir(cycle);
    Region* reg = new Region(0);
    reg->StartBulkLoad();
    for(unsigned int j=0;j<cycle.size()-1;j++){
       Point lp,rp;
       bool smallb;
       smallb = (cycle[j] < cycle[j+1]);
       if(smallb){
         lp = cycle[j];
         rp = cycle[j+1];
       } else {
         lp = cycle[j+1];
         rp = cycle[j];
       }
       HalfSegment hs(true,lp,rp);
       hs.attr.edgeno = j;
       hs.attr.insideAbove = (cw && !smallb) || (!cw && smallb);
       hs.attr.faceno=0;
       hs.attr.cycleno = 0;
       hs.attr.coverageno = 0;
       HalfSegment hs2(hs);
       hs2.SetLeftDomPoint(false);
       *reg += hs;
       *reg += hs2;
    }
    reg->EndBulkLoad();

    if(!result){
      result = reg;
    } else {
      Region* tmp = SetOp(*result,*reg,avlseg::union_op);
      delete result;
      result = tmp;
      delete reg;
    }
  }
  if(result){
    regs.push_back(make_pair(result,isFace));
  }
}


/*
~buildRegion~

Builds a region from a set of cycles.

*/


 Region* buildRegion(vector< vector<Point> >& cycles){
     // first step create a single region from each cycle
     vector<pair<Region*, bool> > sc_regions; // single cycle regions
     for(unsigned int i=0;i<cycles.size(); i++){
        vector<Point> cycle = cycles[i];
        addRegion(sc_regions,cycle);
     }

     // split the vector into faces and holes
     vector<Region*> faces;
     vector<Region*> holes;

     for(unsigned int i=0;i<sc_regions.size();i++){
       if(sc_regions[i].second){
          faces.push_back(sc_regions[i].first);
       } else {
          holes.push_back(sc_regions[i].first);
       }
     }

     // subtract all holes from each face if nessecary
     vector<Region*> faces2;
     for(unsigned int i=0;i<faces.size();i++){
        Region* face = faces[i];
        for(unsigned int j=0; j< holes.size(); j++){
           Region* hole = holes[j];
           if(face->BoundingBox().Intersects(hole->BoundingBox())){
              if(!topops::wcontains(hole,face)){ // may be an island
                 Region* tmp = SetOp(*face,*hole,avlseg::difference_op);
                 delete face;
                 face = tmp;
              }
           }
        }
        if((face->Size())!=0){
           faces2.push_back(face);
        } else { // face was removed completely
           delete face;
        }
     }

     // the hole regions are not longer needed, delete them
     for(unsigned int i=0;i<holes.size();i++){
      delete holes[i];
     }

     if(faces2.size()<1){
         cerr << "no face found within the cycles" << endl;
         return new Region(0);
     }
     // build the union of all faces
     Region* reg = faces2[0];
     for(unsigned int i=1;i<faces2.size();i++){
       Region* face2 = faces2[i];
       Region* tmp = SetOp(*reg, *face2, avlseg::union_op);
       delete reg;
       delete face2;
       reg = tmp;
     }
     return reg;
  }
