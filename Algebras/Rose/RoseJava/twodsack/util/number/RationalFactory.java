/*
 * RationalFactory.java 2005-05-13
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */

package twodsack.util.number;

import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.*;


/**
 * The RationalFactory class is used for the construction of numeric values. Therefor, it conatins a set of methods with the name
 * constRational() with different parameter types. This class supports the use of different implementations of the Rational class. Before
 * a Rational instance can be constructed, the class that shall be used has to be defined using the {@link #setClass} method. After that,
 * this class can be used.<p>
 * Two extensions of the abstract Rational class are delivered with the 2DSACK package:<p>
 * <ul>
 * <li>RationalDouble
 * <li>RationalBigInteger
 * </ul>
 * Another method of this class can be used to set the PRECISE field. It defines wether a more or less precise computation is used. How
 * those two variants differ depends on the implementor of that class. The default value for PRECISE is true.
 */
public class RationalFactory {
    /*
     * fields
     */
    static private Class RATIONAL_CLASS = null;
    static private Boolean PRECISE = null;
    static private Constructor INTEGER_CONSTRUCTOR = null;
    static private Constructor DOUBLE_CONSTRUCTOR = null;
    static private Constructor RATIONAL_CONSTRUCTOR = null;
    static private Constructor NUM_DEN_CONSTRUCTOR = null;
    static private Object[] PARAM_VALUE_LIST_1 = new Object[1];
    static private Object[] PARAM_VALUE_LIST_2 = new Object[2];
    
    static public double DERIV_DOUBLE;
    static public double DERIV_DOUBLE_NEG;


    /*
     * methods
     */
    /**
     * Defines the Rational extension class.
     *
     * @param ratClass the Rational implementation that shall be used
     */
    static public void setClass(Class ratClass) {
	RATIONAL_CLASS = ratClass;
    }//end method setClass


    /**
     * Sets the PRECISE value.
     *
     * @param prec the value for PRECISE
     */
    static public void setPrecision(boolean prec) {
	PRECISE = new Boolean(prec);
	Class [] classParam = { PRECISE.getClass() };
	Rational r = constRational(0);
	Object[] methParam = { PRECISE };
	try {
	    Method m = RATIONAL_CLASS.getMethod("setPrecision", classParam);
	    m.invoke(r,methParam);
	}//try
	catch (Exception e) {
	    System.out.println("RationalFactory.setPrecision: Problems with method setPrecise.");
	    e.printStackTrace();
	}//catch
	
    }//end method setPrecision


    /**
     * Defines the Rational extension class.
     *
     * @param ratClass the name of the Rational class
     * @throws RationalClassNotExistentException if the class doesn't exist
     */
    static public void setClass(String ratClass) throws RationalClassNotExistentException {
	try {
	    RATIONAL_CLASS = Class.forName(ratClass);
	}//try
	catch (Exception e) { throw new RationalClassNotExistentException("Error in RationalFactory: Class file "+ratClass+" was not found.");
	}//catch
    }//end method setClass


    /**
     * Returns the Class of the currently used Rational implemention.
     *
     * @return the currently used Rational class
     */
    static public Class readClass() {
	return RATIONAL_CLASS;
    }//end method getClass


    /**
     * Returns the actual <i>deriv</i> value.
     * Note: If PRECISE = true, the derivation value is returned, otherwise NULL is returned.
     *
     * @return the deriv value as Rational
     * @throws NoDerivationValueFoundException if the derivation value was not implemented in the Rational class extension
     */
    static public Rational readDeriv() throws NoDerivationValueFoundException {
	if (PRECISE == null) {
	    System.out.println("WARNING: PRECISE should be set using RationalFactory.setPrecision()!\n...automatic definition to prevent program termination: PRECISE = TRUE");
	    PRECISE = new Boolean(true);
	}//if
	if (PRECISE.booleanValue()) {
	    Object res;
	    Rational obj = constRational(0);
	    try {
		Field fi = RATIONAL_CLASS.getDeclaredField("deriv");
		res = fi.get(obj);
		return (Rational)res;
	    }//try
	    catch (Exception e) { throw new NoDerivationValueFoundException("Error in RationalFactory: No derivation value was found. It must be implemented in chosen Rational class.");
	    }//catch
	}//if
	else { return null; }
    }//end method readDeriv


    /**
     * Returns the DERIV_DOUBLE value.
     * Additionally to the <i>deriv</i> value, DERIV_DOUBLE is defined which is a doulbe value. Computations using this value instead of
     * <i>deriv</i> are faster. Note, that this method needs a field DERIV_DOUBLE implemented in the Rational class extension.
     *
     * @return DERIV_DOUBLE as double
     * @throws RationalClassUndefinedException if the Rational class extension was not defined
     * @throws NoDerivationValueFoundException if DERIV_DOUBLE was not implemented
     */
    static public double readDerivDouble() throws RationalClassUndefinedException, NoDerivationValueFoundException {
	if (RATIONAL_CLASS == null) {
	    throw new RationalClassUndefinedException("Error in RationalFactory: RATIONAL_CLASS must be set using setClass().");
	}//if
	try {
	    Field fi = RATIONAL_CLASS.getDeclaredField("DERIV_DOUBLE");
	    Double obj = null;
	    DERIV_DOUBLE = ((Double)fi.get(obj)).doubleValue();
	} catch (Exception e) { throw new NoDerivationValueFoundException("Error in RationalFactory: No DERIV_DOUBLE value was found in RATIONAL_CLASS ("+RATIONAL_CLASS+")."); }
	return DERIV_DOUBLE;
    }//end method readDerivDouble


    /**
     * Returns the DERIV_DOULBE_NEG value.
     * Additionally to the <i>deriv</i> value, DERIV_DOUBLE_NEG is defined which is a negative double value. Computations using a double
     * value instead of a Rational value are faster. Note, that this method needs a field DERIV_DOUBLE_NEG implemented in the
     * Rational class extension.
     *
     * @return DERIV_DOUBLE_NEG as double
     * @throws RationalClassUndefinedException if the Rational class extension was not defined
     * @throws NoDerivationValuefoundException if DERIV_DOUBLE_NEG was not implemented
     */
    static public double readDerivDoubleNeg() {
	if (RATIONAL_CLASS == null) {
	    throw new RationalClassUndefinedException("Error in RationalFactory: RATIONAL_CLASS must be seut using setClass().");
	}//if
	try {
	    Field fi = RATIONAL_CLASS.getDeclaredField("DERIV_DOUBLE_NEG");
	    Double obj = null;
	    DERIV_DOUBLE_NEG = ((Double)fi.get(obj)).doubleValue();
	} catch (Exception e) { throw new NoDerivationValueFoundException("Error in RationalFactory: No DERIV_DOUBLE_NEG value was found in RATIONAL_CLASS ("+RATIONAL_CLASS+")."); }
	return DERIV_DOUBLE_NEG;
    }//end method readDerivDoubleNeg


    /**
     * Returns the PRECISE value.
     *
     * @return the PRECISE value
     */
    static public boolean readPrecise() {
	if (PRECISE == null) {
	    System.out.println("WARNING: PRECISE should be set using RationalFactory.setPrecision()!\n...automatic definition to prevent program termination: PRECISE = TRUE");
	    PRECISE = new Boolean(true);
	}//if
	return PRECISE.booleanValue();
    }//end method readPrecise


    /**
     * Constructs a new Rational instance with an int.
     *
     * @param i the int value
     * @return the new Rational instance
     * @throws RationalClassUndefinedException if the Rational class extension was not defined
     * @throws RationalClassConstructorNotFoundException if the constructor for int values was not implemented
     */
    static public Rational constRational(int i) throws RationalClassUndefinedException, RationalClassConstructorNotFoundException {
	if (RATIONAL_CLASS == null) {
	    throw new RationalClassUndefinedException("Error in RationalFactory: RATIONAL_CLASS must be set using setClass()");
	}//if
	else if (INTEGER_CONSTRUCTOR != null) {
	    PARAM_VALUE_LIST_1[0] = new Integer(i);
	    Rational rat = null;
	    try {
		rat = (Rational)INTEGER_CONSTRUCTOR.newInstance(PARAM_VALUE_LIST_1);
	    }//try
	    catch (Exception e) {
		System.out.println("Error in RationalFactory.constRational(integer): Couldn't create a new instance for integer["+i+"].");
		e.printStackTrace();
		System.exit(0);
	    }//catch
	    return rat;
	}//else if

	else {
	    Class[] paramClassList = { int.class };
	    Object[] paramValueList = { new Integer(i) };
	    Rational rat = null;
	    try {
		INTEGER_CONSTRUCTOR = RATIONAL_CLASS.getConstructor(paramClassList);
		rat = (Rational)INTEGER_CONSTRUCTOR.newInstance(paramValueList);
	    }//try
	    catch (Exception e) { throw new RationalClassConstructorNotFoundException("Error in RationalFactory: constructor "+RATIONAL_CLASS+"(int) not found"); }
	    return rat;
	}//else
	    
    }//end method constRational


    /**
     * Constructs a new Rational instance with a double.
     *
     * @param d the double value
     * @return the new Rational instance
     * @throws RationalClassUndefinedException if the Rational class extension was not defined
     * @throws RationalClassConstructorNotFoundException if the constructor for double values was not implemented
     */
    static public Rational constRational(double d) throws RationalClassUndefinedException, RationalClassConstructorNotFoundException {
	if (RATIONAL_CLASS == null) {
	    throw new RationalClassUndefinedException("Error in RationalFactory: RATIONAL_CLASS must be set using setClass()");
	}//if
	else if (DOUBLE_CONSTRUCTOR != null) {
	    PARAM_VALUE_LIST_1[0] = new Double(d);
	    Rational rat = null;
	    try {
		rat = (Rational)DOUBLE_CONSTRUCTOR.newInstance(PARAM_VALUE_LIST_1);
	    }//try
	    catch (Exception e) {
		System.out.println("Error in RationalFactory.constRational(double): Couldn't create a new instance for double["+d+"].");
		e.printStackTrace();
		System.exit(0);
	    }//catch
	    return rat;
	}//else if

	else {
	    Class[] paramClassList = { double.class };
	    Object[] paramValueList = { new Double(d) };
	    Rational rat = null;
	    try {
		DOUBLE_CONSTRUCTOR = RATIONAL_CLASS.getConstructor(paramClassList);
		rat = (Rational)DOUBLE_CONSTRUCTOR.newInstance(paramValueList);
	    }//try
	    catch (Exception e) { throw new RationalClassConstructorNotFoundException("Error in RationalFactory: constructor "+RATIONAL_CLASS+"(double) not found"); }
	    return rat;
	}//else
    }//end method constRational

    
    /**
     * Constructs a new Rational instance with a Rational.
     *
     * @param r the Rational value
     * @return the new Rational instance
     * @throws RationalClassUndefinedException if the Rational class extension was not defined
     * @throws RationalClassConstructorNotFoundException if the constructor for Rational values was not implemented
     */
    static public Rational constRational(Rational r) throws RationalClassUndefinedException, RationalClassConstructorNotFoundException {
	if (RATIONAL_CLASS == null) {
	    throw new RationalClassUndefinedException("Error in RationalFactory: RATIONAL_CLASS must be set using setClass()");
	}//if
	else if (RATIONAL_CONSTRUCTOR != null) {
	    PARAM_VALUE_LIST_1[0]= r;
	    Rational rat = null;
	    try {
		rat = (Rational)RATIONAL_CONSTRUCTOR.newInstance(PARAM_VALUE_LIST_1);
	    }//try
	    catch (Exception e) {
		System.out.println("Error in RationalFactory.constRational(Rational): Couldn't create a new instance for Rational["+r+"].");
		e.printStackTrace();
		System.exit(0);
	    }//catch
	    return rat;
	}//else if

	else {
	    Class[] paramClassList = { Rational.class };
	    Object[] paramValueList = { r };
	    Rational rat;
	    try {
		RATIONAL_CONSTRUCTOR = RATIONAL_CLASS.getConstructor(paramClassList);
		rat = (Rational)RATIONAL_CONSTRUCTOR.newInstance(paramValueList);
	    }//try
	    catch (Exception e) { throw new RationalClassConstructorNotFoundException("Error in RationalFactory: constructor "+RATIONAL_CLASS+"(Rational) not found"); }
	    return rat;
	}//else
    }//end method constRational


    /**
     * Constructs a new Rational instance with two int values.
     *
     * @param num the numerator
     * @param den the denominator
     * @return the new Rational instance
     * @throws RationalClassUndefinedException if the Rational class extension was not defined
     * @throws RationalClassConstructorNotFoundException if the constructor for two int values was not implemented
     */    
    static public Rational constRational(int num, int den) throws RationalClassUndefinedException, RationalClassConstructorNotFoundException {/*
 * Rational.java 2005-05-13
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 */
	if (RATIONAL_CLASS == null) {
	    throw new RationalClassUndefinedException("Error in RationalFactory: RATIONAL_CLASS must be set using setClass()");
	}//if
	
	else if (NUM_DEN_CONSTRUCTOR != null) {
	    PARAM_VALUE_LIST_2[0] = new Integer(num);
	    PARAM_VALUE_LIST_2[1] = new Integer(den);
	    Rational rat = null;
	    try {
		rat = (Rational)NUM_DEN_CONSTRUCTOR.newInstance(PARAM_VALUE_LIST_2);
	    }//try
	    catch (Exception e) {
		System.out.println("Error in RationalFactory.constRational(int,int): Couldn't create a new instance for int x int ["+num+", "+den+"].");
		e.printStackTrace();
		System.exit(0);
	    }//catch 
	    return rat;
	}//else if

	else {
	    Class[] paramClassList = { int.class, int.class };
	    Object[] paramValueList = { new Integer(num), new Integer(den) };
	    Rational rat;
	    try {
		NUM_DEN_CONSTRUCTOR = RATIONAL_CLASS.getConstructor(paramClassList);
		rat = (Rational)NUM_DEN_CONSTRUCTOR.newInstance(paramValueList);
	    }//try
	    catch (Exception e) { throw new RationalClassConstructorNotFoundException("Error in RationalFactory: constructor "+RATIONAL_CLASS+"(int int) not found"); }
	    return rat;
	}//else
    }//end method constRational


}//end class RationalFactory
