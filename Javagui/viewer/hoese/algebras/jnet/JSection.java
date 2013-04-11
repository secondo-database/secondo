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

//2012, June Simone Jandt

package viewer.hoese.algebras.jnet;

import java.awt.geom.*;
import java.awt.*;
import java.util.*;
import sj.lang.ListExpr;
import viewer.*;
import viewer.hoese.*;



/**
 * JSection
 * Describes the spatial curve between two junctions.
 */
public class JSection{

   private static Area basicArrow;
   private boolean isArrow;
   private Point2D.Double p1;
   private Point2D.Double p2;

   private Vector<JRouteInterval> routeIntervals = new Vector<JRouteInterval>();
   private JDirection dir;
   private boolean startSmaller;
   private double lenth;
   private int sid;
   private GeneralPath curve;
   private GeneralPath curveRendered;
   private Rectangle2D.Double bounds = new Rectangle2D.Double(0.0,0.0,0.0,0.0);


  public JSection(ListExpr value){
    if (value.listLength() == 12){
      sid = value.first().intValue();
      dir = new JDirection(value.fifth());
      if (dir.compareTo("Both", true) != 0){
        isArrow = true;
      } else {
        isArrow = false;
      }
      readCurve(value.second());
      readRouteIntervals(value.eighth());
    }
  }

  public JSection(JRouteInterval rint,
                  Point2D.Double pt1, Point2D.Double pt2,
                  GeneralPath cur, GeneralPath curRend, boolean hasArrow,
                  Rectangle2D.Double box, boolean smaller){
    p1 = new Point2D.Double(pt1.x,pt1.y);
    p2 = new Point2D.Double(pt2.x,pt2.y);
    dir = rint.getDir();
    this.isArrow = hasArrow;
    this.routeIntervals.add(rint);
    this.lenth = rint.getLength();
    this.curveRendered = curRend;
    this.curve = cur;
    this.startSmaller = smaller;
    bounds.setRect(box);
  }

  public Integer getId(){
    return sid;
  }

  public double getLength(){
    return lenth;
  }

  public Shape getRenderObject(int i, AffineTransform af, double pointSize){
    if (i == 0){
      return curveRendered;
    } else if (isArrow){
      if (dir.compareTo("Up", true) == 0){
        return getArrow(af, p1, p2, pointSize);
      } else {
        return getArrow(af, p2, p1, pointSize);
      }
    } else {
      return null;
    }
  }

  public Rectangle2D.Double getBounds(){
    return bounds;
  }

  public JRouteInterval getRouteInterval (int pos){
    return routeIntervals.get(pos);
  }

  public Point2D.Double getPosition(RouteLocation rloc, int pos, double tolerance){
    JRouteInterval curInt = routeIntervals.get(pos);
    double distFromStartOfSection = rloc.getPos() - curInt.getStartPos();
    return getPoint(distFromStartOfSection, tolerance);
  }

  public int contains(RouteLocation rloc, double tolerance){
    for (int i = 0; i < routeIntervals.size(); i++){
      JRouteInterval curInt = routeIntervals.get(i);
      if (curInt.contains(rloc, tolerance)){
        return i;
      }
    }
    return -1;
  }

  public int contains(JRouteInterval rint, double tolerance){
    for (int i = 0; i < routeIntervals.size(); i++){
      JRouteInterval curInt = routeIntervals.get(i);
      if (curInt.contains(rint, tolerance)){
        return i;
      }
    }
    return -1;
  }

  public int isCompletelyInside(JRouteInterval rint, double tolerance){
    for (int i = 0; i < routeIntervals.size(); i++){
      JRouteInterval curInt = routeIntervals.get(i);
      if (curInt.completelyInside(rint, tolerance)){
        return i;
      }
    }
    return -1;
  }

  public GeneralPath getCurve(boolean rendered){
    if (rendered)
      return curveRendered;
    else
      return curve;
  }

  public GeneralPath getCurve(JRouteInterval rint, int pos, boolean rendered){
    JRouteInterval curInt = routeIntervals.get(pos);
    double startpos = rint.getStartPos()- curInt.getStartPos();
    double endpos = rint.getEndPos() - curInt.getStartPos();
    return getCurve(startpos, endpos, rendered);
  }

  public GeneralPath getCurveFrom(RouteLocation rloc, int pos,
                                  boolean rendered){
    JRouteInterval curInt = routeIntervals.get(pos);
    double distFromStartOfSection = rloc.getPos() - curInt.getStartPos();
    if (startSmaller)
      return getCurveFrom(distFromStartOfSection, rendered);
    else
       return getCurveTo(distFromStartOfSection, rendered);
  }

  public GeneralPath getCurveTo(RouteLocation rloc, int pos, boolean rendered){
    JRouteInterval curInt = routeIntervals.get(pos);
    double distFromStartOfSection = rloc.getPos() - curInt.getStartPos();
    if (startSmaller)
      return getCurveTo(distFromStartOfSection, rendered);
    else
      return getCurveFrom(distFromStartOfSection, rendered);
  }

  public boolean getStartSmaller(){
    return startSmaller;
  }

  private void curveToString(){
    PathIterator pi1 = curve.getPathIterator(null, 0.0);
    double[] coordsA = new double [6];
    while(!pi1.isDone()){
      pi1.currentSegment(coordsA);
      for (int i = 0; i < 6; i++){
        System.out.println("coordsA " + i + ": " + coordsA[i]);
      }
      pi1.next();
    }
  }

  private void readCurve (ListExpr value){
    if (value.listLength() == 2) {
      if (value.second().atomType() == ListExpr.BOOL_ATOM)
        startSmaller = value.second().boolValue();
      else
        startSmaller = true;
      Vector<Vector<Point2D.Double>> pointSequences =
        new Vector<Vector<Point2D.Double>>();
      ListExpr hslist = value.first();
      while (!hslist.isEmpty()){
        ListExpr curSegment = hslist.first();
        if (curSegment.listLength() == 4){
          Double X1 = LEUtils.readNumeric(curSegment.first());
          Double Y1 = LEUtils.readNumeric(curSegment.second());
          Double X2 = LEUtils.readNumeric(curSegment.third());
          Double Y2 = LEUtils.readNumeric(curSegment.fourth());
          if (X1 != null && Y1 != null && X2 != null && Y2 != null){
            double x1 = X1.doubleValue();
            double y1 = Y1.doubleValue();
            double x2 = X2.doubleValue();
            double y2 = Y2.doubleValue();
            insertSegment(pointSequences, x1,y1,x2,y2);
          }
        }
        hslist = hslist.rest();
      }
      if (pointSequences.size() == 1){
        Vector<Point2D.Double> sequence = pointSequences.get(0);
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
          if (startSmaller && !firstLess){
            int smallindex = findSmallestIndex(sequence);
            if (0 < smallindex)
              reverse(sequence);
          } else {
            if (!startSmaller && !firstLess){
              int smallindex = findSmallestIndex(sequence);
              if (0 < smallindex)
                reverse(sequence);
            }
          }
        }
      }
      curve = new GeneralPath();
      curveRendered = new GeneralPath();
      Point2D.Double rendRes = new Point2D.Double(0.0,0.0);
      Iterator<Vector<Point2D.Double>> it = pointSequences.iterator();
      while(it.hasNext()){
        Vector<Point2D.Double> sequence = it.next();
        for(int i=0 ; i < sequence.size(); i++){
          Point2D.Double p = sequence.get(i);
          if (i == 0){
            curve.moveTo((float) p.x, (float) p.y);
          } else {
            curve.lineTo((float)p.x, (float)p.y);
          }
          if(ProjectionManager.project(p.x,p.y,rendRes)){
            if(i == 0){
              curveRendered.moveTo((float)rendRes.x, (float)rendRes.y);
            } else {
              curveRendered.lineTo((float)rendRes.x,(float)rendRes.y);
            }
          } else {
            if(i == 0){
              curveRendered.moveTo((float)p.x, (float)p.y);
            } else {
              curveRendered.lineTo((float)p.x,(float)p.y);
            }
          }
        }
      }
      bounds.setRect(curveRendered.getBounds2D());
    }
  }

  private void insertSegment(Vector<Vector<Point2D.Double>> sequences,
                             double x1, double y1, double x2, double y2){
    Point2D.Double p1 = new Point2D.Double(x1,y1);
    Point2D.Double p2 = new Point2D.Double(x2,y2);
    Vector<Vector<Point2D.Double>> connectedSequences =
      new Vector<Vector<Point2D.Double>>();
    Iterator<Vector<Point2D.Double>> it = sequences.iterator();
    while(it.hasNext() && connectedSequences.size() < 2){
      Vector<Point2D.Double> seq = it.next();
      if( almostEqual(seq.get(0),p1) || almostEqual(seq.get(0),p2) ||
          almostEqual(seq.get(seq.size()-1),p1) ||
          almostEqual(seq.get(seq.size()-1),p2)){
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
        } else if(almostEqual(seq2.get(seq2.size()-1),seq.get(0))){ // seq extends seq2
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
        }
      }
    }
  }

private static boolean almostEqual(double a, double b){
  return almostEqual(a,b, 0.00000001);
}

private static boolean almostEqual(Point2D.Double p1, Point2D.Double p2){
  return almostEqual(p1.x,p2.x) && almostEqual(p1.y,p2.y);
}

private static boolean almostEqual(double a, double b, double tolerance){
  return Math.abs(a-b) < tolerance;
}

private static boolean isLess(Point2D.Double p1, Point2D.Double p2){
  if(almostEqual(p1.x, p2.x)){
    if(almostEqual(p1.y, p2.y)){
      return false;
    } else {
      return p1.y < p2.y;
    }
  }
  return p1.x < p2.x;
}

private static void reverse(Vector<Point2D.Double >  v){
  Stack<Point2D.Double> stack = new Stack<Point2D.Double>();
  for(int i = 0; i < v.size() ; i++){
       stack.push(v.get(i));
  }
  v.clear();
  while(!stack.isEmpty()){
     v.add(stack.pop());
  }
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
        sequence.set(i, sequenceCopy.get((i+index)%size));
     }
  }

  private Shape getArrow(AffineTransform af, Point2D.Double point1, Point2D.Double point2, double pointSize){
   createBasicArrow(pointSize);
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


 private void createBasicArrow(double x){
    GeneralPath gparrow = new GeneralPath();
    gparrow.moveTo(0,0);
    gparrow.lineTo(5,-1);
    gparrow.lineTo(3,0);
    gparrow.lineTo(5,1);
    gparrow.lineTo(0,0);
    Shape s = gparrow;
    if(x > 0){
      AffineTransform at = AffineTransform.getScaleInstance(x/5,x/5);
      s =  at.createTransformedShape(gparrow);
    } else {
      basicArrow=null;
      return;
    }
    basicArrow = new Area(s);
  }

  private void readRouteIntervals(ListExpr value){
    boolean isFirst = true;
    while (!value.isEmpty()){
      JRouteInterval curInt = new JRouteInterval(value.first());
      if (isFirst) {
        lenth = curInt.getLength();
      }
      routeIntervals.add(curInt);
      value = value.rest();
    }
  }

  private Point2D.Double getPoint(double inpos, double tolerance){
    double pos = inpos;
    if (!startSmaller)
      pos = lenth - pos;
    PathIterator pi = curve.getPathIterator(null, 0.0);
    double[] coordsFrom = new double[6];
    double[] coordsTo = new double[6];
    pi.currentSegment(coordsFrom);
    double distOnSection = 0.0;
    double lSeg = 0.0;
    double distNew = 0.0;
    double x = 0.0;
    double y = 0.0;
    // Look for segment
    pi.next();
    while(!pi.isDone() && (distOnSection < lenth)){
      pi.currentSegment(coordsTo);
      lSeg = Math.sqrt(Math.pow(Math.abs(coordsTo[0] - coordsFrom[0]),2) +
                       Math.pow(Math.abs(coordsTo[1] - coordsFrom[1]),2));
      distNew = distOnSection + lSeg;
      if ((distOnSection <= pos || almostEqual(distOnSection, pos, tolerance)) &&
          (pos <= distNew) ||  almostEqual(pos, distNew, tolerance)) {
        if (lSeg != 0) {
          x = coordsFrom[0] + (pos - distOnSection) * (coordsTo[0]-coordsFrom[0]) / lSeg;
          y = coordsFrom[1] + (pos - distOnSection) * (coordsTo[1]-coordsFrom[1]) / lSeg;
        } else {
          x = coordsFrom[0];
          y = coordsFrom[1];
        }
        return new Point2D.Double(x,y);
      } else {
        coordsFrom[0] = coordsTo[0];
        coordsFrom[1] = coordsTo[1];
        distOnSection = distNew;
      }
      pi.next();
    }
    return new Point2D.Double(coordsFrom[0], coordsFrom[1]);
  }

  private GeneralPath getCurveFrom(double pos, boolean rendered){
    GeneralPath gp = new GeneralPath();
    Point2D.Double rendRes = new Point2D.Double(0.0,0.0);
    PathIterator pi = curve.getPathIterator(null, 0.0);
    double[] coordsFrom = new double[6];
    double[] coordsTo = new double[6];
    pi.currentSegment(coordsFrom);
    double distOnSection = 0.0;
    double lSeg = 0.0;
    double distNew = 0.0;
    boolean startfound = false;
    pi.next();
    while (!pi.isDone() && !startfound && distOnSection < lenth){
      pi.currentSegment(coordsTo);
      lSeg = Math.sqrt(Math.pow(Math.abs(coordsTo[0] - coordsFrom[0]),2) +
                       Math.pow(Math.abs(coordsTo[1] - coordsFrom[1]),2));
      distNew = distOnSection + lSeg;
      if ((distOnSection <= pos || almostEqual(distOnSection, pos)) &&
           (pos <= distNew || almostEqual(distNew, pos))){
        startfound = true;
        double x = coordsFrom[0] + (pos - distOnSection) * (coordsTo[0]-coordsFrom[0]) / lSeg;
        double y = coordsFrom[1] + (pos - distOnSection) * (coordsTo[1]-coordsFrom[1]) / lSeg;
        if (rendered){
          if (ProjectionManager.project(x,y,rendRes))
            gp.moveTo((float)rendRes.x, (float)rendRes.y);
          else
            gp.moveTo((float)x,(float)y);
          if (ProjectionManager.project(coordsTo[0],coordsTo[1],rendRes))
            gp.lineTo((float)rendRes.x,(float) rendRes.y);
          else
            gp.lineTo((float)coordsTo[0],(float)coordsTo[1]);
        } else {
          gp.moveTo((float)x,(float)y);
          gp.lineTo((float)coordsTo[0],(float)coordsTo[1]);
        }

      }
      coordsFrom[0] = coordsTo[0];
      coordsFrom[1] = coordsTo[1];
      distOnSection = distNew;
      pi.next();
    }
    if (startfound){
      while(!pi.isDone() && distOnSection < lenth){
        pi.currentSegment(coordsTo);
        lSeg = Math.sqrt(Math.pow(Math.abs(coordsTo[0] - coordsFrom[0]),2) +
                         Math.pow(Math.abs(coordsTo[1] - coordsFrom[1]),2));
        distOnSection = distOnSection + lSeg;
        if (rendered){
          if (ProjectionManager.project(coordsTo[0],coordsTo[1],rendRes))
            gp.lineTo((float)rendRes.x,(float) rendRes.y);
          else
            gp.lineTo((float)coordsTo[0],(float)coordsTo[1]);
        } else {
          gp.lineTo((float)coordsTo[0],(float)coordsTo[1]);
        }

        coordsFrom[0] = coordsTo[0];
        coordsFrom[1] = coordsTo[1];
        pi.next();
      }
    }
    return gp;
  }

  private GeneralPath getCurveTo(double pos, boolean rendered){
    GeneralPath gp = new GeneralPath();
    Point2D.Double rendRes = new Point2D.Double(0.0,0.0);
    PathIterator pi = curve.getPathIterator(null, 0.0);
    double[] coordsFrom = new double[6];
    double[] coordsTo = new double[6];
    pi.currentSegment(coordsFrom);
    double distOnSection = 0.0;
    double lSeg = 0.0;
    double distNew = 0.0;
    boolean endfound = false;
    if (rendered){
      if (ProjectionManager.project(coordsFrom[0],coordsFrom[1],rendRes))
        gp.moveTo((float)rendRes.x, (float)rendRes.y);
      else
        gp.moveTo((float)coordsFrom[0],(float)coordsFrom[1]);
    } else {
      gp.moveTo((float)coordsFrom[0],(float)coordsFrom[1]);
    }

    pi.next();
    while (!pi.isDone() && !endfound && distOnSection < lenth){
      pi.currentSegment(coordsTo);
      lSeg = Math.sqrt(Math.pow(Math.abs(coordsTo[0] - coordsFrom[0]),2) +
                       Math.pow(Math.abs(coordsTo[1] - coordsFrom[1]),2));
      distNew = distOnSection + lSeg;
      if (distNew <= pos || almostEqual(distNew, pos)){
        if (rendered){
          if (ProjectionManager.project((float)coordsTo[0],(float)coordsTo[1],rendRes))
            gp.lineTo((float)rendRes.x, (float)rendRes.y);
          else
            gp.lineTo((float)coordsTo[0],(float)coordsTo[1]);
        } else {
          gp.lineTo((float)coordsTo[0],(float)coordsTo[1]);
        }
      } else {
        endfound = true;
        double x = coordsFrom[0] + (pos - distOnSection) * (coordsTo[0]-coordsFrom[0]) / lSeg;
        double y = coordsFrom[1] + (pos - distOnSection) * (coordsTo[1]-coordsFrom[1]) / lSeg;
        if (rendered){
          if (ProjectionManager.project(x,y,rendRes))
            gp.lineTo((float)rendRes.x,(float) rendRes.y);
          else
            gp.lineTo((float)x,(float)y);
        } else {
          gp.lineTo((float)x,(float)y);
        }
      }
      coordsFrom[0] = coordsTo[0];
      coordsFrom[1] = coordsTo[1];
      distOnSection = distNew;
      pi.next();
    }
    return gp;
  }

  private GeneralPath getCurve(double infrom , double into, boolean rendered) {
    GeneralPath gp = new GeneralPath();
    Point2D.Double rendRes = new Point2D.Double(0.0,0.0);
    double from = infrom;
    double to = into;
    if (from > to)
    {
      double help = from;
      from = to;
      to = help;
    }
    PathIterator pi = curve.getPathIterator(null, 0.0);
    double[] coordsFrom = new double[6];
    double[] coordsTo = new double[6];
    pi.currentSegment(coordsFrom);
    double distOnSection = 0.0;
    double lSeg = 0.0;
    double distNew = 0.0;
    boolean startfound = false;
    boolean endfound = false;
    pi.next();
    while (!pi.isDone() && !startfound && distOnSection < lenth){
      pi.currentSegment(coordsTo);
      lSeg = Math.sqrt(Math.pow(Math.abs(coordsTo[0] - coordsFrom[0]),2) +
                       Math.pow(Math.abs(coordsTo[1] - coordsFrom[1]),2));
      distNew = distOnSection + lSeg;
      if ((distOnSection <= from || almostEqual(distOnSection, from)) &&
          (from <= distNew || almostEqual(from, distNew))){
        startfound = true;
        double x = coordsFrom[0] + (from - distOnSection) * (coordsTo[0]-coordsFrom[0]) / lSeg;
        double y = coordsFrom[1] + (from - distOnSection) * (coordsTo[1]-coordsFrom[1]) / lSeg;
        if (rendered){
          if (ProjectionManager.project(x,y,rendRes))
            gp.moveTo((float)rendRes.x, (float)rendRes.y);
          else
            gp.moveTo((float)x,(float)y);
        } else {
          gp.moveTo((float)x,(float)y);
        }
        if ((distOnSection <= to || almostEqual(distOnSection, to)) &&
            (to <= distNew || almostEqual(to, distNew))) {
          endfound = true;
          x = coordsFrom[0] + (to - distOnSection) * (coordsTo[0]-coordsFrom[0]) / lSeg;
          y = coordsFrom[1] + (to - distOnSection) * (coordsTo[1]-coordsFrom[1]) / lSeg;
          if (rendered){
            if (ProjectionManager.project(x,y,rendRes))
              gp.lineTo((float)rendRes.x, (float)rendRes.y);
            else
              gp.lineTo((float)x,(float)y);
          } else {
            gp.lineTo((float)x,(float)y);
          }
        } else {
          if (rendered){
            if (ProjectionManager.project(coordsTo[0],coordsTo[1],rendRes))
              gp.lineTo((float)rendRes.x, (float)rendRes.y);
            else
              gp.lineTo((float)coordsTo[0],(float)coordsTo[1]);
          }else {
            gp.lineTo((float)coordsTo[0],(float)coordsTo[1]);
          }
        }
      }
      coordsFrom[0] = coordsTo[0];
      coordsFrom[1] = coordsTo[1];
      distOnSection = distNew;
      pi.next();
    }
    if (startfound){
      if (!endfound) {
         while (!pi.isDone() && !endfound && distOnSection < lenth){
          pi.currentSegment(coordsTo);
          lSeg = Math.sqrt(Math.pow(Math.abs(coordsTo[0] - coordsFrom[0]),2) +
                           Math.pow(Math.abs(coordsTo[1] - coordsFrom[1]),2));
          distNew = distOnSection + lSeg;
          if ((distOnSection <= to || almostEqual(distOnSection, to)) &&
              (to <= distNew || almostEqual(to, distNew))){
            endfound = true;
            double x = coordsFrom[0] + (to - distOnSection) * (coordsTo[0]-coordsFrom[0]) / lSeg;
            double y = coordsFrom[1] + (to - distOnSection) * (coordsTo[1]-coordsFrom[1]) / lSeg;
            if (rendered){
              if (ProjectionManager.project(x,y,rendRes))
                gp.lineTo((float)rendRes.x, (float)rendRes.y);
              else
                gp.lineTo((float)x,(float)y);
            } else {
              gp.lineTo((float)x,(float)y);
            }
          } else {
            if (rendered){
              if (ProjectionManager.project((float)coordsTo[0],(float)coordsTo[1],rendRes))
                gp.lineTo((float)rendRes.x, (float)rendRes.y);
              else
                gp.lineTo((float)coordsTo[0],(float)coordsTo[1]);
            } else {
              gp.lineTo((float)coordsTo[0],(float)coordsTo[1]);
            }
          }
          coordsFrom[0] = coordsTo[0];
          coordsFrom[1] = coordsTo[1];
          distOnSection = distNew;
          pi.next();
        }
      }
    }
    return gp;
  }

}


