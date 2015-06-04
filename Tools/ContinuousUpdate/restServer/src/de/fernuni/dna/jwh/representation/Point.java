package de.fernuni.dna.jwh.representation;

import java.util.Date;

import de.fernuni.dna.jwh.secondo.tools.DateTime;

/**
 * Local Representation of the Secondo-Type Point
 * 
 * @author Jerome White
 *
 */
public class Point extends NLRepresentation {
	public Double x;
	public Double y;

	public Point() {
	}
	
	@Override
	public String toString() {
		return "(" + x + " " + y + ")";
	}

	@Override
	public String getSecondoType() {
		return Point.getStaticType();
	}

	// Returns the Secondo-Type of this Class
	public static String getStaticType() {
		return "point";
	}
}
