/******************************************************************************
---- 
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science, 
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

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

    // size set in the next line for the errors array  must be bigger
    // than the highest error code (this is, if the highest error code is 99,
    // the errors array capacity must be at least 100), because each error
    // message will be stored in the position inside the array according
    // with its error code.
    private static String[] errors = new String[100];

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
      String unknow = "Unknown error code.";
      for(int i=0;i<errors.length;i++)
         errors[i] = unknow;
      // The error messages related to each error code are defined.
      errors[1]="Command not recognized. ";
      errors[2]="Error in (query) expression. ";
      errors[3]="Expression not evaluable. (Operator not recognized or stream?)";
      errors[4]="Error in type expression. No object created. ";
      errors[5]="Error in type expression. No type defined. ";
      errors[6]="No database open. ";
      errors[7]="A database is open. ";
      errors[8]="Undefined object value in (query) expression";
      errors[9]="Syntax error in command/expression";
      errors[10]="Identifier already used. ";
      errors[11]="Identifier is not a known type name. ";
      errors[12]="Identifier is not a known object name. ";
      errors[13]="Type of expression is different from type of object. ";
      errors[14]="Type name is used by an object. Type not deleted. ";
      errors[20]="Transaction already active. ";
      errors[21]="No transaction active. ";
      errors[24]="Error in type or object definitions in file. ";
      errors[25]="Identifier is not a known database name. ";
      errors[26]="Problem in writing to file. ";
      errors[27]="Database name in file different from identifier. ";
      errors[28]="Problem in reading from file. ";
      errors[29]="Error in the list structure in the file. ";
      errors[30]="Command not yet implemented. ";
      errors[31]="Command level not yet implemented. ";
      errors[40]="Error in type definition. ";
      errors[41]="Type name doubly defined. ";
      errors[42]="Error in type expression. ";
      errors[50]="Error in object definition. ";
      errors[51]="Object name doubly defined. ";
      errors[52]="Wrong type expression for object. ";
      errors[53]="Wrong list representation for object. ";
      errors[60]="Kind does not match type expression. ";
      errors[61]="Specific kind checking error for kind. ";
      errors[70]="Value list is not a representation for type constructor. ";
      errors[71]="Specific error for type constructor in value list. ";
      errors[72]="Value list is not a representation for type constructor. ";
      errors[73]="Error at a position within value list for type constructor. ";

      // This two error codes were defined in addition to the error codes
      // returned by the Secondo Server.
      errors[98]="Network error in SecondoInterface. ";
      errors[99]="Internal error in SecondoInterface.";
      errors[80]="Secondo protocol error.";
      errors[81]="Connection to Secondo server lost.";

      // error code for the list algebra xxx operator
      errors[85]="Algebra not know or currently not included";

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
      if(errno<0) // EMPTY ERROR CODE
         return "";
      String errorMessage;
      try {
	errorMessage = errors[errno];
      }
      catch (Exception except){
	errorMessage = "System error: Unknown error code. ErrorCode: "+ String.valueOf(errno);
      }
      return errorMessage;
    }
  }
