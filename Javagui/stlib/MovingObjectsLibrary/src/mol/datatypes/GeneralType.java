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

package mol.datatypes;

import mol.interfaces.GeneralTypeIF;

/**
 * Base class for all data type classes of this library
 * 
 * @author Markus Fuessel
 */
public class GeneralType implements GeneralTypeIF {

   /**
    * The defined flag, indicates if a data type object is defined
    */
   private boolean defined;

   /**
    * Constructor for an undefined 'GeneralType' object
    */
   public GeneralType() {
      this.defined = false;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.interfaces.GeneralTypeIF#isDefined()
    */
   @Override
   public boolean isDefined() {
      return defined;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.interfaces.GeneralTypeIF#setDefined(boolean)
    */
   @Override
   public void setDefined(final boolean defined) {
      this.defined = defined;
   }

}
