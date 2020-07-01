
import secondodb.api.secondoapi as secondo

import secondodb.api.algebras.secondospatialalgebra as spatial
import secondodb.api.support.secondolistexpr as le

import datetime

HOST = '127.0.0.1'  # The server's hostname or IP address
PORT = '1234'        # The port used by the server
user = 'username'
pswd = 'passwd'

# Beispiel 1 - Verbindung zum Secondo-Server

connection = secondo.connect(HOST, PORT, database='berlintest')
cursor = connection.cursor()

result = cursor.execute('query test_mbool3')

print(result)





# test_mregion = 'let test_mregion = [const rel(tuple([Name: string, GeoData: mregion])) value ()]'

# print(result)

# Beispiel 2 - Erzeugung eines Cursor-Objekts

# cursor = connection.cursor()
#
# seq_of_parameters = []
#
# seq_of_parameters.append(['mehringdamm'])
# seq_of_parameters.append(['alexanderplatz'])
# seq_of_parameters.append(['mehringdamm'])
# seq_of_parameters.append(['alexanderplatz'])
# seq_of_parameters.append(['mehringdamm'])
# seq_of_parameters.append(['alexanderplatz'])
#
# cursor.executemany('query {0}', seq_of_parameters)
#
# result = cursor.fetchmany(7)
# for single in result:
#     print(single)
#
# print('fetchall')
#
# result = cursor.fetchall()
# for single in result:
#     print(single)

# result = cursor.execute_simple_query('myfirstrel inserttuple["Straße", 27] count;')
# print(result)

# result = cursor.execute('query koepenick')
# print(result)
# string_res = spatial.convert_region_to_list_exp_str(result)
# print(string_res)

# let_str = 'let testrelkoepenick = [const rel(tuple([Name: string, GeoData: region])) value (("koepenick" ' + string_res + '))]'
# print(let_str)
# Beispiel 3 - Ausführung einer Query (BERLINTEST - 1)

# connection.close_database()
# connection.open_database('creationtest')

# tuple_values = ['"koepenick from api"', string_res]
# tuple_types = ['string', 'region']

# rel = cursor.execute_insert_tuple_into_relation('testrelkoepenick2', tuple_values, tuple_types)

# result = connection.get_list_objects()

#
# pprint(dict(vars(mpoint)))

# result = cursor.execute('query Kinos')
# first_element = result.get_first_element()  # The first element contains always the information of the type
# second_element = result.get_second_element()  # The second element contains the data
# third_element = result.get_third_element()
#
# parser.parse_type_definition(first_element)
#
# print(result.get_list_length())
# print(first_element.get_list_length())
# print(second_element.get_list_length())
# line = spatial.parse_line(second_element)
# print(result)

# print(cursor.execute('query plz feed filter[.Ort starts "A"] plz feed filter[.Ort starts "A"] {a} '
#                      'symmjoin[ (tolower(.Ort) contains tolower(..Ort_a)) and (not( .Ort = ..Ort_a)) ] consume;'))

# Beispiel 4 - Ausführung einer Query (BERLINTEST - 2)

# print(cursor.execute('query Orte feed plz feed {a} itHashJoin[Ort, Ort_a] '
#                     'remove[Ort_a] renameAttr[PLZ : PLZ_a] consume;'))

# Beispiel 5 - Erzeugung einer neuen DB

# connection.create_database('HAGENDB')

# Beispiel 6 - Anlegen einer neuen Relation (Methode execute_let)

# print(cursor.execute_let('myfirstrelation', '[ const rel(tuple([Name : string, Age : int])) value () ]'))
