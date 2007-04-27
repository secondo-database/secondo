package viewer.chess;

import gui.MainWindow;

import java.awt.Color;

import javax.swing.BorderFactory;
import javax.swing.JComponent;
import javax.swing.JOptionPane;
import javax.swing.border.Border;

import sj.lang.IntByReference;
import sj.lang.ListExpr;
import sj.lang.UpdateInterface;
import tools.Reporter;



public abstract class ChessObject extends JComponent implements ChessInterface{
	ListExpr val;
	boolean editModus;
	
	public static String[] selection ={" none "," white pawn "," white bishop "," white knight "," white rook "," white queen "," white king ",
		" black pawn "," black bishop "," black knight "," black rook "," black queen "," black king "};

	public static Border compound5 = BorderFactory.createCompoundBorder(
			BorderFactory.createRaisedBevelBorder() , 
			BorderFactory.createEmptyBorder(5, 5, 5, 5));
	public static Border compound3 = BorderFactory.createCompoundBorder(
			BorderFactory.createRaisedBevelBorder() , 
			BorderFactory.createEmptyBorder(3, 3, 3, 3));
	public static Border compound2 = BorderFactory.createCompoundBorder(
			BorderFactory.createRaisedBevelBorder() , 
			BorderFactory.createEmptyBorder(2, 2, 2, 2));
	public static Border linecompound5 = BorderFactory.createCompoundBorder(
			BorderFactory.createLineBorder(Color.black),
			BorderFactory.createEmptyBorder(5, 5, 5, 5));
	public static Border linecompound3 = BorderFactory.createCompoundBorder(
			BorderFactory.createLineBorder(Color.black), 
			BorderFactory.createEmptyBorder(3, 3, 3, 3));
	public static Border linecompound2 = BorderFactory.createCompoundBorder(
			BorderFactory.createLineBorder(Color.black),
			BorderFactory.createEmptyBorder(2, 2, 2, 2));
	/*
	 * returns true if obejct is in edit-modus false otherwise 
	 */
	public boolean getEditModus() {
		return editModus;
	}
	/*
	 * parses internal figurenumber to the char for ListExpr 
	 */
	static public char parseToChar(int i) {
		switch (i) {
		// sorted after frequency of use
			case 0: return '-';
			case 1: return 'P';
			case 7: return 'p';
			case 3: return 'N';
			case 9: return 'n';
			case 5: return 'Q';
			case 11: return 'q';
			case 2: return 'B';
			case 8: return 'b';
			case 4: return 'R';
			case 10: return 'r';
			case 6: return 'K';
			case 12: return 'k';
			default: return '-';
		}
	}
	
	/*
	 * parses the ListExpr figurechar to a number for internal use 
	 */
	static public int parseFigureChar(char c) {
		switch (c) {
		// sorted after frequency of use
			case '-': return 0;
			case 'P': return 1;
			case 'p': return 7;
			case 'N': return 3;
			case 'n': return 9;
			case 'Q': return 5;
			case 'q': return 11;
			case 'B': return 2;
			case 'b': return 8;
			case 'R': return 4;
			case 'r': return 10;
			case 'K': return 6;
			case 'k': return 12;
			default: return 0;
		}
	}
	
	/**
     * parses the listexpr from chessmove to a chessmovedates object
     */
	static public ChessMoveDates parseChessMove( ListExpr value){
		//System.out.println("my move expr "+ value.writeListExprToString());
		ChessMoveDates cm = null;
		if ( value.listLength() == 6 )
		{
		    ListExpr First = value.first();
		    ListExpr Second = value.second();
		    ListExpr Third = value.third();
		    ListExpr Fourth = value.fourth();
		    ListExpr Fifth = value.fifth();
		    ListExpr Sixth = value.sixth();
		    if (First.isAtom() && (First.atomType() == ListExpr.STRING_ATOM) && 
			Second.isAtom() && (Second.atomType() == ListExpr.STRING_ATOM) && 
			Third.isAtom() && (Third.atomType() == ListExpr.STRING_ATOM) &&
			Fourth.isAtom() && (Fourth.atomType() == ListExpr.STRING_ATOM) && 
			Fifth.isAtom() && (Fifth.atomType() == ListExpr.STRING_ATOM) &&
			Sixth.isAtom() && (Sixth.atomType() == ListExpr.INT_ATOM) &&
			Fourth.stringValue().length() >= 3 &&
			Second.stringValue().length() >= 2 && 
			Third.stringValue().length() >= 2 )
		    {
			int mfig = ChessObject.parseFigureChar(First.stringValue().charAt(0));    
			int colS = 8-Character.digit(Second.stringValue().charAt(1), 10);
			int rowS = Character.getNumericValue(Second.stringValue().charAt(0))-10;
			int colE = 8-Character.digit(Third.stringValue().charAt(1), 10);
			int rowE = Character.getNumericValue(Third.stringValue().charAt(0))-10;
			//System.out.println(rowS+" "+colS+" "+rowE+" "+colE+" "+mfig+ " "+Fourth.stringValue()+ " "+Sixth.intValue());
//			chess or chessmate ?
			char chess = Fourth.stringValue().charAt(0);
//    	 		figure captured ?
			int cfig =0;
			if (Fifth.stringValue().length() == 1) 
				cfig = ChessObject.parseFigureChar(Fifth.stringValue().charAt(0));
//			pawn promotion ?
			int pfig = ChessObject.parseFigureChar(Fourth.stringValue().charAt(1));
//			castling ?
			char cast = Fourth.stringValue().charAt(2);
			int number = Sixth.intValue();
			cm = new ChessMoveDates(rowS,rowE,colS,colE,mfig,cfig,pfig,chess,cast,number);
			//System.out.println("proF "+pfig+" capF "+cfig);
		    }
		    else Reporter.showError("invalid chessmove object"); 
		}
		else Reporter.showError("invalid chessmove object");
		return cm;
	}
	
	/*
	 * parses the intern figurenumber to a string 
	 */
	public static String parseToFigName(int fig) {
		switch (fig) {
		// sorted after frequency of use
			case 0: return "none";
			case 1: return "Pawn";
			case 7: return "pawn";
			case 3: return "Knight";
			case 9: return "knigth";
			case 5: return "Queen";
			case 11: return "queen";
			case 2: return "Bishop";
			case 8: return "bishop";
			case 4: return "Rook";
			case 10: return "rook";
			case 6: return "King";
			case 12: return "king";
			default: return "none";
		}
	}
	/*
	 * parses the intern filenumber to a char 
	 */
	public static char parseCol(int i) {
		switch (i) {
		case 0: return 'a';
		case 1: return 'b';
		case 2: return 'c';
		case 3: return 'd';
		case 4: return 'e';
		case 5: return 'f';
		case 6: return 'g';
		case 7: return 'h';
		default: return '-';
		}
	}
	/*
	 * parses the intern rownumber to a char 
	 */
	public static char parseRow(int i) {
		switch (i) {
		case 0: return '8';
		case 1: return '7';
		case 2: return '6';
		case 3: return '5';
		case 4: return '4';
		case 5: return '3';
		case 6: return '2';
		case 7: return '1';
		default: return '-';
		}
	}
	
	/*
	 * returns a string with the Query of nTh Attribut operator for getting inquirys of chessmoves 
	 */
	public static String getQueryMoves(int i, ChessMoveDates cm) {
		if (cm != null ) {
			switch (i){
				case 0 : //agent 
					return "agent(move) = \""+ChessObject.parseToFigName(cm.movFigure)+"\""; 
				case 1 : //cap
					return "captured(move) = \""+ChessObject.parseToFigName(cm.capFigure)+"\"";
				case 2 : //startR
					return "startrow(move) = "+ChessObject.parseRow(cm.rowS);
				case 3 : //end R
					return "endrow(move) = "+ChessObject.parseRow(cm.rowE);
				case 4 : // startC
					return "startfile(move) = \""+ChessObject.parseCol(cm.colS)+"\"";
				case 5 : // end C
					return "endfile(move) = \""+ChessObject.parseCol(cm.colE)+"\"";
				default: 	return null;
			}
		}
		return null;	
	}
	public static boolean saveObject(StringBuffer errorMessage, StringBuffer objName, String type, String le) {
		String name = JOptionPane.showInputDialog(null,
        		"name of the object for saving", objName);
		StringBuffer command = new StringBuffer();
		if (name.equals(objName.toString())) { //update this object
			command.append("update ");
			command.append(name);
			command.append(" :");
			}
		else { //create new object
			command.append("let ");
			command.append(name);
			}
		command.append("= [const ");
		command.append(type);
		command.append(" value ");
		command.append(le);
		command.append("];");
		System.out.println(command.toString()); 		
		// init secondointerface
		UpdateInterface updateInterface = MainWindow.getUpdateInterface();
		ListExpr resultList = new ListExpr();
		IntByReference errorCode = new IntByReference(0);
		IntByReference errorPos = new IntByReference(0);
		
		// Executes the remote command.
		if (updateInterface.isInitialized()){
			updateInterface.secondo(command.toString(), resultList, errorCode, errorPos, errorMessage);
		}
		else {
			errorMessage = new StringBuffer("Connection to SECONDO lost!");
		}
		if (errorMessage.length()!=0) {
			errorMessage.insert(0,"saving failed\n");
			return false;
		}	
		else {
			errorMessage.insert(0,"saving "+ name +" successful\n");
			objName = new StringBuffer(name);
			//System.out.println(" new  "+ name + " ori  "+ objName);
			return true;
		}
	}
}
