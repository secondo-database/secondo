package Ext_Tools;

import java.util.Iterator;
import java.util.Vector;

/**
 * 
 * <b> Task of this class </b> <br/>
 * It represents a row value constructor which can be a row value constructor element (value expression)
 * or a row value constructor list (comma separated and in brackets).
 */
public class ValueExprList {
	private Vector<ValueExpr> VEL;
	
	public ValueExprList(ValueExpr newElem) {
		VEL = new Vector<ValueExpr>();
		VEL.add(newElem);
	}
	
	public ValueExprList addElement(ValueExpr newElem) {
		this.VEL.add(newElem);
		return this;
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * it is needed to check whether this object contains a value constructor element or a
	 * value constructor list. 
	 * @return true if it is an value constructor element
	 */
	public boolean moreThanOne() {
		return (this.VEL.size() > 1);
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * it returns the ValueExpr of the list. moreThanOne() needs to be checked beforehand and
	 * must be positive
	 * @return
	 */
	public ValueExpr getValueExpr() {
		return this.VEL.get(0);
	}
	
	public boolean containsSub() {
		boolean result = false;
		if (this.VEL.size() == 1)
			result = this.VEL.get(0).containsSub();
		
		return result;		
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * returns the ValueExprList as a String
	 * @return
	 */
	public String getValueExprList() {
		Iterator<ValueExpr> it;
		String result = "";
		
		it = this.VEL.iterator();
		if (it.hasNext())
			result = it.next().getStrValueExpr();
		while (it.hasNext())
			result += ", " + it.next().getStrValueExpr();
		
		if (this.VEL.size() > 1)
			result = "[" + result + "]";
		
		return result;
	}

}
