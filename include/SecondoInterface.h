/*
//paragraph    [10]    title:           [{\Large \bf ] [}]
//paragraph    [21]    table1column:    [\begin{quote}\begin{tabular}{l}]     [\end{tabular}\end{quote}]
//paragraph    [22]    table2columns:   [\begin{quote}\begin{tabular}{ll}]    [\end{tabular}\end{quote}]
//paragraph    [23]    table3columns:   [\begin{quote}\begin{tabular}{lll}]   [\end{tabular}\end{quote}]
//paragraph    [24]    table4columns:   [\begin{quote}\begin{tabular}{llll}]  [\end{tabular}\end{quote}]
//[--------]    [\hline]
//characters    [1]    verbatim:   [$]    [$]
//characters    [2]    formula:    [$]    [$]
//characters    [3]    capital:    [\textsc{]    [}]
//characters    [4]    teletype:   [\texttt{]    [}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [\leq]
//[#]  [\neq]
//[tilde] [\verb|~|]
//[Contents] [\tableofcontents]

1 Header File: Secondo Interface

September 1995 Claudia Freundorfer

January 2, 1997 Ralf Hartmut G[ue]ting. Explanation of commands and
introduction of error codes. 

January 10, 1997 RHG Additional error codes for error lists. Made
variable ~Errors~ available as well as procedure ~NumericTypeExpr~. 

January 17, 1998 RHG Changes in description, new error code 9.

May 15, 1998 RHG Added a command ~model value-expression~ which is
analogous to ~query value-expression~ but computes the result model for
a given query rather than the result value. 

May 2002 Ulrich Telle Port to C++

1.1 Overview

This module defines the procedure ~Secondo~ as defined in ``The Secondo
Project'' [G[ue]95]. The main procedure ~Secondo~ reads a command and
executes it. It possibly returns a result. The first subsection
describes the various commands and the errors that may occur for each of
them. 

The second subsection describes error handling and the specific error
codes; it makes error messages available in an array ~Errors~. 

The third subsection makes a procedure ~NumericTypeExpr~ available wich
transforms type expressions into a numeric form suitable for writing
application programs treating types in a generic way (e.g.
representation of values at the user interface). 

Note that there have been some slight changes with repect to [G[ue]95]
in the treatment of the database state in database commands. 

The current implementation simulates persistency by restoring the
database from a file when a database is opened, and saving it to the
file when the database is closed. The filename for this is identical
with the database name. *Make sure there is no file in the directory
with a name equal to a database name which has other contents than a
database list representation; it would be overwritten.* 

*/

#ifndef SECONDO_INTERFACE_H
#define SECONDO_INTERFACE_H

#include <string>
#include <map>
#include "NestedList.h"
#include "AlgebraTypes.h"

/************************************************************************** 
2.1 Class SecondoInterface

*/ 

class SecondoInterface
{
 public:
  SecondoInterface();
  virtual ~SecondoInterface();
  void Secondo( const string& commandText, /* in ??*/
                const ListExpr commandLE,
                const int commandLevel,
                const bool commandAsText,
                const bool resultAsText,
                ListExpr& resultList,
                int& errorCode,
                int& errorPos,
                string& errorMessage,
                const string& resultFileName = "SecondoResult" );
/*
~Secondo~ reads a command and executes it; it possibly returns a result.
The command is one of a set of SECONDO commands described below. The
parameters have the following meaning. 

A Secondo command can be given at various ~levels~; parameter
~commandLevel~ indicates the level of the current command. The levels
are defined as follows: 

  * 0 -- Secondo executable command in nested list syntax (~list~)

  * 1 -- Secondo executable command in SOS syntax (~text~)

  * 2 -- Secondo descriptive command after, or without, algebraic optimization (list)

  * 3 -- Secondo descriptive command after, or without, algebraic optimization (text)

  * 4 -- Secondo descriptive command before algebraic optimization (list)

  * 5 -- Secondo descriptive command before algebraic optimization (text)

  * 7 -- Command in some specific query language (e.g. SQL, GraphDB, etc.)

If the command is given in ~text~ syntax (command levels 1, 3, or 5),
then the text string must be placed in ~commandText~. If the command is
given in ~list~ syntax, it can be passed either as a text string in
~commandText~, in which case ~commandAsText~ must be true, or as a list
expression in ~commandLE~; in the latter case, ~commandAsText~ must be
false. 

If the command produces a result (e.g. a query), then the result can be
requested to be returned either as a list expression (~resultAsText~ is
false) in the parameter ~resultList~, or (~resultAsText~ is true) in a
text file whose name is fixed to be ~SecondoResult~. 

Finally, the procedure returns an ~errorCode~. There are four general
error code numbers: 

  * 0: no error

  * 1: command not recognized

  * 9: syntax error in command/expression (found by Secondo Parser)

  * 30: command not yet implemented

  * 31: command level not yet implemented

The other error codes are explained below together with the commands
that may produce them. If an error occurred (~errorCode~ different from
0), then the application can print the error message given in
~Errors[errorcode]~. Possibly additional information about the error is
given in parameter ~errorMessage~ (e.g. messages by Secondo Parser) and
in the ~resultList~ which is then a list of errors in the form explained
in Section 1.2. 

Furthermore, ~errorPos~ contains a position within the ~commandBuffer~
where the error was detected (only when the command was given in the
text buffer, of course). - not yet implemented. - 

For more details on error messages and error handling see Section 1.2.

2.1.1 Basic Commands

----    type <identifier> = <type expression>
    delete type <identifier>
    create <identifier> : <type expression>
    update <identifier> := <value expression>
    delete <identifier>
    query <value expression>

    (not yet implemented:)
    let <identifier> = <value expression>
    persistent <identifier>
----

All basic commands are only valid, if currently a database is open.

----    type <identifier> = <type expression>
----

Define a type name ~identifier~ for the type expression. Possible errors:

  * 10: ~identifier~ already used

  * 5: error in type expression

  * 6: no database open

In case of error 5, an error list with further information is returned
in ~ResultList~ (see second subsection). 

----    delete type <identifier>
----    

Delete the type name ~identifier~. Possible errors:

  * 11: ~identifier~ is not a known type name

  * 14: type name is used by an object

  * 6: no database open

----    create <identifier> : <type expression>
----

Create an object called ~identifier~ of the type given by ~type
expression~. The value is still undefined. Possible errors: 

  * 10: ~identifier~ already used

  * 4: error in ~type expression~

  * 6: no database open

In case of error 4, an error list with further information is returned
in ~resultList~ (see second subsection). 


----    update <identifier> := <value expression>
----

Assign the value computed by ~value expression~ to the object
~identifier~. Possible errors: 

  * 12: ~identifier~ is not a known object name

  * 2: error in ~value expression~

  * 3: ~value expression~ is correct, but not evaluable (e.g. a stream)

  * 8 undefined object value in ~value expression~

  * 13: type of value is different from type of object

  * 6: no database open

----    delete <identifier>
----

Destroy the object ~identifier~. Possible errors:

  * 12: ~identifier~ is not a known object name

  * 6: no database open

----    query <value expression>
----

Evaluate the value expression and return the result as a nested list.
Possible errors: 

  * 2: error in ~value expression~

  * 3: ~value expression~ is correct, but not evaluable (e.g. a stream)

  * 8 undefined object value in ~value expression~

  * 6: no database open


*Not yet implemented:*

----    let <identifier> = <value expression>
----

Assign the value resulting from ~value expression~ to a new object
called ~identifier~. The object must not exist yet; it is created by
this command and its type is by definition the one of the value
expression. This object is temporary so far; it will be automatically
destroyed at the end of a session. Possible errors: 

  * 10: ~identifier~ already used

  * 2: error in ~value expression~

  * 3: ~value expression~ is correct, but not evaluable (e.g. a stream)

  * 6: no database open

----    persistent <identifier>
----

Make the object ~identifier~ persistent, so that it will survive the end
of a session. Presumably this object was created by a ~let~ command
earlier. If it is not a temporary object, this command is not an error,
but has no effect. Possible errors: 

  * 12: ~identifier~ is not a known object name

  * 6: no database open


2.1.2 Transaction Commands

----    begin transaction
    commit transaction
    abort transaction
----    

These commands can only be used when a database is open. Any permanent
changes to a database can only be made within a transaction. Only a
single transaction can be active for this particular client at any time.

----    begin transaction
----

Start a transaction. Possible errors:

  * 20: a transaction is already active.

  * 6: no database open

----    commit transaction
----

Commit a running transaction; all changes to the database will be
effective. Possible errors: 

  * 21: no transaction is active.

  * 6: no database open

----    abort transaction
----

Abort a running transaction; all changes to the database will be
revoked. Possible errors: 

  * 21: no transaction is active.

  * 6: no database open


2.1.3 Database Commands

----    create database <identifier>
    delete database <identifier>
    open database <identifier>
    close database 
    save database to <filename>
    restore database <identifier> from <filename>
----

The commands ~create database~ and ~delete database~ are only valid when
currently there is no open database (state = ~DBClosed~). They leave
this state unchanged. 

The commands ~open database~ and ~restore database~ are only valid when
currently there is no open database (state = ~DBClosed~), they change
the state to ~DBOpen~. 

The command ~close database~ is only valid in state ~DBOpen~. It changes
the state to ~DBClosed~. 

The command ~save database~ is only valid in state ~DBOpen~, it leaves
the state unchanged. 

----    create database <identifier>
----

Create a new database. Possible errors:

  * 10: ~identifier~ already used

  * 7: a database is open

----    delete database <identifier>
----

Destroy the database ~identifier~. Possible errors:

  * 25: ~identifier~ is not a known database name.

  * 7: a database is open

----    open database <identifier>
----

Open the database ~identifier~. Changes state to ~DBOpen~. Possible errors:

  * 25: ~identifier~ is not a known database name.

  * 7: a database is open

----    close database
----

Close the currently open database. Possible errors:

  * 6: no database open

----    save database to <filename>
----

Write the entire contents of the database ~identifier~ in nested list
format to the file ~filename~. The structure of the file is the
following: 

----    (DATABASE <database name>
        (TYPES
        (TYPE <type name> <type expression>)*    
        )
        (OBJECTS
        (OBJECT <object name> (<type name>) <type expression> <value>)*
        )
    )
----

If the file exists, it will be overwritten, otherwise be created.
Possible errors: 

  * 6: no database open

  * 26: a problem occurred in writing the file (no permission, file system full, etc.)


----    restore database <identifier> from <filename>
----

Read the contents of the file ~filename~ into the database ~identifier~.
Changes state to ~DBOpen~. Previous contents of the database are lost.
Possible errors: 

  * 25: ~identifier~ is not a known database name

  * 27: the database name in the file is different from ~identifier~

  * 7: a database is open

  * 28: a problem occurred in reading the file (syntax error, no permission, file system full, etc.)

  * 29: the overall list structure of the file is not correct

  * 24: there are errors in type or object definitions in the file

In case of error 24, an error list with further information is returned
in ~ResultList~ (see second subsection). 


2.1.4 Inquiries

----    list databases
    list type constructors
    list operators
    list types
    list objects
----

The last two commands are only valid when a database is open.

----    list databases
----

Returns a list of names of existing databases. Possible errors: none.

----    list type constructors
----

Return a nested list of type constructors (and their specifications).
For the precise format see [G[ue]95]. Possible errors: none. 

----    list operators
----

Return a nested list of operators (and their specifications). For the
precise format see [G[ue]95]. Possible errors: none. 

----    list types
----

Return a nested list of type names defined in the currently open
database. The format is: 

----    (TYPES
        (TYPE <type name> <type expression>)*    
    )
----

Possible errors: 

  * 6: no database open

----    list objects
----

Return a nested list of objects existing in the currently open database.
The format is: 

----    (OBJECTS
        (OBJECT <object name> (<type name>) <type expression>)*
    )
----

This is the same format as the one used in saving and restoring the
database except that the ~value~ component is missing. The type name is
written within a sublist since an object need not have a type name.
Possible errors: 

  * 6: no database open


1.3 Procedure ~NumericType~

The following procedure allows an application to transform a type
expression into an equivalent form with numeric codes. This may be
useful to provide type-specific output or representation procedures. One
can organize such procedures via doubly indexed arrays (indexed by
algebra number and type constructor number). 

*/

  ListExpr NumericTypeExpr( const AlgebraLevel level,
                            const ListExpr type );
/*
Transforms a given type expression into a list structure where each type
constructor has been replaced by the corresponding pair (algebraId,
typeId). For example, 

----    int    ->    (1 1)

    (rel (tuple ((name string) (age int)))

    ->     ((2 1) ((2 2) ((name (1 4)) (age (1 1))))
----

Identifiers such as ~name~, ~age~ are moved unchanged into the result
list. If a type expression contains other constants that are not
symbols, e.g. integer constants as in (array 10 real), they are also
moved unchanged into the result list. 

The resulting form of the type expression is useful for calling the type
specific ~In~ and ~Out~ procedures. 

*/
  bool GetTypeId( const AlgebraLevel level,
                  const string& name,
                  int& algebraId, int& typeId );
  bool LookUpTypeExpr( const AlgebraLevel level,
                       ListExpr type, string& name,
                       int& algebraId, int& typeId );
  NestedList* GetNestedList();
/*
2.2 Error Messages

*/
  static string GetErrorMessage( const int errorCode );
/*
Error messages 1 through 30 are generated within ~SecondoInterface~ and
directly returned in procedure ~Secondo~. Error messages larger than 40
belong to four groups: 

  * 4x -- errors in type definitions in database files

  * 5x -- errors in object definitions in database files

  * 6x -- errors found by kind checking procedures (in module ~Kinds~)

  * 7x -- errors found by ~In~ procedures of algebras in the list representations for values

  * 8x -- errors found by type checking procedures in algebras (this group does not yet exist at the moment)

All such error messages (larger than 40) are appended to a list
~errorList~. Each procedure generating error messages has a VAR
parameter ~errorInfo~ containing a pointer to the current last element
of ~errorList~. It appends the error message as a list with a command of
the form 

----    errorInfo := Append(errorInfo, <message list>)
----

If errors are appended to the list within the execution of a Secondo
command, then the list ~errorList~ is returned in parameter ~resultList~
of procedure ~Secondo~. This currently happens for the commands 

----    type <identifier> = <type expr>
    create <identifier> : <type expr>
    restore database <identifier> from <filename>
----

since these commands involve kind checking and checking of value list
representations for objects. 

The messages that are appended to ~errorList~ usually have further
parameters in addition to the error code number. The list following the
error message below describes the error entry appended. The parameters
after the error number have the following meaning: 

  * ~i~: number of type definition or object definition in database file (the ~i~-th type definition, the ~i~-th object definition),

  * ~n~: type name or object name in that definition,

  * ~k~: kind name,

  * ~t~: type expression,

  * ~j~: error number specific to a given kind ~k~ or type constructor ~tc~,

  * ~tc~: a type constructor,

  * ~v~: value list, list structure representing a value for a given type constructor.

----
  errors[1] := "Command not recognized. ";
  errors[2] := "Error in (query) expression. ";
  errors[3] := "Expression not evaluable. (Operator not recognized or stream?)";
  errors[4] := "Error in type expression. No object created. ";
  errors[5] := "Error in type expression. No type defined. ";
  errors[6] := "No database open. ";
  errors[7] := "A database is open. ";
  errors[8] := "Undefined object value in (query) expression";
  errors[9] := "Syntax error in command/expression";

  errors[10] := "Identifier already used. ";
  errors[11] := "Identifier is not a known type name. ";
  errors[12] := "Identifier is not a known object name. ";
  errors[13] := "Type of expression is different from type of object. ";
  errors[14] := "Type name is used by an object. Type not deleted. ";

  errors[20] := "Transaction already active. ";
  errors[21] := "No transaction active. ";

  errors[24] := "Error in type or object definitions in file. ";
  errors[25] := "Identifier is not a known database name. ";
  errors[26] := "Problem in writing to file. ";
  errors[27] := "Database name in file different from identifier. ";
  errors[28] := "Problem in reading from file. ";
  errors[29] := "Error in the list structure in the file. ";
  errors[30] := "Command not yet implemented. ";
  errors[31] := "Command level not yet implemented. ";
  errors[32] := "Command not yet implemented at this level. ";

  errors[40] := "Error in type definition. ";         //     (40 i)           
  errors[41] := "Type name doubly defined. ";         //     (41 i n)      
  errors[42] := "Error in type expression. ";        //     (42 i n)      

  errors[50] := "Error in object definition. ";        //    (50 i)          
  errors[51] := "Object name doubly defined. ";        //    (51 i n)      
  errors[52] := "Wrong type expression for object. ";    //    (52 i n)      
  errors[53] := "Wrong list representation for object. ";//    (53 i n)      

  errors[60] := "Kind does not match type expression. ";//    (60 k t)      
  errors[61] := "Specific kind checking error for kind. ";//    (61 k j ...)  

  errors[70] := "Value list is not a representation for type constructor. ";
                            //    (70 tc v)     
  errors[71] := "Specific error for type constructor in value list. ";
                            //    (71 tc j ...) 
  errors[72] := "Value list is not a representation for type constructor. ";
                            //    (72 tc)       
  errors[73] := "Error at a position within value list for type constructor. ";
                            //    (73 pos)      
----

The error messages 61 and 71 allow a kind checking procedure or an ~In~
procedure to introduce its own specific error codes (just numbered 1, 2,
3, ...). These error messages may then have further parameters. To
interpret such error messages (and return information to the user) one
needs to add code branching on these specific error code numbers. Such
code may or may not be supplied with an algebra. 

*/
 protected:
 private:
  bool        activeTransaction;
  NestedList* nl;

  static void InitErrorMessages();
  static bool errMsgInitialized;
  static map<int,string> errors;
};

/*
*References*

[G[ue]95] G[ue]ting, R.H., The SECONDO Project. Praktische Informatik
IV, Fernuniversit[ae]t Hagen, working paper, November 1995. 

*/

#endif

