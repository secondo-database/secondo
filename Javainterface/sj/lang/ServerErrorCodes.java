/******************************************************************************

//paragraph	[10]	title:		[\centerline{\Large \bf] [}]

[10] ServerErrorCodes.java
  
November 3, 1998. Jose Antonio Cotelo Lema, Miguel Rodriguez Luaces.

\tableofcontents

******************************************************************************/


/*
1 Overview.

This static class translates the error codes received from the server to one 
string with the appropiated error message text. To change the messages related with each error code of SecondoInterface only this class needs to be modified.

It is part of the package:

*/

package sj.lang;

/*

2 Included files.

This class uses some external classes and packages what are neded for the
correct compiling and execution of this class.

2.1 External packages.

 This class uses the following external classes of the Java Development Kit:

*/

/*  From the java.util package */
import java.util.Vector;


/*

3 The ServerErrorCodes class implementation.

 In this section we describe the implementation of the ServerErrorCodes class.

*/
public class ServerErrorCodes extends Object {

/*

3.1 Public fields.

The following public fields are defined in the ServerErrorCodes class.

*/
    // Here we define some constants.
    public static final int NOT_ERROR_CODE = 0;
    public static final int NOT_OPEN_DB_CODE = 6;
    public static final int ALREADY_OPEN_DB_CODE = 7;
    public static final int SYNTAX_ERROR_CODE = 9;
    public static final int NOT_DB_NAME_CODE = 25;
    public static final int FILE_WRITE_ERROR_CODE = 26;
    public static final int FILE_READ_ERROR_CODE = 28;
    // This two error codes were defined in addition to the error codes 
    // returned by the Secondo Server.
    public static final int NETWORK_ERROR_CODE = 98;
    public static final int INTERNAL_ERROR_CODE = 99;

    // size set in the next line for the errors vector must be bigger
    // than the highest error code (this is, if the highest error code is 99,
    // the errors vector capacity must be at least 100), because each error
    // message will be stored in the possition inside the vector according
    // with its error code.
    private static Vector errors = new Vector(100);

/*

3.2 Private fields.

None private fields are defined in the ServerErrorCodes class.

*/
/*

3.3 The object constructor.

 This constructor creates a new ServerErrorCodes object.

*/ 
    public ServerErrorCodes(){
      // it has nothing to do.
    }   

/*
3.4 The ~static~ statement.

 This ~static~ statement initializes the error messages at class loading time.

*/
    static{
      // Ensures the errors vector will have enough space.
      errors.setSize(errors.capacity());
      // The error messages related to each error code are defined.
      errors.setElementAt("Command not recognized. ",1);
      errors.setElementAt("Error in (query) expression. ",2);
      errors.setElementAt("Expression not evaluable. (Operator not recognized or stream?)",3);
      errors.setElementAt("Error in type expression. No object created. ",4);
      errors.setElementAt( "Error in type expression. No type defined. ",5);
      errors.setElementAt("No database open. ",6);
      errors.setElementAt("A database is open. ",7);
      errors.setElementAt("Undefined object value in (query) expression",8);
      errors.setElementAt("Syntax error in command/expression",9);
      errors.setElementAt("Identifier already used. ",10);
      errors.setElementAt("Identifier is not a known type name. ",11);
      errors.setElementAt("Identifier is not a known object name. ",12);
      errors.setElementAt("Type of expression is different from type of object. ",13);
      errors.setElementAt("Type name is used by an object. Type not deleted. ",14);
      errors.setElementAt("Transaction already active. ",20);
      errors.setElementAt("No transaction active. ",21);
      errors.setElementAt("Error in type or object definitions in file. ",24);
      errors.setElementAt("Identifier is not a known database name. ",25);
      errors.setElementAt("Problem in writing to file. ",26);
      errors.setElementAt("Database name in file different from identifier. ",27);
      errors.setElementAt("Problem in reading from file. ",28);
      errors.setElementAt("Error in the list structure in the file. ",29);
      errors.setElementAt("Command not yet implemented. ",30);
      errors.setElementAt("Command level not yet implemented. ",31);
      errors.setElementAt( "Error in type definition. ",40);  // (40 i) 	      
      errors.setElementAt( "Type name doubly defined. ",41);   // 	(41 i n)      
      errors.setElementAt("Error in type expression. ",42);   // 	(42 i n)      
      errors.setElementAt("Error in object definition. ",50);   //	(50 i)	      
      errors.setElementAt( "Object name doubly defined. ",51);   //	(51 i n)      
      errors.setElementAt("Wrong type expression for object. ",52);   //	(52 i n)      
      errors.setElementAt("Wrong list representation for object. ",53);   // (53 i n)
      errors.setElementAt("Kind does not match type expression. ", 60);   //	(60 k t)      
      errors.setElementAt("Specific kind checking error for kind. ",61);   //	(61 k j ...)  
      errors.setElementAt("Value list is not a representation for type constructor. ",70);   //	(70 tc v)     
      errors.setElementAt("Specific error for type constructor in value list. ",71);   //	(71 tc j ...) 
      errors.setElementAt("Value list is not a representation for type constructor. ", 72);   //	(72 tc)       
      errors.setElementAt("Error at a position within value list for type constructor. ", 73);   //	(73 pos)

      // This two error codes were defined in addition to the error codes 
      // returned by the Secondo Server.
      errors.setElementAt("Network error in SecondoInterface. ", 98); 
      errors.setElementAt("Internal error in SecondoInterface.", 99);
      errors.setElementAt("Secondo protocol error.",80);
      errors.setElementAt("Connection to Secondo server lost.",81);

    }
    
  
/*

3.5 Public methods.

 The following public methods are defined in the ~ServerErrorCodes~ class.

*/
/*
3.5.1 The ~getErrorMessageText~ method.

This public method gets as parameter an error code, ands returns the error 
message related with it. If no error message is related to this error code, a 
generic error message is returned.

*/
    public static String getErrorMessageText(int errno){
      String errorMessage;
      try {
	errorMessage = (String)errors.elementAt(errno);
      }
      catch (ArrayIndexOutOfBoundsException except){
	errorMessage = "System error: Unknown error code. ErrorCode: "+ String.valueOf(errno);
      }
      return errorMessage;
    }
  }
