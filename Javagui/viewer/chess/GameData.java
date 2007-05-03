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
import java.util.*;

/**
 * This class provides the data model of a chessgame.  
 * 
 * */
public class GameData
{
	/**
	 * this HashMap contains custom tags of the chess game. The seven standard tags are attributes of the GameData object
	 */
	private HashMap tags;

	/**
	 * contains the value of the event-tag
	 */
	private String event ="";

	/**
	 *contains the value of the site-tag
	 */
	private String site ="";

	/**
	 *conatins the value of the date-tag
	 */
	private String date ="";

	/**
	 *contains the value of the round-tag
	 */
	private String round="";

	/**
	 *contains the value of the white-tag
	 */
	private String white="";

	/**
	 *contains the value of the black-tag
	 */
	private String black="";

	/**
	 *contains the value of the result tag
	 */
	private String result="";

	/**
	 * contains the list of moves 
	 */
	private ArrayList moves;

	/**
	 * index number of the currently shown move in the viewer. 
	 */
	private int currentMoveIndex =-1;

	/**
	 * game number 
	 */
	private int no = -1;
	

	/**
	 * default constructor
	 */
	public GameData()
	{
		tags = new HashMap(5);
		moves = new ArrayList();
	}

	/**
	 * sets the number of the game 
	 */
	public void setNo(int i)
	{
		no = i;
	}

	/**
	 * returns the number of the game
	 */
	public int getNo()
	{
		return no;
	}

	/**
	 * returns the current move number
	 */
	public int getCurrentMoveIndex()
	{
		return currentMoveIndex;
	}

	/**
	 * setter for the current move index
	 */
	public void setCurrentMoveIndex(int i)
	{
		if (i < moves.size() && i>=0)
			currentMoveIndex = i;
	}

	/**
	 * gettter for the tags hashmap
	 */
	public HashMap getTags()
	{
		return tags;
	}

	/**
	 * adds a move to the move-list of the game
	 */
	public void addMove(MoveData move)
	{
		this.moves.add(move);
	}

	/**
	 * returns the move specified by moveNumber
	 */
	public MoveData getMove(int moveNumber)
	{
		return (MoveData)this.moves.get(moveNumber);
	}

	/**
	 * returns an array of moves
	 */
	public Object[] getMoves()
	{
		return this.moves.toArray();
	}

	/**
	 * returns the array of tag- keys
	 */
	public Object[] getKeys()
	{
		return this.tags.keySet().toArray();
	}

	/**
	 * returns the value of the event-tag
	 */
	public String getEvent()
	{
		return event;
	}

	/**
	 * returns the value of the site-tag
	 */
	public String getSite()
	{
		return site;
	}	

	/**
	 * returns the value of the date-tag
	 */
	public String getDate()
	{
		return date;
	}

	/**
	 * returns the value of the round-tag
	 */
	public String getRound()
	{
		return round;
	}

	/**
	 * returns the value of the white-tag
	 */
	public String getWhite()
	{
		return white;
	}

	/**
	 * returns the value of the black-tag
	 */
	public String getBlack()
	{
		return black;
	}

	/**
	 * returns the value of the result-tag
	 */
	public String getResult()
	{
		return result;
	}

	/**
	 * returns the value of the tag with the key 'key'
	 */
	public String getValue(String key)
	{
		key = key.toLowerCase().trim();
		String value =  (String)this.tags.get(key);
		if (value == null)
			if (key.equals(ChessToolKit.EVENT_KEY))
				return event;
			else
				if(key.equals(ChessToolKit.SITE_KEY))
					return site;
				else
					if(key.equals(ChessToolKit.DATE_KEY))
						return date;
					else
						if(key.equals(ChessToolKit.ROUND_KEY))
							return round;
						else
							if(key.equals(ChessToolKit.WHITE_KEY))
								return white;
							else
								if(key.equals(ChessToolKit.BLACK_KEY))
									return black;
								else
									if(key.equals(ChessToolKit.RESULT_KEY))
										return result;
		return value;
	}

	/**
	 * setter for the event-tag
	 */
	public void setEvent(String event)
	{
		this.event = event;
	}	

	/**
	 *setter for the site-tag
	 */
	public void setSite(String site)
	{
		this.site = site;
	}

	/**
	 * setter for the date-tag
	 */
	public void setDate(String date)
	{
		this.date = date;
	}

	/**
	 * setter for the dound-tag
	 */
	public void setRound(String round)
	{
		this.round = round;
	}

	/**
	 *setter for the white-tag
	 */
	public void setWhite(String white)
	{
		this.white = white;
	}

	/**
	 * setter for the black-tag
	 */
	public void setBlack(String black)
	{
		this.black = black;
	}

	/**
	 *setter for the result-tag
	 */
	public void setResult(String result)
	{
		this.result = result;
	}

	/**
	 *adds a new customized tag to the tags list
	 */
	public void addTag(String key, String value)
	{
		this.tags.put(key, value);
	}

	/**
	 * setter for the tag with the key 'key'
	 */
	public void setTag(String key, String newValue)
	{
		key = key.toLowerCase().trim();
		if (key.equals(ChessToolKit.EVENT_KEY))
			this.event = newValue;
		else
			if(key.equals(ChessToolKit.SITE_KEY))
				this.site = newValue;
			else
				if(key.equals(ChessToolKit.DATE_KEY))
					this.date = newValue;
				else
					if(key.equals(ChessToolKit.ROUND_KEY))
						this.round = newValue;
					else
						if(key.equals(ChessToolKit.WHITE_KEY))
							this.white = newValue;
						else
							if(key.equals(ChessToolKit.BLACK_KEY))
								this.black = newValue;
							else
								if(key.equals(ChessToolKit.RESULT_KEY))
									this.result = newValue;
								else
									this.tags.put(key, newValue);

	}

	/**
	 * returns the String representation of this GameData object
	 */
	public String toString()
	{
		return ChessToolKit.toString(this);
	}

}
