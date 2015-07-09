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
import viewer.hoese.DsplGeneric;
import viewer.hoese.Interval;
import viewer.hoese.LEUtils;
import viewer.hoese.QueryResult;
import tools.Reporter;
import java.util.Vector;

/**
 * A displayclass for the lengthstring type
 */
public class Dspllstring extends DsplGeneric {
  Interval LengthBounds;
  boolean err = true;
  boolean defined;
  Vector Intervals = new Vector(10,5);
  Vector Strings = new Vector(10,5);


  /**
   * Scans the representation of a lengthstring datatype 
   * @param v A list of length-intervals with a string value
   * @see sj.lang.ListExpr
   * @see <a href="Dsplmovingboolsrc.html#ScanValue">Source</a>
   */
  public void ScanValue (ListExpr v) {
    if(isUndefined(v)){
       defined=false;
       return;
    } 
    defined = true;
    while (!v.isEmpty()) {
      ListExpr le = v.first();
      int len = le.listLength();
      ListExpr value;
      Interval in;
      if(len==2){
           in = LEUtils.readInterval(le.first());
           value = le.second();
      } else{ // wrong list length
           return;
      }
      if (in == null)
        return;
      Intervals.add(in);
      if (value.atomType() != ListExpr.STRING_ATOM)
        return;
      String b = value.stringValue();
      Strings.add(b);
      v = v.rest();
    }
    defined = true;
    err = false;
  }

  @Override
  public void init (String name, int nameWidth, int indent, ListExpr type, ListExpr value,  QueryResult qr) {
    AttrName = extendString(name,nameWidth, indent);
    ScanValue(value);
    if (err) {
      Reporter.writeError("Error in ListExpr :parsing aborted");
      qr.addEntry((AttrName + ": List(lustring)"));
      defined=false;
      return;
    } 
    else 
      qr.addEntry(this);
    LengthBounds = null;
    for (int i = 0; i < Intervals.size(); i++) {
      Interval in = (Interval)Intervals.elementAt(i);
      if(!in.isInfinite()){
          if (LengthBounds == null) {
              LengthBounds = in;
          } 
          else {
              LengthBounds = LengthBounds.union(in);
          }
      }
    }
  }

  /** The text representation of this object
     * @return 
   * @see <a href="Dspllengthstringsrc.html#toString">Source</a>
   */
  @Override
  public String toString () {
    return (AttrName + ": List(lustring)");
  }

}