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

operator protocol alias PROTOCOL pattern op(_)
operator host alias HOST pattern op(_)
operator webfilename alias WEBFILENAME pattern op(_)
operator source alias SOURCE pattern op(_)
operator createurl alias CREATEURL pattern op(_)
operator content alias CONTENT pattern op(_)
operator urls alias URLS pattern op(_)
operator containsurl alias CONTAINSURL pattern op(_, _)
operator lastmodified alias LASTMODIFED pattern op(_)
operator metainfo alias METAINFO pattern op(_, _)
operator metainfos alias METAINFOS pattern op(_)
operator numberof alias NUMBEROF pattern op(_, _)
operator similar alias SIMILAR pattern op(_,_,_,_)
operator extracthtml alias EXTRACTHTML pattern op(_)
operator numoffiles alias NUMOFFILES pattern op(_)
operator getfiles alias GETFILES pattern op(_)
operator wget alias WGET pattern op(_, _, _, _, _)
operator pageget alias PAGEGET pattern op(_, _, _, _, _)
operator htmlget alias HTMLGET pattern op(_, _, _, _, _)
operator webequal alias WEBEQUAL pattern op(_, _)
