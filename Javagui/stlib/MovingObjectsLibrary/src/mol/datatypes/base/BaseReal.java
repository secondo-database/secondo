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

package mol.datatypes.base;

import mol.datatypes.GeneralType;
import mol.datatypes.features.Orderable;

/**
 * Class for representation of the 'real' data type
 * 
 * @author Markus Fuessel
 */
public class BaseReal extends GeneralType implements Orderable<BaseReal> {

   /**
    * The 'real' value
    */
   private final double value;

   /**
    * Simple constructor, creates an undefined 'BaseReal' object
    */
   public BaseReal() {
      this.value = 0.0d;
      setDefined(false);
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
   public BaseReal(final BaseReal original) {
      this.value = original.getValue();
      setDefined(original.isDefined());
   }

   /*
    * (non-Javadoc)
    * 
    * @see java.lang.Comparable#compareTo(java.lang.Object)
    */
   @Override
   public int compareTo(final BaseReal otherReal) {

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
      if (obj == null || !(obj instanceof BaseReal)) {
         return false;
      }

      if (this == obj) {
         return true;
      }

      BaseReal otherReal = (BaseReal) obj;

      return compareTo(otherReal) == 0;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.util.Orderable#before(java.lang.Object)
    */
   @Override
   public boolean before(BaseReal otherReal) {

      return (this.compareTo(otherReal) < 0);
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.util.Orderable#after(java.lang.Object)
    */
   @Override
   public boolean after(BaseReal otherReal) {

      return (this.compareTo(otherReal) > 0);
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.util.Orderable#adjacent(java.lang.Object)
    */
   @Override
   public boolean adjacent(BaseReal otherReal) {
      return false;
   }

   /*
    * (non-Javadoc)
    * 
    * @see java.lang.Object#toString()
    */
   @Override
   public String toString() {
      return "BaseReal [value='" + value + "', isDefined()=" + isDefined() + "]";
   }

   /**
    * Getter for the double value
    * 
    * @return the value
    */
   public double getValue() {
      return value;
   }

}
