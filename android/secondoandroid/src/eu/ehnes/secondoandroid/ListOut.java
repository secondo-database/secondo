package eu.ehnes.secondoandroid;

import java.util.ArrayList;
import java.util.List;

import sj.lang.ListExpr;

/**
 * ListOut - Klasse um eine ListExpr in Java auszugeben.
 *
 */
public class ListOut {
	private List<String> al;
	private int aktElem=0;
	
	/**
	 * Konstruktor
	 */
	
	ListOut() {
		al=null;
	}
	
	/**
	 * Getter & Setter
	 */
	public List<String> getArrayListString() {
		return al;
	}
	public void setArrayListString(List<String> al) {
		this.al=al;
	}
	
	/*
	 * Größe des Arrays ermitteln
	 * 
	 * @return int größe des Arrays
	 */
	public int size() {
		if(al!=null)
			return al.size();
		else
			return 0;
	}
	
	/**
	 * Gibt Element mit angegebener Nummer zurück.
	 * 
	 * @param num
	 * @return String des angefragten Elements
	 */
	public String getElem(int num) {
		if(num<size()) {
			return al.get(num);
		} else {
			return "";
		}
				
	}
	
	/**
	 * Gibt das nächste Element in der Liste zurück.
	 * Dient zum linearen Abarbeiten der Liste
	 *  
	 * @return String das nächste Element in der Liste
	 */
	public String nextElem() {
		String result=al.get(aktElem);
		aktElem=(aktElem+1)%al.size();
		return result;
	}
	public String firstElem() {
		aktElem=0;
		return nextElem();
	}
	
	/**
	 *  ListToStringArray Hauptfunktion
	 * Git ein Array zurück in dem alle Texte linear angeordnet sind.
	 * 
	 * @return Eine ArrayList mit Strings der Liste (flache Struktur)
	 */
	public List<String> ListToStringArray(ListExpr li) {
		
		if(al==null)
			al=new ArrayList<String>();
		else
			al.clear();
		
		LTSA(li);
		return al;
	}
	
	/**
	 * Diese Funktion wird rekursiv aufgerufen um die Liste umzuwandeln.
	 * 
	 * @param li Liste mit Elementen, die umgewandelt werden sollen.
	 * @return boolean Ergebns ob Umwandlung geklappt hat oder nicht.
	 */
	private boolean LTSA(ListExpr li) {
		if(li.isEmpty()) {
			return false;
		} else if(li.isAtom()) {
			switch(li.atomType()) {
			case ListExpr.BIN_TEXT: case ListExpr.BIN_STRING: case ListExpr.BIN_SHORTSTRING: case ListExpr.BIN_SHORTTEXT: case ListExpr.BIN_LONGTEXT: case ListExpr.BIN_LONGSTRING:
				al.add(li.textValue());
				break;
			case ListExpr.BIN_DOUBLE: case ListExpr.BIN_REAL:
				al.add(String.valueOf(li.realValue()));
				break;
			case ListExpr.BIN_SYMBOL: case ListExpr.BIN_SHORTSYMBOL: case ListExpr.BIN_LONGSYMBOL:
				al.add(li.symbolValue());
				break;
			case ListExpr.BIN_INTEGER: case ListExpr.BIN_SHORTINT:
				al.add(String.valueOf(li.intValue()));
				break;
			case ListExpr.BIN_BYTE:
				break;
			case ListExpr.BIN_BOOLEAN:
				al.add(String.valueOf(li.boolValue()));
				
				break;
			default:
				break;

			}
		} else {
			    boolean ok = LTSA(li.first());

			    if ( !ok ) {
			      return (ok);
			    }

			    while ( !li.rest().isEmpty())
			    {
			      li = li.rest();
			      ok = LTSA(li.first() );

			      if ( !ok ) {
			        return (ok);
			      }
			    }
		}				
		return true;
	}
}
