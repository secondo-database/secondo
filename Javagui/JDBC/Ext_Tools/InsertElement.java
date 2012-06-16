package Ext_Tools;

import SecExceptions.NotSuppException;

/**
 * 
 * <b> Task of this class </b> <br/>
 * The insert-element "Simple_Table" might need to contain a subquery. In order to receive a subquery
 * this class is needed.
 */
public class InsertElement {
	private QueryClause InsertQuery;
	private String ValueList;
	
	public InsertElement(QueryClause insquery) {
		this.InsertQuery = insquery;
		this.ValueList = "";
	}
	
	public InsertElement(String valuelist) {
		this.ValueList = valuelist;
		this.InsertQuery = null;
	}
	
	public QueryClause getQuery() {
		return this.InsertQuery;
	}
	
	public String getInsertElement() throws Exception{
		String result="";
		
		if (this.InsertQuery != null) {
			result = this.InsertQuery.getQueryExpression();
			if (result.equalsIgnoreCase("-1"))
				throw new NotSuppException("There is an error with the qualifiers. This usage of a Select-Clause within an insert-clause ");
		}
		else
			result = "values " + this.ValueList;
		
		return result;
	}
}
