
# postfix operator with an parameter
operator kafka alias KAFKA pattern _ op [_ ]

#simple prefix operator
operator kafkastream alias KAFKASTREAM pattern op(_)

# postfix operator with an parameter
operator finishStream alias FINISHSTREAM pattern _ op [_ ]

#simple prefix operator
operator signalFinish alias SIGNALFINISH pattern op(_)

# postfix operator with an parameter
operator consoleConsumer alias CONSOLECONSUMER pattern _ op
