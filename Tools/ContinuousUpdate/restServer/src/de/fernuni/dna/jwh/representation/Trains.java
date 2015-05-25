package de.fernuni.dna.jwh.representation;

/**
 * Local Representation of the Secondo-Tuple berlintest.Trains
 * @author Jerome White
 *
 */
public class Trains extends NLRepresentation {
	public Integer Id;
	public Integer Line;
	public Boolean Up;
	public MPoint Trip;
	
	public Trains() {
		Trip = new MPoint();
	}

	/**
	 * Trip holds a special Value
	 * so we have to implement getSpecialType
	 */
	@Override
	public String getSpecialType(String fieldName) {
		//No Switch-Statement, can only be Trip
		return MPoint.getStaticType();
	}	
	
	/**
	 * Trip holds a special Value
	 * so we have to implement getSpecialValue
	 */
	@Override
	public String getSpecialValue(String fieldName) {
		//No Switch-Statement, can only be Trip
		return Trip == null ? null : Trip.getSecondoValue();
	}
}