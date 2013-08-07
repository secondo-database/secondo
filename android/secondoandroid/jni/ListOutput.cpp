/*
----
This file is part of SECONDO.

Copyright (C) 2004-2008, University in Hagen, Faculty of Mathematics and Computer Science,
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

1 The Implementation-Module SecondoInterface

September 1996 Claudia Freundorfer

December 23, 1996 RHG Added error code information.

January 17, 1998 RHG Connected Secondo Parser (= command level 1 available).

This module implements the module ~SecondoInterface~ by using the
modules ~NestedList~, ~SecondoCatalog~, ~QueryProcessor~ and
~StorageManager~.

May 15, 1998 RHG Added a command ~model value-expression~ which is
analogous to ~query value-expression~ but computes the result model for
a given query rather than the result value.

November 18, 1998 Stefan User commands ``abort transaction'' and
``commit transaction'' are implemented by calling SMI\_Abort() and
SMI\_Commit(), respectively.

April 2002 Ulrich Telle Port to C++

August 2002 Ulrich Telle Set the current algebra level for SecondoSystem.

September 2002 Ulrich Telle Close database after creation.

November 7, 2002 RHG Implemented the ~let~ command.

December 2002 M. Spiekermann Changes in Secondo(...) and NumTypeExpr(...).

February 3, 2003 RHG Added a ~list counters~ command.

April 29 2003 Hoffmann Added save and restore commands for single objects.

April 29, 2003 M. Spiekermann bug fix in LookUpTypeExpr(...).

April 30 2003 Hoffmann Changes syntax for the restore objects command.

September 2003 Hoffmann Extended section List-Commands for Secondo-Commands
~list algebras~ and ~list algebra <algebra name>~.

October 2003 M. Spiekermann made the command echo (printing out the command in
NL format) configurable.  This is useful for server configuration, since the
output of big lists consumes more time than processing the command.

May 2004, M. Spiekermann. Support of derived objects (for further Information
see DerivedObj.h) introduced.  A new command derive similar to let can be used
by the user to create objects which are derived from other objects via a more
or less complex value expression. The information about dependencies is stored
in two system tables (relation objects). The save database command omits to
save list expressions for those objects.  After restoring all saved objects the
derived objects are rebuild in the restore database command.

August 2004, M. Spiekermann. The complex nesting of function ~Secondo~ has been reduced.

Sept 2004, M. Spiekermann. A bug in the error handling of restore databases has been fixed.

Dec 2004, M. Spiekermann. The new command ~set~ was implemented to support
interactive changes of runtime parameters.

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

February 2006, M. Spiekermann. A proper handling of errors when commit or abort
transaction fails after command execution was implemented. Further, the scope of
variables in function Secondo was limited to a minimum, e.g. the declarations
were moved nearer to the usage. This gives more encapsulation and is easier to
understand and maintain.

April 2006, M. Spiekermann. Implementation of system tables SEC\_COUNTERS and SEC\_COMMANDS.

August 2006, M. Spiekermann. Bug fix for error messages of create or delete
database.  The error codes of the SMI module are now integrated into the error
reporting of the secondo interface. However, currently only a few SMI error
codes are mapped to strings.

September 2006, M. Spiekermann. System tables excluded into a separate file
called SystemTables.h.

April 2007, M. Spiekermann. Fixed bug concerning transaction management. Started
transactions of errorneous queries were not aborted.

\tableofcontents

*/

#include "ListOutput.h"


ListOutput::ListOutput(NestedList *nl) {
	this->nl=nl;
	zaehler=0;
}

ListOutput::~ListOutput() {
}

bool ListOutput::outputList(ListExpr li) {
	if(nl->IsEmpty(li)) {
		__android_log_print(ANDROID_LOG_INFO,"FU", "Empty");
	} else if(nl->IsAtom(li)) {
		switch(GetBinaryType(li)) {
		case BIN_TEXT: case BIN_STRING: case BIN_SHORTSTRING: 
		case BIN_SHORTTEXT: case BIN_LONGTEXT: case BIN_LONGSTRING:
			__android_log_print(ANDROID_LOG_INFO,
			"FU","String %s", nl->ToString(li).c_str());
			break;
		case BIN_DOUBLE: case BIN_REAL:
			__android_log_write(ANDROID_LOG_INFO,"FU", "Double");
			break;
		case BIN_SYMBOL: case BIN_SHORTSYMBOL: case BIN_LONGSYMBOL:
			__android_log_print(ANDROID_LOG_INFO,
			"FU", "Symbol %s",nl->ToString(li).c_str());

			break;
		case BIN_INTEGER: case BIN_SHORTINT:
			__android_log_write(ANDROID_LOG_INFO,"FU", "Integer");
			break;
		case BIN_BYTE:
			__android_log_write(ANDROID_LOG_INFO,"FU", "Byte");
			break;
		case BIN_BOOLEAN:
			__android_log_write(ANDROID_LOG_INFO,"FU", "boolean");
			break;
		default:
			__android_log_write(ANDROID_LOG_INFO,"FU", "No Output");
			break;

		}
	} else {
		__android_log_write(ANDROID_LOG_INFO,"FU", "List");

		    bool ok = outputList(nl->First( li ));

		    if ( !ok ) {
		      return (ok);
		    }

		    while ( !nl->IsEmpty( nl->Rest( li ) ) )
		    {
		      li = nl->Rest( li );
		      ok = outputList(nl->First( li ) );

		      if ( !ok ) {
		        return (ok);
		      }
		    }
			__android_log_write(ANDROID_LOG_INFO,"FU", "List End");
	}


	return true;
}


NodeType ListOutput::GetBinaryType(ListExpr list) {
	switch( nl->AtomType(list) ) {

	  case BoolType     : return  BIN_BOOLEAN;
	  case IntType      : { long v = nl->IntValue(list);
	                        if(v>=-128 && v<=127)
	                           return BIN_BYTE;
	                        if(v>=-32768 && v<=32767)
	                           return BIN_SHORTINT;
	                        return BIN_INTEGER;
	                      }
	  case RealType     : return BIN_DOUBLE;
	  case SymbolType   : { int len = nl->SymbolValue(list).length();
	                        if(len<256)
	                           return BIN_SHORTSYMBOL;
	                        if(len<65536)
	                           return BIN_SYMBOL;
	                        return BIN_LONGSYMBOL;
	                      }
	  case StringType   : { int len = nl->StringValue(list).length();
	                        if(len<256)
	                           return BIN_SHORTSTRING;
	                        if(len<65536)
	                           return BIN_STRING;
	                        return BIN_LONGSTRING;
	                      }
	  case TextType     : { int len = nl->TextLength(list);
	                        if(len<256)
	                           return BIN_SHORTTEXT;
	                        if(len<65536)
	                           return BIN_TEXT;
	                        return BIN_LONGTEXT;
	                       }
	  case NoAtom        : {int len = nl->ListLength(list);
	                        if(len<256)
	                          return BIN_SHORTLIST;
	                        if(len<65536)
	                           return BIN_LIST;
	                        return BIN_LONGLIST;
	                       }
	  default : return (byte) 255;

	  }

}
void ListOutput::resetZaehler() {
	zaehler=0;
}

