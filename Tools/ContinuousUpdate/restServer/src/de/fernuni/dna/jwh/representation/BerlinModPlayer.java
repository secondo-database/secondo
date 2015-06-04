package de.fernuni.dna.jwh.representation;

import java.util.Date;


/**
 * Local Representation of the Secondo-Tuple tuple([Moid: string, Position: ipoint])
 * @author Jerome White
 *
 */
public class BerlinModPlayer extends NLRepresentation {
	public String Moid;
	public IPoint Position;
	
	public BerlinModPlayer() {
		Position = new IPoint();
	}
	
	@Override
	public String getSpecialType(String fieldName) {
		//No Switch-Statement, can only be Position
		return IPoint.getStaticType();
	}
	
	@Override
	public String getSpecialValue(String fieldName) {
		//No Switch-Statement, can only be Position
		return Position == null ? null : Position.getSecondoValue();
	}
}