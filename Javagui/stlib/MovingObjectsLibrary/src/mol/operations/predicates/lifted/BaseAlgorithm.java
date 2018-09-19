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

package mol.operations.predicates.lifted;

import mol.datatypes.moving.MovingBool;
import mol.interfaces.GeneralTypeIF;
import mol.interfaces.moving.MovingBoolIF;
import mol.interfaces.moving.MovingObjectIF;
import mol.interfaces.unit.UnitObjectIF;

/**
 * Abstract class that provides a basic algorithm using the template method
 * pattern<br>
 * <br>
 * The algorithm runs through both passed 'MovingObject' and executes the
 * specific operation on 'unit' level, where they overlap in their time period.
 * The result is a 'MovingBool' object.<br>
 * <br>
 * {@code getUnitResult} - realizes the real operation on unit level, to
 * implement by a subclass
 * 
 * @author Markus Fuessel
 *
 */
public abstract class BaseAlgorithm {

   private MovingObjectIF<?, ?> mobject1;

   private MovingObjectIF<?, ?> mobject2;

   /**
    * Get the result of the operation, a 'MovingBool' object
    * 
    * @return a 'MovingBool' object
    */
   public final MovingBoolIF getResult() {
      MovingBoolIF mboolResult = new MovingBool(0);

      if (!preliminaryChecksSuccessful()) {
         mboolResult.setDefined(false);

      } else {
         int pos1 = 0;
         int pos2 = 0;

         UnitObjectIF<? extends GeneralTypeIF> uobject1 = mobject1.getUnit(pos1);
         UnitObjectIF<? extends GeneralTypeIF> uobject2 = mobject2.getUnit(pos2);

         while (uobject1.isDefined() && uobject2.isDefined()) {

            if (uobject1.before(uobject2)) {
               pos1++;
               uobject1 = mobject1.getUnit(pos1);

            } else if (uobject2.before(uobject1)) {
               pos2++;
               uobject2 = mobject2.getUnit(pos2);

            } else {
               MovingBoolIF mboolUnits = getUnitResult(uobject1, uobject2);

               mboolResult.add(mboolUnits);

               if (uobject1.periodEndsWithin(uobject2)) {
                  pos1++;
                  uobject1 = mobject1.getUnit(pos1);

               } else if (uobject2.periodEndsWithin(uobject1)) {
                  pos2++;
                  uobject2 = mobject2.getUnit(pos2);

               } else {
                  pos1++;
                  uobject1 = mobject1.getUnit(pos1);

                  pos2++;
                  uobject2 = mobject2.getUnit(pos2);
               }

            }

         }

      }

      return mboolResult;
   }

   /**
    * Preliminary checks before executing the operation
    * 
    * @return true if all checks where successful, false otherwise
    */
   public final boolean preliminaryChecksSuccessful() {
      if (mobject1 == null || mobject2 == null) {
         return false;
      }

      if (!mobject1.isDefined() || !mobject2.isDefined()) {
         return false;
      }

      if (!mobject1.getPeriods().intersects(mobject2.getPeriods())) {
         return false;
      }

      return additionalChecksSuccessful();
   }

   /**
    * @return the mobject1
    */
   @SuppressWarnings("unchecked")
   protected MovingObjectIF<UnitObjectIF<GeneralTypeIF>, GeneralTypeIF> getMobject1() {
      return (MovingObjectIF<UnitObjectIF<GeneralTypeIF>, GeneralTypeIF>) mobject1;
   }

   /**
    * @return the mobject2
    */
   @SuppressWarnings("unchecked")
   protected MovingObjectIF<UnitObjectIF<GeneralTypeIF>, GeneralTypeIF> getMobject2() {
      return (MovingObjectIF<UnitObjectIF<GeneralTypeIF>, GeneralTypeIF>) mobject2;
   }

   /**
    * @param mobject1
    *           the mobject1 to set
    */
   public void setMObject1(final MovingObjectIF<? extends UnitObjectIF<? extends GeneralTypeIF>, ? extends GeneralTypeIF> mobject1) {
      this.mobject1 = mobject1;
   }

   /**
    * @param mobject2
    *           the mobject2 to set
    */
   public void setMObject2(final MovingObjectIF<? extends UnitObjectIF<? extends GeneralTypeIF>, ? extends GeneralTypeIF> mobject2) {
      this.mobject2 = mobject2;
   }

   /**
    * Method for additional checks before executing the operation.<br>
    * To implement by the subclass.<br>
    * Return just {@code true} if no further checks needed
    * 
    * @return true, if additional checks where successful, false otherwise
    */
   public abstract boolean additionalChecksSuccessful();

   /**
    * This method realizes the real operation on unit level.<br>
    * The passed 'UnitObjects' intersects in their defined time period.
    * 
    * @param uobject1
    * @param uobject2
    * 
    * @return a 'MovingBool' object
    */
   public abstract MovingBoolIF getUnitResult(final UnitObjectIF<? extends GeneralTypeIF> uobject1,
                                              final UnitObjectIF<? extends GeneralTypeIF> uobject2);
}
