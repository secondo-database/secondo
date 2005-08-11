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

operator extract_track     alias EXTRACT_TRACK     pattern _ op [_]
operator merge_tracks      alias MERGE_TRACKS      pattern _ op [_, _, _]
operator transpose_track   alias TRANSPOSE_TRACK   pattern _ op [_, _]
operator transpose_midi    alias TRANSPOSE_MIDI    pattern _ op [_]
operator extract_lyrics    alias EXTRACT_LYRICS    pattern _ op [_, _, _]
operator contains_words    alias CONTAINS_WORDS    pattern _ op [_, _ ,_ ,_]
operator contains_sequence alias CONTAINS_SEQUENCE pattern _ op [_, _, _]
operator delete_track      alias DELETE_TRACK      pattern _ op [_]
operator expand_track      alias EXPAND_TRACK      pattern _ op [_]
operator tempo_ms          alias TEMPO_MS          pattern _ op
operator tempo_bpm         alias TEMPO_BPM         pattern _ op
operator format            alias FORMAT            pattern _ op
operator count_tracks      alias COUNT_TRACKS      pattern _ op
operator track_name        alias TRACK_NAME        pattern _ op [_]
operator time_signature    alias TIME_SIGNATURE    pattern _ op
operator beat              alias BEAT              pattern _ op
operator instrument_name   alias INSTRUMENT_NAME   pattern _ op [_]
operator count_channels    alias COUNT_CHANNELS    pattern _ op [_]
operator get_name          alias GET_NAME          pattern _ op

# Operator saveto is also defined in the BinaryFileAlgebra
operator saveto          alias SAVETO            pattern _ infixop _

