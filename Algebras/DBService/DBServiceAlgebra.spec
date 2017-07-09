operator letdconsume alias LETDCONSUME pattern _ op [_]
operator feedpf alias FEEDPF pattern _ op [_,_]
operator checkdbservicestatus alias CHECKDBSERVICESTATUS pattern op()
operator addnode alias ADDNODE pattern op(_,_,_)
operator initdbserviceworker alias INITDBSERVICEWORKER pattern op()
operator getconfigparam alias GETCONFIGPARAM pattern op(_,_)
operator read alias READ pattern _ op[fun] implicit parameter elem type rel
operator ddelete alias DDELETE pattern op(_,_)
operator settracelevel alias SETTRACELEVEL pattern op(_)