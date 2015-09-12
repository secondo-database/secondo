package de.fernuni_hagen.dna.jwh.secondopositiontransmitter.representation;

import java.util.Date;

/**
 * Local Representation of the Secondo-Type timeinterval
 * @author Jerome White
 *
 */
public class TimeInterval extends NLRepresentation {
	public Date i1;
	public Date i2;
	public Boolean j1closed;
	public Boolean j2closed;
	
	public TimeInterval() {
	}

	@Override
	public String getSecondoType() {
		return TimeInterval.getStaticType();
	}

	public static String getStaticType() {
		return "timeinterval";
	}
	
	@Override
	public boolean isValid() {
		if(i1.before(i2) && j1closed != null && j2closed != null){
			return true;
		}else{
			return false;
		}
	}
}
