/*
//paragraph	[10]	title:		[{\Large \bf ] [}]
//paragraph	[11]	title:		[{\large \bf ] [}]
//paragraph	[12]	title:		[{\normalsize \bf ] [}]
//paragraph	[21]	table1column:	[\begin{quote}\begin{tabular}{l}]	[\end{tabular}\end{quote}]
//paragraph	[22]	table2columns:	[\begin{quote}\begin{tabular}{ll}]	[\end{tabular}\end{quote}]
//paragraph	[23]	table3columns:	[\begin{quote}\begin{tabular}{lll}]	[\end{tabular}\end{quote}]
//paragraph	[24]	table4columns:	[\begin{quote}\begin{tabular}{llll}]	[\end{tabular}\end{quote}]
//[--------]	[\hline]
//characters	[1]	verbatim:	[$]	[$]
//characters	[2]	formula:	[$]	[$]
//characters	[3]	teletype:	[\texttt{]	[}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [\leq]
//[#]  [\neq]
//[tilde] [\verb|~|]

1 Header File: Profiles 

January 2002 Ulrich Telle

1.1 Overview

Applications often need a mechanism for providing configuration parameters.
One way to supply those parameters is a profile. A profile is a text file
consisting of one ore more named sections which contain one or more named 
parameters and their values, respectively. Each line in the profile is
either a section heading, a key/value pair or a comment. A section heading
is enclosed in square brackets; key name and key value are separated by an
equal sign (leading and trailing whitespace is removed from both key name
and value, but intervening blanks are considered to be part of the name or
the value); a comment line starts with semicolon. All Text in a profile has
to be left aligned. Example:

----  [section 1]
      ;   comment
      key 1=value 1
      key 2=value 2

      [section 2]
      ;   comment
      key 1=value 1
      ....     (and so on)
----

1.1 Interface methods

This module offers routines to get or set a parameter:

[21]	Routines\\
	[--------]
	GetParameter \\
	SetParameter \\

1.3 Class "SmiProfile"[1]

The class ~SmiProfile~ implements routines to manipulate profile strings
stored in a text file:

*/

#ifndef SMI_PROFILES_H
#define SMI_PROFILES_H

#include "SecondoConfig.h"
#include <string>

class SMI_EXPORT SmiProfile
{
 public:
  static string GetParameter( const string& sectionName,
                              const string& keyName, 
                              const string& defaultValue,
                              const string& fileName );
/*
searches the profile ~fileName~ for the key ~keyName~ under the section heading
~sectionName~. If found, the associated string is returned, else the 
default value is returned.

*/

  static long   GetParameter( const string& sectionName,
                              const string& keyName,
                              long          defaultValue,
                              const string& fileName );
/*
searches the profile ~fileName~ for the key ~keyName~ under the section heading
~sectionName~. The function returns

  * ~zero~ -- if the key value is not an integer,

  * ~actual value~ -- if it is possible to interpret the key value as an 
integer (Note that only the beginning of the key value is interpreted, i.e.
"key=345abc"[3] returns "345"[3])

  * ~default value~ -- if key or section not found

*/
  static bool   SetParameter( const string& sectionName,
                              const string& keyName,
                              const string& keyValue,
                              const string& fileName );
/*
searches the profile ~fileName~ for the section heading ~sectionName~ and
the key ~keyName~. If found the value is changed to ~keyValue~, otherwise
it will be added.

The first special case is when ~keyValue~ is an empty string. Then the
corresponding line is deleted.

The second special case is when both ~keyValue~ and ~keyName~ are empty
strings. In that case the whole section ~sectionName~ is deleted.
Lines beginning with ';' are however kept since they are considered comments.

*/
};

#endif // SMI_PROFILES_H

