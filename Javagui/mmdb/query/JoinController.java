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
import java.util.concurrent.ConcurrentHashMap;

import mmdb.data.MemoryRelation;
import mmdb.data.MemoryRelation.RelationHeaderItem;
import mmdb.data.MemoryTuple;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.features.Matchable;
import mmdb.data.indices.MemoryIndex;
import mmdb.data.indices.unsorted.IndexSimpleHash;
import mmdb.error.memory.MemoryException;
import mmdb.error.query.JoinException;
import mmdb.error.query.QueryException;
import mmdb.operator.OperationController;
import mmdb.operator.OperationController.COperator;
import mmdb.service.MemoryWatcher;

/**
 * This class is responsible for executing JOIN queries.
 *
 * @author Alexander Castor
 */
public class JoinController extends AbstractQueryController {

	/**
	 * The smaller relation (less tuples).
	 */
	private MemoryRelation smallerRelation;

	/**
	 * The bigger relation (more tuples).
	 */
	private MemoryRelation biggerRelation;

	/**
	 * The attribute header index of the smaller relation.
	 */
	private int smallerAttributeHeaderIndex;

	/**
	 * The attribute header index of the bigger relation.
	 */
	private int biggerAttributeHeaderIndex;

	/**
	 * The attribute identifier of the smaller relation.
	 */
	private String smallerAttributeIdentifier;

	/**
	 * The attribute identifier of the smaller relation.
	 */
	private String biggerAttributeIdentifier;

	/**
	 * The operator method.
	 */
	private Method operatorMethod;

	/**
	 * Tuples of result relation.
	 */
	private List<MemoryTuple> outputTuples;

	/**
	 * The hash table for hash joins.
	 */
	private ConcurrentHashMap<Matchable, List<MemoryTuple>> hashMap;

	/**
	 * Flag to indicate whether attributes are switched when determining smaller
	 * and bigger relation.
	 */
	private boolean attributesSwitched = false;

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
			MemoryRelation firstRelation = (MemoryRelation) parameters[0];
			MemoryRelation secondRelation = (MemoryRelation) parameters[1];
			String firstAttribute = (String) parameters[2];
			String secondAttribute = (String) parameters[3];
			COperator operator = (COperator) parameters[4];
			queryModeNormal = (Boolean) parameters[5];
			initializeMembers(firstRelation, secondRelation, firstAttribute, secondAttribute,
					operator);
			List<RelationHeaderItem> newHeader = createNewHeader();
			if (COperator.EQUALS.equals(operator)) {
				performHashJoin();
			} else {
				performNestedLoopJoin();
			}
			if (queryModeNormal) {
				result = new MemoryRelation(newHeader);
				result.setTuples(outputTuples);
			} else {
				result = constructMeasureModeResult();
			}
		} catch (Throwable e) {
			throw new JoinException(e);
		}
		if (threadError != null) {
			if (threadError instanceof MemoryException) {
				throw (MemoryException) threadError;
			}
			throw new JoinException(threadError);
		}
		if (queryModeNormal && result.getTuples().isEmpty()) {
			throw new JoinException("-> No tuples matched condition.");
		}
		return result;
	}

	/**
	 * Initializes the class' members.
	 * 
	 * @param firstRelation
	 *            the first selected relation
	 * @param secondRelation
	 *            the second selected relation
	 * @param firstAttribute
	 *            the attribute of the first relation
	 * @param secondAttribute
	 *            the attribute of the second relation
	 * @param operator
	 *            the conditional operator
	 * @throws Exception
	 */
	private void initializeMembers(MemoryRelation firstRelation, MemoryRelation secondRelation,
			String firstAttribute, String secondAttribute, COperator operator) throws Exception {
		if (firstRelation.getTuples().size() > secondRelation.getTuples().size()) {
			biggerRelation = firstRelation;
			biggerAttributeHeaderIndex = firstRelation.getHeaderIndex(firstAttribute);
			biggerAttributeIdentifier = firstAttribute;
			smallerRelation = secondRelation;
			smallerAttributeHeaderIndex = secondRelation.getHeaderIndex(secondAttribute);
			smallerAttributeIdentifier = secondAttribute;
			attributesSwitched = false;
		} else {
			biggerRelation = secondRelation;
			biggerAttributeHeaderIndex = secondRelation.getHeaderIndex(secondAttribute);
			biggerAttributeIdentifier = secondAttribute;
			smallerRelation = firstRelation;
			smallerAttributeHeaderIndex = firstRelation.getHeaderIndex(firstAttribute);
			smallerAttributeIdentifier = firstAttribute;
			attributesSwitched = true;
		}
		operatorMethod = getOperatorMethod(operator);
		outputTuples = Collections.synchronizedList(new ArrayList<MemoryTuple>());
		hashMap = new ConcurrentHashMap<Matchable, List<MemoryTuple>>();
	}

	/**
	 * Retrieves the condition operator method used in the join condition.
	 * 
	 * @param operator
	 *            the conditional operator
	 * @return the operator method
	 * @throws Exception
	 */
	private Method getOperatorMethod(COperator operator) throws Exception {
		RelationHeaderItem smallerHeaderItem = smallerRelation.getHeader().get(
				smallerAttributeHeaderIndex);
		RelationHeaderItem biggerHeaderItem = biggerRelation.getHeader().get(
				biggerAttributeHeaderIndex);
		Class<? extends MemoryAttribute> firstArgument = smallerHeaderItem.getType();
		Class<? extends MemoryAttribute> secondArgument = biggerHeaderItem.getType();
		Method operatorMethod = OperationController.getCondMethod(operator, firstArgument,
				secondArgument);
		return operatorMethod;
	}

	/**
	 * Creates the header for the result relation. Old identifiers are renamed
	 * to avoid duplicate names.
	 * 
	 * @return the new header
	 */
	private List<RelationHeaderItem> createNewHeader() {
		List<RelationHeaderItem> newHeader = new ArrayList<RelationHeaderItem>();
		for (RelationHeaderItem item : smallerRelation.getHeader()) {
			if (!item.isProjected()) {
				continue;
			}
			RelationHeaderItem newItem = new RelationHeaderItem(item.getIdentifier() + "_R1",
					item.getTypeName());
			newHeader.add(newItem);
		}
		for (RelationHeaderItem item : biggerRelation.getHeader()) {
			if (!item.isProjected()) {
				continue;
			}
			RelationHeaderItem newItem = new RelationHeaderItem(item.getIdentifier() + "_R2",
					item.getTypeName());
			newHeader.add(newItem);
		}
		return newHeader;
	}

	/**
	 * Performs a hash join in case the conditional operator is EQUALS.
	 * 
	 * @throws Exception
	 */
	private void performHashJoin() throws Exception {
		MemoryIndex<?> biggerIndex = biggerRelation.getIndex(biggerAttributeIdentifier);
		MemoryIndex<?> smallerIndex = smallerRelation.getIndex(smallerAttributeIdentifier);
		if (smallerIndex != null && smallerIndex instanceof IndexSimpleHash) {
			hashMap = ((IndexSimpleHash) smallerIndex).getIndexMap();
			startHashThreads(biggerRelation, false, true);
			return;
		}
		if (biggerIndex != null && biggerIndex instanceof IndexSimpleHash) {
			hashMap = ((IndexSimpleHash) biggerIndex).getIndexMap();
			startHashThreads(smallerRelation, false, false);
			return;
		}
		startHashThreads(biggerRelation, true, true);
		threads.clear();
		if (threadError == null) {
			startHashThreads(smallerRelation, false, false);
		}
	}

	/**
	 * Starts the hash threads.
	 * 
	 * @param relation
	 *            the relation that is being hashed
	 * @param isBuild
	 *            indicates whether threads shall perform a build (true) or
	 *            probe (false) operation
	 * @param isBigger
	 *            indicates whether the relation is the bigger one (true) or the
	 *            smaller one (false
	 * @throws Exception
	 */
	private void startHashThreads(MemoryRelation relation, boolean isBuild, boolean isBigger)
			throws Exception {
		int[] segments = divideTupleListForThreads(relation.getTuples());
		Thread firstThread = this.new HashJoinThread(0, segments[0], isBuild, isBigger);
		threads.add(firstThread);
		firstThread.start();
		for (int i = 1; i < segments.length; i++) {
			if (segments[i] == -1) {
				break;
			}
			Thread thread = this.new HashJoinThread(segments[i - 1] + 1, segments[i], isBuild,
					isBigger);
			threads.add(thread);
			thread.start();
		}
		joinThreads();
	}

	/**
	 * Performs a nested loop join.
	 * 
	 * @throws Exception
	 */
	private void performNestedLoopJoin() throws Exception {
		int[] segmentsBigger = divideTupleListForThreads(biggerRelation.getTuples());
		Thread firstThread = this.new NestedLoopThread(0, segmentsBigger[0]);
		threads.add(firstThread);
		firstThread.start();
		for (int i = 1; i < segmentsBigger.length; i++) {
			if (segmentsBigger[i] == -1) {
				break;
			}
			Thread thread = this.new NestedLoopThread(segmentsBigger[i - 1] + 1, segmentsBigger[i]);
			threads.add(thread);
			thread.start();
		}
		joinThreads();
	}

	/**
	 * Thread for executing a nested loop join on a subset of the input tuples
	 * from rangeBegin to rangeEnd.
	 *
	 * @author Alexander Castor
	 */
	public class NestedLoopThread extends Thread {

		private int rangeBegin;
		private int rangeEnd;

		NestedLoopThread(int rangeBegin, int rangeEnd) {
			this.rangeBegin = rangeBegin;
			this.rangeEnd = rangeEnd;
		}

		@Override
		public void run() {
			List<MemoryTuple> resultTuples = new ArrayList<MemoryTuple>();
			List<MemoryTuple> biggerTuples = biggerRelation.getTuples();
			List<MemoryTuple> smallerTuples = smallerRelation.getTuples();
			int threadResults = 0;
			outerloop: for (int i = rangeBegin; i <= rangeEnd; i++) {
				try {
					MemoryTuple biggerTuple = biggerTuples.get(i);
					MemoryAttribute biggerAttribute = biggerTuple
							.getAttribute(biggerAttributeHeaderIndex);
					for (MemoryTuple smallerTuple : smallerTuples) {
						MemoryAttribute smallerAttribute = smallerTuple
								.getAttribute(smallerAttributeHeaderIndex);
						boolean match = false;
						if (attributesSwitched) {
							match = (Boolean) operatorMethod.invoke(null, smallerAttribute,
									biggerAttribute);
						} else {
							match = (Boolean) operatorMethod.invoke(null, biggerAttribute,
									smallerAttribute);
						}
						if (!queryModeNormal) {
							threadResults = match ? ++threadResults : threadResults;
							continue;
						}
						if (match) {
							MemoryTuple newTuple = new MemoryTuple();
							newTuple.getAttributes().addAll(smallerTuple.getAttributes());
							newTuple.getAttributes().addAll(biggerTuple.getAttributes());
							resultTuples.add(newTuple);
						}
					}
					if (queryModeNormal && i % COPY_BATCH_SIZE == 0) {
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
					break outerloop;
				}
			}
			if (queryModeNormal) {
				outputTuples.addAll(resultTuples);
			} else {
				synchronizedIncrement(threadResults);
			}
		}

	}

	/**
	 * Thread for executing a hash join on a subset of the input tuples from
	 * rangeBegin to rangeEnd. Both build and probe operations are supported.
	 *
	 * @author Alexander Castor
	 */
	public class HashJoinThread extends Thread {

		private int rangeBegin;
		private int rangeEnd;
		private boolean isBuild;
		private boolean isBigger;

		HashJoinThread(int rangeBegin, int rangeEnd, boolean isBuild, boolean isBigger) {
			this.rangeBegin = rangeBegin;
			this.rangeEnd = rangeEnd;
			this.isBuild = isBuild;
			this.isBigger = isBigger;
		}

		@Override
		public void run() {
			if (isBuild) {
				build();
			} else {
				probe();
			}
		}

		void build() {
			List<MemoryTuple> tuples;
			if (isBigger) {
				tuples = biggerRelation.getTuples();
			} else {
				tuples = smallerRelation.getTuples();
			}
			for (int i = rangeBegin; i <= rangeEnd; i++) {
				try {
					MemoryTuple tuple = tuples.get(i);
					Matchable attribute;
					if (isBigger) {
						attribute = (Matchable) tuple.getAttribute(biggerAttributeHeaderIndex);
					} else {
						attribute = (Matchable) tuple.getAttribute(smallerAttributeHeaderIndex);
					}
					List<MemoryTuple> tupleList = hashMap.get(attribute);
					if (tupleList == null) {
						tupleList = Collections.synchronizedList(new ArrayList<MemoryTuple>());
						tupleList.add(tuple);
						List<MemoryTuple> existing = hashMap.putIfAbsent(attribute, tupleList);
						if (existing != null) {
							existing.add(tuple);
						}
					} else {
						tupleList.add(tuple);
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
		}

		void probe() {
			List<MemoryTuple> resultTuples = new ArrayList<MemoryTuple>();
			List<MemoryTuple> tuples = null;
			Matchable attribute = null;
			int threadResults = 0;
			if (isBigger) {
				tuples = biggerRelation.getTuples();
			} else {
				tuples = smallerRelation.getTuples();
			}
			for (int i = rangeBegin; i <= rangeEnd; i++) {
				try {
					MemoryTuple tuple = tuples.get(i);
					if (isBigger) {
						attribute = (Matchable) tuple.getAttribute(biggerAttributeHeaderIndex);
					} else {
						attribute = (Matchable) tuple.getAttribute(smallerAttributeHeaderIndex);
					}
					List<MemoryTuple> retrievedTuples = hashMap.get(attribute);
					if (retrievedTuples != null) {
						if (!queryModeNormal) {
							threadResults += retrievedTuples.size();
							continue;
						}
						for (MemoryTuple retrievedTuple : retrievedTuples) {
							MemoryTuple newTuple = new MemoryTuple();
							if (isBigger) {
								newTuple.getAttributes().addAll(retrievedTuple.getAttributes());
								newTuple.getAttributes().addAll(tuple.getAttributes());
							} else {
								newTuple.getAttributes().addAll(tuple.getAttributes());
								newTuple.getAttributes().addAll(retrievedTuple.getAttributes());
							}
							resultTuples.add(newTuple);
						}
					}
					if (queryModeNormal && i % COPY_BATCH_SIZE == 0) {
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