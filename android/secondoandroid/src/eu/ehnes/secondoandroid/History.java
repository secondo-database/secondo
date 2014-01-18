package eu.ehnes.secondoandroid;

import java.util.ArrayList;
import java.util.List;


// TODO: Auto-generated Javadoc
/**
 * Diese Klasse implementiert eine Historyfunktion für das Eingabefeld 
 * Es ist als Ringspeicher mit maximal 10 Elementen geschrieben.
 * Die maximale Anzahl an Elementen ist in der Variable maxHistory zu finden.
 * 
 * @author juergen
 */
public class History {

	/** The ring speicher. */
	private static List<String> ringSpeicher=null;

	/** The Constant maxHistory. */
	private final static int maxHistory=30;

	/** The akt pos. */
	private static int aktPos;

	/** The history. */
	private static History history;


	/**
	 * Instantiates a new history.
	 */
	private History() { }

	/**
	 * Gets the single instance of History.
	 *
	 * @return single instance of History
	 */
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
		}
		return 0;
	}

	/**
	 * Adds the string to the ringmemory
	 *
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
	 * 
	 * @return Zuletzt eingetragenes Element.  
	 */
	public String last() {
		if(size()>0) {
			String result=ringSpeicher.get(aktPos);
			if(--aktPos<0) aktPos=size()-1;
			return result;
		}
		return "";

	}

	/**
	 * Returns the next string
	 *
	 * @return the string
	 */
	public String next() {
		if(size()>0) {
			if(++aktPos>=size()) aktPos=0;
			String result=ringSpeicher.get(aktPos);
			return result;
		}
		return "";
	}

}
