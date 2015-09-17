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

package mmdb.data;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.indices.MemoryIndex;
import mmdb.data.indices.MemoryIndex.IndexType;
import mmdb.error.convert.ConvertToObjectException;
import mmdb.error.index.IndexingException;
import mmdb.error.memory.MemoryException;
import sj.lang.ListExpr;

/**
 * This class represents memory relation objects.
 *
 * @author Alexander Castor
 */
public class MemoryRelation extends MemoryObject {

	/**
	 * The header of the relation containing pairs of identifier and type.
	 */
	private List<RelationHeaderItem> header;

	/**
	 * The list of tuples that belong to the relation.
	 */
	private List<MemoryTuple> tuples;

	/**
	 * Indices for this relation containing pairs of identifier and index.
	 */
	private Map<String, MemoryIndex<?>> indices;

	/**
	 * Creates a new relation object.
	 * 
	 * @param name
	 *            the name of the relation
	 * @param header
	 *            the header of the relation
	 */
	public MemoryRelation(List<RelationHeaderItem> header) {
		this.header = header;
		tuples = new ArrayList<MemoryTuple>();
		indices = new HashMap<String, MemoryIndex<?>>();
	}

	/**
	 * Creates and initializes a single new tuples from a nested list.
	 * 
	 * @param list
	 *            the nested list containing the tuple's values
	 * @throws ConvertToObjectException
	 */
	public void createTupleFromList(ListExpr list)
			throws ConvertToObjectException {
		ListExpr tmp = list;
		MemoryTuple tuple = new MemoryTuple();
		for (RelationHeaderItem headerItem : header) {
			Class<? extends MemoryAttribute> type = headerItem.getType();
			if (type == null) {
				throw new ConvertToObjectException("-> Type '"
						+ headerItem.getTypeName() + "' is not supported.");
			}
			try {
				MemoryAttribute attribute = type.newInstance();
				attribute.fromList(tmp.first());
				tuple.addAttribute(attribute);
				tmp = tmp.rest();
			} catch (Exception e) {
				throw new ConvertToObjectException(
						"-> Could not create tuples from nested list.");
			}
		}
		tuples.add(tuple);
	}

	/**
	 * Returns the list of all tuples of this relation.
	 * 
	 * @return the list of tuples
	 */
	public List<MemoryTuple> getTuples() {
		return tuples;
	}

	/**
	 * Setter for tuples.
	 * 
	 * @param tuples
	 *            the list of tuples
	 */
	public void setTuples(List<MemoryTuple> tuples) {
		this.tuples = tuples;
	}

	/**
	 * Creates a new index and adds it to the relation's index list.
	 * 
	 * @param identifier
	 *            the attribute's identifier for which this index is used
	 * @param index
	 *            the index to be added
	 * @throws IndexingException
	 * @throws MemoryException
	 */
	public void createIndex(String identifier, String indexType)
			throws IndexingException, MemoryException {
		MemoryIndex<?> index = null;
		try {
			Class<? extends MemoryIndex<?>> indexClass = IndexType
					.valueOf(indexType).indexClass;
			index = indexClass.newInstance();
		} catch (Exception e) {
			throw new IndexingException("-> Could not instantiate index class.");
		}
		index.create(getHeaderIndex(identifier), this);
		indices.put(identifier, index);
	}

	/**
	 * Returns the list of all indices.
	 * 
	 * @return the index list
	 */
	public Map<String, MemoryIndex<?>> getIndices() {
		return indices;
	}

	/**
	 * Returns the index for a given attribute's identifier name.
	 * 
	 * @param identifier
	 *            the attribute's identifier for which this index is used
	 * @return the memory index
	 */
	public MemoryIndex<?> getIndex(String identifier) {
		return indices.get(identifier);
	}

	/**
	 * Removes an existing index from the relation's index list.
	 * 
	 * @param identifier
	 *            the attribute's identifier for which this index is used
	 */
	public void removeIndex(String identifier) {
		indices.remove(identifier);
	}

	/**
	 * Getter for header.
	 * 
	 * @return the header
	 */
	public List<RelationHeaderItem> getHeader() {
		return header;
	}

	/**
	 * Retrieves the header index position for a given identifier.
	 * 
	 * @param identifier
	 *            the identifier whose index position is being searched
	 * @return the index position of the identifier
	 */
	public int getHeaderIndex(String identifier) {
		int result = 0;
		for (int i = 0; i < header.size(); i++) {
			if (header.get(i).getIdentifier().equals(identifier)) {
				result = i;
				break;
			}
		}
		return result;
	}

	/**
	 * Retrieves the type name for a given identifier.
	 * 
	 * @param identifier
	 *            the identifier whose type name is being searched
	 * @return the index position of the identifier
	 */
	public String getTypeName(String identifier) {
		String result = null;
		for (RelationHeaderItem item : header) {
			if (item.getIdentifier().equals(identifier)) {
				result = item.getTypeName();
				break;
			}
		}
		return result;
	}

	/**
	 * Factorymethod creating a typecheck instance for MemoryRelations. This
	 * instance contains only the type of the relation (its header) but not any
	 * data.
	 * 
	 * @param typecheckInfo
	 *            the typecheck instance's header.
	 * @return the typecheckInstance.
	 */
	public static MemoryRelation createTypecheckInstance(
			List<RelationHeaderItem> typecheckInfo) {
		return new MemoryRelation(typecheckInfo);
	}

	/**
	 * Retrieves the typecheck instance's info, its header.
	 * 
	 * @return the header stored in this typecheck instance.
	 */
	public List<RelationHeaderItem> getTypecheckInfo() {
		return header;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Object#equals(java.lang.Object)
	 */
	@Override
	public boolean equals(Object other) {
		if (other == this) {
			return true;
		}
		if (other == null) {
			return false;
		}
		if (!(other instanceof MemoryRelation)) {
			return false;
		}
		MemoryRelation otherRel = (MemoryRelation) other;
		if (!otherRel.getHeader().equals(header)) {
			return false;
		}
		if (!otherRel.getTuples().equals(tuples)) {
			return false;
		}
		return true;
	}

}
