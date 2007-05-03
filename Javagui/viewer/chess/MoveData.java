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
/**
 * This class is a model class for a chess game move.
 */
public class MoveData
{
	/**
	 * number of the move
	 */	
	private int no;

	/**
	 * the chessman that makes the move and the chessman that is captured if one is captured in the move
	 */
	private char agent, captured;

	/**
	 * the file where the move starts and the file where the move ends
	 */
	private char startFile, targetFile;

	/**
	 * the row where the move starts and the row where the move ends
	 */
	private int startRow, targetRow;

	/**
	 * hints whether this is a checking move
	 */
	private boolean check;

	/**
	 * pgn-notation of the move
	 */
	private String pgn;

	/**
	 *  constructor of a move which sets all the fields. 
	 *  no is the index of the move in all the moves of chessgame
	 *  agent is the chessman that is set in the move
	 *  captured is the captured chessman of this move if there is any
	 *  startFile is the file of the chessgame in which the move starts
	 *  startRow is the row in which the move starts
	 *  targetFile is the file in which the move ends
	 *  targetRow is the row in which the move ends
	 *  if check is true this move is a checking move (the enemy's king is checked after this move)
	 *  pgn is the phn-notation of this move
	 */
	public MoveData(int no, char agent, char captured, char startFile, int startRow, char targetFile, int targetRow, boolean check, String pgn)
	{
		this.no = no;
		this.agent = agent;
		this.captured = captured;
		this.startFile = startFile;
		this.startRow = startRow;
		this.targetFile = targetFile;
		this.targetRow = targetRow;
		this.check = check;
		this.pgn = pgn;
	}

	/**
	 * creates an empty or initial move. After this faked move the chessboard shows the starting position
	 */
	public MoveData()
	{
		this.no = -1;
	}

	/**
	 * returns the pgn-notation of this move
	 */
	public String getPgn()
	{
		return this.pgn;
	}

	/**
	 * returns true if in this move the king is kingside castling, false otherwise
	 */
	public boolean isKingSideCastling()
	{
		return pgn.equals(ChessToolKit.KINGSIDE_CASTLING_PGN);
			
	}

	/**
	 * returns true if the king is queenside castling in this move, false otherwise
	 */
	public boolean isQueenSideCastling()
	{
		return pgn.equals(ChessToolKit.QUEENSIDE_CASTLING_PGN);
	}

	/**
	 * returns if this move is made by the white player, false if black player.
	 */
	public boolean isWhite()
	{
		return Character.isUpperCase(agent);
	}

	/**
	 * returns true if a pawn is promoted in this move, false otherwise
	 */
	public boolean isPawnPromotion()
	{
		return (pgn.indexOf('=')>-1);
	}

	/**
	 * if a pawn is promoted to another chessman in this move this returns the chessman, otherwise returns 'z' meaning there is no pawn promotion.
	 */
	public char getPromotionPiecePgn()
	{
		int piecePos = pgn.indexOf('=')+1;
		if(piecePos < pgn.length() && isPawnPromotion())
		{
			return pgn.charAt(piecePos);
		}
		return 'z';
	}

	/**
	 * sets the pgn-notation of this move
	 */
	public void setPgn(String pgn)
	{
		this.pgn = pgn;
	}

	/**
	 * returns the index number of this move
	 */
	public int getNo()
	{
		return no;
	}

	/**
	 * returns the row where the move starts
	 */
	public int getStartRow()
	{
		return startRow;
	}

	/**
	 * returns the row where the move ends
	 */
	public int getTargetRow()
	{
		return targetRow;
	}

	/**
	 * returns the chessman set in this move
	 */
	public char getAgent()
	{
		return agent;
	}

	/**
	 * returns the chessman captured in this move if there is any, ChessToolKit.NONE otherwise
	 */
	public char getCaptured()
	{
		return captured;
	}

	/**
	 * returns the file where the move starts
	 */
	public char getStartFile()
	{
		return startFile;
	}

	/**
	 * returns the file where the move ends
	 */
	public char getTargetFile()
	{
		return targetFile;
	}

	/**
	 * returns true if the enemy's king is checked after this move
	 */
	public boolean isCheck()
	{
		return check;
	}

	/**
	 * returns a String representation of this move
	 */
	public String toString()
	{
		return	ChessToolKit.toString(this);
	}

	
	



	
}
