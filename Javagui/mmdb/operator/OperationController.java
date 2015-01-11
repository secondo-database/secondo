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

package mmdb.operator;

import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;

import mmdb.operator.aggregation.AggregationOperator;
import mmdb.operator.aggregation.OperatorAverage;
import mmdb.operator.aggregation.OperatorMax;
import mmdb.operator.aggregation.OperatorMin;
import mmdb.operator.aggregation.OperatorSum;
import mmdb.operator.condition.ConditionOperator;
import mmdb.operator.condition.OperatorContains;
import mmdb.operator.condition.OperatorEquals;
import mmdb.operator.condition.OperatorEqualsGreater;
import mmdb.operator.condition.OperatorEqualsLess;
import mmdb.operator.condition.OperatorGreater;
import mmdb.operator.condition.OperatorLess;
import mmdb.operator.condition.OperatorPresent;
import mmdb.operator.condition.OperatorTrue;
import mmdb.operator.extension.ExtensionOperator;
import mmdb.operator.extension.OperatorAddString;
import mmdb.operator.extension.OperatorConcat;
import mmdb.operator.extension.OperatorRandomInt;
import mmdb.operator.extension.OperatorSumInt;
import mmdb.operator.extension.OperatorSumReal;

/**
 * This class is responsible for controlling all operator related tasks.
 *
 * @author Alexander Castor
 */
public abstract class OperationController {

	/**
	 * The name of operate-methods.
	 */
	public static final String OPERATE_METHOD = "operate";

	/**
	 * The name of init-methods for extension operators.
	 */
	public static final String INIT_METHOD = "initialize";

	/**
	 * Enum for collecting all aggregation operators.
	 */
	public static enum AOperator {
		AVERAGE(OperatorAverage.class), MAX(OperatorMax.class), MIN(OperatorMin.class), SUM(
				OperatorSum.class);

		public final Class<? extends AggregationOperator> operatorClass;

		AOperator(Class<? extends AggregationOperator> operatorClass) {
			this.operatorClass = operatorClass;
		}
	}

	/**
	 * Enum for collecting all condition operators.
	 */
	public static enum COperator {
		CONTAINS(OperatorContains.class, true), EQUALS(OperatorEquals.class, true), EQUALS_GREATER(
				OperatorEqualsGreater.class, true), EQUALS_LESS(OperatorEqualsLess.class, true), EQUALS_NOT(
				OperatorEquals.class, true), GREATER(OperatorGreater.class, true), LESS(
				OperatorLess.class, true), PRESENT(OperatorPresent.class, false), 
				TRUE(OperatorTrue.class, false);

		public final Class<? extends ConditionOperator> operatorClass;
		public final boolean parameterTypesEqual;

		COperator(Class<? extends ConditionOperator> operatorClass, boolean attributeAndValueEqual) {
			this.operatorClass = operatorClass;
			this.parameterTypesEqual = attributeAndValueEqual;
		}
	}

	/**
	 * Enum for collecting all extension operators.
	 */
	public static enum EOperator {
		ADD_STRING(OperatorAddString.class), CONCAT(OperatorConcat.class), RANDOM_INT(
				OperatorRandomInt.class), SUM_INT(OperatorSumInt.class), SUM_REAL(
				OperatorSumReal.class);

		public final Class<? extends ExtensionOperator> operatorClass;

		EOperator(Class<? extends ExtensionOperator> operatorClass) {
			this.operatorClass = operatorClass;
		}
	}

	/**
	 * Retrieves the attribute classes of a condition operator's operate-methods
	 * which is the first parameter of the method.
	 * 
	 * @param operator
	 *            the operator which will be used
	 * @return list of all attribute classes
	 */
	public static List<Class<?>> getCondAttributeClasses(COperator operator) {
		List<Class<?>> classes = new ArrayList<Class<?>>();
		Method[] methods = operator.operatorClass.getDeclaredMethods();
		for (Method method : methods) {
			if (!OPERATE_METHOD.equals(method.getName())) {
				continue;
			}
			Class<?>[] parameters = method.getParameterTypes();
			classes.add(parameters[0]);
		}
		return classes;
	}

	/**
	 * Retrieves the value classes of a condition operator's operate-methods
	 * which is the second parameter of the method depending on an attribute
	 * class which is the first parameter.
	 * 
	 * @param operator
	 *            the operator which will be used
	 * @param attributeClass
	 *            the attribute class
	 * @return list of all attribute classes
	 */
	public static List<Class<?>> getCondValueClasses(COperator operator, Class<?> attributeClass) {
		List<Class<?>> classes = new ArrayList<Class<?>>();
		Method[] methods = operator.operatorClass.getDeclaredMethods();
		for (Method method : methods) {
			Class<?>[] parameters = method.getParameterTypes();
			if (parameters.length == 2 && parameters[0].isAssignableFrom(attributeClass)) {
				classes.add(parameters[1]);
			}
		}
		return classes;
	}

	/**
	 * Retrieves the correct method for a condition operator and arguments.
	 * 
	 * @param operator
	 *            the operator which will be used
	 * @param firstArgument
	 *            the attribute used as first argument
	 * @param secondArgument
	 *            the attribute used as second argument
	 * @return the correct method of this operator
	 */
	public static Method getCondMethod(COperator operator, Class<?> firstArgument,
			Class<?> secondArgument) {
		Method result = null;
		Method[] methods = operator.operatorClass.getDeclaredMethods();
		for (Method method : methods) {
			if (!OPERATE_METHOD.equals(method.getName())) {
				continue;
			}
			Class<?> firstParameter = method.getParameterTypes()[0];
			Class<?> secondParameter = method.getParameterTypes()[1];
			if (firstParameter.isAssignableFrom(firstArgument)
					&& secondParameter.isAssignableFrom(secondArgument)) {
				result = method;
				break;
			}
		}
		return result;
	}

	/**
	 * Retrieves the method signature for an extension operator's
	 * operate-method.
	 * 
	 * @param operator
	 *            the operator whose signatures shall be retrieved
	 * @return the signature of the operate-method
	 */
	public static Class<?>[] getExtMethodSignature(EOperator operator) {
		Method[] methods = operator.operatorClass.getDeclaredMethods();
		for (Method method : methods) {
			if (OPERATE_METHOD.equals(method.getName())) {
				return method.getParameterTypes();
			}
		}
		return new Class<?>[0];
	}

	/**
	 * Retrieves the operate-method for an extension operator.
	 * 
	 * @param operator
	 *            the operator which will be used
	 * @param methodName
	 *            the method's name
	 * @return the operate-method of this operator
	 */
	public static Method getExtMethod(EOperator operator, String methodName) {
		Method[] methods = operator.operatorClass.getMethods();
		for (Method method : methods) {
			if (methodName.equals(method.getName())) {
				return method;
			}
		}
		return null;
	}

	/**
	 * Retrieves the parameter class for a given aggregation operator.
	 * 
	 * @param operator
	 *            the operator
	 * @return the operator's parameter class
	 */
	public static Class<?> getAggMethodParameter(AOperator operator) {
		Method[] methods = operator.operatorClass.getDeclaredMethods();
		for (Method method : methods) {
			if (OPERATE_METHOD.equals(method.getName())) {
				return method.getParameterTypes()[1];
			}
		}
		return null;
	}

	/**
	 * Retrieves the operate-method for an aggregation operator.
	 * 
	 * @param operator
	 *            the operator which will be used
	 * @param the
	 *            method's name
	 * @return the operate-method of this operator
	 */
	public static Method getAggMethod(AOperator operator) {
		Method[] methods = operator.operatorClass.getMethods();
		for (Method method : methods) {
			if (OPERATE_METHOD.equals(method.getName())) {
				return method;
			}
		}
		return null;
	}

}
