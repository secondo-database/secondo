

operator connect alias CONNECT pattern op(_,_,_)
operator checkConnections alias CHECKCONNECTIONS pattern op()
operator rcmd alias RCMD pattern op(_,_)
operator disconnect alias DISCONNECT pattern op(_,_)
operator rquery alias rquery pattern op(_,_)
operator prcmd alias PRCMD pattern _ op [_,_]
operator transferFile alias TRANSFERFILE pattern op(_,_,_)
operator requestFile alias REQUESTFILE pattern op(_,_,_)
operator ptransferFile alias PTRANSFERFILE  pattern _ op [_,_,_]
