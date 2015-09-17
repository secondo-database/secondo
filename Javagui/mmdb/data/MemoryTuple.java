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
import java.util.Arrays;
import java.util.List;

import mmdb.data.attributes.MemoryAttribute;

/**
 * This class is the object representation for tuples which are part of
 * relations.
 *
 * @author Alexander Castor
 */
public class MemoryTuple extends MemoryObject {

	/**
	 * Typecheck Info
	 */
	private List<RelationHeaderItem> typecheckInfo;

	/**
	 * The tuple's attributes.
	 */
	private List<MemoryAttribute> attributes = new ArrayList<MemoryAttribute>();

	/**
	 * Explicit Standard Constructor
	 */
	public MemoryTuple() {
	}

	/**
	 * Private constructor for TypecheckInstance Factorymethod
	 * 
	 * @param typecheckInfo
	 */
	private MemoryTuple(List<RelationHeaderItem> typecheckInfo) {
		this.typecheckInfo = typecheckInfo;
	}

	/**
	 * Create MemoryTuple from multiple MemoryAttributes
	 * 
	 * @param attributes
	 *            an Array of all Attributes to add
	 */
	public MemoryTuple(MemoryAttribute... attributes) {
		this.attributes.addAll(0, Arrays.asList(attributes));
	}

	/**
	 * Adds an attribute to the tuple.
	 * 
	 * @param identifier
	 *            the attribute's identifier
	 * @param attribute
	 *            the attribute to be added
	 */
	public void addAttribute(MemoryAttribute attribute) {
		attributes.add(attribute);
	}

	/**
	 * Retrieves an attribute from the tuple's attribute list.
	 * 
	 * @param identifier
	 *            the attribute's identifier
	 * @return the tuple being retrieved
	 */
	public MemoryAttribute getAttribute(int index) {
		return attributes.get(index);
	}

	/**
	 * Retrieves all attributes from the tuple.
	 * 
	 * @return the list of attributes
	 */
	public List<MemoryAttribute> getAttributes() {
		return attributes;
	}

	/**
	 * Factorymethod creating a typecheck instance for MemoryTuples. This
	 * instance contains only the type of the tuple (its [relation's] header)
	 * but not any data.
	 * 
	 * @param typecheckInfo
	 *            the typecheck instance's header.
	 * @return the typecheckInstance.
	 */
	public static MemoryTuple createTypecheckInstance(
			List<RelationHeaderItem> info) {
		return new MemoryTuple(info);
	}

	/**
	 * Retrieves the typecheck instance's info, its header.
	 * 
	 * @return the header stored in this typecheck instance.
	 */
	public List<RelationHeaderItem> getTypecheckInfo() {
		return typecheckInfo;
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
		if (!(other instanceof MemoryTuple)) {
			return false;
		}
		MemoryTuple otherTuple = (MemoryTuple) other;
		if (attributes.size() != otherTuple.getAttributes().size()) {
			return false;
		}
		for (int i = 0; i < attributes.size(); i++) {
			if (!attributes.get(i).equals(otherTuple.getAttribute(i))) {
				return false;
			}
		}
		return true;
	}

}
