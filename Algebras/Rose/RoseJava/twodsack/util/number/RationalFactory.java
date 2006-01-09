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
 * The RationalFactory class is used for the construction of numeric values.
 * Therefore, it conatins a set of methods with the name
 * <tt>constRational()</tt> with different parameter types. This class supports the use of different implementations of the {@link Rational} class. Before
 * a Rational instance can be constructed, the class that shall be used has to be defined using the {@link #setClass(Class)} method. After that,
 * this class can be used.<p>
 * Two extensions of the abstract Rational class are delivered with the 2DSACK package:<p>
 * <ul>
 * <li>{@link RationalDouble}
 * <li>{@link RationalBigInteger}
 * </ul>
 * Another method of this class can be used to set the <tt>PRECISE</tt> field. It defines wether a more or less precise computation is used.
 * The default value for <tt>PRECISE</tt> is <tt>true</tt>. If <tt>true</tt>,
 * for equality checks between Rationals a number <tt>deviation</tt> is used. Two Rationals <tt>r,s</tt> are supposed to be equal, if
 * <tt>-deviation < r - s < deviation</tt>. <tt>deviation</tt> must be a field of the Rational class.<p>
 * If <tt>PRESICE = false</tt> the same mechanism is used, but the computation is done by using doubles, which is faster but less precise.
 * The two fields <tt>DEVIATION_DOUBLE</tt> and <tt>DEVIATIION_DOUBLE_NEG</tt> are implemented in the Rational class, too. All three deviation values are
 * read from the actual Rational implemention and can be read from this class, then.
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
    
    /**
     * A deviation value used for precise computations.
     */
    static public Rational deviation;


    /**
     * A deviation value used for non-precise computations.
     */
    static public double DEVIATION_DOUBLE;

    /**
     * A deviation value used for non-precise computations. Should be set to <tt>-1*DEVIATION_DOUBLE</tt>.
     */
    static public double DEVIATION_DOUBLE_NEG;


    /*
     * constructors
     */
    /**
     * Don't use this constructor.
     */
    private RationalFactory(){}

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
     * Sets the <tt>PRECISE</tt> value.
     *
     * @param prec the value for <tt>PRECISE</tt>
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
     * Returns the class of the currently used Rational implemention.
     *
     * @return the currently used Rational class
     */
    static public Class readClass() {
	return RATIONAL_CLASS;
    }//end method getClass


    /**
     * Returns the actual <tt>deviation</tt> value.<p>
     * Note: If <tt>PRECISE == true</tt>, the deviation value is returned, otherwise <tt>NULL</tt> is returned.
     *
     * @return the <tt>deviation</tt> value as Rational
     * @throws NoDeviationValueFoundException if the deviation value was not implemented in the Rational class extension
     */
    static public Rational readDeviation() throws NoDeviationValueFoundException {
	if (PRECISE == null) {
	    System.out.println("WARNING: PRECISE should be set using RationalFactory.setPrecision()!\n...automatic definition to prevent program termination: PRECISE = TRUE");
	    PRECISE = new Boolean(true);
	}//if
	if (PRECISE.booleanValue()) {
	    Object res;
	    Rational obj = constRational(0);
	    try {
		Field fi = RATIONAL_CLASS.getDeclaredField("deviation");
		res = fi.get(obj);
		return (Rational)res;
	    }//try
	    catch (Exception e) { throw new NoDeviationValueFoundException("Error in RationalFactory: No deviation value was found. It must be implemented in chosen Rational class.");
	    }//catch
	}//if
	else { return null; }
    }//end method readDeviation


    /**
     * Returns the <tt>DEVIATION_DOUBLE</tt> value.<p>
     * Additionally to the <tt>deviation</tt> value, <tt>DEVIATION_DOUBLE</tt> is defined which is a <tt>double</tt> value. Computations using this value 
     * instead of
     * <tt>deviation</tt> are faster. Note, that this method needs a field <tt>DEVIATION_DOUBLE</tt> implemented in the Rational class extension.
     *
     * @return <tt>DEVIATION_DOUBLE</tt> as <tt>double</tt>
     * @throws RationalClassUndefinedException if the Rational class extension was not defined
     * @throws NoDeviationValueFoundException if <tt>DEVIATION_DOUBLE</tt> was not implemented
     */
    static public double readDeviationDouble() throws RationalClassUndefinedException, NoDeviationValueFoundException {
	if (RATIONAL_CLASS == null) {
	    throw new RationalClassUndefinedException("Error in RationalFactory: RATIONAL_CLASS must be set using setClass().");
	}//if
	try {
	    Field fi = RATIONAL_CLASS.getDeclaredField("DEVIATION_DOUBLE");
	    Double obj = null;
	    DEVIATION_DOUBLE = ((Double)fi.get(obj)).doubleValue();
	} catch (Exception e) { throw new NoDeviationValueFoundException("Error in RationalFactory: No DEVIATION_DOUBLE value was found in RATIONAL_CLASS ("+RATIONAL_CLASS+")."); }
	return DEVIATION_DOUBLE;
    }//end method readDeviationDouble


    /**
     * Returns the <tt>DEVIATION_DOUBLE_NEG</tt> value.<p>
     * Additionally to the <tt>deviation</tt> value, <tt>DEVIATION_DOUBLE_NEG</tt> is defined which is a negative <tt>double</tt> value.
     * Computations using a <tt>double</tt>
     * value instead of a Rational value are faster. Note, that this method needs a field <tt>DEVIATION_DOUBLE_NEG</tt> implemented in the
     * Rational class extension.
     *
     * @return <tt>DEVIATION_DOUBLE_NEG</tt> as <tt>double</tt>
     * @throws RationalClassUndefinedException if the Rational class extension was not defined
     * @throws NoDeviationValuefoundException if <tt>DEVIATION_DOUBLE_NEG</tt> was not implemented
     */
    static public double readDeviationDoubleNeg() {
	if (RATIONAL_CLASS == null) {
	    throw new RationalClassUndefinedException("Error in RationalFactory: RATIONAL_CLASS must be seut using setClass().");
	}//if
	try {
	    Field fi = RATIONAL_CLASS.getDeclaredField("DEVIATION_DOUBLE_NEG");
	    Double obj = null;
	    DEVIATION_DOUBLE_NEG = ((Double)fi.get(obj)).doubleValue();
	} catch (Exception e) { throw new NoDeviationValueFoundException("Error in RationalFactory: No DEVIATION_DOUBLE_NEG value was found in RATIONAL_CLASS ("+RATIONAL_CLASS+")."); }
	return DEVIATION_DOUBLE_NEG;
    }//end method readDeviationDoubleNeg


    /**
     * Returns the <tt>PRECISE</tt> value.
     *
     * @return the <tt>PRECISE</tt> value
     */
    static public boolean readPrecise() {
	if (PRECISE == null) {
	    System.out.println("WARNING: PRECISE should be set using RationalFactory.setPrecision()!\n...automatic definition to prevent program termination: PRECISE = TRUE");
	    PRECISE = new Boolean(true);
	}//if
	return PRECISE.booleanValue();
    }//end method readPrecise


    /**
     * Constructs a new Rational instance with an <tt>int</tt>.
     *
     * @param i the <tt>int</tt> value
     * @return the new Rational instance
     * @throws RationalClassUndefinedException if the Rational class extension was not defined
     * @throws RationalClassConstructorNotFoundException if the constructor for <tt>int</tt> values was not implemented
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
		throw new RuntimeException("An error occurred in the ROSEAlgebra.");
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
     * Constructs a new Rational instance with a <tt>double</tt>.
     *
     * @param d the <tt>double</tt> value
     * @return the new Rational instance
     * @throws RationalClassUndefinedException if the Rational class extension was not defined
     * @throws RationalClassConstructorNotFoundException if the constructor for <tt>double</tt> values was not implemented
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
		throw new RuntimeException("An error occurred in the ROSEAlgebra.");
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
		throw new RuntimeException("An error occurred in the ROSEAlgebra.");
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
     * Constructs a new Rational instance with two <tt>int</tt> values.
     *
     * @param num the numerator
     * @param den the denominator
     * @return the new Rational instance
     * @throws RationalClassUndefinedException if the Rational class extension was not defined
     * @throws RationalClassConstructorNotFoundException if the constructor for two <tt>int</tt> values was not implemented
     */    
    static public Rational constRational(int num, int den) throws RationalClassUndefinedException, RationalClassConstructorNotFoundException {
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
		throw new RuntimeException("An error occurred in the ROSEAlgebra.");
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


    /**
     * Sets the <tt>deviation</tt> field of the actual Rational class.
     *
     * @param r the new deviation value
     */
    public static void setDeviation(Rational r) {
	Class[] classParam = { RATIONAL_CLASS };
	Object[] methParam = { r };
	try {
	    Method m = RATIONAL_CLASS.getMethod("setDeviation", classParam);
	    m.invoke(r,methParam);
	} catch (Exception e) {
	    System.out.println("RationalFactory.setDeviation: Problems with method setDeviation.");
	    e.printStackTrace();
	    throw new RuntimeException("An error occurred in the ROSEAlgebra.");
	}//catch
    }//end method setDeviation


    /**
     * Sets the <tt>DEVIATION_DOUBLE</tt> and <tt>DEVIATION_DOUBLE_NEG</tt> fields of the actual Rational class.
     *
     * @param d the new deviation value
     */
    public static void setDeviationDouble(double d){
	Double dd = new Double(d);
	Class[] classParam = { dd.getClass() };
	Object[] methParam = { dd };
	Rational r = constRational(0);
	try { 
	    Method m = RATIONAL_CLASS.getMethod("setDeviationDouble", classParam);
	    m.invoke(r,methParam);
	} catch (Exception e) {
	    System.out.println("RationalFactory.setDeviationDouble: Problems with method setDeviationDouble.");
	    e.printStackTrace();
	    throw new RuntimeException("An error occurred in the ROSEAlgebra.");
	}
    }//end method setDeviationDouble

}//end class RationalFactory
