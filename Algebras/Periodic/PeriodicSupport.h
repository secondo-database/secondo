/*

//[_] [\_]

1 Header File for supporting functions.

1.1 Function Declarations


*/

#ifndef PERIODIC_SUPPORT_H
#define PERIODIC_SUPPORT_H

#include "NestedList.h"
#include <iostream>


/* 
~[_][_]TRACE[_][_]~

This macro is for debugging purposes. At the begin of all functions should the
[_][_]TRACE[_][_] symbol.

*/
//#define TRACEON 
#ifdef TRACEON
#define __TRACE__ cout << __POS__ << endl;
#else
#define __TRACE__
#endif

#define TTRACE 
#ifdef TTRACE
#define __TTRACE__ cout << __POS__ << endl;
#else
#define __TTRACE__
#endif


// the __POS__ macro can be used in debug-messages
#define __POS__ __FILE__ << ".." << __PRETTY_FUNCTION__ << "@" << __LINE__ 



namespace periodic {

static const double EPSILON = 0.00001;

/*
~About~

Fuzzy check for equalitty

*/
bool About(const double a, const double b);

/*
~Signum~

returns the signum of the argument. 

*/
int Signum(const double arg);


/*
~Distance~

Computes the distance between the points defined by (~Ax~, ~Ay~)
and the point defined by (~Bx~, ~By~).

*/
double Distance(const double Ax, const double Ay,
                const double Bx, const double By);

/*
~SignedArea~

Computes the area of the triangle defined by the points ~P~, ~Q~, ~R~.

*/
double SignedArea(const double Px, const double Py,
                  const double Qx, const double Qy,
                  const double Rx, const double Ry);


/*
~PointOnSegment~

Checks wether the point ~P~ is located on the segment (~A~, ~B~). 

*/
bool PointOnSegment(const double Px, const double Py,
                    const double Ax, const double Ay,
                    const double Bx, const double By);

/*
~PointPosOnSegment~

Returns the relative position of the point (x, y) to the segment
((x1,y1) [->] (x2,y2)) in range [0,1]. If the point is not located
on the segment, the result will be -1.

*/
double PointPosOnSegment(double x1, double y1, 
                         double x2, double y2, 
                         double x, double y);

/*
~heapsort~

Sorts an array of type ~T~.

*/
template <typename T> 
void heapsort(const int size, T values[]);

/*
~find~

Returns the position of ~elem~ in the ordered array ~field~ using binary
search. If ~elem~ was not found, the reuslt is -1.

*/

template <typename T>
int find(const int size,const T field[],const T elem);

/*
~GetNumeric~

Reads a numeric value form ~List~. The list may be an integer, a real
or a rational. If the list does not represent a numeric value, the
result will be false.

*/
bool GetNumeric(const ListExpr List, double &value);

/*
~WriteListExprToStream~

Writes ~L~ to ~os~.

*/
void WriteListExprToStream(ostream &os, const ListExpr L);


/*
1.2 Definition of template functions


*/


/* 
~ReHeap~

This function performs a reheap of a given (partially) heap.
Is a support function for heapsort. This function places the
element at position in in __values__ at the right place 
according to the definition of a heap where the heap itself is
given by the range i ... k in the __values__ array.


[3]   log(k-i)


*/
template <typename T>
void reheap(T values[], int i, int k){
  int j,son;
   j = i;
   bool done = false; 
   while(!done){
     if( 2*j > k ){ // end of array reached
         done = true;
     }
     else{ if(2*j+1<=k){
               if(values[2*j-1]<values[2*j])
                   son = 2*j;
               else
                   son = 2*j+1;
           }else {
                son = 2*j;
           }
           if(values[j-1]>values[son-1]){
              // swap values
              T tmp = values[j-1];
              values[j-1]=values[son-1];
              values[son-1] = tmp;
              j = son;
           }else{ // final position reached
               done = true;
           }
     }
   } 
}
/*
~HeapSort~

This function sorts an array with elements supporting
comparisons. After calling this function the elements in
the array are sorted in decreasing order.

[3]   O(n log(n)) where n is the number of elements in the array.


*/
template <typename T> 
void heapsort(const int size, T values[]){
   int n = size;
    int i;
    for (i=n/2;i>=1;i--)
         reheap(values,i,n);
    for(i=n;i>=2;i--){
       T tmp = values[0];
       values[0] = values[i-1];
       values[i-1] = tmp;
       reheap(values,1,i-1);
    }
}

/*
~find~

This function finds an entry in an sorted array applying 
binary search. If nothing is found, -1 is returned otherwise
the first index of a matching element.

*/
template <typename T>
int find(const int size,const T field[],const T elem){
  int min=0;
   int max=size;
   int mid=0;
   bool found = false;
   while(min<max && !found){
     mid = (min+max)/2;
     if(field[mid]>elem)
         max=mid-1;
     else if(field[mid]<elem)
         min=mid+1;
     else
         found=true;
   }
   if(!found) return -1;
   // At this point we know that mid is the index of one
   // element equals to *elem*. Now we have to find the 
   // smallest index.
   max = mid;
   while(max!=min){
     mid=(max+min)/2;
     if(field[mid]<elem)
        min=mid+1;
     else
        max=mid;
   } 
   return max;
}



} // end of namespace periodic


#endif
