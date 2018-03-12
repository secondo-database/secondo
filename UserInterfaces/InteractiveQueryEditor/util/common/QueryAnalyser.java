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

package util.common;

import java.io.IOException;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import sj.lang.ListExpr;
import sj.lang.JavaListExpr.NLParser;
import util.domain.ComplexToken;
import util.domain.Operator;
import util.domain.enums.ErrorType;
import util.domain.enums.OperatorType;
import util.domain.enums.ParanthesisType;
import util.domain.enums.TokenType;
import util.secondo.SecondoFacade;

/**
 * This class is responsible for the complete analysis of a query.
 * The {@link #analyseCommand(String)} method parses and analyses the query.
 * @author D.Merle
 */
public class QueryAnalyser {
	private static final Pattern WHITESPACE_PATTERN = Pattern.compile("\\S+");//Every white space character splits the string into another token
	//There several characters and even operators which can appear within a query but do not need to be separated by whitespaces
	private static final Pattern ADVANCED_PATTERN = Pattern.compile("\".*?\"|'.*?'|>=|<=|[\\[\\]\\(\\),;=#<>]");

	private static final HashSet<String> PARANTHESIS = new HashSet<>(Arrays.asList("[", "(", ")", "]"));
	private static final HashSet<String> DELIMITER = new HashSet<>(Arrays.asList(",", ";"));
	private final HashMap<String, Operator> secondoOperators;


	public static void main(final String[] args) {
		final QueryAnalyser analyser = new QueryAnalyser("specs");
		try {
			analyser.analyseCommand("query Orte feed filter[.Ort=\"Zwickau\"] consume");
		} catch (final Exception e) {
			e.printStackTrace();
		}
	}

	public QueryAnalyser(final String operatorSpecFilePath) {
		try {
			secondoOperators = OperatorSpecParser.parse(operatorSpecFilePath);
		} catch (final IOException e) {
			throw new IllegalArgumentException(String.format("The spec file %s couldn't be parsed", operatorSpecFilePath), e);
		}
	}

	public String analyseCommand(final String command) throws Exception {
		final String[] tokens = parseSimpleTokens(command);
		//		return "Token size: " + tokens.length + "\n" + "Token:\n" + StringUtilities.appendStrings(tokens);
		// used for debugging purpose. if you remove the comment and comment the following lines the analysers will show all recognized tokens
		final ArrayList<ComplexToken> tokenList = new ArrayList<>();
		analyseTokenType(tokenList, tokens);
		analyseTokens(tokenList);
		return createTypeInformation(tokenList);
	}

	public static String[] parseSimpleTokens(final String command) {
		final ArrayList<String> simpleStringTokens = new ArrayList<>();
		final Matcher tokenMatcher = WHITESPACE_PATTERN.matcher(command);
		while (tokenMatcher.find()) {
			final String splitByWhitespace = tokenMatcher.group(0);
			final Matcher advancedMatcher = ADVANCED_PATTERN.matcher(splitByWhitespace);
			int lastFoundEndIndex = 0;
			boolean foundDelimiters = advancedMatcher.find();
			while (foundDelimiters) {
				final String subString = splitByWhitespace.substring(lastFoundEndIndex, advancedMatcher.start(0));
				if (!subString.equals("")) {
					simpleStringTokens.add(subString);
				}
				lastFoundEndIndex = advancedMatcher.end(0);
				simpleStringTokens.add(advancedMatcher.group(0));

				foundDelimiters = advancedMatcher.find();
			}

			if (lastFoundEndIndex < splitByWhitespace.length()) {
				simpleStringTokens.add(splitByWhitespace.substring(lastFoundEndIndex, splitByWhitespace.length()));
			}
		}
		return simpleStringTokens.toArray(new String[simpleStringTokens.size()]);
	}

	private void analyseTokenType(final ArrayList<ComplexToken> tokenList, final String[] tokens) {
		for (int i = 1; i < tokens.length; i++) {//Ignore first token, because it is always "query" and it is not an operator of Secondo
			if (secondoOperators.containsKey(tokens[i])) {
				final ComplexToken token = new ComplexToken(tokens[i], TokenType.OPERATOR);
				token.setOperator(secondoOperators.get(tokens[i]));
				tokenList.add(token);
			} else if (PARANTHESIS.contains(tokens[i])) {
				tokenList.add(new ComplexToken(tokens[i], TokenType.PARANTHESIS));
			} else {
				//TODO Test auf gültigen Bezeichner
				tokenList.add(new ComplexToken(tokens[i], TokenType.IDENTIFIER));
			}
		}
	}

	private void analyseTokens(final ArrayList<ComplexToken> tokenList) {
		for (int i = 0; i < tokenList.size(); i++) {
			final ComplexToken token = tokenList.get(i);
			if (token.getType().equals(TokenType.OPERATOR)) {
				if (checkPostfixArguments(tokenList, token, i)) {
					checkPrefixArguments(tokenList, token, i);
				} else if (token.getOccurredErrorType() != null &&
						(token.getOccurredErrorType().equals(ErrorType.MISSING_PARANTHESIS)
								|| token.getOccurredErrorType().equals(ErrorType.WRONG_PARANTHESIS))) {
					break;
				}
			}
		}
	}

	private boolean checkPostfixArguments(final ArrayList<ComplexToken> tokenList, final ComplexToken token, final int i) {
		if (!token.getOperator().getParanthesisType().equals(ParanthesisType.NONE)) {
			if (!checkParanthesis(tokenList, token, i)) {
				return false;
			}
		}

		if (token.getOperator().getOperatorType().equals(OperatorType.INFIXOP)) {//Erstmal nur infix behandeln der rest wird morgen erledigt
			final int numberOfPostOpArguments = token.getOperator().getPostOpArguments().size();
			if (numberOfPostOpArguments > 0) {
				int numberOfFoundArgments = 0;
				for (int j = i+1; j < tokenList.size(); j++) {
					final ComplexToken nextElement = tokenList.get(j);
					if (nextElement.getType().equals(TokenType.IDENTIFIER)
							|| nextElement.getType().equals(TokenType.OPERATOR)) {
						nextElement.setConsumedByOperator(true);
						numberOfFoundArgments++;
					} else if (nextElement.getType().equals(TokenType.PARANTHESIS)) {
						if (nextElement.getText().equals("[") || nextElement.getText().equals("(")) {
							j = nextElement.getLastPostOpTokenIndex();
							continue;
						} else if (nextElement.getText().equals("]") || nextElement.getText().equals(")")) {
							break;
						}
					}

					if (numberOfFoundArgments == numberOfPostOpArguments) {
						return true;
					}
				}
			} else {
				return true;
			}
		} else {
			return true;
		}
		token.setOccurredErrorType(ErrorType.MISSING_POSTFIX_ARGUMENTS);
		token.setErrorMessage(String.format("Missing postfix arguments for operator %s", token.getText()));
		return false;
	}

	private boolean checkParanthesis(final ArrayList<ComplexToken> tokenList, final ComplexToken token, final int i) {
		int openParanthesisCounter = 0;
		ComplexToken firstToken = null;
		if (i + 1 < tokenList.size()) {
			firstToken = tokenList.get(i+1);
			if (firstToken.getType().equals(TokenType.PARANTHESIS)) {
				if (token.getOperator().getParanthesisType().equals(ParanthesisType.ROUND) && firstToken.getText().equals(ParanthesisType.ROUND.getOpeningParanthesis()) ||
						token.getOperator().getParanthesisType().equals(ParanthesisType.SQUARED) && firstToken.getText().equals(ParanthesisType.SQUARED.getOpeningParanthesis())) {
					openParanthesisCounter++;
				} else {
					token.setOccurredErrorType(ErrorType.WRONG_PARANTHESIS);
					token.setErrorMessage("Paranthesis does not match operator definition");
					return false;//wrong paranthesis after the operator
				}
			} else {
				token.setOccurredErrorType(ErrorType.MISSING_PARANTHESIS);
				token.setErrorMessage("Missing opening paranthesis");
				return false;//missing paranthesis after the operator
			}
		} else {
			return false;//no paranthesis but still nothing typed after the operator
		}

		for (int j = i+2; j < tokenList.size(); j++) {
			final ComplexToken nextToken = tokenList.get(j);
			if (nextToken.getType().equals(TokenType.PARANTHESIS)) {
				if (token.getOperator().getParanthesisType().equals(ParanthesisType.ROUND) && nextToken.getText().equals(ParanthesisType.ROUND.getOpeningParanthesis()) ||
						token.getOperator().getParanthesisType().equals(ParanthesisType.SQUARED) && nextToken.getText().equals(ParanthesisType.SQUARED.getOpeningParanthesis())) {
					openParanthesisCounter++;
				} else if (token.getOperator().getParanthesisType().equals(ParanthesisType.ROUND) && nextToken.getText().equals(ParanthesisType.ROUND.getClosingParanthesis()) ||
						token.getOperator().getParanthesisType().equals(ParanthesisType.SQUARED) && nextToken.getText().equals(ParanthesisType.SQUARED.getClosingParanthesis())) {
					openParanthesisCounter--;
				} else {
					throw new IllegalArgumentException("One token was identified to be a paranthesis but its value does not fit the specified brackets!");
				}
			}

			if (openParanthesisCounter == 0) {
				token.setLastPostOpTokenIndex(j);
				firstToken.setLastPostOpTokenIndex(j);
				nextToken.setFirstPreOpTokenIndex(i+1);
				return true;//paranthesis closed
			}
		}
		token.setOccurredErrorType(ErrorType.UNFINISHED_PARANTHESIS);
		return false;//open paranthesis
	}

	private boolean checkPrefixArguments(final ArrayList<ComplexToken> input, final ComplexToken token, final int opIndex) {
		final int numberOfPreOpArguments = token.getOperator().getPreOpArguments().size();
		int foundArguments = 0;
		for (int i = opIndex - 1; i >= 0; i--) {
			final ComplexToken previousElement = input.get(i);
			if ((previousElement.getType().equals(TokenType.IDENTIFIER) || previousElement.getType().equals(TokenType.OPERATOR))
					&& !previousElement.isConsumedByOperator()) {
				previousElement.setConsumedByOperator(true);
				token.setFirstPreOpTokenIndex(i);
				foundArguments++;
			}
			else if (previousElement.getType().equals(TokenType.PARANTHESIS)) {
				if (previousElement.getText().equals("[") || previousElement.getText().equals("(")) {
					break;
				} else if (previousElement.getText().equals("]") || previousElement.getText().equals(")")) {
					i = previousElement.getFirstPreOpTokenIndex();
					continue;
				}
			}

			if (foundArguments == numberOfPreOpArguments) {
				return true;
			}
		}
		token.setOccurredErrorType(ErrorType.MISSING_PREFIX_ARGUMENTS);
		token.setErrorMessage(String.format("Missing prefix arguments for operator %s", token.getText()));
		return false;
	}


	private String createTypeInformation(final ArrayList<ComplexToken> tokenList) throws Exception {
		final StringBuilder typeInfo = new StringBuilder();
		for (int i = 0; i < tokenList.size(); i++) {
			final ComplexToken token = tokenList.get(i);
			if (token.getType().equals(TokenType.IDENTIFIER) && !token.isConsumedByOperator()) {
				final ListExpr typeExpr = SecondoFacade.query("query " + token.getText() + " getTypeNL", false);
				if (typeExpr != null) {
					final StringReader reader = new StringReader(typeExpr.second().textValue());
					final NLParser parser = new NLParser(reader);
					typeInfo.append(((ListExpr)parser.parse().value).toString());
					typeInfo.append("\n");
				}
			} else if (token.getType().equals(TokenType.OPERATOR) && !token.isConsumedByOperator()) {
				if ((token.getOperator().getPreOpArguments().size() != 0 && token.getFirstPreOpTokenIndex() == -1) ||
						(token.getOperator().getPostOpArguments().size() != 0 && token.getLastPostOpTokenIndex() == -1)) {
					if (token.getOccurredErrorType() != null && token.getErrorMessage() != null) {
						typeInfo.append("\n").append(token.getErrorMessage()).append("\n");
					}
					continue;//operator paranthesis unfinished
				}

				int startIndex = token.getFirstPreOpTokenIndex();
				for (int j = i; j >= startIndex; j--) {
					final ComplexToken tempToken = tokenList.get(j);
					if (tempToken.getFirstPreOpTokenIndex() != -1 && tempToken.getFirstPreOpTokenIndex()<startIndex) {
						startIndex = tempToken.getFirstPreOpTokenIndex();
					}
				}

				final StringBuilder opQuery = new StringBuilder();
				opQuery.append("query ");
				for (int j = startIndex; j < i; j++) {
					final ComplexToken tempToken = tokenList.get(j);
					opQuery.append(tempToken.getText()).append(" ");
				}
				opQuery.append(token.getText()).append(" ");
				for (int j = i+1; j <= token.getLastPostOpTokenIndex(); j++) {
					final ComplexToken tempToken = tokenList.get(j);
					opQuery.append(tempToken.getText()).append(" ");
				}
				opQuery.append("getTypeNL");

				//				typeInfo.append("\nOpQuery:").append(opQuery.toString()).append("\n");

				final ListExpr typeExpr = SecondoFacade.query(opQuery.toString(), false);
				if (typeExpr != null) {
					final StringReader reader = new StringReader(typeExpr.second().textValue());
					final NLParser parser = new NLParser(reader);
					typeInfo.append(((ListExpr)parser.parse().value).toString());
					typeInfo.append("\n");
				} else {
					typeInfo.append(SecondoFacade.getErrorMessage());
					typeInfo.append("\n");
					if (token.getErrorMessage() != null) {
						typeInfo.append(token.getErrorMessage());
						typeInfo.append("\n");
					}
				}
			}
			else if (token.getType().equals(TokenType.PARANTHESIS) && !token.isConsumedByOperator()) {

			}

			if (token.getOccurredErrorType() != null) {
				break;
			}
		}
		return typeInfo.toString();
	}
}
