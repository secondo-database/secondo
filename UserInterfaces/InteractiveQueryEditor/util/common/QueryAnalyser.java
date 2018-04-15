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
 * The {@link #analyseQuery(String)} method parses and analyses the query.
 * @author D.Merle
 */
public class QueryAnalyser {
	//Finds sequences of non-whitespace characters. This way all whitespaces get removed
	private static final Pattern WHITESPACE_PATTERN = Pattern.compile("\\S+");
	//There several characters and even operators which can appear within a query but do not need to be separated by whitespaces
	private static final Pattern ADVANCED_PATTERN = Pattern.compile("\".*?\"|'.*?'|>=|<=|[\\[\\]\\(\\),;=#<>]");

	private static final HashSet<String> PARANTHESIS = new HashSet<>(Arrays.asList("[", "(", ")", "]"));
	private static final HashSet<String> DELIMITER = new HashSet<>(Arrays.asList(",", ";"));
	private final HashMap<String, Operator> secondoOperators;

	/**
	 * This method is used for debugging purposes only.
	 * @param args
	 */
	public static void main(final String[] args) {
		final QueryAnalyser analyser = new QueryAnalyser("specs");
		try {
			analyser.analyseQuery("query Orte feed filter[.Ort=\"Berlin\"] consume");
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

	public String analyseQuery(final String query) throws Exception {
		final String normalizedQuery = normalizeQuery(query);
		final String[] tokens = parseTokens(normalizedQuery);
		//		return "Token size: " + tokens.length + "\n" + "Token:\n" + StringUtilities.appendStrings(tokens);
		// used for debugging purpose. if you remove the comment and comment the following lines the analysers will show all recognized tokens
		final ComplexToken[] complexTokens = analyseTokenType(tokens);
		analyseTokens(complexTokens);
		return createTypeInformation(complexTokens);
	}

	private String normalizeQuery(final String query) {
		return query.replaceAll("(\\{)(.*)(\\})", " rename [$2] ");
	}

	private static String[] parseTokens(final String query) {
		final ArrayList<String> tokens = new ArrayList<>();
		final Matcher whitespaceMatcher = WHITESPACE_PATTERN.matcher(query);
		while (whitespaceMatcher.find()) {
			final String splitByWhitespace = whitespaceMatcher.group(0);
			final Matcher advancedMatcher = ADVANCED_PATTERN.matcher(splitByWhitespace);
			//This part of the algorithm searches for occurrence of the advanced pattern within the substring split by whitespaces
			int lastFoundEndIndex = 0;
			while (advancedMatcher.find()) {
				final String splitByAdvancedPattern = splitByWhitespace.substring(lastFoundEndIndex, advancedMatcher.start(0));
				if (!splitByAdvancedPattern.equals("")) {
					tokens.add(splitByAdvancedPattern);//Add the substring before the found group
				}
				lastFoundEndIndex = advancedMatcher.end(0);
				tokens.add(advancedMatcher.group(0));//Add the found group
			}

			if (lastFoundEndIndex < splitByWhitespace.length()) {
				tokens.add(splitByWhitespace.substring(lastFoundEndIndex)); //Add the leftover part of the substring
			}
		}
		return tokens.toArray(new String[tokens.size()]);
	}

	private ComplexToken[] analyseTokenType(final String[] tokens) {
		final ArrayList<ComplexToken> tokenList = new ArrayList<>();
		for (int i = 1; i < tokens.length; i++) {//Ignore first token, because it is always "query" and it is not an operator of Secondo
			if (secondoOperators.containsKey(tokens[i])) {
				final ComplexToken token = new ComplexToken(tokens[i], TokenType.OPERATOR);
				token.setOperator(secondoOperators.get(tokens[i]));
				tokenList.add(token);
			} else if (PARANTHESIS.contains(tokens[i])) {
				tokenList.add(new ComplexToken(tokens[i], TokenType.PARANTHESIS));
			} else if (DELIMITER.contains(tokens[i])) {
				tokenList.add(new ComplexToken(tokens[i], TokenType.DELIMITER));
			} else {
				tokenList.add(new ComplexToken(tokens[i], TokenType.IDENTIFIER));
			}
		}
		return tokenList.toArray(new ComplexToken[tokenList.size()]);
	}

	/**
	 * This method checks the syntactical and semantical correctness of the given tokens
	 * @param complexTokens
	 */
	private void analyseTokens(final ComplexToken[] complexTokens) {
		for (int i = 0; i < complexTokens.length; i++) {
			final ComplexToken currentToken = complexTokens[i];
			if (currentToken.getType().equals(TokenType.OPERATOR) && !currentToken.isConsumedByOperator()) {
				if(checkPostfixArguments(complexTokens, currentToken, i)) {//If everything is ok
					checkPrefixArguments(complexTokens, currentToken, i);//then check the prefix arguments
				}

				if (currentToken.getOccurredErrorType() != null) {//If an error occurred
					if (currentToken.getOccurredErrorType().equals(ErrorType.MISSING_PARANTHESIS)//If these two errors occur the analysis
							|| currentToken.getOccurredErrorType().equals(ErrorType.WRONG_PARANTHESIS)
							|| currentToken.getOccurredErrorType().equals(ErrorType.MISSING_POSTFIX_ARGUMENTS)
							|| currentToken.getOccurredErrorType().equals(ErrorType.TOO_MANY_POSTFIX_ARGUMENTS)) {//should abort
						break;
					}//else continue
				}
			}
		}
	}

	private boolean checkPostfixArguments(final ComplexToken[] tokens, final ComplexToken currentToken, final int currentTokenIndex) {
		if (currentToken.getOperator().getOperatorType().equals(OperatorType.INFIXOP)) {
			if (currentTokenIndex + 1 < tokens.length) {
				final ComplexToken nextElement = tokens[currentTokenIndex + 1];
				if (nextElement.getType().equals(TokenType.IDENTIFIER)
						|| nextElement.getType().equals(TokenType.OPERATOR)) {
					nextElement.setConsumedByOperator(true);
					return true;
				} else {
					currentToken.setOccurredErrorType(ErrorType.MISSING_POSTFIX_ARGUMENTS);
					currentToken.setErrorMessage(String.format("Missing postfix argument for infix operator %s", currentToken.getText()));
					return false;
				}
			}
			return true;
		} else {
			if (currentToken.getOperator().getParanthesisType().equals(ParanthesisType.NONE) ||
					(currentToken.getOperator().getParanthesisType().equals(ParanthesisType.ROUND) &&
							currentToken.getOperator().getPostfixArguments().size() == 0)) {
				//Operators with postfix arguments always use a paranthesis. Operators with no paranthesis do not have any postfix arguments.
				//Some operators with a round paranthesis use zero arguments. they get ignored too
				return true;
			}

			//First step for all other operators is a check of the paranthesis
			if (!analyseParanthesis(tokens, currentToken, currentTokenIndex)) {//If checkParanthesis evaluates to true then continue
				return false;
			}

			//Before we can count the amount of postfix arguments within the paranthesis
			//all tokens within the paranthesis need to get analysed. Therefore we
			//use a recursive call to analyse all tokens within the paranthesis
			final int next = currentTokenIndex + 1;//The token after the operator (the paranthesis)
			final ComplexToken openingParanthesis = tokens[next];
			final int numberOfSubTokens = openingParanthesis.getLastCorrespondingTokenIndex() - next -1;
			if (numberOfSubTokens > 0) {
				final ComplexToken[] subTokens = new ComplexToken[numberOfSubTokens];
				for (int i = 0; i < subTokens.length; i++) {
					subTokens[i] = tokens[next + 1 + i];
				}

				analyseTokens(subTokens);//recurive call for every paranthesis
			}

			//when all the tokens within a paranthesis are analysed we do count all unsused identifiers and operators
			final int numberOfPostFixArguments = currentToken.getOperator().getPostfixArguments().size();
			int numberOfFoundArgments = 0;
			for (int j = currentTokenIndex + 2; j < tokens.length; j++) {
				final ComplexToken nextElement = tokens[j];
				if ((nextElement.getType().equals(TokenType.IDENTIFIER)
						|| nextElement.getType().equals(TokenType.OPERATOR)) && !nextElement.isConsumedByOperator()) {
					nextElement.setConsumedByOperator(true);
					numberOfFoundArgments++;
				} else if (nextElement.getType().equals(TokenType.PARANTHESIS)) {
					if (nextElement.getText().equals("[") || nextElement.getText().equals("(")) {
						j = nextElement.getLastCorrespondingTokenIndex();
						continue;
					} else if (nextElement.getText().equals("]") || nextElement.getText().equals(")")) {
						break;
					}
				}
			}

			if (numberOfFoundArgments == numberOfPostFixArguments) {
				return true;
			} else if (numberOfFoundArgments < numberOfPostFixArguments) {
				currentToken.setOccurredErrorType(ErrorType.MISSING_POSTFIX_ARGUMENTS);
				currentToken.setErrorMessage(String.format("Missing postfix arguments for operator %s\nThe operator syntax is defined as:%s", currentToken.getText(), currentToken.getOperator().getPattern()));
				return false;
			} else {
				currentToken.setOccurredErrorType(ErrorType.TOO_MANY_POSTFIX_ARGUMENTS);
				currentToken.setErrorMessage(String.format("Too many postfix arguments for operator %s\nThe operator syntax is defined as:%s", currentToken.getText(), currentToken.getOperator().getPattern()));
				return false;
			}
		}
	}

	/**
	 * This methode checks for a given token of type operator if the paranthesis is correct and completed
	 * @param tokens
	 * @param currentToken
	 * @param currentTokenIndex
	 * @return
	 */
	private boolean analyseParanthesis(final ComplexToken[] tokens, final ComplexToken currentToken, final int currentTokenIndex) {
		int openParanthesisCounter = 0;
		ComplexToken firstFollowingToken = null;
		if (currentTokenIndex + 1 < tokens.length) {
			firstFollowingToken = tokens[currentTokenIndex+1];
			if (firstFollowingToken.getType().equals(TokenType.PARANTHESIS)) {
				if (currentToken.getOperator().getParanthesisType().equals(ParanthesisType.ROUND) && firstFollowingToken.getText().equals(ParanthesisType.ROUND.getOpeningParanthesis()) ||
						currentToken.getOperator().getParanthesisType().equals(ParanthesisType.SQUARED) && firstFollowingToken.getText().equals(ParanthesisType.SQUARED.getOpeningParanthesis())) {
					openParanthesisCounter++;
				} else {
					currentToken.setOccurredErrorType(ErrorType.WRONG_PARANTHESIS);
					currentToken.setErrorMessage(String.format("Paranthesis does not match operator definition for operator %s\nThe operator syntax is defined as:%s", currentToken.getText(), currentToken.getOperator().getPattern()));
					return false;//wrong paranthesis after the operator
				}
			} else {
				currentToken.setOccurredErrorType(ErrorType.MISSING_PARANTHESIS);
				currentToken.setErrorMessage(String.format("Missing opening paranthesis for operator %s", currentToken.getText()));
				return false;//missing paranthesis after the operator
			}
		} else {
			return false;//no paranthesis but still nothing typed after the operator
		}

		for (int j = currentTokenIndex+2; j < tokens.length; j++) {
			final ComplexToken nextToken = tokens[j];
			if (nextToken.getType().equals(TokenType.PARANTHESIS)) {
				if (currentToken.getOperator().getParanthesisType().equals(ParanthesisType.ROUND) && nextToken.getText().equals(ParanthesisType.ROUND.getOpeningParanthesis()) ||
						currentToken.getOperator().getParanthesisType().equals(ParanthesisType.SQUARED) && nextToken.getText().equals(ParanthesisType.SQUARED.getOpeningParanthesis())) {
					openParanthesisCounter++;
				} else if (currentToken.getOperator().getParanthesisType().equals(ParanthesisType.ROUND) && nextToken.getText().equals(ParanthesisType.ROUND.getClosingParanthesis()) ||
						currentToken.getOperator().getParanthesisType().equals(ParanthesisType.SQUARED) && nextToken.getText().equals(ParanthesisType.SQUARED.getClosingParanthesis())) {
					openParanthesisCounter--;
				} else {
					throw new IllegalArgumentException("A token was identified to be a paranthesis but it does not fit the operator definition!");
				}
			}

			if (openParanthesisCounter == 0) {
				currentToken.setLastCorrespondingTokenIndex(j);//the operator
				firstFollowingToken.setLastCorrespondingTokenIndex(j);//the openening paranthesis
				nextToken.setFirstCorrespondingTokenIndex(currentTokenIndex+1);//the closing paranthesis
				return true;//paranthesis complete and closed
			}
		}
		currentToken.setOccurredErrorType(ErrorType.UNFINISHED_PARANTHESIS);
		return false;//open paranthesis
	}

	private boolean checkPrefixArguments(final ComplexToken[] tokens, final ComplexToken currentToken, final int currentTokenIndex) {
		final int numberOfPreOpArguments = currentToken.getOperator().getPrefixArguments().size();
		int foundArguments = 0;
		for (int i = currentTokenIndex - 1; i >= 0; i--) {
			final ComplexToken previousElement = tokens[i];
			if ((previousElement.getType().equals(TokenType.IDENTIFIER) || previousElement.getType().equals(TokenType.OPERATOR))
					&& !previousElement.isConsumedByOperator()) {
				previousElement.setConsumedByOperator(true);
				currentToken.setFirstCorrespondingTokenIndex(i);
				foundArguments++;
			}
			else if (previousElement.getType().equals(TokenType.PARANTHESIS)) {
				if (previousElement.getText().equals("[") || previousElement.getText().equals("(")) {
					break;
				} else if (previousElement.getText().equals("]") || previousElement.getText().equals(")")) {
					i = previousElement.getFirstCorrespondingTokenIndex();
					continue;
				}
			}

			if (foundArguments == numberOfPreOpArguments) {
				return true;
			}
		}
		currentToken.setOccurredErrorType(ErrorType.MISSING_PREFIX_ARGUMENTS);
		currentToken.setErrorMessage(String.format("Missing prefix arguments for operator %s\nThe operator syntax is defined as:%s", currentToken.getText(), currentToken.getOperator().getPattern()));
		return false;
	}


	private String createTypeInformation(final ComplexToken[] complexTokens) throws Exception {
		final StringBuilder typeInfo = new StringBuilder();
		for (int i = 0; i < complexTokens.length; i++) {
			final ComplexToken token = complexTokens[i];
			if (token.getType().equals(TokenType.IDENTIFIER) && !token.isConsumedByOperator()) {
				final ListExpr typeExpr = SecondoFacade.query("query " + token.getText() + " getTypeNL", false);
				if (typeExpr != null) {
					final StringReader reader = new StringReader(typeExpr.second().textValue());
					final NLParser parser = new NLParser(reader);
					typeInfo.append(((ListExpr)parser.parse().value).toString());
					typeInfo.append("\n");
				}
			} else if (token.getType().equals(TokenType.OPERATOR) && !token.isConsumedByOperator()) {
				if ((token.getOperator().getPrefixArguments().size() != 0 && token.getFirstCorrespondingTokenIndex() == -1) ||
						(token.getOperator().getPostfixArguments().size() != 0 && token.getLastCorrespondingTokenIndex() == -1)) {
					if (token.getOccurredErrorType() != null && token.getErrorMessage() != null) {
						typeInfo.append("\n").append(token.getErrorMessage()).append("\n");
					}
					continue;//operator paranthesis unfinished
				}

				int startIndex = token.getFirstCorrespondingTokenIndex();
				for (int j = i; j >= startIndex; j--) {
					final ComplexToken tempToken = complexTokens[j];
					if (tempToken.getFirstCorrespondingTokenIndex() != -1 && tempToken.getFirstCorrespondingTokenIndex()<startIndex) {
						startIndex = tempToken.getFirstCorrespondingTokenIndex();
					}
				}

				final StringBuilder opQuery = new StringBuilder();
				opQuery.append("query ");
				for (int j = startIndex; j < i; j++) {
					final ComplexToken tempToken = complexTokens[j];
					opQuery.append(tempToken.getText()).append(" ");
				}
				opQuery.append(token.getText()).append(" ");
				for (int j = i+1; j <= token.getLastCorrespondingTokenIndex(); j++) {
					final ComplexToken tempToken = complexTokens[j];
					opQuery.append(tempToken.getText()).append(" ");
				}
				opQuery.append("getTypeNL");

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

			if (token.getOccurredErrorType() != null) {
				break;
			}
		}
		return typeInfo.toString();
	}
}
