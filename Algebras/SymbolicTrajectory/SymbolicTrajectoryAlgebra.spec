operator tolabel alias TOLABEL pattern op ( _ )
operator mstringtomlabel alias MSTRINGTOMLABEL pattern op ( _ )
operator contains alias CONTAINS pattern _ infixop _
operator topattern alias TOPATTERN pattern _ op
operator toclassifier alias TOCLASSIFIER pattern _ op
operator matches alias MATCHES pattern _ infixop _
operator indexmatches alias INDEXMATCHES pattern _ op [ _ , _ , _ ]
operator filtermatches alias FILTERMATCHES pattern _ op [ _ , _ ]
operator rewrite alias REWRITE pattern op ( _ , _ )
operator classify alias CLASSIFY pattern op ( _ , _ )
operator indexclassify alias INDEXCLASSIFY pattern _ op [ _ , _ , _ ]
operator compress alias COMPRESS pattern op ( _ )
operator fillgaps alias FILLGAPS pattern op ( _ , _ )
operator createml alias CREATEML pattern op ( _ , _ )
operator createmlrelation alias CREATEMLRELATION pattern op ( _ , _ , _ )
operator createindex alias INDEX pattern op(_)
operator createtrie alias CREATETRIE pattern _ op [ _ ]
operator triptompoint alias TRIPTOMPOINT pattern _ op