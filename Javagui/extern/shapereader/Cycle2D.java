//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

package extern.shapereader;

import sj.lang.ListExpr;
import java.awt.geom.GeneralPath;
import java.util.Vector;

public class Cycle2D{


public Cycle2D(){
   start=null;
   end = null;
   current = null;
   minX = null;
   NumberOfElements = 0;
   x_min=x_max=y_min=y_max=0.0;
}


/**appends a new point,
  returns false if the first point or the last point in this cycle is
  equals to (x,y)
  */
public boolean append(double x, double y){
  if(start==null){  // the first Element
    Element E = new Element(x,y);
    start = E;
    end = E;
    minX = E;
    current = E;
    NumberOfElements = 1;
    x_min = x_max = x;
    y_min = y_max = y;
    return true;
  }
  Point2D P = new Point2D(x,y);
  // a cycle is atomatic closed and can not contains zero length segments
  if(P.equals(start.P) || P.equals(end.P))
     return false;

  Element E = new Element(P);
  E.prev = end;
  end.next = E;
  end = E;
  if(x<minX.P.getX()){  // a new minX Element
      minX=E;
  }
  // update the bounding box
  x_min = Math.min(x_min,x);
  y_min = Math.min(y_min,y);
  x_max = Math.max(x_max,x);
  y_max = Math.max(y_max,y);
  NumberOfElements++;
  return true;
}

// returns true if this cycle contains least 3 elements
public boolean isValidPolygon(){
  return NumberOfElements>2;
}


/** returns the x-coordinate from the first point in this cycle*/
public double getFirstX(){
  if(start==null)
     return 0;
  else
     return start.P.getX();
}


/** returns the y-coordinate from the first point in this cycle*/
public double getFirstY(){
  if(start==null)
     return 0;
  else
     return start.P.getY();
}



// returns true if the given point is contained in this area defined by this cycle
public boolean contains(double x, double y){

  if(NumberOfElements<3)
     return false;

 if(x<x_min || x>x_max || y<y_min || y>y_max) // outside from  bounding box
      return false;

  GeneralPath GP = new GeneralPath(); // later contruct GP with this cycle ??
  Element E = start;
  GP.moveTo((float)E.P.getX(),(float)E.P.getY());
  E = E.next;
  while(E!=null){
    GP.lineTo((float)E.P.getX(),(float)E.P.getY());
    E=E.next;
  }
  GP.closePath();
  return GP.contains(x,y);
}


// returns the List representation of this cycle
public ListExpr getList(){
  if(start==null)
     return ListExpr.theEmptyList();

  ListExpr LE = ListExpr.oneElemList(start.P.getList());
  ListExpr Last = LE;
  Element E = start.next;
  while(E!=null){
     Last = ListExpr.append(Last,E.P.getList());
     E = E.next;
  }
  return LE;
}


public int getDirection(){
   if(NumberOfElements<3)
      return NO_DIRECTION;

   Point2D P1,P2,P3;
   P2 = minX.P;
   P1 = minX.prev==null?end.P:minX.prev.P;
   P3 = minX.next==null?start.P:minX.next.P;

   double x1=P1.getX(),x2=P2.getX(),x3=P3.getX();
   double y1=P1.getY(),y2=P2.getY(),y3=P3.getY();

   // P1->P2 is vertical
   if(x1==x2){
      if(y1>y2)
         return COUNTERCLOCKWISE;
      else
         return CLOCKWISE;
   }

   // P2->P3 is vertical
   if(x2==x3){
      if(y2>y3)
         return COUNTERCLOCKWISE;
      else
         return CLOCKWISE;
   }

   double a2_1 = (y1-y2)/(x1-x2);
   double a2_3 = (y3-y2)/(x3-x2);

   if(a2_1>a2_3)
      return  COUNTERCLOCKWISE;
   else
      return CLOCKWISE;
}

/*
This function checks wether the cycle contains selfcuts and
creates a set of cycles without selfcuts.
*/
public Cycle2D[] split(){
   Vector result = new Vector();
   Cycle2D StartCycle = new Cycle2D();
   current = start;
   Element C = start;
   int count = 0;
   while(C!=null){
       count++;
       Point2D P = C.P;
       C = C.next;
       // search this point in StartCycle
       Element SCE = StartCycle.start;
       while(SCE!=null){
          if(SCE.P.equals(P)){ // point is repeated // cut out the subcycle
             // cut out the subcycle
             Cycle2D SubCycle = new Cycle2D();
             SubCycle.start=SCE.next;
             SubCycle.current = SubCycle.start;
             SubCycle.end = StartCycle.end;
             // disconnect the subcycle from the startcycle
             StartCycle.end=SCE;
             SCE.next.prev=null;
             SCE.next = null;
             SubCycle.minX=SubCycle.start;
             SubCycle.append(P.getX(),P.getY());
             // compute bounding box and number of elements of the subcycle
             Element E = SubCycle.start;
             SubCycle.minX = E;
             SubCycle.x_min=SubCycle.x_max = E.P.getX();
             SubCycle.y_min=SubCycle.y_max = E.P.getY();
             SubCycle.NumberOfElements=1;
             E=E.next;
             while(E!=null){
                SubCycle.NumberOfElements += 1;
                if(E.P.getX()<SubCycle.x_min){
                   SubCycle.minX = E;
                   SubCycle.x_min =E.P.getX();
                }
                SubCycle.x_max = Math.max(SubCycle.x_max,E.P.getX());
                SubCycle.y_max = Math.max(SubCycle.y_max,E.P.getY());
                SubCycle.y_min = Math.min(SubCycle.y_min,E.P.getY());
                E=E.next;
             }
             // check whether subcycle is ok
             if(SubCycle.NumberOfElements>2){
                 result.add(SubCycle);
             }else{
                 ;
             }
             SCE = null; // finish the test
          } else{
             SCE = SCE.next; // check the next point
          }
       } // startcycle don't contains P => extend with P
       StartCycle.append(P.getX(),P.getY());
   }
   // at this pointt we have all SubCycles included in the result
   Element E = StartCycle.start;
   if(E!=null){
      StartCycle.minX = E;
      StartCycle.x_min = StartCycle.x_max = E.P.getX();
      StartCycle.y_min = StartCycle.y_max = E.P.getY();
      StartCycle.NumberOfElements = 1;
      E = E.next;
      while(E!=null){
        StartCycle.NumberOfElements += 1;
        if(E.P.getX()<StartCycle.x_min){
           StartCycle.minX = E;
           StartCycle.x_min =E.P.getX();
        }
        StartCycle.x_max = Math.max(StartCycle.x_max,E.P.getX());
        StartCycle.y_max = Math.max(StartCycle.y_max,E.P.getY());
        StartCycle.y_min = Math.min(StartCycle.y_min,E.P.getY());
        E=E.next;
      }
   }
   if(StartCycle.NumberOfElements>2){
      result.add(StartCycle);
   }else{
        ;
   }
   Cycle2D[] res = new Cycle2D[result.size()];
   for(int i=0;i<result.size();i++){
        res [i] = (Cycle2D) result.get(i);
   }
   return res;
}

public String toString(){
   Element E = start;
   String res="";
   while(E!=null){
      res+=E.toString();
      E=E.next;
   }
   return res;
}

Point2D getFirst(){
   current = start;
   return getNext();
}

Point2D getNext(){
   if(current==null)
      return null;
   Point2D P = current.P;
   current = current.next;
   return P;
}

public static final int CLOCKWISE = 0;
public static final int COUNTERCLOCKWISE = 1;
public static final int NO_DIRECTION = -1;

// the first element of this cycle
private Element start;
// the last Element of this cycle
private Element end;
// the current element in this cycle
private Element current;
// the element with minimal x value
// needed to compute the direction of this path
private Element minX;
// the number of containing elements
private int NumberOfElements;
// the values of the bounding box
private double x_min;
private double y_min;
private double x_max;
private double y_max;


private class Element{

public Element(double x, double y){
   P = new Point2D(x,y);
   next = null;
   prev = null;
}

public Element(Point2D P){
   this.P = P;
   next = null;
   prev = null;
}

public String toString(){
   String res ="";
   if(prev==null)
      res += "|->";
   res += P.toString();
   if(next==null)
      res+= "->|";
   else
      res += "->";
   return res;
}

 Point2D P;
 Element next;
 Element prev;
}

}
