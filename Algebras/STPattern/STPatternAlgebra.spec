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

operator vec alias CREATESTVECTOR pattern op(_)
operator stpattern alias STPATTERN pattern _ op[funlist;list]
operator stpattern2 alias STPATTERN2 pattern _ op[funlist;list]
operator stpatternex alias STPATTERNEX pattern _ op[funlist;list;_]
operator stpatternex2 alias STPATTERNEX2 pattern _ op[funlist;list;_]
operator stpatternextend alias STPATTERNEXTEND pattern _ op[funlist;list] implicit parameter streamelem type STREAMELEM
operator stpatternextend2 alias STPATTERNEXTEND2 pattern _ op[funlist;list] implicit parameter streamelem type STREAMELEM
operator stpatternexextend alias STPATTERNEXEXTEND pattern _ op[funlist;list;fun] implicit parameter streamelem type STREAMELEM
operator stpatternexextend2 alias STPATTERNEXEXTEND2 pattern _ op[funlist;list;fun] implicit parameter streamelem type STREAMELEM
operator stpatternextendstream alias STPATTERNEXTENDSTREAM pattern _ op[funlist;list] implicit parameter streamelem type STREAMELEM
operator stpatternextendstream2 alias STPATTERNEXTENDSTREAM2 pattern _ op[funlist;list] implicit parameter streamelem type STREAMELEM
operator stpatternexextendstream alias STPATTERNEXEXTENDSTREAM pattern _ op[funlist;list;fun] implicit parameter streamelem type STREAMELEM
operator stpatternexextendstream2 alias STPATTERNEXEXTENDSTREAM2 pattern _ op[funlist;list;fun] implicit parameter streamelem type STREAMELEM
operator stconstraint alias STCONSTRAINT pattern op(_,_,_)
operator end alias END pattern op(_)
operator start alias START pattern op(_)
operator randommbool alias RandomMBool pattern op(_)
operator passmbool alias PassMBool pattern op(_)
operator randomdelay alias RANDOMDELAY pattern op(_,_)
operator computeclosure alias COMPUTECLOSURE pattern _ op[funlist;list]