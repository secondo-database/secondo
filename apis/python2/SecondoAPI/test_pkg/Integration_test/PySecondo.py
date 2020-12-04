#Normal case and pysend



from PySecondo import *


com1 = 'open database berlintest'
com2 = 'sql select * from staedte where bev > 900000'
com3 = 'query plz feed head[10] pysend[30000] consume'
com4 = 'query plz feed head[16] pysend[30000] count'
com5 = 'let ReviewsSchema1 = [const rel(tuple([Plz: int, Ort: string])) value ()]'
com6 = 'let Reviews = ReviewsSchema1 pyreceive[30000] consume'
com7 = 'query Reviews feed consume'
com8 = 'sql select * from Reviews'
com9 = 'query [ const rel(tuple([Plz: int, Ort: string])) value() ] pyreceive[30000] count'
com10 = 'query [ const rel(tuple([Plz: int, Ort: string])) value() ] pyreceive[30000] consume'
com11 = 'query plz feed head[10] count'
com12 = 'query Staedte feed filter[(.Bev > 100000)] count'
com13 = 'query Staedte  feed head[2] filter[(.Bev > 100000)] {1}  consume'
com14 = 'list algebras'
com15 = 'list algebra PyStreamAlgebra'
com16 = 'restore database berlintest from berlintest'
com17 = 'close database'



scn = Secondo()

#Openning a database
result = await scn.command_exec(com1)
assert scn.get_result_error_code() == 0, 'The execution of open database command was not successful.'

#A sql-query
result = await scn.command_exec(com2)
assert scn.get_result_error_code() == 0, 'The execution of query was not successful.'
assert scn.fetch_result_type() == ['rel', ['tuple', [['SName', 'string'], ['Bev', 'int'], ['PLZ', 'int'], ['Vorwahl', 'string'], ['Kennzeichen', 'string']]]], 'Unexpected result type of the query.'
assert scn.fetch_result_rows() == [['"Berlin"', 1859000, 1000, '"030"', '"B"'], ['"Hamburg"', 1580000, 2000, '"040"', '"HH"'], ['"Koeln"', 916000, 5000, '"0221"', '"K"'], ['"Muenchen"', 1267000, 8000, '"089"', '"M"']], 'Unexpected result of the query.'


#An executable-level query

result = await scn.command_exec(com13)
assert scn.get_result_error_code() == 0, 'The execution of query was not successful.'
assert scn.fetch_result_type() == ['rel', ['tuple', [['SName', 'string'], ['Bev', 'int'], ['PLZ', 'int'], ['Vorwahl', 'string'], ['Kennzeichen', 'string']]]], 'Unexpected result type of the query.'
assert scn.fetch_result_rows() == [['"Aachen"', 239000, 5100, '"0241"', '"AC"'], ['"Berlin"', 1859000, 1000, '"030"', '"B"']], 'Unexpected result of the query.'


#Restore a database
result = await scn.command_exec(com16)
assert scn.get_result_error_code() == 0, 'The execution of restore database command was not successful.'



#Receiving the result of secondo as stream of tupels with pysend[]








#print(scn.fetch_result())
#print(scn.fetch_result_type())
#print(scn.fetch_result_rows())






#scn.parse_result_to_secondo_int()
#scn.parse_result_to_relation()



#pysend
#for item in scn.fetch_stream_result():
    #print(item)
#scn.parse_stream_result_to_relation()
    
#scn.close()

