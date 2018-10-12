operator streamvalve     alias STREAMVALVE     pattern _ op [_,_]
operator streamnext      alias STREAMNEXT      pattern op (_,_)
operator m2mm            alias M2MM            pattern _ op
operator mm2m            alias MM2M            pattern _ op
operator barrier         alias BARRIER         pattern op (_,_)
operator appendpositions alias APPENDPOSITIONS pattern _ op [_,_]
operator enterwormhole   alias ENTERWORMHOLE   pattern _ op [_]
operator leavewormhole   alias LEAVEWORMHOLE   pattern op(_)

# Operators "copied" from TemporalAlgebra:
operator atperiods alias ATPERIODS pattern _ infixop _
operator bbox alias BBOX pattern op ( _, _ )
operator translateappendS alias TRANSLATEAPPENDS pattern _ op [_, _ ]
operator trajectory alias TRAJECTORY pattern op ( _ )
