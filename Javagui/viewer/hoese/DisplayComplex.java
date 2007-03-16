
//This file is part of SECONDO.

//Copyright (C) 2006, University in Hagen,
//Faculty of Mathematics and Computer Science, 
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

package viewer.hoese;

import java.awt.*;
import java.awt.geom.*;

/** DisplayComplex.
  * Displayclasses implementing this interface can draw Objects not 
  * representable by the Shape classes. For example, this interface has 
  * to be used for raster images or (rotated) text. 
  **/


public interface DisplayComplex{
  

  /** Draw this object to g.
    * The time parameter can be used to create a special representation of 
    * a time dependent object or for asking the renderattribute for some
    * values. The currently used transformation matrix can be used for example
    * if an object should have the same size in each zoom level. 
    * @param g:    the used graphics context
    * @param time: the current time in the animation. 
    * @param at:   the currently used transformation matrix.
    **/
                  
  public void draw(Graphics g, double time, AffineTransform at);

}


