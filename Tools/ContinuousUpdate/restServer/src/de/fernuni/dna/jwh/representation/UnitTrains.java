package de.fernuni.dna.jwh.representation;

public class UnitTrains extends NLRepresentation {
	public Integer Id;
	public Integer Line;
	public Boolean Up;
	public UPoint UTrip;
	public Integer No;
	
	public UnitTrains() {
		UTrip = new UPoint();
	}
	
	@Override
	public String getSpecialType(String fieldName) {
		//No Switch-Statement, can only be UTrip
		return UPoint.getStaticType();
	}
	
	@Override
	public String getSpecialValue(String fieldName) {
		return UTrip == null ? null : UTrip.getSecondoValue();
	}
}