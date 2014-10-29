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
import mmdb.data.attributes.MemoryAttribute;
import mmdb.error.memory.MemoryException;
import mmdb.error.query.QueryException;
import mmdb.error.query.UnionException;
import mmdb.service.MemoryWatcher;

/**
 * This class is responsible for executing UNION queries.
 *
 * @author Alexander Castor
 */
public class UnionController extends AbstractQueryController {

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.query.AbstractQueryController#executeQuery(java.lang.Object[])
	 */
	@Override
	public MemoryRelation executeQuery(Object... parameters) throws QueryException, MemoryException {
		MemoryRelation result = null;
		try {
			MemoryWatcher.getInstance().checkMemoryStatus();
			MemoryRelation firstRelation = (MemoryRelation) parameters[0];
			MemoryRelation secondRelation = (MemoryRelation) parameters[1];
			List<RelationHeaderItem> firstHeader = firstRelation.getHeader();
			secondRelation = adjustSecondRelation(secondRelation, firstHeader);
			result = doUnion(firstRelation, secondRelation, firstHeader);
		} catch (Throwable e) {
			if (e instanceof MemoryException) {
				throw (MemoryException) e;
			}
			throw new UnionException(e);
		}
		return result;
	}

	/**
	 * Adjusts the second relation to the first one. Necessary if headers are
	 * sorted in a different way.
	 * 
	 * @param secondRelation
	 *            the second relation that needs to be adjusted
	 * @param firstHeader
	 *            the first relation's header
	 * @return the second relation in the ordering of the first relation
	 */
	private MemoryRelation adjustSecondRelation(MemoryRelation secondRelation,
			List<RelationHeaderItem> firstHeader) {
		List<RelationHeaderItem> secondHeader = secondRelation.getHeader();
		if (firstHeader.equals(secondHeader)) {
			return secondRelation;
		}
		List<Integer> positions = getPositions(firstHeader, secondHeader);
		List<MemoryTuple> tuples = new ArrayList<MemoryTuple>(secondRelation.getTuples().size());
		for (MemoryTuple tuple : secondRelation.getTuples()) {
			MemoryTuple newTuple = new MemoryTuple();
			for (int i = 0; i < positions.size(); i++) {
				MemoryAttribute attribute = tuple.getAttribute(i);
				newTuple.addAttribute(attribute);
			}
			tuples.add(newTuple);
		}
		MemoryRelation result = new MemoryRelation(firstHeader);
		result.setTuples(tuples);
		return result;
	}

	/**
	 * Determines the list of attribute positions of the second relation
	 * corresponding to the first relation.
	 * 
	 * @param firstHeader
	 *            the first relation's header
	 * @param secondHeader
	 *            the second relation's header
	 * @return the list of attribute positions
	 */
	private List<Integer> getPositions(List<RelationHeaderItem> firstHeader,
			List<RelationHeaderItem> secondHeader) {
		List<Integer> positions = new ArrayList<Integer>();
		for (RelationHeaderItem headerItemFirst : firstHeader) {
			for (int i = 0; i < secondHeader.size(); i++) {
				if (headerItemFirst.getIdentifier().equals(secondHeader.get(i).getIdentifier())) {
					positions.add(i);
					break;
				}
			}
		}
		return positions;
	}

	/**
	 * Unions the tuples of both relations.
	 * 
	 * @param firstRelation
	 *            the first relation
	 * @param secondRelation
	 *            the second relation
	 * @param header
	 *            the first relation's header
	 * @return the result relation
	 */
	private MemoryRelation doUnion(MemoryRelation firstRelation, MemoryRelation secondRelation,
			List<RelationHeaderItem> header) {
		int capacity = firstRelation.getTuples().size() + secondRelation.getTuples().size();
		List<MemoryTuple> tuples = new ArrayList<MemoryTuple>(capacity);
		MemoryRelation result = new MemoryRelation(header);
		List<MemoryTuple> firstTupleList = firstRelation.getTuples();
		List<MemoryTuple> secondTupleList = secondRelation.getTuples();
		tuples.addAll(firstTupleList);
		tuples.addAll(secondTupleList);
		result.setTuples(tuples);
		return result;
	}

}
