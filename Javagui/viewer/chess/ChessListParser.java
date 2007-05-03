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

package viewer.chess;

import sj.lang.*;
import java.util.*;
/**
* this class provides method to check is a ListExpr contains a chessgame or a chessgame within a relation. Furthermore methods which parse a chessgame to a GameData object are provided
*/
public class ChessListParser
{
	/** method to check whether the ListExpr contains an attribute of type chessgame or is of type chessgame */
	public boolean containsChessGame(ListExpr list) 
	{
		return (isChessRelation(list) || isChessGame(list));	
	}
	
	/** method to check whether the ListExpr is a relation which contains a chessgame */
	public boolean isChessRelation(ListExpr list)
	{
		ListExpr attributeTypes, values;
		if (isRelation(list)) //check if list is a relation
		{
			attributeTypes = list.first().second().second(); // if list is relation the attribute types are this
			values = list.second(); // extract the values from the relation
			if (containsChessAttribute(attributeTypes, values))
				return true;
		}
		return false;
	}
	/** method to check whether the ListExpr is a relation**/
	private boolean isRelation(ListExpr list)
	{	
		if (list.listLength()!=2)
			return false; 
		ListExpr type = list.first();
		if (type.isAtom())
			return false;
		ListExpr mainType = type.first();
		if(type.listLength()!=2 ||
				!mainType.isAtom() ||
				mainType.atomType()!= ListExpr.SYMBOL_ATOM ||
				!(mainType.symbolValue().equals(ChessToolKit.RELATION_TYPE_STRING)))
			return false;
		ListExpr tupletype = type.second();
		ListExpr tupleAtom = tupletype.first();
		if(tupletype.listLength()!=2 ||
				!tupleAtom.isAtom() ||
				tupleAtom.atomType() != ListExpr.SYMBOL_ATOM ||
				!(tupleAtom.symbolValue().equals(ChessToolKit.TUPLE_TYPE_STRING)))
			return false;
		return true;
	}

	/**
	* checks whether the attribute types contain a type of chessgame
	*/
	private boolean containsChessAttribute(ListExpr attributeTypes, ListExpr values)
	{
		boolean valueExists = true;
		boolean chessFound = false;
		int position = 0;
		String[] types = new String[attributeTypes.listLength()];
		ListExpr attributeType;
		while (!attributeTypes.isEmpty()&&valueExists&&position<types.length)
		{
			attributeType = attributeTypes.first();
			if(attributeType.listLength()!=2)
				valueExists = false;
			else
			{
				types[position] = attributeType.second().writeListExprToString().trim();
				if(attributeType.second().writeListExprToString().trim().equals(ChessToolKit.CHESSGAME_TYPE_STRING))
					chessFound = true;
			}
			attributeTypes = attributeTypes.rest();
			position++;
		}
		if(! chessFound)
		{
			return false;
		}
		return isReallyChessGame(values, types);
	}

	/**
	* checks if the values of the relation which are of type chessgame really contain chessgames
	*/
	private boolean isReallyChessGame(ListExpr values, String[] types)
	{

		int position;
		boolean falseChessType = false;
		ListExpr tupleValue;
		while(!values.isEmpty() && !falseChessType)
		{
			tupleValue = values.first();
			position = 0;
			while(!tupleValue.isEmpty() && position < types.length)
			{
				if(types[position].equals(ChessToolKit.CHESSGAME_TYPE_STRING))
				{
					falseChessType = !isChessGame(tupleValue);
				}
				position++;
				tupleValue = tupleValue.rest();
			}
			values = values.rest();
		}
		return !falseChessType;
	}
	
	/**
	* method to check if moveList contains chessMoves 
	*/
	private boolean checkMoveList(ListExpr moveList)
	{
		ListExpr move;
		if (moveList.isAtom() )
			return false;
		while(!moveList.isEmpty()) //check move list
		{
			move = moveList.first();
			if (move.listLength() != 8 ||
					!move.first().isAtom()||
					move.first().atomType() != ListExpr.STRING_ATOM ||
					!move.second().isAtom() ||
					move.second().atomType() != ListExpr.STRING_ATOM ||
					!move.third().isAtom() ||
					move.third().atomType() != ListExpr.STRING_ATOM ||
					!move.fourth().isAtom() ||
					move.fourth().atomType() != ListExpr.INT_ATOM ||
					!move.fifth().isAtom() ||
					move.fifth().atomType() != ListExpr.STRING_ATOM ||
					!move.sixth().isAtom() ||
					move.sixth().atomType() != ListExpr.INT_ATOM ||
					!move.rest().sixth().isAtom() ||
					move.rest().sixth().atomType() != ListExpr.BOOL_ATOM ||
					!move.rest().rest().sixth().isAtom() ||
					move.rest().rest().sixth().atomType() != ListExpr.STRING_ATOM)
				return false;
			moveList = moveList.rest();
		}
		return true;
	}
	
	/**
	* this method checks if metaDataList is a correct ListExpr of a meta data list
	*/
	private boolean checkMetaDataList(ListExpr metaDataList)
	{
		ListExpr metaDataPair;
		if (metaDataList.isAtom())
			return false;
		while(!metaDataList.isEmpty()) //check metadata pairs
		{
			metaDataPair = metaDataList.first();
			if( metaDataPair.listLength() != 2 || 
					!metaDataPair.first().isAtom()|| 
					metaDataPair.first().atomType()!=ListExpr.STRING_ATOM || 
					!metaDataPair.second().isAtom() ||
					metaDataPair.second().atomType()!= ListExpr.STRING_ATOM)
				return false;
			metaDataList = metaDataList.rest();
		}
		return true;
	}
	
	
	/**
	* tests if list is of type chessgame. checks the list format for correctness and correct datatypes
	*/
	public boolean isChessGame(ListExpr list)
	{
		
		ListExpr chessType, chessList, metaDataList, moveList, metaDataPair, move;
		if(list.listLength() == 2 && !list.isAtom()) // if list is only a chessgame and not a chessgame in a relation this is true
		{
			chessType = list.first();
			if (!chessType.isAtom()|| chessType.atomType()!= ListExpr.SYMBOL_ATOM || !chessType.symbolValue().equals(ChessToolKit.CHESSGAME_TYPE_STRING))
				chessList = list.first();
			else
				chessList = list.second();
		}
		else
		{
			chessList = list.first();
		}
		if(chessList.listLength() != 2 || chessList.isAtom())
		{
			return false;
		}
		metaDataList = chessList.first();
		moveList = chessList.second();
		return this.checkMetaDataList(metaDataList) && this.checkMoveList(moveList);
	}
	
	/**
	* this method parses a ListExpr of a chessgame to a GameData object which is returned
	*/
	public GameData parseChessGame(ListExpr newGame) throws Exception
	{
		GameData game = new GameData();
		ListExpr metaDataList, moveList;
	
		if (newGame.first().writeListExprToString().trim().equals(ChessToolKit.CHESSGAME_TYPE_STRING))
		{
			metaDataList = newGame.second().first();
			moveList = newGame.second().second();	
		}
		else
		{
			metaDataList = newGame.first().first();
			moveList = newGame.first().second();
		}
		ListExpr metaDataPair, move;
		String key, value;
		char startFile, targetFile, agent, captured;
		int startRow, targetRow, no;
        	boolean check;	
		String pgn;
		while(!metaDataList.isEmpty())
		{
			metaDataPair = metaDataList.first();
			key = metaDataPair.first().stringValue();
			value = metaDataPair.second().stringValue();
			game.setTag(key,value);
			metaDataList = metaDataList.rest();
		}
		no = 1;
		while(!moveList.isEmpty())
		{
			move = moveList.first();
			game.addMove(validateMove(move,no));
			moveList = moveList.rest();
			no++;
		}
		return game;	
	}

	/** method to check if the ListExpr of a move contains correct values. This method does not only check the types but also the values (e.g. if file and row really describe a square on a chessboard)**/
	private MoveData validateMove(ListExpr move, int no) throws Exception 
	{
		MoveData m;
		String agent = move.first().stringValue().trim();
	        String captured = move.second().stringValue().trim();
		char startFile = move.third().stringValue().trim().charAt(0);	
		int startRow = move.fourth().intValue();
		char targetFile = move.fifth().stringValue().trim().charAt(0);
		int targetRow = move.sixth().intValue();
		boolean check = move.rest().sixth().boolValue();
		String pgn = move.rest().rest().sixth().stringValue().trim();
		try
		{
			if(isFile(startFile) && isFile(targetFile) && isRow(startRow) && isRow(targetRow) && isAgent(agent, false) && isAgent(agent, true))
			{
				m = new MoveData(no,ChessToolKit.getCharAgent(agent),ChessToolKit.getCharAgent(captured),startFile, startRow, targetFile, targetRow, check, pgn);
				return m;
			}
		}
		catch (Exception e)
		{
			throw new Exception(e.getMessage()+"\n\rYour input was: "+move.writeListExprToString());
		}
		return null ; //does not happen
	}

	/**checks if file is really a correct file on a chessboard, if not throws an Exception*/
	private boolean isFile(char file) throws Exception
	{
		if(!ChessToolKit.isFile(file))
			throw new Exception("error while parsing list:\n\r"+file+" is not a correct file on a chessboard.");
		return true;
	}

	/**checks if row is a row on a chessboard, if not throws an exception*/
	private boolean isRow(int row) throws Exception
	{
		if(!ChessToolKit.isRow(row))
			throw new Exception ("error while parsing list:\n\r"+row+" is not a correct row number on a chessboard. Allowed row numbers: 0-7");
		return true;
	}
	
	/**checks if agent is a correct chess agent, noneAllowed means that the 'none' value for a agent is allowed*/
	private boolean isAgent(String agent, boolean noneAllowed) throws Exception
	{
		if(!ChessToolKit.isAgent(agent, noneAllowed))
			throw new Exception("error while parsing list:\n\r"+agent+" is not a correct representation of a chess agent.");	
		return true;
	}

	
	/**
	* this method extracts the names of the attributeTypes to the attribNames array. Furthermore marks attributes of type chessgame with true in the boolean array. Returns the total number of chessgames in the attibuteTypes.
	*/
	private int saveAttributeTypes(ListExpr attributeTypes, boolean[] isGame, String[] attribNames)
	{
		int gameCount = 0;
		int pos = 0;
		ListExpr attributeType;
		boolean valueExists = true;
		while (!attributeTypes.isEmpty()&&valueExists)//save the attributetypes
		{
			attributeType = attributeTypes.first();
			if(attributeType.listLength()!=2)
				valueExists = false;
			else
			{
				attribNames[pos] = attributeType.first().writeListExprToString();
				if(attributeType.second().writeListExprToString().trim().equals(ChessToolKit.CHESSGAME_TYPE_STRING))
				{
					gameCount++;
					isGame[pos] = true;
				}
				else
					isGame[pos] = false;	
			}
			attributeTypes = attributeTypes.rest();
			pos ++;
		}
		return gameCount;
	}

	/**
	*  this methods extracts the chessgames and other values in the relation to the chessGames and relationValues ArrayList.
	*/
	private void saveValues(ListExpr values, ArrayList chessGames, ArrayList relationValues, boolean[] isGame, String[] attribNames) throws Exception
	{
		HashMap attributeVals;
		int gamePos;
		int pos;
		ListExpr tupleValue;
		while(!values.isEmpty()) //save values
		{
			tupleValue = values.first();
			pos = 0;
			gamePos = 0;
			attributeVals = new HashMap();
			while((pos < attribNames.length) && !tupleValue.isEmpty())
			{
				if(isGame[pos])
				{
					((ArrayList)chessGames.get(gamePos)).add(parseChessGame(tupleValue));
					gamePos++;
				}
				else
				{
					attributeVals.put(attribNames[pos],tupleValue.first().writeListExprToString().trim());
				}
				pos++;
				tupleValue = tupleValue.rest();
			}
			relationValues.add(attributeVals);
			values = values.rest();
		}
	}
	
	/**
	* parses a ListExpr of a relation to an ArrayList which contains the relationvalues an ArrayList of chessgames and an ArrayList which contains the names of the chessgame attributes. 
	*/
	public void parseRelation(ListExpr newRelation, ArrayList relationValues, ArrayList chessGames, ArrayList chessAttributeNames) throws Exception
	{
		boolean[] isGame;
		String[] attribNames;
		ListExpr values = newRelation.second();
		ListExpr attributeTypes = newRelation.first().second().second();
		
		isGame = new boolean[attributeTypes.listLength()];
		attribNames = new String[attributeTypes.listLength()];
		
		int gameCount = saveAttributeTypes(attributeTypes, isGame, attribNames);	
		
		ArrayList al;
		for (int i=0; i< gameCount; i++) //create arrayList chessGames where an ArrayList of chessgames is saved
		{
			al = new ArrayList();
			chessGames.add(al);
		}
		
		saveValues(values, chessGames, relationValues, isGame, attribNames);
		
		for (int i = 0; i< attribNames.length;i++) //create tabs if multiple attributes of type chessgame are in one relation
		{
			if (isGame[i])
				chessAttributeNames.add(attribNames[i]);
		}
	}	
}	
