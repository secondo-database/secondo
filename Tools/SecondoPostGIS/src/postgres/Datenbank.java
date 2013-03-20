/**
 * 
 */
package postgres;

import java.util.LinkedHashMap;
import java.util.LinkedList;

/**
 * @author Bill
 *
 */
public class Datenbank {

	private StringBuffer sbName;
	

	public Datenbank()
	{
		sbName = new StringBuffer();
	}
	
	public StringBuffer getSbName() {
		return sbName;
	}


	public void setSbName(StringBuffer sbName) {
		this.sbName.delete(0, this.sbName.length());
		this.sbName.append(sbName);
	}
	
}
