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

operator bitrate alias BITRATE pattern _ op
operator version alias VERSION pattern _ op
operator frequency alias FREQUENCY pattern _ op
operator framecount alias FRAMECOUNT pattern _ op
operator songlength alias SONGLENGTH pattern _ op
operator getid3 alias GETID3 pattern _ op
operator author alias AUTHOR pattern _ op
operator titleof alias TITLEOF pattern _ op
operator album alias ALBUM pattern _ op
operator comment alias COMMENT pattern _ op
operator genre alias GENRE pattern _ op
operator track alias TRACK pattern _ op
operator savemp3to alias SAVEMP3TO pattern _ infixop _
operator getlyrics alias GETLYRICS pattern _ op
operator lyricswords alias LYRICSWORDS pattern _ _ op
operator removelyrics alias REMOVELYRICS pattern _ op
operator putlyrics alias PUTLYRICS pattern _ _ op
operator putid3 alias PUTID3 pattern _ _ op
operator removeid3 alias REMOVEID3 pattern _ op
operator concatmp3 alias CONCATMP3 pattern _ _ op
operator songyear alias SONGYEAR pattern _ op
operator submp3 alias SUBMP3 pattern _ _ _ op



