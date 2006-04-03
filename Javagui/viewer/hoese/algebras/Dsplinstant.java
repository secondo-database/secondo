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

import sj.lang.ListExpr;
import viewer.*;
import viewer.hoese.*;
import java.util.*;
import javax.swing.*;
import java.awt.*;
import tools.Reporter;


/**
 * A displayclass for the instant-type (spatiotemp algebra), alphanumeric with TimePanel
 */
public class Dsplinstant extends DsplGeneric
    implements Timed {
  Interval TimeBounds;
  boolean err = true;
  boolean defined;
  String entry;

  /** A method of the Timed-Interface
   * 
   * @return the global time boundaries [min..max] this instance is defined at
   * @see <a href="Dsplinstantsrc.html#getTimebounds">Source</a>
   */
  public Interval getTimeBounds () {
    return  TimeBounds;
  }

  /**
   * A method of the Timed-Interface to render the content of the TimePanel
   * @param PixelTime pixel per hour
   * @return A JPanel component with the renderer
   * @see <a href="Dsplinstantsrc.html#getTimeRenderer">Source</a>
   */
  public JPanel getTimeRenderer (double PixelTime) {
    int start = 0;              //(int)(TimeBounds.getStart()*PixelTime);
    JLabel label = new JLabel("|"+LEUtils.convertTimeToString(TimeBounds.getStart()).substring(11, 
        16), JLabel.LEFT);
    label.setBounds(start , 15, 100, 15);
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
   * @see <a href="Dsplinstantsrc.html#ScanValue">Source</a>
   */
  public String getString(ListExpr v) {
    if(isUndefined(v)){
        defined=false;
        return "undefined";
    }
    Double d;
    d = LEUtils.readInstant(v);
    if (d == null){
     defined = false;
      return "<error>";
    }
    defined = true;
    TimeBounds = new Interval(d.doubleValue(), d.doubleValue(), true, true);
    err = false;
    return LEUtils.convertTimeToString(TimeBounds.getStart());
  }

  /**
   * Init. the Dsplinstant instance.
   * @param type The symbol instant
   * @param value The value of an instant .
   * @param qr queryresult to display output.
   * @see generic.QueryResult
   * @see sj.lang.ListExpr
   * @see <a href="Dsplinstantsrc.html#init">Source</a>
   */
  public void init (ListExpr type, ListExpr value, QueryResult qr) {
    AttrName = type.symbolValue();
    String v = getString(value);
    entry = AttrName + ":"+v;
    if (err) {
      qr.addEntry(new String("(" + AttrName + ": TA(Instant))"));
      return;
    } 
    else 
      qr.addEntry(this);
  }
  public void init (ListExpr type,int typewidth,ListExpr value,int valuewidth, QueryResult qr)
  {
     String T = new String(type.symbolValue());
     String V = getString(value);
     T=extendString(T,typewidth);
     V=extendString(V,valuewidth);
     entry=(T + " : " + V);
     qr.addEntry(this);
     return;
  }



  /** The text representation of this object 
   * @see <a href="Dsplinstantsrc.html#toString">Source</a>
   */
  public String toString () {
    return  entry;
  }
  /** A method of the Timed-Interface
   * @return The Vector representation of the time intervals this instance is defined at 
   * @see <a href="Dsplinstantsrc.html#getIntervals">Source</a>
   */
  public Vector getIntervals(){
    Vector v=new Vector(1,0);
    v.add(TimeBounds);
    return v;
    } 
}



