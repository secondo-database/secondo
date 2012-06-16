package Ext_Tools;

import java.util.*;

/**
 * 
 * <b> Task of this class </b> <br/>
 * represents a FROM-Clause which contains one TableElement or a comma separatet
 * list of TableElements
 */
public class TableClause {
	
	private Vector<TableElement> TElems;
	
	public TableClause(TableElement NewElem) {
		TElems = new Vector<TableElement>();
		TElems.add(NewElem);
	}
	
	public TableClause addElement(TableElement NewElem) {
		TElems.add(NewElem);
		return this;
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * checks whether any of the stored TableElements has got the passed qualifier or can be set
	 * with it. 
	 * @param quali
	 * @return true if a TableElement could be set with the qualifier or had had it before
	 */
	public boolean CheckOrAddQuali(String quali) {
		boolean result = false;
		Iterator<TableElement> it;
		TableElement CurrentElement;
		
		it = this.TElems.iterator();
		while (it.hasNext() && !result) {
			CurrentElement = it.next();
			result = CurrentElement.checkQuali(quali);
		}
		
		return result;
	}
	
	public String OneElement() {
		String result = "";
		if (this.TElems.size() == 1)
			result = this.TElems.get(0).getNickname();
			
		return result;			
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * outputs the TableClause in form of a string
	 * @return
	 */
	public String getTableList() {
		String result="";
		boolean inSquareBrackets = false;
		Iterator<TableElement> it;
		
		if (this.TElems.size() > 1){ 
			inSquareBrackets=true;
			result = "[";
		}
		it = this.TElems.iterator();
		result+=it.next().getTElement();
		
		while (it.hasNext())
			result+=", " + it.next().getTElement();
		if (inSquareBrackets)
			result += "]";
		
		return result;
		
	}
}
