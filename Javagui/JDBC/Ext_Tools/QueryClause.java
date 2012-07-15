package Ext_Tools;

import java.util.*;
import SecExceptions.*;

/**
 * 
 * <b>Task of this class: </b> <br/>
 * It represents a Query Specification. So this class also includes subqueries. 
 */
public class QueryClause {
	
	private SelectClause SClause;
	private TableClause TClause;
	private WhereClause WClause;
	private GroupClause GClause;
	private OrderClause OClause;
	private boolean SClauseSet;
	private boolean TClauseSet;
	private QueryClause UpperQC;
	private QualifierList QList;
	
	public QueryClause() {
		this.SClauseSet = false;
		this.TClauseSet = false;
		this.QList = new QualifierList();
	}
	
	/**
	 * 
	 * <b>Task of this method: </b> <br/> 
	 * it adds a SelectClause to the Query Specification
	 * @param sclause
	 */
	public void addSClause(SelectClause sclause) {
		this.SClause = sclause;
		this.SClauseSet = true;
	}
	
	/**
	 * 
	 * <b>Task of this method: </b> <br/> 
	 * it adds a TableClause to the Query Specification
	 * @param tlist
	 */
	public void addTClause(TableClause tlist) {
		this.TClause = tlist;
		this.TClauseSet = true;
	}
	
	/**
	 * 
	 * <b>Task of this method: </b> <br/> 
	 * it adds a WhereClause to the Query Specification. It also defines this Query Specification
	 * as upper query to all subqueries the WhereClause contains.
	 * @param wclause
	 */
	public void addWClause(WhereClause wclause) {
		
		this.WClause = wclause;
		this.WClause.SetUpperQuery(this);
		
	}
	
	public void addGClause(GroupClause gclause) {
		this.GClause = gclause;
	}
	
	public void addOClause(OrderClause oclause) {
		this.OClause = oclause;
	}
	
	/**
	 * 
	 * <b>Task of this method: </b> <br/> 
	 * if this object represents a subquery the Query Specification which contains this subquery
	 * is set here. 
	 * @param UC
	 */
	public void SetUpperClause(QueryClause UC) {
		this.UpperQC = UC;
	}
	
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * it sets the passed qualifier the SelectClause of the QuerySpecification, TableClause respectively.
	 * @param ql the Qualifier-Object which contains the qualifier the attribute needs to be qualified with
	 * and the attribute which needs to be qualified
	 * @return true if setting of the qualification was successful
	 */
	private boolean AddQuali(Qualifier ql ) {
		boolean result = false;
		
		if (TClauseSet && SClauseSet) 
			if (TClause.CheckOrAddQuali(ql.getQuali())) {
				//if (this.SClause.addQuali(ql.getQuali(), ql.getAtt()))
				result = true;
				this.SClause.addQuali(ql.getQuali(), ql.getAtt());
			}
		return result;
	}
	
	
	/**
	 * 
	 * <b>Task of this method: </b> <br/> 
	 * It recursively sets and checks for qualifiers in this Query Specification and all containing subqueries.
	 * The QualifierList is filled up with all needed qualifiers while this recursive method 
	 * goes through all WhereClauses of this QuerySpecification and through all WhereClauses of all the connected 
	 * subqueries. Every subquery and finally the first QuerySpecification checks all qualifiers in the 
	 * QualifierList and tries to set them in its SelectList, TableList respectively. If successful the referred 
	 * qualifier is removed from the QualifierList. Afterwards it is checked that no Select-Element uses
	 * qualifiers which are not set in any Table-Element. This is necessary in case there is no where-clause
	 * but the Select-Element uses qualifiers anyway.
	 * @param QuLi
	 * @return false if not all qualifiers from the QualifierList could be set.
	 */
	public boolean SetQualifier(QualifierList QuLi) {
		boolean result = true;
		Iterator<Qualifier> it;
		Qualifier CurrentQuali;
		
		if (this.WClause.NeedQualifier(QuLi)) {
			it = QuLi.getIterator();
			while (it.hasNext())
				if (this.AddQuali(it.next()))
					it.remove();   // if Qualifier could be set it is removed from the QualifierList
				else
					result = false;  // if any one Qualifier could not be set then it has to be passed to an upper Query. 
									// if there is no upper Query it results to a mistake
		}
		
		/* In this section it is checked if there are any qualifiers in the select-elements which need to
		 * be considered in the table-elements, Group-Elements or Orderby-elements. 
		 */
		if (this.SClauseSet && this.TClauseSet) {
			QualifierList SelectOnlyQualifiers = new QualifierList();
			if (this.SClause.qualifierNeeded(SelectOnlyQualifiers)) {
				it = SelectOnlyQualifiers.getIterator();
				while (it.hasNext()) {
					CurrentQuali = it.next();
					if (!this.GClause.isEmpty())
						this.GClause.CheckAndAddQuali(CurrentQuali);
					if (!this.OClause.isEmpty())
						this.OClause.CheckAndAddQuali(CurrentQuali);
					if (this.TClause.CheckOrAddQuali(CurrentQuali.getQuali()))
						it.remove();
				}
				result &= SelectOnlyQualifiers.isEmpty();
				QuLi.mergeShadows(SelectOnlyQualifiers);
			}
		}
		
		/* This section checks if there are any unqualified select-Elements in case there is 
		 * just one Table-Element and it has an alias. If there is only one Table-Element and 
		 * it has an alias every select-element needs to be qualified with this alias. If there are
		 * more Table-Elements this problem cannot be solved because it is not possible to determine
		 * which Select-Element belongs to which Table-Element. 
		 */
		if (this.SClauseSet && this.TClauseSet) {
			String Alias = this.TClause.OneElement();
			if (Alias != "") {
				this.SClause.setQuali(Alias, QuLi);
				if (!this.GClause.isEmpty()) this.GClause.SetAllOtherQualis(Alias);
				if (!this.OClause.isEmpty()) this.OClause.SetAllOtherQualis(Alias);
			}
		}
		
		
		return result;
	}
	
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * evaluates the query expression and outputs a string.
	 * It is important that the qualifiers are not set earlier than in this method. This method is invoked by 
	 * the parser at the very end of the evaluation. That ensures all the subqueries are properly connected 
	 * and the method SetQualifier is firstly invoked by the highest QuerySpecification and not by any subquery.
	 * @return the query expression as a string or "-1" if there is a mistake
	 */
	public String getQueryExpression() throws Exception{
		String result;
		String WPreresult;
		String GPreresult;
		String OPreresult;
		//boolean couldSetQualifier = true;
		
		if (this.SClause.groupbyNeeded(!this.GClause.isEmpty()) && this.GClause.isEmpty()) // in Secondo a groupby part is needed if there is a Set-Func-Specification and another Select-Element
			throw new NotSuppException("This usage of Set-Function-Specifications without groupby ");
		
		if (this.SetQualifier(this.QList) || this.UpperQC != null) {	//here the qualifiers are set	
			result = "select " + this.SClause.getSelectClause();		//the SelectClause with qualifiers is added
			result += " from " + this.TClause.getTableList();			//the TableClause with qualifiers is added
			WPreresult = this.WClause.getWhereClause();	
			GPreresult = this.GClause.getGroupClause();
			OPreresult = this.OClause.getOrderClause();
			result += WPreresult;
			if (GPreresult != "")
				result += " groupby " + GPreresult;
			if (OPreresult != "")
				result += " orderby " + OPreresult;
			if (this.UpperQC != null)
				result = "(" + result + ")";       // this QueryExpression is a subclause
		}
		else 
			result = "-1";	// error because Qualifiers could not resolved and there is no more upper Query
		
		return result;
	}
	
	public Vector<ShadowQualifier> getShadowList() {
		return this.QList.getShadowList();
	}
}
