//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science,
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

package util.common;

/**
 * Convenient methods to handle strings
 * @author D.Merle
 */
public class StringUtilities {

	/**
	 * Privater Konstruktor, der den Default Konstruktor überschreibt.
	 */
	private StringUtilities() {}

	/**
	 * Prüfung auf null und "" String.
	 * @param string Der zu prüfende String
	 * @return true - falls der zu prüfende String null oder "" ist
	 */
	public static boolean isStringNull(final String string) {
		return string == null || "".equals(string);
	}

	/**
	 * Verknüpft eine beliebige Anzahl von Strings, die sich als Parameter an die
	 * Methode übergeben lassen.
	 * @param strings Die zu verknüpfenden Strings
	 * @return Das zusammengefügte Ergebnis
	 */
	public static String appendStrings(final String ... strings) {
		final StringBuilder resultingString = new StringBuilder();
		for (final String string : strings) {
			resultingString.append(string).append("\n");
		}
		return resultingString.toString();
	}
}