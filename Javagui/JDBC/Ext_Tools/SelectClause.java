package Ext_Tools;

/**
 * 
 * <b> Task of this class </b> <br/>
 * It represents a SelectClause in the parser.
 */
public class SelectClause {
	
	private boolean NoQualifierNeeded;	//in case Select Element is *
	private SelectList SList;
	private String STResult;
	private String DiClause;
	
	/**
	 * 
	 * <b> Task of this constructor </b> <br/>
	 * in this case the SelectClause contains a SelectList
	 * @param sl
	 */
	public SelectClause(SelectList sl) {
		this.NoQualifierNeeded = false;
		this.STResult="";
		this.SList = sl;
	}
	
	/**
	 * 
	 * <b> Task of this constructor </b> <br/>
	 * In this case the SelectClause contains a * 
	 * @param NonList
	 */
	public SelectClause(String NonList) {
		this.NoQualifierNeeded = true;
		this.SList = null;
		this.STResult = NonList;
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * The DistinctClause (DISTINT, ALL or nothing) is set
	 * @param DiCl
	 */
	public void setDistinctClause(String DiCl) {
		this.DiClause = DiCl;
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * The content of the SelectClause is returned in a string
	 * @return
	 */
	public String getSelectClause() {
		String result;
		if (this.NoQualifierNeeded)
			result = this.DiClause + this.STResult;
		else
			result = this.DiClause + this.SList.getSelectList();
		
		return result;
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * A qualifier can be added here. It will be done if the need for a qualifier
	 * is recognized in a WhereElement
	 * @param quali
	 * @param att
	 * @return true if a Qualifier could be set
	 */
	public boolean addQuali(String quali, String att) {
		boolean result = false;
		if (!this.NoQualifierNeeded)
			result = this.SList.addQuali(quali, att);
		
		return result;
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * All Select-Elements needs to be set with the Alias as Qualifier. The QualifierList does not need
	 * to be set, because it does not need to be resolved. The ShadowQualifier List however needs to be
	 * set. This method is needed in case the only table-element uses a qualifier which is not enforced or
	 * just enforced by one of the Select-Elements but there are more Select-Elements of this Table-Element.
	 * @param quali
	 * @param ql
	 */
	public void setQuali(String Alias, QualifierList ql) {
		if (!this.NoQualifierNeeded)
			this.SList.setQualifier(Alias, ql);
	}
	
	public boolean qualifierNeeded(QualifierList ql) {
		boolean result = false;
		
		if  (!this.NoQualifierNeeded)
			result = this.SList.qualifierNeeded(ql);
		
		return result;
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * in case Select Element has more than one element and a Set_Function_Specification
	 * or group_by is used but no alias is set or an alias is used without a group_by.
	 * @return
	 */
	public boolean groupbyNeeded(boolean groupbySet) {
		boolean result = false;
		
		if (!this.NoQualifierNeeded)   // in case SelectElement = *
			result = this.SList.groupbyNeeded(groupbySet);
		return result;
	}

}
