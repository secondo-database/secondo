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
import  java.util.*;
import  javax.swing.*;
import  java.awt.*;
import  javax.swing.border.*;
import  java.awt.event.*;
import  java.awt.geom.*;
import gui.Environment;
import tools.Reporter;


/**
 * A displayclass for the a ureal instance.
 * Basically it can be interpreted as a single part of a MReal containing just
 * one unit. So it is designed as a subclass of this type.  
 */
public class Dsplureal extends Dsplmovingreal implements 
      Timed,Function,ExternDisplay,LabelAttribute, RenderAttribute{

  /**
   * Scans the representation of a ureal datatype
   * @param v A list of time-intervals with the parameter for a quadratic or squareroot formula
   * @see sj.lang.ListExpr
   * @see <a href="Dsplmovingrealsrc.html#ScanValue">Source</a>
   */
  public void ScanValue (ListExpr v) {
    if(isUndefined(v)){
       err=false;
       defined=false;
       return;
    }
    defined=true;
    
		ListExpr le = v;
		Interval in=null;
		ListExpr map=null;
		int len = le.listLength();
		if(len!=2 && len !=8)
			 return;
		if (len == 8){
			 Reporter.writeWarning("Warning: deprecated list representation for ureal");
			 in = LEUtils.readInterval(ListExpr.fourElemList(le.first(),
										 le.second(), le.third(), le.fourth()));
			 map = le.rest().rest().rest().rest();
		}
		if(len == 2){
			 in = LEUtils.readInterval(le.first());
			 map = le.second();
		}
		MRealMap rm = readMRealMap(map);
		if ((in == null) || (rm == null)){
				return;
		}
		Intervals.add(in);
		MRealMaps.add(rm);
		v = v.rest();
    err = false;
  }


private static  FunctionFrame functionframe = new FunctionFrame();



}



