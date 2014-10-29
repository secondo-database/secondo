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

import java.util.Random;

import javax.swing.JOptionPane;

import mmdb.data.attributes.standard.AttributeInt;

/**
 * This class represents the 'RANDOM' operator for ints.
 *
 * @author Alexander Castor
 */
public class OperatorRandomInt extends ExtensionOperator {

	/**
	 * The random interval's lower bound.
	 */
	private int lowerBound = 0;

	/**
	 * The random interval's upper bound.
	 */
	private int upperBound = 10;

	/*
	 * (non-Javadoc)
	 * 
	 * @see mmdb.operator.extension.ExtensionOperator#init()
	 */
	@Override
	public void initialize() {
		String input = JOptionPane.showInputDialog(null,
				"Please enter the random interval's lower and upper bound:.\n"
						+ "(format: 'int,int' / example: '10,20')", "", JOptionPane.PLAIN_MESSAGE);
		try {
			String[] tokens = input.split(",");
			lowerBound = Integer.parseInt(tokens[0]);
			upperBound = Integer.parseInt(tokens[1]);
		} catch (Throwable e) {
			JOptionPane.showMessageDialog(null,
					"Could not parse values. Operator will use default boundaries [0,10].", "",
					JOptionPane.INFORMATION_MESSAGE);
		}
	}

	/**
	 * Generates a random integer in the interval [lowerBound, upperBound].
	 * 
	 * @return a new attribute of type int containing the random integer.
	 */
	public AttributeInt operate() {
		AttributeInt result = new AttributeInt();
		Random random = new Random();
		int randomInt = random.nextInt((upperBound - lowerBound) + 1) + lowerBound;
		result.setValue(randomInt);
		return result;
	}

}
