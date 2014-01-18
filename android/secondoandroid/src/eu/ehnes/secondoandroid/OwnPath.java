package eu.ehnes.secondoandroid;

import android.app.Activity;

/** 
 * OnwPath
 * Diese Klasse hält den aktuellen Programmpfad als statische Variable
 * 
 * @author Jürgen Ehnes
 * @version 1.0
 */

public class OwnPath extends Activity {
	private  static String path=null;
	private static OwnPath ownpath;
	
	private OwnPath() {}
	
	public static synchronized OwnPath getInstance() {
		if(ownpath==null) {
			ownpath=new OwnPath();
		}
		return ownpath;
	}
	// Getter
	public String getPath() {
		return path;
	}
	
	// Setter
	public void setPath(String newpath) {
		path=newpath;
	}
}
