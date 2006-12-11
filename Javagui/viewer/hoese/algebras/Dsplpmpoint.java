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
import tools.Reporter;
import java.text.DecimalFormat;

public class Dsplpmpoint extends DisplayTimeGraph implements LabelAttribute{

Point2D.Double point;
Rectangle2D.Double bounds;
Class linearClass = (new PMPLinear()).getClass();
Time T = new Time();
TotalMove Move=null;
static DecimalFormat format = new DecimalFormat("#.####");
boolean error;


public boolean isPointType(int num){
   return true;
}

public int numberOfShapes(){
  return 1;
}

/** returns the position at the given time.
  * If the point is undefined at the given instant, null is 
  * returned.
  **/
public Point2D.Double getPosition(Time T){
   return (Point2D.Double) Move.getObjectAt(T);
}

public Time getMinTime(){
    return Move.getStartTime();
}

public Time getMaxTime(){
    return Move.getEndTime();
}

public Shape getRenderObject(int num,AffineTransform at){
  if(num!=0){
      return null;
   }
  if(Move==null){
     return null;
  }
  double t = RefLayer.getActualTime();
  T.readFrom(t);
  Point2D.Double Pos =  (Point2D.Double) Move.getObjectAt(T);
  if(Pos==null){
     return null;
  }
  double ps = Cat.getPointSize(renderAttribute,CurrentState.ActualTime); 
  double pixy = Math.abs(ps/at.getScaleY());
  double pixx = Math.abs(ps/at.getScaleX());
  Shape shp;
  if(Cat.getPointasRect()){
    shp = new Rectangle2D.Double(Pos.getX()-pixx/2,Pos.getY()-pixy/2,pixx,pixy);
  }else{
    shp = new Ellipse2D.Double(Pos.getX()-pixx/2,Pos.getY()-pixy/2,pixx,pixy);
  }
  return shp;
}

public String getLabel(double time){
  if(Move==null){
     return null;
  }
  T.readFrom(time);
  Point2D.Double Pos = (Point2D.Double) Move.getObjectAt(T);
  if(Pos==null){
     return null;
  }
  return format.format(Pos.getX())+", "+format.format(Pos.getY());

}


public void init(ListExpr type,ListExpr value,QueryResult qr){
  error = false;
  AttrName = type.symbolValue();
  Move = new TotalMove();
  if(!Move.readFrom(value,linearClass)){
     if(qr!=null){
         qr.addEntry("("+AttrName +" WrongListFormat )");
         error=true;
     }
     return;
  }
  if(!Move.isDefined()){
     if(qr!=null){
        qr.addEntry(AttrName+" : undefined ");
     }
     return;
  }

  if(qr!=null){
     qr.addEntry(this);
  }
  if(Move.getBoundingBox()==null){
     Reporter.writeError("Bounding Box can't be created");
  }
  bounds = Move.getBoundingBox().toRectangle2D();
  double StartTime = Move.getStartTime().getDouble();
  RelInterval D = Move.getInterval();
  double EndTime = StartTime;
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

/** returns the success of the last init command */
public boolean getError(){
   return error;
}


}
