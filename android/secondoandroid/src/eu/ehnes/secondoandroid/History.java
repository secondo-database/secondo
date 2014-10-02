
package eu.ehnes.secondoandroid;

import java.util.ArrayList;
import java.util.List;


/**
 * History
 * Diese Klasse implementiert eine Historyfunktion für das Eingabefeld 
 * Es ist als Ringspeicher mit maximal 10 Elementen geschrieben.
 * Die maximale Anzahl an Elementen ist in der Variable maxHistory zu finden.
 * 
 * 
 */
public class History {
	private static List<String> ringSpeicher=null;
	private final static int maxHistory=30;
	private static int aktPos;
	private static History history;
	
	// Konstruktor
	private History() { }
	
	public static synchronized History getInstance() {
		
		if(history == null) {
			ringSpeicher=new ArrayList<String>();
			aktPos=0;
			history=new History();
		}
		return history;
		
	}
	
	/**
	 * Gibt die Größe des Ringspeichers oder 0 zurück.
	 * 
	 * @return int Größe des Ringspeichers
	 */
	private int size() {
		if(ringSpeicher!=null) {
			return ringSpeicher.size();
		} else {
			return 0;
		}
	}
	/**
	 * @param s String der in den Ringspeicher eingetragen wird.
	 */
	public void add(String s) {
		ringSpeicher.add(s);
		if(size()>=maxHistory) {
			ringSpeicher.remove(0);
		}
		aktPos=size()-1;

	}
	/**
	 * Gibt das letzte eingetragene Element in der Liste zurück.
	 * @return Zuletzt eingetragenes Element.  
	 */
	public String last() {
		if(size()>0) {
			String result=ringSpeicher.get(aktPos);
			if(--aktPos<0) aktPos=size()-1;
			return result;
		}else{
			return "";
		}
	
	}
	public String next() {
		if(size()>0) {
			if(++aktPos>=size()) aktPos=0;
			String result=ringSpeicher.get(aktPos);
			return result;
		}
		else{
			return "";
		}
	}
	
}
