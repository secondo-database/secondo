package Ext_Tools;

import java.util.*;

/**
 * 
 * <b>Task of this class: </b> <br/>
 *	contains a list of Qualifier-Objects
 */
public class QualifierList {
	
	private Vector<Qualifier> QElems;
	private Vector<ShadowQualifier> ShadowList;
	
	public QualifierList() {
		this.QElems = new Vector<Qualifier>();
		this.ShadowList = new Vector<ShadowQualifier>();  // for the passed answer
	}
	
	/**
	 * 
	 * <b>Task of this method: </b> <br/> 
	 * adds a Qualifier-Element if it is not already part of the list
	 * @param newElem
	 * @return true if the passed Qualifier-Object could be added
	 */
	public boolean addQualifier(Qualifier newElem) {
		boolean result = true;
		Iterator<Qualifier> it;
		
		it = QElems.iterator();
		while (it.hasNext() && result) 
			result = !it.next().equals(newElem);
		if (result)
			QElems.add(newElem);
		
		this.addQualifierToShadow(newElem);
		
		return result;
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * The new Qualifier also needs to be added to the ShadowList
	 * @param newElem
	 */
	public void addQualifierToShadow(Qualifier newElem) {
		boolean ShadowResult = true;
		Iterator<ShadowQualifier> it;
		ShadowQualifier CurrentElem = null;
		
		it = this.ShadowList.iterator();
		while(it.hasNext() && ShadowResult) {
			CurrentElem = it.next();
			ShadowResult = !CurrentElem.equals(newElem);
		}
		if(ShadowResult) {
			if (newElem instanceof ShadowQualifier) // in case newElem is a ShadowQualifier a cast is needed so 
													// the alias is transferred too
				this.ShadowList.add((ShadowQualifier) newElem);
			else
				this.ShadowList.add(new ShadowQualifier(newElem)); // in case a Qualifier Instance is passed
		}
		else if (CurrentElem != null){		// in case the current element has an AsExpr and is already stored in the Qualifier List
			if (CurrentElem.getAsExpr()== "")
				if (newElem instanceof ShadowQualifier)
					CurrentElem.setAsExpr(((ShadowQualifier) newElem).getAsExpr());
		}
	}
	
	public boolean isEmpty() {
		return this.QElems.isEmpty();
	}
	
	public boolean isShadowEmpty() {
		return this.ShadowList.isEmpty();
	}
	
	/**
	 * 
	 * <b>Task of this method: </b> <br/> 
	 * The Qualifier-Objects of the passed QualifierList are added to this QualifierList.
	 * @param ql
	 * @return this QualifierList
	 */
	public QualifierList addQualifierList(QualifierList ql) {
		
		if (ql != null && !ql.isEmpty()) {
			Iterator<Qualifier> it = ql.getIterator();
			while (it.hasNext())
				this.addQualifier(it.next());
		}
		
		// in case there are Elements in the Shadowlist which are not in the QualifierList
		this.mergeShadows(ql);
		
		return this;
				
	}
	
	/**
	 * 
	 * <b> Task of this method </b> <br/>
	 * The list of ShadowQualifiers transferred by the QualifierList is added to this list
	 * of ShadowQualifiers
	 * @param ql
	 */
	public void mergeShadows(QualifierList ql) {
		
		if (ql != null && !ql.isShadowEmpty()) {
			Iterator<ShadowQualifier> it = ql.getShadowIterator();
			while (it.hasNext())
				this.addQualifierToShadow(it.next());
		}
	}
	
	public Iterator<Qualifier> getIterator() {
		return this.QElems.iterator();
	}
	
	public Iterator<ShadowQualifier> getShadowIterator() {
		return this.ShadowList.iterator();
	}
	
	public Vector<ShadowQualifier> getShadowList() {
		return this.ShadowList;
	}
	
}
