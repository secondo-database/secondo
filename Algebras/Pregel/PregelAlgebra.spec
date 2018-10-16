operator messageDistribute alias MESSAGEDISTRIBUTE pattern _ op [_]

operator messageFeed alias MESSAGEFEED pattern op ()

operator startMessageClient alias STARTMESSAGECLIENT pattern op (_,_,_)

operator startMessageServer alias STARTMESSAGESERVER pattern op (_)

operator setupPregel alias SETUPPREGEL pattern op (_)

operator initPregelMessages alias INITPREGELMESSAGES pattern _ op

operator startLoopbackMessageClient alias STARTLOOPBACKMESSAGECLIENT pattern op (_)

operator preparePregel alias PREPAREPREGEL pattern op (_)

operator startPregel alias STARTPREGEL pattern op (_)

operator resetPregel alias RESETPREGEL pattern op ()

operator initPregelMessageWorker alias INITPREGELMESSAGEWORKER pattern op ()

operator pregelHealth alias PREGELHEALTH pattern op ()

operator preparePregelWorker alias PREPAREPREGELWORKER pattern op (_,_)

operator startPregelWorker alias STARTPREGELWORKER pattern op (_)