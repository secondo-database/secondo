/*
 * Mercator.java 2004-11-29
 *
 * Thomas Behr, FernUniversitaet Hagen
 * adapted by Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.io;

/**
 * This class provides methods for coordinate transformation. Of the huge amount of different
 * transformations for geographical data, this class provides methods for the Mercator transformation.
 */
public class Mercator {

    /*
     * fields
     */
    static final double LOG_E = Math.log(Math.E);
    static final double PI = Math.PI;
    
    /*
     * methods
     */
    /**
     * Computes the projection of the x coordinate.
     *
     * @param lambda lambda value
     * @param phi phi value
     */
    public static double getPrjX(double lambda, double phi) {
	return lambda-Lambda_0;
    }//end method getPrjX
    
    /**
     * Computes the projection of the y coordinate.
     *
     * @param lambda lambda value
     * @param phi phi value
     */
    public static double getPrjY(double lambda, double phi) throws InvalidInputException{
	if(phi>90 || phi<-90)
	    throw new InvalidInputException("phi out of range");
	if( phi<=(-90+secure_distance))
	    phi = -90+secure_distance;
	if( phi>=(90-secure_distance))
	    phi = 90-secure_distance;
	
	double phi_2 = phi*PI/180;
	double y =   Math.log(Math.tan(PI/4 + phi_2/2))/LOG_E;
	return y*180/PI;
    }//end method getPrjY
    
    private static double Lambda_0 = 0;
    private static double secure_distance = 1;
    
}//end class Mercator
