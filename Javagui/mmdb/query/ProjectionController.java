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
import mmdb.error.memory.MemoryException;
import mmdb.error.query.ProjectionException;
import mmdb.error.query.QueryException;
import mmdb.service.MemoryWatcher;

/**
 * This class is responsible for executing PROJECTION queries.
 *
 * @author Alexander Castor
 */
public class ProjectionController extends AbstractQueryController {

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.query.AbstractQueryController#executeQuery(java.lang.Object[])
	 */
	@Override
	@SuppressWarnings("unchecked")
	public MemoryRelation executeQuery(Object... parameters) throws QueryException, MemoryException {
		MemoryRelation result = null;
		try {
			MemoryWatcher.getInstance().checkMemoryStatus();
			MemoryRelation relation = (MemoryRelation) parameters[0];
			List<String> attributes = (List<String>) parameters[1];
			List<RelationHeaderItem> header = createHeader(relation.getHeader(), attributes);
			result = new MemoryRelation(header);
			result.setTuples(relation.getTuples());
		} catch (Throwable e) {
			if (e instanceof MemoryException) {
				throw (MemoryException) e;
			}
			throw new ProjectionException(e);
		}
		return result;
	}

	/**
	 * Creates the header for the result relation containing only the selected
	 * attributes.
	 * 
	 * @param header
	 *            the header of the input relation
	 * @param attributes
	 *            the selected attributes
	 * @return the header for the result relation
	 */
	private List<RelationHeaderItem> createHeader(List<RelationHeaderItem> header,
			List<String> attributes) {
		List<RelationHeaderItem> newHeader = new ArrayList<RelationHeaderItem>();
		for (RelationHeaderItem item : header) {
			RelationHeaderItem newItem = new RelationHeaderItem(item.getIdentifier(),
					item.getTypeName());
			if (!attributes.contains(item.getIdentifier())) {
				newItem.setProjected(false);
			}
			newHeader.add(newItem);
		}
		return newHeader;
	}

}
