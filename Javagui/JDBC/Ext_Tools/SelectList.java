package Ext_Tools;

import java.util.*;

/**
 * 
 * <b> Task of this class </b> <br/>
 * It contains a list of SelectElements and represents SelectListExpr in the parser
 * which is a comma seperated list of SelectElements.
 */
public class SelectList {
	
	private Vector<SelectElement> SElems;
	private int groupbyCounter; // to create an AsExpression in case a groupby is needed
	
	public SelectList(SelectElement newElem) {
		SElems = new Vector<SelectElement>();
		SElems.add(newElem);
		this.groupbyCounter = 0;
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * Another SelectElement is added 
	 * @param newElem
	 * @return this SelectList itself
	 */
	public SelectList addElement(SelectElement newElem) {
		SElems.add(newElem);
		return this;
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * Checks if any of the SelectElements equals contains an attribut equal to Attribut
	 * an has no Qualifier. It then sets the qualifier Quali in this SelectElement
	 * @param Quali
	 * @param Attribut
	 * @return true if a qualifier could be set
	 */
	public boolean addQuali(String Quali, String Attribut) {
		boolean result = false;
		Iterator<SelectElement> it;
		SelectElement CurrentElement;
		
		it = this.SElems.iterator();
		while (it.hasNext() && !result) {
			CurrentElement = it.next();
			if (CurrentElement.checkName(Attribut))
				if (!CurrentElement.hasQuali()) {
					CurrentElement.setQuali(Quali);
					result = true;
				}
				else if (CurrentElement.checkQuali(Quali))
					result = true;
		}
		
		return result;
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * if select-elements use qualifiers 
	 * @param ql
	 * @return
	 */
	public boolean qualifierNeeded(QualifierList ql) {
		boolean result = false;
		Iterator<SelectElement> it;
		SelectElement CurrentElement;
		
		it = this.SElems.iterator();
		while (it.hasNext()) {
			CurrentElement = it.next();
			if (CurrentElement.hasQuali()) {
				ql.addQualifier(new Qualifier(CurrentElement.getAtt(), CurrentElement.getquali()));
				result = true; 
			}	
			/* In case the Select-Element has an alias just the ShadowQualifier can be set
			 * If the Qualifier is also set it can not be resolved because there is no qualified name
			 */
			if (CurrentElement.hasAsExp()) {
				ShadowQualifier sq = new ShadowQualifier(CurrentElement.getAtt(), CurrentElement.getquali());
				sq.setAsExpr(CurrentElement.getAsExpr());
				ql.addQualifierToShadow(sq);
				result = true;
			}
		}
		
 		return result;
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * All Select-Elements needs to be set with the Alias as Qualifier. The QualifierList does not need
	 * to be set, because it does not need to be resolved. The ShadowQualifier List however needs to be
	 * set. This method is needed in case the only table-element uses a qualifier which is not enforced or
	 * just enforced by one of the Select-Elements but there are more Select-Elements of this Table-Element. 
	 * @param Alias
	 * @param ql
	 */
	public void setQualifier(String Alias, QualifierList ql) {
		Iterator<SelectElement> it;
		SelectElement CurrentElement;
		
		it = this.SElems.iterator();
		while (it.hasNext()) {
			CurrentElement = it.next();
			if (!CurrentElement.getquali().equalsIgnoreCase(Alias) /*&& !CurrentElement.hasAsExp()*/) {
				CurrentElement.setQuali(Alias);
				ql.addQualifierToShadow(new Qualifier(CurrentElement.getAtt(), CurrentElement.getquali()));
			}				
		}
	}
	
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * outputs the SelectList as a string. It therefore uses the output function 
	 * of SelectElement getSElement()
	 * @return
	 */
	public String getSelectList() {
		String result="";
		boolean inSquareBrackets = false;
		Iterator<SelectElement> it;
		
		if (this.SElems.size() > 1){ 
			inSquareBrackets=true;
			result = "[";
		}
		it = this.SElems.iterator();
		result+=it.next().getSElement();
		
		while (it.hasNext())
			result+=", " + it.next().getSElement();
		if (inSquareBrackets)
			result += "]";
		
		return result;
		
	}
	
	/**
	 * 
	 * <b>Task of this method: </b> <br/> 
	 * it recognizes whether a group_by is needed and returns true if positive.
	 * if group_by has set it will automatically add an alias to the set_function_spec
	 * if it has none. 
	 * @param groupbySet
	 * @return
	 */
	public boolean groupbyNeeded(boolean groupbySet) {
		boolean result = false;
		Iterator<SelectElement> it;
		SelectElement CurrentElem;
		
		it = this.SElems.iterator();
		if (this.SElems.size() == 1) {
			CurrentElem = it.next();
			if (CurrentElem.hasSetFunc()) 
				if (CurrentElem.hasAsExp())
					result = true;  // for constructions like this: "select min(treal) as mintreal from typetest groupby tint"
				else if (groupbySet) {
					CurrentElem.setAsExpr("goupatt"+this.groupbyCounter++); // for constructions like this: "select min(treal) from typetest group by tint"
					result = true;
				}
		}
		else {
			while (it.hasNext()) {
				CurrentElem = it.next();
				if (CurrentElem.hasSetFunc()) { 
					result = true;
					if (!CurrentElem.hasAsExp())
						CurrentElem.setAsExpr("groupatt"+this.groupbyCounter++);
				}
			}
		}
		
		
		return result;
	}

}
