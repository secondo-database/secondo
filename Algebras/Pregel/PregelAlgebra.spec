operator messageDistribute alias MESSAGEDISTRIBUTE pattern _ op

operator messageFeed alias MESSAGEFEED pattern op ()

operator startMessageClient alias STARTMESSAGECLIENT pattern op (_,_,_)

operator startMessageServer alias STARTMESSAGESERVER pattern op (_)

operator setupPregel alias SETUPPREGEL pattern op (_)

operator initPregelMessages alias INITPREGELMESSAGES pattern _ op

operator startLoopbackMessageClient alias STARTLOOPBACKMESSAGECLIENT pattern op (_)

operator setPregelFunction alias SETPREGELFUNCTION pattern op (_)

operator startPregel alias STARTPREGEL pattern op (_)

operator resetPregel alias RESETPREGEL pattern op ()

operator expectPregelMessages alias EXPECTPREGELMESSAGES pattern op ()

operator pregelStatus alias PREGELSTATUS pattern op ()

operator setPregelFunctionWorker alias SETPREGELFUNCTIONWORKER pattern op (_,_)

operator startPregelWorker alias STARTPREGELWORKER pattern op (_)

operator remotePregelCommand alias REMOTEPREGELCOMMAND pattern op (_)