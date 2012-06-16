package Ext_Tools;

import java.util.*;

/**
 * 
 * <b>Task of this class: </b> <br/>
 * it provides a list of where-elements which are connected by the logical operator AND
 */
public class WhereList {
	
	private Vector<WhereElement> WElems;
	private QualifierList QList;
	
	public WhereList(WhereElement newElem) {
		WElems = new Vector<WhereElement>();
		WElems.add(newElem);
	}
	
	public WhereList addElement(WhereElement newElem) {
		WElems.add(newElem);
		return this;
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * all WhereElements of this WhereList are looked through and ql is filled up
	 * with all Qualifiers needed.
	 * @param ql
	 * @return
	 */
	public boolean getQualifierNeeded(QualifierList ql) {
		boolean result = false;
		
		Iterator<WhereElement> it;
		it = this.WElems.iterator();
		while (it.hasNext())
			result |= it.next().getQualifierNeeded(ql);
		
		return result;
	}
	
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * checks whether any of the Where-Elements of the where-List contains a sub
	 * it is needed for Search-Condition in Bool-Primaries because they are not supported
	 * if the Search-Condition contains a sub.
	 * @return
	 */
	public boolean containsSubs() {
		boolean result = false;
		Iterator<WhereElement> it;
		
		it = this.WElems.iterator();
		while (it.hasNext())
			result |= it.next().isSubqueryInUse();
		
		return result;
	}
	
	/**
	 * in case any of the WhereElements uses Subqueries the upper query is set here
	 */
	public void SetUpperQuery(QueryClause UQ) {
		Iterator<WhereElement> it;
		it = this.WElems.iterator();
		
		while (it.hasNext())
			it.next().SetUpperQuery(UQ);
	}
	
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * Outputs the WhereList as a string
	 * it throws an exception in case the QueryExpression of the Where-Element has a groupby-conflict
	 * @return
	 */
	public String getWhereList() throws Exception{
		String result="";
		
		Iterator<WhereElement> it;
		it = this.WElems.iterator();
		if (it.hasNext())
			result = "(" + it.next().getWhereElement() + ")";
		while (it.hasNext())
			result += " and (" + it.next().getWhereElement() + ")";
		
		return result;
	}
	
}
