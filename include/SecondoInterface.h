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

May 2002 Ulrich Telle Port to C++, added initialization and termination
methods for hiding the details of setting up the "Secondo"[3] environment
from the user interface program.

December 2002 M. Spiekermann an application specific nested list memory (al) was
introduced. Every Interface method which returns a nested list copies the
list from the nl Object into the al Object. If a list is input for some Interface
methods e.g. NumTypeExpr(...) the list is copied from al to nl. Now the core systems 
own list memory is refreshed in the Interface method Secondo(...) so that every
command is processed with an initially empty list container.
The application has to take care about its own list memory to avoid infinte growing.  

April 29 2003 Hoffmann Added save and restore commands for single objects.


1.1 Overview

This module defines the procedure ~Secondo~ as defined in ``The Secondo
Project'' [G[ue]95]. The main procedure ~Secondo~ reads a command and
executes it. It possibly returns a result.

The first subsection describes the initialization and termination of the
interface.

The second subsection describes the various commands and the errors that
may occur for each of them. 

The third subsection describes error handling and the specific error
codes; it makes error messages available in an array ~errors~. 

The fourth subsection makes some procedures for type transformation and
information available, namely ~NumericTypeExpr~, ~GetTypeId~ and ~LookUpTypeExpr~.
~NumericTypeExpr~ transforms type expressions into a numeric form suitable
for writing application programs treating types in a generic way (e.g.
representation of values at the user interface). ~GetTypeId~ returns the
algebra and type identifier for a type.

Note that there have been some slight changes in respect to [G[ue]95]
in the treatment of the database state in database commands. 

*/

#ifndef SECONDO_INTERFACE_H
#define SECONDO_INTERFACE_H

#include <string>
#include <map>
#include "SocketIO.h"
#include "NestedList.h"
#include "AlgebraTypes.h"

/************************************************************************** 
2.1 Class "SecondoInterface"[1]

2.1.1 Creation, Deletion, Initialization and Termination

*/ 

class SecondoInterface
{
 public:
  SecondoInterface();
/*
Constructs a "Secondo"[3] interface. Depending on the implementation of
the interface different member variables are initialized.

*/
  virtual ~SecondoInterface();
/*
Destroys a "Secondo"[3] interface.

*/
  bool Initialize( const string& user, const string& pswd,
                   const string& host, const string& port,
                   string& profile,
                   const bool multiUser = false );
/*
Starts up the "Secondo"[3] interface. Depending on the implementation
not all parameters are required for the interface to be operational.

The current implementation of the "Secondo"[3] system does not support user
authentification. Nevertheless the ~user~ identification and the password ~pswd~
can be specified. In the client/server version this information is passed to
the "Secondo"[3] server to identify the user session.

In the client/server version the ~host~ address and the ~port~ of the "Secondo"[3]
server are needed to establish a connection to the server, but these parameters
may be specified via the configuration file ~profile~. The method arguments
~host~ and ~port~ take precedence over specifications in the configuration file.

In the single user version only the name of the configuration file
~profile~ must be specified. Values for ~host~ and ~port~ are ignored.

*/
  void Terminate();
/*
Shuts down the "Secondo"[3] interface. In the client/server version
the connection to the "Secondo"[3] server is closed; in the single user version
the "Secondo"[3] system and the ~SmiEnvironment~ are shut down.

*/
/*
2.1.2 The "Secondo"[3] main interface method

*/
  void Secondo( const string& commandText,
                const ListExpr commandLE,
                const int commandLevel,
                const bool commandAsText,
                const bool resultAsText,
                ListExpr& resultList,
                int& errorCode,
                int& errorPos,
                string& errorMessage,
                const string& resultFileName =
                                "SecondoResult" );
/*
Reads a command and executes it; it possibly returns a result.
The command is one of a set of "Secondo"[3] commands described below. The
parameters have the following meaning. 

A "Secondo"[3] command can be given at various ~levels~; parameter
~commandLevel~ indicates the level of the current command. The levels
are defined as follows: 

  * 0 -- "Secondo"[3] executable command in nested list syntax (~list~)

  * 1 -- "Secondo"[3] executable command in SOS syntax (~text~)

  * 2 -- "Secondo"[3] descriptive command after, or without, algebraic optimization (~list~)

  * 3 -- "Secondo"[3] descriptive command after, or without, algebraic optimization (~text~)

  * 4 -- "Secondo"[3] descriptive command before algebraic optimization (~list~)

  * 5 -- "Secondo"[3] descriptive command before algebraic optimization (~text~)

  * 7 -- Command in some specific query language (e.g. SQL, GraphDB, etc.)

If the command is given in ~text~ syntax (command levels 1, 3, or 5),
then the text string must be placed in ~commandText~. If the command is
given in ~list~ syntax, it can be passed either as a text string in
~commandText~, in which case ~commandAsText~ must be "true"[4], or as a list
expression in ~commandLE~; in the latter case, ~commandAsText~ must be
"false"[4]. 

If the command produces a result (e.g. a query), then the result can be
requested to be returned either as a list expression (~resultAsText~ is
"false"[4]) in the parameter ~resultList~, or (~resultAsText~ is "true"[4]) in a
text file whose name is set to *SecondoResult* by default, but may be
overwritten. 

Finally, the procedure returns an ~errorCode~. There are four general
error code numbers: 

  * 0: no error

  * 1: command not recognized

  * 9: syntax error in command/expression (found by "Secondo"[3] Parser)

  * 30: command not yet implemented

  * 31: command level not yet implemented

The other error codes are explained below together with the commands
that may produce them. If an error occurred (~errorCode~ different from
0), then the application can print the error message given in
~errors[errorcode]~. Possibly additional information about the error is
given in parameter ~errorMessage~ (e.g. messages by "Secondo"[3] Parser) and
in the ~resultList~ which is then a list of errors in the form explained
below. 

Furthermore, ~errorPos~ contains a position within the ~commandBuffer~
where the error was detected (only when the command was given in the
text buffer, of course). - not yet implemented. - 

2.1.1 Basic Commands

----  type <identifier> = <type expression>
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

----  type <identifier> = <type expression>
----

Define a type name ~identifier~ for the type expression. Possible errors:

  * 10: ~identifier~ already used

  * 5: error in type expression

  * 6: no database open

In case of error 5, an error list with further information is returned
in ~resultList~.

----  delete type <identifier>
----    

Delete the type name ~identifier~. Possible errors:

  * 11: ~identifier~ is not a known type name

  * 14: type name is used by an object

  * 6: no database open

----  create <identifier> : <type expression>
----

Create an object called ~identifier~ of the type given by ~type
expression~. The value is still undefined. Possible errors: 

  * 10: ~identifier~ already used

  * 4: error in ~type expression~

  * 6: no database open

In case of error 4, an error list with further information is returned
in ~resultList~.


----  update <identifier> := <value expression>
----

Assign the value computed by ~value expression~ to the object
~identifier~. Possible errors: 

  * 12: ~identifier~ is not a known object name

  * 2: error in ~value expression~

  * 3: ~value expression~ is correct, but not evaluable (e.g. a stream)

  * 8: undefined object value in ~value expression~

  * 13: type of value is different from type of object

  * 6: no database open

----  delete <identifier>
----

Destroy the object ~identifier~. Possible errors:

  * 12: ~identifier~ is not a known object name

  * 6: no database open

----  query <value expression>
----

Evaluate the value expression and return the result as a nested list.
Possible errors: 

  * 2: error in ~value expression~

  * 3: ~value expression~ is correct, but not evaluable (e.g. a stream)

  * 8: undefined object value in ~value expression~

  * 6: no database open

----  let <identifier> = <value expression>
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

----  persistent <identifier>
----

Make the object ~identifier~ persistent, so that it will survive the end
of a session. Presumably this object was created by a ~let~ command
earlier. If it is not a temporary object, this command is not an error,
but has no effect. Possible errors: 

  * 12: ~identifier~ is not a known object name

  * 6: no database open

*NOTE*: Beginning with version 2 of the "Secondo"[3] system all objects are
persistent by default. That is the ~persistent~ command is obsolete.

2.1.2 Transaction Commands

----  begin transaction
      commit transaction
      abort transaction
----    

These commands can only be used when a database is open. Any permanent
changes to a database can only be made within a transaction. Only a
single transaction can be active for this particular client at any time.

----  begin transaction
----

Start a transaction. Possible errors:

  * 20: a transaction is already active.

  * 6: no database open

----  commit transaction
----

Commit a running transaction; all changes to the database will be
effective. Possible errors: 

  * 21: no transaction is active.

  * 6: no database open

----  abort transaction
----

Abort a running transaction; all changes to the database will be
revoked. Possible errors: 

  * 21: no transaction is active.

  * 6: no database open

*NOTE*: All commands not enclosed in a ~begin transaction~ ...
~commit/abort transaction~ block are implicitly surrounded by a
transaction.

2.1.3 Database Commands

----  create database <identifier>
      delete database <identifier>
      open database <identifier>
      close database 
      save database to <filename>
      save <objectname> to <filename>
      restore database <identifier> from <filename>
      restore <objectname> from <filename>
----

*NOTE*: All database commands can not be enclosed in a transaction.

The commands ~create database~ and ~delete database~ are only valid when
currently there is no open database ("IsDatabaseOpen() == false"[4]).
They leave this state unchanged. 

The commands ~open database~ and ~restore database~ are only valid when
currently there is no open database ("IsDatabaseOpen() == false"[4]), the
database is open after successful completion.

The command ~close database~ is only valid if "IsDatabaseOpen() == true"[4].
No database is open after successful completion.

The command ~save database~ is only valid if "IsDatabaseOpen() == true"[4],
it leaves the state of the database unchanged. 

----  create database <identifier>
----

Create a new database. Possible errors:

  * 10: ~identifier~ already used

  * 7: a database is open

----  delete database <identifier>
----

Destroy the database ~identifier~. Possible errors:

  * 25: ~identifier~ is not a known database name.

  * 7: a database is open

----  open database <identifier>
----

Open the database ~identifier~. Possible errors:

  * 25: ~identifier~ is not a known database name.

  * 7: a database is open

----  close database
----

Close the currently open database. Possible errors:

  * 6: no database open

----  save database to <filename>
----

Write the entire contents of the database ~identifier~ in nested list
format to the file ~filename~. The structure of the file is the
following: 

----  (DATABASE <database name>
        (DESCRIPTIVE ALGEBRA) 
          (TYPES
            (TYPE <type name> <type expression>)*    
          )
          (OBJECTS
            (OBJECT <object name> (<type name>) <type expression>
                                                <value>)*
          )
        (EXECUTABLE ALGEBRA) 
          (TYPES
            (TYPE <type name> <type expression>)*    
          )
          (OBJECTS
            (OBJECT <object name> (<type name>) <type expression>
                                                <value>)*
          )
      )
----

If the file exists, it will be overwritten, otherwise created.
Possible errors: 

  * 6: no database open

  * 26: a problem occurred in writing the file (no permission, file system full, etc.)
  
----  save <objectname> to <filename>
----

Writes the contents of an object, type list, value list and model list in nested
list format to the file ~filename~. The structure of the file is the
following:

---- (OBJECT <object name> (<type name>) <type expression>
                                                <value>)
----

If the file exists, it will be overwritten, otherwise be created.
Possible errors: 

  * 6 : database not open

  * 26: a problem occurred in writing the file (no permission, file system full, etc.)
  
  * 82: identifier of object is not known in the currently opened database
	
----  restore database <identifier> from <filename>
----

Read the contents of the file ~filename~ into the database ~identifier~.
The database is in open state after successful completion.
Previous contents of the database are lost.
Possible errors: 

  * 25: ~identifier~ is not a known database name

  * 27: the database name in the file is different from ~identifier~

  * 7: a database is open

  * 28: a problem occurred in reading the file (syntax error, no permission, file system full, etc.)

  * 29: the overall list structure of the file is not correct

  * 24: there are errors in type or object definitions in the file

In case of error 24, an error list with further information is returned
in ~resultList~.

----  restore <objectname> from <filename>
----

Reads the file ~filename~ and creates a Secondo object according to this content.

Precondition:

The database must be in open state and the object is not known in the currently opened database.

Possible errors: 

  * 84: the object name is not known in the database

  * 83: the object name in the file is different from ~identifier~

  * 6: a database is not open

  * 28: a problem occurred in reading the file (syntax error, no permission, file system full, etc.)

  * 29: the overall list structure of the file is not correct

  * 24: there are errors in the object definition in the file

In case of error 24, an error list with further information is returned
in ~resultList~.

2.1.4 Inquiries

----  list databases
      list type constructors
      list operators
      list types
      list objects
      list counters
      list algebras
      list algebra <algebraname>
----

The commands list types and list objects are only valid when a database is open.

All Secondo list commands return a nested list with the following general list structure:

----  ( inquiry ( <inquirydestination> <valuelist> ) )
----

where ~inquiry~ is a fixed symbol atom, <inquirydestination> a symbol atom with the value ~databases~,
~algebras~, ~algebra~, ~types~, ~objects~, ~constructors~ or operators and <valuelist> is of type nested
list.  

----  list databases
      ( inquiry ( databases <valuelist> ) )
      E.G.: ( inquiry ( databases ( GEO OPT BERLIN ) ) )
----

Returns a list of names of existing databases. Possible errors: none.

----  list type constructors
      ( inquiry ( constructors <valuelist> ) )
      E.G: ( inquiry (constructors 
             (
               (date 
                 ("Signature" "Example Type List" "List Rep" "Example List")
                 ("-> DATA" "date" <text>"<year>-<month>-<day>"</text---> <text>"2003-09-05"</text--->)))))
----

Return a nested list of type constructors (and their specifications).
For the precise format see [G[ue]95]. Possible errors: none. 

----  list operators
      ( inquiry ( operators <valuelist> ) )
      E.G.: (inquiry (operators 
              (
                (and 
                  ("Signature" "Syntax" "Meaning" "Example")
                  (<text>(bool bool) -> bool</text---> <text>_ and _</text---> <text>Logical And.
                   </text---> <text>query (8 = 8) and (3 < 4)</text--->)))))
----

Return a nested list of operators (and their specifications). For the
precise format see [G[ue]95]. Possible errors: none. 

----  list types
      ( inquiry ( types <valuelist> ) )
----

Return a nested list of type names defined in the currently open
database. The format is: 

----  (TYPES
        (TYPE <type name> <type expression>)*    
      )
----

Possible errors: 

  * 6: no database open

----  list objects
      ( inquiry ( objects <valuelist> ) )
----

Return a nested list of objects existing in the currently open database.
The format is: 

----  (OBJECTS
        (OBJECT <object name> (<type name>) <type expression>)*
      )
----

This is the same format as the one used in saving and restoring the
database except that the ~value~ component is missing. The type name is
written within a sublist since an object need not have a type name.
Possible errors: 

  * 6: no database open

----	list counters
----

Returns a nested list containing pairs of the form (<counter number> <counter
value>). Counters can be associated with operators within a query. The list
contains the counter values from the last query.
Possible errors:

  * 6: no database open
  
----    list algebras
        ( inquiry ( algebras <valuelist> ) )
        E.G.: (inquiry (algebras 
              (StandardAlgebra RelationAlgebra))) 
----

Returns a list of the currently active included algebras at Secondo runtime.
Possible errors: none.

----    list algebra <algebraname>
        ( inquiry ( algebra <valuelist> ) )
        E.G.: (inquiry (algebra 
               (RectangleAlgebra 
                 (
                   (
                     (rect 
                       ("Signature" "Example Type List" "List Rep" "Example List")
                       ("-> DATA" "rect" "(<left> <right> <bottom> <top>)" "(0 1 0 1)")))
                   (
                     (intersects 
                        ("Signature" "Syntax" "Meaning" "Example")
                        (<text>(rect x rect) -> bool </text---> <text>_ intersects _</text---> 
                         <text>Intersects.</text---> <text>query rect1 intersects rect2</text--->)))))))
----


1.3.1 Type transformation and information methods

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
typeId). The catalog corresponding to the current ~level~ (descriptive or
executable) is used to resolve type names in the type expression.
For example, 

----  int    ->    (1 1)

      (rel (tuple ((name string) (age int)))

      ->     ((2 1) ((2 2) ((name (1 4)) (age (1 1))))
----

Identifiers such as ~name~, ~age~ are moved unchanged into the result
list. If a type expression contains other constants that are not
symbols, e.g. integer constants as in "(array 10 real)"[4], they are also
moved unchanged into the result list. 

The resulting form of the type expression is useful for calling the type
specific ~In~ and ~Out~ procedures. 

*/
  bool GetTypeId( const AlgebraLevel level,
                  const string& name,
                  int& algebraId, int& typeId );
/*
Finds the ~algebraId~ and ~typeId~ of a named type.
The catalog corresponding to the current ~level~ (descriptive or
executable) is used to resolve the type name.

*/
  bool LookUpTypeExpr( const AlgebraLevel level,
                       ListExpr type, string& name,
                       int& algebraId, int& typeId );
/*
Finds the ~name~, ~algebraId~ and ~typeId~ of a type given by the type expression
~type~. The catalog corresponding to the current ~level~ (descriptive or
executable) is used to resolve the type name.

*/
  NestedList* GetNestedList();
/*
Returns a reference to the application specific nested list container. 

2.2 Error Messages

*/
  static string GetErrorMessage( const int errorCode );
/*
Error messages 1 through 30 are generated within ~SecondoInterface~ and
directly returned in procedure ~Secondo~. Error messages larger than 40
belong to four groups: 

  * 4x -- errors in type definitions in database files

  * 5x -- errors in object definitions in database files

  * 6x -- errors found by kind checking procedures

  * 7x -- errors found by ~In~ procedures of algebras in the list representations for values

  * 8x -- errors found by type checking procedures in algebras (this group does not yet exist at the moment)

All such error messages (larger than 40) are appended to a list
~errorList~. Each procedure generating error messages has a
parameter ~errorInfo~ containing a pointer to the current last element
of ~errorList~. It appends the error message as a list with a command of
the form 

----  errorInfo = Append(errorInfo, <message list>)
----

If errors are appended to the list within the execution of a "Secondo"[3]
command, then the list ~errorList~ is returned in parameter ~resultList~
of procedure ~Secondo~. This currently happens for the commands 

----  type <identifier> = <type expr>
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
    errors[1]  = "Command not recognized.";
    errors[2]  = "Error in (query) expression.";
    errors[3]  = "Expression not evaluable. "
                 "(Operator not recognized or stream?)";
    errors[4]  = "Error in type expression. No object created.";
    errors[5]  = "Error in type expression. No type defined.";
    errors[6]  = "No database open.";
    errors[7]  = "A database is open.";
    errors[8]  = "Undefined object value in (query) expression";
    errors[9]  = "Syntax error in command/expression";

    errors[10] = "Identifier already used.";
    errors[11] = "Identifier is not a known type name.";
    errors[12] = "Identifier is not a known object name.";
    errors[13] = "Type of expression is different from type of "
                 "object.";
    errors[14] = "Type name is used by an object. Type not deleted.";

    errors[20] = "Transaction already active.";
    errors[21] = "No transaction active.";
    errors[22] = "Begin transaction failed.";
    errors[23] = "Commit/Abort transaction failed.";

    errors[24] = "Error in type or object definitions in file.";
    errors[25] = "Identifier is not a known database name.";
    errors[26] = "Problem in writing to file.";
    errors[27] = "Database name in file different from identifier.";
    errors[28] = "Problem in reading from file.";
    errors[29] = "Error in the list structure in the file.";
    errors[30] = "Command not yet implemented.";
    errors[31] = "Command level not yet implemented.";
    errors[32] = "Command not yet implemented at this level.";

    errors[40] = "Error in type definition.";             // (40 i)
    errors[41] = "Type name doubly defined.";             // (41 i n)
    errors[42] = "Error in type expression.";             // (42 i n)

    errors[50] = "Error in object definition.";           // (50 i)
    errors[51] = "Object name doubly defined.";           // (51 i n)
    errors[52] = "Wrong type expression for object.";     // (52 i n)
    errors[53] = "Wrong list representation for object."; // (53 i n)

    errors[60] = "Kind does not match type expression.";  // (60 k t)
    errors[61] = "Specific kind checking error for kind.";
                                 // (61 k j ...)

    errors[70] = "Value list is not a representation for type "
                 "constructor."; // (70 tc v)
    errors[71] = "Specific error for type constructor in "
                 "value list.";  // (71 tc j ...)
    errors[72] = "Value list is not a representation for type "
                 "constructor."; // (72 tc)
    errors[73] = "Error at a position within value list for type "
                 "constructor."; // (73 pos)

    errors[80] = "Secondo protocol error.";
    errors[81] = "Connection to Secondo server lost.";
    
    errors[82] = "Identifier of object is not a known database object. ";
    errors[83] = "Object name in file different from indentifier for object. ";
    errors[84] = "Identifier of object already known in the database. ";
    errors[85] = "Algebra not known or currently not included."

----

The error messages 61 and 71 allow a kind checking procedure or an ~In~
procedure to introduce its own specific error codes (just numbered 1, 2,
3, ...). These error messages may then have further parameters. To
interpret such error messages (and return information to the user) one
needs to add code branching on these specific error code numbers. Such
code may or may not be supplied with an algebra. 

*/
  void SetDebugLevel( const int level );
/*
Sets the debug level of the query processor.

*/
	
 protected:
 private:
  void StartCommand();
  void FinishCommand( int& errorCode );

  bool        initialized;       // state of interface
  bool        activeTransaction; // state of transaction block
  NestedList  *nl, *al;          // References of
                                 // nested list containers
  Socket*     server;            // used in C/S version only

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

