#This file is part of SECONDO.

#Copyright (C) 2015, University in Hagen, Department of Computer Science,
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

operator isempty alias ISEMPTY pattern op(_)
operator no_components alias NO_COMPONENTS pattern op(_)
operator to_dline alias TO_DLINE pattern op(_)
operator to_pointseq alias TO_POINTSEQ pattern op(_)
operator to_tpointseq alias TO_TPOINTSEQ pattern op(_)

operator dist_origin alias DIST_ORIGIN pattern op(_, _)
operator dist_destination alias DIST_DESTINATION pattern op(_, _)
operator dist_origin_and_destination alias DIST_DESTINATION pattern op(_, _)
