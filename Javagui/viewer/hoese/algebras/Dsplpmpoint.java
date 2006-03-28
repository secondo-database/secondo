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

public class Dsplpmpoint extends DisplayTimeGraph{

Point2D.Double point;
Rectangle2D.Double bounds;
Class linearClass = (new PMPLinear()).getClass();
Time T = new Time();
TotalMove Move=null;

public Shape getRenderObject(AffineTransform at){
  if(Move==null){
     RenderObject = null;
     return null;
  }
  double t = RefLayer.getActualTime();
  T.readFrom(t);
  Point2D.Double Pos =  (Point2D.Double) Move.getObjectAt(T);
  if(Pos==null){
     RenderObject = null;
     return null;
  }
  double ps = Cat.getPointSize(renderAttribute,CurrentState.ActualTime); 
  double pixy = Math.abs(ps/at.getScaleY());
  double pixx = Math.abs(ps/at.getScaleX());
  if(Cat.getPointasRect()){
    RenderObject = new Rectangle2D.Double(Pos.getX()-pixx/2,Pos.getY()-pixy/2,pixx,pixy);
  }else{
    RenderObject = new Ellipse2D.Double(Pos.getX()-pixx/2,Pos.getY()-pixy/2,pixx,pixy);
  }
  return RenderObject;
}

public void init(ListExpr type,ListExpr value,QueryResult qr){
  AttrName = type.symbolValue();
  ispointType = true;
  Move = new TotalMove();
  if(!Move.readFrom(value,linearClass)){
     qr.addEntry("("+AttrName +" WrongListFormat )");
     return;
  }
  if(!Move.isDefined()){
     qr.addEntry(AttrName+" : undefined ");
     return;
  }

  qr.addEntry(this);
  if(Move.getBoundingBox()==null){
     System.err.println("Bounding Box can't be created");
  }
  bounds = Move.getBoundingBox().toRectangle2D();
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
