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

package mol;

import org.junit.runner.RunWith;
import org.junit.runners.Suite;
import org.junit.runners.Suite.SuiteClasses;

import mol.suites.TestSuiteDatatypes;
import mol.suites.TestSuiteOperations;
import mol.suites.TestSuiteSpatialDatatypes;
import mol.suites.TestSuiteTemporalDatatypes;
import mol.suites.TestSuiteUtilClasses;

/**
 * Suite for collecting ALL tests of the MovingObjectsLibrary
 * 
 * @author Markus Fuessel
 *
 */
@RunWith(Suite.class)
@SuiteClasses({ TestSuiteDatatypes.class, TestSuiteSpatialDatatypes.class, TestSuiteUtilClasses.class,
      TestSuiteTemporalDatatypes.class, TestSuiteOperations.class })
public class MOLTestRunner {

}
