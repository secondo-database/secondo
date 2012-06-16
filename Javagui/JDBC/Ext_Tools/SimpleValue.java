package Ext_Tools;



/**
 * 
 * <b> Task of this class </b> <br/>
 * Objects of this class represent a VALUE EXPRESSION PRIMARY. 
 * All SQL Non Terminals a VALUE EXPRESSION PRIMARY is built by are represented by strings. 
 *
 */
public class SimpleValue {
	
	private String StrInput;
	private String Operator; // used to combine several simpleValues to a ValueList (* or /)
	private QueryClause QuInput; // in case a VALUE EXPRESSION PRIMARY contains a subquery
	private boolean hasSubQuery;
	private boolean hasSetFunction; // in case VALUE EXPRESSION PRIMARY contains a SET_FUNCITON_SPECIFICATION
	
	
	/**
	 * Constructor in case VALUE EXPRESSION PRIMARY is not a subquery
	 * @param inp
	 */
	public SimpleValue(String inp) {
		this.StrInput = inp;
		this.Operator = "";
		this.hasSubQuery = false;
		this.hasSetFunction = false;
	}
	
	/**
	 * Constructor if VALUE EXPRESSION PRIMARY is a subquery
	 * @param inp
	 */
	public SimpleValue(QueryClause inp) {
		this.QuInput = inp;
		this.hasSubQuery = true;
		this.hasSetFunction = false;
	}
	
	/**
	 * 
	 * @return the VALUE EXPRESSION PRIMARY if it is a subquery
	 */
	public QueryClause getQuery() {
		return this.QuInput;
	}
	
	/**
	 * Simple Values might add up to a Term. Therefore they need to be able to contain an operator like * or /
	 * The operator is set by the Term-Class (called ValueList) if needed.
	 * @param op either an ASTERISK or a SOLIDUS
	 */
	public void addOperator(String op) {
		this.Operator = op;
	}
	
	/**
	 * 
	 * @return returns a simple value if it is not a subquery
	 */
	public String getString() {
		String result = "";
		if (this.Operator !="")
			result = this.Operator;
		result += this.StrInput;
		
		return result;
	}
	
	public boolean containsSub() {
		return this.hasSubQuery;
	}
	
	public boolean containsSetFunc() {
		return this.hasSetFunction;
	}
	
	public void setSetFunc() {
		this.hasSetFunction = true;
	}
	
	/**
	 * Simple Value also represents a Factor. Therefore it needs to be able to contain a minus sign
	 * @param si either - or an empty string
	 * @return the simpleValue as a Factor.
	 */
	public SimpleValue setSign(String si) {
		SimpleValue result;
		
		result = null;
		if (!this.hasSubQuery) {
			this.StrInput = si + this.StrInput;
			result = this;
		}
		
		return result;
	}

}
