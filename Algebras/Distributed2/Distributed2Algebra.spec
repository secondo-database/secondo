

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

operator createDArray alias CREATEDARRAY pattern _ op[_,_,_,_,_,_]

operator pput alias PPUT pattern _ op [_,_]
operator ddistribute2 alias DDISTRIBUTE2 pattern _ op [_,_,_,_]
operator ddistribute3 alias DDISTRIBUTE3 pattern _ op [_,_,_,_]
operator ddistribute4 alias DDISTRIBUTE4 pattern _ op [fun,_,_,_] implicit parameter elem type STREAMELEM

operator fdistribute5 alias FDISTRIBUTE5 pattern _ op [_,_,_]
operator fdistribute6 alias FDISTRIBUTE6 pattern _ op [_,_]

operator closeWorkers alias closeWorkers pattern op(_)
operator showWorkers alias showWorkers pattern op(_)

operator dloop alias DLOOP pattern _ op[_,fun] implicit parameter darrayelem type DARRAYELEM


operator dmap alias DMAP pattern _ op[_,fun] implicit parameter dmapelem type ARRAYFUNARG1

operator dloopa alias DLOOPA pattern _ _ op[_,fun] implicit parameters elem1, elem2 types DARRAYELEM, DARRAYELEM2

operator dmap2 alias DMAP2 pattern _ _ op[_,fun] implicit parameters elem1, elem2 types ARRAYFUNARG1, ARRAYFUNARG2

operator dsummarize alias DSUMMARIZE pattern _ op

operator getValue alias GETVALUE pattern _op 

operator deleteRemoteObjects alias DELETEREMOTEOBJECTS pattern op(_,_)

operator clone alias CLONE pattern _ op[_]

operator share alias SHARE pattern op(_,_,_)

operator cleanUp alias CLEANUP pattern op(_,_)

operator dfdistribute alias DFDISTRIBUTE pattern _ op [_,_,_,_,_]
operator dfdistribute3 alias DFDISTRIBUTE3 pattern _ op [_,_,_,_]
operator dfdistribute4 alias DFDISTRIBUTE4 pattern _ op [fun,_,_,_] implicit parameter elem type STREAMELEM

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

operator fdistribute7 alias FDISTRIBUTE7 pattern _ op[fun,_,_,_] implicit parameter elem type STREAMELEM

operator partition alias PARTITION pattern _ op[fun,_,_] implicit parameter elem type SUBSUBTYPE1 

operator collect2 alias COLLECT2 pattern _ op[_,_]

operator areduce alias AREDUCE pattern _ op [_ , fun, _] implicit parameter elem type ARRAYFUNARG1 



