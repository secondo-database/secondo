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

package viewer.hoese.algebras;

import java.awt.geom.*;
import java.awt.*;
import viewer.*;
import viewer.hoese.*;
import sj.lang.ListExpr;
import java.util.*;
import viewer.hoese.algebras.periodic.*;
import javax.swing.JPanel;

public class Dsplpmpoints extends DisplayTimeGraph{

Rectangle2D.Double bounds;
Class linearClass = (new PMPsLinear()).getClass();
Time T = new Time();
TotalMove Move=null;

public Shape getRenderObject(AffineTransform at){
  double t = RefLayer.getActualTime();
  T.readFrom(t);
  Point2D.Double[] Pts = (Point2D.Double[]) Move.getObjectAt(T);
  if(Pts==null || Pts.length==0){
    RenderObject=null;
    return RenderObject;
  }
  Area res = new Area();
  double pixx = Math.abs(Cat.getPointSize()/at.getScaleX());
  double pixy = Math.abs(Cat.getPointSize()/at.getScaleY());
  for(int i=0; i< Pts.length;i++){
     if(Cat.getPointasRect()){
        res.add(new Area(new Rectangle2D.Double(Pts[i].getX()-pixx/2,
	                                        Pts[i].getY()-pixy/2,pixx,pixy)));
     }else{
        res.add(new Area(new Ellipse2D.Double(Pts[i].getX()-pixx/2,
	                                        Pts[i].getY()-pixy/2,pixx,pixy)));
     }
  }
  RenderObject=res;
  return res;
  
}


public void init(ListExpr type,ListExpr value,QueryResult qr){
  AttrName = type.symbolValue();
  ispointType = true;
  Move = new TotalMove();
  if(!Move.readFrom(value,linearClass)){
     qr.addEntry("("+AttrName +"WrongListFormat )");
     return;
  }
  qr.addEntry(this);
  if(Move.getBoundingBox()==null){
     System.err.println("Bounding Box can't be created");
  }
  bounds = Move.getBoundingBox().toRectangle2D();
  System.out.println("Bonding Box = " + Move.getBoundingBox());
  System.out.println("Resulting rectangle: " + Move.getBoundingBox().toRectangle2D());

  
  double StartTime = Move.getStartTime().getDouble();
  RelInterval D = Move.getInterval();
  if(D.isLeftInfinite())
     StartTime -= MaxToLeft;
  double EndTime = StartTime;
  if(D.isRightInfinite())
    EndTime += MaxToRight;
  else
    EndTime += D.getLength().getDouble();
  TimeBounds = new Interval(StartTime,EndTime,D.isLeftClosed(),D.isRightClosed());
}

public JPanel getTimeRenderer(double PixelTime){
   return new JPanel();
}

/* returns the minimum bounding box of this moving point */
public Rectangle2D.Double getBounds(){
   return bounds;
}

// we need this beacuse the Hoese viewer can't handle infinite time intervals
private static final double MaxToLeft = 3000;
private static final double MaxToRight = 3000;

}
