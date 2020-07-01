from unittest import TestCase
from unittest.mock import Mock, patch

from secondodb.api.secondoapi import Connection, InternalError, OperationalError, Cursor
from secondodb.api.secondoapi import InterfaceError, connect
from socket import socket


class TestConnection(TestCase):

    def setUp(self):
        self.connection = None

    def tearDown(self):
        if self.connection is not None:
            if self.connection.initialized:
                self.connection.close()
            if self.connection.socket_object is not None:
                self.connection.socket_object.close()

    def test_init_server_only(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = ''

        self.connection = Connection(HOST, PORT, user, pswd, db)
        self.assertTrue(self.connection)
        self.assertTrue(self.connection.server_mode_only)
        self.assertTrue(self.connection.initialized)
        self.assertIsInstance(self.connection.socket_object, socket)

    def test_init_with_db(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = 'username'
        pswd = 'passwd'
        db = 'berlintest'

        self.connection = Connection(HOST, PORT, user, pswd, db)
        self.assertTrue(self.connection)
        self.assertFalse(self.connection.server_mode_only)
        self.assertTrue(self.connection.initialized)
        self.assertIsInstance(self.connection.socket_object, socket)

    def test_init_with_non_existant_db(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = 'username'
        pswd = 'passwd'
        db = 'nonexistantdb'

        with self.assertRaises(InterfaceError):
            self.connection = Connection(HOST, PORT, user, pswd, db)

    def test_init_with_non_reachable_host(self):

        HOST = '127.0.0.125'
        PORT = 1234
        user = ''
        pswd = ''
        db = ''

        with self.assertRaises(InterfaceError):
            self.connection = Connection(HOST, PORT, user, pswd, db)

    def test_close_server_only(self):

        self.test_init_server_only()
        result = self.connection.close()
        self.assertTrue(result)

    def test_close_not_init(self):

        self.test_init_server_only()
        self.test_close_server_only()
        with self.assertRaises(InterfaceError):
            self.connection.close()

    def test_close_with_open_db(self):

        self.test_init_with_db()
        result = self.connection.close()
        self.assertTrue(result)

    def test_close_with_init_transaction(self):

        self.test_start_transaction_with_db()
        result = self.connection.close()
        self.assertTrue(result)

    def test_close_with_init_transaction_operational_error(self):

        mock_rollback = Mock()
        mock_rollback.side_effect = OperationalError('error')

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = ''

        conn = Connection(HOST, PORT, user, pswd, db)
        conn.transaction_init = True
        conn.rollback = mock_rollback
        conn.socket_object.close()

        with self.assertRaises(OperationalError):
            conn.close()

    def test_close_with_init_transaction_interface_error(self):

        mock_rollback = Mock()
        mock_rollback.side_effect = InterfaceError('error')

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = ''

        conn = Connection(HOST, PORT, user, pswd, db)
        conn.transaction_init = True
        conn.rollback = mock_rollback
        conn.socket_object.close()

        with self.assertRaises(InterfaceError):
            conn.close()

    def test_close_with_db_error_close(self):

        mock_close_database = Mock()
        mock_close_database.side_effect = InterfaceError('error')

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = ''

        conn = Connection(HOST, PORT, user, pswd, db)
        conn.close_database = mock_close_database
        conn.server_mode_only = False
        conn.socket_object.close()

        with self.assertRaises(InterfaceError):
            conn.close()

    def test_open_database_without_connection(self):
        self.test_close_server_only()
        with self.assertRaises(InterfaceError):
            self.connection.open_database('berlintest')

    def test_close_database_without_connection(self):
        self.test_close_server_only()
        with self.assertRaises(InterfaceError):
            self.connection.close_database()

    def test_commit_only_server(self):
        self.test_init_server_only()
        with self.assertRaises(InterfaceError):
            self.connection.commit()

    def test_commit_with_db_without_init_transaction(self):
        self.test_init_with_db()
        with self.assertRaises(InternalError):
            self.connection.commit()

    def test_commit_without_connection(self):
        self.test_close_server_only()
        with self.assertRaises(InterfaceError):
            self.connection.commit()

    def test_commit_with_db_with_init_transaction(self):
        self.test_start_transaction_with_db()
        self.assertTrue(self.connection.commit())
        self.assertFalse(self.connection.transaction_init)

    def test_start_transaction_with_db(self):
        self.test_init_with_db()
        self.assertTrue(self.connection.start_transaction())
        self.assertTrue(self.connection.transaction_init)

    def test_start_transaction_only_server(self):
        self.test_init_server_only()
        with self.assertRaises(OperationalError):
            self.connection.start_transaction()
        self.assertFalse(self.connection.transaction_init)

    def test_start_transaction_without_connection(self):
        self.test_close_server_only()
        with self.assertRaises(InterfaceError):
            self.connection.start_transaction()

    def test_rollback_only_server(self):
        self.test_init_server_only()
        with self.assertRaises(InterfaceError):
            self.connection.rollback()

    def test_rollback_with_no_transaction(self):
        self.test_init_with_db()
        with self.assertRaises(OperationalError):
            self.connection.rollback()

    def test_rollback_with_init_transaction(self):
        self.test_start_transaction_with_db()
        self.assertTrue(self.connection.rollback())
        self.assertFalse(self.connection.transaction_init)

    def test_rollback_without_connection(self):
        self.test_close_server_only()
        with self.assertRaises(InterfaceError):
            self.connection.rollback()

    def test_cursor_with_db(self):
        self.test_init_with_db()
        result = self.connection.cursor()
        self.assertIsInstance(result, Cursor)

    def test_create_database_with_db_exists(self):
        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = ''

        conn = Connection(HOST, PORT, user, pswd, db)

        response_string = 'test'
        receive_list = []
        error_dict = {}
        error_dict.update({'code': 701})

        mock_socket = Mock()
        mock_socket.return_value.sendall = Mock()
        conn.socket_object = mock_socket

        with patch('secondodb.api.secondoapi.parser.receive_response',
                   return_value=(response_string, receive_list, error_dict)):
            with self.assertRaises(OperationalError):
                conn.create_database('testdb')

    def test_create_database_with_other_error(self):
        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = ''

        conn = Connection(HOST, PORT, user, pswd, db)

        response_string = 'test'
        receive_list = []
        error_dict = {}
        error_dict.update({'code': 999})
        error_dict.update({'message': 'error message'})

        mock_socket = Mock()
        mock_socket.return_value.sendall = Mock()
        conn.socket_object = mock_socket

        with patch('secondodb.api.secondoapi.parser.receive_response',
                   return_value=(response_string, receive_list, error_dict)):
            with self.assertRaises(OperationalError):
                conn.create_database('testdb')

    def test_create_database_invalid_chars(self):
        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = ''

        conn = Connection(HOST, PORT, user, pswd, db)

        error_dict = {}
        error_dict.update({'code': 0})
        error_dict.update({'message': ''})

        mock_socket = Mock()
        mock_socket.return_value.sendall = Mock()
        conn.socket_object = mock_socket

        with patch('secondodb.api.secondoapi.parser.check_identifier',
                   return_value=False):
            with self.assertRaises(OperationalError):
                conn.create_database('testdb')

    def test_create_database_successful(self):

        self.test_init_server_only()

        response_string = 'test'
        receive_list = []
        error_dict = {}
        error_dict.update({'code': 0})
        error_dict.update({'message': ''})

        mock_socket = Mock()
        mock_socket.return_value.sendall = Mock()
        self.connection.socket_object.close()
        self.connection.socket_object = mock_socket

        with patch('secondodb.api.secondoapi.parser.receive_response',
                   return_value=(response_string, receive_list, error_dict)):
            self.assertTrue(self.connection.create_database('testdb'))

    def test_create_database_without_connection(self):

        self.test_close_server_only()
        with self.assertRaises(InterfaceError):
            self.connection.create_database('testdb')

    def test_delete_database_with_db(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        conn = Connection(HOST, PORT, user, pswd, db)

        response_string = 'test'
        receive_list = []
        error_dict = {}
        error_dict.update({'code': 0})
        error_dict.update({'message': ''})

        mock_socket = Mock()
        mock_socket.return_value.sendall = Mock()
        conn.socket_object.close()
        conn.socket_object = mock_socket

        conn.close_database = Mock()

        with patch('secondodb.api.secondoapi.parser.receive_response',
                   return_value=(response_string, receive_list, error_dict)):
            self.assertTrue(conn.delete_database('testdb'))

    def test_delete_database_with_db_error_close(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = 'berlintest'

        conn = Connection(HOST, PORT, user, pswd, db)

        response_string = 'test'
        receive_list = []
        error_dict = {}
        error_dict.update({'code': 0})
        error_dict.update({'message': ''})

        mock_socket = Mock()
        mock_socket.return_value.sendall = Mock()
        conn.socket_object.close()
        conn.socket_object = mock_socket

        conn.close_database = Mock()
        conn.close_database.side_effect = InterfaceError('test')

        with patch('secondodb.api.secondoapi.parser.receive_response',
                   return_value=(response_string, receive_list, error_dict)):
            with self.assertRaises(InterfaceError):
                conn.delete_database('testdb')

    def test_delete_database_with_other_error(self):
        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = ''

        conn = Connection(HOST, PORT, user, pswd, db)

        response_string = 'test'
        receive_list = []
        error_dict = {}
        error_dict.update({'code': 999})
        error_dict.update({'message': 'error message'})

        mock_socket = Mock()
        mock_socket.return_value.sendall = Mock()
        conn.socket_object.close()
        conn.socket_object = mock_socket

        with patch('secondodb.api.secondoapi.parser.receive_response',
                   return_value=(response_string, receive_list, error_dict)):
            with self.assertRaises(InterfaceError):
                conn.delete_database('testdb')

    def test_delete_database_invalid_chars(self):
        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = ''

        conn = Connection(HOST, PORT, user, pswd, db)

        error_dict = {}
        error_dict.update({'code': 0})
        error_dict.update({'message': ''})

        mock_socket = Mock()
        mock_socket.return_value.sendall = Mock()
        conn.socket_object.close()
        conn.socket_object = mock_socket

        with patch('secondodb.api.secondoapi.parser.check_identifier',
                   return_value=False):
            with self.assertRaises(OperationalError):
                conn.delete_database('testdb')

    def test_delete_database_successful(self):

        self.test_init_server_only()

        response_string = 'test'
        receive_list = []
        error_dict = {}
        error_dict.update({'code': 0})
        error_dict.update({'message': ''})

        mock_socket = Mock()
        mock_socket.return_value.sendall = Mock()
        self.connection.socket_object.close()
        self.connection.socket_object = mock_socket

        with patch('secondodb.api.secondoapi.parser.receive_response',
                   return_value=(response_string, receive_list, error_dict)):
            self.assertTrue(self.connection.delete_database('testdb'))

    def test_delete_database_without_connection(self):

        self.test_close_server_only()
        with self.assertRaises(InterfaceError):
            self.connection.delete_database('testdb')

    def test_restore_database_successful(self):

        self.test_init_server_only()

        response_string = 'test'
        receive_list = []
        error_dict = {}
        error_dict.update({'code': 0})
        error_dict.update({'message': ''})

        mock_socket = Mock()
        mock_socket.return_value.sendall = Mock()
        self.connection.socket_object.close()
        self.connection.socket_object = mock_socket

        with patch('secondodb.api.secondoapi.parser.receive_response',
                   return_value=(response_string, receive_list, error_dict)):
            self.assertTrue(self.connection.restore_database('testdb', 'db_location'))

    def test_restore_database_successful_error_db_close(self):

        self.test_init_server_only()

        response_string = 'test'
        receive_list = []
        error_dict = {}
        error_dict.update({'code': 0})
        error_dict.update({'message': ''})

        mock_socket = Mock()
        mock_socket.return_value.sendall = Mock()
        self.connection.socket_object.close()
        self.connection.socket_object = mock_socket

        mock_close_db = Mock()
        mock_close_db.side_effect = InterfaceError('error')

        self.connection.close_database = mock_close_db

        with patch('secondodb.api.secondoapi.parser.receive_response',
                   return_value=(response_string, receive_list, error_dict)):
            with self.assertRaises(InterfaceError):
                self.connection.restore_database('testdb', 'db_location')

    def test_restore_database_error(self):

        self.test_init_server_only()

        response_string = 'test'
        receive_list = []
        error_dict = {}
        error_dict.update({'code': 999})
        error_dict.update({'message': ''})

        mock_socket = Mock()
        mock_socket.return_value.sendall = Mock()
        self.connection.socket_object.close()
        self.connection.socket_object = mock_socket

        with patch('secondodb.api.secondoapi.parser.receive_response',
                   return_value=(response_string, receive_list, error_dict)):
            with self.assertRaises(InterfaceError):
                self.connection.restore_database('testdb', 'db_location')

    def test_restore_database_invalid_chars(self):

        self.test_init_server_only()

        error_dict = {}
        error_dict.update({'code': 0})
        error_dict.update({'message': ''})

        mock_socket = Mock()
        mock_socket.return_value.sendall = Mock()
        self.connection.socket_object.close()
        self.connection.socket_object = mock_socket

        with patch('secondodb.api.secondoapi.parser.check_identifier',
                   return_value=False):
            with self.assertRaises(OperationalError):
                self.connection.restore_database('testdb', 'db_location')

    def test_restore_database_without_connection(self):

        self.test_close_server_only()
        with self.assertRaises(InterfaceError):
            self.connection.restore_database('testdb', 'db_location')

    @patch("secondodb.api.secondoapi.parser.receive_response")
    def test_init_with_no_response(self, mocked_parser):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = ''

        mocked_parser.return_value = ('test', 'test')
        with self.assertRaises(InterfaceError):
            Connection(HOST, PORT, user, pswd, db)

    @patch("secondodb.api.secondoapi.socket")
    @patch("secondodb.api.secondoapi.parser.receive_response")
    def test_init_with_bad_response_connect_start(self, mocked_parser, mocked_socket):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = ''

        mock_socket_connect = Mock()
        mock_socket_sendall = Mock()

        mocked_socket.return_value.connect = mock_socket_connect
        mocked_socket.return_value.sendall = mock_socket_sendall

        mocked_parser.return_value = ('<SecondoOk/>', 'test')

        with self.assertRaises(OperationalError):
            Connection(HOST, PORT, user, pswd, db)

    @patch("secondodb.api.secondoapi.socket")
    @patch("secondodb.api.secondoapi.parser.receive_response")
    def test_init_with_error_in_parser(self, mocked_parser, mocked_socket):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = ''

        mock_socket_connect = Mock()
        mocked_socket.return_value.connect = mock_socket_connect

        mocked_parser.side_effect = OSError('test', 'test')

        with self.assertRaises(InterfaceError):
            Connection(HOST, PORT, user, pswd, db)

    @patch("secondodb.api.secondoapi.socket")
    @patch("secondodb.api.secondoapi.parser.receive_response")
    def test_init_with_close_error(self, mocked_parser, mocked_socket):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = ''

        mock_socket_connect = Mock()
        mock_socket_close = Mock()
        mocked_socket.return_value.connect = mock_socket_connect
        mocked_socket.return_value.close = mock_socket_close

        mock_socket_close.side_effect = OSError('test', 'test')

        mocked_parser.return_value = ('not_secondo_ok', 'not_secondo_ok')

        with self.assertRaises(InterfaceError):
            Connection(HOST, PORT, user, pswd, db)

    @patch("secondodb.api.secondoapi.socket")
    @patch("secondodb.api.secondoapi.parser.receive_response")
    def test_init_with_os_error(self, mocked_parser, mocked_socket):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = ''

        mock_socket_connect = Mock()
        mock_socket_close = Mock()
        mocked_socket.return_value.connect = mock_socket_connect
        mocked_socket.return_value.close = mock_socket_close

        mock_socket_connect.side_effect = OSError('test', 'test')

        mocked_parser.return_value = ('not_secondo_ok', 'not_secondo_ok')

        with self.assertRaises(InterfaceError):
            Connection(HOST, PORT, user, pswd, db)

    @patch("secondodb.api.secondoapi.socket")
    @patch("secondodb.api.secondoapi.parser.receive_response")
    def test_init_with_parse_operational_error(self, mocked_parser, mocked_socket):

        HOST = '127.0.0.1'
        PORT = 1234
        user = ''
        pswd = ''
        db = ''

        mock_socket_connect = Mock()
        mock_socket_close = Mock()
        mocked_socket.return_value.connect = mock_socket_connect
        mocked_socket.return_value.close = mock_socket_close

        mocked_parser.side_effect = OperationalError('test')

        with self.assertRaises(OperationalError):
            Connection(HOST, PORT, user, pswd, db)

    def test_get_list_databases_with_db(self):
        self.test_init_server_only()
        result = self.connection.get_list_databases()
        self.assertIsInstance(result, list)

    def test_get_list_databases_without_connection(self):
        self.test_close_server_only()
        with self.assertRaises(InterfaceError):
            self.connection.get_list_databases()

    def test_get_list_objects_only_server(self):
        self.test_init_server_only()
        with self.assertRaises(InterfaceError):
            self.connection.get_list_objects()

    def test_get_list_objects_with_db(self):
        self.test_init_with_db()
        result = self.connection.get_list_objects()
        self.assertIsInstance(result, list)

    def test_get_list_objects_without_connection(self):
        self.test_close_server_only()
        with self.assertRaises(InterfaceError):
            self.connection.get_list_objects()

    def test_get_list_types_without_connection(self):
        self.test_close_server_only()
        with self.assertRaises(InterfaceError):
            self.connection.get_list_types()

    def test_get_list_types_only_server(self):
        self.test_init_server_only()
        with self.assertRaises(InterfaceError):
            self.connection.get_list_types()

    def test_get_list_types_with_db(self):
        self.test_init_with_db()
        result = self.connection.get_list_types()
        self.assertIsInstance(result, list)

    def test_get_list_type_constructors_without_connection(self):
        self.test_close_server_only()
        with self.assertRaises(InterfaceError):
            self.connection.get_list_type_constructors()

    def test_get_list_type_constructors_only_server(self):
        self.test_init_server_only()
        result = self.connection.get_list_type_constructors()
        self.assertIsInstance(result, list)

    def test_get_list_algebras_without_connection(self):
        self.test_close_server_only()
        with self.assertRaises(InterfaceError):
            self.connection.get_list_algebras()

    def test_get_list_algebras_only_server(self):
        self.test_init_server_only()
        result = self.connection.get_list_algebras()
        self.assertIsInstance(result, list)

    def test_get_algebra_without_connection(self):
        self.test_close_server_only()
        with self.assertRaises(InterfaceError):
            self.connection.get_algebra('test')

    def test_get_algebra_only_server(self):
        self.test_init_server_only()
        result = self.connection.get_algebra('StandardAlgebra')
        self.assertIsInstance(result, object)
        self.assertIsInstance(result.algebra_name, str)
        self.assertIsInstance(result.type_list, list)
        self.assertIsInstance(result.operator_list, list)

    def test_connect_successful(self):

        HOST = '127.0.0.1'
        PORT = '1234'

        result = connect(HOST, PORT)
        self.assertIsInstance(result, Connection)

    @patch('secondodb.api.secondoapi.Connection')
    def test_connect_refused(self, mock_connection):

        HOST = '127.0.0.1'
        PORT = '1234'

        mock_connection.return_value.initialized = False

        with self.assertRaises(InterfaceError):
            connect(HOST, PORT)

    def test_connect_error_no_host(self):

        HOST = ''
        PORT = '1234'

        with self.assertRaises(InterfaceError):
            connect(HOST, PORT)

    def test_connect_error_no_port(self):

        HOST = '127.0.0.1'
        PORT = ''

        with self.assertRaises(InterfaceError):
            connect(HOST, PORT)

    def test_connect_error_port_invalid(self):

        HOST = '127.0.0.1'
        PORT = 'ABCD'

        with self.assertRaises(InterfaceError):
            connect(HOST, PORT)

    def test_connect_error_username_invalid(self):

        HOST = '127.0.0.1'
        PORT = '1234'
        USERNAME = 'sj/ 0212'

        with self.assertRaises(InterfaceError):
            connect(HOST, PORT, username=USERNAME)

    def test_connect_error_passwd_invalid(self):

        HOST = '127.0.0.1'
        PORT = '1234'
        USERNAME = 'username'
        PASSWD = 'sj/ 0212'

        with self.assertRaises(InterfaceError):
            connect(HOST, PORT, username=USERNAME, passwd=PASSWD)

    def test_connect_error_dbname_invalid(self):

        HOST = '127.0.0.1'
        PORT = '1234'
        USERNAME = 'username'
        PASSWD = 'passwd'
        DBNAME = 'sj/ 0212'

        with self.assertRaises(InterfaceError):
            connect(HOST, PORT, username=USERNAME, passwd=PASSWD, database=DBNAME)






