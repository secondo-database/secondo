operator appendto    alias APPENDTO    pattern _ infixop _
operator streamvalve alias STREAMVALVE pattern _ op [_,_]
operator streamnext  alias STREAMNEXT  pattern op (_,_)
operator m2mm        alias M2MM        pattern _ op
operator mm2m        alias MM2m        pattern _ op
operator barrier     alias BARRIER     pattern op (_,_)
