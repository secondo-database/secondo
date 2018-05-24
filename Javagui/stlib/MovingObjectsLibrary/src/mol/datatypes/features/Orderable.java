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

package mol.datatypes.features;

/**
 * Objects of classes that implement this interface can be ordered.
 * 
 * @author Markus Fuessel
 *
 * @param <T>
 *           - specifies the concrete type
 */
public interface Orderable<T> extends Comparable<T> {

   /**
    * If this object lies in the order before the passed object.
    * 
    * @param other
    * @return true if this object lies in the order before the passed object, false
    *         otherwise
    */
   public boolean before(T other);

   /**
    * If this object lies in the order after the passed object.
    * 
    * @param other
    * @return true if this object lies in the order after the passed object, false
    *         otherwise
    */
   public boolean after(T other);

   /**
    * Check if this object is adjacent to the passed object
    * <p>
    * Depends on specific type {@code <T>}, implementation must therefore defined
    * by the concrete class
    * 
    * @param other
    *           - the other object
    * @return true if this object and the passed one are adjacent, false otherwise
    */
   public boolean adjacent(T other);
}
