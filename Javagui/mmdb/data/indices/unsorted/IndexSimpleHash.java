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

package mmdb.data.indices.unsorted;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import mmdb.data.MemoryTuple;
import mmdb.data.features.Matchable;
import mmdb.error.index.IndexAddElementException;
import mmdb.error.index.IndexSearchElementException;

/**
 * This class provides a simple hash index structure. Tuples are stored in a
 * hash map.
 *
 * @author Alexander Castor
 */
public class IndexSimpleHash extends UnsortedIndex {

	/**
	 * The hash map containing the index.
	 */
	private Map<Matchable, List<MemoryTuple>> indexMap = new HashMap<Matchable, List<MemoryTuple>>();

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.indices.MemoryIndex#insertElement(java.lang.Object,
	 * mmdb.data.MemoryTuple)
	 */
	@Override
	protected void insertElement(Matchable attribute, MemoryTuple tuple)
			throws IndexAddElementException {
		try {
			List<MemoryTuple> tupleList = indexMap.get(attribute);
			if (tupleList == null) {
				tupleList = Collections.synchronizedList(new ArrayList<MemoryTuple>());
				tupleList.add(tuple);
				indexMap.put(attribute, tupleList);
			} else {
				tupleList.add(tuple);
			}
		} catch (Throwable e) {
			throw new IndexAddElementException(
					"-> Technical error when inserting element to hash index.");
		}
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.indices.MemoryIndex#searchElements(java.lang.Object)
	 */
	@Override
	public List<MemoryTuple> searchElements(Matchable attribute) throws IndexSearchElementException {
		try {
			List<MemoryTuple> result = indexMap.get(attribute);
			if (result == null) {
				result = new ArrayList<MemoryTuple>();
			}
			return result;
		} catch (Throwable e) {
			throw new IndexSearchElementException(
					"-> Technical error when searching element in hash index.");
		}
	}

	/**
	 * Getter for indexMap.
	 * 
	 * @return the indexMap
	 */
	public Map<Matchable, List<MemoryTuple>> getIndexMap() {
		return indexMap;
	}

}
