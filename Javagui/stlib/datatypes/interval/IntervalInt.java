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

package stlib.datatypes.interval;

import stlib.datatypes.base.BaseInt;
import stlib.interfaces.base.BaseIntIF;
import stlib.interfaces.interval.IntervalIntIF;

/**
 * Concrete class to represent 'IntervalInt' objects, an 'Interval' object with
 * 'BaseInt' objects as lower and upper bound.
 * 
 * @author Markus Fuessel
 *
 */
public class IntervalInt extends Interval<BaseIntIF> implements IntervalIntIF {

   /**
    * Constructor for an undefined 'IntervalInt' object
    */
   public IntervalInt() {
   }

   /**
    * Constructs an integer interval object
    * 
    * @param lowerBound
    *           BaseInt
    * @param upperBound
    *           BaseInt
    * @param leftClosed
    *           boolean
    * @param rightClosed
    *           boolean
    */
   public IntervalInt(final BaseIntIF lowerBound, final BaseIntIF upperBound, final boolean leftClosed,
                      final boolean rightClosed) {
      super(lowerBound, upperBound, leftClosed, rightClosed);
   }

   /**
    * Constructs an integer interval object
    * 
    * @param lowerBound
    *           int
    * @param upperBound
    *           int
    * @param leftClosed
    *           boolean
    * @param rightClosed
    *           boolean
    */
   public IntervalInt(final int lowerBound, final int upperBound, final boolean leftClosed, final boolean rightClosed) {
      this(new BaseInt(lowerBound), new BaseInt(upperBound), leftClosed, rightClosed);

   }

   /**
    * Copy constructor
    * 
    * @param original
    *           - the original 'IntervalInt' object to copy
    */
   private IntervalInt(final IntervalInt original) {
      this(new BaseInt(original.getLowerBound()), new BaseInt(original.getUpperBound()), original.isLeftClosed(),
            original.isRightClosed());
      setDefined(original.isDefined());
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.interval.Interval#copy()
    */
   @Override
   public IntervalInt copy() {
      return new IntervalInt(this);
   }

}
