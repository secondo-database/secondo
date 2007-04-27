package viewer.chess;

import sj.lang.ListExpr;

public interface ChessInterface {
	
	/*
	 * returns type of chessbject as a string
	 */
	public String getType();
	
	/*
	 * updates an chessobject with a new ListExpr
	 * used when Object within a relation switches to next tuple
	 */
	public void update(ListExpr value);
	
	/*
	 * changes the layout of object for interaction 
	 * or changes back for no interaction
	 */
	public boolean changeToEdit(boolean edit);
	
	/*
	 * returns true if object is editable
	 * false if not
	 */
	public boolean canEdit();
	
	/*
	 * returns true if object is exportable
	 * false if not
	 */
	public boolean canExport();
	
	/*
	 * returns the ListExpr of actual shown object 
	 */
	public ListExpr getListExpr();
}
