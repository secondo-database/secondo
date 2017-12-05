operator write alias WRITE pattern _ op [_,_]
operator checkdbservicestatus alias CHECKDBSERVICESTATUS pattern op()
operator addnode alias ADDNODE pattern op(_,_,_)
operator initdbserviceworker alias INITDBSERVICEWORKER pattern op()
operator getconfigparam alias GETCONFIGPARAM pattern op(_,_)
operator read alias READ pattern op(_)
operator read2 alias READ2 pattern _ op [fun] implicit parameter elem type DBSARG
operator read3 alias READ3 pattern _ op [fun] implicit parameter elem type DBRARG
operator ddelete alias DDELETE pattern op(_,_)
operator ddeleteDB alias DDELETEDB pattern op(_,_)
operator settracelevel alias SETTRACELEVEL pattern op(_)
operator pingdbservice alias PINGDBSERVICE pattern op()
operator startdbservice alias STARTDBSERVICE pattern op()
operator rderive alias RDERIVE pattern _ op[_,fun] implicit parameter elem type RELARG
operator read3_1 alias READ3_1 pattern _ _ op[fun] implicit parameters elem1, elem2 types DBRARG, DBIARG1
operator read3_2 alias READ3_2 pattern _ _ _ op[fun] implicit parameters elem1, elem2, elem3 types DBRARG, DBIARG1, DBIARG2
operator useincrementalmetadataupdate alias USEINCREMENTALMETADATAUPDATE pattern op(_)

