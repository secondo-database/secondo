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

operator memload alias MEMLOAD pattern op (_)
operator memloadflob alias MEMLOADFLOB pattern op (_)
operator meminit alias MEMINIT pattern op (_)
operator mfeed alias MFEED pattern _ op
operator letmconsume alias LETMCONSUME pattern _ op [_]
operator letmconsumeflob alias LETMCONSUMEFLOB pattern _ op [_]
operator memdelete alias MEMDELETE pattern op (_)
operator memobject alias MEMOBJECT pattern op (_)
operator memlet alias MEMLET pattern op(_,_)
operator memletflob alias MEMLETFLOB pattern op(_,_)
operator memupdate alias MEMUPDATE pattern op(_,_)
#operator mcreateRtree alias MCREATERTREE pattern op(_,_)
operator mcreateRtree alias MCREATERTREE pattern _ op [_]
operator memgetcatalog alias MEMGETCATALOG pattern op()
operator memsize alias MEMSIZE pattern op()
operator memclear alias MEMCLEAR pattern op()
operator minsert alias MINSERT pattern _ op[_]
operator mwindowintersects alias MWINDOWINTERSECTS pattern _ _ op[_]
operator mwindowintersectsS alias MWINDOWINTERSECTSS pattern  _ op[_]
operator mconsume alias MCONSUME pattern _ op
operator mcreateAVLtree2 alias MCREATEAVLTREE2 pattern _ op[_,_,_]
operator mcreateAVLtree alias  MCREATEAVLTREE pattern _ op [_]
operator mexactmatch alias MEXACTMATCH pattern _ _ op [_]
operator mrange alias MRANGE pattern _ _ op [_, _]
operator matchbelow alias MATCHBELOW pattern _ _ op [_]
operator mcreateRtree2 alias MCREATERTREE2 pattern _ op [_,_]

operator mcreateMtree2 alias MCREATEMTREE2 pattern _ op [_,_,_]
operator mdistRange2 alias MDISTRANGE2 pattern _ op [_,_]
operator mdistScan2 alias MDISTSCAN2 pattern _ op [_]

operator mcreateMtree alias MCREATEMTREE pattern _ op [_,_]
operator mdistRange alias MDISTRANGE pattern _ _ op [_,_]
operator mdistScan alias MDISTSCAN  pattern _ _ op [_]



operator mexactmatchS alias MEXACTMATCHS pattern _ op [_]
operator mrangeS alias MRANGES pattern _ op[_,_]
operator matchbelowS alias MATCHBELOWS pattern _ op[_]
