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

import gui.ObjectList;
import gui.SecondoObject;

import java.util.ArrayList;
import java.util.List;

import mmdb.data.MemoryObject;
import mmdb.data.MemoryRelation;
import mmdb.data.MemoryRelation.RelationHeaderItem;
import mmdb.data.MemoryTuple;
import mmdb.error.convert.ConversionException;
import mmdb.error.convert.ConvertToListException;
import mmdb.error.convert.ConvertToObjectException;
import mmdb.error.memory.MemoryException;
import sj.lang.ListExpr;

/**
 * This class is responsible for converting nested list to memory relations and
 * vice versa.
 *
 * @author Alexander Castor
 */
public final class ObjectConverter {

	/**
	 * The class' singleton instance.
	 */
	private static ObjectConverter instance = new ObjectConverter();

	/**
	 * Creates a new object converter object.
	 */
	private ObjectConverter() {
	}

	/**
	 * Retrieves the singleton instance.
	 * 
	 * @return the singleton instance
	 */
	public static ObjectConverter getInstance() {
		return instance;
	}

	/**
	 * Converts a given nested list to a memory relation object.
	 * 
	 * @param nestedList
	 *            the nested list to be converted
	 * @return the converted memory relation
	 * @throws ConvertToObjectException
	 * @throws MemoryException
	 */
	public MemoryRelation convertListToObject(ListExpr nestedList) throws ConvertToObjectException,
			MemoryException {
		if (!isListRelation(nestedList)) {
			throw new ConvertToObjectException(
					"-> Nested list to be converted is not a valid relation.");
		}
		List<RelationHeaderItem> header = createHeaderFromList(nestedList);
		MemoryRelation relation = new MemoryRelation(header);
		ListExpr tuples = nestedList.second();
		int count = 0;
		while (!tuples.isEmpty()) {
			relation.createTupleFromList(tuples.first());
			tuples = tuples.rest();
			count++;
			if (count % MemoryWatcher.MEMORY_CHECK_FREQUENCY == 0) {
				MemoryWatcher.getInstance().checkMemoryStatus();
			}
		}
		return relation;
	}

	/**
	 * Converts a given memory object to a nested list.
	 * 
	 * @param memoryObject
	 *            the memory object to be converted
	 * @return the converted nested list
	 * @throws ConvertToListException
	 * @throws MemoryException
	 */
	public ListExpr convertObjectToList(MemoryObject memoryObject) throws ConvertToListException,
			MemoryException {
		ListExpr resultList = null;
		try {
			MemoryRelation memoryRelation = (MemoryRelation) memoryObject;
			List<RelationHeaderItem> header = memoryRelation.getHeader();
			List<MemoryTuple> tuples = memoryRelation.getTuples();
			ListExpr headerList = createHeaderList(header);
			ListExpr tuplesList = createTuplesList(header, tuples);
			resultList = ListExpr.twoElemList(headerList, tuplesList);
		} catch (Throwable e) {
			throw new ConvertToListException("-> Technical conversion failed.");
		}
		return resultList;
	}

	/**
	 * Adds the nested list representation to a given secondo object containing
	 * the memory object.
	 * 
	 * @param secondoObject
	 *            the secondo object containing the memory object
	 * @throws ConvertToListException
	 * @throws MemoryException
	 */
	public void addNestedListToSecondoObject(SecondoObject secondoObject)
			throws ConvertToListException, MemoryException {
		ListExpr nestedList = convertObjectToList(secondoObject.getMemoryObject());
		secondoObject.fromList(nestedList);
		String newObjectName = secondoObject.getName().replace("[+]", "[++]");
		secondoObject.setName(newObjectName);
	}

	/**
	 * Add nested list representation to all objects that are passed.
	 * 
	 * @param objects
	 *            the objects to convert
	 * @param objectList
	 *            the object browser to be updated
	 * @return the list of failures
	 * @throws ConvertToListException
	 * @throws MemoryException
	 */
	public List<String> convertAllObjects(List<SecondoObject> objects, ObjectList objectList)
			throws ConvertToListException, MemoryException {
		if (objects.isEmpty()) {
			throw new ConvertToListException("-> There are no objects to convert.");
		}
		List<String> failures = new ArrayList<String>();
		for (SecondoObject object : objects) {
			try {
				objectList.removeObject(object);
				addNestedListToSecondoObject(object);
			} catch (ConversionException e) {
				failures.add("**" + object.getName() + "**");
			} finally {
				objectList.addEntry(object);
			}
		}
		if (objects.size() == failures.size()) {
			throw new ConvertToListException(
					"-> Could not convert any object from object explorer.");
		}
		return failures;
	}

	/**
	 * Creates the nested list header consisting of (rel(tuple ... ).
	 * 
	 * @param header
	 *            the header of the memory relation
	 * @return the header as nested list
	 * @throws ConvertToListException
	 */
	private ListExpr createHeaderList(List<RelationHeaderItem> header)
			throws ConvertToListException {
		ListExpr relList = null;
		try {
			ListExpr identifierTypePairs = new ListExpr();
			for (RelationHeaderItem item : header) {
				if (!item.isProjected()) {
					continue;
				}
				ListExpr identifierTypePair = ListExpr.twoElemList(
						ListExpr.symbolAtom(item.getIdentifier()),
						ListExpr.symbolAtom(item.getTypeName()));
				identifierTypePairs = ListExpr.concat(identifierTypePairs, identifierTypePair);
			}
			ListExpr tupleList = ListExpr.twoElemList(ListExpr.symbolAtom("tuple"),
					identifierTypePairs);
			relList = ListExpr.twoElemList(ListExpr.symbolAtom("rel"), tupleList);
		} catch (Exception e) {
			throw new ConvertToListException("-> Could not create list header.");
		}
		return relList;
	}

	/**
	 * Creates the second part of the relation list containing the tuples'
	 * values.
	 * 
	 * @param header
	 *            the header of the memory relation
	 * @param tuples
	 *            the list of tuples of the memory relation
	 * @return the tuple's values as nested list
	 * @throws ConvertToListException
	 * @throws MemoryException
	 */
	private ListExpr createTuplesList(List<RelationHeaderItem> header, List<MemoryTuple> tuples)
			throws ConvertToListException, MemoryException {
		ListExpr result = new ListExpr();
		ListExpr lastTuple = new ListExpr();
		boolean isFirst = true;
		int count = 0;
		for (MemoryTuple tuple : tuples) {
			ListExpr tupleList = new ListExpr();
			for (int i = 0; i < header.size(); i++) {
				if (!header.get(i).isProjected()) {
					continue;
				}
				try {
					ListExpr attributeList = tuple.getAttribute(i).toList();
					tupleList = ListExpr.concat(tupleList, attributeList);
				} catch (Exception e) {
					throw new ConvertToListException(
							"-> Could not create nested list for attribute '"
									+ header.get(i).getIdentifier() + "' of type '"
									+ header.get(i).getTypeName() + "'.");
				}
			}
			if (isFirst) {
				result = ListExpr.concat(result, tupleList);
				lastTuple = result;
				isFirst = false;
			} else {
				lastTuple = ListExpr.append(lastTuple, tupleList);
			}
			count++;
			if (count % MemoryWatcher.MEMORY_CHECK_FREQUENCY == 0) {
				MemoryWatcher.getInstance().checkMemoryStatus();
			}
		}
		return result;
	}

	/**
	 * Checks whether a given nested list is a relation.
	 * 
	 * @param nestedList
	 *            the nested list to be checked
	 * @return true if it is a valid relation, else false
	 * @throws ConvertToObjectException
	 */
	private boolean isListRelation(ListExpr nestedList) throws ConvertToObjectException {
		boolean isRelation = true;
		try {
			if (nestedList.listLength() != 2) {
				isRelation = false;
			} else {
				ListExpr type = nestedList.first();
				if (type.listLength() != 2) {
					isRelation = false;
				} else {
					ListExpr reltype = type.first();
					if (!reltype.isAtom()
							|| reltype.atomType() != ListExpr.SYMBOL_ATOM
							|| !(reltype.symbolValue().equals("rel")
									|| reltype.symbolValue().equals("mrel") || reltype
									.symbolValue().equals("trel"))) {
						isRelation = false;
					} else {
						ListExpr tupletype = type.second();
						ListExpr tupleFirst = tupletype.first();
						if (tupletype.listLength() != 2
								|| !tupleFirst.isAtom()
								|| tupleFirst.atomType() != ListExpr.SYMBOL_ATOM
								|| !(tupleFirst.symbolValue().equals("tuple") || tupleFirst
										.symbolValue().equals("mtuple"))) {
							isRelation = false;
						}
					}
				}
			}
		} catch (Exception e) {
			throw new ConvertToObjectException(
					"-> Could not check if nested list is a valid relation.");
		}
		return isRelation;
	}

	/**
	 * Creates the relation's header from a nested list.
	 * 
	 * @param nestedList
	 *            the nested list from which the header is created
	 * @return the header
	 * @throws ConvertToObjectException
	 */
	private List<RelationHeaderItem> createHeaderFromList(ListExpr nestedList)
			throws ConvertToObjectException {
		List<RelationHeaderItem> header = new ArrayList<RelationHeaderItem>();
		try {
			ListExpr tupleType = nestedList.first().second().second();
			while (!tupleType.isEmpty()) {
				ListExpr tupleSubType = tupleType.first();
				String identifier = tupleSubType.first().writeListExprToString().replace("\n", "");
				String type = tupleSubType.second().writeListExprToString().replace("\n", "");
				RelationHeaderItem item = new RelationHeaderItem(identifier, type);
				header.add(item);
				tupleType = tupleType.rest();
			}
		} catch (Exception e) {
			throw new ConvertToObjectException("-> Could not create relation header.");
		}
		return header;
	}

}