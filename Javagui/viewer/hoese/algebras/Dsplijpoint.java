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

package viewer.hoese.algebras;

import viewer.hoese.algebras.jnet.*;
import java.awt.geom.*;
import java.awt.*;
import javax.swing.*;
import sj.lang.ListExpr;
import java.util.*;
import viewer.*;
import viewer.hoese.*;
import tools.Reporter;
import gui.SecondoObject;


/**
 * A Displayclass for graphical representation of timed JNetwork jnet.
 */
public class Dsplijpoint extends DisplayGraph implements Timed{
	/** The internal datatype representation */
	String label;
	JPoint jp;
	Interval timeBounds;
  Rectangle2D.Double bounds=null;

  public Dsplijpoint(){
    super();
  }
/**
 * Init ijpoint instance.
 * @param value
 *            the nestedlist representation of the jpoint
 * @param qr
 *            queryresult to display output.
 */

public void init(String name, int nameWidth, int indent,ListExpr type,
                 ListExpr value, QueryResult qr){
  AttrName = extendString(name,nameWidth, indent);
  try{
    Double d = LEUtils.readInstant(value.first());
    timeBounds = new Interval(d.doubleValue(), d.doubleValue(), true, true);
    jp = new JPoint(value.second());
    label = jp.toString();
    qr.addEntry(this);
    return;
  } catch (JNetworkNotAvailableException ex){
    err = true;
    Reporter.writeError(ex.getMessage());
    qr.addEntry(AttrName + ex.getMessage());
    return;
  } catch (Exception ex) {
    ex.printStackTrace();
    err = true;
    Reporter.writeError("Error in ListExpr :parsing aborted");
    qr.addEntry(AttrName + "error");
    return;
  }

}

public String toString(){
  return jp.toString();
}

 public int numberOfShapes(){
    return 1;
 }

 public boolean isPointType(int no){
     return true;
 }

 public boolean isLineType(int no){
    return false;
 }


/**
 * Returns the bounding rectangle of the jpoint
 * @return The bound rectangle of the jpoint
 */

public Rectangle2D.Double getBounds(){
  if (bounds == null)
    bounds = jp.getBounds();
  return bounds;
}

/**
* This instance is only visible at its defined time.
* @param at The actual transformation, used to calculate the correct size.
* @return Rectangle or circle Shape if ActualTime == defined time
* @see <a href="Dsplintimepointsrc.html#getRenderObject">Source</a>
*/

public Shape getRenderObject (int no,AffineTransform af) {
  double t = RefLayer.getActualTime();
  double pointSize = Cat.getPointSize(renderAttribute,CurrentState.ActualTime);
  boolean asRect = Cat.getPointasRect();
  if (Math.abs(t - timeBounds.getStart()) < 0.000001)
    return  jp.getRenderObject(no, af, pointSize, asRect);
  else
    return  null;
}


/** A method of the Timed-Interface
*
* @return the global time boundaries [min..max] this instance is defined at
* @see <a href="Dsplintimepointsrc.html#getTimebounds">Source</a>
*/
 public Interval getBoundingInterval () {
   return  timeBounds;
}

/** A method of the Timed-Interface
* @return The Vector representation of the time intervals this instance is defined at
* @see <a href="Dsplintimepointsrc.html#getIntervals">Source</a>
*/
public Vector getIntervals(){
  Vector v=new Vector(1,0);
  v.add(timeBounds);
  return v;
}

/**
* A method of the Timed-Interface to render the content of the TimePanel
* @param PixelTime pixel per hour
* @return A JPanel component with the renderer
* @see <a href="Dsplintimepointsrc.html#getTimeRenderer">Source</a>
*/
public JPanel getTimeRenderer (double PixelTime) {
  int start = 0;
  JLabel label = new JLabel("|"+LEUtils.convertTimeToString(timeBounds.getStart()).substring(11,
                            16), JLabel.LEFT);
  label.setBounds(start, 15, 100, 15);
  label.setVerticalTextPosition(JLabel.CENTER);
  label.setHorizontalTextPosition(JLabel.RIGHT);
  JPanel jpan = new JPanel(null);
  jpan.setPreferredSize(new Dimension(100, 25));
  jpan.add(label);
  //Add labels to the JPanel.
  return  jpan;
}


};