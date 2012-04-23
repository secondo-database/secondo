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

package  viewer.hoese.algebras;



import  java.awt.geom.*;
import  java.awt.*;
import  sj.lang.ListExpr;
import  viewer.hoese.*;
import java.util.Vector;
import java.util.Iterator;
import java.util.Stack;
import tools.Reporter;


/**
 * The displayclass of the sline datatype.
 */
public class Dsplsline extends Dsplline {

private static Area basicArrow;
boolean isArrow = true;
boolean startSmaller;
Point2D.Double p1;
Point2D.Double p2;

public int numberOfShapes(){
  return 2;
}

public boolean isLineType(int num){
   if (num==0){
      return super.isLineType(num);
   }
   return false;  // draw arrow as a region
}


public Shape getRenderObject(int num, AffineTransform at){
   if(num==0){
      return super.getRenderObject(num,at);
   }
   if(!isArrow){
      return null;
   }
   if(!err && defined && p1!=null && p2!=null){
     return getArrow(at,p1,p2);
   }
   return null;
}


public static boolean almostEqual(double a, double b){
   return Math.abs(a-b) < 0.00000001;
}

public static boolean almostEqual(Point2D.Double p1, Point2D.Double p2){
   return almostEqual(p1.x,p2.x) && almostEqual(p1.y,p2.y);
}

public static boolean isLess(Point2D.Double p1, Point2D.Double p2){
   if(almostEqual(p1.x,p2.x)){
      if(almostEqual(p1.y,p2.y)){
         return false;
      } else {
          return p1.y < p2.y;
      }
    }
    return p1.x < p2.x;


}

public static void reverse(Vector<Point2D.Double >  v){
  Stack<Point2D.Double> stack = new Stack<Point2D.Double>();
  for(int i=0;i<v.size();i++){
       stack.push(v.get(i));
  }
  v.clear();
  while(!stack.isEmpty()){
     v.add(stack.pop());
  }
}


private boolean insertSegment(Vector<Vector<Point2D.Double>> sequences, double x1, double y1, double x2, double y2){
   Point2D.Double p1 = new Point2D.Double(x1,y1);
   Point2D.Double p2 = new Point2D.Double(x2,y2);
   Vector<Vector<Point2D.Double>> connectedSequences = new Vector<Vector<Point2D.Double>>();
   Iterator<Vector<Point2D.Double>> it = sequences.iterator();
   while(it.hasNext() && connectedSequences.size() < 2){
     Vector<Point2D.Double> seq = it.next();
     if( almostEqual(seq.get(0),p1) || almostEqual(seq.get(0),p2) ||
         almostEqual(seq.get(seq.size()-1), p1) || almostEqual(seq.get(seq.size()-1),p2)){
         connectedSequences.add(seq);
     }
   }
   if(connectedSequences.size()==0){ // new unconnected segment
     Vector<Point2D.Double> newSeq = new Vector<Point2D.Double>();
     newSeq.add(p1);
     newSeq.add(p2);
     sequences.add(newSeq);
   } else { // new segment extends a single sequence
     // extend the first sequence by one point
     Vector<Point2D.Double> seq = connectedSequences.get(0);
     if(almostEqual(p1,seq.get(0))){
        seq.add(0,p2);
     } else if(almostEqual(p2,seq.get(0))){
        seq.add(0,p1);
     } else if(almostEqual(p1,seq.get(seq.size()-1))){
        seq.add(p2);
     } else {
        seq.add(p1);
     }
     if(connectedSequences.size()==2){ // we have to connect both sequences
        Vector<Point2D.Double> seq2 = connectedSequences.get(1);
        if(almostEqual(seq.get(seq.size()-1), seq2.get(0))){ // seq2 extends seq
           sequences.remove(seq2);
           seq.addAll(seq2);
        }  else if(almostEqual(seq2.get(seq2.size()-1),seq.get(0))){ // seq extends seq2
           seq2.addAll(seq);
           sequences.remove(seq);
        } else if( almostEqual(seq.get(seq.size()-1),seq2.get(seq2.size()-1))) { // both endpoints are equal
           sequences.remove(seq2);
           reverse(seq2);
           seq.addAll(seq2);
        } else if(almostEqual(seq.get(0),seq2.get(0))){ // both starting points are equal
            reverse(seq);
            seq.addAll(seq2);
            sequences.remove(seq2);
        } else { // not a sline
           return false;
        }
     }


   }
   return true;
}


public void ScanValue(ListExpr value){

  if(isUndefined(value)){
     defined = false;
     GP = null;
     return;
  }

  defined = true;

  startSmaller = true;
  if(value.listLength()==2 && value.second().atomType()==ListExpr.BOOL_ATOM){// new style
     startSmaller = value.second().boolValue();
     value = value.first();
  }
  // try to build a point sequence from the shuffled segments
  if(value.isEmpty()){
    GP = null;
    return;
  }

  Vector<Vector<Point2D.Double> > pointSequences = new Vector<Vector<Point2D.Double> >();

  while(!value.isEmpty()){
    ListExpr segment = value.first();
    value = value.rest();
    if(segment.listLength()!=4){
        Reporter.writeError("Error: No correct segment expression: 4 elements needed");
        GP=null;
        err = true;
        return;
    }
    Double X1 = LEUtils.readNumeric(segment.first());
    Double Y1 = LEUtils.readNumeric(segment.second());
    Double X2 = LEUtils.readNumeric(segment.third());
    Double Y2 = LEUtils.readNumeric(segment.fourth());
    if(X1==null || X2==null || Y1==null || Y2==null){
        Reporter.writeError("Error: No correct segment expression: 4 elements needed");
        GP=null;
        err = true;
        return;
    }
    double x1 = X1.doubleValue();
    double y1 = Y1.doubleValue();
    double x2 = X2.doubleValue();
    double y2 = Y2.doubleValue();
    // prject the coordinates if possible
    try{
      if(!ProjectionManager.project(x1,y1,aPoint)){
        Reporter.writeError("Error: Problem in projection of coordinates");
        GP=null;
        err = true;
        return;
      }
      x1 = aPoint.x;
      y1 = aPoint.y;
      if(!ProjectionManager.project(x2,y2,aPoint)){
        Reporter.writeError("Error: Problem in projection of coordinates");
        GP=null;
        err = true;
        return;
      }
      x2 = aPoint.x;
      y2 = aPoint.y;
    } catch(Exception e){
        Reporter.writeError("Error: No correct segment expression: 4 elements needed");
        GP=null;
        err = true;
        return;
    }
    if(!insertSegment(pointSequences, x1,y1,x2,y2)){
      Reporter.writeError("found problem in reconstructing pointsequence");
    }
  }
  if(pointSequences.size()!=1){
     Reporter.writeError("Error: the segments do not form a simple line, no arrow will be drawn");
     isArrow = false;
  } else {
     Vector<Point2D.Double> sequence = pointSequences.get(0);
     if(sequence.size()<2){
        Reporter.writeError("Error: internal error, found sequnece with less than 2 points");
        GP=null;
        err = true;
        return;
     }

     if(isCycle(sequence)){
       int end = findSmallestIndex(sequence);
       int start;
       if(startSmaller){
          start=end-1;
          if(start<0){
            start = sequence.size()-1;
          }
       } else {
          start = end+1;
          if(start>=sequence.size()){
             start=0;
          }
       }
       p1 = sequence.get(start);
       p2 = sequence.get(end);
     } else {
       boolean firstLess = isLess(sequence.get(0) ,
                              sequence.get(sequence.size()-1));
       if(startSmaller == firstLess){
         this.p1 = sequence.get(sequence.size()-2);
         this.p2 = sequence.get(sequence.size()-1);
       } else {
         this.p1 = sequence.get(1);
         this.p2 = sequence.get(0);
       }
    }
  }

  GP = new GeneralPath();
  Iterator<Vector<Point2D.Double>> it = pointSequences.iterator();
  while(it.hasNext()){
    Vector<Point2D.Double> sequence = it.next();
    for(int i=0;i<sequence.size(); i++){
      Point2D.Double p = sequence.get(i);
      if(i==0){
         GP.moveTo((float)p.x, (float)p.y);
      } else {
         GP.lineTo((float)p.x,(float)p.y);
      }
    }
  }
  defined = true;
  err = false;
}

  private static boolean isCycle(Vector<Point2D.Double> s){
    if(s.size()<2){
      return false;
    }
    return s.get(0).equals(s.get(s.size()-1));
  }

  private static int findSmallestIndex(Vector<Point2D.Double> s){
     int index = 0;
     Point2D.Double p = s.get(0);
     for(int i=1;i<s.size();i++){
       Point2D.Double p2 = s.get(i);
       if(isLess(p2,p)){
         index = i;
         p = p2;
       }
     }
     return index;
  }

  private static void resort(Vector<Point2D.Double> sequence){
     if(sequence.size()<2){
        return;
     }

     if(! sequence.get(0).equals(sequence.get(sequence.size()-1))){
        // not a cycle
        return;
     }

     // cycle found, sort sequence, that the smallest contained point
     // is the first one
     // part 1 search the index of the smallest point
     int index = findSmallestIndex(sequence);

     // TODO implement this algorithm without copying vectors
     Vector<Point2D.Double> sequenceCopy = new Vector<Point2D.Double>(sequence);
     int size = sequence.size();
     for(int i=0;i<size;i++){
        System.out.println((i+index)%size + "->" + i);
        sequence.set(i, sequenceCopy.get((i+index)%size));
     }
  }

  public Shape getArrow(AffineTransform af, Point2D.Double point1, Point2D.Double point2){

   createBasicArrow(Cat);
   // transform the basicArrow to be in the correct angle at the end of the connection
   double x1 = point1.getX();
   double x2 = point2.getX();
   double y1 = point1.getY();
   double y2 = point2.getY();

   AffineTransform aat = new AffineTransform();

   double sx = af.getScaleX();
   double sy = af.getScaleY();

   AffineTransform trans = AffineTransform.getTranslateInstance(x2,y2);

   aat.concatenate(trans);
   // normalize
   double dx =  x1-x2;
   double dy = y1-y2;
   double len = Math.sqrt(dx*dx+dy*dy); // the length
   dx = dx / len;
   dy = dy / len;

   AffineTransform Rot = new AffineTransform(dx,dy,-dy,dx,0,0);
   aat.concatenate(Rot);


   AffineTransform scale = AffineTransform.getScaleInstance(5/sx,5/sy);

   aat.concatenate(scale);

   Shape S = aat.createTransformedShape(basicArrow);
   return S;
  }


  void createBasicArrow(Category cat){
     GeneralPath gparrow = new GeneralPath();
     gparrow.moveTo(0,0);
     gparrow.lineTo(5,-1);
     gparrow.lineTo(3,0);
     gparrow.lineTo(5,1);
     gparrow.lineTo(0,0);
     Shape s = gparrow;
     if(cat!=null){
        double x = cat.getPointSize(renderAttribute, CurrentState.ActualTime);
        if(x>0){
           AffineTransform at = AffineTransform.getScaleInstance(x/5,x/5);
           s =  at.createTransformedShape(gparrow);
        } else {
           basicArrow=null;
           return;
        }


     }

     basicArrow = new Area(s);
  }

public void init (String name, int nameWidth, int indent, ListExpr type, ListExpr value, QueryResult qr) {
    AttrName = extendString(name,nameWidth, indent);
    ScanValue(value);
    if (err) {
      Reporter.writeError("Error in ListExpr :parsing aborted");
      entry = AttrName + " : <error>";
      qr.addEntry(entry);
      bounds =null;
      GP=null;
      return;
    }
    else if(!defined){
      entry = AttrName+" : undefined";
      qr.addEntry(entry);
      return;
    }
    // normal case-> defined line
    entry = AttrName + " : sline";
    defined=GP!=null;
    err=false;
    qr.addEntry(this);
    if(GP==null)
        bounds = null;
    else{
       bounds = new Rectangle2D.Double();
       bounds.setRect(GP.getBounds2D());
    }
  }

  /** returns the string for the textual representation of this **/
  public String toString(){
     if(err || !defined){
        return entry;

     }
     else{
        return entry + ", " + startSmaller + " ("+Cat.getName()+")";
     }

  }
}


