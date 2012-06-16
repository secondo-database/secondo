package Ext_Tools;

import java.util.Iterator;
import java.util.Vector;

/**
 * 
 * <b> Task of this class </b> <br/>
 * It represents a list of column names the selection is sorted by. The column names are 
 * represented by OrderElement.
 */
public class OrderClause {
	private Vector<OrderElement> OElems;
	private boolean Empty;
	
	public OrderClause(OrderElement NewElem) {
		OElems = new Vector<OrderElement>();
		OElems.add(NewElem);
		this.Empty = false;
	}
	
	public OrderClause(String Nothing) {
		this.Empty = true;
	}
	
	public boolean isEmpty() {
		return this.Empty;
	}
	
	public OrderClause addElement(OrderElement NewElem) {
		OElems.addElement(NewElem);
		return this;
	}
	
	public void CheckAndAddQuali(Qualifier QualiElem) {
		boolean result = false;
		Iterator<OrderElement> it;
		OrderElement CurrentElement;
		String quali = QualiElem.getQuali();
		String att = QualiElem.getAtt();
		
		it = this.OElems.iterator();
		while (it.hasNext() && !result) {
			CurrentElement = it.next();
			if (!CurrentElement.hasQuali() && CurrentElement.getAtt().equalsIgnoreCase(att))
				CurrentElement.setQuali(quali);
		}
	}
	
	// In case more Qualifiers have to be set
	public void SetAllOtherQualis(String Alias) {
		Iterator<OrderElement> it;
		OrderElement CurrentElement;
		
		it = this.OElems.iterator();
		while (it.hasNext()) {
			CurrentElement = it.next();
			if (!CurrentElement.hasQuali())
				CurrentElement.setQuali(Alias);
		}
	}
	
	public String getOrderClause() {
		String result="";
		boolean inSquareBrackets = false;
		Iterator<OrderElement> it;
		
		if (!this.Empty) {
			if (this.OElems.size() > 1){ 
				inSquareBrackets=true;
				result = "[";
			}
			it = this.OElems.iterator();
			result+=it.next().getGElement();
		
			while (it.hasNext())
				result+=", " + it.next().getGElement();
			if (inSquareBrackets)
				result += "]";
		}
		
		return result;
	}

}
