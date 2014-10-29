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

package mmdb.data.indices;

import java.util.List;

import mmdb.data.MemoryRelation;
import mmdb.data.MemoryTuple;
import mmdb.data.indices.sorted.IndexRBTree;
import mmdb.data.indices.sorted.IndexSimpleArray;
import mmdb.data.indices.sorted.IndexTTree;
import mmdb.data.indices.unsorted.IndexSimpleHash;
import mmdb.error.index.IndexAddElementException;
import mmdb.error.index.IndexInitializationException;
import mmdb.error.index.IndexSearchElementException;
import mmdb.error.index.IndexingException;
import mmdb.error.memory.MemoryException;
import mmdb.service.MemoryWatcher;

/**
 * Superclass for all index types.
 *
 * @author Alexander Castor
 */
public abstract class MemoryIndex<T> {

	/**
	 * Enum for collecting all index types.
	 */
	public static enum IndexType {
		T_TREE(IndexTTree.class), RB_TREE(IndexRBTree.class), SIMPLE_ARRAY(IndexSimpleArray.class), SIMPLE_HASH(
				IndexSimpleHash.class);

		public final Class<? extends MemoryIndex<?>> indexClass;

		IndexType(Class<? extends MemoryIndex<?>> indexClass) {
			this.indexClass = indexClass;
		}
	}

	/**
	 * Generates the actual index structure.
	 * 
	 * @param attributeIndex
	 *            the index position of the attribute in the header
	 * @param relation
	 *            the relation this index is created for
	 * @return the number if tuples that are linked to the index
	 * @throws IndexingException
	 * @throws MemoryException
	 */
	@SuppressWarnings("unchecked")
	public int create(int attributeIndex, MemoryRelation relation) throws IndexingException,
			MemoryException {
		int count = 0;
		for (MemoryTuple tuple : relation.getTuples()) {
			T attribute = (T) tuple.getAttribute(attributeIndex);
			insertElement(attribute, tuple);
			count++;
			if (count % MemoryWatcher.MEMORY_CHECK_FREQUENCY == 0) {
				MemoryWatcher.getInstance().checkMemoryStatus();
			}
		}
		initialize();
		return count;
	}

	/**
	 * Searches the index for a given attribute and returns all matching tuples
	 * 
	 * @param attribute
	 *            the attribute which is being searched
	 * @return the list of tuples matching the attribute
	 * @throws IndexSearchElementException
	 */
	public abstract List<MemoryTuple> searchElements(T attribute)
			throws IndexSearchElementException;

	/**
	 * Inserts an element to the index structure.
	 * 
	 * @param attribute
	 *            the attribute whose value is used for storing
	 * @param tuple
	 *            the tuple that will be stored
	 * @throws IndexAddElementException
	 */
	protected abstract void insertElement(T attribute, MemoryTuple tuple)
			throws IndexAddElementException;

	/**
	 * This method is called after all tuples are inserted into the index
	 * structure. Subclasses can override this method to do one time
	 * initialization tasks, for example sorting operations.
	 */
	protected void initialize() throws IndexInitializationException {
		// nothing to to in default
	}

}
