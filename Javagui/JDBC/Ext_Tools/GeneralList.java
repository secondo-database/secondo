package Ext_Tools;

import java.util.Vector;
import java.util.Iterator;


/**
 * 
 * <b> Task of this class </b> <br/>
 * can receive a lists of elements and outputs it comma separated or space separated
 */
public class GeneralList {
	
	private Vector<String> gList;
	
	public GeneralList(String newElem) {
		gList = new Vector<String>();
		gList.add(newElem);
	}
	
	public GeneralList addElement(String newElem) {
		this.gList.add(newElem);
		return this;
	}
	
	public String getGList() {
		String result = "";
		Iterator<String> it;
		
		it = this.gList.iterator();
		if (it.hasNext())
			result = it.next();
		while (it.hasNext())
			result += ", " + it.next();
		if (this.gList.size() > 1)
			result = "[" + result + "]";
		
		return result;	
	}
	
	public String getGListWithoutComma() {
		String result = "";
		Iterator<String> it;
		
		it = this.gList.iterator();
		if (it.hasNext())
			result = it.next();
		while (it.hasNext())
			result = it.next() + " " + result;
		
		return result;	
	}
}
