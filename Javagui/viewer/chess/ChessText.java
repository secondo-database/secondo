package viewer.chess;

import java.awt.BorderLayout;

import javax.swing.JTextArea;

import sj.lang.ListExpr;

/*
 * this class is used to show all not chessojects
 */
public class ChessText extends ChessObject{
	private JTextArea text;
	
	public  ChessText(ListExpr value) {
		val  = value;
		text = new JTextArea();
		text.setText(val.writeListExprToString());
		this.setLayout(new BorderLayout());
		this.add(text, BorderLayout.CENTER);
	}
	
	/*
	 * 
	 * @see viewer.chess.ChessInterface#changeToEdit(boolean)
	 */
	public boolean changeToEdit(boolean edit) {
		editModus = false;
		return false;
	}

	/*
	 * @see viewer.chess.ChessInterface#getType()
	 */
	public String getType() {
		return null;
	}

	/*
	 * @see viewer.chess.ChessInterface#getListExpr()
	 */
	public ListExpr getListExpr() {	
		return val;
	}

	/*
	 * @see viewer.chess.ChessInterface#update(sj.lang.ListExpr)
	 */
	public void update(ListExpr value) {
		val  = value;
		text.setText(val.writeListExprToString());
	}
	
	/*
	 * @see viewer.chess.ChessInterface#canEdit()
	 */
	public boolean canEdit() {
		return false;
	}
	
	/* 
	 * @see viewer.chess.ChessInterface#canExport()
	 */
	public boolean canExport() {
		return false;
	}

}
