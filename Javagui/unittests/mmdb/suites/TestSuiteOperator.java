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

package unittests.mmdb.suites;

import unittests.mmdb.operator.OperationControllerTests;
import unittests.mmdb.operator.aggregation.AggregationOperatorTests;
import unittests.mmdb.operator.condition.ConditionOperatorTests;
import unittests.mmdb.operator.condition.OperatorPresentTests;
import unittests.mmdb.operator.extension.ExtensionOperatorTests;

import org.junit.runner.RunWith;
import org.junit.runners.Suite;
import org.junit.runners.Suite.SuiteClasses;

/**
 * Suite for collecting all tests within the "operator" module.
 *
 * @author Alexander Castor
 */
@RunWith(Suite.class)
@SuiteClasses({ OperationControllerTests.class, AggregationOperatorTests.class,
		ConditionOperatorTests.class, ExtensionOperatorTests.class, OperatorPresentTests.class })
public class TestSuiteOperator {

}
