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

package unittests.mmdb.operator;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import java.lang.reflect.Method;
import java.util.List;

import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.attributes.standard.AttributeString;
import mmdb.data.features.Matchable;
import mmdb.data.features.Summable;
import mmdb.operator.OperationController;
import mmdb.operator.OperationController.AOperator;
import mmdb.operator.OperationController.COperator;
import mmdb.operator.OperationController.EOperator;

import org.junit.Test;

/**
 * Tests for class "OperationController".
 *
 * @author Alexander Castor
 */
public class OperationControllerTests {

	@Test
	public void testGetCondAttributeClasses() {
		List<Class<?>> classes = OperationController.getCondAttributeClasses(COperator.EQUALS);
		assertTrue(classes.size() == 1);
		assertEquals(Matchable.class, classes.get(0));
	}

	@Test
	public void testGetCondValueClasses() {
		List<Class<?>> classes = OperationController.getCondValueClasses(COperator.EQUALS,
				AttributeInt.class);
		assertTrue(classes.size() == 1);
		assertEquals(Matchable.class, classes.get(0));
	}

	@Test
	public void testGetCondMethodValid() {
		Method method = OperationController.getCondMethod(COperator.EQUALS, AttributeInt.class,
				AttributeInt.class);
		assertEquals(OperationController.OPERATE_METHOD, method.getName());
	}

	@Test
	public void testGetCondMethodInvalid() {
		Method method = OperationController.getCondMethod(COperator.EQUALS, Integer.class,
				AttributeInt.class);
		assertNull(method);
	}

	@Test
	public void testGetExtMethodSignature() {
		Class<?>[] signature = OperationController.getExtMethodSignature(EOperator.CONCAT);
		assertTrue(signature.length == 2);
		assertEquals(AttributeString.class, signature[0]);
		assertEquals(AttributeString.class, signature[1]);
	}

	@Test
	public void testGetExtMethodValid() {
		Method method = OperationController.getExtMethod(EOperator.CONCAT,
				OperationController.INIT_METHOD);
		assertEquals(OperationController.INIT_METHOD, method.getName());
	}

	@Test
	public void testGetExtMethodInvalid() {
		Method method = OperationController.getExtMethod(EOperator.CONCAT, "invalid");
		assertNull(method);
	}

	@Test
	public void testGetAggMethodParameter() {
		Class<?> parameter = OperationController.getAggMethodParameter(AOperator.SUM);
		assertEquals(Summable.class, parameter);
	}

	@Test
	public void testGetAggMethod() {
		Method method = OperationController.getAggMethod(AOperator.SUM);
		assertEquals(OperationController.OPERATE_METHOD, method.getName());
	}

}
