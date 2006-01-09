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

operator createinsertrel alias CREATEINSERTREL pattern _ op
operator createdeleterel alias CREATEDELETEREL pattern _ op
operator createupdaterel alias CREATEUPDATEREL pattern _ op
operator insert alias INSERT pattern _ _ op
operator insertsave alias INSERTSAVE pattern _ _ _ op
operator inserttuple alias INSERTTUPLE pattern _ op [list]
operator inserttuplesave alias INSERTTUPLESAVE pattern _ _ op [list]
operator deletedirect alias DELETEDIRECT pattern _ _ op
operator deletesearch alias DELETESEARCH pattern _ _ op
operator deletesearchsave alias DELETESEARCHSAVE pattern _ _ _ op
operator deletedirectsave alias DELETEDIRECTSAVE pattern _ _ _ op
operator updatesearch alias UPDATESEARCH pattern _ _ op [funlist]implicit parameter tuple type TUPLE
operator updatedirect alias UPDATEDIRECT pattern _ _ op [funlist]implicit parameter tuple type TUPLE
operator updatesearchsave alias UPDATESEARCHSAVE pattern _ _ _ op [funlist]implicit parameter tuple type TUPLE
operator updatedirectsave alias UPDATEDIRECTSAVE pattern _ _ _ op [funlist]implicit parameter tuple type TUPLE
operator appendidentifier alias APPENDIDENTIFIER pattern _ op
operator deletebyid alias DELETEBYID pattern _ op [ _ ]
operator updatebyid alias UPDATEBYID pattern _ op [ _; funlist]implicit parameter tuple type TUPLE
