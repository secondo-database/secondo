package Ext_Tools;

import java.util.Iterator;
import java.util.Vector;

/**
 * 
 * <b> Task of this class </b> <br/>
 * It represents the group part of a selection query and consists of a list
 * of column references (which represented by groupElement)
 */
public class GroupClause {
	private Vector<GroupElement> GElems;
	private boolean Empty;
	
	public GroupClause(GroupElement NewElem) {
		GElems = new Vector<GroupElement>();
		GElems.add(NewElem);
		this.Empty = false;
	}
	
	public GroupClause(String Nothing) {
		this.Empty = true;
	}
	
	public boolean isEmpty() {
		return this.Empty;
	}
	
	public GroupClause addElement(GroupElement NewElem) {
		GElems.addElement(NewElem);
		return this;
	}
	
	public void CheckAndAddQuali(Qualifier QualiElem) {
		boolean result = false;
		Iterator<GroupElement> it;
		GroupElement CurrentElement;
		String quali = QualiElem.getQuali();
		String att = QualiElem.getAtt();
		
		it = this.GElems.iterator();
		while (it.hasNext() && !result) {
			CurrentElement = it.next();
			if (!CurrentElement.hasQuali() && CurrentElement.getAtt().equalsIgnoreCase(att))
				CurrentElement.setQuali(quali);
		}
	}
	
	public void SetAllOtherQualis(String Alias) {
		Iterator<GroupElement> it;
		GroupElement CurrentElement;
		
		it = this.GElems.iterator();
		while (it.hasNext()) {
			CurrentElement = it.next();
			if (!CurrentElement.hasQuali())
				CurrentElement.setQuali(Alias);
		}
	}
	
	public String getGroupClause() {
		String result="";
		boolean inSquareBrackets = false;
		Iterator<GroupElement> it;
		
		if (!this.Empty) {
			if (this.GElems.size() > 1){ 
				inSquareBrackets=true;
				result = "[";
			}
			it = this.GElems.iterator();
			result+=it.next().getGElement();
		
			while (it.hasNext())
				result+=", " + it.next().getGElement();
			if (inSquareBrackets)
				result += "]";
		}
		
		return result;
	}

}
