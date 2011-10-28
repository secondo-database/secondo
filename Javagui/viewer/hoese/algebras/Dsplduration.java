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

import  sj.lang.ListExpr;
import  viewer.*;
import viewer.hoese.*;


/**
 * A displayclass for the duration type 
 */
public class Dsplduration extends DsplGeneric implements LabelAttribute, RenderAttribute{

  String entry;
  boolean defined;
  double value;

  /** Converts a duration given in nested list format into 
    * a string representing the same value. If the list don't
    * is a valid representation for a durationtype (day ms), null 
    * is returned. 
    **/
  private String getString(ListExpr value){
    this.value = 0;
    if(isUndefined(value)){
       defined=false;
       return "undefined";
    }
    // first, extract day and millisecond infro from value
    if(value.listLength()!=2)
        return null;
    ListExpr f = value.first();
    ListExpr s = value.second();
    if(f.atomType()!=ListExpr.INT_ATOM || s.atomType()!=ListExpr.INT_ATOM)
       return null;

    defined = true;
    int d = f.intValue();
    int ms = s.intValue();

    this.value = ((double)d) + (((double)ms) / (24*3600000));
    // very simple output !
    String time = "";
    int H = ms / (1000*60*60);
    if (H>0) {
	    time = "" + H + "h ";
    }
    ms = ms % (1000*60*60);
    int M = ms / (1000*60);
    if(M>0 || H > 0 ){
       time += M + "m ";
    }
    ms = ms % (1000*60);
    int S = ms / 1000;
    if(H>0 || M > 0 || S>0){
	time += S + "s ";
    }
    ms = ms % 1000;
    time += ms + "ms";    

    if(d==0){ 
      return time;  
    } else{
      return d+"days " + time ;
    }
  }

  

  public String toString(){
     return entry;
  }

  public void init (String name, int nameWidth, int indent, 
                    ListExpr type, ListExpr value, QueryResult qr)
  {
     String T = name;
     String V = getString(value);
     if(V==null)
       V = "<error>";
     T=extendString(T,nameWidth, indent);
     entry=(T + " : " + V);
     qr.addEntry(this);
     return;
  }

  public String getLabel(double time){
	  return entry;
  }

   /** returns the defined state at the given time **/
   public boolean isDefined(double time ) {
	   return defined;
   }
   /** returns the minimum value of this attribute **/
   public double getRenderValue(double time){
	   return value;
   }
   /** return whether this objects is defined at any time**/
   public boolean mayBeDefined(){ 
	   return defined;
   }
   /** returns the maximum value of this attribute **/
   public double getMinRenderValue(){
	   return value;
   }
   /** returns the value of this attribute for the given time **/
   public double getMaxRenderValue(){
	   return value;
   }

}



