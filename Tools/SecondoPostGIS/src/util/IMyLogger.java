/**
 * 
 */
package util;

import java.io.IOException;


/**
 * @author Bill
 *
 */
public interface IMyLogger {

	//Variablen eines Interfaces sind automatisch final
	
	int miMaxSize = 104857600; //Angabe in Bytes hier dann 100MB 
	int miCountRotate = 10; //Anzahl der Dateien die hier mit angeleget werden
	String mstrLogDatei = System.getProperty("user.home")+ OSValidator.getDateipfadZeichen() + ".Hjort"+ OSValidator.getDateipfadZeichen() + ".hjort.log";
	boolean mbLogAppend = true;
	
	public void initLogger(StringBuffer _strClassName, boolean _bLogConsole) throws SecurityException, IOException;
	
}
