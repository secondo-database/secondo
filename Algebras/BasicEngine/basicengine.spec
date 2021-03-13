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

operator be_init alias BE_INIT pattern op (_,_,_,_,_,_)
operator be_init_cluster alias BE_INIT_CLUSTER pattern op (_,_,_,_,_,_)

operator be_shutdown alias BE_SHUTDOWN pattern op ()
operator be_shutdown_cluster alias BE_SHUTDOWN_CLUSTER pattern op ()

operator be_query alias BE_QUERY pattern op (_,_)
operator be_command alias BE_COMMAND pattern op (_)
operator be_collect alias BE_COLLECT pattern op (_)
operator be_copy alias BE_COPY pattern op (_,_)
operator be_mquery alias BE_MQUERY pattern op (_,_)
operator be_mcommand alias BE_MCOMMAND pattern op (_)
operator be_union alias BE_UNION pattern op (_)
operator be_struct alias BE_STRUCT pattern op (_)
operator be_runsql alias BE_RUNSQL pattern op (_)

operator be_share alias BE_SHARE pattern op (_)
operator be_validate_query alias BE_VALIDATE_QUERY pattern op (_)

operator be_part_rr alias BE_PART_RR pattern op (_,_,_)
operator be_part_hash alias BE_PART_HASH pattern op (_,_,_)
operator be_part_fun alias BE_PART_FUN pattern op (_,_,_)
operator be_part_grid alias BE_PART_GRID pattern op (_,_,_,_,_,_,_)

operator be_repart_rr alias BE_REPART_RR pattern op (_,_,_)
operator be_repart_hash alias BE_REPART_HASH pattern op (_,_,_)

