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

import util.common.OperatorSpecParser;
import util.domain.enums.BracketType;
import util.domain.enums.OperatorType;
import util.domain.enums.ParameterType;

/**
 * Represents an operator of secondo.
 * Used by the {@link OperatorSpecParser} to store the information from the operator definition file.
 * @author D.Merle
 */
public class Operator {
	private String name;
	private String alias;
	private OperatorType operatorType;
	private ArrayList<ParameterType> prefixArguments;
	private BracketType bracketType;
	private ArrayList<ParameterType> postfixArguments;
	private String pattern;
	private String implicitInformation;

	public String getName() {
		return name;
	}

	public void setName(final String name) {
		this.name = name;
	}

	public String getAlias() {
		return alias;
	}

	public void setAlias(final String alias) {
		this.alias = alias;
	}

	public OperatorType getOperatorType() {
		return operatorType;
	}

	public void setOperatorType(final OperatorType operatorType) {
		this.operatorType = operatorType;
	}

	public ArrayList<ParameterType> getPrefixArguments() {
		return prefixArguments;
	}

	public void setPrefixArguments(final ArrayList<ParameterType> prefixArguments) {
		this.prefixArguments = prefixArguments;
	}

	public BracketType getBracketType() {
		return bracketType;
	}

	public void setBracketType(final BracketType bracketType) {
		this.bracketType = bracketType;
	}

	public ArrayList<ParameterType> getPostfixArguments() {
		return postfixArguments;
	}

	public void setPostfixArguments(final ArrayList<ParameterType> postfixArguments) {
		this.postfixArguments = postfixArguments;
	}

	public String getPattern() {
		return pattern;
	}

	public void setPattern(final String pattern) {
		this.pattern = pattern;
	}

	public String getImplicitInformation() {
		return implicitInformation;
	}

	public void setImplicitInformation(final String implicitInformation) {
		this.implicitInformation = implicitInformation;
	}

	@Override
	public String toString() {
		final StringBuilder builder = new StringBuilder();
		builder.append("name:").append(getName()).append("\n");
		builder.append("alias:").append(getAlias()).append("\n");
		builder.append("operatorType:").append(getOperatorType()).append("\n");
		builder.append("pre:").append(getPrefixArguments() != null ? getPrefixArguments().size(): 0).append("\n");
		builder.append("bracket:").append(getBracketType()).append("\n");
		builder.append("post:").append(getPostfixArguments()!= null ? getPostfixArguments().size(): 0).append("\n");
		builder.append("pattern:").append(getPattern()).append("\n");
		builder.append("implicit:").append(getImplicitInformation()).append("\n");
		return builder.toString();
	}
}