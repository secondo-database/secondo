operator write alias WRITE pattern _ op [_,_]
operator checkdbservicestatus alias CHECKDBSERVICESTATUS pattern op()
operator addnode alias ADDNODE pattern op(_,_,_)
operator initdbserviceworker alias INITDBSERVICEWORKER pattern op()
operator getconfigparam alias GETCONFIGPARAM pattern op(_,_)
operator read alias READ pattern op(_)
operator read2 alias READ2 pattern _ op [fun] implicit parameter elem type DBSARG
operator read3 alias READ3 pattern _ op [fun] implicit parameter elem type DBRARG
operator ddelete alias DDELETE pattern op(_,_)
operator settracelevel alias SETTRACELEVEL pattern op(_)
operator pingdbservice alias PINGDBSERVICE pattern op()
operator startdbservice alias STARTDBSERVICE pattern op()
operator rderive alias RDERIVE pattern _ op[_,fun] implicit parameter elem type RELARG
operator useincrementalmetadataupdate alias USEINCREMENTALMETADATAUPDATE pattern op(_)