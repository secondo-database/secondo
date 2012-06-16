package Ext_Tools;

import java.util.Iterator;
import java.util.Vector;

/**
 * 
 * <b> Task of this class </b> <br/>
 * A ValueExpr represents a Numeric Value Expression or a Value Expression which can contain several Terms
 * Each Term needs to be connected via an operator (Plus or Minus) to another Term
 * The operator is stored inside the Term class (called ValueList)
 * This class basically consists of a vector of ValueList elements.
 */
public class ValueExpr {
	
	private Vector<ValueList> VList;
	private boolean hasSubQuery;
	private boolean hasSetFunction;
	
	public ValueExpr(ValueList newElem) {
		VList = new Vector<ValueList>();
		VList.add(newElem);
		this.hasSubQuery = newElem.containsSub();
		this.hasSetFunction = newElem.containsSetFunc();
	}
	
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * Adds a new ValueList and returns the resulting ValueExpr.
	 * @param newElem
	 * @param op
	 * @return
	 */
	public ValueExpr addElement(ValueList newElem, String op) {
		newElem.addOperator(op);
		VList.add(newElem);
		return this;
	}
	
	public boolean containsSub() {
		return this.hasSubQuery;
	}
	
	public boolean containsSetFunc() {
		return this.hasSetFunction;
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * returns the content of this ValueExpr in form of a string
	 * @return
	 */
	public String getStrValueExpr() {
		String result = "";
		Iterator<ValueList> it;
		
		it = this.VList.iterator();
		while (it.hasNext())
			result += it.next().getStrValueList();
		
		return result;
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * returns the subquery in case this ValueExpr contains a subquery
	 * @return
	 */
	public QueryClause getSubValueExpr() {
		Iterator<ValueList> it = this.VList.iterator();
		 
		return it.next().getSubValueList();
		
	}

}
