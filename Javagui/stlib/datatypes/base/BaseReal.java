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

package stlib.datatypes.base;

import stlib.datatypes.GeneralType;
import stlib.interfaces.base.BaseRealIF;

/**
 * Class for representation of the 'real' data type
 * 
 * @author Markus Fuessel
 */
public class BaseReal extends GeneralType implements BaseRealIF {

   /**
    * The 'real' value
    */
   private final double value;

   /**
    * Simple constructor, creates an undefined 'BaseReal' object
    */
   public BaseReal() {
      this.value = 0.0d;
   }

   /**
    * Constructor, create an defined 'BaseReal' object
    * 
    * @param value
    *           - the double value
    */
   public BaseReal(final double value) {
      this.value = value;
      setDefined(true);

   }

   /**
    * Copy constructor
    * 
    * @param original
    *           - the 'BaseReal' object to copy
    */
   public BaseReal(final BaseRealIF original) {
      this.value = original.getValue();
      setDefined(original.isDefined());
   }

   /*
    * (non-Javadoc)
    * 
    * @see java.lang.Comparable#compareTo(java.lang.Object)
    */
   @Override
   public int compareTo(final BaseRealIF otherReal) {

      return Double.compare(value, otherReal.getValue());
   }

   /*
    * (non-Javadoc)
    * 
    * @see java.lang.Object#hashCode()
    */
   @Override
   public int hashCode() {
      return Double.hashCode(value);
   }

   /*
    * (non-Javadoc)
    * 
    * @see java.lang.Object#equals(java.lang.Object)
    */
   @Override
   public boolean equals(final Object obj) {
      if (!(obj instanceof BaseRealIF)) {
         return false;
      }

      if (this == obj) {
         return true;
      }

      BaseRealIF otherReal = (BaseRealIF) obj;

      return compareTo(otherReal) == 0;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.features.Orderable#before(java.lang.Object)
    */
   @Override
   public boolean before(BaseRealIF otherReal) {

      return (this.compareTo(otherReal) < 0);
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.features.Orderable#after(java.lang.Object)
    */
   @Override
   public boolean after(BaseRealIF otherReal) {

      return (this.compareTo(otherReal) > 0);
   }

   /**
    * If other 'BaseReal' object is adjacent to this.<br>
    * Always returns false because 'BaseReal' is considered as a continuous value
    * and there is always another 'BaseReal' object between two 'BaseReal' objects.
    * 
    * @see stlib.datatypes.util.Orderable#adjacent(java.lang.Object)
    */
   @Override
   public boolean adjacent(BaseRealIF otherReal) {
      return false;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.base.BaseRealIF#getValue()
    */
   @Override
   public double getValue() {
      return value;
   }

}
