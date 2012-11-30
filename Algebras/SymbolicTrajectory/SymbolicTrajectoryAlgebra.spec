operator topattern alias TOPATTERN pattern _ op
operator matches alias MATCHES pattern _ infixop _
operator filterMatches alias FILTERMATCHES pattern _ op [ _ , _ ]
operator rewrite alias REWRITE pattern _ infixop _
operator compress alias COMPRESS pattern op ( _ )
operator fillgaps alias FILLGAPS pattern op ( _ , _ )
operator createml alias CREATEML pattern op ( _ , _ )
operator createmlrelation alias CREATEMLRELATION pattern op ( _ , _ , _ )
operator index alias INDEX pattern op(_)