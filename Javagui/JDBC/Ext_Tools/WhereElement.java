package Ext_Tools;

/**
 * 
 * <b> Task of this class </b> <br/>
 * It represents a predicate which basically consists of ValueExpressions and Operators.
 * It can also contain a subquery.
 * It also represents BoolPrim, BoolTest and BoolFactor since they do not differ much from predicate
 */
public class WhereElement {
	
	private boolean NotExpr = false; // it is for a bool-factor: not Bool-Test
	private StringBuffer RVC1;
	private StringBuffer RVC1Quali = new StringBuffer("");
	private StringBuffer RVC2;
	private StringBuffer RVC2Quali = new StringBuffer("");
	private String FirstOperator;
	private String SecondOperator;
	private boolean QualifierNeeded;
	private boolean SubqueryInUse;
	private QueryClause Sub;
	private QualifierList QList;
	private WhereClause SearchCond;
	private boolean SearchCondinUse;
	
	
	/**
	 * 
	 * <b> Task of this constructor </b> <br/>
	 * it creates a WhereElement without a subquery. It checks whether qualifiers are needed and 
	 * in case they are needed it stores the needed qualifiers in a QualifierList 
	 * @param rvc1
	 * @param rvc2
	 * @param op
	 */
	public WhereElement(String rvc1, String rvc2, String op) {
		this.QList = new QualifierList();
		this.RVC1 = new StringBuffer(rvc1);
		this.QualifierNeeded = this.check4Qualifiers(this.RVC1, this.RVC1Quali);
		if (this.QualifierNeeded)
			this.QList.addQualifier(new Qualifier(this.RVC1.toString(), this.RVC1Quali.toString()));
		this.RVC2 = new StringBuffer(rvc2);
		if (this.check4Qualifiers(this.RVC2, this.RVC2Quali)) {
			this.QList.addQualifier(new Qualifier(this.RVC2.toString(), this.RVC2Quali.toString()));
			this.QualifierNeeded = true;
		}
		this.FirstOperator = op;
		this.SecondOperator = "";
		this.SubqueryInUse = false;
		this.SearchCondinUse = false;
	}
	
	/**
	 * 
	 * <b> Task of this constructor </b> <br/> 
	 * creates a WhereElement with a subquery. It also checks whether a qualifier is needed and 
	 * stores the needed qualifier in a QualifierList
	 *@param rvc
	 *@param op1
	 *@param op2
	 *@param sub
	 */
	public WhereElement(String rvc, String op1, String op2, QueryClause sub) {
		this.QList = new QualifierList();
		this.RVC1 = new StringBuffer(rvc);
		this.QualifierNeeded = this.check4Qualifiers(this.RVC1, this.RVC1Quali);
		if (this.QualifierNeeded)
			this.QList.addQualifier(new Qualifier(this.RVC1.toString(), this.RVC1Quali.toString()));
		this.RVC2 = new StringBuffer("");
		this.FirstOperator = op1.trim();
		this.SecondOperator = op2.trim();
		this.SubqueryInUse = true;
		this.Sub = sub;
		this.SearchCondinUse = false;
	}
	
	/**
	 * 
	 * <b> Task of this constructor </b> <br/>
	 * It is needed in case a Bool-Primary does not just contain a predicate but a 
	 * Search Condition in brackets
	 * @param wc
	 */
	public WhereElement(WhereClause wc) {
		this.SearchCond = wc;
		this.SubqueryInUse = false;
		this.SearchCondinUse = true;
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * if this WhereElement contains a subquery the upper query for the subquery is set here
	 * @param UQ
	 */
	public void SetUpperQuery(QueryClause UQ) {
		if (this.SubqueryInUse)
			this.Sub.SetUpperClause(UQ);
	}
	
	public boolean isSubqueryInUse() {
		return this.SubqueryInUse;
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * A BoolFactor can contain NOT
	 */
	public void SetNotExpr() {
		this.NotExpr = true;
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * outputs the WhereElement as a string
	 * it throws an exception in case the QueryExpression has a groupby-conflict
	 * @return
	 */
	public String getWhereElement() throws Exception{
		String result;
		
		if (!this.SubqueryInUse && !this.SearchCondinUse) {
			result = this.addQualiToElement(RVC1Quali.toString(), RVC1.toString());
			result += this.FirstOperator;
			result += this.addQualiToElement(RVC2Quali.toString(), RVC2.toString());			
		}
		else if (this.SearchCondinUse) {
			result = "(" + this.SearchCond.getWhereClause() + ")";
		}
		else {
			if (this.FirstOperator.equalsIgnoreCase("exists"))
				result = "[exists" + this.Sub.getQueryExpression() + "]";
			else {
				result = this.addQualiToElement(RVC1Quali.toString(), RVC1.toString());
				if (this.FirstOperator != "")
					result += " " + this.FirstOperator + " ";  // in case first operator is Not_Expression it could be ""
				if (this.SecondOperator != "") {
					if (this.checkForQuantifier(this.SecondOperator))
						result += "(" + this.SecondOperator + this.Sub.getQueryExpression() +")"; //... where no > (any (subclause))
					else
						result += " " + this.SecondOperator + this.Sub.getQueryExpression();     // ... where no not in (subclause)
				}
				else
					result += this.Sub.getQueryExpression();										// ... where no > (subclause)
				//result += " " + this.Sub.getQueryExpression();
			}
		}
		
		if (this.NotExpr)
			result = "not " + result;
		
		return result;
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * connects qualifier and attribut to one expression. Needed for output as a string
	 * @param quali
	 * @param elem
	 * @return
	 */
	private String addQualiToElement(String quali, String elem) {
		String result;
		
		result = elem;
		if (!quali.equalsIgnoreCase("")) 
			result = quali + ":" + result;
		
		return result;
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * checks whether a passed operator is a quantifier such as all, any ...
	 * @param quanti
	 * @return
	 */
	private boolean checkForQuantifier(String quanti) {
		boolean result = false;
		if (quanti.equalsIgnoreCase("all") || quanti.equalsIgnoreCase("any") || quanti.equalsIgnoreCase("some"))
			result = true;
		return result;
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * checks whether the passed attribut contains a qualifier and stores the attribut under Att and 
	 * the qualifier under Quali. Att and Quali need to be StringBuffer because they are changed and
	 * need to be passed back.
	 * @param Att
	 * @param Quali
	 * @return
	 */
	private boolean check4Qualifiers(StringBuffer Att, StringBuffer Quali) {
		boolean result = false;
		int PosColon;
		
		PosColon = Att.indexOf(":");
		if (PosColon!=-1) {
			Quali.append(Att.substring(0, PosColon));
			Att = Att.delete(0, PosColon+1);
			result = true;
		}
		
		return result;
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * the passed QualifierList will be added with the qualifiers needed in the subquery or 
	 * with the qualifiers of this WhereElement
	 * @param ql
	 * @return true if ql contains Qualifiers
	 */
	public boolean getQualifierNeeded(QualifierList ql) {
		
		if (!this.SearchCondinUse) {
			if (this.SubqueryInUse)
				this.Sub.SetQualifier(ql); // ql is filled if the Subquery needs a qualifier
			
			/* this.QList is the QualifierList created by this WhereElement. ql is the 
			 * QualifierList with can contain many other Qualifiers from different WhereElements, 
			 * this Subquery or can be empty. It is important to call addQualifierList from ql 
			 * because the passed QualifierList needs to be filled up with Qualifiers. If we
			 * would fill up this.QList and then assign ql=this.QList only the formal parameter 
			 * will be assigned to a new QList. The actual parameter will still point to the same 
			 * QList as before.
			 */
			if (this.QualifierNeeded)
				this.QList = ql.addQualifierList(this.QList);
		}
		else
			this.SearchCond.NeedQualifier(ql);
		
		return (!ql.isEmpty());
	}

}
