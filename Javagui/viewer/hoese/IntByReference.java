/******************************************************************************
 //paragraph	[10]	title:		[\centerline{\Large \bf] [}]
 [10] IntByReference.java
 
 March, 1999. Jose Antonio Cotelo Lema.
 \tableofcontents
 ***************************************************************************** *
 */
/*
 
 1 Overview.
 
 This class (IntByReference) let us pass integers by reference to a method.
 Due to the way as java pass the parameters to the methods (always by value,
 never by  reference), a modified object can be returned as parameter
 (getting a side effect), but the object can not be replaced by a different
 object, because the handle is passed by value. In the same way, a primitive
 value can not be changed an returned as argument to the caller method. This
 is why we use an IntByReference object instead an int primitive parameter
 in some internals methods in the Secondo() public method in the
 SecondoInterface class.
 It is part of the package:
 */


package  viewer.hoese;

/*
 2 The IntByReference class implementation.
 In this section we describe the implementation of the IntByReference class.
 */
/** This class is part of the SecondoJava-User-Interface. See documentation there. */
public class IntByReference extends Object {
  /*
   2.1 Public fields.
   The next public fields are defined in the IntByReference class and hence they are accessible by the user code.
   */
  public int value;

  /*
   3.3 The object constructors.
   3.3.1 the IntByReference() constructor.
   This constructor creates a new IntByReference object with value = 0.
   */
  public IntByReference () {
    this.value = 0;
  }

  /*
   3.3.2 the IntByReference(int val) constructor.
   This constructor creates a new IntByReference object with value = val.
   */
  public IntByReference (int value) {
    this.value = value;
  }
}



