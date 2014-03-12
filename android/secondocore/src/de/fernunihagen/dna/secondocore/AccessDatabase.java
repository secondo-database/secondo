package de.fernunihagen.dna.secondocore;

import de.fernunihagen.dna.secondocore.itf.ISecondoDba;
import sj.lang.ListExpr;

/**
 * Dient dazu den Namen der aktuellen Datenbank abzufragen.
 */
public class AccessDatabase {

	/**
	 * Returns the name of the active Database in Secondo if there is any one
	 *
	 * @return a String with the name of the active Database
	 */
	public static String ActiveDatabase(ISecondoDba secondoDba) {
		
		// Der aktuelle Name der Datenbank wird abgefragt
		ListExpr liste=(ListExpr)secondoDba.querySync("query getDatabaseName()");
		ListOut lo=new ListOut();
		String dbname="";
		
		// Das Ergebnis ist eine Liste, diese Liste enthält nur ein Element vom Typ String
		if(liste!=null)  {
			lo.ListToStringArray(liste);
			if(lo.getElem(0).equals("string")) {
				dbname=lo.getElem(1);
			}
		}
		return dbname;
	}
}
