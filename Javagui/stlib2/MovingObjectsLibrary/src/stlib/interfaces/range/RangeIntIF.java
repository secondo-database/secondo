//This file is part of SECONDO.

//Copyright (C) 2014, University in Hagen, Department of Computer Science,
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
package stlib.interfaces.range;

import stlib.interfaces.base.BaseIntIF;
import stlib.interfaces.interval.IntervalIntIF;

/**
 * Interface that should be provided by objects of type RangeInt
 * 
 * @author Markus Fuessel
 */
public interface RangeIntIF extends RangeIF<BaseIntIF> {

   /*
    * (non-Javadoc)
    * 
    * @see stlib.datatypes.range.Range#first()
    */
   IntervalIntIF first();

   /*
    * (non-Javadoc)
    * 
    * @see stlib.datatypes.range.Range#last()
    */
   IntervalIntIF last();

   /*
    * (non-Javadoc)
    * 
    * @see stlib.datatypes.rangeset.RangeSet#getInterval(int)
    */
   IntervalIntIF get(int index);

}