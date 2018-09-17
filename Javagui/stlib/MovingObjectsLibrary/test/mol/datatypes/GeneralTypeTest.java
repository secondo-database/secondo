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

import static org.junit.Assert.*;

import org.junit.Before;
import org.junit.Test;

/**
 * Tests for base class 'GeneralType'
 * 
 * @author Markus Fuessel
 */
public class GeneralTypeTest {

   public GeneralType typeInstance;

   @Before
   public void setUp() {
      typeInstance = new GeneralType();
   }

   @Test
   public void testSetDefined_setTrue() {
      typeInstance.setDefined(true);

      assertTrue(typeInstance.isDefined());
   }

   @Test
   public void testSetDefined_setFalse() {
      typeInstance.setDefined(false);

      assertFalse(typeInstance.isDefined());
   }

}
