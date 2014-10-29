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

package mmdb.data.indices.sorted;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import mmdb.data.MemoryTuple;
import mmdb.data.features.Orderable;
import mmdb.error.index.IndexAddElementException;
import mmdb.error.index.IndexSearchElementException;

/**
 * This class provides a red black tree index structure which is used in Java
 * TreeMaps.
 *
 * @author Alexander Castor
 */
public class IndexRBTree extends SortedIndex {

	/**
	 * The tree map containing the index.
	 */
	private Map<Orderable, List<MemoryTuple>> indexMap = new TreeMap<Orderable, List<MemoryTuple>>();;

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.indices.MemoryIndex#insertElement(java.lang.Object,
	 * mmdb.data.MemoryTuple)
	 */
	@Override
	protected void insertElement(Orderable attribute, MemoryTuple tuple)
			throws IndexAddElementException {
		try {
			List<MemoryTuple> tupleList = indexMap.get(attribute);
			if (tupleList == null) {
				tupleList = new ArrayList<MemoryTuple>();
				tupleList.add(tuple);
				indexMap.put(attribute, tupleList);
			} else {
				tupleList.add(tuple);
			}
		} catch (Throwable e) {
			throw new IndexAddElementException(
					"-> Technical error when inserting element to rb-tree index.");
		}
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.indices.MemoryIndex#searchElements(java.lang.Object)
	 */
	@Override
	public List<MemoryTuple> searchElements(Orderable attribute) throws IndexSearchElementException {
		try {
			List<MemoryTuple> result = indexMap.get(attribute);
			if (result == null) {
				result = new ArrayList<MemoryTuple>();
			}
			return result;
		} catch (Throwable e) {
			throw new IndexSearchElementException(
					"-> Technical error when searching element in rb-tree index.");
		}
	}

}
