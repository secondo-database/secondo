/*****************
 ----
 This file is part of SECONDO.

 Copyright (C) 2011, University in Hagen, Department of Computer Science,
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

//paragraph [3] verse:	[\begin{verse}]	[\end{verse}]

3 OptSecUtils

This is the document describes the implementation of the OptSecUtils checks which are done in the Optparser.

********************/

#include "NestedList.h"
#include "SecondoInterface.h"

extern NestedList* nl; // use global si
extern SecondoInterface* si; // use the same si as the rest of prolog


namespace optutils
{

/* ~dbOpen~ Checks whether a database is opened. If so, its name is returns
 via the name argument.*/

bool isDatabaseOpen(string& name, string& errorMsg);

/* ~isObject~ Checks whether in the opened database an object with the given name exists.
 The name is checked case insensitive. If more than one object exists, the first one
 is selected. If a corresponding object was found, its type is returned via
 the type argument. */

bool isObject(const string& name, string& realName, ListExpr& type);

/* ~isRelationDescription~ */

bool isRelDescription(const ListExpr type, const string reltype);

/* ~getAttributeNames~ Inserts all attributes names of a relation description to a set of strings. */

void getAttributeNames(const ListExpr type, set<string>& names);

void getAttributeNames(const string& name, set<string>& names);

/* ~isValidId~ This function checks whether a string can be used as a name
 (for an attribute, an object etc.). If this name is
 not valid, the result of this function will be false.
 If the boolean parameter is set to true, the result will also
 be false, if an object with this name is present in the currently
 opened database. */

bool isValidID(const string& id);

/*~checkKind~ Checks wether the type is member of the kind */
bool checkKind(const string& type, const string& kind);

/* ~getDatabaseObjectNames~ Gets all database object names so we can check wheter an aliases is valid or
 is already being used. */
void getDatabaseObjectNames(set<string>& names);

/* Checks whether two strings are equal. Using the boolean parameter,
 the case sensitivity of the comparison can be controlled. */
bool strequal(const string& s1, const string& s2, const bool case_sensitive);

/* ~checkOperator~ Checks if a operator exists and does work with all parameters given.*/

bool checkOperator(string op, list<string>& parameters);

/* ~getOperatorType~ return the operator return type for a given operator and its parameters */

string getOperatorType(string operatorname, list<string>& parameters);

/* ~getAttributeType~ This method takes an attribute and relationname and tries to get the type of the attribute */
string getAttributeType(string attribute, string relation);

/* ~isOperator~ Checks if operator is an operator via quering SEC2OPERATORINFO */
bool isOperator(string operatorname);

} // end of namespace


