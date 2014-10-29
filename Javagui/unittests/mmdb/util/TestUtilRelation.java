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

package unittests.mmdb.util;

import java.util.ArrayList;
import java.util.List;

import mmdb.data.MemoryRelation;
import mmdb.data.MemoryRelation.RelationHeaderItem;
import mmdb.data.MemoryTuple;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.attributes.standard.AttributeString;
import sj.lang.ListExpr;

/**
 * This class provides commonly used testing objects concerning relations.
 *
 * @author Alexander Castor
 */
public class TestUtilRelation {

	public static MemoryRelation getIntStringRelation(int tupleCount, boolean stringFirst,
			boolean reversed) {
		RelationHeaderItem intHeaderItem = new RelationHeaderItem("identifierInt", "int");
		RelationHeaderItem stringHeaderItem = new RelationHeaderItem("identifierString", "string");
		List<RelationHeaderItem> header = new ArrayList<RelationHeaderItem>();
		if (!stringFirst) {
			header.add(intHeaderItem);
			header.add(stringHeaderItem);
		} else {
			header.add(stringHeaderItem);
			header.add(intHeaderItem);
		}
		MemoryRelation relation = new MemoryRelation(header);
		int fromCount = reversed ? (tupleCount * (-1)) : 1;
		int toCount = reversed ? -1 : tupleCount;
		for (int i = fromCount; i <= toCount; i++) {
			AttributeInt attributeInt = new AttributeInt();
			attributeInt.setValue(Math.abs(i));
			AttributeString attributeString = new AttributeString();
			attributeString.setValue("string_" + Math.abs(i));
			MemoryTuple tuple = new MemoryTuple();
			if (!stringFirst) {
				tuple.addAttribute(attributeInt);
				tuple.addAttribute(attributeString);
			} else {
				tuple.addAttribute(attributeString);
				tuple.addAttribute(attributeInt);
			}
			relation.getTuples().add(tuple);
		}
		return relation;
	}

	public static ListExpr getValidRelationList() {
		ListExpr intPart = ListExpr.twoElemList(ListExpr.symbolAtom("identifier"),
				ListExpr.symbolAtom("int"));
		ListExpr definitionPart = ListExpr.oneElemList(intPart);
		ListExpr tuplePart = ListExpr.twoElemList(ListExpr.symbolAtom("tuple"), definitionPart);
		ListExpr relPart = ListExpr.twoElemList(ListExpr.symbolAtom("rel"), tuplePart);
		ListExpr firstIntValue = ListExpr.oneElemList(ListExpr.intAtom(1));
		ListExpr secondIntValue = ListExpr.oneElemList(ListExpr.intAtom(2));
		ListExpr valuesPart = ListExpr.twoElemList(firstIntValue, secondIntValue);
		return ListExpr.twoElemList(relPart, valuesPart);
	}

	public static ListExpr getInvalidRelationList() {
		ListExpr intPart = ListExpr.twoElemList(ListExpr.symbolAtom("identifier"),
				ListExpr.symbolAtom("int"));
		ListExpr definitionPart = ListExpr.oneElemList(intPart);
		ListExpr tuplePart = ListExpr.twoElemList(ListExpr.symbolAtom("invalid"), definitionPart);
		ListExpr relPart = ListExpr.twoElemList(ListExpr.symbolAtom("rel"), tuplePart);
		ListExpr firstIntValue = ListExpr.oneElemList(ListExpr.intAtom(1));
		ListExpr secondIntValue = ListExpr.oneElemList(ListExpr.intAtom(2));
		ListExpr valuesPart = ListExpr.twoElemList(firstIntValue, secondIntValue);
		return ListExpr.twoElemList(relPart, valuesPart);
	}

}
