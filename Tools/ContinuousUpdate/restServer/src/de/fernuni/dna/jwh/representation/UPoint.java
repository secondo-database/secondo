package de.fernuni.dna.jwh.representation;

/**
 * Local Representation of the Secondo-Type upoint
 * @author Jerome White
 *
 */
public class UPoint extends NLRepresentation {
	public TimeInterval interval;
	public Double x1;
	public Double x2;
	public Double y1;
	public Double y2;
	
	public UPoint() {
		interval = new TimeInterval();
	}

	@Override
	public String toString() {
		return "(" + interval.getSecondoValue() + "(" + x1 + " " + y1 + " " + x2 + " " + y2 + "))";
	}
	
	/**
	 * interval holds a special Value
	 * so we have to implement getSpecialType
	 */
	@Override
	public String getSpecialType(String fieldName) {
		//No Switch-Statement, can only be interval
		return TimeInterval.getStaticType();
	}
	
	/**
	 * interval holds a special Value
	 * so we have to implement getSpecialValue
	 */
	@Override
	public String getSpecialValue(String fieldName) {
		//No Switch-Statement, can only be interval
		return interval == null ? null : interval.getSecondoValue();
	}
	
	@Override
	public String getSecondoType() {
		return UPoint.getStaticType();
	}

	// Returns the Secondo-Type of this Class 
	public static String getStaticType() {
		return "upoint";
	}
}
