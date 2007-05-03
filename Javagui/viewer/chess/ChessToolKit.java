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
* This class provides constants and static methods which are used by all classes
* in the chess package. 
*/
public class ChessToolKit
{
	/** constant for the minimum row number on a chessboard**/
	public static final int MIN_ROW = 1;
	
	/** constant for the maximum row number on a chessboard**/
	public static final int MAX_ROW = 8;
	
	/** char constant for the a marked file on a chessboard **/
	public static final char A = 'a';
	
	/** char constant for the b marked file on a chessboard **/
	public static final char B = 'b';
	
	/** char constant for the c marked file on a chessboard **/
	public static final char C = 'c';
	
	/** char constant for the d marked file on a chessboard **/
	public static final char D = 'd';
	
	/** char constant for the e marked file on a chessboard **/
	public static final char E = 'e';
	
	/** char constant for the f marked file on a chessboard **/
	public static final char F = 'f';
	
	/** char constant for the g marked file on a chessboard **/
	public static final char G = 'g';
	
	/** char constant for the h marked file on a chessboard **/
	public static final char H = 'h';
	
	/** char constant to mark a agent which does not exist**/
	public static final char FALSE_AGENT = 'e';
	
	/** key for the event - tag*/
	public static final String EVENT_KEY = "event";
	
	/** key for the site - tag*/
        public static final String SITE_KEY = "site";
	
	/** key for the date - tag*/
	public static final String DATE_KEY = "date";
	
	/** key for the round - tag*/
	public static final String ROUND_KEY = "round";
	
	/** key for the white - tag*/
	public static final String WHITE_KEY = "white";
	
	/** key for the black - tag*/
	public static final String BLACK_KEY = "black";
	
	/** key for the result - tag*/
	public static final String RESULT_KEY = "result";
	
	/** string representation of a relation type in a ListExpr **/
	public static final String RELATION_TYPE_STRING = "rel";
	
	/** string representation of the tuple type in a ListExpr **/
	public static final String TUPLE_TYPE_STRING = "tuple";
	
	/** string representation of the chessgame type in a ListExpr **/
	public static final String CHESSGAME_TYPE_STRING = "chessgame";
	
	/** directory which contains the image files of the chess viewer **/
	public static final String IMAGES_DIR = "images";
	
	/** path to the image directory **/
	public static final String PATH_TO_THIS_CLASS = "viewer"+System.getProperty("file.separator")+"chess";
	
	/** absoulute path to the images needed by this viewer **/
	public static final String IMAGES_PATH = System.getProperty("user.dir")+
					System.getProperty("file.separator")+
					PATH_TO_THIS_CLASS+System.getProperty("file.separator")+
					IMAGES_DIR+System.getProperty("file.separator");
	
	/** file name of the black bishop image**/				
	public static final String BBISHOP_FILENAME = "black.bishop.png";
	
	/** file name of the black rook image**/
	public static final String BROOK_FILENAME = "black.rook.png";
	
	/** file name of the black knight image**/
	public static final String BKNIGHT_FILENAME = "black.knight.png";
	
	/** file name of the black queen image**/
	public static final String BQUEEN_FILENAME = "black.queen.png";
	
	/** file name of the black king image**/
	public static final String BKING_FILENAME = "black.king.png";
	
	/** file name of the black pawn image**/
	public static final String BPAWN_FILENAME = "black.pawn.png";
	
	/** file name of a white bishop image**/
	public static final String WBISHOP_FILENAME = "white.bishop.png";
	
	/** file name of a white rook image**/
	public static final String WROOK_FILENAME = "white.rook.png";
	
	/** file name of a white knight image**/
	public static final String WKNIGHT_FILENAME = "white.knight.png";
	
	/** file name of a white king image**/
	public static final String WKING_FILENAME = "white.king.png";
	
	/** file name of a white pawn image**/
	public static final String WPAWN_FILENAME = "white.pawn.png";
	
	/** file name of a white queen image**/
	public static final String WQUEEN_FILENAME = "white.queen.png";
	
	/** pgn string for kingside castling **/
	public static final String KINGSIDE_CASTLING_PGN = "O-O";
	
	/** pgn string for queenside castling **/
	public static final String QUEENSIDE_CASTLING_PGN = "O-O-O";
	
	/** char representation of a white king **/
	public static final char WHITE_KING = 'K';
	
	/** char representation of a black king **/
	public static final char BLACK_KING = 'k';
	
	/** char representation of a white queen**/
	public static final char WHITE_QUEEN = 'Q';
	
	/** char representation of a black queen**/
	public static final char BLACK_QUEEN = 'q';
	
	/** char representation of a white bishop**/
	public static final char WHITE_BISHOP = 'B';
	
	/** char representation of a black bishop **/
	public static final char BLACK_BISHOP = 'b';
	
	/** char representation of a white knight **/
	public static final char WHITE_KNIGHT = 'N';
	
	/** char representation of a black knight **/
	public static final char BLACK_KNIGHT = 'n';
	
	/** char representation of a white rook **/
	public static final char WHITE_ROOK = 'R'; 
	
	/** char representation of a black rook **/
	public static final char BLACK_ROOK = 'r';
	
	/** char representation of a white pawn **/
	public static final char WHITE_PAWN = 'P';
	
	/** char representation of a black pawn **/
	public static final char BLACK_PAWN = 'p';
	
	/** char representation for a square where no chessman is on **/
	public static final char NONE = 'z';
	
	/**
	 * String representation of a Black King
	 */
	public static final String BLACK_KING_STRING = "black king";
	
	/**
	 * String representation of a Black Queen
	 */
	public static final String BLACK_QUEEN_STRING = "black queen";
	
	/**
	 * String representation of a Black Bishop
	 */
	public static final String BLACK_BISHOP_STRING = "black bishop";
	
	/**
	 * String representation of a Black Knight
	 */
	public static final String BLACK_KNIGHT_STRING = "black knight";
	
	/**
	 * String representation of a Black Rook
	 */
	public static final String BLACK_ROOK_STRING = "black rook";
	
	/**
	 * String representation of a Black Pawn
	 */
	public static final String BLACK_PAWN_STRING = "black pawn";
	
	/**
	 * String representation of a White King
	 */
	public static final String WHITE_KING_STRING =  "white king";
	
	/**
	 * String representation of a White Queen
	 */
	public static final String WHITE_QUEEN_STRING = "white queen";
	
	/**
	 * String representation of a White bishop
	 */
	public static final String WHITE_BISHOP_STRING = "white bishop";
	
	/**
	 * String representation of a White Knight
	 */
	public static final String WHITE_KNIGHT_STRING = "white knight";
	
	/**
	 * String representation of a White Rook
	 */
	public static final String WHITE_ROOK_STRING = "white rook";
	
	/**
	 * String representation of a white pawn
	 */
	public static final String WHITE_PAWN_STRING = "white pawn";
	
	/**
	 * String representation of a square where no chess is 
	 */
	public static final String NONE_STRING = "none";
	
	/** String representation of a white king in the list format of chessgame*/
	public static final String PGN_WHITE_KING = "King";
	
	/** String representation of a white queen in the list format of chessgame*/
	public static final String PGN_WHITE_QUEEN = "Queen";
	
	/** String representation of a white bishop in the list format of chessgame*/
	public static final String PGN_WHITE_BISHOP = "Bishop";
	
	/** String representation of a white knight in the list format of chessgame*/
	public static final String PGN_WHITE_KNIGHT = "Knight";
	
	/** String representation of a white rook in the list format of chessgame*/
	public static final String PGN_WHITE_ROOK = "Rook";
	
	/** String representation of a white pawn in the list format of chessgame*/
	public static final String PGN_WHITE_PAWN = "Pawn";
	
	/** String representation of a black king in the list format of chessgame*/
	public static final String PGN_BLACK_KING = "king"; 
	
	/** String representation of a black queen in the list format of chessgame*/
	public static final String PGN_BLACK_QUEEN = "queen";
	
	/** String representation of a black bsihop in the list format of chessgame*/
	public static final String PGN_BLACK_BISHOP = "bishop";
	
	/** String representation of a black knight in the list format of chessgame*/
	public static final String PGN_BLACK_KNIGHT = "knight";
	
	/** String representation of a black rook in the list format of chessgame*/
	public static final String PGN_BLACK_ROOK = "rook";
	
	/** String representation of a black pawn in the list format of chessgame*/
	public static final String PGN_BLACK_PAWN = "pawn";
	
	/** String representation of no chessman*/
	public static final String PGN_NONE = "none";
	 
	
	/**
	* this method returns the char representation for the chessman in the String parameter
	*/
	public static char getCharAgent(String agent)
	{
		if (agent.trim().equals(ChessToolKit.PGN_WHITE_KING))
			return ChessToolKit.WHITE_KING;
		if(agent.trim().equals(ChessToolKit.PGN_BLACK_KING))
			return ChessToolKit.BLACK_KING;
		if(agent.trim().equals(ChessToolKit.PGN_WHITE_QUEEN))
			return ChessToolKit.WHITE_QUEEN;
		if(agent.trim().equals(ChessToolKit.PGN_BLACK_QUEEN))
			return ChessToolKit.BLACK_QUEEN;
		if(agent.trim().equals(ChessToolKit.PGN_WHITE_BISHOP))
			return ChessToolKit.WHITE_BISHOP;
		if(agent.trim().equals(ChessToolKit.PGN_BLACK_BISHOP))
			return ChessToolKit.BLACK_BISHOP;
		if(agent.trim().equals(ChessToolKit.PGN_WHITE_KNIGHT))
			return ChessToolKit.WHITE_KNIGHT;
		if(agent.trim().equals(ChessToolKit.PGN_BLACK_KNIGHT))
			return ChessToolKit.BLACK_KNIGHT;
		if(agent.trim().equals(ChessToolKit.PGN_WHITE_ROOK))
			return ChessToolKit.WHITE_ROOK;
		if(agent.trim().equals(ChessToolKit.PGN_BLACK_ROOK))
			return ChessToolKit.BLACK_ROOK;
		if(agent.trim().equals(ChessToolKit.PGN_WHITE_PAWN))
			return ChessToolKit.WHITE_PAWN;
		if(agent.trim().equals(ChessToolKit.PGN_BLACK_PAWN))
			return ChessToolKit.BLACK_PAWN;
		if(agent.trim().toLowerCase().equals(PGN_NONE))
			return ChessToolKit.NONE;
		return ChessToolKit.FALSE_AGENT;	
	}
	
	/** this method checks if file is really a correct file on a chessboard**/
	public static boolean isFile(char file)
	{
		char c = Character.toLowerCase(file);
		if(c == ChessToolKit.A||
			c==ChessToolKit.B ||
	  		c==ChessToolKit.C ||
			c==ChessToolKit.D ||
	      		c==ChessToolKit.E ||
			c==ChessToolKit.F ||
			c==ChessToolKit.G ||
			c==ChessToolKit.H	
			)
			return true;
		return false;
	}

	/** checks if row is a correct row on a chessboard **/
	public static boolean isRow(int row)
	{
		if(row >=ChessToolKit.MIN_ROW && row <= ChessToolKit.MAX_ROW)
			return true;
		return false;
	}
	
	/**checks if agent is a correct chess agent, noneAllowed means that the 'none' value for a agent is allowed*/
	public static boolean isAgent(String agent, boolean noneAllowed)
	{
		char c = ChessToolKit.getCharAgent(agent);
			if(c==ChessToolKit.FALSE_AGENT)
				return false;
		if(c==ChessToolKit.NONE && !noneAllowed)
			return false;
		return true;
	}
	
	/** toString method for a GameData object**/
	public static String toString(GameData game)
	{
		StringBuffer out = new StringBuffer();
		if (game.getNo() != -1)
		{
			out.append(game.getNo());
			out.append('.');
			out.append(' ');
		}
		out.append(game.getEvent());
		out.append(": ");
		out.append(game.getWhite());
		out.append(" vs. ");
		out.append(game.getBlack());
		return out.toString();
	}
	
	/** returns the string representation of the chessman in the char parameter **/
	public static String stringForChar(char c)
	{
		switch (c)
		{
			case ChessToolKit.WHITE_KING:return ChessToolKit.WHITE_KING_STRING;
			case ChessToolKit.WHITE_QUEEN:return ChessToolKit.WHITE_QUEEN_STRING;				     
			case ChessToolKit.WHITE_BISHOP:return ChessToolKit.WHITE_BISHOP_STRING;
			case ChessToolKit.WHITE_KNIGHT:return ChessToolKit.WHITE_KNIGHT_STRING;
			case ChessToolKit.WHITE_ROOK:return ChessToolKit.WHITE_ROOK_STRING;
			case ChessToolKit.WHITE_PAWN:return ChessToolKit.WHITE_PAWN_STRING;
			case ChessToolKit.BLACK_KING:return ChessToolKit.BLACK_KING_STRING;
			case ChessToolKit.BLACK_QUEEN:return ChessToolKit.BLACK_QUEEN_STRING;
			case ChessToolKit.BLACK_BISHOP:return ChessToolKit.BLACK_BISHOP_STRING;
			case ChessToolKit.BLACK_KNIGHT:return ChessToolKit.BLACK_KNIGHT_STRING;
			case ChessToolKit.BLACK_ROOK:return ChessToolKit.BLACK_ROOK_STRING;
			case ChessToolKit.BLACK_PAWN:return ChessToolKit.BLACK_PAWN_STRING;
		}
		return ChessToolKit.NONE_STRING;
	}
	
	/** to String method for a MoveData object **/
	public static String toString(MoveData move)
	{
		if(move.getNo() != -1)
			{
				StringBuffer out = new StringBuffer();
				out.append(move.getNo());
				out.append(". ");
				out.append(ChessToolKit.stringForChar(move.getAgent()));
				out.append(": ");
				if(move.getCaptured()!=ChessToolKit.NONE)
				{
					out.append("captures ");
					out.append(ChessToolKit.stringForChar(move.getCaptured()));
					out.append(" on ");
					out.append(move.getTargetFile());
					out.append(move.getTargetRow());
				}
				else
				{
					if (move.isKingSideCastling())
					{
						out.append("Kingside Castling");
					}
					else
					{
						if(move.isQueenSideCastling())
							out.append("Queenside Castling");
						else
						{
							if(move.isPawnPromotion())
							{
								out.append("is promoted to ");
								char piece = move.getPromotionPiecePgn();
								if (move.isWhite())
									out.append(ChessToolKit.stringForChar(Character.toUpperCase(piece)));
								else
									out.append(ChessToolKit.stringForChar(Character.toLowerCase(piece)));
							}
							else
							{
								out.append(move.getStartFile());
								out.append(move.getStartRow());
								out.append(" --> ");
								out.append(move.getTargetFile());
								out.append(move.getTargetRow());
							}
						}
					}
				}
				if(move.isCheck())
				{
					out.append("(check)");
				}
				return out.toString();
			}
			return ("start of game");
	}
	
	/** returns the char represenation of the file denoted in the int parameter **/
	public static char fileForNumber(int file)
	{
		switch(file)
		{
			case 1: return ChessToolKit.A;
			case 2: return ChessToolKit.B;
			case 3: return ChessToolKit.C;
			case 4: return ChessToolKit.D;
			case 5: return ChessToolKit.E;
			case 6: return ChessToolKit.F;
			case 7: return ChessToolKit.G;
			case 8: return ChessToolKit.H;
		}
		return 'Z'; //does not happen!
	}
	
	/** returns the int value for the file denoted in the char parameter **/
	public static int fileToNumber(char file)
	{
		switch(Character.toLowerCase(file))
		{
			case ChessToolKit.A:return 0;
			case ChessToolKit.B:return 1;
			case ChessToolKit.C:return 2;
			case ChessToolKit.D:return 3;
			case ChessToolKit.E:return 4;
			case ChessToolKit.F:return 5;
			case ChessToolKit.G:return 6;
			case ChessToolKit.H:return 7;
		}
		return -1; //should not happen
	}

	/** converts the key string to the notation of the key string in the list expression of a chessgame. This is neccessary
	 * since the "getKey"-operation of a chessgame in secondo is case sensitive. In the Viewer all keys are lowercased but for queries we need a
	 * uppercased first letter at the moment. So this method converts "white" to "White"*/
	public static String convertKeyStringToListFormat(String key)
	{
		StringBuffer newKey = new StringBuffer();
		char[] chars = key.toCharArray();
		chars[0] = Character.toUpperCase(chars[0]);
		return new String(chars);
	}
}
