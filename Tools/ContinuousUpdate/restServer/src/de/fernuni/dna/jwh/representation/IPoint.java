package de.fernuni.dna.jwh.representation;

import java.util.Date;

import de.fernuni.dna.jwh.secondo.tools.DateTime;

/**
 * Local Representation of the Secondo-Type IPoint
 * 
 * @author Jerome White
 *
 */
public class IPoint extends NLRepresentation {
	public Date instant;
	public Double x;
	public Double y;

	public IPoint() {
		instant = new Date();
	}
	
	@Override
	public String toString() {
		return "(" + DateTime.getDateTime(instant) + " (" + x + " " + y + "))";
	}

	@Override
	public String getSecondoType() {
		return IPoint.getStaticType();
	}

	// Returns the Secondo-Type of this Class
	public static String getStaticType() {
		return "ipoint";
	}
}
