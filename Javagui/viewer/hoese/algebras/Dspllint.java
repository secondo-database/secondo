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
public class Dspllint extends DsplGeneric  
  implements Function,ExternDisplay {
  Interval LengthBounds;
  boolean err = true;
  boolean defined;
  Vector Intervals = new Vector(10,5);
  Vector Ints = new Vector(10,5);
  private static final  LFunctionFrame functionframe = new LFunctionFrame();
protected int min;
protected int max;

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
    int index = IntervalSearch.getTimeIndex(length,Intervals);
    if(index<0) return null;
    return new Double(((Integer)Ints.get(index)).doubleValue()); 
  }


  /**
   * Scans the representation of a lengthbool datatype 
   * @param v A list of lengt-intervals with a int value
   * @see sj.lang.ListExpr
   * @see <a href="Dsplmovingboolsrc.html#ScanValue">Source</a>
   */
  public void ScanValue (ListExpr v) {
    if(isUndefined(v)){
       defined=false;
       err=false;
       return;
    }
    if(v.atomType()!=ListExpr.NO_ATOM){
       err=true;
       return;
    } 
    boolean first=true;
    while (!v.isEmpty()) {
      ListExpr le = v.first();
      Interval in = null;
      ListExpr value=null;
      if(le.listLength()==2){ // (interval int)
         in = LEUtils.readInterval(le.first());
         value = le.second();
      }
      if (in == null){ // error in reading interval
        Reporter.debug("Dspllengthint: cannot read the interval from list ");
        return;
      } 
			if (value.atomType() != ListExpr.INT_ATOM){ // error in reading value
				Reporter.debug("Dspllengthint: error in ListExpr, int atom required");
				return;
			}
			int i = value.intValue();
			if(first){
				min = i;
				max = i;
				first = false;
			} else{
				min = i<min?i:min;
				max = i>max?i:max;  
			}
      Intervals.add(in);
	  	Ints.add(new Integer(i));
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
      qr.addEntry((AttrName + ": List(luint)"));
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
    return (AttrName + ": List(luint)");
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
         Reporter.showInfo("The length int is empty");
     }
  }

}