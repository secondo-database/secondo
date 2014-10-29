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
import java.util.Collections;
import java.util.List;

import mmdb.data.MemoryRelation;
import mmdb.data.MemoryRelation.RelationHeaderItem;
import mmdb.data.MemoryTuple;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.error.memory.MemoryException;
import mmdb.error.query.ExtensionException;
import mmdb.error.query.QueryException;
import mmdb.operator.OperationController;
import mmdb.operator.OperationController.EOperator;
import mmdb.operator.extension.ExtensionOperator;
import mmdb.service.MemoryWatcher;

/**
 * This class is responsible for executing EXTENSION queries.
 *
 * @author Alexander Castor
 */
public class ExtensionController extends AbstractQueryController {

	/**
	 * Tuples of the selected relation.
	 */
	private List<MemoryTuple> inputTuples;

	/**
	 * Tuples of result relation.
	 */
	private List<MemoryTuple> outputTuples;

	/**
	 * The operator's arguments.
	 */
	private List<String> arguments;

	/**
	 * The operator method.
	 */
	private Method operatorMethod;

	/**
	 * The operator object.
	 */
	private ExtensionOperator operatorObject;

	/**
	 * The argument attribute positions.
	 */
	private int[] headerIndices;

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.query.AbstractQueryController#executeQuery(java.lang.Object[])
	 */
	@SuppressWarnings("unchecked")
	@Override
	public MemoryRelation executeQuery(Object... parameters) throws QueryException, MemoryException {
		MemoryRelation result = null;
		try {
			threads.clear();
			MemoryRelation relation = (MemoryRelation) parameters[0];
			EOperator operator = (EOperator) parameters[1];
			String identifier = (String) parameters[2];
			List<String> arguments = (List<String>) parameters[3];
			initializeMembers(relation, operator, arguments);
			List<RelationHeaderItem> newHeader = createNewHeader(relation.getHeader(), identifier);
			result = new MemoryRelation(newHeader);
			OperationController.getExtMethod(operator, OperationController.INIT_METHOD).invoke(
					operatorObject);
			int[] segments = divideTupleListForThreads(relation.getTuples());
			startThreads(segments);
			joinThreads();
			result.setTuples(outputTuples);
		} catch (Throwable e) {
			throw new ExtensionException(e);
		}
		if (threadError != null) {
			if (threadError instanceof MemoryException) {
				throw (MemoryException) threadError;
			}
			throw new ExtensionException(threadError);
		}
		return result;
	}

	/**
	 * Initializes the class' members.
	 * 
	 * @param relation
	 *            the selected relation
	 * @param operator
	 *            the selected operator
	 * @param arguments
	 *            selected arguments of the operator
	 * @throws Exception
	 */
	private void initializeMembers(MemoryRelation relation, EOperator operator,
			List<String> arguments) throws Exception {
		inputTuples = relation.getTuples();
		outputTuples = Collections.synchronizedList(new ArrayList<MemoryTuple>());
		operatorMethod = OperationController.getExtMethod(operator,
				OperationController.OPERATE_METHOD);
		operatorObject = operator.operatorClass.newInstance();
		this.arguments = arguments;
		this.headerIndices = new int[arguments.size()];
		for (int i = 0; i < arguments.size(); i++) {
			headerIndices[i] = relation.getHeaderIndex(arguments.get(i));
		}
	}

	/**
	 * Creates the new header for the result relation by appending the extension
	 * argument.
	 * 
	 * @param header
	 *            the input relation's header
	 * @param identifier
	 *            the attribute's identifier
	 * @return the new header
	 */
	private List<RelationHeaderItem> createNewHeader(List<RelationHeaderItem> header,
			String identifier) {
		List<RelationHeaderItem> newHeader = new ArrayList<RelationHeaderItem>();
		for (RelationHeaderItem item : header) {
			if (!item.isProjected()) {
				continue;
			}
			RelationHeaderItem newItem = new RelationHeaderItem(item.getIdentifier(),
					item.getTypeName());
			newHeader.add(newItem);
		}
		String newType = MemoryAttribute.getTypeName(operatorMethod.getReturnType());
		RelationHeaderItem newItem = new RelationHeaderItem(identifier, newType);
		newHeader.add(newItem);
		return newHeader;
	}

	/**
	 * Initializes and start the threads to perform the extension.
	 * 
	 * @param segments
	 *            the list of segments for partitioning the input tuples
	 */
	private void startThreads(int[] segments) {
		Thread firstThread = this.new ExtensionThread(0, segments[0]);
		threads.add(firstThread);
		firstThread.start();
		for (int i = 1; i < segments.length; i++) {
			if (segments[i] == -1) {
				break;
			}
			Thread thread = this.new ExtensionThread(segments[i - 1] + 1, segments[i]);
			threads.add(thread);
			thread.start();
		}
	}

	/**
	 * Thread for executing the extension on a subset of the input tuples from
	 * rangeBegin to rangeEnd.
	 *
	 * @author Alexander Castor
	 */
	public class ExtensionThread extends Thread {

		private int rangeBegin;
		private int rangeEnd;

		ExtensionThread(int rangeBegin, int rangeEnd) {
			this.rangeBegin = rangeBegin;
			this.rangeEnd = rangeEnd;
		}

		@Override
		public void run() {
			List<MemoryTuple> resultTuples = new ArrayList<MemoryTuple>();
			for (int i = rangeBegin; i <= rangeEnd; i++) {
				try {
					MemoryTuple inputTuple = inputTuples.get(i);
					MemoryTuple newTuple = new MemoryTuple();
					newTuple.getAttributes().addAll(inputTuple.getAttributes());
					List<MemoryAttribute> argumentObjects = new ArrayList<MemoryAttribute>();
					for (int j = 0; j < arguments.size(); j++) {
						MemoryAttribute attribute = inputTuple.getAttribute(headerIndices[j]);
						argumentObjects.add(attribute);
					}
					MemoryAttribute extendedAttribute = invokeOperator(operatorMethod,
							argumentObjects);
					newTuple.addAttribute(extendedAttribute);
					resultTuples.add(newTuple);
					if (i % COPY_BATCH_SIZE == 0) {
						outputTuples.addAll(resultTuples);
						resultTuples.clear();
					}
					if (i % MemoryWatcher.MEMORY_CHECK_FREQUENCY == 0) {
						MemoryWatcher.getInstance().checkMemoryStatus();
					}
				} catch (Throwable e) {
					threadError = e;
				}
				if (threadError != null) {
					break;
				}
			}
			outputTuples.addAll(resultTuples);
		}

		/**
		 * Invokes the operator.
		 * 
		 * @param operatorMethod
		 *            the operator to be invoked
		 * @param argumentObjects
		 *            the operator's arguments
		 * @return the attribute which will extend the relation
		 * @throws Exception
		 */
		private MemoryAttribute invokeOperator(Method operatorMethod,
				List<MemoryAttribute> argumentObjects) throws Exception {
			if (argumentObjects.isEmpty()) {
				return (MemoryAttribute) operatorMethod.invoke(operatorObject);
			}
			if (argumentObjects.size() == 1) {
				return (MemoryAttribute) operatorMethod.invoke(operatorObject,
						argumentObjects.get(0));
			}
			if (argumentObjects.size() == 2) {
				return (MemoryAttribute) operatorMethod.invoke(operatorObject,
						argumentObjects.get(0), argumentObjects.get(1));
			}
			if (argumentObjects.size() == 3) {
				return (MemoryAttribute) operatorMethod.invoke(operatorObject,
						argumentObjects.get(0), argumentObjects.get(1), argumentObjects.get(2));
			}
			return null;
		}

	}
}
