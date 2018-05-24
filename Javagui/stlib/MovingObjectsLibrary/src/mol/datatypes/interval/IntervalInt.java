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

package mol.datatypes.interval;

import mol.datatypes.base.BaseInt;

/**
 * Concrete class to represent integer interval objects
 * 
 * @author Markus Fuessel
 *
 */
public class IntervalInt extends Interval<BaseInt> {

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
   public IntervalInt(final BaseInt lowerBound, final BaseInt upperBound, final boolean leftClosed,
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
   private IntervalInt(IntervalInt original) {
      this(new BaseInt(original.getLowerBound()), new BaseInt(original.getUpperBound()), original.isLeftClosed(),
            original.isRightClosed());
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.interval.Interval#clone()
    */
   @Override
   public IntervalInt clone() {
      return new IntervalInt(this);
   }

}