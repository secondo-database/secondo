//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

package util.domain;

import java.util.ArrayList;

import util.domain.enums.ErrorType;
import util.domain.enums.TokenType;

/**
 * Class to represent tokens of a query.<br>
 * All the relevant information about a token needed for the analysis is stored in an object of this class
 * @author D.Merle
 */
public class ComplexToken {
	private String text;
	private TokenType type;
	private boolean consumedByOperator;
	private ErrorType occurredErrorType;
	private String errorMessage;

	private Operator operator;
	private final ArrayList<Integer> indexesOfPrecedingTokens;//can be arguments or an opening bracket
	private int indexOfLastAssociatedToken;//used to memorize the last argument or the closing bracket of an operator

	public ComplexToken(final String text, final TokenType type) {
		this.text = text;
		this.type = type;
		indexesOfPrecedingTokens = new ArrayList<>();
		indexOfLastAssociatedToken = -1;//can be an argument or the corresponding closing bracket
	}

	public String getText() {
		return text;
	}

	public void setText(final String text) {
		this.text = text;
	}

	public TokenType getType() {
		return type;
	}

	public void setType(final TokenType type) {
		this.type = type;
	}

	public boolean isConsumedByOperator() {
		return consumedByOperator;
	}

	public void setConsumedByOperator(final boolean consumedByOperator) {
		this.consumedByOperator = consumedByOperator;
	}

	public ErrorType getOccurredErrorType() {
		return occurredErrorType;
	}

	public void setOccurredErrorType(final ErrorType occurredErrorType) {
		this.occurredErrorType = occurredErrorType;
	}

	public String getErrorMessage() {
		return errorMessage;
	}

	public void setErrorMessage(final String errorMessage) {
		this.errorMessage = errorMessage;
	}

	public Operator getOperator() {
		return operator;
	}

	public void setOperator(final Operator operator) {
		this.operator = operator;
	}

	public void addIndexOfPrecedingToken (final int position, final int value) {
		indexesOfPrecedingTokens.add(position, Integer.valueOf(value));
	}

	public ArrayList<Integer> getIndexesOfPrecedingTokens() {
		return indexesOfPrecedingTokens;
	}

	public int getIndexOfLastAssociatedToken() {
		return indexOfLastAssociatedToken;
	}

	public void setIndexOfLastAssociatedToken(final int index) {
		this.indexOfLastAssociatedToken = index;
	}
}