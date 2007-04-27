package viewer.chess;


import javax.swing.JPanel;
import javax.swing.JLabel;
import javax.swing.JTextField;
import javax.swing.border.*;
import javax.swing.BorderFactory;
import java.awt.BorderLayout;
import java.awt.Color;

import sj.lang.ListExpr;
import tools.Reporter;

/*
 * ChessPositionFrame is used for showing a chesspositionobject
 *  
 */

public class ChessPositionFrame extends ChessObject {

	private JTextField number;
	private JLabel label;
	private ChessField cf;
	private int[][] myField;
	
	 /*
     * constructor of ChessPositionFrame
     */
	public ChessPositionFrame(ListExpr value){
        val = value;
		//System.out.println("Hier in chesspositionframe ");
		//System.out.println(val.writeListExprToString());
		JPanel noPanel = new JPanel();
		label = new JLabel("number of position: ");
		label.setBorder(BorderFactory.createEmptyBorder(5,5,5,5));
		number = new JTextField(3);
		number.setText(" 0");
		number.setHorizontalAlignment(JTextField.RIGHT);
		number.setEditable(false);
		number.setBorder(ChessObject.linecompound5);
		noPanel.add(label);
		noPanel.add(number);
		noPanel.setBorder(BorderFactory.createEmptyBorder(5,5,5,5));
	
		if (this.parseChessPosition(val))
			cf = new ChessField(myField);
		else cf = new ChessField();
		this.setLayout(new BorderLayout());
		this.add(noPanel, BorderLayout.WEST);
		this.add(cf, BorderLayout.CENTER);
		this.validate();
	}
	
	/*
	 * updates an chessposition with a new ListExpr
	 * used when Object within a relation switches to next tuple
	 * @see viewer.chess.ChessInterface#update(sj.lang.ListExpr)
	 */
	public void update(ListExpr value) {
		if (this.parseChessPosition(value))
			cf.setField(myField); 
	}
	
	/*
	 * changes the layout of object for interaction 
	 * or changes back for no interaction
	 * @see viewer.chess.ChessInterface#changeToEdit(boolean)
	 */
	public boolean changeToEdit(boolean edit) {
		editModus = edit;
		cf.editPosition(edit);
		return true;
	}
	
	/*
	 * returns the ListExpr of actual shown chesspositionobject 
	 */
	public ListExpr getListExpr() {
//		 built the resultlist
		StringBuffer fieldString = new StringBuffer();
		int[][] field = cf.getField();
		for (int i=0; i<8;i++) {
			for (int j=0; j<8;j++) {
				fieldString.append(ChessObject.parseToChar(field[j][i]));
			}
		}
		ListExpr commandList = ListExpr.twoElemList
			(ListExpr.textAtom(fieldString.toString()),
			ListExpr.intAtom(Integer.parseInt(number.getText())));
//		System.out.println("resultList "+ commandList.writeListExprToString());
		return commandList;
	}
	
	/*
	 * returns type of chessbject as a string
	 */
	public String getType() {
		return "chessposition";
	}   
    
	/*
	 * parses the given ListExpr into the internal representation of a chesspositionobject
	 */
	private boolean parseChessPosition(ListExpr value) {
		myField = new int[8][8];
		if (value.listLength() == 2 && value.first().isAtom() && value.second().isAtom()) {
			if (value.first().atomType() == ListExpr.TEXT_ATOM &&
			value.first().textValue().length() >= 64) {
				//System.out.println(" pos"+ value.first().writeListExprToString() + "length"+ value.first().getTextLength() );
				int i= 0;
				for (int x = 0; x < 8; x++) {
					for (int y = 0; y < 8; y++) {
						myField[y][x]= ChessObject.parseFigureChar(value.first().textValue().charAt(i));
						i++;
					}
				}
			}
			else {
				Reporter.showError("invalid chessposition object");
				return false;
			}
			if (value.second().atomType() == ListExpr.INT_ATOM) 
				number.setText(Integer.toString(value.second().intValue()));
			else {
				Reporter.showError("invalid chessposition object");
				return false;
			}
		}
		else {
			Reporter.showError("invalid chessposition object"); 
			return false;
		}
		return true; 
	}

	/*
	 * chessposition is editable
	 * @see viewer.chess.ChessInterface#canEdit()
	 */
	public boolean canEdit() {
		return true;
	}
	
	/*
	 * chesposition is not exportable
	 * @see viewer.chess.ChessInterface#canExport()
	 */
	public boolean canExport() {
		return false;
	}
	
}

