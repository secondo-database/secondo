/**
 * 
 */
package util;

import java.util.Properties;
import java.util.Collections;
import java.util.Enumeration;
import java.util.Vector;
/**
 * @author Bill
 *http://www.rgagnon.com/javadetails/java-0614.html
 */
public class SortedProperties extends Properties {
	  /**
	   * Overrides, called by the store method.
	   */
	  @SuppressWarnings("unchecked")
	  public synchronized Enumeration keys() {
	     Enumeration keysEnum = super.keys();
	     Vector keyList = new Vector();
	     while(keysEnum.hasMoreElements()){
	       keyList.add(keysEnum.nextElement());
	     }
	     Collections.sort(keyList);
	     return keyList.elements();
	  }
	  
	
	}
