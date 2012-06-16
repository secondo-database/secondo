package Ext_Tools;

import java.util.Iterator;
import java.util.Vector;

/**
 * 
 * <b>Task of this class: </b> <br/>
 * it provides a list of where-elements which are connected by the logical operator OR
 */
public class WhereClause {
	
	private Vector<WhereList> WLists;
	private boolean Empty = false;
	
	public WhereClause(WhereList newElem) {
		WLists = new Vector<WhereList>();
		WLists.add(newElem);
	}
	
	/**
	 * 
	 * <b> Task of this constructor </b> <br/>
	 * if no WhereList is passed the queryclause has no where-section
	 * @param Nothing
	 */
	public WhereClause(String Nothing) {
		this.Empty = true;
	}
	
	public WhereClause addElement(WhereList newElem) {
		WLists.add(newElem);
		return this;
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * checks whether any of the Where-Elements of the where-Lists contains a sub
	 * it is needed for Search-Condition in Bool-Primaries because they are not supported
	 * if the Search-Condition contains a sub.
	 * @return
	 */
	public boolean containsSubs() {
		boolean result = false;
		
		if (!this.Empty) {
			Iterator<WhereList> it;
			it = this.WLists.iterator();
		
			while (it.hasNext() && !result)
				result |= it.next().containsSubs();
		}
		
		return result;
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * in case any of the WhereElements of the WhereLists uses Subqueries the upper query is 
	 * set here
	 * @param UQ
	 */
	public void SetUpperQuery(QueryClause UQ) {
		if (!this.Empty) {
			Iterator<WhereList> it;
			it = this.WLists.iterator();
		
			while (it.hasNext())
				it.next().SetUpperQuery(UQ);
		}
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * all WhereElements of all WhereLists are looked through and ql is filled up
	 * with all Qualifiers needed.
	 * @param ql
	 * @return
	 */
	public boolean NeedQualifier(QualifierList ql) {
		boolean result = false;
		
		if (!this.Empty) {
			Iterator<WhereList> it;
			it = this.WLists.iterator();
			while (it.hasNext())
				result |= it.next().getQualifierNeeded(ql);
		}
		
		return result;
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * Outputs the WhereClause as a string
	 * it throws an exception in case the QueryExpression of the Where-Element has a groupby-conflict
	 * @return
	 */
	public String getWhereClause() throws Exception{
		String result="";
		
		if (!this.Empty) {
			Iterator<WhereList> it;
			it = this.WLists.iterator();
			if (it.hasNext())
				result = "(" + it.next().getWhereList() + ")";
			while (it.hasNext())
				result += " or (" + it.next().getWhereList() + ")";
			if (result != "")
				result = " where " + result;
		}
		
		return result;
	}
}
