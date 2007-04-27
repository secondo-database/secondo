package viewer.chess;

import java.awt.BorderLayout;
import javax.swing.JTable;
import javax.swing.border.Border;
import javax.swing.BorderFactory;
import sj.lang.ListExpr;
import tools.Reporter;
/*
 * ChessMaterialFrame is used for showing a chessmaterialobject
 *  
 */
public class ChessMaterialFrame extends ChessObject {
	private JTable material;
	private static String[] oneFigur = {" pawn"," knigth"," bishop"," rook"," queen"," king"};
	private static String[] moreFigur = {" pawns"," knigths"," bishops"," rooks"};

/*
 * constructer of ChessMaterialFrame
 */	
public ChessMaterialFrame(ListExpr value){
	//System.out.println("Hier in chessmaterialframe ");
	val = value;
	material = new JTable(7,2);
	material.setValueAt("white figures:",0,0);
	material.setValueAt("black figures:",0,1);
	this.parseChessMaterial(val);
	material.setShowGrid(false);
	material.setEnabled(false);
	this.setLayout(new BorderLayout());
	add(material,BorderLayout.CENTER);
}

/*
 * parses the given ListExpr into the internal representation of a chessmaterialobject
 */	
private void parseChessMaterial(ListExpr value) {
	int wRow=1;
	int bRow=1;
	if (value.listLength() == 6) {
		ListExpr elem = value.first();
		ListExpr rest = value.rest();
		for (int i = 0; i < 6; i++) {
			if (elem.listLength() == 2 &&  
				elem.first().isAtom() && 
				elem.first().atomType() == ListExpr.INT_ATOM && 
				elem.second().isAtom()&& 
				elem.second().atomType() == ListExpr.INT_ATOM)
			{
				int whitePlayer = elem.first().intValue();
				int blackPlayer = elem.second().intValue();
				String textW,textB;
				if (whitePlayer > 0) {
					try {
						if (whitePlayer == 1)
							textW = new String("1"+oneFigur[i]);
						else {
							textW = new String
							(Integer.toString(whitePlayer)+moreFigur[i]);
						}
						material.setValueAt(textW,wRow++,0);
					}
					catch (Exception e){
						Reporter.showError("invalid chessmaterial object");
						// should never happen, only if error in materiallist
						// (2 kings or 2 queens)
					}
				}
				if (blackPlayer > 0) {
					try {
						if (blackPlayer == 1) 
							textB = new String("1"+oneFigur[i]);
						else {
							textB = new String
							(Integer.toString(blackPlayer)+moreFigur[i]);
						}
						material.setValueAt(textB,bRow++,1);
					}
					catch (Exception e){
						Reporter.showError("invalid chessmaterial object");
						// should never happen, only if error in materiallist
						// (2 kings or 2 queens)
					}
				} 
				elem = rest.first();
				rest = rest.rest();
			}
			else Reporter.showError("invalid chessmaterial object"); 
		}
	}
	else Reporter.showError("invalid chessmaterial object");  
}

/*
 * chessmaterial is not editable
 * @see viewer.chess.ChessInterface#changeToEdit(boolean)
 */
public boolean changeToEdit(boolean edit) {
	editModus = false;
	return false;
}

/*
 * (non-Javadoc)
 * @see viewer.chess.ChessInterface#getType()
 */
public String getType() {
	return "chessmaterial";
}

/*
 * (non-Javadoc)
 * @see viewer.chess.ChessInterface#getListExpr()
 */
public ListExpr getListExpr() {
	return val;
}

/*
 * update chessmaterialobject
 * @see viewer.chess.ChessInterface#update(sj.lang.ListExpr)
 */
public void update(ListExpr value) {
	val = value;
	for (int i = 1; i < 7; i++) {
		material.setValueAt("",i,0);
		material.setValueAt("",i,1);
	}
	this.parseChessMaterial(val);
}

/*
 * chessmaterial is not editable
 * @see viewer.chess.ChessInterface#canEdit()
 */
public boolean canEdit() {
	return false;
}

/*
 * chesmaterial is not exportable
 * @see viewer.chess.ChessInterface#canExport()
 */
public boolean canExport() {
	return false;
}

}

