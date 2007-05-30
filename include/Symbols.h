/*
---- 
This file is part of SECONDO.

Copyright (C) 2004-2007, University in Hagen, Faculty of Mathematics and Computer Science, 
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

May 2007, M. Spiekermann. Initial version.

This file should contain all symbols in nested lists which are 
types or reserved words.

*/

#include <string>

#undef INT
#undef REAL
#undef BOOL
#undef STRING
#undef TEXT 

namespace symbols {

 typedef const std::string Sym;

 // standard types
 Sym INT("int");	
 Sym REAL("real");
 Sym BOOL("bool");
 Sym STRING("string");
 Sym TEXT("text");

 // standard types
 Sym REL("rel");
 Sym TUPLE("TUPLE");

 // some reserved words of the query processor
 Sym MAP("map"); 
 Sym STREAM("stream"); 
 Sym APPEND("APPEND"); 
}	


