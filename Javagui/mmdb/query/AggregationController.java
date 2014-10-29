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

package mmdb.query;

import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;

import mmdb.data.MemoryRelation;
import mmdb.data.MemoryRelation.RelationHeaderItem;
import mmdb.data.MemoryTuple;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.error.memory.MemoryException;
import mmdb.error.query.AggregationException;
import mmdb.error.query.QueryException;
import mmdb.operator.OperationController;
import mmdb.operator.OperationController.AOperator;
import mmdb.service.MemoryWatcher;

/**
 * This class is responsible for executing AGGREGATION queries.
 *
 * @author Alexander Castor
 */
public class AggregationController extends AbstractQueryController {

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.query.AbstractQueryController#executeQuery(java.lang.Object[])
	 */
	@Override
	public MemoryRelation executeQuery(Object... parameters) throws QueryException, MemoryException {
		MemoryRelation result = null;
		try {
			MemoryWatcher.getInstance().checkMemoryStatus();
			MemoryRelation relation = (MemoryRelation) parameters[0];
			AOperator operator = (AOperator) parameters[1];
			String attributeIdentifier = (String) parameters[2];
			MemoryAttribute attribute = doAggregation(relation, operator, attributeIdentifier);
			result = createResultRelation(attribute, attributeIdentifier, operator.toString());
		} catch (Throwable e) {
			if (e instanceof MemoryException) {
				throw (MemoryException) e;
			}
			if (e instanceof AggregationException) {
				throw (AggregationException) e;
			}
			throw new AggregationException(e);
		}
		return result;
	}

	/**
	 * Performs the actual aggregation.
	 * 
	 * @param relation
	 *            the relation
	 * @param operator
	 *            the operator
	 * @param attributeIdentifier
	 *            the attribute's identifier
	 * @return the aggregation's result
	 * @throws Exception
	 */
	private MemoryAttribute doAggregation(MemoryRelation relation, AOperator operator,
			String attributeIdentifier) throws Exception {
		List<MemoryAttribute> inputAttributes = new ArrayList<MemoryAttribute>();
		int index = relation.getHeaderIndex(attributeIdentifier);
		for (MemoryTuple tuple : relation.getTuples()) {
			inputAttributes.add(tuple.getAttribute(index));
		}
		if (inputAttributes.isEmpty()) {
			throw new AggregationException("-> Relation does not contain any tuples to aggregate.");
		}
		Method operatorMethod = OperationController.getAggMethod(operator);
		return (MemoryAttribute) operatorMethod.invoke(null, inputAttributes,
				inputAttributes.get(0));
	}

	/**
	 * Creates the result relation containing the resulting attribute in a
	 * single tuple.
	 * 
	 * @param attribute
	 *            the attribute which was result of the aggregation
	 * @param attributeIdentifier
	 *            the original attribute's identifier
	 * @param operatorName
	 *            the operator's name
	 * @return the result relation
	 */
	private MemoryRelation createResultRelation(MemoryAttribute attribute,
			String attributeIdentifier, String operatorName) {
		String attributeType = MemoryAttribute.getTypeName(attribute.getClass());
		RelationHeaderItem item = new RelationHeaderItem(operatorName + "_" + attributeIdentifier,
				attributeType);
		List<RelationHeaderItem> header = new ArrayList<RelationHeaderItem>();
		header.add(item);
		MemoryRelation result = new MemoryRelation(header);
		MemoryTuple tuple = new MemoryTuple();
		tuple.addAttribute(attribute);
		result.getTuples().add(tuple);
		return result;
	}

}
