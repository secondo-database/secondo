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

package mmdb.operator.extension;

import javax.swing.JOptionPane;

import mmdb.data.attributes.standard.AttributeString;

/**
 * This class represents the 'ADD' operator for strings.
 *
 * @author Alexander Castor
 */
public class OperatorAddString extends ExtensionOperator {

	/**
	 * The constant string to be added.
	 */
	private String constValue = "";

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.operator.extension.ExtensionOperator#init()
	 */
	@Override
	public void initialize() {
		String input = JOptionPane.showInputDialog(null, "Please enter string-value to be added:",
				"", JOptionPane.PLAIN_MESSAGE);
		if (input != null) {
			constValue = input;
		} else {
			JOptionPane.showMessageDialog(null,
					"No value was entered. Operator will use an empty string.", "",
					JOptionPane.INFORMATION_MESSAGE);
		}
	}

	/**
	 * Adds the constant value string to the given attribute.
	 * 
	 * @param mpoint
	 *            the moving point
	 * @return a new attribute of type string containing the concatenated
	 *         constant value and attribute value.
	 */
	public AttributeString operate(AttributeString attribute) {
		AttributeString result = new AttributeString();
		result.setValue(attribute.getValue() + constValue);
		return result;
	}

}
