

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
operator pquery2 alias PQUERY2  pattern _ op[_,_,_,_,_]

operator put alias PUT pattern op (_,_,_)
operator get alias GET pattern op (_,_)
operator size alias SIZE pattern op (_)
operator getWorkers alias getWorkers pattern op (_)

operator fconsume5 alias FCONSUME5 pattern _ op [_]
operator ffeed5 alias FFEED5 pattern op(_)

operator createDarray2 alias CREATEDARRAY2 pattern _ op[_,_,_,_,_,_]

operator pput alias PPUT pattern _ op [_,_]
operator ddistribute2 alias DDISTRIBUTE2 pattern _ op [_,_,_,_]
operator ddistribute3 alias DDISTRIBUTE3 pattern _ op [_,_,_,_]
operator ddistribute4 alias DDISTRIBUTE4 pattern _ op [_,fun,_,_] implicit parameter elem type STREAMELEM

operator fdistribute5 alias FDISTRIBUTE5 pattern _ op [_,_,_]
operator fdistribute6 alias FDISTRIBUTE6 pattern _ op [_,_]

operator closeWorkers alias closeWorkers pattern op(_)
operator showWorkers alias showWorkers pattern op(_)

operator dloop1 alias DLOOP1 pattern _ op[_,fun] implicit parameter darray2elem type DARRAY2ELEM

operator dloop2a alias DLOOP2A pattern _ _ op[_,fun] implicit parameters elem1, elem2 types DARRAY2ELEM, DARRAY2ELEM2

operator dmap2 alias DMAP2 pattern _ _ op[_,fun] implicit parameters elem1, elem2 types ARRAYFUNARG1, ARRAYFUNARG2

operator dsummarize2 alias DSUMMARIZE2 pattern _ op

operator getValue alias GETVALUE pattern _op 

operator deleteRemoteObjects alias DELETEREMOTEOBJECTS pattern op(_,_)

operator clone alias CLONE pattern _ op[_]

operator share alias SHARE pattern op(_,_,_)

operator cleanUp alias CLEANUP pattern op(_,_)

operator fddistribute2 alias FDDISTRIBUTE2 pattern _ op [_,_,_,_,_]
operator fddistribute3 alias FDDISTRIBUTE3 pattern _ op [_,_,_,_]
operator fddistribute4 alias FDDISTRIBUTE4 pattern _ op [fun,_,_,_] implicit parameter elem type STREAMELEM

operator gettuples alias GETTUPLES pattern _ _ op

operator transferFileServer alias TRANSFERFILESERVER pattern op(_)

operator receiveFileClient alias RECEIVEFILECLIENT pattern op(_,_,_,_)

operator transferFile alias TRANSFERFILE pattern op(_,_,_,_,_)

operator traceCommands alias TRACECOMMANDS pattern op(_)


operator staticFileTransferator alias STATICFILETRANSFERATOR pattern op(_,_)
operator killStaticFileTransferator alias KILLSTATICFILETRANSFERATOR pattern op(_)
operator putFileTCP alias PUTFILETCP pattern op(_,_,_,_,_)
operator getFileTCP alias GETFILETCP pattern op(_,_,_,_,_)

operator fsfeed5 alias FSFEED5 pattern _ op[_]

operator fdistribute7 alias FDISTRIBUTE7 pattern _ op[_,fun,_,_] implicit parameter elem type STREAMELEM

operator fsfeed5 alias FSFEED5 pattern _ op[_]

operator dloop2schedule alias DLOOP2SCHEDULE pattern _ _ op[_,fun] implicit parameters elem1, elem2 types DARRAY2ELEM,  DARRAY2ELEM2
operator schedule alias SCHEDULE pattern _ _ op



