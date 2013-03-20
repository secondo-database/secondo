/**
 * 
 */
package postgres;

import java.util.LinkedHashMap;

/**
 * @author Bill
 *
 */
public class PostgresRel {

	LinkedHashMap<StringBuffer, Datenbank> lhmDatenbank;
	
	public PostgresRel()
	{
		lhmDatenbank = new LinkedHashMap<StringBuffer, Datenbank>();
		
	}
}
