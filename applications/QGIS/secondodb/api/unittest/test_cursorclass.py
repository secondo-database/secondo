from unittest import TestCase
from unittest.mock import Mock, patch

from secondodb.api.secondoapi import Connection, InternalError, OperationalError, ProgrammingError
from secondodb.api.secondoapi import InterfaceError


class TestCursor(TestCase):

    def setUp(self):
        self.connection = None

    def tearDown(self):
        if self.connection is not None:
            if self.connection.initialized:
                self.connection.close()

    def test_create_cursor_without_db(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = ''

        self.connection = Connection(HOST, PORT, user, pswd, db)

        with self.assertRaises(OperationalError):
            self.connection.cursor()

    def test_create_cursor_with_db_pre(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        self.connection = Connection(HOST, PORT, user, pswd, db)
        self.cursor = self.connection.cursor()
        self.assertTrue(self.cursor.initialized)

    def test_create_cursor_with_db_post(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = ''

        self.connection = Connection(HOST, PORT, user, pswd, db)
        self.connection.database = 'berlintest'
        result = self.connection.cursor()
        self.assertTrue(result.initialized)

    def test_create_cursor_with_non_exist_db(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = ''

        self.connection = Connection(HOST, PORT, user, pswd, db)
        self.connection.database = 'nonexistantdb'
        with self.assertRaises(InterfaceError):
            self.connection.cursor()

    def test_close_with_init(self):

        self.test_create_cursor_with_db_pre()
        self.assertTrue(self.cursor.close())

    def test_close_close_db_error(self):

        mock_close_db = Mock()
        mock_close_db.side_effect = InterfaceError('test')

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        connection.close_database = mock_close_db

        with self.assertRaises(InterfaceError):
            cursor.close()

    def test_execute_successful(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        query_response, object_type, receive_list = cursor.execute('query mehringdamm')

        self.assertIsInstance(query_response, object)
        self.assertIsInstance(object_type, str)
        self.assertIsInstance(receive_list, str)
        self.assertIsInstance(cursor.result, list)

    def test_executemany_successful(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        seq_of_parameters = [['mehringdamm'], ['alexanderplatz']]

        response_list = cursor.executemany('query {0}', seq_of_parameters)

        self.assertIsInstance(response_list, list)
        self.assertTrue(len(response_list) == 2)

    def test_fetchone_successful(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        seq_of_parameters = [['mehringdamm'], ['alexanderplatz']]

        cursor.executemany('query {0}', seq_of_parameters)
        result = cursor.fetchone()

        self.assertIsInstance(result, list)
        self.assertTrue(len(cursor.result) == 1)

    def test_fetchone_no_elements(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        seq_of_parameters = [['mehringdamm'], ['alexanderplatz']]

        cursor.executemany('query {0}', seq_of_parameters)
        cursor.fetchone()
        cursor.fetchone()
        result3 = cursor.fetchone()

        self.assertIsNone(result3)

    def test_fetchmany_successful(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        seq_of_parameters = [['mehringdamm'], ['alexanderplatz']]

        cursor.executemany('query {0}', seq_of_parameters)
        result = cursor.fetchmany(2)

        self.assertTrue(len(result) == 2)
        self.assertTrue(len(cursor.result) == 0)

    def test_fetchmany_less_elements_than_size(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        seq_of_parameters = [['mehringdamm'], ['alexanderplatz'], ['mehringdamm'], ['alexanderplatz']]

        cursor.executemany('query {0}', seq_of_parameters)
        cursor.fetchone()
        result = cursor.fetchmany(4)

        self.assertTrue(len(result) == 3)
        self.assertTrue(len(cursor.result) == 0)

    def test_fetchmany_no_rows_available(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        with self.assertRaises(InternalError):
            cursor.fetchmany(4)

    def test_fetchmany_invalid_size(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        with self.assertRaises(InterfaceError):
            cursor.fetchmany(-4)

    def test_fetchmany_no_cursor(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()
        cursor.close()

        with self.assertRaises(InternalError):
            cursor.fetchmany(1)

    def test_fetchall_no_rows_available(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        with self.assertRaises(InternalError):
            cursor.fetchall()

    def test_fetchall_no_cursor(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()
        cursor.close()

        with self.assertRaises(InternalError):
            cursor.fetchall()

    def test_fetchall_no_elements_left(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        seq_of_parameters = [['mehringdamm'], ['alexanderplatz'], ['mehringdamm'], ['alexanderplatz']]

        cursor.executemany('query {0}', seq_of_parameters)
        cursor.fetchmany(4)
        result = cursor.fetchall()

        self.assertIsInstance(result, list)
        self.assertTrue(len(result) == 0)

    def test_execute_create_empty_relation_successful(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        with patch('secondodb.api.secondoapi.Cursor.execute',
                   return_value=['Success']):
            result = cursor.execute_create_empty_relation('testrel', [['name', 'type'], ['name2', 'type2']])
            self.assertTrue(result == 'Success')

    def test_execute_create_empty_relation_no_attr(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        with patch('secondodb.api.secondoapi.Cursor.execute',
                   return_value=['Success']):

            with self.assertRaises(ProgrammingError):
                cursor.execute_create_empty_relation('testrel', [])

    def test_execute_create_empty_relation_no_cursor(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()
        cursor.close()

        with patch('secondodb.api.secondoapi.Cursor.execute',
                   return_value=['Success']):

            with self.assertRaises(InternalError):
                cursor.execute_create_empty_relation('testrel', [['name', 'type']])

    def test_execute_create_empty_relation_error(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        with patch('secondodb.api.secondoapi.Cursor.execute',
                   side_effect=ProgrammingError('test')):

            with self.assertRaises(ProgrammingError):
                cursor.execute_create_empty_relation('testrel', [['name', 'type']])

    def test_execute_insert_tuple_into_relation_successful(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        with patch('secondodb.api.secondoapi.Cursor.execute',
                   return_value=['Success']):
            result = cursor.execute_insert_tuple_into_relation('testrel', [['value', 'type'], ['value', 'type2']])
            self.assertTrue(result == 'Success')

    def test_execute_insert_tuple_into_relation_no_cursor(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()
        cursor.close()

        with patch('secondodb.api.secondoapi.Cursor.execute',
                   return_value=['Success']):

            with self.assertRaises(InternalError):
                cursor.execute_insert_tuple_into_relation('testrel', [['name', 'type']])

    def test_execute_simple_query_successful(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        result = cursor.execute_simple_query('mehringdamm')
        self.assertIsInstance(result, object)

    def test_execute_simple_query_no_cursor(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()
        cursor.close()

        with self.assertRaises(InternalError):
            cursor.execute_simple_query('mehringdamm')

    def test_execute_simple_query_error(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        with patch('secondodb.api.secondoapi.Cursor.execute',
                   side_effect=ProgrammingError('error')):
            with self.assertRaises(ProgrammingError):
                cursor.execute_simple_query('value_expresion')

    def test_execute_let_no_cursor(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()
        cursor.close()

        with self.assertRaises(InternalError):
            cursor.execute_let('identifier', 'value_expression')

    def test_execute_let_successful(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        with patch('secondodb.api.secondoapi.Cursor.execute',
                   return_value=['Success']):
            result = cursor.execute_let('identifier', 'value_expression')
            self.assertTrue(result == 'Success')

    def test_execute_let_invalid_identifier(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        with self.assertRaises(OperationalError):
            cursor.execute_let('test&/_ rel', 'value_expression')

    def test_execute_let_error(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        with patch('secondodb.api.secondoapi.Cursor.execute',
                   side_effect=ProgrammingError('error')):
            with self.assertRaises(ProgrammingError):
                cursor.execute_let('test', 'value_expression')

    def test_execute_derive_no_cursor(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()
        cursor.close()

        with self.assertRaises(InternalError):
            cursor.execute_derive('identifier', 'value_expression')

    def test_execute_derive_successful(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        with patch('secondodb.api.secondoapi.Cursor.execute',
                   return_value=['Success']):
            result = cursor.execute_derive('identifier', 'value_expression')
            self.assertTrue(result == 'Success')

    def test_execute_derive_invalid_identifier(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        with self.assertRaises(OperationalError):
            cursor.execute_derive('test&/_ rel', 'value_expression')

    def test_execute_derive_error(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        with patch('secondodb.api.secondoapi.Cursor.execute',
                   side_effect=ProgrammingError('error')):
            with self.assertRaises(ProgrammingError):
                cursor.execute_derive('test', 'value_expression')

    def test_execute_update_no_cursor(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()
        cursor.close()

        with self.assertRaises(InternalError):
            cursor.execute_update('identifier', 'value_expression')

    def test_execute_update_successful(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        with patch('secondodb.api.secondoapi.Cursor.execute',
                   return_value=['Success']):
            result = cursor.execute_update('identifier', 'value_expression')
            self.assertTrue(result == 'Success')

    def test_execute_update_invalid_identifier(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        with self.assertRaises(OperationalError):
            cursor.execute_update('test&/_ rel', 'value_expression')

    def test_execute_update_error(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        with patch('secondodb.api.secondoapi.Cursor.execute',
                   side_effect=ProgrammingError('error')):
            with self.assertRaises(ProgrammingError):
                cursor.execute_update('test', 'value_expression')

    def test_execute_delete_no_cursor(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()
        cursor.close()

        with self.assertRaises(InternalError):
            cursor.execute_delete('identifier')

    def test_execute_delete_successful(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        with patch('secondodb.api.secondoapi.Cursor.execute',
                   return_value=['Success']):
            result = cursor.execute_delete('identifier')
            self.assertTrue(result == 'Success')

    def test_execute_delete_invalid_identifier(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        with self.assertRaises(OperationalError):
            cursor.execute_delete('test&/_ rel')

    def test_execute_delete_error(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        with patch('secondodb.api.secondoapi.Cursor.execute',
                   side_effect=ProgrammingError('error')):
            with self.assertRaises(ProgrammingError):
                cursor.execute_delete('test')

    def test_execute_create_type_no_cursor(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()
        cursor.close()

        with self.assertRaises(InternalError):
            cursor.execute_create_type('identifier', 'type_expression')

    def test_execute_create_type_successful(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        with patch('secondodb.api.secondoapi.Cursor.execute',
                   return_value=['Success']):
            result = cursor.execute_create_type('identifier', 'type_expression')
            self.assertTrue(result == 'Success')

    def test_execute_create_type_invalid_identifier(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        with self.assertRaises(OperationalError):
            cursor.execute_create_type('test&/_ rel', 'type_expression')

    def test_execute_create_type_error(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        with patch('secondodb.api.secondoapi.Cursor.execute',
                   side_effect=ProgrammingError('error')):
            with self.assertRaises(ProgrammingError):
                cursor.execute_create_type('test', 'type_expression')

    def test_execute_delete_type_no_cursor(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()
        cursor.close()

        with self.assertRaises(InternalError):
            cursor.execute_delete_type('identifier')

    def test_execute_delete_type_successful(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        with patch('secondodb.api.secondoapi.Cursor.execute',
                   return_value=['Success']):
            result = cursor.execute_delete_type('identifier')
            self.assertTrue(result == 'Success')

    def test_execute_delete_type_invalid_identifier(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        with self.assertRaises(OperationalError):
            cursor.execute_delete_type('test&/_ rel')

    def test_execute_delete_type_error(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        with patch('secondodb.api.secondoapi.Cursor.execute',
                   side_effect=ProgrammingError('error')):
            with self.assertRaises(ProgrammingError):
                cursor.execute_delete_type('test')

    def test_execute_create_no_cursor(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()
        cursor.close()

        with self.assertRaises(InternalError):
            cursor.execute_create('identifier', 'type_expression')

    def test_execute_create_successful(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        with patch('secondodb.api.secondoapi.Cursor.execute',
                   return_value=['Success']):
            result = cursor.execute_create('testrel', 'type_expression')
            self.assertTrue(result == 'Success')

    def test_execute_create_invalid_identifier(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        with self.assertRaises(OperationalError):
            cursor.execute_create('test&/_ rel', 'type_expression')

    def test_execute_create_error(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        with patch('secondodb.api.secondoapi.Cursor.execute',
                   side_effect=ProgrammingError('error')):
            with self.assertRaises(ProgrammingError):
                cursor.execute_create('testrel', 'type_expression')

    def test_execute_kill_no_cursor(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()
        cursor.close()

        with self.assertRaises(InternalError):
            cursor.execute_kill('identifier')

    def test_execute_kill_successful(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        with patch('secondodb.api.secondoapi.Cursor.execute',
                   return_value=['Success']):
            result = cursor.execute_kill('testrel')
            self.assertTrue(result == 'Success')

    def test_execute_kill_invalid_identifier(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        with self.assertRaises(OperationalError):
            cursor.execute_kill('test&/_ rel')

    def test_execute_kill_error(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        connection = Connection(HOST, PORT, user, pswd, db)
        cursor = connection.cursor()

        with patch('secondodb.api.secondoapi.Cursor.execute',
                   side_effect=ProgrammingError('error')):
            with self.assertRaises(ProgrammingError):
                cursor.execute_kill('testrel')



