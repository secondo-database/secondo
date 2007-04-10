
//This file is part of SECONDO.

//Copyright (C) 2004-2007, University in Hagen, i
//Faculty of Mathematics and Computer Science, 
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

package gui;

import sj.lang.*;
import javax.swing.*;
import java.awt.*;

public class ProgressView extends JPanel implements MessageListener{


/** paint a rectangle with a size depending on the current progress **/
public void paint(Graphics g){
  super.paint(g);
  g.setColor(Color.BLACK);
 int height = getHeight();
 int width = getWidth();
 g.drawRect(0,0,width-1,height-1);
 if(current>0 && max>=0){
    int h = ((height*current) / max);
    g.setColor(getColor());
    g.fillRect(0,height-h,width,h);
  }
}

/** computes a color from the current value **/
public Color getColor(){
  int r = 255 - 255*current/max;
  int g = 255*current/max;
  return new Color(r,g,0);
}

/** analyzes the message and updates the view. **/
public void processMessage(ListExpr message){

// check format of the list, assumed to be (progress (int int))
  if(message.listLength()!=2){
     return;
  }
  if(message.first().atomType()!=ListExpr.SYMBOL_ATOM ||
     !message.first().symbolValue().equals("progress")){
     return;
  }
  message = message.second();
  if(message.listLength()!=2 || 
    message.first().atomType()!=ListExpr.INT_ATOM || 
    message.second().atomType()!=ListExpr.INT_ATOM ){
     return;
  }
  int tmpcurrent = message.first().intValue();
  int tmpmax = message.second().intValue();
  if((tmpcurrent!=this.current ||
     tmpmax!=this.max ) && (tmpcurrent<=tmpmax)){
     this.current = tmpcurrent;
     this.max = tmpmax;
     paintImmediately(0,0,getWidth(),getHeight());
  } else{
     current = -1;
     paintImmediately(0,0,getWidth(),getHeight());
  }
}

public Dimension getMaximumSize(){
  Dimension d = super.getMaximumSize();
  d.width = 10; // set 10 pixels 
  return d;
}

public Dimension getPreferredSize(){
   return getMaximumSize();
}

public String toString(){
  return "progress("+current+"/"+max+")";
}

private int current=-1;
private int max=-1;

}


