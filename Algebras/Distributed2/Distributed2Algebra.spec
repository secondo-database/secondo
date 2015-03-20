

operator connect alias CONNECT pattern op(_,_,_)
operator checkConnections alias CHECKCONNECTIONS pattern op()
operator rcmd alias RCMD pattern op(_,_)
operator disconnect alias DISCONNECT pattern op(_,_)
operator rquery alias rquery pattern op(_,_)
operator prcmd alias PRCMD pattern _ op [_,_]
operator sendFile alias SENDFILE pattern op(_,_,_)
operator requestFile alias REQUESTFILE pattern op(_,_,_)
operator psendFile alias PSENDFILE  pattern _ op [_,_,_]
operator prequestFile alias PREQUESTFILE  pattern _ op [_,_,_]
operator getRequestFolder alias GETREQUESTFOLDER pattern op(_)
operator getSendFolder alias GETSENDFOLDER pattern op(_)
operator pconnect alias PCONNECT pattern _ op[_,_,_]


