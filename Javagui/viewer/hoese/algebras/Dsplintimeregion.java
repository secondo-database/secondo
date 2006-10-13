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
import  java.util.*;
import  javax.swing.*;
import tools.Reporter;


/**
 * A displayclass for the intimeregion-type (spatiotemp algebra), 2D with TimePanel
 */
public class Dsplintimeregion extends Dsplregion
    implements Timed {
  Interval TimeBounds;

  /** A method of the Timed-Interface
   * 
   * @return the global time boundaries [min..max] this instance is defined at
   * @see <a href="Dsplintimeregionsrc.html#getTimebounds">Source</a>
   */
  public Interval getBoundingInterval () {
    return  TimeBounds;
  }

  /**
   * A method of the Timed-Interface to render the content of the TimePanel
   * @param PixelTime pixel per hour
   * @return A JPanel component with the renderer
   * @see <a href="Dsplintimeregionsrc.html#getTimeRenderer">Source</a>
   */
  public JPanel getTimeRenderer (double PixelTime) {
    int start = 0;
    JLabel label = new JLabel("|"+LEUtils.convertTimeToString(TimeBounds.getStart()).substring(11, 
        16), JLabel.LEFT);
    label.setBounds(start, 15, 100, 15);
    label.setVerticalTextPosition(JLabel.CENTER);
    label.setHorizontalTextPosition(JLabel.RIGHT);
    JPanel jp = new JPanel(null);
    jp.setPreferredSize(new Dimension(100, 25));
    jp.add(label);
    //Add labels to the JPanel. 
    return  jp;
  }

  /**
   * Scans the representation of an instant datatype 
   * @param v An instant value
   * @see sj.lang.ListExpr
   * @see <a href="Dsplintimeregionsrc.html#ScanValue">Source</a>
   */
  public void ScanValue (ListExpr v) {
    Double d;
    if (v.listLength() != 2) {                  //perhaps changes later
      Reporter.writeError("Error: No correct intimeregion expression: 2 elements needed");
      err = true;
      return;
    }
    d = LEUtils.readInstant(v.first());
    if (d == null) {
      err = true;
      return;
    }
    TimeBounds = new Interval(d.doubleValue(), d.doubleValue(), true, true);
    super.ScanValue(v.second());
  }

  /**
   * This instance is only visible at its defined time.
   * @param at The actual Transformation, used to calculate the correct size.
   * @return Region Shape if ActualTime == defined time
   * @see <a href="Dsplintimeregionsrc.html#getRenderObject">Source</a>
   */
  public Shape getRenderObject (int num,AffineTransform at) {
    double t = RefLayer.getActualTime();
    if (Math.abs(t - TimeBounds.getStart()) < 0.000001)
      return  super.getRenderObject(num,at); 
    else 
      return  null;
  }

  /**
   * Init. the Dsplintimeregon instance.
   * @param type The symbol intimeregion
   * @param value The value of an instant and a region
   * @param qr queryresult to display output.
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="Dsplintimeregionsrc.html#init">Source</a>
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    AttrName = type.symbolValue();
    ScanValue(value);
    if (err) {
      Reporter.writeError("Error in ListExpr :parsing aborted");
      qr.addEntry(new String("(" + AttrName + ": GTA(IntimeRegion))"));
      return;
    } 
    else 
      qr.addEntry(this);
  }

  /** A method of the Timed-Interface
   * @return The Vector representation of the time intervals this instance is defined at 
   * @see <a href="Dsplintimeregionsrc.html#getIntervals">Source</a>
   */
  public Vector getIntervals(){
    Vector v=new Vector(1,0);
    v.add(TimeBounds);
    return v;
    } 
}



