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
import viewer.hoese.ExternDisplay;
import viewer.hoese.Interval;
import viewer.hoese.LEUtils;
import viewer.hoese.QueryResult;
import tools.Reporter;
import java.util.Vector;

/**
 * A displayclass for the lengthbool-type (spatiotemp algebra),
 */
public class Dspllbool extends DsplGeneric  
  implements Function,ExternDisplay {
  Interval LengthBounds;
  boolean err = true;
  boolean defined;
  Vector Intervals = new Vector(10,5);
  Vector Bools = new Vector(10,5);
  private static final Double VALUE_NOT_DEFINED = null;
  private static final Double VALUE_TRUE = (double) 1;
  private static final Double VALUE_FALSE = (double) 0;
  private static final  LFunctionFrame functionframe = new LFunctionFrame();


  /** returns the definition time interval
     * @return  **/
  @Override
  public Interval getInterval(){
    return LengthBounds;
  }
 
  /** returns the value converted into a double
     * @param length
     * @return  **/
  @Override
  public Double getValueAt(double length){
    if(err | !defined){
       return VALUE_NOT_DEFINED;
    }
    int index = IntervalSearch.getTimeIndex(length,Intervals);
    if(index<0){
       return VALUE_NOT_DEFINED;
    }
    if( ((Boolean)Bools.get(index))){
       return VALUE_TRUE;
    } else{
       return VALUE_FALSE;
    }

  }

  /**
   * Scans the representation of a lengthbool datatype 
   * @param v A list of time-intervals with a bool value
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
      if (value.atomType() != ListExpr.BOOL_ATOM)
        return;
      boolean b = value.boolValue();
      Bools.add(b);
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
      qr.addEntry((AttrName + ": List(lubool)"));
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
   * @see <a href="Dsplmovingboolsrc.html#toString">Source</a>
   */
  @Override
  public String toString () {
    return (AttrName + ": List(lubool)");
  }

  @Override
  public boolean isExternDisplayed(){
    return (functionframe.isVisible() && this.equals(functionframe.getSource()));
  }  

  @Override
  public void displayExtern(){
     if(err){
       Reporter.showInfo("cannot display because list representation is invalid");
       return;
     }
     if(!defined){
        Reporter.showInfo("the object is not defined at all instants");
        return;
     }
     if(LengthBounds!=null){
          functionframe.setSource(this);
          functionframe.setVisible(true);
          functionframe.toFront();
     }else{
         Reporter.showInfo("The length bool is empty");
     }
  }

}