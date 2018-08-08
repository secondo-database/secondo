

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
operator getRequestFolder alias GETREQUESTFOLDER pattern op(_,_)
operator getSendFolder alias GETSENDFOLDER pattern op(_,_)
operator pconnect alias PCONNECT pattern _ op[_,_,_]
operator prquery alias PRQUERY  pattern _ op[_,_]
operator prquery2 alias PRQUERY2  pattern _ op[_,_,_,_,_]

operator put alias PUT pattern op (_,_,_)
operator get alias GET pattern op (_,_)
operator size alias SIZE pattern op (_)
operator getWorkers alias getWorkers pattern op (_)

operator fconsume5 alias FCONSUME5 pattern _ op [_]
operator ffeed5 alias FFEED5 pattern _ op 

operator createDArray alias CREATEDARRAY pattern _ op[_,_,_]

operator pput alias PPUT pattern _ op [_,_]
operator ddistribute2 alias DDISTRIBUTE2 pattern _ op [_,_,_,_]
operator ddistribute3 alias DDISTRIBUTE3 pattern _ op [_,_,_,_]
operator ddistribute4 alias DDISTRIBUTE4 pattern _ op [_,fun,_,_] implicit parameter elem type STREAMELEM

operator fdistribute5 alias FDISTRIBUTE5 pattern _ op [_,_,_]
operator fdistribute6 alias FDISTRIBUTE6 pattern _ op [_,_]
operator fdistribute7 alias FDISTRIBUTE7 pattern _ op[_,fun,_,_] implicit parameter elem type STREAMELEM

operator closeWorkers alias closeWorkers pattern op(_)
operator showWorkers alias showWorkers pattern op(_)

operator dloop alias DLOOP pattern _ op[_,fun] implicit parameter darrayelem type ARRAYFUNARG1 
operator dloop2 alias DLOOP2 pattern _ _ op[_,fun] implicit parameters elem1, elem2 types DARRAYELEM, DARRAYELEM2


operator dmap alias DMAP pattern _ op[_,fun] implicit parameter dmapelem type ARRAYFUNARG1
operator dmap2 alias DMAP2 pattern _ _ op[_,fun,_] implicit parameters elem1, elem2 types ARRAYFUNARG1, ARRAYFUNARG2


operator dmap2n alias DMAP2n pattern _ _ op[_,fun,_] implicit parameters elem1, elem2 types ARRAYFUNARG1, ARRAYFUNARG2


operator dmap3 alias DMAP3 pattern _ _ _ op[_,fun,_] implicit parameters elem1, elem2, elem3  types ARRAYFUNARG1, ARRAYFUNARG2, ARRAYFUNARG3
operator dmap4 alias DMAP4 pattern _ _ _ _ op[_,fun,_] implicit parameters elem1, elem2, elem3, elem4  types ARRAYFUNARG1, ARRAYFUNARG2, ARRAYFUNARG3, ARRAYFUNARG4
operator dmap5 alias DMAP5 pattern _ _ _ _ _ op[_,fun,_] implicit parameters elem1, elem2, elem3, elem4 , elem5  types ARRAYFUNARG1, ARRAYFUNARG2, ARRAYFUNARG3, ARRAYFUNARG4, ARRAYFUNARG5
operator dmap6 alias DMAP6 pattern _ _ _ _ _ _ op[_,fun,_] implicit parameters elem1, elem2, elem3, elem4, elem5, elem6  types ARRAYFUNARG1, ARRAYFUNARG2, ARRAYFUNARG3, ARRAYFUNARG4, ARRAYFUNARG5, ARRAYFUNARG6
operator dmap7 alias DMAP7 pattern _ _ _ _ _ _ _ op[_,fun,_] implicit parameters elem1, elem2, elem3, elem4, elem5, elem6, elem7  types ARRAYFUNARG1, ARRAYFUNARG2, ARRAYFUNARG3, ARRAYFUNARG4, ARRAYFUNARG5, ARRAYFUNARG6, ARRAYFUNARG7
operator dmap8 alias DMAP8 pattern _ _ _ _ _ _ _ _ op[_,fun,_] implicit parameters elem1, elem2, elem3, elem4, elem5, elem6, elem7, elem8  types ARRAYFUNARG1, ARRAYFUNARG2, ARRAYFUNARG3, ARRAYFUNARG4, ARRAYFUNARG5, ARRAYFUNARG6, ARRAYFUNARG7, ARRAYFUNARG8

operator dsummarize alias DSUMMARIZE pattern _ op

operator getValue alias GETVALUE pattern _op 

operator deleteRemoteObjects alias DELETEREMOTEOBJECTS pattern op(_,_)

operator clone alias CLONE pattern _ op[_]

operator share alias SHARE pattern op(_,_,_)
operator share2 alias SHARE2 pattern op(_,_,_,_)

operator cleanUp alias CLEANUP pattern op(_,_)

operator dfdistribute2 alias DFDISTRIBUTE2 pattern _ op [_,_,_,_,_]
operator dfdistribute3 alias DFDISTRIBUTE3 pattern _ op [_,_,_,_]
operator dfdistribute4 alias DFDISTRIBUTE4 pattern _ op [_,fun,_,_] implicit parameter elem type STREAMELEM

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


operator partition alias PARTITION pattern _ op[_,fun,_] implicit parameter elem type SUBSUBTYPE1 
operator partitionF alias PARTITIONF pattern _ op[_,fun,fun,_] implicit parameters elem1,elem2 types FFR, FFR 

operator collect2 alias COLLECT2 pattern _ op[_,_]

operator areduce alias AREDUCE pattern _ op [_ , fun, _] implicit parameter elem type AREDUCEARG1 
operator areduce2 alias AREDUCE2 pattern _ _ op [_ , fun, _] implicit parameters elem1, elem2 types AREDUCEARG1 , AREDUCEARG2


operator saveAttr alias SAVEATTR pattern _ op[_] 
operator loadAttr alias LOADATTR pattern  op(_) 

operator createFrel alias CREATEFREL pattern op(_,_,_)

operator createFSrel alias CREATEFSREL pattern _ op[_]

operator saveObjectToFile alias SAVEOBJECTTOFILE pattern _ op[_]

operator getObjectFromFile alias GETOBJECTFROMFILE pattern _ op

operator dproduct alias DPRODUCT  pattern _ _ op[_,fun,_] implicit parameters elem1, elem2 types DPRODUCTARG1, DPRODUCTARG2 

operator ddistribute8  alias DDISTRIBUTE pattern _ op[_,fun, fun,_,_, _] implicit parameter elem type STREAMELEM
operator dfdistribute8  alias DFDISTRIBUTE pattern _ op[_,fun, fun,_,_, _] implicit parameter elem type STREAMELEM

operator partition8Local alias PARTITION8LOCAL pattern _ op[ fun,fun, _,_,_,_,_] implicit parameter elem type STREAMELEM

operator partitionF8 alias PARTITIONF8 pattern  _ op [ _, fun, fun, fun, _,_] implicit parameter elem type P8TM 

operator da2enableLog alias DA2ENABLELOG pattern op(_)
operator da2clearLog  alias DA2CLEARLOG pattern op()
operator da2Log alias DA2LOG pattern op()

operator deleteRemoteDatabases alias DELETEREMOTEDATABASES pattern op(_,_)

operator setAccount alias SETACCOUNT pattern op(_,_)

operator writeRel alias WRITEREL pattern _ op[_]

operator write2 alias WRITE2 pattern _ op
operator write3 alias WRITE3 pattern _ op

operator db2LogToFile alias DB2LOGTOFILE pattern op(_,_)

operator enableDFSFT alias ENABLEDSFFT pattern op(_,_)
operator disableDFSFT alias DISABLEDSFFT pattern op()
operator removeTempInDFS alias REMOVETEMPINDFS pattern op()
operator removeDFSFilesInDB alias REMOVEDFSFILESINDB pattern op()
operator removeAllDFSFiles alias REMOVEALLDFSFILES pattern op()

operator createintdarray alias CREATEINTDARRAY pattern op(_,_)
operator dcommand alias DCOMMAND pattern _ op[_]
operator dlet alias DLET pattern _ op[_,_]

operator makeSimple alias MAKESIMPLE pattern _ op[_,_]
operator makeDArray alias MAKEDARRAY pattern _ op[_,_]




