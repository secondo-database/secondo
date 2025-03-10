package de.fernuni_hagen.dna.jwh.secondopositiontransmitter.representation;


/**
 * TODO
 * @author Jerome White
 *
 */
public class Position extends NLRepresentation {
	@Order(pos = 0)
	public String Id;
	@Order(pos = 1)
	public UPoint Position;
	
	public Position() {
		Position = new UPoint();
	}
	
	@Override
	public String getSpecialType(String fieldName) {
		//No Switch-Statement, can only be Position
		return UPoint.getStaticType();
	}
	
	@Override
	public String getSpecialValue(String fieldName) {
		//No Switch-Statement, can only be Position
		return Position == null ? null : Position.getSecondoValue();
	}
}
