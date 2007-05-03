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
 * This is a model class for a position of a chessgame, representing all 64 squares of a chess board and accordingly the chessman on each square. Furthermore this class offers methods to manipulate the position on the chess board. However, checking if this is a correct position on the chessboard is not done. 
 */
public class PositionData
{
	/**
	 * representation of the chessboard as a char array
	 */
	private char[][] positions = new char[8][8];

	/**
	 * default constructor initialising an empty chess board
	 */
	public PositionData()
	{
		this.clear();
	}

	/**
	 * this method sets the chessmen on the chessboard to their initial positions.
	 */
	public void startPositions()
	{
		this.clear();
		positions[0][0] = ChessToolKit.WHITE_ROOK;
	        positions[0][1] = ChessToolKit.WHITE_KNIGHT;
	        positions[0][2] = ChessToolKit.WHITE_BISHOP;
		positions[0][3] = ChessToolKit.WHITE_QUEEN;
		positions[0][4] = ChessToolKit.WHITE_KING;
		positions[0][5] = ChessToolKit.WHITE_BISHOP;
		positions[0][6] = ChessToolKit.WHITE_KNIGHT;
		positions[0][7] = ChessToolKit.WHITE_ROOK;
		for (int i=0; i< ChessToolKit.MAX_ROW ;i++)
		{
			positions[1][i] = ChessToolKit.WHITE_PAWN;
		}
		positions[7][0] = ChessToolKit.BLACK_ROOK;
	        positions[7][1] = ChessToolKit.BLACK_KNIGHT;
	        positions[7][2] = ChessToolKit.BLACK_BISHOP;
		positions[7][3] = ChessToolKit.BLACK_QUEEN;
		positions[7][4] = ChessToolKit.BLACK_KING;
		positions[7][5] = ChessToolKit.BLACK_BISHOP;
		positions[7][6] = ChessToolKit.BLACK_KNIGHT;
		positions[7][7] = ChessToolKit.BLACK_ROOK;
		for (int i=0; i< ChessToolKit.MAX_ROW ;i++)
		{
			positions[6][i] = ChessToolKit.BLACK_PAWN;
		}	
	}

	/**
	 * clears the chessboard after calling this the board is empty
	 */
	public void clear()
	{
		for (int i = 0; i< ChessToolKit.MAX_ROW; i++)
		{
			for (int j=0; j< ChessToolKit.MAX_ROW; j++)
			{
				positions[i][j]=ChessToolKit.NONE;
			}
		}	
	}

	/**
	 * changes the positions on the chessboard so that the move m is done. This method does not check if the move is possible in this position. 
	 */
	public void doMove(MoveData m)
	{
		if(m.isKingSideCastling() && m.getAgent() == ChessToolKit.WHITE_KING ) // first of all check if this is a castling move and if so change the position of the rook first
		{
			changePosition(0,'H',0,'F',ChessToolKit.WHITE_ROOK);
		}
		if(m.isKingSideCastling() && m.getAgent() == ChessToolKit.BLACK_KING)
		{
			changePosition(7,'H',7,'F',ChessToolKit.BLACK_ROOK);
		}
		if(m.isQueenSideCastling() && m.getAgent() == ChessToolKit.WHITE_KING)
		{
			changePosition(0,'A',0,'D',ChessToolKit.WHITE_ROOK);
		}
		if(m.isQueenSideCastling() && m.getAgent() == ChessToolKit.BLACK_KING)
		{
			changePosition(7,'A',7,'D',ChessToolKit.BLACK_ROOK);
		}
		if((m.getAgent() == ChessToolKit.WHITE_PAWN || m.getAgent() == ChessToolKit.BLACK_PAWN) && 
				(m.getCaptured() == ChessToolKit.WHITE_PAWN || m.getCaptured() == ChessToolKit.BLACK_PAWN) &&
				getAgentAt(m.getTargetRow(), m.getTargetFile()) == ChessToolKit.NONE) //check for capturing en passant
		{
			if (m.getAgent() == ChessToolKit.WHITE_PAWN)
			{
				changePosition(m.getTargetRow()-2,m.getTargetFile(),m.getTargetRow()-2,m.getTargetFile(),ChessToolKit.NONE);
			}
			else
			{
				changePosition(m.getTargetRow(),m.getTargetFile(),m.getTargetRow(),m.getTargetFile(),ChessToolKit.NONE);
			}	
		}
		changePosition(m.getStartRow()-1,m.getStartFile(),m.getTargetRow()-1,m.getTargetFile(),m.getAgent()); //do the actual move
		if(m.isPawnPromotion()) //if this is a pawn promotion move change the pawn to the new chessman
		{
		     	char piece = m.getPromotionPiecePgn();
			if(Character.toUpperCase(piece) == ChessToolKit.WHITE_QUEEN || Character.toUpperCase(piece) == ChessToolKit.WHITE_BISHOP || Character.toUpperCase(piece) == ChessToolKit.WHITE_ROOK || Character.toUpperCase(piece) == ChessToolKit.WHITE_KNIGHT)
			{
				if(m.isWhite())
					changePosition(m.getTargetRow()-1,m.getTargetFile(),m.getTargetRow()-1,m.getTargetFile(),Character.toUpperCase(piece));
				else
					changePosition(m.getTargetRow()-1,m.getTargetFile(),m.getTargetRow()-1,m.getTargetFile(),Character.toLowerCase(piece));
			}
		}
	}
	
	/**
	 * changes the position on the chess board so that the chessman 'agent' is moved from startRow/startFile to targetRow/targetFile. The position in startRow/startFile is empty afterwards. returns true if there was a chessman on the square targetRow/targetFile meaning it was captured false otherwise  
	 */
	private boolean changePosition(int startRow, char startFile, int targetRow, char targetFile, char agent ) 
	{
		boolean captured = false;
		this.positions[startRow][ChessToolKit.fileToNumber(startFile)]=ChessToolKit.NONE;
		if(this.positions[targetRow][ChessToolKit.fileToNumber(targetFile)]!= ChessToolKit.NONE);
			captured = true;
		this.positions[targetRow][ChessToolKit.fileToNumber(targetFile)] = agent;
		return captured;
	}

	/**
	 * changes the position at (row; file) to agent
	 */
	public void changePosition(int row, int file, char agent)
	{
		this.positions[row][file] = agent;
	}

	/**
	 * returns the agent at the specified square on the chessboard
	 */
	public char getAgentAt(int row, int file)
	{
		return positions[row][file];
	}

	/**
	 * returns the agent at the specified square on the chessboard
	 */
	public char getAgentAt(int row, char file)
	{
		return positions[row][ChessToolKit.fileToNumber(file)];
	}

	/**
	 * returns the positions array
	 */
	public char[][] getPositions()
	{
		return positions;
	}

	/**
	 * sets the positions array
	 */
	public void setPositions(char[][] positions)
	{
		this.positions = positions;
	}

	/**
	 * returns the string representation of a PositionData object
	 */
	public String toString()
	{
		StringBuffer out = new StringBuffer();
		for (int i = 0;i<ChessToolKit.MAX_ROW;i++)
		{
			for(int j=0; j<ChessToolKit.MAX_ROW;j++)
			{
				out.append(positions[i][j]);
				out.append(' ');
			}
			out.append("\n\r");
		}
		return out.toString();
		
	}

	/**
	 * copies the positions from newP. Needed if a PositionData object needs to be copied without having a reference.
	 */
	public void copyPositions(char[][] newP)
	{
		for (int i=0;i<ChessToolKit.MAX_ROW;i++)
		{
			for(int j=0;j<ChessToolKit.MAX_ROW;j++)
			{
				this.positions[i][j] = newP[i][j];
			}
		}
	}

	/**
	 * returns a string representation of the position in nested list format used by secondo
	 */
	public String toListString()
	{
		StringBuffer out = new StringBuffer();
		out.append('(');
		for (int i=0;i<ChessToolKit.MAX_ROW;i++)
		{
			for(int j=0;j<ChessToolKit.MAX_ROW;j++)
			{
				if(positions[i][j] !=ChessToolKit.NONE)
				{
					this.addSquare(out, i,j,positions[i][j]);
				}
			}
		}
		out.append(')');
		return out.toString();
	}

	/**
	 * supplying method for toListString. Adds a single square to the StringBuffer
	 */
	private void addSquare(StringBuffer out, int row, int file, char chessman)
	{
		out.append('(');
		out.append('\"');
		out.append(chessman);
		out.append('\"');
		out.append(' ');
		out.append('\"');
		out.append(Character.toLowerCase(ChessToolKit.fileForNumber(file+1)));
		out.append('\"');
		out.append(' ');
		out.append((row+1));
		out.append(')');	
	}
}
