#This file is part of SECONDO.

#Copyright (C) 2008, University in Hagen,
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


operator csvexport alias CSVEXPORT pattern _ op [ _ , _ ]
operator shpexport alias SHPEXPORT pattern _ op [ _ ]
operator db3export alias DB3EXPORT pattern _ op [ _ ]
operator shptype alias SHPTYPE pattern op( _ )
operator shpimport alias SHPIMPORT pattern _ op [ _ ]
operator dbtype alias dbTYPE pattern op( _ )
operator dbimport alias DBIMPORT pattern _ op [ _ ]
operator saveObject alias SAVEOBJECT pattern _ op [ _ , _ ]
operator csvimport  alias csvimport pattern _ op [ _ , _ , _ ]
operator isFile  alias ISFILE pattern op( _ )
operator isDirectory alias ISDIRECTORY pattern op( _ )
operator removeFile alias removeFILE pattern op( _ )
operator createDirectory alias CREATEDIRECTORY pattern op( _ )
operator fileSize alias FILESIZE pattern op( _, _ )
operator writeFile alias WRITEFILE pattern op( _, _, _ )
operator readFile alias READFILE pattern op( _ )
operator moveFile alias MOVEFILE pattern op( _, _ )
operator getDirectory alias GETDIRECTORY pattern op( _, _ )
operator toCSVtext alias TOCSVTEXT pattern op( _ )
operator fromCSVtext alias FROMCSVTEXT pattern op( _, _ )
operator getSecondoVersion alias GETSECONDOVERSION pattern op ( _ )
operator getBDBVersion alias GETBDBVERSION pattern op ( _ )
operator getPID alias GETPID pattern op ( _ )
operator getSecondoPlatform alias GETSECONDOPLATFORM pattern op ( _ )
operator getPageSize alias GETPAGESIZE pattern op ( _ )
operator sqlExport alias SQLEXPORT pattern _ op [ _,_,_] 

