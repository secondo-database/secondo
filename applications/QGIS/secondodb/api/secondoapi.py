# ----------------------------------------------------------------------------------------------------------------------
# The Secondo Python API (pySecondo)
# Victor Silva (victor.silva@posteo.de)
# October 2019
# ----------------------------------------------------------------------------------------------------------------------
# Module
# ----------------------------------------------------------------------------------------------------------------------
# Secondo API
# secondoapi.py
# ----------------------------------------------------------------------------------------------------------------------
"""
The module Secondo API implements the Application Programming Interface for Python for the interaction with a |sec|
server. The API implements the Python Database API 2.0 Specification (PEP 249).
"""

from socket import socket, AF_INET, SOCK_STREAM, TCP_NODELAY, IPPROTO_TCP
# from secondodb.api.support import secondoparser as parser
import secondodb.api.support.secondoparser as parser
import secondodb.api.support.secondomessages as messages
import secondodb.api.support.secondocommands as com

apilevel = '2.0'
threadsafety = 1
paramstyle = 'pyformat'
encoding = 'latin-1'
error_encoding = 'ignore'


def connect(host, port, username='username', passwd='passwd', database=''):
    """
    Constructor for creating a connection to the |sec| server. If further data is provided (database and login data),
    a connection to an existing database (if available on the server) will be established.

    Returns a Connection Object.

    :param host: The host of the |sec| server as IP-address or as qualified name (localhost).
    :param port: The port of the |sec| server.
    :param username: The username for the connection (optional).
    :param passwd: The password for the connection (optional).
    :param database: The name of the database (optional).
    :return: A Connection Object.
    """

    # Check if minimum parameters for the connection are available and valid

    if host == '':
        raise InterfaceError('The host is not valid.')

    if port == '':
        raise InterfaceError('The port is not valid.')

    if not parser.check_port(port):
        raise InterfaceError('The port has invalid characters.')

    if not parser.check_identifier(username):
        raise InterfaceError('The username is not valid.')

    if not parser.check_identifier(passwd):
        raise InterfaceError('The password is not valid.')

    if database != '' and not parser.check_identifier(database):
        raise InterfaceError('The database name is not valid.')

    port = int(port)

    # Create Connection object

    connection = Connection(host, port, username, passwd, database)

    if connection.initialized:
        return connection
    else:
        raise InterfaceError('Connection refused.')


# --- Exceptions ---

class Warning(Exception):
    """
    Exception raised for important warnings like data truncations while inserting, etc.
    """
    def __init__(self, message, *args):
        self.message = message
        super(Warning, self).__init__(message, *args)


class Error(Exception):
    """
    Exception that is the base class of all other error exceptions.
    You can use this to catch all errors with one single except statement.
    Warnings are not considered errors and thus should not use this class as base.
    """
    def __init__(self, message, *args):
        self.message = message
        super(Error, self).__init__(message, *args)


class InterfaceError(Error):
    """
    Exception raised for errors that are related to the database interface rather than the database itself.
    """
    def __init__(self, message, *args):
        self.message = message
        super(InterfaceError, self).__init__(message, *args)


class DatabaseError(Error):
    """
    Exception raised for errors that are related to the database.
    """
    def __init__(self, message, *args):
        self.message = message
        super(DatabaseError, self).__init__(message, *args)


class DataError(DatabaseError):
    """
    Exception raised for errors that are due to problems with the processed data like division by zero,
    numeric value out of range, etc.
    """
    def __init__(self, message, *args):
        self.message = message
        super(DataError, self).__init__(message, *args)


class OperationalError(DatabaseError):
    """
    Exception raised for errors that are related to the database's operation and not necessarily under
    the control of the programmer, e.g. an unexpected disconnect occurs, the data source name is not found,
    a transaction could not be processed, a memory allocation error occurred during processing, etc.
    """
    def __init__(self, message, *args):
        self.message = message
        super(OperationalError, self).__init__(message, *args)


class IntegrityError(DatabaseError):
    """
    Exception raised when the relational integrity of the database is affected, e.g. a foreign key check fails.
    """
    def __init__(self, message, *args):
        self.message = message
        super(IntegrityError, self).__init__(message, *args)


class InternalError(DatabaseError):
    """
    Exception raised when the database encounters an internal error, e.g. the cursor is not valid anymore,
    the transaction is out of sync, etc.
    """
    def __init__(self, message, *args):
        self.message = message
        super(InternalError, self).__init__(message, *args)


class ProgrammingError(DatabaseError):
    """
    Exception raised for programming errors, e.g. table not found or already exists, syntax error in the SQL statement,
    wrong number of parameters specified, etc.
    """
    def __init__(self, message, *args):
        self.message = message
        super(ProgrammingError, self).__init__(message, *args)


class NotSupportedError(DatabaseError):
    """
    Exception raised in case a method or database API was used which is not supported by the database,
    e.g. requesting a .rollback() on a connection that does not support transaction or has transactions turned off.
    """
    def __init__(self, message, *args):
        self.message = message
        super(NotSupportedError, self).__init__(message, *args)


# --- Connection Objects ---

class Connection:
    """
    This class implements the connection object of the |sec| API. The connection object manages all operations at server
    level and provides access to the current connection instance with the server. The connection object provides the
    cursor() method to create a cursor for the execution of operations at the database level.
    """

    def __init__(self, host, port, username, passwd, database):
        """
        Constructor of the Connection object.

        :param host: The host of the |sec| Server as IP-address or as qualified name (localhost).
        :param port: The port of the |sec| Server.
        :param username: The username for the connection.
        :param passwd: The password for the connection.
        :param database: The name of the database.
        """
        self.host = host
        self.port = port
        self.username = username
        self.passwd = passwd
        self.database = database
        self.socket_object = socket(AF_INET, SOCK_STREAM)
        self.socket_object.setsockopt(IPPROTO_TCP, TCP_NODELAY, 1)
        self.initialized = False
        self.server_mode_only = False
        self.transaction_init = False

        self.__initialize()

    def __initialize(self):
        """
        Initializes the connection to the |sec| server.

        :return: None
        """

        # Connect to socket using the host and the port

        try:
            self.socket_object.connect((self.host, self.port))
        except ConnectionRefusedError as e:
            self.socket_object.close()
            raise InterfaceError(e.args[1])
        except OSError as e:
            self.socket_object.close()
            raise InterfaceError(e.args[1] + ' - Check connection parameters.')

        # Receive initial data from socket

        intro_string = ''

        try:
            intro_string, ok_message = parser.receive_response(self.socket_object)
        except OSError as e:
            raise InterfaceError(e.args[1])
        except OperationalError:
            raise

        if intro_string == messages.SECONDO_OK:

            # If Secondo accepted the token, proceed with the connection to the server

            conn_string = messages.SECONDO_CONNECT_START + self.username + '\n' \
                                                         + self.passwd + '\n' \
                                                         + messages.SECONDO_CONNECT_END
            self.socket_object.sendall(conn_string.encode())

            # Get response list from socket

            try:
                intro_string, success_message = parser.receive_response(self.socket_object)
            except OperationalError:
                raise

            # Check intro string, print success message from the server

            if intro_string == messages.SECONDO_INTRO_START:
                self.initialized = True
                print(success_message)

                # If a database name was provided, connect to database

                if self.database != '':
                    try:
                        self.open_database(self.database)
                    except OperationalError:
                        self.server_mode_only = True
                        self.database = ''
                        self.username = ''
                        self.passwd = ''
                        self.socket_object.close()
                        raise
                    except InterfaceError:
                        raise
                    else:
                        self.server_mode_only = False
                        return True

                else:
                    self.server_mode_only = True
                    return True
            else:
                raise OperationalError('Error: Bad server response.')

        else:
            try:
                self.socket_object.close()
            except OSError as e:
                raise InterfaceError(e.args[1])
            else:
                self.socket_object = None
                self.initialized = False
                raise InterfaceError('Error: No response from Secondo server')

    def close(self):
        """
        Close the connection now.

        The connection will be unusable from this point forward; an InternalError exception will be raised if any
        operation is attempted with the connection. The same applies to all cursor objects trying to use the connection.
        Note that closing a connection without committing the changes first will cause an implicit rollback to be
        performed.

        :return: True, if the connection was closed successfully.
        """

        if self.initialized:

            # Any pending transaction will be rolled back

            if self.transaction_init:
                try:
                    self.rollback()
                except OperationalError:
                    raise
                except InterfaceError:
                    raise

            close_string = messages.SECONDO_DISCONNECT_END

            # Close open databases

            if not self.server_mode_only:
                try:
                    self.close_database()
                except InterfaceError:
                    raise
            try:
                self.socket_object.sendall(close_string.encode())
            except OSError as e:
                raise OperationalError(e.args[1])
            else:
                self.socket_object.close()
                self.socket_object = None
                self.initialized = False
                return True

        else:
            raise InterfaceError('Error: The connection object is not initialized.')

    def start_transaction(self):
        """
        Starts a transaction in the database.

        :return: True, if the transaction was started successfully.
        """
        if self.initialized:

            if self.server_mode_only:
                raise OperationalError('No connection to a database available.')

            command_level = 1
            operation = com.SECONDO_COM_BEGIN_TR
            command_string = messages.SECONDO_COMMAND_START + str(command_level) + '\n' \
                                                                + operation + '\n' \
                                                                + messages.SECONDO_COMMAND_END

            self.socket_object.sendall(command_string.encode())

            try:
                response_string, receive_list, error_dict = parser.receive_response(self.socket_object)
            except OperationalError:
                raise

            if error_dict['code'] != 0:
                if error_dict['code'] == 100:
                    raise InterfaceError('Secondo: Unexpected fatal system error.')
                else:
                    raise InterfaceError(error_dict['message'])
            else:
                self.transaction_init = True
                return True

        else:
            raise InterfaceError('Error: The connection object is not initialized.')

    def commit(self):
        """
        Commit any pending transaction to the database.

        :return: True, if the current transaction was commit successfully.
        """
        if self.initialized:

            if self.server_mode_only:
                raise InterfaceError('No connection to a database available.')

            if self.transaction_init:

                command_level = 1
                operation = com.SECONDO_COM_COMMIT_TR
                command_string = messages.SECONDO_COMMAND_START + str(command_level) + '\n' \
                                                                + operation + '\n' \
                                                                + messages.SECONDO_COMMAND_END

                self.socket_object.sendall(command_string.encode())

                try:
                    response_string, receive_list, error_dict = parser.receive_response(self.socket_object)
                except OperationalError:
                    raise

                if error_dict['code'] != 0:
                    if error_dict['code'] == 100:
                        raise InterfaceError('Secondo: Unexpected fatal system error.')
                    else:
                        raise InterfaceError(error_dict['message'])
                else:
                    self.transaction_init = False
                    return True

            else:
                raise InternalError('No transactions were initiated.')

        else:
            raise InterfaceError('Error: The connection object is not initialized.')

    def rollback(self):
        """
        In case a database does provide transactions this method causes the database to roll back to the start of any
        pending transaction. Closing a connection without committing the changes first will cause an implicit rollback
        to be performed.

        :return: True, if the current transaction was rolled back successfully.
        """
        if self.initialized:

            if self.server_mode_only:
                raise InterfaceError('No connection to a database available.')

            if self.transaction_init:

                command_level = 1
                operation = com.SECONDO_COM_ABORT_TR
                command_string = messages.SECONDO_COMMAND_START + str(command_level) + '\n' \
                                                                + operation + '\n' \
                                                                + messages.SECONDO_COMMAND_END

                self.socket_object.sendall(command_string.encode())

                try:
                    response_string, receive_list, error_dict = parser.receive_response(self.socket_object)
                except OperationalError:
                    raise

                if error_dict['code'] != 0:
                    if error_dict['code'] == 100:
                        raise InterfaceError('Secondo: Unexpected fatal system error.')
                    else:
                        raise InterfaceError(error_dict['message'])
                else:
                    self.transaction_init = False
                    return True

            else:
                raise OperationalError('No transactions were initiated.')

        else:
            raise InterfaceError('Error: The connection object is not initialized.')

    def cursor(self):
        """
        Return a new Cursor Object using the connection.

        If the database does not provide a direct cursor concept, the module will have to emulate cursors using other
        means to the extent needed by this specification.

        :return: A Cursor object.
        """
        cursor = Cursor(self)
        return cursor

    def create_database(self, database_name):
        """
        Creates a database on the |sec| server. The database remains open after creation.

        :param database_name: The name of the new database.
        :return: True, if the database was created successfully.
        """

        if self.initialized:

            command_level = 1
            database_name.lower()

            if parser.check_identifier(database_name):

                operation = com.SECONDO_COM_CREATE_DB.format(database_name)
                command_string = messages.SECONDO_COMMAND_START + str(command_level) + '\n' \
                                                                + operation + '\n' \
                                                                + messages.SECONDO_COMMAND_END

                self.socket_object.sendall(command_string.encode())

                try:
                    response_string, receive_list, error_dict = parser.receive_response(self.socket_object)
                except OperationalError:
                    raise

                if error_dict['code'] != 0:
                    if error_dict['code'] == 701:
                        raise OperationalError('The database ' + database_name.upper() + ' already exists.')
                    if error_dict['code'] == 100:
                        raise InterfaceError('Secondo: Unexpected fatal system error.')
                    else:
                        raise OperationalError(error_dict['message'])
                else:
                    return True

            else:
                raise OperationalError('The database name has invalid characters.')

        else:
            raise InterfaceError('The connection has not been initialized.')

    def restore_database(self, database_name, database_location):
        """
        Restores a database from a specific location into the |sec| server. After restoring the database will be
        available for establish a connection.

        :param database_name: The name of the database.
        :param database_location: The location of the database.
        :return: True, if the database was successfully restored.
        """

        if self.initialized:

            command_level = 1

            if parser.check_identifier(database_name):
                operation = com.SECONDO_COM_RESTORE_DB_FROM.format(database_name, database_location)
                command_string = messages.SECONDO_COMMAND_START + str(command_level) + '\n' \
                                                                + operation + '\n' \
                                                                + messages.SECONDO_COMMAND_END

                self.socket_object.sendall(command_string.encode())

                try:
                    response_string, receive_list, error_dict = parser.receive_response(self.socket_object)
                except OperationalError:
                    raise

                if error_dict['code'] != 0:
                    if error_dict['code'] == 100:
                        raise InterfaceError('Secondo: Unexpected fatal system error.')
                    else:
                        raise InterfaceError(error_dict['message'])
                else:
                    try:
                        self.close_database()
                    except InterfaceError:
                        raise
                    else:
                        return True

            else:
                raise OperationalError('The database name has invalid characters.')

        else:
            raise InterfaceError('The connection has not been initialized.')

    def delete_database(self, database_name):
        """
        Deletes a database on the |sec| server. Before deletion, all opened databases on the Secondo server will be
        closed. The connection object will be set to server-only mode.

        :param database_name: The name of the database.
        :return: True, if the database was deleted successfully.
        """

        if self.initialized:

            # Close databases, if any of them are open

            if not self.server_mode_only:
                try:
                    self.close_database()
                except InterfaceError:
                    raise

            if parser.check_identifier(database_name):

                command_level = 1

                database_name.lower()
                operation = com.SECONDO_COM_DELETE_DB.format(database_name)
                command_string = messages.SECONDO_COMMAND_START + str(command_level) + '\n' \
                                                                + operation + '\n' \
                                                                + messages.SECONDO_COMMAND_END

                self.socket_object.sendall(command_string.encode())

                try:
                    response_string, receive_list, error_dict = parser.receive_response(self.socket_object)
                except OperationalError:
                    raise

                if error_dict['code'] != 0:
                    if error_dict['code'] == 100:
                        raise InterfaceError('Secondo: Unexpected fatal system error.')
                    else:
                        raise InterfaceError(error_dict['message'])
                else:
                    return True

            else:
                raise OperationalError('The database name has invalid characters.')

        else:
            raise InterfaceError('The connection has not been initialized.')

    def get_list_databases(self):
        """
        Returns a list of the databases in the |sec| server.

        :return: A Python list with the names of the available databases on the |sec| server.
        """

        if self.initialized:

            command_level = 1
            operation = com.SECONDO_COM_LIST_DB
            command_string = messages.SECONDO_COMMAND_START + str(command_level) + '\n' \
                                                            + operation + '\n' \
                                                            + messages.SECONDO_COMMAND_END

            self.socket_object.sendall(command_string.encode())

            try:
                response_string, receive_list, error_dict = parser.receive_response(self.socket_object)
            except OperationalError:
                raise

            if error_dict['code'] != 0:
                if error_dict['code'] == 100:
                    raise InterfaceError('Secondo: Unexpected fatal system error.')
                else:
                    raise InterfaceError(error_dict['message'])
            else:
                databases = parser.parse_inquiry_databases(receive_list)
                return databases
        else:
            raise InterfaceError('The connection has not been initialized.')

    def get_list_objects(self):
        """
        Returns a list of the objects in the |sec| server.

        :return: A Python list with the names available objects of the currently opened database.
        """

        if self.initialized:
            command_level = 1
            operation = com.SECONDO_COM_LIST_OBJECTS
            command_string = messages.SECONDO_COMMAND_START + str(command_level) + '\n' \
                                                            + operation + '\n' \
                                                            + messages.SECONDO_COMMAND_END

            self.socket_object.sendall(command_string.encode())

            try:
                response_string, receive_list, error_dict = parser.receive_response(self.socket_object)
            except OperationalError:
                raise

            if error_dict['code'] != 0:
                if error_dict['code'] == 100:
                    raise InterfaceError('Secondo: Unexpected fatal system error.')
                else:
                    raise InterfaceError(error_dict['message'])
            else:
                objects = parser.parse_inquiry_objects(receive_list)
                return objects
        else:
            raise InterfaceError('The connection has not been initialized.')

    def get_list_types(self):
        """
        Returns a list of the types available in the current open database.

        :return: A list expression object with the available types of the currently opened database.
        """

        if self.initialized:
            command_level = 1
            operation = com.SECONDO_COM_LIST_TYPES
            command_string = messages.SECONDO_COMMAND_START + str(command_level) + '\n' \
                                                            + operation + '\n' \
                                                            + messages.SECONDO_COMMAND_END

            self.socket_object.sendall(command_string.encode())

            try:
                response_string, receive_list, error_dict = parser.receive_response(self.socket_object)
            except OperationalError:
                raise

            if error_dict['code'] != 0:
                if error_dict['code'] == 100:
                    raise InterfaceError('Secondo: Unexpected fatal system error.')
                else:
                    raise InterfaceError(error_dict['message'])
            else:
                types = parser.parse_inquiry_types(receive_list)
                return types
        else:
            raise InterfaceError('The connection has not been initialized.')

    def get_list_type_constructors(self):
        """
        Returns a list of the type constructors available in the |sec| server.

        :return: A Python list with the constructors of the available types of the currently opened database.
        """

        if self.initialized:
            command_level = 1
            operation = com.SECONDO_COM_LIST_TYPE_CONS
            command_string = messages.SECONDO_COMMAND_START + str(command_level) + '\n' \
                                                            + operation + '\n' \
                                                            + messages.SECONDO_COMMAND_END

            self.socket_object.sendall(command_string.encode())

            try:
                response_string, receive_list, error_dict = parser.receive_response(self.socket_object)
            except OperationalError:
                raise

            if error_dict['code'] != 0:
                if error_dict['code'] == 100:
                    raise InterfaceError('Secondo: Unexpected fatal system error.')
                else:
                    raise InterfaceError(error_dict['message'])
            else:
                type_constructors = parser.parse_inquiry_type_constructors(receive_list)
                return type_constructors
        else:
            raise InterfaceError('The connection has not been initialized.')

    def get_list_algebras(self):
        """
        Returns a list of the available algebras in the |sec| server.

        :return: A Python list with the available algebras_1 on the |sec| server.
        """

        if self.initialized:
            command_level = 1
            operation = com.SECONDO_COM_LIST_ALGEBRAS
            command_string = messages.SECONDO_COMMAND_START + str(command_level) + '\n' \
                                                            + operation + '\n' \
                                                            + messages.SECONDO_COMMAND_END

            self.socket_object.sendall(command_string.encode())

            try:
                response_string, receive_list, error_dict = parser.receive_response(self.socket_object)
            except OperationalError:
                raise

            if error_dict['code'] != 0:
                if error_dict['code'] == 100:
                    raise InterfaceError('Secondo: Unexpected fatal system error.')
                else:
                    raise InterfaceError(error_dict['message'])
            else:
                algebras = parser.parse_inquiry_algebras(receive_list)
                return algebras
        else:
            raise InterfaceError('The connection has not been initialized.')

    def get_algebra(self, algebra_name):
        """
        Returns the details for an algebra from the |sec| server.

        :return: A Python list with the available algebras on the |sec| server.
        """

        if self.initialized:
            command_level = 1
            operation = com.SECONDO_COM_LIST_ALGEBRA.format(algebra_name)
            command_string = messages.SECONDO_COMMAND_START + str(command_level) + '\n' \
                                                            + operation + '\n' \
                                                            + messages.SECONDO_COMMAND_END

            self.socket_object.sendall(command_string.encode())

            try:
                response_string, receive_list, error_dict = parser.receive_response(self.socket_object)
            except OperationalError:
                raise

            if error_dict['code'] != 0:
                if error_dict['code'] == 100:
                    raise InterfaceError('Secondo: Unexpected fatal system error.')
                else:
                    raise InterfaceError(error_dict['message'])
            else:
                algebra = parser.parse_inquiry_algebra(receive_list)
                return algebra
        else:
            raise InterfaceError('The connection has not been initialized.')

    def open_database(self, database_name):
        """
        Opens a database on the |sec| server. The server-only mode will be set to False.

        :param database_name: The name of the database.
        :return: True, if the database was opened successfully.
        """

        if self.initialized:

            command_level = 1

            operation = com.SECONDO_COM_OPEN_DB.format(database_name)
            command_string = messages.SECONDO_COMMAND_START + str(command_level) + '\n' \
                                                            + operation + '\n' \
                                                            + messages.SECONDO_COMMAND_END

            self.socket_object.sendall(command_string.encode())

            try:
                response_string, receive_list, error_dict = parser.receive_response(self.socket_object)
            except OperationalError:
                raise

            if error_dict['code'] != 0:
                if error_dict['code'] == 100:
                    raise InterfaceError('Secondo: Unexpected fatal system error.')
                else:
                    raise InterfaceError(error_dict['message'])
            else:
                self.server_mode_only = False
                self.database = database_name
                return True
        else:
            raise InterfaceError('The connection has not been initialized.')

    def close_database(self):
        """
        Closes the currently opened database on the |sec| server. The connection object returns to the server-only
        mode.

        :return: True, if the database was closed successfully.
        """

        if self.initialized:
            command_level = 1
            operation = com.SECONDO_COM_CLOSE_DB
            command_string = messages.SECONDO_COMMAND_START + str(command_level) + '\n' \
                                                                                 + operation + '\n' \
                                                                                 + messages.SECONDO_COMMAND_END

            self.socket_object.sendall(command_string.encode())

            try:
                response_string, receive_list, error_dict = parser.receive_response(self.socket_object)
            except OperationalError:
                raise

            if error_dict['code'] != 0:
                if error_dict['code'] == 100:
                    self.server_mode_only = True
                    raise InterfaceError('Secondo: Unexpected fatal system error.')
                else:
                    raise InterfaceError(error_dict['message'])
            else:
                self.server_mode_only = True
                self.database = ''
                return True
        else:
            raise InterfaceError('The connection has not been initialized.')


class Cursor:
    """
    This class implements the cursor object of the |sec| API. A cursor implements functions to operate within a specific
    database connection. The main method of the class is the execute method, which implements the programm logic towards
    the execution of CRUD operations at database level.
    """

    description = None
    rowcount = 0

    def __init__(self, connection):
        """
        Constructor of the Cursor object. The database can be initialized during the initialization of the Connection
        object or alternatively in the Cursor object through this constructor. It is important to set previously the
        variable database in the Connection object, otherwise the database won´t be initialized. In that case, an
        InterfaceError will be raised.

        :param connection: A Connection object.
        """
        self.connection = connection
        self.initialized = False
        self.result = []

        # If the connection to the database is not existent, connect to provided database

        if self.connection.server_mode_only:
            if self.connection.database != '':
                try:
                    self.connection.open_database(self.connection.database)
                except OperationalError:
                    raise
                else:
                    self.initialized = True
            else:
                raise OperationalError("No database provided or initialized. The cursor couldn´t be created.")
        else:
            self.initialized = True

    def close(self):
        """
        Close the cursor now (rather than whenever __del__ is called).

        The cursor will be unusable from this point forward; an InternalError exception will be raised if any
        operation is attempted with the cursor.

        :return: True, if the cursor was closed successfully.
        """
        if self.initialized:
            try:
                self.connection.close_database()
            except InterfaceError:
                raise
            else:
                self.connection.database = ''
                self.initialized = False
                return True
        else:
            raise OperationalError("The cursor has not been initialized")

    def execute(self, operation, parameters=None):
        """
        Prepare and execute a database operation (query or command).

        Parameters may be provided as sequence or mapping and will be bound to variables in the operation. The
        placeholders in the operation must use the following format: {i}, where i is an integer, which
        specifies the position of the parameter in the list of parameters.

        Example 1: Simple Query

        Operation = query {0}
        Parameter list = ['mehringdamm']
        Formatted operation = query mehringdamm

        Example 2: Inserting Single Tuples

        Operation = query {0} inserttuple[{1}, {2}] count;
        Parameter list = ['myfirstrel','"Anna"', '27']
        Formatted operation = query myfirstrel inserttuple["Anna", 27] count;

        :param operation: A string with a |sec| command.
        :param parameters: Further parameters for the execution of the command (currently not in use).
        :return: A |sec| object, a list with |sec| objects or a |sec| response.
        """

        self.description = ''
        self.rowcount = 0

        # Apply parameters, if some are supplied

        if parameters is not None:
            operation = com.apply_parameters_to_operation(operation, parameters)

        # Process operation

        if self.initialized:

            command_level = 1
            command_string = messages.SECONDO_COMMAND_START + str(command_level) + '\n' \
                                                            + operation + '\n' \
                                                            + messages.SECONDO_COMMAND_END

            try:
                self.connection.socket_object.sendall(command_string.encode(encoding, error_encoding))
            except UnicodeEncodeError as e:
                raise ProgrammingError(e.args[0])
            else:
                try:
                    response_string, receive_list, error_dict = parser.receive_response(self.connection.socket_object)
                except OperationalError:
                    raise

            if error_dict['code'] != 0:
                if error_dict['code'] == 6:  # No database open
                    raise InterfaceError(error_dict['message'])
                else:
                    raise ProgrammingError(error_dict['message'])
            else:
                if receive_list.get_list_length() > 1:
                    try:
                        query_response, object_type = parser.parse_query(receive_list)
                    except InterfaceError as e:
                        raise InterfaceError(e.args[0])
                    else:
                        self.rowcount = 1
                        self.description = object_type

                        string_output = str(receive_list)

                        self.result.append([query_response, object_type, string_output])
                        return query_response, object_type, string_output

                else:
                    return 'Success', '', 'Success'

        else:
            raise InternalError('The cursor has not been initialized or is already closed.')

    def executemany(self, operation, seq_of_parameters):
        """
        Prepare a database operation (query or command) and then execute it against all parameter sequences or mappings
        found in the sequence seq_of_parameters. Every entry of the list seq_of_parameters must be declared as a list,
        even when just one parameter is being used.

        The placeholders in the operation must use the following format: {i}, where i is an integer number, which
        specifies the position of the parameter in the list of parameters.

        Example 1: Simple Query

        Operation = query {0}
        Parameter list = ['mehringdamm']
        Formatted operation = query mehringdamm

        Example 2: Inserting Single Tuples

        Operation = query {0} inserttuple[{1}, {2}] count;
        Parameter list = ['myfirstrel','"Anna"', '27']
        Formatted operation = query myfirstrel inserttuple["Anna", 27] count;

        :param operation: A |sec| command with placeholders.
        :param seq_of_parameters: A list with further parameters for the execution of the commands of the list.
        :return: A list with entries. Each entry is a list with an |sec| object, the type of the response and the list
                 expression as string.
        """

        response_list = []
        self.description = ''
        self.rowcount = 0

        # Apply parameters to operation

        for parameter_list in seq_of_parameters:

            response = self.execute(operation=operation, parameters=parameter_list)
            response_list.append(response)
            self.rowcount += 1

        self.description = 'response list'

        return response_list

    def fetchone(self):
        """
        Fetch the next row of a query result set, returning a single sequence, or None when no more data is available.

        An InterfaceError exception is raised if the previous call to .execute*() did not produce any result set or
        no call was issued yet.

        :return: A single row of a query result set.
        """
        if self.initialized:
            if self.rowcount >= 1:
                try:
                    one_row = self.result.pop(0)
                    return one_row
                except IndexError:
                    return None
            else:
                raise InterfaceError('Error: No rows in cursor available.')
        else:
            raise InternalError('The cursor has not been initialized or is already closed.')

    def fetchmany(self, size=1):
        """
        Fetch the next set of rows of a query result, returning a sequence of sequences (e.g. a list of tuples). An
        empty sequence is returned when no more rows are available. If there are not enough rows to fulfil the size
        parameter, only the available rows will be returned.

        :param size: The number of the rows to be returned from the cursor.
        :return: A list with the fetched elements.
        """
        if self.initialized:
            if size > 0:
                if self.rowcount >= 1:

                    qty_elements = len(self.result)

                    if qty_elements >= size:
                        try:
                            response_set = self.result[0:size]
                            del self.result[0:size]
                        except IndexError:
                            return None
                        else:
                            return response_set
                    else:
                        try:
                            response_set = self.result[0:qty_elements]
                            del self.result[0:qty_elements]
                        except IndexError:
                            return None
                        else:
                            return response_set
                else:
                    raise InternalError('Error: No rows in cursor available.')
            else:
                raise InterfaceError('Error: Invalid set size.')
        else:
            raise InternalError('The cursor has not been initialized or is already closed.')

    def fetchall(self):
        """
        Fetch all (remaining) rows of a query result, returning them as a sequence of sequences (e.g. a list of tuples).
        An empty sequence is returned when no more rows are available.

        An InterfaceError exception is raised if the previous call to .execute*() did not produce any result set or
        no call was issued yet.

        :return: A list with the fetched elements.
        """
        if self.initialized:
            qty_elements = len(self.result)
            if self.rowcount >= 1:
                if qty_elements > 0:
                    return self.fetchmany(qty_elements)
                else:
                    return []
            else:
                raise InternalError('Error: No rows in cursor available.')
        else:
            raise InternalError('The cursor has not been initialized or is already closed.')

    def execute_create_empty_relation(self, relation_name: str, list_attributes: []):
        """
        Creates an empty relation in the database. The list of the attributes must follow the following format:

        list_attributes = [attr]

        where

        attr = [Name, type]

        IMPORTANT: The name of the attributes must begin with an upper case character.

        Example:

        attr1 = ['Att1', 'string']
        attr2 = ['Att2', 'string']

        list_attributes = [attr1, attr2]

        :param relation_name: The name of the relation.
        :param list_attributes: A list with the attributes of the relation (please be aware of the format).
        :return: The string 'Success', if the relation was successfully created, otherwise None.
        """
        if self.initialized:

            if len(list_attributes) > 0:
                str_attr_list = '['
                counter = 0
                for attr in list_attributes:

                    if attr[0] is not None:
                        str_attr_list = str_attr_list + attr[0] + ' : ' + attr[1]
                        if counter < len(list_attributes) - 1:
                            str_attr_list = str_attr_list + ', '
                        counter += 1
                    else:
                        raise ProgrammingError('The attribute at position '
                                               + str(counter) + ' has no valid identifier.')

                str_attr_list = str_attr_list + ']'

                command_string = com.SECONDO_COM_REL_CREATE_EMPTY.format(relation_name, str_attr_list)

                try:
                    response = self.execute(command_string)
                    return response[0]
                except ProgrammingError:
                    raise

            else:
                raise ProgrammingError('Error: List of attributes is empty.')

        else:
            raise InternalError('The cursor has not been initialized or is already closed.')

    def execute_insert_tuple_into_relation(self, relation_name: str, single_tuple: []):
        """
        Inserts a list of values into a relation.

        :param relation_name: The name of the relation.
        :param single_tuple: A single tuple of the relation. The single tuple is a list must contain two elements: the
                             first element is a list with the values of the tuple, and the second element is a list with
                             the types of the relation.
        :return: The string 'Success', if the tuples were successfully added, otherwise None.
        """

        if self.initialized:

            if len(single_tuple) > 0:

                counter = 0

                str_value_list = []

                for column in range(len(single_tuple[0])):

                    field_type = single_tuple[1][column]
                    value = single_tuple[0][column]

                    const_value = com.SECONDO_COM_REL_CONST_VALUE.format(field_type, value)

                    str_value_list.append(const_value)
                    if counter < len(single_tuple[0]) - 1:
                        str_value_list.append(', ')

                    # str_value_list = str_value_list + const_value
                    # if counter < len(single_tuple[0]) - 1:
                    #     str_value_list = str_value_list + ', '
                    counter += 1

                str_value = "".join(str_value_list)

                command_string = com.SECONDO_COM_REL_INSERT_TUPLE.format(relation_name, str_value)

                try:
                    response = self.execute(command_string)
                    return response[0]
                except ProgrammingError:
                    raise

            else:
                raise ProgrammingError('Error: List of values is empty.')

        else:
            raise InternalError('The cursor has not been initialized or is already closed.')

    def execute_simple_query(self, value_expression):
        """
        Evaluates the given value expression and displays the result object.

        :param value_expression: A value expression for the command 'query <value expression>'
        :return: A list expression object with the response from the |sec| server.
        """

        if self.initialized:

            command_string = com.SECONDO_COM_QUERY.format(value_expression)

            try:
                response = self.execute(command_string)
            except ProgrammingError:
                raise
            else:
                return response[0]

        else:
            raise InternalError('The cursor has not been initialized or is already closed.')

    def execute_let(self, identifier, value_expression):
        """
        This command does almost the same as the query command. In contrast, the result of the <value expression> is not
        displayed on the screen. Instead, the result is stored in an object with the name <identifier>. The command only
        runs successfully if the object does not exist yet in the database; otherwise, an error message is displayed.

        :param identifier: The identifier of the object.
        :param value_expression: A value expression for the command 'let <identifier> = <value expression>'
        :return: A list expression object with the response from the |sec| server.
        """

        if self.initialized:

            if not parser.check_identifier(identifier):
                raise OperationalError('Invalid identifier.')

            command_string = com.SECONDO_COM_LET.format(identifier, value_expression)

            try:
                response = self.execute(command_string)
            except ProgrammingError:
                raise
            else:
                return response[0]

        else:
            raise InternalError('The cursor has not been initialized or is already closed.')

    def execute_derive(self, identifier, value_expression):
        """
        This works basically in the same way as the let command. The difference is the handling of the created objects
        during creating and restoring a database dump. The derive command should be used for objects that have no
        external representation, e.g., indexes.

        :param identifier: The identifier of the object.
        :param value_expression: A value expression for the command 'derive <identifier> = <value expression>'
        :return: None
        """

        if self.initialized:

            if not parser.check_identifier(identifier):
                raise OperationalError('Invalid identifier.')

            command_string = com.SECONDO_COM_DERIVE.format(identifier, value_expression)

            try:
                response = self.execute(command_string)
            except ProgrammingError:
                raise
            else:
                return response[0]

        else:
            raise InternalError('The cursor has not been initialized or is already closed.')

    def execute_update(self, identifier, value_expression):
        """
        Assigns the result of the value expression to an existing object in the database.

        :param identifier: The identifier of the object.
        :param value_expression: A value expression for the command 'update <identifier> := <value expression>'
        :return: The string Success, if the object was updated successfully.
        """
        if self.initialized:

            if not parser.check_identifier(identifier):
                raise OperationalError('Invalid identifier.')

            command_string = com.SECONDO_COM_UPDATE.format(identifier, value_expression)

            try:
                response = self.execute(command_string)
            except ProgrammingError:
                raise
            else:
                return response[0]
        else:
            raise InternalError('The cursor has not been initialized or is already closed.')

    def execute_delete(self, identifier):
        """
        Deletes the object with the name <identifier> from the currently opened database.

        :param identifier: The identifier of the object.
        :return: The string Success, if the object was deleted successfully.
        """
        if self.initialized:
            if not parser.check_identifier(identifier):
                raise OperationalError('Invalid identifier.')

            command_string = com.SECONDO_COM_DELETE.format(identifier)

            try:
                response = self.execute(command_string)
            except ProgrammingError:
                raise
            else:
                return response[0]
        else:
            raise InternalError('The cursor has not been initialized or is already closed.')

    def execute_create_type(self, identifier, type_expression):
        """
        Creates a named type in the database.

        :param identifier: The identifier of the type to be created.
        :param type_expression: A type expression for the command 'type <identifier> = <type expression>'
        :return: The string Success, if the named type was created successfully.
        """
        if self.initialized:
            if not parser.check_identifier(identifier):
                raise OperationalError('Invalid identifier.')

            command_string = com.SECONDO_COM_TYPE.format(identifier, type_expression)

            try:
                response = self.execute(command_string)
            except ProgrammingError:
                raise
            else:
                return response[0]
        else:
            raise InternalError('The cursor has not been initialized or is already closed.')

    def execute_delete_type(self, identifier):
        """
        Deletes a named type from the database.

        :param identifier: The identifier of the type to be deleted.
        :return: The string Success, if the named type was deleted successfully.
        """
        if self.initialized:
            if not parser.check_identifier(identifier):
                raise OperationalError('Invalid identifier.')

            command_string = com.SECONDO_COM_DELETE_TYPE.format(identifier)

            try:
                response = self.execute(command_string)
            except ProgrammingError:
                raise
            else:
                return response[0]
        else:
            raise InternalError('The cursor has not been initialized or is already closed.')

    def execute_create(self, identifier, type_expression):
        """
        Creates an object of the given type with undefined value.

        :param identifier: The identifier of the object to be created.
        :param type_expression: A type expression for the command 'create <identifier> = <type expression>'
        :return: The string Success, if the object was created successfully.
        """
        if self.initialized:
            if not parser.check_identifier(identifier):
                raise OperationalError('Invalid identifier.')

            command_string = com.SECONDO_COM_CREATE.format(identifier, type_expression)

            try:
                response = self.execute(command_string)
            except ProgrammingError:
                raise
            else:
                return response[0]
        else:
            raise InternalError('The cursor has not been initialized or is already closed.')

    def execute_kill(self, identifier):
        """
        Removes the object with the name <identifier> from the opened database catalog without removing its data
        structures. Generally, the delete command should be used to remove database objects. The kill command should
        only be used if the delete command crashes the database due to corrupted persistent data structures for this
        object.

        :param identifier: The identifier of the object to be "killed".
        :return: The string Success, if the object was "killed" successfully.
        """
        if self.initialized:
            if not parser.check_identifier(identifier):
                raise OperationalError('Invalid identifier.')

            command_string = com.SECONDO_COM_KILL.format(identifier)

            try:
                response = self.execute(command_string)
            except ProgrammingError:
                raise
            else:
                return response[0]
        else:
            raise InternalError('The cursor has not been initialized or is already closed.')
