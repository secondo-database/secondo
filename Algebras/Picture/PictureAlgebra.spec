operator getWidth alias GETWIDTH pattern _ op
operator getHeight alias GETHEIGHT pattern _ op
operator isgrayscale alias ISGRAYSCALE pattern _ op
operator getFilename alias GETFILENAME pattern _ op
operator getCategory alias getCATEGORY pattern _ op
operator getPictureDate alias GETPICTUREDATE pattern _ op
operator isportrait alias ISPORTRAIT pattern _ op
operator colordist alias COLORDIST pattern _ op [ _ ]
operator equals alias EQUALS pattern _  _ op [ _, _ ]
#operator contains alias CONTAINS pattern _ infixop _
operator simpleequals alias SIMPLEEQUALS pattern _ infixop _
operator like alias LIKE pattern _ op [ _, _, _, _ ]
operator cut alias CUT pattern _ op [ _, _, _, _ ]
operator scale alias SCALE pattern _ op [ _, _ ]
operator flipleft alias FLIPLEFT pattern _ op [ _ ]
operator mirror alias MIRROR pattern _ op [ _ ]
operator display alias DISPLAY pattern _ op
operator export alias EXPORT pattern _ op [ _ ]

operator distance alias DISTANCE pattern op(_,_)
operator getHistHsv8 alias GETHISTHSV8 pattern op(_)
operator getHistHsv16 alias GETHISTHSV16 pattern op(_)
operator getHistHsv32 alias GETHISTHSV32 pattern op(_)
operator getHistHsv64 alias GETHISTHSV64 pattern op(_)
operator getHistHsv128 alias GETHISTHSV128 pattern op(_)
operator getHistHsv256 alias GETHISTHSV256 pattern op(_)
operator getHistLab256 alias GETHISTLAB256 pattern op(_)

operator importpicture alias IMPORTPICTURE pattern op(_)
