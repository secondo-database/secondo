import java.lang.reflect.Constructor;
import java.lang.reflect.Field;

public class RationalFactory {
    //This class is used for constructing numeric values.
    //It contains a set of methods called constRational()
    //which call constructors of implementations of the
    //abstract class Rational. At least two implementations
    //are provided, namely
    // - RationalBigInteger
    // - RationalDouble
    //
    //The implementation used is stored in the variable
    //RATIONAL_CLASS which must set using the method
    //setClass().
    //
    //Another variable PRECISE holds the user-selected value
    //TRUE or FALSE. It determines whether a precise but
    //slow computation is used when computing the following
    //predicates:
    //Mathset.linearly_dependent(Segment,Segment)
    //PointSeg_Ops.liesOn(Point,Segment)
    //PRECISE should be set using setPrecicision(). If not,
    //a warning is printed and TRUE is set.

    //members
    static private Class RATIONAL_CLASS = null;
    static private Boolean PRECISE = null;

    //methods
    static public void setClass(Class ratClass) {
	RATIONAL_CLASS = ratClass;
    }//end method setClass


    static public void setPrecision(boolean prec) {
	PRECISE = new Boolean(prec);
    }//end method setPrecision


    static public void setClass(String ratClass) {
	try {
	    RATIONAL_CLASS = Class.forName(ratClass);
	}//try
	catch (Exception e) { throw new RationalClassNotExistentException("Error in RationalFactory: Class file "+ratClass+" was not found.");
	}//catch
    }//end method setClass

    /*
    static public void setClass_RationalBigInteger() {
	//written by Mirco 
	//replaces this by setClass(String)
	setClass(RationalBigInteger.class);
    }

    static public void setClass_RationalDoubles() {
	//written by Mirco
	//replace this by setClass(String)
	setClass(RationalDouble.class);
    }
    */

    static public Class readClass() {
	return RATIONAL_CLASS;
    }//end method getClass


    static public Rational readDeriv() {
	//if PRECISE = TRUE, it returns the derivation value from RATIONAL_CLASS
	//otherwise null is returned
	
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
		//res = (Rational)RATIONAL_CLASS.getDeclaredField("deriv").get(RATIONAL_CLASS.newInstance());
		return (Rational)res;
	    }//try
	    catch (Exception e) { throw new NoDerivationValueFoundException("Error in RationalFactory: No derivation value was found. It must be implemented in chosen Rational class.");
	    }//catch
	}//if
	else { return null; }
    }//end method readDeriv


    static public boolean readPrecise() {
	//returns the value of PRECISE
	if (PRECISE == null) {
	    System.out.println("WARNING: PRECISE should be set using RationalFactory.setPrecision()!\n...automatic definition to prevent program termination: PRECISE = TRUE");
	    PRECISE = new Boolean(true);
	}//if
	return PRECISE.booleanValue();
    }//end method readPrecise


    static public Rational constRational(int i) {
	if (RATIONAL_CLASS == null) {
	    throw new RationalClassUndefinedException("Error in RationalFactory: RATIONAL_CLASS must be set using setClass()");
	}//if
	else {
	    Class[] paramClassList = { int.class };
	    Object[] paramValueList = { new Integer(i) };
	    Rational rat;
	    try {
		Constructor constructor = RATIONAL_CLASS.getConstructor(paramClassList);
		rat = (Rational)constructor.newInstance(paramValueList);
	    }//try
	    catch (Exception e) { throw new RationalClassConstructorNotFoundException("Error in RationalFactory: constructor "+RATIONAL_CLASS+"(int) not found"); }
	    return rat;
	}//else
    }//end method constRational


    static public Rational constRational(double d) {
	if (RATIONAL_CLASS == null) {
	    throw new RationalClassUndefinedException("Error in RationalFactory: RATIONAL_CLASS must be set using setClass()");
	}//if
	else {
	    Class[] paramClassList = { double.class };
	    Object[] paramValueList = { new Double(d) };
	    Rational rat;
	    try {
		Constructor constructor = RATIONAL_CLASS.getConstructor(paramClassList);
		rat = (Rational)constructor.newInstance(paramValueList);
	    }//try
	    catch (Exception e) { throw new RationalClassConstructorNotFoundException("Error in RationalFactory: constructor "+RATIONAL_CLASS+"(double) not found"); }
	    return rat;
	}//else
    }//end method constRational

    
    static public Rational constRational(Rational r) {
	if (RATIONAL_CLASS == null) {
	    throw new RationalClassUndefinedException("Error in RationalFactory: RATIONAL_CLASS must be set using setClass()");
	}//if
	else {
	    Class[] paramClassList = { Rational.class };
	    Object[] paramValueList = { r };
	    Rational rat;
	    try {
		Constructor constructor = RATIONAL_CLASS.getConstructor(paramClassList);
		rat = (Rational)constructor.newInstance(paramValueList);
	    }//try
	    catch (Exception e) { throw new RationalClassConstructorNotFoundException("Error in RationalFactory: constructor "+RATIONAL_CLASS+"(Rational) not found"); }
	    return rat;
	}//else
    }//end method constRational

    
    static public Rational constRational(int num, int den) {
	if (RATIONAL_CLASS == null) {
	    throw new RationalClassUndefinedException("Error in RationalFactory: RATIONAL_CLASS must be set using setClass()");
	}//if
	else {
	    Class[] paramClassList = { int.class, int.class };
	    Object[] paramValueList = { new Integer(num), new Integer(den) };
	    Rational rat;
	    try {
		Constructor constructor = RATIONAL_CLASS.getConstructor(paramClassList);
		rat = (Rational)constructor.newInstance(paramValueList);
	    }//try
	    catch (Exception e) { throw new RationalClassConstructorNotFoundException("Error in RationalFactory: constructor "+RATIONAL_CLASS+"(int int) not found"); }
	    return rat;
	}//else
    }//end method constRational


}//end class RationalFactory
