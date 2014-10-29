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
import java.util.Collections;
import java.util.List;

import mmdb.data.MemoryTuple;
import mmdb.data.features.Orderable;
import mmdb.error.index.IndexAddElementException;
import mmdb.error.index.IndexInitializationException;
import mmdb.error.index.IndexSearchElementException;

/**
 * This class provides a sorted array index structure. Tuple retrieval is done
 * via binary search.
 *
 * @author Alexander Castor
 */
public class IndexSimpleArray extends SortedIndex {

	/**
	 * The list containing the index.
	 */
	private List<ListElement> indexList = new ArrayList<ListElement>();

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
			ListElement listElement = new ListElement(attribute, tuple);
			indexList.add(listElement);
		} catch (Throwable e) {
			throw new IndexAddElementException(
					"-> Technical error when inserting element to array index.");
		}
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.indices.MemoryIndex#searchElements(java.lang.Object)
	 */
	@Override
	public List<MemoryTuple> searchElements(Orderable attribute) throws IndexSearchElementException {
		List<MemoryTuple> result = new ArrayList<MemoryTuple>();
		ListElement searchKey = new ListElement(attribute, null);
		try {
			int index = Collections.binarySearch(indexList, searchKey);
			if (index >= 0) {
				for (int i = index; i < indexList.size(); i++) {
					ListElement currentElement = indexList.get(i);
					if (searchKey.equals(currentElement)) {
						result.add(currentElement.tuple);
					} else {
						break;
					}
				}
			}
		} catch (Throwable e) {
			throw new IndexSearchElementException(
					"-> Technical error when searching element in array index.");
		}
		return result;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.data.indices.MemoryIndex#initialize()
	 */
	@Override
	protected void initialize() throws IndexInitializationException {
		try {
			Collections.sort(indexList);
		} catch (Throwable e) {
			throw new IndexInitializationException(
					"-> Technical error when initializing array index.");
		}
	}

	/**
	 * This class represents an element of the index list. It consists of an
	 * indexable attribute which determines the sorting order and a reference to
	 * its corresponding tuple.
	 *
	 * @author Alexander Castor
	 */
	class ListElement implements Comparable<ListElement> {

		Orderable indexAttribute;
		MemoryTuple tuple;

		ListElement(Orderable indexAttribute, MemoryTuple tuple) {
			this.indexAttribute = indexAttribute;
			this.tuple = tuple;
		}

		@Override
		public int compareTo(ListElement other) {
			return indexAttribute.compareTo(other.indexAttribute);
		}

		@Override
		public boolean equals(Object obj) {
			if (obj == null) {
				return false;
			}
			ListElement other = (ListElement) obj;
			return indexAttribute.equals(other.indexAttribute);
		}

	}

}
