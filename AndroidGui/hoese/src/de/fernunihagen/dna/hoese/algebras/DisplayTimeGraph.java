package de.fernunihagen.dna.hoese.algebras;

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

import  javamini.awt.geom.*;
import  javamini.awt.*;
import  sj.lang.ListExpr;
import  java.util.*;

import de.fernunihagen.dna.hoese.Interval;
import de.fernunihagen.dna.hoese.Timed;
import tools.Reporter;


/**
* The base class for moving 2d-Objects
*/
public abstract class DisplayTimeGraph extends DisplayGraph
  implements Timed {
Interval TimeBounds;
Vector Intervals;

Vector connectedIntervals = null;
/**
 * A method of the Timed-Interface to render the content of the TimePanel
 * @param PixelTime pixel per hour
 * @return A JPanel component with the renderer
 * @see <a href="DisplayTimeGraphsrc.html#getTimeRenderer">Source</a>
 */

// TODO: JPanel ersetzen
//
//public JPanel getTimeRenderer(double PixelTime){
//   if(connectedIntervals==null){
//     connectedIntervals = TimeRenderer.computeConnectedIntervals(Intervals);
//   }
//   return TimeRenderer.getTimeRenderer(PixelTime, connectedIntervals, TimeBounds);
//}

/** A method of the Timed-Interface
 *
 * @return the global time boundaries [min..max] this instance is defined at
 * @see <a href="DisplayTimeGraphsrc.html#getTimebounds">Source</a>
 */
public Interval getBoundingInterval () {
  return  TimeBounds;
}


/**
 * Moving objects have 0..n Intervals and calculation unit
 * @param time A double representing a time
 * @param iv The Time intervals to check
 * @param maps The maps of this instance.
 * @return A unit of this instance, that is defined at time, or null if not defined
 * @see <a href="DisplayTimeGraphsrc.html#getMapAt">Source</a>
 */
public Object getMapAt (double time, Vector iv, Vector maps) {
  for (int i = 0; i < iv.size(); i++){
    Interval interval = (Interval) iv.get(i);
    if (interval.isDefinedAt(time)){
       if(time<interval.getStart() | time>interval.getEnd())
	          Reporter.writeError("wrong interval found");
       return  maps.elementAt(i);
    }
   }
  return  null;
}
/** A method of the Timed-Interface
 * @return The Vector representation of the time intervals this instance is defined at
 * @see <a href="DisplayTimeGraphsrc.html#getIntervals">Source</a>
 */
public Vector getIntervals(){
  return Intervals;
  }


}



