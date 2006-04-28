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


package  viewer.hoese;

import  java.util.*;
import  javax.swing.*;
import viewer.HoeseViewer;

  /** If datatypes have a temporal component this interface must be implemented.
  */

public interface Timed {

  /**
   * Gets the over all time boundarys
   * @return Interval
   * @see generic.Interval
   */
  public Interval getBoundingInterval();

  
 /**
   * Gets the list of intervals this object is defined at
   * @return Vector of intervals
   * @see generic.Interval
   */

  public Vector getIntervals();
  /**
   * In the TimePanel component a temporal datatype can be represented individually.
   * This method defines a specific output as JPanel
   * @param PixelTime How much timeunits have a pixel.
   */  
  public JPanel getTimeRenderer (double PixelTime);
}



