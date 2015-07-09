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

import java.text.DecimalFormat;
import java.util.Vector;

import sj.lang.ListExpr;
import tools.Reporter;

import viewer.hoese.Interval;
import viewer.hoese.QueryResult;
import viewer.hoese.DsplGeneric;
import viewer.hoese.LEUtils;
import viewer.hoese.ExternDisplay;


public class Dspllreal extends DsplGeneric implements Function, ExternDisplay
{
  private Interval boundingInterval;
  private Vector Intervals = new Vector(10, 5);
  private Vector LRealMaps = new Vector(10, 5);
  private final double min=Double.POSITIVE_INFINITY ;
  private final double max=Double.NEGATIVE_INFINITY ;
  private boolean err;
  private boolean defined;
  private static final  LFunctionFrame functionframe = new LFunctionFrame();
  
    DecimalFormat df = new DecimalFormat("###.###");
  


  @Override
  public boolean isExternDisplayed()
  {
      return(functionframe.isVisible() && this.equals(functionframe.getSource()));
  }

  @Override
  public void  displayExtern(){
      if(!defined){
          Reporter.showInfo("not defined");
          return;
      }
      if(boundingInterval!=null){
         functionframe.setSource(this);
         functionframe.setVisible(true);
         functionframe.toFront();
      } else{
         Reporter.showInfo("The length real is empty");
      }
  }

  /**  Computes the value of this real for a given length.
    *  The length is just given as a double. 
    *  If the lenth real is not defined at the given length null is returned.
     * @param length Length Position
     * @return  index of Interval containing the length position
    **/
  @Override
  public    Double getValueAt(double length){
    int index = IntervalSearch.getTimeIndex(length,Intervals);
    return getValueAt(length,index); 
  }


  /** Computes the value for a given index 
    * There is no check wether the given length is contained in
    * the interval determined by index.
    **/
  private Double getValueAt(double length,int index){ 
       // check for correct index
       if(index<0 || index >= Intervals.size())
          return null;
       Interval CurrentInterval = (Interval) Intervals.get(index);
       LRealMap CurrentMap = (LRealMap) LRealMaps.get(index);
       double start=CurrentInterval.getStart();
       length=start;
       double polyvalue = CurrentMap.m * length + CurrentMap.n;
        return polyvalue;
}

  
  /** Returns the interval of this length real
     * @return Returns the interval of this length real **/
  @Override
  public Interval getInterval()
  {
    return  boundingInterval;
  }


  /**
   * Scans the representation of a lengthreal datatype
   * @param v A list of length-intervals with the parameter for length function
   * @see sj.lang.ListExpr
   * @see <a href="Dspllrealsrc.html#ScanValue">Source</a>
   */
  public void ScanValue (ListExpr v) {
    if(isUndefined(v)){
       err=false;
       defined=false;
       return;
    }
    defined=true;
    if (v.isEmpty()){
      err=false; // allow empty lreals
      return;
    }
    while (!v.isEmpty()) {
      ListExpr le = v.first();
      Interval in=null;
      ListExpr map=null;
      int len = le.listLength();
      if(len!=2 ){
         Reporter.debug("invalid listlength, (2 expected but got " + len);
         return;
      }
      if(len == 2){
           in = LEUtils.readInterval(le.first());
         map = le.second();
      }
      LRealMap lm = readLRealMap(map);
      if ((in == null) || (lm == null)) {
          if(in==null){
             Reporter.debug("Error in interval representation ");
          }
          if(lm==null){
             Reporter.debug("error in map representation");
          }
          return;
      }
      Intervals.add(in);
      LRealMaps.add(lm);
      v = v.rest();
    }
    err = false;
  }


  /**
   * This method reads the parameter for the length function into a LRealMap-instance
   * @param le A 2 element list
   * @return A LRealMap-instance with the formula parameter
   */
  private  LRealMap readLRealMap (ListExpr le) {
    Double value[] =  {
      null, null
    };
    if (le.listLength() != 2)
      return  null;
    for (int i = 0; i < 2; i++) {
      value[i] = LEUtils.readNumeric(le.first());
      if (value[i] == null)
        return  null;
      le = le.rest();
    }
    return  new LRealMap(value[0], value[1]);
  }


  @Override
  public void init (String name, int nameWidth,int indent, 
                    ListExpr type,  ListExpr value, QueryResult qr) {
    AttrName = extendString(name,nameWidth, indent);
    err=true;
    ScanValue(value);
    if (err) {
      defined = false;
      Reporter.writeError("Error in ListExpr :parsing aborted");
      qr.addEntry((AttrName + ": Error"));
      return;
    } 
    else 
      qr.addEntry(this);
    // compute the bounding box of all intervals
    boundingInterval = null;
    for (int i = 0; i < Intervals.size(); i++) {
      Interval in = (Interval)Intervals.elementAt(i);
      if(!in.isInfinite()){
        if (boundingInterval == null) {
            boundingInterval = in.copy();
        } 
        else {
            boundingInterval.unionInternal(in);
        }
      }
    }
  }

  /** The text representation of this object
     * @return Name of this Type
   * @see <a href="Dsplmovingrealsrc.html#toString">Source</a>
   */
  @Override
  public String toString () {
    return  AttrName + ": list (lureal)";
  }
  


  /** The class which holds the formula parameter for an interval */
  class LRealMap {
    double m,n;

    /**
     * Constructor
     * @param     double m Gradient
     * @param     double n Intersection with y-Axis
     */
    public LRealMap (double x1, double x2) {
      m = x1;
      n = x2;
    }
  }
}