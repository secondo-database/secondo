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
	public MemoryTuple() {}
	
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
	 * 			an Array of all Attributes to add
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
	 * Factorymethod for Typechecking-Instance
	 * 
	 * @param info
	 *            TypecheckInfo
	 * @return Typechecking Instance
	 */
	public static MemoryTuple createTypecheckInstance(
			List<RelationHeaderItem> info) {
		return new MemoryTuple(info);
	}

	/**
	 * Returns the Typechecking Info
	 * 
	 * @return Typechecking Info
	 */
	public List<RelationHeaderItem> getTypecheckInfo() {
		return typecheckInfo;
	}

}
