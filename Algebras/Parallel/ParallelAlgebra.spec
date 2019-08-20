#This file is part of SECONDO.

#Copyright (C) 2019, 
#University in Hagen, 
#Faculty of Mathematics and Computer Science,
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


operator multicount alias MULTICOUNT pattern op(_,_,_)
operator pbuffer alias PBUFFER pattern _ op [_] 
operator pbuffer1 alias PBUFFER1 pattern _ op  
operator pbufferU alias PBUFFERU pattern _ op  
operator pfilterS alias PFILTERS pattern _ op[_,_,_,fun] implicit parameter streamelem type STREAMELEM
operator pextend alias PEXTEND pattern _ op[_,_,_; funlist] implicit parameter streamelem type STREAMELEM
operator pextendstream alias PEXTENDSTREAM pattern _ op[_,_,_; funlist] implicit parameter streamelem type STREAMELEM
operator punion alias PUNION pattern _ _ op

operator ploopsel alias PLOOPSEL pattern _ op[_, fun] implicit parameter streamelem type STREAMELEM
operator ploopjoin alias PLOOPJOIN pattern _ op[_, fun] implicit parameter streamelem type STREAMELEM





