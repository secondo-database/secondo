
operator fsfeed5 alias FSFEED5 pattern _ op[_]
#operator partitiondmap alias PARTITIONDMAP pattern _ op[_,fun,_,fun] error: symbol fun is used, but is not defined as a token and has no rules
operator partitiondmap alias PARTITIONDMAP pattern _ op[_,fun,_,fun] implicit parameter elem type PDTS
operator fdistribute2tes alias FDISTRIBUTE2TES pattern _ op[_,fun,_,_] implicit parameter elem type STREAMELEM
operator distribute2tes alias DISTRIBUTE2TES pattern _ op[_,fun,_,_] implicit parameter elem type STREAMELEM
operator setuptes alias SETUPTES pattern op (_)
operator resettes alias RESETTES pattern op()
operator killtes alias KILLTES pattern op()
operator setTupleType alias SETTUPLETYPE pattern op (_,_)
operator startLoopbackTESClient alias STARTLOOPBACKTESCLIENT pattern op (_)
operator startTESServer alias STARTTESSERVER pattern op (_)
operator startTESClient alias STARTTESCLIENT pattern op (_,_,_,_)
operator feedtes alias FEEDTES pattern op (_,_)
operator testests alias TESTESTS pattern op (_,_)


