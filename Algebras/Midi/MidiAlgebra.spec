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
# operator saveto          alias SAVETO            pattern _ infixop _

