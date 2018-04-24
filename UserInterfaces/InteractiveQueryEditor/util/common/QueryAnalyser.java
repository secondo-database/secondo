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
import util.domain.enums.BracketType;
import util.domain.enums.Delimiter;
import util.domain.enums.ErrorType;
import util.domain.enums.OperatorType;
import util.domain.enums.ParameterType;
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
	private static final Pattern ADVANCED_PATTERN = Pattern.compile("\".*?\"|'.*?'|>=|<=|[\\[\\]\\(\\),;=#<>]");//TODO +-/\\*

	private static final HashSet<String> BRACKETS = new HashSet<>(Arrays.asList("[", "(", ")", "]"));
	private static final HashSet<String> DELIMITER = new HashSet<>(Arrays.asList(",", ";"));
	private final HashMap<String, Operator> secondoOperators;

	/**
	 * This method is used for debugging purposes only.
	 * @param args
	 */
	public static void main(final String[] args) {
		final QueryAnalyser analyser = new QueryAnalyser("specs");
		try {
			analyser.analyseQuery("query Orte feed filter[.Ort=\"Test\"");
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
		analyseTokens(complexTokens, 0, complexTokens.length -1);
		return createTypeInformation(complexTokens);
	}

	private String normalizeQuery(final String query) {
		return query.replaceAll("\\{", " rename [").replaceAll("\\}", " ] ");
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
			} else if (BRACKETS.contains(tokens[i])) {
				tokenList.add(new ComplexToken(tokens[i], TokenType.BRACKET));
			} else if (DELIMITER.contains(tokens[i])) {
				tokenList.add(new ComplexToken(tokens[i], TokenType.DELIMITER));
			} else if (tokens[i].startsWith(".")) {
				tokenList.add(new ComplexToken(tokens[i], TokenType.PARAMETER_FUNCTION));
			} else {
				tokenList.add(new ComplexToken(tokens[i], TokenType.VALUE));
			}
		}
		return tokenList.toArray(new ComplexToken[tokenList.size()]);
	}

	/**
	 * This method checks the syntactical and semantical correctness of the given tokens
	 * @param complexTokens
	 */
	private void analyseTokens(final ComplexToken[] complexTokens, final int beginIndex, final int endIndex) {
		for (int i = beginIndex; i <= endIndex; i++) {
			final ComplexToken currentToken = complexTokens[i];
			if (!currentToken.isConsumedByOperator()) {//because of the recursive use of this method tokens within a bracket are already analysed and/or consumed
				if (currentToken.getType().equals(TokenType.OPERATOR)) {
					if(checkPrefixArguments(complexTokens, currentToken, i)) {//If everything is ok
						if(checkPostfixArguments(complexTokens, currentToken, i)) {//then check the postfix arguments
							consumePrefixArguments(complexTokens, currentToken, i);
							if (currentToken.getIndexOfLastAssociatedToken() != -1) {//skip the bracket of the operator
								i = currentToken.getIndexOfLastAssociatedToken();//arguments within the bracket have already been analysed
							}
						}
					}

					//If an error occured during analysis and if the error type is not ErrorType.UNFINISHED_BRACKET then abort analysis
					if (currentToken.getOccurredErrorType() != null && !currentToken.getOccurredErrorType().equals(ErrorType.UNFINISHED_BRACKET)) {
						break;
					}
				} else if (currentToken.getType().equals(TokenType.PARAMETER_FUNCTION)) {
					if (!checkParameterFunction(complexTokens, currentToken)) {
						break;
					}
				}
			}
		}
	}

	private boolean checkPrefixArguments(final ComplexToken[] tokens, final ComplexToken currentToken, final int currentTokenIndex) {
		final int numberOfPrefixArguments = currentToken.getOperator().getPrefixArguments().size();
		int foundArguments = 0;
		for (int i = currentTokenIndex - 1; i >= 0; i--) {
			final ComplexToken previousElement = tokens[i];
			if ((previousElement.getType().equals(TokenType.VALUE)
					|| previousElement.getType().equals(TokenType.OPERATOR)
					|| previousElement.getType().equals(TokenType.PARAMETER_FUNCTION))
					&& !previousElement.isConsumedByOperator()) {
				currentToken.addIndexOfPrecedingToken(0, i);
				foundArguments++;
			}
			else if (previousElement.getType().equals(TokenType.BRACKET)) {
				if (previousElement.getText().equals(BracketType.SQUARED.getOpeningBracket()) ||
						previousElement.getText().equals(BracketType.ROUND.getOpeningBracket())) {
					break;
				} else if (previousElement.getText().equals(BracketType.SQUARED.getClosingBracket()) ||
						previousElement.getText().equals(BracketType.ROUND.getClosingBracket())) {
					i = previousElement.getIndexesOfPrecedingTokens().get(0);
					continue;
				}
			}

			if (foundArguments == numberOfPrefixArguments) {
				return true;
			}
		}
		currentToken.setOccurredErrorType(ErrorType.MISSING_PREFIX_ARGUMENTS);
		currentToken.setErrorMessage(String.format("Missing prefix arguments for operator %s\nThe operator syntax is defined as: %s", currentToken.getText(), currentToken.getOperator().getPattern()));
		return false;
	}

	private boolean checkPostfixArguments(final ComplexToken[] tokens, final ComplexToken currentToken, final int currentTokenIndex) {
		if (currentToken.getOperator().getOperatorType().equals(OperatorType.INFIXOP)) {
			if (currentTokenIndex + 1 < tokens.length) {
				final ComplexToken nextElement = tokens[currentTokenIndex + 1];
				if (nextElement.getType().equals(TokenType.VALUE)
						|| nextElement.getType().equals(TokenType.OPERATOR)
						|| nextElement.getType().equals(TokenType.PARAMETER_FUNCTION)) {
					nextElement.setConsumedByOperator(true);
					currentToken.setIndexOfLastAssociatedToken(currentTokenIndex + 1);
					return true;
				} else {
					currentToken.setOccurredErrorType(ErrorType.MISSING_POSTFIX_ARGUMENTS);
					currentToken.setErrorMessage(String.format("Missing postfix argument for infix operator %s", currentToken.getText()));
					return false;
				}
			}
			return false;
		} else {
			if (currentToken.getOperator().getBracketType().equals(BracketType.NONE) ||
					(currentToken.getOperator().getBracketType().equals(BracketType.ROUND) &&
							currentToken.getOperator().getPostfixArguments().size() == 0)) {
				//Operators with postfix arguments always use a bracket. Operators with no bracket do not have any postfix arguments.
				//Some operators with a round bracket use zero arguments. they get ignored too
				return true;
			}

			//First step for all other operators is a check of the bracket
			if (!analyseBracket(tokens, currentToken, currentTokenIndex)) {//If analyseBracket evaluates to true then continue (therefore the bracket must be completed)
				return false;
			}

			//Before we can count the amount of postfix arguments within the bracket
			//all tokens within the bracket need to get analysed. Therefore we
			//use a recursive call to analyse all tokens within the bracket
			final int nextTokenIndex = currentTokenIndex + 1;//The token after the operator (the opening bracket)
			final ComplexToken openingBracket = tokens[nextTokenIndex];
			final int startIndex = nextTokenIndex + 1;
			final int lastIndex = openingBracket.getIndexOfLastAssociatedToken() - 1;
			analyseTokens(tokens, startIndex, lastIndex);//recurive call for every bracket

			//when all the tokens within a bracket are analysed all unsused identifiers and operators get counted and assigned to the corresponding ParameterType
			final ArrayList<Delimiter> argumentDelimiters = currentToken.getOperator().getArgumentDelimiters();
			final ArrayList<ParameterType> parameterTypes = currentToken.getOperator().getPostfixArguments();

			//this algorithm assumes that argumentDelimiters.size() + 1 == parameterTypes.size()
			if (argumentDelimiters.size() > 0) {
				int foundDelimiters = 0;
				int lastFoundDelimiterIndex = startIndex;
				for (int j = startIndex; j <= lastIndex; j++) {
					final ComplexToken token = tokens[j];
					if (token.getType().equals(TokenType.DELIMITER) && token.getText().equals(argumentDelimiters.get(foundDelimiters).getText())) {
						if (!checkArguments(tokens, currentToken, parameterTypes.get(foundDelimiters), lastFoundDelimiterIndex, j - 1)) {
							return false;
						}
						lastFoundDelimiterIndex = j;
						foundDelimiters++;
					}
				}
				return checkArguments(tokens, currentToken, parameterTypes.get(foundDelimiters + 1), lastFoundDelimiterIndex, lastIndex);
			} else {
				return checkArguments(tokens, currentToken, parameterTypes.get(0), startIndex, lastIndex);
			}
		}
	}

	private boolean checkArguments(final ComplexToken[] tokens, final ComplexToken currentToken, final ParameterType parameterType, final int startIndex, final int lastIndex) {
		int numberOfFoundArgments = 0;
		for (int j = startIndex; j <= lastIndex; j++) {
			final ComplexToken nextElement = tokens[j];
			if ((nextElement.getType().equals(TokenType.VALUE)
					|| nextElement.getType().equals(TokenType.OPERATOR)) && !nextElement.isConsumedByOperator()) {
				nextElement.setConsumedByOperator(true);
				numberOfFoundArgments++;
			} else if (nextElement.getType().equals(TokenType.BRACKET)) {
				if (nextElement.getText().equals(BracketType.SQUARED.getOpeningBracket()) ||
						nextElement.getText().equals(BracketType.ROUND.getOpeningBracket())) {
					j = nextElement.getIndexOfLastAssociatedToken();
					continue;
				} else if (nextElement.getText().equals(BracketType.SQUARED.getClosingBracket()) ||
						nextElement.getText().equals(BracketType.ROUND.getClosingBracket())) {
					break;
				}
			}
		}

		if (parameterType.equals(ParameterType.WILDCARD)) {
			if (numberOfFoundArgments == 0) {
				currentToken.setOccurredErrorType(ErrorType.MISSING_POSTFIX_ARGUMENTS);
				currentToken.setErrorMessage(String.format("Missing postfix arguments for operator %s\nThe operator syntax is defined as: %s",
						currentToken.getText(), currentToken.getOperator().getPattern()));
				return false;
			} else if (numberOfFoundArgments > 1) {
				currentToken.setOccurredErrorType(ErrorType.TOO_MANY_POSTFIX_ARGUMENTS);
				currentToken.setErrorMessage(String.format("Too many postfix arguments for operator %s\nThe operator syntax is defined as: %s", currentToken.getText(),
						currentToken.getOperator().getPattern()));
				return false;
			}
		} else if (parameterType.equals(ParameterType.LIST)) {
			if (numberOfFoundArgments == 0) {
				currentToken.setOccurredErrorType(ErrorType.MISSING_POSTFIX_ARGUMENTS);
				currentToken.setErrorMessage(String.format("Missing postfix arguments for operator %s\nThe operator syntax is defined as: %s",
						currentToken.getText(), currentToken.getOperator().getPattern()));
				return false;
			}
		} else if (parameterType.equals(ParameterType.FUNCTION)) {
			//TODO
		} else if (parameterType.equals(ParameterType.FUNCTION_LIST)) {
			//TODO
		}
		return true;
	}

	/**
	 * This methode checks for a given token of type operator if the bracket is correct and completed
	 * @param tokens
	 * @param currentToken
	 * @param currentTokenIndex
	 * @return
	 */
	private boolean analyseBracket(final ComplexToken[] tokens, final ComplexToken currentToken, final int currentTokenIndex) {
		int openBracketCounter = 0;
		ComplexToken firstFollowingToken = null;
		if (currentTokenIndex + 1 < tokens.length) {
			firstFollowingToken = tokens[currentTokenIndex+1];
			if (firstFollowingToken.getType().equals(TokenType.BRACKET)) {
				if (currentToken.getOperator().getBracketType().equals(BracketType.ROUND) && firstFollowingToken.getText().equals(BracketType.ROUND.getOpeningBracket()) ||
						currentToken.getOperator().getBracketType().equals(BracketType.SQUARED) && firstFollowingToken.getText().equals(BracketType.SQUARED.getOpeningBracket())) {
					openBracketCounter++;
				} else {
					currentToken.setOccurredErrorType(ErrorType.WRONG_BRACKET);
					currentToken.setErrorMessage(String.format("The opening bracket does not match operator definition for operator %s\nThe operator syntax is defined as: %s",
							currentToken.getText(), currentToken.getOperator().getPattern()));
					return false;//wrong bracket after the operator
				}
			} else {
				currentToken.setOccurredErrorType(ErrorType.MISSING_BRACKET);
				currentToken.setErrorMessage(String.format("Missing opening bracket for operator %s", currentToken.getText()));
				return false;//missing bracket after the operator
			}
		} else {
			return false;//no bracket but still nothing typed after the operator
		}

		for (int j = currentTokenIndex+2; j < tokens.length; j++) {
			final ComplexToken nextToken = tokens[j];
			if (nextToken.getType().equals(TokenType.BRACKET)) {
				if (currentToken.getOperator().getBracketType().equals(BracketType.ROUND) && nextToken.getText().equals(BracketType.ROUND.getOpeningBracket()) ||
						currentToken.getOperator().getBracketType().equals(BracketType.SQUARED) && nextToken.getText().equals(BracketType.SQUARED.getOpeningBracket())) {
					openBracketCounter++;
				} else if (currentToken.getOperator().getBracketType().equals(BracketType.ROUND) && nextToken.getText().equals(BracketType.ROUND.getClosingBracket()) ||
						currentToken.getOperator().getBracketType().equals(BracketType.SQUARED) && nextToken.getText().equals(BracketType.SQUARED.getClosingBracket())) {
					openBracketCounter--;
				}
			} else if (nextToken.getType().equals(TokenType.PARAMETER_FUNCTION)) {
				nextToken.addIndexOfPrecedingToken(0, currentTokenIndex);//A parameter function within a bracket memorizes the index of the corresponding operator
			}

			if (openBracketCounter == 0) {
				currentToken.setIndexOfLastAssociatedToken(j);//the operator memorizes the index of the closing bracket
				firstFollowingToken.setIndexOfLastAssociatedToken(j);//the openening bracket memorizes the index of the closing bracket
				firstFollowingToken.addIndexOfPrecedingToken(0, currentTokenIndex);//and the index of the corresponding operator
				nextToken.addIndexOfPrecedingToken(0, currentTokenIndex+1);//the closing bracket memorizes the index of the opening bracket
				return true;//bracket complete and closed
			}
		}
		currentToken.setOccurredErrorType(ErrorType.UNFINISHED_BRACKET);
		return false;//open bracket
	}

	private void consumePrefixArguments(final ComplexToken[] tokens, final ComplexToken currentToken, final int currentTokenIndex) {
		final ArrayList<Integer> indexes = currentToken.getIndexesOfPrecedingTokens();
		for (final Integer index : indexes) {
			final ComplexToken previousElement = tokens[index.intValue()];
			previousElement.setConsumedByOperator(true);
		}
	}

	private boolean checkParameterFunction(final ComplexToken[] tokens, final ComplexToken currentToken) {
		final String tokenText = currentToken.getText();
		final int count = tokenText.length() - tokenText.replaceAll("^\\.*", "").length();

		if (currentToken.getIndexesOfPrecedingTokens().size() == 0) {//parameter function outside of a bracket
			currentToken.setOccurredErrorType(ErrorType.PARAMETER_FUNCTION_AT_ILLEGEAL_POSITION);
			currentToken.setErrorMessage(String.format("The parameter function %s isn't part of a bracket and therefore doesn't refer to an operator!", currentToken.getText()));
			return false;
		}

		final ComplexToken operatorToken = tokens[currentToken.getIndexesOfPrecedingTokens().get(0)];
		if (operatorToken.getOperator().getPrefixArguments().size() < count) {
			currentToken.setOccurredErrorType(ErrorType.PARAMETER_FUNCTION_REFERS_TO_ILLEGAL_PREFIX);
			currentToken.setErrorMessage(String.format("The parameter function %s uses too many periods.\nThe syntax of the operator %s is defined as: %s", currentToken.getText(), operatorToken.getText(), operatorToken.getOperator().getPattern()));
			return false;
		}

		return true;
	}

	private String createTypeInformation(final ComplexToken[] allTokens) throws Exception {
		final StringBuilder typeInfo = new StringBuilder();
		for (int i = 0; i < allTokens.length; i++) {
			final ComplexToken currentToken = allTokens[i];
			if (!currentToken.isConsumedByOperator()) {
				if (currentToken.getType().equals(TokenType.VALUE)) {
					final ListExpr typeExpr = SecondoFacade.query("query " + currentToken.getText() + " getTypeNL", false);
					if (typeExpr != null) {
						final StringReader reader = new StringReader(typeExpr.second().textValue());
						final NLParser parser = new NLParser(reader);
						typeInfo.append(((ListExpr)parser.parse().value).toString());
						typeInfo.append("\n");
					} else {
						typeInfo.append(SecondoFacade.getErrorMessage());
						typeInfo.append("\n");
					}
				} else if (currentToken.getType().equals(TokenType.OPERATOR)) {
					if (currentToken.getOccurredErrorType() != null) {
						if(currentToken.getOccurredErrorType().equals(ErrorType.UNFINISHED_BRACKET)) {
							continue;						} else {
							typeInfo.append("\n").append(currentToken.getErrorMessage()).append("\n");
							break;
						}
					} else if (currentToken.getOperator().getPostfixArguments().size() != 0 && currentToken.getIndexOfLastAssociatedToken() == -1) {
						break;//the user hasn't entered a bracket and the postfix arguments at all
					}

					//calculate the beginning of the corresponding arguments - an argument of an operator can be an operator too
					int startIndex = currentToken.getIndexesOfPrecedingTokens().get(0);
					ComplexToken tempToken = allTokens[startIndex];
					while(tempToken.getIndexesOfPrecedingTokens().size() != 0 && tempToken.getIndexesOfPrecedingTokens().get(0).intValue() < startIndex) {
						startIndex = tempToken.getIndexesOfPrecedingTokens().get(0).intValue();
						tempToken = allTokens[startIndex];
					}

					final StringBuilder opQuery = new StringBuilder();
					opQuery.append("query ");
					for (int j = startIndex; j < i; j++) {
						tempToken = allTokens[j];
						opQuery.append(tempToken.getText()).append(" ");
					}
					opQuery.append(currentToken.getText()).append(" ");
					for (int j = i+1; j <= currentToken.getIndexOfLastAssociatedToken(); j++) {
						tempToken = allTokens[j];
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
						if (currentToken.getErrorMessage() != null) {
							typeInfo.append(currentToken.getErrorMessage());
							typeInfo.append("\n");
						}
					}
				} else if (currentToken.getType().equals(TokenType.PARAMETER_FUNCTION)) {
					if (currentToken.getOccurredErrorType() != null) {
						typeInfo.append("\n").append(currentToken.getErrorMessage()).append("\n");
						break;
					}

					//TODO
				}
			}

			if (currentToken.getOccurredErrorType() != null) {
				break;
			}
		}
		return typeInfo.toString();
	}
}