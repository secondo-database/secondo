#This file is part of SECONDO.

#Copyright (C) 2004, University in Hagen, Department of Computer Science, 
#Database Systems for New Applications.

#SECONDO is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation; either version 2 of the License, or
#(at your option) any later version.

#SECONDO is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.

#You should have received a copy of the GNU General Public License
#along with SECONDO; if not, write to the Free Software
#Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#!/usr/bin/awk -f
#
# specs.awk
# Created in 06.12.2002 by Victor Teixeira de Almeida
# This shell script will read the result of the ordered (without duplicates) concatenation of the specs files for all algebras. 
# It will be called by the makefile of the Secondo System. If this script finds an operator that have more than one specification, 
# it will return a WARNING containing the important information for the makefile.
#
BEGIN {ret = 0}
$1 == "operator" { count[$2]++ }
END {
  for (w in count) 
    if( count[w] > 1 )
    { 
      print "WARNING: Operator", w, "has", count[w], "occurrencies in the file specs!";
      ret = 1
    };
  exit ret
}

