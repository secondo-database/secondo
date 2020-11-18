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

operator create_irgrid2d alias CREATE_IRGRID2D pattern _ op [_, _, _]
operator create_irgrid3d alias CREATE_IRGRID3D pattern _ op [_, _, _]
operator feed alias FEED pattern _ op
operator cellnos_ir alias CELLNOS_IR pattern op(_, _)
operator trccell_ir alias TRCCELL_IR pattern op(_,_,_)
operator trc_ir alias TRC_IR pattern op(_,_)
operator scc_ir2d alias SCC_IR2D pattern op(_,_,_,_)
operator getcell_ir2d alias GETCELL_IR2D pattern op(_,_)
operator trc_ir3d alias TRC_IR3D pattern op(_,_,_)
operator trccell_ir3d alias TRCCELL_IR3D pattern op(_,_,_)
operator scc_ir3d alias SCC_IR3D pattern op(_,_,_,_)
operator getcell_ir3d alias GETCELL_IR3D pattern op(_,_)
operator create_2dtree alias CREATE_2DTREE pattern _ op [_]
operator cellnos_kd alias CELLNOS_KD pattern op(_, _)
operator trc_kd alias TRC_KD pattern op(_,_,_)
operator trccell_kd alias TRCCELL_KD pattern op(_,_,_)
operator scc_kd alias SCC_KD pattern op(_,_,_,_)
operator getcell_kd alias GETCELL_KD pattern op(_,_)
operator create_3dtree alias CREATE_3DTREE pattern _ op [_]
operator trc_3d alias TRC_3D pattern op(_,_,_)
operator trccell_3d alias TRCCELL_3D pattern op(_,_,_)
operator scc_3d alias SCC_3D pattern op(_,_,_,_)
operator getcell_3d alias GETCELL_3D pattern op(_,_)
operator bbox_grid alias BBOX_GRID pattern _ op
operator bbox_grid3d alias BBOX_GRID3D pattern _ op