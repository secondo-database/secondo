



operator insertRindex  alias INSERTRINDEX  pattern _ op 
operator findRindex    alias   FINDRINDEX  pattern _ op [ _, _ ]
operator heightRindex  alias HEIGHTRINDEX  pattern _ op 

operator statRindex  alias STATRINDEX  pattern _ op[_ ] 

operator joinRindex    alias   JOINRINDEX  pattern _ _ op [ _, _ ]
operator symmJoinRindex    alias   SYMMJOINRINDEX  pattern _ _ op [ _, _ ]
operator realJoinRindex    alias   REALJOINRINDEX  pattern _ _ op [ _, _ ]


operator realJoinMMRTree    alias   REALJOINMMRTREE  pattern _ _ op [ _, _ ,_,_]
operator statMMRTree    alias   STATMMRTREE  pattern _ op [ _, _, _]
operator insertMMRTree    alias   INSERTMMRTREE  pattern _ op [ _, _ ]

