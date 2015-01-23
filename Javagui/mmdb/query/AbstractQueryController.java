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

import java.util.ArrayList;
import java.util.List;

import mmdb.data.MemoryRelation;
import mmdb.data.MemoryRelation.RelationHeaderItem;
import mmdb.data.MemoryTuple;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.error.memory.MemoryException;
import mmdb.error.query.QueryException;

/**
 * Superclass for all query controllers.
 *
 * @author Alexander Castor
 */
public abstract class AbstractQueryController {

	/**
	 * The number of threads used to perform the selection, extension and join
	 * queries.
	 */
	public static final int NUMBER_OF_THREADS = Runtime.getRuntime().availableProcessors();

	/**
	 * Determines the size of batches when intermediate result tuples get copied
	 * to the result relation.
	 */
	public static final int COPY_BATCH_SIZE = 1000;

	/**
	 * The list of threads.
	 */
	protected List<Thread> threads = new ArrayList<Thread>();

	/**
	 * This field is shared by all threads to synchronize in cases of errors. If
	 * one thread fails, this field is being set to the corresponding throwable,
	 * all operation will be aborted and the user will be informed about the
	 * error.
	 */
	protected Throwable threadError = null;

	/**
	 * The result for selection and join queries in measure mode.
	 */
	protected Integer resultInMeasureMode = 0;

	/**
	 * Main method for executing queries which is overridden by corresponding
	 * controllers.
	 * 
	 * @param parameters
	 *            variable list of objects which each sub-controller needs to
	 *            parse individually
	 * @return the result relation
	 * @throws QueryException
	 * @throws MemoryException
	 */
	public abstract MemoryRelation executeQuery(Object... parameters) throws QueryException,
			MemoryException;

	/**
	 * Divides the tuple list in equally sized disjoint portions depending on
	 * the number of threads so that each thread can process one of this
	 * portions.
	 * 
	 * @param tuples
	 *            the tuple list to be divided
	 * @return an array of integers in which each entry is the end of a segment
	 */
	public int[] divideTupleListForThreads(List<MemoryTuple> tuples) {
		int[] result = new int[NUMBER_OF_THREADS];
		int tupleCount = tuples.size();
		int segmentSize = tupleCount / NUMBER_OF_THREADS;
		if (tupleCount == 1 || segmentSize == 0) {
			result[0] = tupleCount - 1;
			for (int i = 1; i < NUMBER_OF_THREADS; i++) {
				result[i] = -1;
			}
		} else {
			for (int i = 0; i < NUMBER_OF_THREADS - 1; i++) {
				result[i] = (segmentSize - 1) + (i * segmentSize);
			}
			result[NUMBER_OF_THREADS - 1] = tupleCount - 1;
		}
		return result;
	}

	/**
	 * Joins several threads to synchronize them.
	 * 
	 * @throws InterruptedException
	 */
	protected void joinThreads() throws InterruptedException {
		for (Thread thread : threads) {
			thread.join();
		}
	}

	/**
	 * Creates a new relation containing one tuple with one attribute storing
	 * the result count for measure mode.
	 * 
	 * @return a memory relation for measure mode
	 */
	protected MemoryRelation constructMeasureModeResult() {
		RelationHeaderItem item = new RelationHeaderItem("count", "int");
		List<RelationHeaderItem> header = new ArrayList<RelationHeaderItem>();
		header.add(item);
		MemoryRelation result = new MemoryRelation(header);
		MemoryTuple tuple = new MemoryTuple();
		AttributeInt attribute = new AttributeInt();
		attribute.setValue(resultInMeasureMode);
		tuple.addAttribute(attribute);
		result.getTuples().add(tuple);
		return result;
	}

	/**
	 * Thread-safe increment of an Integer.
	 * 
	 * @param increment
	 *            the increment
	 */
	protected synchronized void synchronizedIncrement(int increment) {
		resultInMeasureMode += increment;
	}

}
