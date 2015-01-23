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
import mmdb.data.MemoryTuple;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.indices.MemoryIndex;
import mmdb.error.index.IndexSearchElementException;
import mmdb.error.memory.MemoryException;
import mmdb.error.query.QueryException;
import mmdb.error.query.SelectionException;
import mmdb.operator.OperationController;
import mmdb.operator.OperationController.COperator;
import mmdb.service.MemoryWatcher;
import tools.Reporter;

/**
 * This class is responsible for executing SELECTION queries.
 *
 * @author Alexander Castor
 */
public class SelectionController extends AbstractQueryController {

	/**
	 * Tuples of the selected relation.
	 */
	private List<MemoryTuple> inputTuples;

	/**
	 * Tuples of result relation.
	 */
	private List<MemoryTuple> outputTuples;

	/**
	 * Second argument of operator Method.
	 */
	private MemoryAttribute secondArgument;

	/**
	 * The operator method.
	 */
	private Method operatorMethod;

	/**
	 * Indicates whether the query is to run in normal mode.
	 */
	private boolean queryModeNormal;

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.query.AbstractQueryController#executeQuery(java.lang.Object[])
	 */
	@Override
	public MemoryRelation executeQuery(Object... parameters) throws QueryException, MemoryException {
		MemoryRelation result = null;
		try {
			threads.clear();
			resultInMeasureMode = 0;
			MemoryRelation relation = (MemoryRelation) parameters[0];
			COperator operator = (COperator) parameters[1];
			String firstArgumentName = (String) parameters[2];
			MemoryAttribute secondArgument = (MemoryAttribute) parameters[3];
			queryModeNormal = (Boolean) parameters[4];
			initializeMembers(relation, operator, firstArgumentName, secondArgument);
			if (!indexSelect(relation, operator, firstArgumentName, secondArgument)) {
				int[] segments = divideTupleListForThreads(relation.getTuples());
				startThreads(segments, relation.getHeaderIndex(firstArgumentName));
				joinThreads();
			}
			if (queryModeNormal) {
				result = new MemoryRelation(relation.getHeader());
				result.setTuples(outputTuples);
			} else {
				result = constructMeasureModeResult();
			}
		} catch (Throwable e) {
			throw new SelectionException(e);
		}
		if (threadError != null) {
			if (threadError instanceof MemoryException) {
				throw (SelectionException) threadError;
			}
			throw new SelectionException(threadError);
		}
		if (queryModeNormal && result.getTuples().isEmpty()) {
			throw new SelectionException("-> No tuples matched condition.");
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
	 * @param firstArgumentName
	 *            the operators's first argument as string
	 * @param secondArgument
	 *            the operator's second argument
	 */
	private void initializeMembers(MemoryRelation relation, COperator operator,
			String firstArgumentName, MemoryAttribute secondArgument) {
		inputTuples = relation.getTuples();
		outputTuples = Collections.synchronizedList(new ArrayList<MemoryTuple>());
		String attributeTypeName = relation.getTypeName(firstArgumentName);
		Class<? extends MemoryAttribute> attributeClass = MemoryAttribute
				.getTypeClass(attributeTypeName);
		operatorMethod = OperationController.getCondMethod(operator, attributeClass,
				secondArgument.getClass());
		this.secondArgument = secondArgument;
	}

	/**
	 * Checks whether the given attribute has an index and if so uses it to
	 * calculate the result tuples.
	 * 
	 * @param relation
	 *            the selected relation
	 * @param operator
	 *            the selected operator
	 * @param firstArgumentName
	 *            the operators's first argument as string
	 * @param secondArgument
	 *            the operator's second argument
	 */
	@SuppressWarnings({ "rawtypes", "unchecked" })
	private boolean indexSelect(MemoryRelation relation, COperator operator,
			String firstArgumentName, MemoryAttribute secondArgument) {
		if (COperator.EQUALS.equals(operator)) {
			MemoryIndex index = relation.getIndex(firstArgumentName);
			if (index != null) {
				try {
					if (queryModeNormal) {
						outputTuples = index.searchElements(secondArgument);
					} else {
						resultInMeasureMode = index.searchElements(secondArgument).size();
					}
					return true;
				} catch (IndexSearchElementException e) {
					Reporter.showInfo("Could not access index. Selection will be continued anyway.");
					return false;
				}
			}
		}
		return false;
	}

	/**
	 * Initializes and start the threads to perform the join.
	 * 
	 * @param segments
	 *            the list of segments for partitioning the input tuples
	 * @param headerIndex
	 *            the header index of the selected attribute
	 */
	private void startThreads(int[] segments, int headerIndex) {
		Thread firstThread = this.new SelectionThread(0, segments[0], headerIndex);
		threads.add(firstThread);
		firstThread.start();
		for (int i = 1; i < segments.length; i++) {
			if (segments[i] == -1) {
				break;
			}
			Thread thread = this.new SelectionThread(segments[i - 1] + 1, segments[i], headerIndex);
			threads.add(thread);
			thread.start();
		}
	}

	/**
	 * Thread for executing the select on a subset of the input tuples from
	 * rangeBegin to rangeEnd.
	 *
	 * @author Alexander Castor
	 */
	public class SelectionThread extends Thread {

		private int rangeBegin;
		private int rangeEnd;
		private int headerIndex;

		SelectionThread(int rangeBegin, int rangeEnd, int headerIndex) {
			this.rangeBegin = rangeBegin;
			this.rangeEnd = rangeEnd;
			this.headerIndex = headerIndex;
		}

		@Override
		public void run() {
			List<MemoryTuple> resultTuples = new ArrayList<MemoryTuple>();
			int threadResults = 0;
			for (int i = rangeBegin; i <= rangeEnd; i++) {
				try {
					MemoryTuple tuple = inputTuples.get(i);
					MemoryAttribute attribute = tuple.getAttribute(headerIndex);
					boolean match = (Boolean) operatorMethod
							.invoke(null, attribute, secondArgument);
					if (!queryModeNormal) {
						threadResults = match ? ++threadResults : threadResults;
						continue;
					}
					if (match) {
						resultTuples.add(tuple);
					}
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
			if (queryModeNormal) {
				outputTuples.addAll(resultTuples);
			} else {
				synchronizedIncrement(threadResults);
			}
		}

	}

}
