package Ext_Tools;

import java.util.Iterator;
import java.util.Vector;


/**
 * 
 * <b> Task of this class </b> <br/>
 * A ValueList represents a Term which can contain several Factors
 * Each Factor needs to be connected via an operator (Asterisk or Solidus) to another Factor
 * The operator is stored inside the Factor class (called SimpleValue)
 * This class basically consists of a vector of SimpleValue elements. 
 * 
 */
public class ValueList {
	
	private Vector<SimpleValue> SV;
	private boolean hasSubQuery;
	private boolean hasSetFunction;
	private String Operator;
	
	/**
	 * <b> Task of the constructor </b> <br/>
	 * It automatically stores a term
	 * @param newElem
	 */
	public ValueList(SimpleValue newElem) {
		SV = new Vector<SimpleValue>();
		SV.add(newElem);
		this.hasSubQuery = newElem.containsSub();
		this.hasSetFunction = newElem.containsSetFunc();
		this.Operator = "";
	}
	
	
	/**
	 * <b> Task of this method </b> <br/>
	 * It stores a new element and the operator the new element is connected with
	 * it is eather a Asterisk or a Solidus
	 * @param newElem
	 * @param op
	 * @return
	 */
	public ValueList addElement(SimpleValue newElem, String op) {
		newElem.addOperator(op);
		SV.add(newElem);
		return this;
	}
	
	public boolean containsSub() {
		return this.hasSubQuery;
	}
	
	public boolean containsSetFunc() {
		return this.hasSetFunction;
	}
	
	/**
	 * <b> Task of this method </b> <br/>
	 * ValueLists might add up to a Numeric Value Expression. Therefore they need to be able to contain an operator like + or -
	 * The operator is set by the NumericValueExpression-Class (called ValueExpr) if needed.
	 * 
	 * @param op Either Plus or Minus
	 */
	public void addOperator(String op) {
		this.Operator = op;
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * returns the content of this ValueList in form of a string
	 * @return
	 */
	public String getStrValueList() {
		String result = "";
		Iterator<SimpleValue> it;
		
		it = this.SV.iterator();
		while (it.hasNext())
			result += it.next().getString();
		
		if (this.Operator != "")
			result = this.Operator + " " + result;
		
		return result;
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * returns the subquery in case the ValueList contains a subquery
	 * @return
	 */
	public QueryClause getSubValueList() {
		Iterator<SimpleValue> it = this.SV.iterator();
		 
		return it.next().getQuery();
		
	}

}
