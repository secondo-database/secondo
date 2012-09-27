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

//2012, July Simone Jandt

package  viewer.hoese.algebras.jnet;

import java.awt.geom.*;
import java.awt.*;
import java.util.*;
import sj.lang.ListExpr;
import viewer.*;
import viewer.hoese.*;
import viewer.hoese.algebras.IntervalSearch;
import javax.swing.*;


/**
 * MJPoint
 * Describes the movement of a single position in the network.
 */
public class MJPoint{

   String netId;
   private Vector<UJPoint> units = new Vector<UJPoint> ();
   private Vector<Interval> times = new Vector<Interval>();
   private Interval deftime;


  public MJPoint(ListExpr value) throws JNetworkNotAvailableException {
    if (value.listLength() == 2){
      netId = value.first().stringValue();
      JNetwork jnet = JNetworkManager.getInstance().getNetwork(netId);
      ListExpr uList = value.second();
      boolean first = true;
      double starttime = 0.0;
      double endtime = starttime;
      UJPoint actUP = null;
      while (!uList.isEmpty()){
        ListExpr actUList = uList.first();
        actUP = new UJPoint(jnet, actUList);
        units.add(actUP);
        if (first){
          starttime = actUP.getStartTime();
          first = false;
          endtime = starttime;
        }
        endtime = actUP.getEndTime();
        times.add(LEUtils.readInterval(actUList.first()));
        uList = uList.rest();
      }
      deftime = new Interval(starttime, endtime, true, true);
    } else {
      netId = "undefined";
    }
  }

  public String toString(){
    if (netId.compareTo("undefined") != 0)
      return "mjpoint";
    else
      return "undefined";
  }

  public Rectangle2D.Double getBounds(){
    if (units.size()>0){
      Rectangle2D.Double bounds = ((UJPoint)units.get(0)).getBounds();
      for (int i = 1; i < units.size();  i++){
        Rectangle2D.Double next = ((UJPoint)units.get(i)).getBounds();
        if (next != null)
          bounds.add(next);
      }
      return bounds;
    }
    return null;
  }

  public int numOfShapes(){
    return 1;
  }

  public boolean isPointType(int no){
    return true;
  }

  public boolean isLineType(int no){
    return false;
  }

  public Point2D.Double getPointAtTime(double actTime){
    int index = IntervalSearch.getTimeIndex(actTime,times);
    if(index < 0){ // t is outside from the deftime
      return null;
    }
    UJPoint u = (UJPoint) units.get(index);
    return u.getPointAtTime(actTime);
  }

  public Vector getIntervals(){
    return times;
  }

  public Interval getBoundingInterval(){
    return deftime;
  }

  public JPanel getTimeRenderer (double PixelTime) {
    int index = IntervalSearch.getTimeIndex(PixelTime, times);
    JPanel jpan = new JPanel(null);
    if (index > -1) {
      UJPoint u = (UJPoint) units.get(index);
      Interval t = u.getBoundingInterval();
      int start = 0;
      JLabel label = new JLabel("|"+LEUtils.convertTimeToString(t.getStart()).substring(11,
                                16), JLabel.LEFT);
      label.setBounds(start, 15, 100, 15);
      label.setVerticalTextPosition(JLabel.CENTER);
      label.setHorizontalTextPosition(JLabel.RIGHT);

      jpan.setPreferredSize(new Dimension(100, 25));
      jpan.add(label);
      //Add labels to the JPanel.
      return  jpan;
    }
    return  jpan;
  }

}



