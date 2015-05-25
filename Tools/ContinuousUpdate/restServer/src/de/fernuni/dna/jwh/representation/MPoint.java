package de.fernuni.dna.jwh.representation;

import java.util.Iterator;
import java.util.List;
import java.util.Vector;

/**
 * Local Representation of the Secondo-Type mpoints
 * @author Jerome White
 *
 */
public class MPoint extends NLRepresentation {

	public List<UPoint> points;
	
	public MPoint() {
		points = new Vector<UPoint>();
	}

	/**
	 * points holds a special Value (list of upoints)
	 * so we have to implement getSpecialType
	 */
	public String getSpecialType(String fieldName) {
		//No Switch-Statement, can only be points
		return "upoint";
	}

	/**
	 * points holds a special Value (list of upoints)
	 * so we have to implement getSpecialValue
	 */
	public String getSpecialValue(String fieldName) {
		//No Switch-Statement, can only be points

		// Get alle the Upoints and append them to the String-Builder
		// Each UPoint is enclosed in Brackets the returned String will
		// be enclosed once more by the calling method
		StringBuilder sb = new StringBuilder();
		for (Iterator<UPoint> iterator = points.iterator(); iterator.hasNext();) {
			UPoint uPoint = iterator.next();
			sb.append(uPoint.getSecondoValue());
		}
		return sb.toString();
	}

	// Returns the Secondo-Type of this Class 
	public static String getStaticType() {
		return "mpoint";
	};
}
