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
package stlib.util;

import static org.junit.Assert.assertEquals;

import org.junit.Test;

import stlib.util.CrossPointScalars;

/**
 * Trivial Tests for the 'CrossPointScalars' class
 * 
 * @author Markus Fuessel
 */
public class CrossPointScalarsTest {

   @Test
   public void testCrossPointScalars() {
      double timeScalar = 0.3;
      double positionScalar = 0.345;

      CrossPointScalars cps = new CrossPointScalars(timeScalar, positionScalar);

      assertEquals(timeScalar, cps.timeScalar, 0.0d);
      assertEquals(positionScalar, cps.positionScalar, 0.0d);

   }

}
