
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
import  viewer.*;
import viewer.hoese.*;
import  sj.lang.ListExpr;
import  javax.swing.border.*;
import  java.util.*;
import  javax.swing.*;
import tools.Reporter;


public class TimeRenderer{
  
  static Vector computeConnectedIntervals(Vector intervals){
       if(intervals==null){
          return null;
       }
       Vector connectedIntervals = new Vector();
       if(intervals.size()==0){
          return connectedIntervals;
       }
       Interval current = ((Interval)intervals.get(0)).copy(); 
       for(int i=1;i<intervals.size();i++){
          Interval in = (Interval) intervals.get(i);
          if(current.connected(in)){
            current.unionInternal(in);
          } else{
            connectedIntervals.add(current);
            current = in.copy();
          }

       }
       connectedIntervals.add(current);
       return connectedIntervals;
    }


  public static JPanel getTimeRenderer (double PixelTime, Vector connectedIntervals, 
                                        Interval TimeBounds) {
    JPanel jp = new JPanel(null);
    if (connectedIntervals == null){
      return  null;
    } 
    if(connectedIntervals.size()==0){
      return jp;
    }

    ListIterator li = connectedIntervals.listIterator();

    if(connectedIntervals.size() > 500){
      System.err.println("Too much intervals for representing timed");
      return jp;
    }

    while (li.hasNext()) {
      Interval in = (Interval)li.next();
      int start = (int)((in.getStart() - TimeBounds.getStart())*PixelTime);
      int end = (int)((in.getEnd() - TimeBounds.getStart())*PixelTime);
      JLabel jc = new JLabel();
      jc.setOpaque(true);
      jc.setBackground(Color.BLACK);
      jc.setPreferredSize(new Dimension(1, 10));
      jc.setBorder(new MatteBorder(2, (in.isLeftclosed()) ? 2 : 0, 2, (in.isRightclosed()) ?
          2 : 0, Color.black));
      Dimension d = jc.getPreferredSize();
      jc.setBounds(start, (int)d.getHeight()*0 + 15, end - start, (int)d.getHeight());
      
      
      jc.setToolTipText(LEUtils.convertTimeToString(in.getStart()) + "..." +
          LEUtils.convertTimeToString(in.getEnd()));

      jc.addMouseListener(new java.awt.event.MouseAdapter(){
         public void mouseEntered(java.awt.event.MouseEvent evt){
                int a = ToolTipManager.sharedInstance().getDismissDelay();
                if(a!=40000)
                     oldDelay=a;
                ToolTipManager.sharedInstance().setDismissDelay(40000);
               
         }
         public void mouseExited(java.awt.event.MouseEvent evt){
                ToolTipManager.sharedInstance().setDismissDelay(oldDelay);
         }
         int oldDelay;   
       });

      jp.setPreferredSize(new Dimension((int)((TimeBounds.getEnd() - TimeBounds.getStart())*PixelTime),
          25));
      jp.add(jc);
    }
    return  jp;
  }



}
