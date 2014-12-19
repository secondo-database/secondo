/*
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
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

*/

#include <algorithm>
#include <stdio.h>

/*
GIS includes

*/
#include "contour.h"

/*
Raster2 and Tile includes

*/
#include "../Raster2/sint.h"
#include "../Raster2/sreal.h"
#include "../Tile/t/tint.h"
#include "../Tile/t/treal.h"
#include "StopWatch.h"

#include "DLine.h"
#include "WinUnix.h"

typedef DLine LineType;



/*
declaration of namespace GISAlgebra

*/
namespace GISAlgebra {


/*
declaration of struct ResultInfo

*/
struct ResultInfo {   
    ResultInfo(int dummy): level(-32000), cline(0){
    }   
    ResultInfo() {}  
    int level;
    LineType* cline;
};  



class ContourLocalInfo{
public:
  ContourLocalInfo(const int _num, const double _min, 
                   const int _interval, ListExpr tupleType) :
      clines(_num), num(_num),min(_min), interval(_interval){
     ResultInfo line(1);
     for (int i = 0; i<num; i++) {
          clines.Put(i,line);
     }
     tt = new TupleType(tupleType);
  }

  ~ContourLocalInfo(){
     tt->DeleteIfAllowed();
  }



  void addSegment(int level, const Point& p1, const Point & p2){
    HalfSegment hs(true, p1, p2);

    // calculate array element
    int i = floor((level - min) / interval);

    // find correct contour line
    ResultInfo temp(1);
    clines.Get(i,temp);

    // if contour line exists, add segment
    if (temp.level != -32000)
    {            
      LineType* line = temp.cline;

      int s = line->Size();

      hs.attr.edgeno = s/2;
      (*line) += hs;
      hs.SetLeftDomPoint(false);
      (*line) += hs;
    }
    else
    // if not, create new line
    {
      LineType* line = new LineType(0);
      line->StartBulkLoad();
      hs.attr.edgeno = 0;
      (*line) += hs;
      hs.SetLeftDomPoint(false);
      (*line) += hs;

      int l2 = static_cast<int>(level);

      ResultInfo lines(0);

      lines.level = l2;
      lines.cline = line;

      clines.Put(i,lines);
    }

  }

  void finish(){};

  Tuple* getNext() {
       int i = clines.Size();

       if(i<= 0){
         return 0;
       }
       ResultInfo temp(1);
       clines.Get(i-1,temp);
       clines.resize(i-1);

       int j = 2;

       while ((temp.level == -32000) && (j <= i)) {
          clines.Get(i-j,temp);
          clines.resize(i-j);
          j++;
       }
       if ( temp.level == -32000 ) {
            return 0;
       }

       CcInt* level = new CcInt(true,temp.level);
       LineType* line = temp.cline;
       line->EndBulkLoad();
       Tuple* result = new Tuple(tt);

       result->PutAttribute(0,level);
       result->PutAttribute(1,line);
       return result;
  }

  private:
    DbArray<ResultInfo> clines;
    int num;
    double min;
    int interval;
    TupleType* tt;
};




/*
1 class ~hSegment~

This class represents a single segment assigned to a height. 
There are two spezial values for a normal mark and a specialized
end mark.


*/
class hSegment{
  public:
     hSegment(){}

     hSegment(const int _level, const Point& _p1, const Point& _p2):
        level(_level), p1(_p1), p2(_p2){
       if(p1.IsDefined() && p2.IsDefined() && (p1< p2)) swap(p1,p2);
     }

     hSegment(const hSegment& h): level(h.level), p1(h.p1), p2(h.p2){}

     ~hSegment(){}

     hSegment& operator=(const hSegment& h){
       level = h.level;
       p1 = h.p1;
       p2 = h.p2;
       return *this;
     }

     bool operator==(const hSegment& h)const{
       return (level==level) && AlmostEqual(p1,h.p1) && AlmostEqual(p2,h.p2);
     }

     bool operator<(const hSegment& h) const{
       if(level!=h.level){
          return level < h.level;
       }
       if(!AlmostEqual(p1,h.p1)){
         return p1 < h.p1;
       }
       if(!AlmostEqual(p2,h.p2)){
         return p2 < h.p2;
       }
       return  false;
     }

     inline int getLevel() const{ return level; }
     inline const Point& getP1()const{ return p1; }
     inline const Point& getP2()const{ return p2; }

     static hSegment getEnd(){
       return hSegment(0,Point(false,0,0),Point(false,0,0));
     }
     static hSegment getMarker(){
       return hSegment(0,Point(false,0,0),Point(true,0,0));
     }

     bool isMarker(){
       return !p1.IsDefined();
     }
     
     bool isEnd(){
       return !p1.IsDefined() && !p2.IsDefined();
     }
  private:
    int level;
    Point p1;
    Point p2;
};


/*
2 class restrictedHeap

This class provides a heap implementation having a restricted size.
The size is given during construction an instance of this class.
When inserting a new element into the heap, the smallest value
(including the new element) is removed from the set of heap elements
and the new element and given back.


*/

template<class T>
class restrictedHeap{
 public:
  restrictedHeap(size_t slots): slots(slots), used(0){
    content = new T[slots];
  }

  ~restrictedHeap(){
     delete[] content;
  }

/*
Inserts a new element. If the heap overflows, the element's value
is set to the smallest value in the heap (including the element itselfs.
The smallest element is removed from the heap and the result will be true;

*/  

  bool insert( T& element){
     if(used<slots){ // there is space available
        content[used] = element;
        used++;
        climb();
        return false;
     }
     if(element == content[0] || element < content[0]){
        return true;
     }
     std::swap(element,content[0]);
     sink();
     return true;
  }

  size_t size() const{
     return used;
  }

  bool empty() const{
    return used==0;
  }

  bool full() const{
    return used == slots;
  }

  const T& getMin(){
    assert(used>0);
    return content[0];
  }

  const void deleteMin(){
     if(used>0){
        swap(content[0], content[used-1]);
        used --;
        sink();
     }
  }


  private:
    size_t slots;
    size_t used;
    T* content;

/*
lets the last element in the heap climb to its final position 

*/
  void climb(){
     size_t pos = used;
     while(pos>1){
        size_t father= pos/2;
        if(content[father-1] < content[pos-1]){
             return;
        }
        swap(content[father-1], content[pos-1]);
        pos = father;
     }
  }

/*
lets the first element in heap sink to its final position

*/  
  void sink(){
     size_t pos = 1;
     while(pos < slots){
        size_t s1 = pos*2;
        if(s1>used){
           return;
        }
        size_t s2 = s1 + 1;
        size_t s;
        if(s2>used){
           // only one son available
           s = s1;
        } else {
           s = content[s1-1] < content[s2-1]?s1:s2;
        }
        if(content[pos-1] < content[s-1]){
          return; // reached final position
        }
        swap(content[pos-1],content[s-1]);
        pos = s;
     } 
  }
};




/*
3 class segmentStorage

This class provides a storage for hSegment objects. 
There are two stages of usage this storage. Within the
first stage, elements can be inserted into this storage.
This stage is finished using the finish() method. 
In the second stage (after calling finished), elements 
can be retrieved from the storage.  The segments are 
returned according to its order.

*/
class segmentStorage{
 public:

/*
3.1 Contructor

This constructor takes the available memory in bytes as its input.

*/
     segmentStorage(size_t mem){
     if(mem<4096){
       mem = 4096;
     }
     slots = mem/(sizeof(hSegment) + sizeof(void*) );
     if(slots<2){
        slots = 2; 
     }
     h1 = new restrictedHeap<hSegment>(slots/2);
     h2 = 0;
     f1 = 0;
     f2 = 0;
     outputCache = 0;
     buffersize = 8192; // buffersize for file operations
   }

/*
3.2 Destructor

*/ 
  ~segmentStorage(){
      if(h1) delete h1;
      if(h2) delete h1;
      if(outputCache) delete[] outputCache;
      if(f1){ f1->close(); delete f1; }
      if(f2){ f2->close(); delete f2; };
      for(size_t i=0;i<filenames.size();i++){
         remove(filenames[i].c_str());
      }
      for(size_t i=0;i<filebuffers.size();i++){
         delete[] filebuffers[i];
      }
   }


/*
3.3 insert

This function inserts a new element to this storage.

*/
   void insert(hSegment& seg){
      if(!h2 && !h1->full()){
         // first filling of h1
         bool overflow = h1->insert(seg);
         assert(!overflow);
         return;
      }
      // stage 2
      hSegment cmin = h1->getMin();
      h1->deleteMin();
      append(f1,cmin); // append to file
      if(seg < cmin){
         if(!h2){
           h2 = new restrictedHeap<hSegment>(slots/2);
         }
         h2->insert(seg);
      } else {
        h1->insert(seg);
      }
      if(h1->size()==0){
         appendMarker(f1);
         swap(f1,f2);
         swap(h1,h2);
      } 
   }


/*
3.4 finish

switch from insertion phase to retrieval phase.

*/

   void finish(){
     //  case 1: h2, f1 and f2 are not present : all items are included in h1
     if(h1 && !h2 && !f1 && !f2){
       //cout << "Case : all elements are in h1" << endl;
       size_t size = h1->size();
       if(size>0){
          outputCache = new hSegment[size];
          int i=0;
          while(!h1->empty()){
             outputCache[i] = h1->getMin();
             i++; 
             h1->deleteMin();
          }
          outPos = 0;
          outMax = size;
       } else {
          outPos = 0;
          outMax = 0;
       }
       return;
     }

     // case 2: h1 and f1 are in use
     if(f1 && !h2 && !f2){
       // all elements in h1 are greater than those in
       // f1, thus we just move the content of h1 to
       // the file, generate the output buffer and
       // fill the buffer with the content of f1
       //cout << "case 2: h1 and f1 are used, h2 and f2 not" << endl;
       append(f1,h1,false,true);
       outMax = slots;
       outputCache = new hSegment[outMax];
       f1->seekg(0);
       fillOutputCache();
       return; 
     }

    // case 3: h1, h2 and f1 is used, f2 not
    if(h1 && h2 && f1){
       //cout << "case 3 : one or two files and both heaps is used" << endl;
      // all elements in f1 are smaller than
      // those in h1, elements in h2 are
      // smaller than the last element in f1
      append(f1,h1,true,true);      
      append(f2,h2,true,true);
      merge(f1,f2);
      outputCache = new hSegment[slots];
      outMax = slots;
      f1->seekg(0);
      fillOutputCache();
      return;
    }



     assert(false); //  forgotten case

   }


/*
3.5 ~hasNext~

Within the retrieval phase, it can be checked whether more elements are available.

*/
   bool hasNext() const{
      return outPos < outMax;
   }

   const hSegment& current(){
      return outputCache[outPos];
   }

   void next(){
      outPos++;
      if(outPos == outMax){
         if(f1){
            fillOutputCache();
         }
      }
   }

 private:
    restrictedHeap<hSegment>* h1; // heap
    restrictedHeap<hSegment>* h2; // heap
    fstream* f1;                  // file for heap overflow
    fstream* f2;                  // second file
    vector<char*> filebuffers;    // vector of  buffers
    size_t slots;                 
    hSegment* outputCache;
    size_t outPos;
    size_t outMax;
    vector<string> filenames;
    size_t buffersize;

/*
3.6 append

Appends ~seg~ to ~f~. if ~f~ does not exist, it will be created.

*/
    void append(fstream*& f , const hSegment& seg){
       if(!f){
          string fname = generateFName();
          f = new fstream(fname.c_str(), ios_base::in | ios_base::out 
                          | ios_base::binary | ios_base::trunc);
          char* buffer = new char[buffersize];
          filebuffers.push_back(buffer);
          f->rdbuf()->pubsetbuf(buffer, buffersize);
       }
       f->write((char*) &seg, sizeof(hSegment));
    }

/*
3.7 append

Appends a normal marker to the given file.

*/

    void appendMarker(fstream*& f){
      static hSegment marker = hSegment::getMarker();
      append(f,marker);
    }


/*
3.8 gerenateFName

Generates a new filename.

*/
    string generateFName(){
      static int no = 0;
      stringstream ss;
      ss << "tmp/T_" << WinUnix::getpid() << "_segStorage_" << no;
      no++;
      filenames.push_back(ss.str());
      return ss.str();
    }


/*
3.9 fillOutputcache

Copies data from file f1 to the outputcache.

*/

    void fillOutputCache(){
       assert(f1);
       outPos = 0;
       size_t pos = 0;
       hSegment seg;
       while( (pos < outMax) && !f1->eof()){
          f1->read((char*)&seg, sizeof(hSegment));
          if(!seg.isMarker()){
            outputCache[pos] = seg;
            pos++;
          } else if(seg.isEnd()){
              f1->close();
              delete f1;
              f1 = 0;
              outMax = pos;
              return;
          }
       }
       if(f1->eof()){
          f1->close();
          delete f1;
          f1 = 0;
       }
       outMax = pos;
    }


/*
3.10 append

Appends all elements within the heap to ~f~. If finishMarker
is set to ~true~, an marker will be appended after the elements
of ~h~. If destroy Heap is set to true, the heap is destroyed 
after copying the elements.

*/
    void append(fstream*& f, 
                restrictedHeap<hSegment>*& h, 
                bool finishMarker, bool destroyHeap){
       while(!h->empty()){
         append(f,h->getMin());
         h->deleteMin();
       }
       if(finishMarker){
         appendMarker(f);
       }
       if(destroyHeap){
         delete h;
         h = 0;
       }
    }


/*
3.11 merge

Merges the contents of f1 and f2 together in result. 
The result is written to f1. 


*/

    void merge(fstream*& f1, fstream*& f2){
       StopWatch sw;
       size_t phases = 0;
       fstream* g1 = new fstream(generateFName().c_str(), ios_base::in | 
                                      ios_base::out | ios_base::binary | 
                                                        ios_base::trunc);
       fstream* g2 = new fstream(generateFName().c_str(), ios_base::in | 
                                      ios_base::out | ios_base::binary | 
                                                        ios_base::trunc);
       append(f1, hSegment::getEnd());
       append(f2, hSegment::getEnd());
       char g1buf[buffersize];
       char g2buf[buffersize];
       g1->rdbuf()->pubsetbuf(g1buf,buffersize);
       g2->rdbuf()->pubsetbuf(g2buf,buffersize);

       int runs=0;
       do{
          runs = merge(f1,f2,g1,g2);
          swap(f1,g1);
          swap(f2,g2);
          phases++;
       } while(runs > 1);
       delete f2;
       f2 = 0;
       delete g1;
       g1 = 0;
       delete g2;
       g2 = 0;
    }

/*
3.12 merge

This function perform a single phase of merging f1 and f2. The merged
runs are written to g1 and g2.

*/
    int merge(fstream* f1, fstream* f2, fstream* g1, fstream* g2){
      f1->seekg(0);
      f2->seekg(0);
      g1->seekp(0);
      g2->seekp(0);
      hSegment f1Cur;
      hSegment f2Cur;
      int runs = 0;
      f1->read((char*) &f1Cur, sizeof(hSegment));
      f2->read((char*) &f2Cur, sizeof(hSegment));

      while(!f1Cur.isEnd() || !f2Cur.isEnd()){

        while(!f1Cur.isMarker() || !f2Cur.isMarker()){
           if(f1Cur.isMarker()){
             append(g1,f2Cur);
             f2->read((char*) &f2Cur, sizeof(hSegment));
           } else if(f2Cur.isMarker()){
             append(g1,f1Cur);
             f1->read((char*) &f1Cur, sizeof(hSegment));
           } else {
              if(f1Cur < f2Cur){
                  append(g1,f1Cur);
                 f1->read((char*) &f1Cur, sizeof(hSegment));
              } else {
               append(g1,f2Cur);
               f2->read((char*) &f2Cur, sizeof(hSegment));
              }
           }
        }
        // both element are markers
        append(g1,hSegment::getMarker());
        runs++;
        swap(g1,g2);
        if(!f1Cur.isEnd()){
           f1->read((char*) &f1Cur, sizeof(hSegment));
        }
        if(!f2Cur.isEnd()){
           f2->read((char*) &f2Cur, sizeof(hSegment));
        }
      }
      append(g1,hSegment::getEnd());
      append(g2,hSegment::getEnd());
      return runs ;
    }
};




class ContourLineLocalInfo2{
public:
  ContourLineLocalInfo2(const int _interval, ListExpr tupleType, size_t mem) :
      interval(_interval),pos(0){
     tt = new TupleType(tupleType);
     store = new segmentStorage(mem);
  }

  ~ContourLineLocalInfo2(){
     tt->DeleteIfAllowed();
     delete store;
  }

  void addSegment(int level, const Point& p1, const Point & p2){
     hSegment seg(level,p1,p2);
     store->insert(seg);
  }

  void finish(){
     store->finish();
  }

  Tuple* getNext() {

       if(!store->hasNext()){
         return 0;
       }
       LineType* line = new LineType(0);
       line->StartBulkLoad();
       hSegment seg = store->current();
       int level = seg.getLevel();
       int edge = 0;
       while(store->hasNext() && level == seg.getLevel()){
         insert(seg,line, edge);
         edge++;
         store->next();
         if(store->hasNext()){
            seg = store->current();
         }
       }
       line->EndBulkLoad();
       Tuple* result = new Tuple(tt);
       result->PutAttribute(0,new CcInt(true,level));
       result->PutAttribute(1,line);
       return result;
  }

  private:
    int interval;
    size_t pos;
   // vector<hSegment> v; 
    segmentStorage* store;
    TupleType* tt;

    void insert(const hSegment seg, LineType* line, int edgeno){
        HalfSegment hs(true,seg.getP1(), seg.getP2());
        hs.attr.edgeno=edgeno;
        (*line) += hs;
        hs.SetLeftDomPoint(false);
        (*line) += hs;
    }

};


/*
Method ProcessRectangle returns true if processing was successful

Parameters: values and coordinates of four points, the wanted interval, 
            the minimum value and DbArray ResultInfo to store the found 
            segments

*/
   template<class LI>
    bool ProcessRectangle(double, double, double,
                              double, double, double,
                              double, double, double,
                              double, double, double, 
                              int, LI*);

/*
Method AddSegment addes the found segments to the ResultInfo DbArray

Parameters: level value, coordinates of segments start and stop point, 
            the minimum value, the interval value and DbArray ResultInfo 
            to store the segments

*/
  template<class LI>
        void AddSegment(int, double, double,
                        double, double, LI*);



/*
Method contourFuns: calculates the contour lines for a given sint or sreal
                    object

Return value: stream of tuple (level, line)

*/
  template <typename T>
  int contourFun(Word* args, Word& result, 
                     int message, Word& local, Supplier s)
  {
    int returnValue = FAILURE;

    typename T::this_type* s_in =
          static_cast<typename T::this_type*>(args[0].addr);
    CcInt* lines = static_cast<CcInt*>(args[1].addr);


    typedef ContourLineLocalInfo2 LIT;
    LIT* li = (LIT*) local.addr;

    switch(message)
    {
      case OPEN:
      {
        // initialize the local storage
        double min = s_in->getMinimum();
        double max = s_in->getMaximum();
        if(   !lines->IsDefined() 
            || s_in->isUndefined(min) || s_in->isUndefined(max)){
           return 0;
        }
        
        int interval = lines->GetValue();
        
        if ( interval < 1 )
        {
          cmsg.error() << "Interval < 1 not allowed" << endl;
          cmsg.send();
          return CANCEL;
        }

        // double diff = max - min;
       // int num = ceil(diff / interval) + 1;

         

        if(li){ delete li; }
   //   li = new LIT(num,min, interval, 
   //                                     nl->Second(GetTupleResultType(s)));
         size_t mem;
#ifdef contourlines_fixed_cache
         mem = 64*1024*1024;
#else
         mem = qp->GetMemorySize(s)*1024*1024;
#endif         
         li = new LIT(interval, 
                                    nl->Second(GetTupleResultType(s)),
                                    mem);
        
        local.addr = li;

        raster2::grid2 grid = s_in->getGrid();
        Rectangle<2> bbox = s_in->bbox();

        double gridOriginX = grid.getOriginX();
        double gridOriginY = grid.getOriginY();
        double cellsize = grid.getLength();

        raster2::RasterIndex<2> from = 
                                grid.getIndex(bbox.MinD(0), bbox.MinD(1));
        raster2::RasterIndex<2> to = 
                                grid.getIndex(bbox.MaxD(0), bbox.MaxD(1));

        for (raster2::RasterIndex<2> index = from; index < to;
                                     index.increment(from, to))
        {
          // central cell
          double e = s_in->get(index);

          if (!(s_in->isUndefined(e)))
          {
            double a = s_in->get((int[]){index[0] - 1, index[1] + 1});
            double a1 = s_in->get((int[]){index[0] - 1, index[1] + 2});
            double b = s_in->get((int[]){index[0], index[1] + 1});
            double b1 = s_in->get((int[]){index[0], index[1] + 2});
            double c = s_in->get((int[]){index[0] + 1, index[1] + 1});
            double d = s_in->get((int[]){index[0] - 1, index[1]});
            double g = s_in->get((int[]){index[0] - 1, index[1] - 1});
            double h = s_in->get((int[]){index[0], index[1] - 1});
            double f = s_in->get((int[]){index[0] + 1, index[1]});

            // calculate coordinates
            double X = index[0] * cellsize + cellsize/2 + gridOriginX;
            double Y = index[1] * cellsize + cellsize/2 + gridOriginY;

            // if all four cells have valid values
            if (!(s_in->isUndefined(a)) && !(s_in->isUndefined(b)) && 
                !(s_in->isUndefined(d)) && !(s_in->isUndefined(e)))
            {
              // special case for bottom right cell
              if ((s_in->isUndefined(h)) && (s_in->isUndefined(f)))
              {
                ProcessRectangle(a, X - cellsize, Y + cellsize, 
                                 d, X - cellsize, Y - cellsize/2,
                                 e, X + cellsize/2, Y - cellsize/2, 
                                 b, X + cellsize/2, Y + cellsize, 
                                 interval, li);       
              }
              // special case for first row
              else if ((s_in->isUndefined(h)) && (s_in->isUndefined(g)))
              {
                ProcessRectangle(a, X - cellsize, Y + cellsize, 
                                 d, X - cellsize, Y - cellsize/2,
                                 e, X, Y - cellsize/2, 
                                 b, X, Y + cellsize, 
                                 interval, li);       
              }
              // special case for top right cell
              else if ((s_in->isUndefined(f)) && (s_in->isUndefined(a1)) 
                                              && (s_in->isUndefined(b1)))
              {
                ProcessRectangle(a, X - cellsize, Y + cellsize + cellsize/2,
                                 d, X - cellsize, Y,
                                 e, X + cellsize/2, Y, 
                                 b, X + cellsize/2, Y + cellsize+cellsize/2,
                                 interval, li);       
              }
              // special case for last column
              else if ((s_in->isUndefined(f)) && (s_in->isUndefined(c)))
              {
                ProcessRectangle(a, X - cellsize, Y + cellsize, 
                                 d, X - cellsize, Y,
                                 e, X + cellsize/2, Y, 
                                 b, X + cellsize/2, Y + cellsize, 
                                 interval, li);       
              }
              // special case for top row
              else if ((s_in->isUndefined(a1)) && (s_in->isUndefined(b1)))
              {
                ProcessRectangle(a, X - cellsize, Y + cellsize + cellsize/2, 
                                 d, X - cellsize, Y,
                                 e, X, Y, 
                                 b, X, Y + cellsize+cellsize/2,
                                 interval, li);       
              }
              // normal case
              else
              {
                ProcessRectangle(a, X - cellsize, Y + cellsize, 
                                 d, X - cellsize, Y,
                                 e, X, Y, 
                                 b, X, Y + cellsize, 
                                 interval, li);       
              }
            }
            else
            {
              // determine which cells have defined values, accumulate values
              // and divide through number of valid cells
              double sum = 0;
              int good = 0;
              double center = 0;

              if (!(s_in->isUndefined(a)))
              {
                sum += a;
                good++;
              }

              if (!(s_in->isUndefined(b)))
              {
                sum += b;
                good++;
              }

              if (!(s_in->isUndefined(d)))
              {
                sum += d;
                good++;
              }

              if (!(s_in->isUndefined(e)))
              {
                sum += e;
                good++;
              }

              center = sum / good;

              // calculate alternative values
              double top;
              double left;
              double right;
              double bottom;

              if(!(s_in->isUndefined(a)))
              {
                if(!(s_in->isUndefined(b)))
                  top = (a + b) / 2.0;
                else
                  top = a;

                if(!(s_in->isUndefined(d)))
                  left = (a + d) / 2.0;
                else
                  left = a;
              }
              else
              {
                if (!(s_in->isUndefined(b)))
                  top = b;
                else
                  top = e;

                if (!(s_in->isUndefined(d)))
                  left = d;
                else
                  left = e;
              }

              if(!(s_in->isUndefined(b)))
                right = (e + b) / 2.0;
              else
                right = e;
  
              if(!(s_in->isUndefined(d)))
                bottom = (e + d) / 2.0;
              else
                bottom = e;

              // if one cell is not defined
              // -> calculation with alternative values
              if (!(s_in->isUndefined(a)))
              {
                ProcessRectangle(a, X - cellsize, Y + cellsize, 
                                 left, X - cellsize, Y + cellsize/2,
                                 center, X - cellsize/2, Y + cellsize/2, 
                                 top, X - cellsize/2, Y + cellsize, 
                                 interval, li);
              }
          
              if (!(s_in->isUndefined(d)))
              {
                if ((s_in->isUndefined(f)) && (s_in->isUndefined(b)))
                {
                  // special case top right cell
                }
                else if (!(s_in->isUndefined(e)) && !(s_in->isUndefined(d)) &&
                          (s_in->isUndefined(a)) && !(s_in->isUndefined(b)))
                {
                  // special case cell under undefined cell
                  ProcessRectangle(left, X - cellsize, Y + cellsize/2, 
                                   d, X - cellsize, Y,
                                   bottom, X - cellsize/2, Y, 
                                   center, X - cellsize/2, Y + cellsize/2, 
                                   interval, li);
                }
                else if (!(s_in->isUndefined(e)) && !(s_in->isUndefined(d)) &&
                         !(s_in->isUndefined(a)) && (s_in->isUndefined(b)))
                {
                  // special case cell left under undefined cell
                  ProcessRectangle(left, X - cellsize, Y + cellsize/2, 
                                   d, X - cellsize, Y,
                                   bottom, X - cellsize/2, Y, 
                                   center, X - cellsize/2, Y + cellsize/2, 
                                   interval,  li);
                }
              }
          
              if (!(s_in->isUndefined(e)) && (s_in->isUndefined(h))
                                          && (s_in->isUndefined(d)))
              {
                // special case left bottom cell
              }
              else if (!(s_in->isUndefined(e)) && (s_in->isUndefined(a)) &&
                       !(s_in->isUndefined(b)) && (s_in->isUndefined(b1)))
              {
                // special case top left cell
                ProcessRectangle(b, X-cellsize/2, Y+cellsize+cellsize/2,
                                 bottom, X - cellsize/2, Y,
                                 e, X, Y, 
                                 b, X, Y + cellsize + cellsize/2, 
                                 interval,  li);
              }
              else if (!(s_in->isUndefined(e)) && (s_in->isUndefined(a))
                                               && (s_in->isUndefined(b)))
              {
                // special case top row
              }
              else if (!(s_in->isUndefined(e)) && !(s_in->isUndefined(f)))
              {
                // special case right top cell
                ProcessRectangle(center, X - cellsize/2, Y + cellsize/2, 
                                 bottom, X - cellsize/2, Y,
                                 e, X, Y, 
                                 right, X, Y + cellsize/2, 
                                 interval, li);
              }

              if (!(s_in->isUndefined(e)) && (s_in->isUndefined(a)) &&
                       !(s_in->isUndefined(b)) && (s_in->isUndefined(b1)) &&
                        (s_in->isUndefined(a1)))
              {
                // special case left top cell
              }
              else if (!(s_in->isUndefined(b)) && (s_in->isUndefined(h)))
              {
                ProcessRectangle(top, X - cellsize/2, Y + cellsize, 
                                 e, X - cellsize/2, Y - cellsize/2,
                                 e, X, Y - cellsize/2, 
                                 b, X, Y + cellsize, 
                                 interval, li);
              }
              else if (!(s_in->isUndefined(b)) && (s_in->isUndefined(d))
                                               && !(s_in->isUndefined(b1)))
              {
                ProcessRectangle(top, X - cellsize/2, Y + cellsize, 
                                 center, X - cellsize/2, Y + cellsize/2,
                                 right, X, Y + cellsize/2, 
                                 b, X, Y + cellsize, 
                                 interval, li);
              }
              else if (!(s_in->isUndefined(b)) && !(s_in->isUndefined(e))
                                               && (s_in->isUndefined(a)))
              {
                // special case right of undefined
                ProcessRectangle(top, X - cellsize/2, Y + cellsize, 
                                 center, X - cellsize/2, Y + cellsize/2,
                                 right, X, Y + cellsize/2, 
                                 b, X, Y + cellsize, 
                                 interval,  li);
              }
            }
          }//if e def
        }// for 
        li->finish();
        return 0;
      }
    
      case REQUEST:
      {
        result.addr = li?li->getNext():0;
        return result.addr?YIELD:CANCEL;

      }

      case CLOSE:
      {
          if(li){
             delete li;
             local.addr = 0;
          } 
          return 0;
      }

      default:
      {
        assert(false);
      }
    }
 
    return returnValue;
  }

/*
Method contourFuns: calculates the contour lines for a given stream of 
                    tint or treal objects

Return value: stream of tuple (level, line)

*/
  template <typename T, typename SourceTypeProperties>
  int contourFunTile(Word* args, Word& result, 
                     int message, Word& local, Supplier s)
  {
    int returnValue = FAILURE;

    CcInt* lines = static_cast<CcInt*>(args[1].addr);

    vector<Tuple*> current;
    vector<Tuple*> last;
    vector<Tuple*> next;
    Tuple* nextElement;
    double fromX;
    double fromY;
    double toX;
    double toY;
    double tileSize;
    double cellSize;
    bool firstTuple = true;
    bool readNextElement = true;
    bool newLine = false;
    bool skipNextRow;
    bool skipLastRow;
    int currentSize;
    int nextSize;
    int lastSize;
    int currentTuple = 0;

    int xDimensionSize = TileAlgebra::tProperties<char>::GetXDimensionSize();
    int yDimensionSize = TileAlgebra::tProperties<char>::GetYDimensionSize();

    int maxX = xDimensionSize - 1;
    int maxY = yDimensionSize - 1;
    //ContourLocalInfo* li = (ContourLocalInfo*) local.addr;
    ContourLineLocalInfo2* li = (ContourLineLocalInfo2*) local.addr;

    switch(message)
    {
      case OPEN:
      {

        if(!lines->IsDefined()){
            return 0;
        }
        int interval = lines->GetValue();

        // Check for valid interval        
        if ( interval < 1 )
        {
          cmsg.error() << "Interval < 1 not allowed" << endl;
          cmsg.send();
          return CANCEL;
        }

        Word elem;
        Tuple* tuple;
        T* s_in;

        typename SourceTypeProperties::TypeProperties::PropertiesType min;
        typename SourceTypeProperties::TypeProperties::PropertiesType max;

        qp->Open(args[0].addr);
        qp->Request(args[0].addr, elem);

        bool firstValue = true;

        // Get minimum and maximum values of complete tile
        while(qp->Received(args[0].addr))
        {
          tuple = (Tuple*)elem.addr;
          s_in = static_cast<T*>(tuple->GetAttribute(0));

          typename SourceTypeProperties::TypeProperties::PropertiesType minTemp;
          s_in->minimum(minTemp);
          typename SourceTypeProperties::TypeProperties::PropertiesType maxTemp;
          s_in->maximum(maxTemp);

          if (firstValue ||(minTemp < min) )
          {
            min = minTemp;
          }

          if (firstValue || (maxTemp > max) )
          {
            max = maxTemp;
          }

          firstValue = false;
          tuple->DeleteIfAllowed();
          qp->Request(args[0].addr, elem);
        }

        qp->Close(args[0].addr);

     //   double diff = max - min;
     //   int num = ceil(diff / interval) + 1;
        if(li) delete li;

     //   li = new ContourLocalInfo(num, min, interval, 
     //                                   nl->Second(GetTupleResultType(s)));
     size_t mem;
#ifdef contourlines_fixed_cache
         mem = 64*1024*1024;
#else
         mem = qp->GetMemorySize(s)*1024*1024;
#endif         
        li = new ContourLineLocalInfo2(interval, 
                                  nl->Second(GetTupleResultType(s)),
                                  mem);

        local.addr = li;

        qp->Open(args[0].addr);

        double gridOriginX;
        double gridOriginY;
        double lastOriginX;
        double lastOriginY;

        while (readNextElement == true)
        {
          qp->Request(args[0].addr, elem);

          if(qp->Received(args[0].addr))
          {
            tuple = (Tuple*)elem.addr;

            s_in = static_cast<T*>(tuple->GetAttribute(0));

            TileAlgebra::tgrid grid;
            s_in->getgrid(grid);
            gridOriginX = grid.GetX();
            gridOriginY = grid.GetY();
            cellSize = grid.GetLength();
            tileSize = cellSize * xDimensionSize;

            // read cells until Y coordinate changes
            if ( firstTuple || (gridOriginY == lastOriginY) )
            {
              if (!(firstTuple == true))
              {
                // if there is a gap between two read tiles, fill vector
                // with dummy tile
                while ((gridOriginX - lastOriginX) - tileSize > cellSize)
                {
                  TupleType *tupleType = tuple->GetTupleType();
                  Tuple* dummy = new Tuple( tupleType );
                  T* s_out = new T(true);
                  dummy->PutAttribute(0,s_out);

                  current.push_back(dummy);
                  lastOriginX = lastOriginX + tileSize;
                }
              }

              current.push_back(tuple);
              lastOriginX = gridOriginX;
              lastOriginY = gridOriginY;
              firstTuple = false;
            }
            else
            {
              readNextElement = false;
              firstTuple = true;
              next.push_back(tuple);
              lastOriginX = gridOriginX;
            }
          }
          else
          {
            readNextElement = false;
          }
        } // while currentLine


        currentSize = current.size();

        readNextElement = true;

        while (currentSize != 0)
        {
          while (readNextElement == true)
          {
            qp->Request(args[0].addr, elem);

            if(qp->Received(args[0].addr))
            {
              tuple = (Tuple*)elem.addr;

              s_in = static_cast<T*>(tuple->GetAttribute(0));

              TileAlgebra::tgrid grid;
              s_in->getgrid(grid);
              gridOriginX = grid.GetX();
              gridOriginY = grid.GetY();
              cellSize = grid.GetLength();

              // read cells until Y coordinate changes
              if ((gridOriginY == lastOriginY) || (firstTuple == true))
              {
                // if there is a gap between two read tiles, fill vector
                // with dummy tile
                while ((gridOriginX-lastOriginX) - tileSize > cellSize)
                {
                  TupleType *tupleType = tuple->GetTupleType();
                  Tuple* dummy = new Tuple( tupleType );
                  T* s_out = new T(true);
                  dummy->PutAttribute(0,s_out);

                  next.push_back(dummy);
                  lastOriginX = lastOriginX + tileSize;
                }

                next.push_back(tuple);
                lastOriginX = gridOriginX;
                lastOriginY = gridOriginY;
                firstTuple = false;
              }
              else
              {
                readNextElement = false;
                firstTuple = true;
                newLine = true;

                nextElement = tuple;
                lastOriginX = gridOriginX;
              }
            }
            else
            {
              readNextElement = false;
            }
          } // while NextLine

          int factorNext = 0;
          int factorLast = 0;

          skipNextRow = false;
          skipLastRow = false;

          nextSize = next.size();
          lastSize = last.size();

          Tuple* cTuple = current[0];
          T* c = static_cast<T*>(cTuple->GetAttribute(0));
          TileAlgebra::tgrid cGrid;
          c->getgrid(cGrid);
          double cGridOriginX = cGrid.GetX();
          double cGridOriginY = cGrid.GetY();

          if (nextSize > 0)
          {
            Tuple* nTuple = next[0];
            T* n = static_cast<T*>(nTuple->GetAttribute(0));
            TileAlgebra::tgrid nGrid;
            n->getgrid(nGrid);
            double nGridOriginX = nGrid.GetX();
            double nGridOriginY = nGrid.GetY();

            // calculate factor if tile rows starts at different coordinates
            if ((cGridOriginX - nGridOriginX) > 0)
            {
              while ((cGridOriginX - nGridOriginX) > 0)
              {
                factorNext++;
                nGridOriginX = nGridOriginX + tileSize;              
              }
            }
            else if ((cGridOriginX - nGridOriginX) < 0)
            {
              while ((cGridOriginX - nGridOriginX) < 0)
              {
                factorNext--;
                cGridOriginX = cGridOriginX + tileSize;              
              }
            }

            // check if one or more rows are missing
            if ((nGridOriginY - cGridOriginY) > tileSize)
            {
              skipNextRow = true;
            }          
          }

          if (lastSize > 0)
          {
            Tuple* lTuple = last[0];
            T* l = static_cast<T*>(lTuple->GetAttribute(0));
            TileAlgebra::tgrid lGrid;
            l->getgrid(lGrid);
            double lGridOriginX = lGrid.GetX();
            double lGridOriginY = lGrid.GetY();
            cGridOriginX = cGrid.GetX();
            cGridOriginY = cGrid.GetY();

            // calculate factor if tile rows starts at different coordinates
            if ((cGridOriginX - lGridOriginX) > 0)
            {
              while ((cGridOriginX - lGridOriginX) > 0)
              {
                factorLast++;
                lGridOriginX = lGridOriginX + tileSize;              
              }
            }
            else if ((cGridOriginX - lGridOriginX) < 0)
            {
              while ((cGridOriginX - lGridOriginX) < 0)
              {
                factorLast--;
                cGridOriginX = cGridOriginX + tileSize;              
              }
            }

            // check if one or more rows are missing
            if ((cGridOriginY - lGridOriginY) > tileSize)
            {
              skipLastRow = true;
            }          
          }

          while (currentTuple < currentSize)
          {
            tuple = current[currentTuple];

            if ((tuple == 0) || (tuple->GetNoAttributes() != 1))
            {
              while (tuple == 0)
              {
                currentTuple++;
                tuple = current[currentTuple];
              }

              while (tuple->GetNoAttributes() != 1)
              {
                currentTuple++;
                tuple = current[currentTuple];
              }
            }

            s_in = static_cast<T*>(tuple->GetAttribute(0));

            TileAlgebra::Index<2> from;
            TileAlgebra::Index<2> to;

            s_in->GetBoundingBoxIndexes(from, to);
            fromX = from[0];
            fromY = from[1];
            toX = to[0];
            toY = to[1];      

            TileAlgebra::tgrid grid;
            s_in->getgrid(grid);
              
            cellSize = grid.GetLength();

            gridOriginX = grid.GetX();
            gridOriginY = grid.GetY();

            for(int row = fromY; row <= toY; row++)
            {
              for(int column = fromX; column <= toX; column++)
              {
                TileAlgebra::Index<2> index((int[]){column, row});
  
                // central cell
                double e = s_in->GetValue(index);
 
                if(!(SourceTypeProperties::TypeProperties::IsUndefinedValue(e)))
                {
                  double a =
                    SourceTypeProperties::TypeProperties::GetUndefinedValue();
                  double a1 = 
                    SourceTypeProperties::TypeProperties::GetUndefinedValue();
                  double b = 
                    SourceTypeProperties::TypeProperties::GetUndefinedValue();
                  double b1 = 
                    SourceTypeProperties::TypeProperties::GetUndefinedValue();
                  double c = 
                    SourceTypeProperties::TypeProperties::GetUndefinedValue();
                  double d = 
                    SourceTypeProperties::TypeProperties::GetUndefinedValue();
                  double f = 
                    SourceTypeProperties::TypeProperties::GetUndefinedValue();
                  double g = 
                    SourceTypeProperties::TypeProperties::GetUndefinedValue();
                  double h = 
                    SourceTypeProperties::TypeProperties::GetUndefinedValue();

                  GetValuesContour<T, SourceTypeProperties>
                    (&a, &a1, &b, &b1, &c, &d, &f, &g, &h, row, column, 
                     currentTuple, s_in,
                     maxX, maxY, factorNext, factorLast, 
                     skipNextRow, skipLastRow,
                     current, next, last,
                     currentSize, nextSize, lastSize);

                  // calculate coordinates
                  double X = index[0] * cellSize + cellSize/2 + gridOriginX;
                  double Y = index[1] * cellSize + cellSize/2 + gridOriginY;

                  // if all four cells have valid values
                  if (!(SourceTypeProperties::TypeProperties::
                        IsUndefinedValue(a)) && 
                      !(SourceTypeProperties::TypeProperties::
                        IsUndefinedValue(b)) && 
                      !(SourceTypeProperties::TypeProperties::
                        IsUndefinedValue(d)) && 
                      !(SourceTypeProperties::TypeProperties::
                        IsUndefinedValue(e)))
                  {
                    // special case for bottom right cell
                    if ((SourceTypeProperties::TypeProperties::
                         IsUndefinedValue(h)) && 
                        (SourceTypeProperties::TypeProperties::
                         IsUndefinedValue(f)))
                    {
                      ProcessRectangle(a, X - cellSize, Y + cellSize, 
                                       d, X - cellSize, Y - cellSize/2,
                                       e, X + cellSize/2, Y - cellSize/2, 
                                       b, X + cellSize/2, Y + cellSize, 
                                       interval, li);       
                    }
                    // special case for first row
                    else if ((SourceTypeProperties::TypeProperties::
                              IsUndefinedValue(h)) && 
                             (SourceTypeProperties::TypeProperties::
                              IsUndefinedValue(g)))
                    {
                      ProcessRectangle(a, X - cellSize, Y + cellSize, 
                                       d, X - cellSize, Y - cellSize/2,
                                       e, X, Y - cellSize/2, 
                                       b, X, Y + cellSize, 
                                       interval, li);       
                    }
                    // special case for top right cell
                    else if ((SourceTypeProperties::TypeProperties::
                              IsUndefinedValue(f)) && 
                             (SourceTypeProperties::TypeProperties::
                              IsUndefinedValue(a1)) && 
                             (SourceTypeProperties::TypeProperties::
                              IsUndefinedValue(b1)))
                    {
                      ProcessRectangle(a, X - cellSize, Y+cellSize+cellSize/2,
                                       d, X - cellSize, Y,
                                       e, X + cellSize/2, Y, 
                                       b, X + cellSize/2, Y+cellSize+cellSize/2,
                                       interval, li);       
                    }
                    // special case for last column
                    else if ((SourceTypeProperties::TypeProperties::
                              IsUndefinedValue(f)) && 
                             (SourceTypeProperties::TypeProperties::
                              IsUndefinedValue(c)))
                    {
                      ProcessRectangle(a, X - cellSize, Y + cellSize, 
                                       d, X - cellSize, Y,
                                       e, X + cellSize/2, Y, 
                                       b, X + cellSize/2, Y + cellSize, 
                                       interval, li);       
                    }
                    // special case for top row
                    else if ((SourceTypeProperties::TypeProperties::
                              IsUndefinedValue(a1)) && 
                             (SourceTypeProperties::TypeProperties::
                              IsUndefinedValue(b1)))
                    {
                      ProcessRectangle(a, X - cellSize, Y + cellSize+cellSize/2,
                                       d, X - cellSize, Y,
                                       e, X, Y, 
                                       b, X, Y + cellSize + cellSize/2,
                                       interval, li);       
                    }
                    // normal case
                    else
                    {
                      ProcessRectangle(a, X - cellSize, Y + cellSize, 
                                       d, X - cellSize, Y,
                                       e, X, Y, 
                                       b, X, Y + cellSize, 
                                       interval, li);       
                    }
                  }
                  else
                  {  
                    // determine which cells have defined values, accumulate 
                    // values and divide through number of valid cells
                    double sum = 0;
                    int good = 0;
                    double center = 0;
      
                    if (!(SourceTypeProperties::TypeProperties::
                                                IsUndefinedValue(a)))
                    {
                      sum += a;
                      good++;
                    }
    
                    if (!(SourceTypeProperties::TypeProperties::
                                                 IsUndefinedValue(b)))
                    {
                      sum += b;
                      good++;
                    }
    
                    if (!(SourceTypeProperties::TypeProperties::
                                                IsUndefinedValue(d)))
                    {
                      sum += d;
                      good++;
                    }
    
                    if (!(SourceTypeProperties::TypeProperties::
                                                IsUndefinedValue(e)))
                    {
                      sum += e;
                      good++;
                    }
    
                    center = sum / good;
  
                    // calculate alternative values
                    double top;
                    double left;
                    double right;
                    double bottom;
  
                    if(!(SourceTypeProperties::TypeProperties::
                                               IsUndefinedValue(a)))
                    {
                      if(!(SourceTypeProperties::TypeProperties::
                                                 IsUndefinedValue(b)))
                        top = (a + b) / 2.0;
                      else
                        top = a;
  
                      if(!(SourceTypeProperties::TypeProperties::
                                                 IsUndefinedValue(d)))
                        left = (a + d) / 2.0;
                      else
                        left = a;
                    }
                    else
                    {
                      if (!(SourceTypeProperties::TypeProperties::
                                                  IsUndefinedValue(b)))
                        top = b;
                      else
                        top = e;

                      if (!(SourceTypeProperties::TypeProperties::
                                                  IsUndefinedValue(d)))
                        left = d;
                      else
                        left = e;
                    }
  
                    if(!(SourceTypeProperties::TypeProperties::
                                               IsUndefinedValue(b)))
                      right = (e + b) / 2.0;
                    else
                      right = e;
      
                    if(!(SourceTypeProperties::TypeProperties::
                                               IsUndefinedValue(d)))
                      bottom = (e + d) / 2.0;
                    else
                      bottom = e;
  
                    // if one cell is not defined
                    // -> calculation with alternative values
                    if (!(SourceTypeProperties::TypeProperties::
                          IsUndefinedValue(a)))
                    {
                      ProcessRectangle(a, X - cellSize, Y + cellSize, 
                                       left, X - cellSize, Y + cellSize/2,
                                       center, X - cellSize/2, Y + cellSize/2,
                                       top, X - cellSize/2, Y + cellSize, 
                                       interval, li);
                    }
              
                    if (!(SourceTypeProperties::TypeProperties::
                          IsUndefinedValue(d)))
                    {
                      if ((SourceTypeProperties::TypeProperties::
                           IsUndefinedValue(f)) && 
                          (SourceTypeProperties::TypeProperties::
                           IsUndefinedValue(b)))
                      {
                        // special case top right cell
                      }
                      else if (!(SourceTypeProperties::TypeProperties::
                                 IsUndefinedValue(e)) && 
                                !(SourceTypeProperties::TypeProperties::
                                 IsUndefinedValue(d)) &&
                                (SourceTypeProperties::TypeProperties::
                                 IsUndefinedValue(a)) && 
                               !(SourceTypeProperties::TypeProperties::
                                 IsUndefinedValue(b)))
                      {
                        // special case cell under undefined cell
                        ProcessRectangle(left, X - cellSize, Y + cellSize/2, 
                                         d, X - cellSize, Y,
                                         bottom, X - cellSize/2, Y, 
                                         center, X - cellSize/2, Y + cellSize/2,
                                         interval, li);
                      }
                      else if (!(SourceTypeProperties::TypeProperties::
                                 IsUndefinedValue(e)) && 
                               !(SourceTypeProperties::TypeProperties::
                                 IsUndefinedValue(d)) &&
                               !(SourceTypeProperties::TypeProperties::
                                 IsUndefinedValue(a)) && 
                                (SourceTypeProperties::TypeProperties::
                                 IsUndefinedValue(b)))
                      {
                        // special case cell left under undefined cell
                        ProcessRectangle(left, X - cellSize, Y + cellSize/2, 
                                         d, X - cellSize, Y,
                                         bottom, X - cellSize/2, Y, 
                                         center, X - cellSize/2, Y + cellSize/2,
                                         interval, li);
                      }
                    }
            
                    if (!(SourceTypeProperties::TypeProperties::
                          IsUndefinedValue(e)) && 
                         (SourceTypeProperties::TypeProperties::
                          IsUndefinedValue(h)) && 
                         (SourceTypeProperties::TypeProperties::
                          IsUndefinedValue(d)))
                    {
                      // special case left bottom cell
                    }
                    else if (!(SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(e)) && 
                              (SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(a)) &&
                             !(SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(b)) && 
                              (SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(b1)))
                    {
                      // special case top left cell
                      ProcessRectangle(b, X-cellSize/2, Y+cellSize+cellSize/2,
                                       bottom, X - cellSize/2, Y,
                                       e, X, Y, 
                                       b, X, Y + cellSize + cellSize/2, 
                                       interval, li);
                    }
                    else if (!(SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(e)) && 
                              (SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(a)) && 
                              (SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(b)))
                    {
                      // special case top row
                    }
                    else if (!(SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(e)) && 
                             !(SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(f)))
                    {
                      // special case right top cell
                      ProcessRectangle(center, X - cellSize/2, Y + cellSize/2, 
                                       bottom, X - cellSize/2, Y,
                                       e, X, Y, 
                                       right, X, Y + cellSize/2,   
                                       interval, li);
                    }

                    if (!(SourceTypeProperties::TypeProperties::
                          IsUndefinedValue(e)) && 
                         (SourceTypeProperties::TypeProperties::
                          IsUndefinedValue(a)) &&
                        !(SourceTypeProperties::TypeProperties::
                          IsUndefinedValue(b)) && 
                         (SourceTypeProperties::TypeProperties::
                          IsUndefinedValue(b1)) &&
                         (SourceTypeProperties::TypeProperties::
                          IsUndefinedValue(a1)))
                    {
                      // special case left top cell
                    }
                    else if (!(SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(b)) && 
                              (SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(h)))
                    {
                      ProcessRectangle(top, X - cellSize/2, Y + cellSize, 
                                       e, X - cellSize/2, Y - cellSize/2,
                                       e, X, Y - cellSize/2, 
                                       b, X, Y + cellSize, 
                                       interval,  li);
                    }
                    else if (!(SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(b)) && 
                              (SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(d)) && 
                             !(SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(b1)))
                    {
                      ProcessRectangle(top, X - cellSize/2, Y + cellSize, 
                                       center, X - cellSize/2, Y + cellSize/2,
                                       right, X, Y + cellSize/2, 
                                       b, X, Y + cellSize, 
                                       interval,  li);
                    }
                    else if (!(SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(b)) && 
                             !(SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(e)) && 
                              (SourceTypeProperties::TypeProperties::
                               IsUndefinedValue(a)))
                    {
                      // special case right of undefined
                      ProcessRectangle(top, X - cellSize/2, Y + cellSize, 
                                       center, X - cellSize/2, Y + cellSize/2,
                                       right, X, Y + cellSize/2, 
                                       b, X, Y + cellSize, 
                                       interval, li);
                    }
                  }
                }//if e def
              }// for
            }// for 

            currentTuple++;
          }

          // change of tile rows
          for(int i=0;i<last.size();i++){
             last[i]->DeleteIfAllowed();
          }

          last = current;
          current = next;
          next.clear();

          currentSize = current.size();

          if (newLine == true)
          {
            next.push_back(nextElement);
            newLine = false;
          }
          currentTuple = 0;
          readNextElement = true;
        }
        for(int i=0;i<last.size();i++){
           last[i]->DeleteIfAllowed();
        }
 
        li->finish();
        return 0;
      }
    
      case REQUEST:
      {
         result.addr=li?li->getNext():0;
         return result.addr?YIELD:CANCEL;
      }

      case CLOSE:
      {
        if(li){
           delete li;
           local.addr = 0;
        }
        return 0;
      }

      default:
      {
        assert(false);
      }
    }
 
    return returnValue;
  }
/*
declaration of contourFuns array

*/
  ValueMapping contourFuns[] =
  {
    contourFun<raster2::sint>,
    contourFun<raster2::sreal>,
    contourFunTile<TileAlgebra::tint, TileAlgebra::tProperties<int> >,
    contourFunTile<TileAlgebra::treal, TileAlgebra::tProperties<double> >,
    0
  };

/*
Value Mapping

*/
  int contourSelectFun(ListExpr args)
  {
    int nSelection = -1;
    
    NList type(args);

    if (type.first() == NList(raster2::sint::BasicType()))
    {
      return 0;
    }
    
    else if (type.first() == NList(raster2::sreal::BasicType()))
    {
      return 1;
    }

    ListExpr stream = nl->First(args);
    NList list = nl->Second(nl->First(nl->Second(nl->Second(stream))));

    if (list == TileAlgebra::tint::BasicType())
    {
      return 2;
    }

    if (list == TileAlgebra::treal::BasicType())
    {
      return 3;
    }

    return nSelection;
  }

/*
Type Mapping

*/
  ListExpr contourTypeMap(ListExpr args)
  {

    string error = "Expecting an sint, sreal or a stream of "
                   "tint or treal and an integer (interval).";

    NList type(args);

    ListExpr attrList=nl->TheEmptyList();
    
    ListExpr attr1 = nl->TwoElemList( nl->SymbolAtom("Height"),
                                     nl->SymbolAtom(CcInt::BasicType()));

    ListExpr attr2 = nl->TwoElemList( nl->SymbolAtom("Contour"),
                                     nl->SymbolAtom(LineType::BasicType()));

    attrList = nl->TwoElemList( attr1, attr2 );

    if(type.length() != 2)
    {
      return NList::typeError("two arguments required");
    }

    if ((type == NList(raster2::sint::BasicType(), CcInt::BasicType())) ||
        (type == NList(raster2::sreal::BasicType(), CcInt::BasicType())))
    {
      return nl->TwoElemList(nl->SymbolAtom(Stream<Tuple>::BasicType()),
                             nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                             attrList));
    }

    ListExpr stream = nl->First(args);
    NList attr = nl->Second(args);

    if(!listutils::isTupleStream(stream))
    {
      return listutils::typeError(error);
    }

    NList list = nl->Second(nl->First(nl->Second(nl->Second(stream))));

    if((list == TileAlgebra::tint::BasicType() && attr == CcInt::BasicType()) ||
       (list == TileAlgebra::treal::BasicType() && attr == CcInt::BasicType()))
    {
      return nl->TwoElemList(nl->SymbolAtom(Stream<Tuple>::BasicType()),
                             nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                             attrList));
    }

    return listutils::typeError(error);
  }

/*
Method ProcessRectangle returns true if processing was successful

Parameters: values and coordinates of four points, the wanted interval, 
            the minimum value and DbArray ResultInfo to store the found 
            segments

*/
  template<class LI>
  bool ProcessRectangle(double a, double aX, double aY,
                        double g, double gX, double gY,
                        double i, double iX, double iY,
                        double c, double cX, double cY, 
                        int interval,  
                        LI* li)
  {
    // calculate minimum and maximum
    double Min = MIN(MIN(a,c),MIN(g,i));
    double Max = MAX(MAX(a,c),MAX(g,i));

    int startLevel = (int) floor(Min / interval);
    int endLevel = (int) ceil(Max / interval);

    // calculate intersection
    for(int iLevel = startLevel; iLevel <= endLevel; iLevel++)
    {
      int level = iLevel * interval;

      int nPoints = 0; 
      int nPoints1 = 0, nPoints2 = 0, nPoints3 = 0;
      double pointsX[4], pointsY[4];

      Intersect( a, aX, aY, g, gX, gY, i, 
                 level, &nPoints, pointsX, pointsY );
      nPoints1 = nPoints;

      Intersect( g, gX, gY, i, iX, iY, c,
                 level, &nPoints, pointsX, pointsY );
      nPoints2 = nPoints;

      Intersect( i, iX, iY, c, cX, cY, a,
                 level, &nPoints, pointsX, pointsY );
      nPoints3 = nPoints;

      Intersect( c, cX, cY, a, aX, aY, g,
                 level, &nPoints, pointsX, pointsY );

      if( nPoints == 2 )
      {
        // left and bottom
        if ( nPoints1 == 1 && nPoints2 == 2)
        {
          if ( !(g == level && i == level) )
            AddSegment( level, pointsX[0], pointsY[0], 
                        pointsX[1], pointsY[1], li);
        }
        // left and right
        else if ( nPoints1 == 1 && nPoints3 == 2 )
        {
          AddSegment( level, pointsX[0], pointsY[0], 
                      pointsX[1], pointsY[1], li);
        }
        // left and top
        else if ( nPoints1 == 1 && nPoints == 2 )
        { 
          AddSegment( level, pointsX[0], pointsY[0], 
                      pointsX[1], pointsY[1], li);
        }
        // bottom and right
        else if(  nPoints2 == 1 && nPoints3 == 2)
        {
          if ( !(c == level && i == level) )
            AddSegment( level, pointsX[0], pointsY[0], 
                        pointsX[1], pointsY[1], li);
        }
        // bottom and top
        else if ( nPoints2 == 1 && nPoints == 2 )
        {
          AddSegment( level, pointsX[0], pointsY[0], 
                      pointsX[1], pointsY[1], li);
        }
        // right and top
        else if ( nPoints3 == 1 && nPoints == 2 )
        { 
           AddSegment( level, pointsX[0], pointsY[0], 
                       pointsX[1], pointsY[1], li);
        }
        else
        {
          return error;
        }
      }

      if( nPoints == 3 )
      {
        // left, bottom and right
        if ( nPoints1 == 1 && nPoints2 == 2 && nPoints3 == 3 )
        {
          AddSegment( level, pointsX[0], pointsY[0], 
                      pointsX[1], pointsY[1], li);

          AddSegment( level, pointsX[1], pointsY[1], 
                      pointsX[2], pointsY[2], li);
        }
        // left, bottom and top
        else if ( nPoints1 == 1 && nPoints2 == 2 && nPoints == 3 )
        {
          AddSegment( level, pointsX[0], pointsY[0], 
                      pointsX[1], pointsY[1],  li);

          AddSegment( level, pointsX[0], pointsY[0], 
                      pointsX[2], pointsY[2],  li);
        }
        // bottom, right and top
        else if ( nPoints2 == 1 && nPoints3 == 2 && nPoints == 3 )
        {
          AddSegment( level, pointsX[0], pointsY[0], 
                      pointsX[1], pointsY[1], li);

          AddSegment( level, pointsX[1], pointsY[1], 
                      pointsX[2], pointsY[2], li);
        }
        // left, right and top
        else if ( nPoints1 == 1 && nPoints3 == 2 && nPoints == 3 )
        {
          AddSegment( level, pointsX[0], pointsY[0], 
                      pointsX[1], pointsY[1], li);

          AddSegment( level, pointsX[1], pointsY[1], 
                      pointsX[2], pointsY[2], li);
        }
      }

      if( nPoints == 4 )
      {
        if ( !(c == level && a == level) )
        {
          AddSegment( level, pointsX[1], pointsY[1], 
                      pointsX[2], pointsY[2], li);
          AddSegment( level, pointsX[0], pointsY[0], 
                      pointsX[3], pointsY[3], li);
        }
      }
    } 
    return false;
  }

/*
Method Intersects calculates if a line between two points intersects the level

Parameters: values and coordinates of two points, value of a third point, 
            the level value, a pointer for a counter to store the number 
            of intersect and two pointer two store the point where the line is
            intersected

*/
  void Intersect(double val1, double val1X, double val1Y,
                 double val2, double val2X, double val2Y,
                 double val3, double level, int *pnPoints,
                 double *ppointsX, double *ppointsY )
  {
    if( val1 < level && val2 >= level )
    {
      double diff = (level - val1) / (val2 - val1);

      ppointsX[*pnPoints] = val1X * (1.0 - diff) + val2X * diff;
      ppointsY[*pnPoints] = val1Y * (1.0 - diff) + val2Y * diff;
      (*pnPoints)++;
    }
    else if( val1 > level && val2 <= level )
    {
      double diff = (level - val2) / (val1 - val2);

      ppointsX[*pnPoints] = val2X * (1.0 - diff) + val1X * diff;
      ppointsY[*pnPoints] = val2Y * (1.0 - diff) + val1Y * diff;
      (*pnPoints)++;
    }
    else if( val1 == level && val2 == level && val3 != level )
    {
      ppointsX[*pnPoints] = val2X;
      ppointsY[*pnPoints] = val2Y;
      (*pnPoints)++;
    }
  }

/*
Method AddSegment addes the found segments to the ResultInfo DbArray

Parameters: level value, coordinates of segments start and stop point, 
            the minimum value, the interval value and DbArray ResultInfo 
            to store the segments

*/
  template<class LI>
  void  AddSegment(int l, double startX, double startY,
                  double endX, double endY, LI* clines)
  {
    Point p1(true, startX, startY);
    Point p2(true, endX, endY);
    clines->addSegment(l,p1,p2);    
  }

/*
Method GetValuesContour reads the values for 3x3 cells

parameters:
   a - reference to top left cell \\
   a1 - reference to top left cell + 1 \\
   b - reference to top middle cell \\
   b1 - reference to top middle cell + 1 \\
   c - reference to top right cell \\
   d - reference to middle left cell \\
   f - reference to middle right cell \\
   g - reference to bottom left cell \\
   h - reference to bottom right cell \\
   row - number of current row \\
   column - number of current column \\
   currentTuple - number of current tuple \\
   s\_in - current tuple \\
   maxX - maximum X in a tuple \\
   maxY - maximum Y in a tuple \\
   factorNext - if vector current and next have different start points \\
   factorlast - if vector current and last have different start points \\
   skipNextRow - if difference between next and current is more 
   than one tile \\
   skipLastRow - if difference between last and current is more 
   than one tile \\
   current - current vector \\
   next - next vector \\
   last - last vector \\
   currentSize - size of current vector \\
   nextSize - size of next vector \\
   lastSize - size of last vector \\
return value: -
exceptions: -

*/

  template <typename T, typename SourceTypeProperties>
  void GetValuesContour(double* a, double* a1, double* b, double* b1, double* c,
               double* d, double* f, double* g, double* h,
               int row, int column, int currentTuple, T* s_in,
               int maxX, int maxY,
               int factorNext, int factorLast,
               bool skipNextRow, bool skipLastRow,
               vector<Tuple*> current, vector<Tuple*> next,
               vector<Tuple*> last, 
               int currentSize, int nextSize, int lastSize)
  {
    Tuple* tuple_help;
    T* s_in_help;

    // left lower corner
    if ((column == 0) && (row == 0))
    {
      if (currentTuple > 0)
      {
        tuple_help = current[currentTuple - 1];

        if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
        {
          s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

          *a = s_in_help->GetValue((int[]){maxX, 1});
          *a1 = s_in_help->GetValue((int[]){maxX, 2});
          *d = s_in_help->GetValue((int[]){maxX, 0});
        }
      }

      *b = s_in->GetValue((int[]){column, row + 1});
      *b1 = s_in->GetValue((int[]){column, row + 2});
      *c = s_in->GetValue((int[]){column + 1, row + 1});
      *f = s_in->GetValue((int[]){column + 1, row});
  
      if ((lastSize > 0) && (skipLastRow == false))
      {
        if ((currentTuple + factorLast > 0) &&
            (currentTuple + factorLast - 1 < lastSize))
        {  
          tuple_help = last[currentTuple - 1 + factorLast];
  
          if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
          {
            s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));
  
            *g = s_in_help->GetValue((int[]){maxX, maxY});
          }
        }
  
        if ((currentTuple + factorLast >= 0) &&
            (currentTuple + factorLast < lastSize))
        {
          tuple_help = last[currentTuple + factorLast];
  
          if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
          {
            s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));
  
            *h = s_in_help->GetValue((int[]){0, maxY});
          }
        }
      }
    }
    // left upper corner - 1
    else if ((column == 0) && (row == maxY - 1))
    {
      if ((nextSize > 0) && (skipNextRow == false))
      {
        if ((currentTuple + factorNext > 0) &&
            (currentTuple + factorNext - 1 < nextSize))
        {  
          tuple_help = next[currentTuple - 1 + factorNext];
  
          if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
          {
            s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));
  
            *a1 = s_in_help->GetValue((int[]){maxX, 0});
          }
        }
  
        if ((currentTuple + factorNext >= 0) &&
            (currentTuple + factorNext < nextSize))
        {
          tuple_help = next[currentTuple + factorNext];

          if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
          {
            s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

            *b1 = s_in_help->GetValue((int[]){0, 0});
          }
        }
      }

      if (currentTuple > 0)
      {
        tuple_help = current[currentTuple - 1];
  
        if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
        {
          s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));
  
          *a = s_in_help->GetValue((int[]){maxX, maxY});
          *d = s_in_help->GetValue((int[]){maxX, maxY - 1});
          *g = s_in_help->GetValue((int[]){maxX, maxY - 2});
        }
      }
  
      *b = s_in->GetValue((int[]){column, row + 1});
      *c = s_in->GetValue((int[]){column + 1, row + 1});
      *f = s_in->GetValue((int[]){column + 1, row});
      *h = s_in->GetValue((int[]){column, row - 1});
    }
    // left upper corner  
    else if ((column == 0) && (row == maxY))
    {
      if ((nextSize > 0) && (skipNextRow == false))
      {
        if ((currentTuple + factorNext > 0) &&
            (currentTuple + factorNext - 1 < nextSize))
        {  
          tuple_help = next[currentTuple - 1 + factorNext];
  
          if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
          {
            s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));
  
            *a = s_in_help->GetValue((int[]){maxX, 0});
            *a1 = s_in_help->GetValue((int[]){maxX, 1});
          }
        }
  
        if ((currentTuple + factorNext >= 0) &&
            (currentTuple + factorNext < nextSize))
        {
          tuple_help = next[currentTuple + factorNext];
  
          if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
          {
            s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

            *b = s_in_help->GetValue((int[]){0, 0});
            *b1 = s_in_help->GetValue((int[]){0, 1});
            *c = s_in_help->GetValue((int[]){1, 0});
          }
        }
      }

      if (currentTuple > 0)
      {
        tuple_help = current[currentTuple - 1];

        if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
        {
          s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

          *d = s_in_help->GetValue((int[]){maxX, maxY});
          *g = s_in_help->GetValue((int[]){maxX, maxY - 1});
        }
      }

      *f = s_in->GetValue((int[]){column + 1, row});
      *h = s_in->GetValue((int[]){column, row - 1});
    }
    // right lower corner
    else if ((column == maxX) && (row == 0))
    {
      *a = s_in->GetValue((int[]){column - 1, row + 1});
      *a1 = s_in->GetValue((int[]){column - 1, row + 2});
      *b = s_in->GetValue((int[]){column, row + 1});
      *b1 = s_in->GetValue((int[]){column, row + 2});
      *d = s_in->GetValue((int[]){column - 1, row});

      if (currentTuple + 1 < currentSize)
      {
        tuple_help = current[currentTuple + 1];

        if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
        {
          s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

          *c = s_in_help->GetValue((int[]){0, 1});
          *f = s_in_help->GetValue((int[]){0, 0});
        }
      }

      if ((lastSize > 0) && (skipLastRow == false))
      {
        if ((currentTuple + factorLast >= 0) &&
            (currentTuple + factorLast < lastSize))
        {
          tuple_help = last[currentTuple + factorLast];

          if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
          {
            s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

            *g = s_in_help->GetValue((int[]){maxX - 1, maxY});
            *h = s_in_help->GetValue((int[]){maxX, maxY});
          }
        }
      }
    }
    // right upper corner - 1
    else if ((column == maxX) && (row == maxY - 1))
    {
      if ((nextSize > 0) && (skipNextRow == false))
      {
        if ((currentTuple + factorNext >= 0) &&
            (currentTuple + factorNext < nextSize))
        {
          tuple_help = next[currentTuple + factorNext];

          if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
          {  
            s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

            *a1 = s_in_help->GetValue((int[]){maxX - 1, 0});
            *b1 = s_in_help->GetValue((int[]){maxX, 0});
          }
        }
      }

      *a = s_in->GetValue((int[]){column - 1, row + 1});
      *b = s_in->GetValue((int[]){column, row + 1});
      *d = s_in->GetValue((int[]){column - 1, row});
      *g = s_in->GetValue((int[]){column - 1, row - 1});
      *h = s_in->GetValue((int[]){column, row - 1});

      if (currentTuple + 1 < currentSize)
      {
        tuple_help = current[currentTuple + 1];

        if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
        {
          s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));
  
          *c = s_in_help->GetValue((int[]){0, maxY});
          *f = s_in_help->GetValue((int[]){0, maxY - 1});
        }
      }
    }
    // right upper corner
    else if ((column == maxX) && (row == maxY))
    {
      if ((nextSize > 0) && (skipNextRow == false))
      {
        if ((currentTuple + factorNext >= 0) &&
            (currentTuple + factorNext < nextSize))
        {
          tuple_help = next[currentTuple + factorNext];

          if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
          {
            s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

            *a = s_in_help->GetValue((int[]){maxX - 1, 0});
            *a1 = s_in_help->GetValue((int[]){maxX - 1, 1});
            *b = s_in_help->GetValue((int[]){maxX, 0});
            *b1 = s_in_help->GetValue((int[]){maxX, 1});
          }  
        }

        if ((currentTuple + factorNext >= 0) &&
            (currentTuple + factorNext + 1 < nextSize))
        {  
          tuple_help = next[currentTuple + 1 + factorNext];

          if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
          {
            s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

            *c = s_in_help->GetValue((int[]){0, 0});
          }
        }
      }

      *d = s_in->GetValue((int[]){column - 1, row});
      *g = s_in->GetValue((int[]){column - 1, row - 1});
      *h = s_in->GetValue((int[]){column, row - 1});

      if (currentTuple + 1 < currentSize)
      {
        tuple_help = current[currentTuple + 1];

        if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
        {
          s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

          *f = s_in_help->GetValue((int[]){0, maxY});
        }
      }
    }
    // left column
    else if (column == 0)
    {
      *b = s_in->GetValue((int[]){column, row + 1});
      *b1 = s_in->GetValue((int[]){column, row + 2});
      *c = s_in->GetValue((int[]){column + 1, row + 1});
      *f = s_in->GetValue((int[]){column + 1, row});  
      *h = s_in->GetValue((int[]){column, row - 1});

      if (currentTuple > 0)
      {
        tuple_help = current[currentTuple - 1];

        if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
        {
          s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

          *a = s_in_help->GetValue((int[]){maxX, row + 1});
          *a1 = s_in_help->GetValue((int[]){maxX, row + 2});
          *d = s_in_help->GetValue((int[]){maxX, row});
          *g = s_in_help->GetValue((int[]){maxX, row - 1});
        }
      }
    }
    // lower row
    else if (row == 0)
    {
      *a = s_in->GetValue((int[]){column - 1, row + 1});
      *a1 = s_in->GetValue((int[]){column - 1, row + 2});
      *b = s_in->GetValue((int[]){column, row + 1});
      *b1 = s_in->GetValue((int[]){column, row + 2});
      *c = s_in->GetValue((int[]){column + 1, row + 1});
      *d = s_in->GetValue((int[]){column - 1, row});
      *f = s_in->GetValue((int[]){column + 1, row});

      if ((lastSize > 0)  && (skipLastRow == false))
      {
        if ((currentTuple + factorLast >= 0) &&
            (currentTuple + factorLast < lastSize))
        {
          tuple_help = last[currentTuple + factorLast];
  
          if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
          {
            s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

            *g = s_in_help->GetValue((int[]){column - 1, maxY});
            *h = s_in_help->GetValue((int[]){column, maxY});
          }
        }
      }
    }
    // right column
    else if (column == maxX)
    {
      *a = s_in->GetValue((int[]){column - 1, row + 1});
      *a1 = s_in->GetValue((int[]){column - 1, row + 2});
      *b = s_in->GetValue((int[]){column, row + 1});
      *b1 = s_in->GetValue((int[]){column, row + 2});
      *d = s_in->GetValue((int[]){column - 1, row});
      *g = s_in->GetValue((int[]){column - 1, row - 1});
      *h = s_in->GetValue((int[]){column, row - 1});

      if (currentTuple + 1 < currentSize)
      {
        tuple_help = current[currentTuple + 1];

        if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
        {
          s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

          *c = s_in_help->GetValue((int[]){0, row + 1});
          *f = s_in_help->GetValue((int[]){0, row});
        }
      }
    }
    // upper row - 1
    else if (row == maxY - 1)
    {
      *a = s_in->GetValue((int[]){column - 1, row + 1});
      *b = s_in->GetValue((int[]){column, row + 1});
      *c = s_in->GetValue((int[]){column + 1, row + 1});
      *d = s_in->GetValue((int[]){column - 1, row});
      *f = s_in->GetValue((int[]){column + 1, row});
      *g = s_in->GetValue((int[]){column - 1, row - 1});
      *h = s_in->GetValue((int[]){column, row - 1});

      if ((nextSize > 0) && (skipNextRow == false))
      {
        if ((currentTuple + factorNext >= 0) &&
            (currentTuple + factorNext < nextSize))
        {
          tuple_help = next[currentTuple + factorNext];
  
          if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
          {
            s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));
  
            *a1 = s_in_help->GetValue((int[]){column - 1, 0});
            *b1 = s_in_help->GetValue((int[]){column, 0});
          }
        }
      }
    }
    // upper row
    else if (row == maxY)
    {
      *d = s_in->GetValue((int[]){column - 1, row});
      *f = s_in->GetValue((int[]){column + 1, row});
      *g = s_in->GetValue((int[]){column - 1, row - 1});
      *h = s_in->GetValue((int[]){column, row - 1});

      if ((nextSize > 0) && (skipNextRow == false))
      {
        if ((currentTuple + factorNext >= 0) &&
            (currentTuple + factorNext < nextSize))
        {
          tuple_help = next[currentTuple + factorNext];

          if ((tuple_help != 0) && (tuple_help->GetNoAttributes() == 1))
          {
            s_in_help = static_cast<T*>(tuple_help->GetAttribute(0));

            *a = s_in_help->GetValue((int[]){column - 1, 0});
            *a1 = s_in_help->GetValue((int[]){column - 1, 1});
            *b = s_in_help->GetValue((int[]){column, 0});
            *b1 = s_in_help->GetValue((int[]){column, 1});
            *c = s_in_help->GetValue((int[]){column + 1, 0});
          }
        }
      }
    }
    // no border cells
    else
    {
      *a = s_in->GetValue((int[]){column - 1, row + 1});
      *a1 = s_in->GetValue((int[]){column - 1, row + 2});
      *b = s_in->GetValue((int[]){column, row + 1});
      *b1 = s_in->GetValue((int[]){column, row + 2});
      *c = s_in->GetValue((int[]){column + 1, row + 1});  
      *d = s_in->GetValue((int[]){column - 1, row});
      *f = s_in->GetValue((int[]){column + 1, row});
      *g = s_in->GetValue((int[]){column - 1, row - 1});
      *h = s_in->GetValue((int[]){column, row - 1});
    }
  }
}
