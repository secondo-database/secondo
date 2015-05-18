

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
operator pquery alias PQUERY  pattern _ op[_,_]

operator put alias PUT pattern op (_,_,_)
operator get alias GET pattern op (_,_)
operator size alias SIZE pattern op (_)
operator getWorkers alias getWorkers pattern op (_)

operator fconsume5 alias FCONSUME5 pattern _ op [_]
operator ffeed5 alias FFEED5 pattern op(_)

operator createDarray2 alias CREATEDARRAY2 pattern _ op[_,_,_,_,_,_]

operator pput alias PPUT pattern _ op [_,_]
operator ddistribute2 alias DDISTRIBUTE2 pattern _ op [_,_]
operator fdistribute5 alias FDISTRIBUTE5 pattern _ op [_,_,_]

operator closeWorkers alias closeWorkers pattern op(_)
operator showWorkers alias showWorkers pattern op(_)

operator dloop2 alias DLOOP2 pattern _ op[_,fun] implicit parameter darray2elem type DARRAY2ELEM



