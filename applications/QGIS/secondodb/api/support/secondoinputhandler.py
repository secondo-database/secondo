# ----------------------------------------------------------------------------------------------------------------------
# The Secondo Python API (pySecondo)
# Victor Silva (victor.silva@posteo.de)
# January 2020
# ----------------------------------------------------------------------------------------------------------------------
# Module
# ----------------------------------------------------------------------------------------------------------------------
# Secondo Input Handler
# secondoinputhandler.py
# ----------------------------------------------------------------------------------------------------------------------
"""
The module Secondo Input Handler handles the conversion of different values from a socket stream in form of binary
values. The implementation is based on the Java-Class MyInputStreamReader of the Secondo JavaGUI and supports the
reception and construction of a list expression object originated through the response to an inquiry (or command) from
the |sec| server.
"""

import struct
from socket import socket

import secondodb.api.secondoapi as api
import secondodb.api.support.secondocommands as com
import secondodb.api.support.secondolistexpr as listexpr

encoding = 'Latin-1'

# Constants describing codings for binary writing of a nested list

BIN_LONGLIST = 0
BIN_INTEGER = 1
BIN_REAL = 2
BIN_BOOLEAN = 3
BIN_LONGSTRING = 4
BIN_LONGSYMBOL = 5
BIN_LONGTEXT = 6
BIN_LIST = 10
BIN_SHORTLIST = 11
BIN_SHORTINT = 12
BIN_BYTE = 13
BIN_STRING = 14
BIN_SHORTSTRING = 15
BIN_SYMBOL = 16
BIN_SHORTSYMBOL = 17
BIN_TEXT = 18
BIN_SHORTTEXT = 19
BIN_DOUBLE = 20


def read_line_of_text(socket_object: socket) -> str:
    """
    Returns a single text line from a response. The input stream will be read until the occurrence of a new line.

    :param socket_object: A socket object.
    :return: A string with a single text line.
    """

    byte_array = bytearray()

    while True:

        byte = socket_object.recv(1)

        try:
            if byte != b'\n':
                byte_array.append(byte[0])
            else:
                break
        except IndexError:
            raise api.OperationalError('Error: Invalid response from server.')

    try:
        string: str = byte_array.decode(encoding)
    except UnicodeDecodeError as e:
        raise api.InterfaceError(e.args[0])
    else:
        return string


def read_string(socket_object: socket, size: int) -> str:
    """
    Reads a string of a specified size from a socket object.

    :param socket_object: A socket object with the response from the |sec| server.
    :param size: The size of the string.
    :return: A string value.
    """

    byte_array = bytearray()

    for i in range(0, size):
        byte = socket_object.recv(1)
        byte_array.append(byte[0])

    try:
        string: str = byte_array.decode(encoding)
    except UnicodeDecodeError as e:
        raise api.InterfaceError(e.args[0])
    else:
        return string


def read_short(socket_object: socket) -> int:
    """
    Reads a short value from the socket response. A short value takes 2 bytes to be built.

    :param socket_object: A socket object with the response from the |sec| server.
    :return: A short value.
    """

    byte_vector = socket_object.recv(2)
    short_value: int = struct.unpack('>H', byte_vector)[0]

    return short_value


def read_byte(socket_object: socket) -> bytes:
    """
    Reads a single byte from the socket response.

    :param socket_object: A socket object with the response from the |sec| server.
    :return: A byte value.
    """

    byte_value = socket_object.recv(1)

    return byte_value


def read_int(socket_object: socket) -> int:
    """
    Reads an integer value from the socket response. An integer value takes 4 bytes to be built.

    :param socket_object: A socket object with the response from the |sec| server.
    :return: An integer value.
    """

    byte_vector = socket_object.recv(4)
    int_value: int = struct.unpack('>I', byte_vector)[0]

    return int_value


def read_real(socket_object: socket) -> float:
    """
    Reads a real value from the socket response.

    :param socket_object: A socket object with the response from the |sec| server.
    :return: A float representing the real value.
    """

    byte_vector = socket_object.recv(4)
    real_value: float = struct.unpack('>f', byte_vector)[0]

    return real_value


def read_bool(socket_object: socket) -> bool:
    """
    Reads a boolean value from the socket response.

    :param socket_object: A socket object with the response from the |sec| server.
    :return: A boolean value.
    """

    int_value = read_byte(socket_object)[0]

    if int_value == 0:
        return False
    else:
        return True


def read_double(socket_object: socket) -> float:
    """
    Reads a double value from the socket response.

    :param socket_object: A socket object with the response from the |sec| server.
    :return: A float representing the double value.
    """

    byte_vector = socket_object.recv(8)
    double_value: float = struct.unpack('>d', byte_vector)[0]

    return double_value


def read_long(socket_object: socket) -> int:
    """
    Reads a long value from the socket response.

    :param socket_object: A socket object with the response from the |sec| server.
    :return: A long (int) value.
    """

    byte_vector = socket_object.recv(8)
    long_value: int = struct.unpack('>Q', byte_vector)[0]

    return long_value


def read_binary_record(socket_object: socket) -> listexpr.ListExp:
    """
    Reads on each iteration a single buffered vector of bytes and translates it to the specified type. Some types
    requiere the specification of the length for the buffer. The length is normally expressed as the first binary value
    after the specification of the type integer.

    :param socket_object: A socket object with the response from the |sec| server.
    :return: None
    """

    # Read byte from the socket to identify the type

    type_value = read_byte(socket_object)

    if type_value[0] == BIN_LONGLIST:

        length = read_int(socket_object)

        if length == 0:
            return listexpr.ListExp()
        else:
            le_f = read_binary_record(socket_object)
            le_le = listexpr.one_element_list(le_f)
            le_last = le_le

            for i in range(1, length):
                le_next = read_binary_record(socket_object)
                le_last = listexpr.append_to_last_element(le_last, le_next)

        return le_le

    elif type_value[0] == BIN_INTEGER:

        int_value = read_int(socket_object)
        return listexpr.create_integer_atom(int_value)

    elif type_value[0] == BIN_REAL:

        real_value = read_real(socket_object)
        return listexpr.create_real_atom(real_value)

    elif type_value[0] == BIN_BOOLEAN:

        bool_value = read_bool(socket_object)
        return listexpr.create_bool_atom(bool_value)

    elif type_value[0] == BIN_LONGSTRING:

        length = read_int(socket_object)
        string_value = read_string(socket_object, length)
        return listexpr.create_string_atom(string_value)

    elif type_value[0] == BIN_LONGSYMBOL:

        length = read_int(socket_object)
        symbol_value = read_string(socket_object, length)
        return listexpr.create_symbol_atom(symbol_value)

    elif type_value[0] == BIN_LONGTEXT:

        length = read_int(socket_object)
        text = read_string(socket_object, length)
        return listexpr.create_text_atom(text)

    elif type_value[0] == BIN_LIST:

        length = read_short(socket_object)

        if length == 0:
            return listexpr.ListExp()
        else:
            le_f = read_binary_record(socket_object)
            le_le = listexpr.one_element_list(le_f)
            le_last = le_le

            for i in range(1, length):
                le_next = read_binary_record(socket_object)
                le_last = listexpr.append_to_last_element(le_last, le_next)

        return le_le

    elif type_value[0] == BIN_SHORTLIST:

        length = read_byte(socket_object)[0]

        if length == 0:
            return listexpr.ListExp()
        else:
            le_f = read_binary_record(socket_object)
            le_le = listexpr.one_element_list(le_f)
            le_last = le_le

            for i in range(1, length):
                le_next = read_binary_record(socket_object)
                le_last = listexpr.append_to_last_element(le_last, le_next)

        return le_le

    elif type_value[0] == BIN_SHORTINT:

        short_int_value = read_short(socket_object)
        return listexpr.create_integer_atom(short_int_value)

    elif type_value[0] == BIN_BYTE:

        return listexpr.create_integer_atom(read_byte(socket_object)[0])

    elif type_value[0] == BIN_STRING:

        length = read_short(socket_object)
        string = read_string(socket_object, length)
        return listexpr.create_string_atom(string)

    elif type_value[0] == BIN_SHORTSTRING:

        length = read_byte(socket_object)[0]
        string_value = read_string(socket_object, length)
        return listexpr.create_string_atom(string_value)

    elif type_value[0] == BIN_SYMBOL:

        length = read_short(socket_object)
        symbol_value = read_string(socket_object, length)
        return listexpr.create_symbol_atom(symbol_value)

    elif type_value[0] == BIN_SHORTSYMBOL:

        length = read_byte(socket_object)[0]
        symbol_value = read_string(socket_object, length)
        return listexpr.create_symbol_atom(symbol_value)

    elif type_value[0] == BIN_TEXT:

        length = read_short(socket_object)
        text = read_string(socket_object, length)
        return listexpr.create_text_atom(text)

    elif type_value[0] == BIN_SHORTTEXT:

        length = read_byte(socket_object)[0]
        text = read_string(socket_object, length)
        return listexpr.create_text_atom(text)

    elif type_value[0] == BIN_DOUBLE:

        double_value = read_double(socket_object)
        return listexpr.create_real_atom(double_value)


def build_list_expr_from_binary(socket_object: socket) -> listexpr.ListExp:
    """
    This method builds a list expression object from a binary response from the |sec| server.
    In javagui: readBinaryFrom()

    :param socket_object:
    :return: A list expression object with the contents of the nested list.
    """

    # Read and check signature string 'bnl'

    signature = read_string(socket_object, 3)

    if signature != com.SECONDO_VALIDITY_STRING:
        raise api.InterfaceError('Wrong validity string')

    # Read and check version

    version_1 = read_short(socket_object)  # major
    version_2 = read_short(socket_object)  # minor

    if version_1 != 1 or (version_2 != 0 and version_2 != 1 and version_2 != 2):
        raise api.InterfaceError('Wrong version number: ' + str(version_1) + "/" + str(version_2))

    # Create list expression from binary

    list_expr = read_binary_record(socket_object)

    return list_expr
