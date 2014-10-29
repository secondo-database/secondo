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

package mmdb.service;

import gui.SecondoObject;

import java.util.List;

import mmdb.data.MemoryRelation;
import mmdb.error.memory.MemoryException;
import sj.lang.ListExpr;

/**
 * This class is responsible for watching the heap memory status to prevent
 * OutOfMemoryErrors. Besides this class is the only place where garbage
 * collections are performed.
 *
 * @author Alexander Castor
 */
public final class MemoryWatcher {

	/**
	 * The frequency of memory checks in loops that process tuples.
	 */
	public static final int MEMORY_CHECK_FREQUENCY = 100;

	/**
	 * The frequency in seconds of garbage collections performed in memory
	 * dialog.
	 */
	private static final int GC_FREQUENCY = 60;

	/**
	 * The minimum percentage of free memory of the heap which shall be always
	 * available. It is recommended to set this to at least 20% to ensure that
	 * there is enough space for computing intermediate results in queries.
	 */
	private static final int MIN_PERCENTAGE = 20;

	/**
	 * The minimum heap size in megabyte which shall be always available. It is
	 * recommended to set this to at least 64MB to ensure that there is enough
	 * space for computing intermediate results in queries.
	 */
	private static final int MIN_MB = 64;

	/**
	 * The maximum heap size in megabyte which shall be always available in case
	 * MIN_PERCENTAGE is bigger than MAX_MB, this will be used in memory checks.
	 */
	private static final int MAX_MB = 256;

	/**
	 * Constant for megabyte.
	 */
	private static final int MB = 1024 * 1024;

	/**
	 * The class' singleton instance.
	 */
	private static MemoryWatcher instance = new MemoryWatcher();

	/**
	 * Creates a new memory watcher object.
	 */
	private MemoryWatcher() {
	}

	/**
	 * Retrieves the singleton instance.
	 * 
	 * @return the singleton instance
	 */
	public static MemoryWatcher getInstance() {
		return instance;
	}

	/**
	 * Checks the memory status to prevent impending OutOfMemoryErrors and in
	 * these cases throws an exception.
	 * 
	 * @throws MemoryException
	 */
	public void checkMemoryStatus() throws MemoryException {
		Runtime runtime = Runtime.getRuntime();
		long total = runtime.maxMemory() / MB;
		long free = (total * MB - (runtime.totalMemory() - runtime.freeMemory())) / MB;
		long freeSpaceAllowed = (MIN_PERCENTAGE * total) / 100;
		long required = (MIN_MB > freeSpaceAllowed) ? MIN_MB : freeSpaceAllowed;
		required = (MAX_MB < freeSpaceAllowed) ? MAX_MB : freeSpaceAllowed;
		if (free < required) {
			throw new MemoryException(free, required);
		}
	}

	/**
	 * Returns the current heap memory statistics. Performs a garbage collection
	 * depending on the number of calls and the defined frequency.
	 * 
	 * @param counter
	 *            the number of calls
	 * @return ([0] = total, [1] = used, [2] = free)
	 */
	public String[] getMemoryStatistics(int calls) {
		if (calls % GC_FREQUENCY == 0) {
			System.gc();
		}
		String[] result = new String[3];
		Runtime runtime = Runtime.getRuntime();
		long total = runtime.maxMemory();
		long free = total - (runtime.totalMemory() - runtime.freeMemory());
		long used = total - free;
		result[0] = Long.toString(total / MB);
		result[1] = Long.toString(used / MB);
		result[2] = Long.toString(free / MB);
		return result;
	}

	/**
	 * Returns the object statistics for each secondo object currently loaded.
	 * Each row is one object.
	 * 
	 * @param objects
	 *            the list of currently loaded secondo objects
	 * 
	 * @return [0] = name, [1] = tuples, [2] = list, [3] = relation, [4] =
	 *         indices
	 */
	public String[][] getObjectStatistics(List<SecondoObject> objects) {
		String[][] result = new String[objects.size()][5];
		int counter = 0;
		for (SecondoObject object : objects) {
			MemoryRelation relation = (MemoryRelation) object.getMemoryObject();
			ListExpr list = object.toListExpr();
			result[counter][0] = object.getName();
			if (relation != null && !relation.getTuples().isEmpty()) {
				result[counter][1] = String.valueOf(relation.getTuples().size());
				result[counter][3] = "X";
			} else {
				result[counter][1] = "n/a";
				result[counter][3] = "";
			}
			if (list != null) {
				result[counter][2] = "X";
			} else {
				result[counter][2] = "";
			}
			if (relation != null && !relation.getIndices().keySet().isEmpty()) {
				result[counter][4] = "X";
			} else {
				result[counter][4] = "";
			}
			counter++;
		}
		return result;
	}

}
