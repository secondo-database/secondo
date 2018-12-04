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

package stlib.interfaces.interval;

import stlib.interfaces.time.TimeInstantIF;

/**
 * Interface that should be provided by objects of type Period
 * 
 * @author Markus Fuessel
 */
public interface PeriodIF extends IntervalIF<TimeInstantIF> {

   /**
    * Return the amount of milliseconds of this 'Period' object between lower bound
    * and upper bound
    * 
    * @return amount of milliseconds
    */
   long getDurationInMilliseconds();

   /*
    * (non-Javadoc)
    * 
    * @see stlib.datatypes.interval.Interval#copy(stlib.datatypes.interval.Interval)
    */
   PeriodIF copy();

   /*
    * (non-Javadoc)
    * 
    * @see stlib.datatypes.interval.Interval#merge(stlib.datatypes.interval.Interval)
    */
   PeriodIF merge(IntervalIF<TimeInstantIF> otherInterval);

   /*
    * (non-Javadoc)
    * 
    * @see
    * stlib.datatypes.interval.Interval#mergeLeft(stlib.datatypes.interval.Interval)
    */
   PeriodIF mergeLeft(IntervalIF<TimeInstantIF> otherInterval);

   /*
    * (non-Javadoc)
    * 
    * @see
    * stlib.datatypes.interval.Interval#mergeRight(stlib.datatypes.interval.Interval)
    */
   PeriodIF mergeRight(IntervalIF<TimeInstantIF> otherInterval);

   /*
    * (non-Javadoc)
    * 
    * @see
    * stlib.datatypes.interval.Interval#intersection(stlib.datatypes.interval.Interval)
    */
   PeriodIF intersection(IntervalIF<TimeInstantIF> otherInterval);

}