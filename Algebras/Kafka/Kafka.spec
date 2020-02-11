# postfix operator with an parameter
operator writetokafka alias WRITETOKAFKA pattern _ op [_ ]

#simple prefix operator
operator readfromkafka alias READFROMKAFKA pattern op(_)

# postfix operator with an parameter
operator finishStream alias FINISHSTREAM pattern _ op [_ ]

#simple prefix operator
operator signalFinish alias SIGNALFINISH pattern op(_)

# postfix operator with an parameter
operator consoleConsumer alias CONSOLECONSUMER pattern _ op

#simple prefix operator
operator readfromwebsocket alias READFROMWEBSOCKET pattern op(_)

#simple prefix operator
operator installLocalKafka alias INSTALLLOCALKAFKA pattern op(_)

#simple prefix operator
operator startLocalKafka alias STARTLOCALKAFKA pattern op(_)

#simple prefix operator
operator stopLocalKafka alias STOPLOCALKAFKA pattern op(_)

#simple prefix operator
operator statusLocalKafka alias STATUSLOCALKAFKA pattern op(_)

