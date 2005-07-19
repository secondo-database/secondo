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

public class Dsplpmreal extends DsplGeneric implements Function {

Class linearClass = (new PMRealUnit()).getClass();
Time T = new Time();
TotalMove Move=null;


public void init(ListExpr type,ListExpr value,QueryResult qr){
  AttrName = type.symbolValue();
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

/** Returns the interval of this periodic moving real */
 public Interval getInterval(){
    return TimeBounds;
 }

 /** Returns true when the function of this periodic moving real is displayed **/
 public boolean isExternDisplayed(){
      return(functionframe.isVisible() && this.equals(functionframe.getSource()));
  }

public void  displayExtern(){
      if(TimeBounds!=null){
         functionframe.setSource(this);
         functionframe.setVisible(true);
         functionframe.toFront();
      } else{
         viewer.MessageBox.showMessage("The periodic moving real is empty");
      }
}


public Double getValueAt(double x){
  if(Move==null) return null;
  if(!Move.isDefined()) return null;
  anInstant.readFrom(x);
  return (Double) Move.getObjectAt(anInstant);
}


// we need this beacuse the Hoese viewer can't handle infinite time intervals
private static final double MaxToLeft = 3000;
private static final double MaxToRight = 3000;
private static Time anInstant = new Time();

private Interval TimeBounds;

private static FunctionFrame functionframe = new FunctionFrame();

}
