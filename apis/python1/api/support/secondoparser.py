# ----------------------------------------------------------------------------------------------------------------------
# The Secondo Python API (pySecondo)
# Victor Silva (victor.silva@posteo.de)
# October 2019
# ----------------------------------------------------------------------------------------------------------------------
# Module
# ----------------------------------------------------------------------------------------------------------------------
# Secondo Parser
# secondoparser.py
# ----------------------------------------------------------------------------------------------------------------------
"""
The module Secondo Parser implements functions for parsing a response to an inquiry or a command from the |sec|
server. A list expression object can be constructed through the function receive_response. A list expression object
for a specific inquiry can be then converted to a Python structure (dictionary or list), which can be used to print or
display the results in an ad-hoc format.
"""
from socket import socket
import secondodb.api.support.secondomessages as messages
import secondodb.api.support.secondocommands as com
from secondodb.api.support.secondolistexpr import ListExp
import secondodb.api.support.secondoinputhandler as inputhandler
import secondodb.api.algebras.secondostandardalgebra as standardalgebra
import secondodb.api.algebras.secondospatialalgebra as spatialalgebra
import secondodb.api.algebras.secondorelationalalgebra as relationalalgebra
import secondodb.api.algebras.secondospatiotemporalalgebra as spatiotemporalalgebra
import secondodb.api.secondoapi as api
import re
from secondodb.api.algebras.secondogeneraldataclasses import Attribute, Type, Operator, Algebra, Object


def receive_response(socket_object: socket):
    """
    This method handles the |sec| response from the server. After processing two parameters will be returned: The first
    parameter contains the opening string of the response (for example <SecondoIntro>). The second parameter depends on
    this string, as it can be a simple string with a message or a List Expression object with the result of an inquiry.

    In case of an error, a dictionary structure with the details (error code, position and message) will be returned. In
    any case, the returned result list corresponds to the fourth element of the obtained list expression object from the
    server response.

    :param socket_object: A socket object with the response from the server.
    :return: The opening string of the response and a list or a message.
    """

    try:
        line = inputhandler.read_line_of_text(socket_object)
    except api.OperationalError:
        raise

    if line == messages.SECONDO_OK:
        ok_message = 'Response from Secondo Server received.'
        return messages.SECONDO_OK, ok_message

    # Handling of the case <SecondoIntro>

    if line == messages.SECONDO_INTRO_START:

        success_message = ''

        while True:
            try:
                message = inputhandler.read_line_of_text(socket_object)
            except api.OperationalError:
                raise
            if message != messages.SECONDO_INTRO_END:
                success_message = message
            else:
                break
        return messages.SECONDO_INTRO_START, success_message

    # Handling of the case <Message>

    if line == messages.SECONDO_MESSAGE_START:

        while line == messages.SECONDO_MESSAGE_START:

            result_list = inputhandler.build_list_expr_from_binary(socket_object)

            first_element = result_list.get_first_element()
            second_element = result_list.get_second_element()

            if first_element.value == messages.SECONDO_PROGRESS:
                __progress_display(second_element.value.value, second_element.next.value, 'completed.')

            try:
                response_end = inputhandler.read_line_of_text(socket_object)
            except api.OperationalError:
                raise

            # Get the end line of the response

            if response_end != messages.SECONDO_MESSAGE_END:
                raise api.InterfaceError('End of response not reached.')

            try:
                line = inputhandler.read_line_of_text(socket_object)
            except api.OperationalError:
                raise

        __line_flush()

    # Handling of the case <SecondoError>

    if line == messages.SECONDO_ERROR_START:
        raise api.InterfaceError('Unknown error')

    # Handling of the case <SecondoResponse>

    if line == messages.SECONDO_RESPONSE_START:

        response_string = messages.SECONDO_RESPONSE_START
        result_list = inputhandler.build_list_expr_from_binary(socket_object)

        # Error handling - Error details will be returned as a dictionary structure

        error_code = result_list.get_first_element().value
        error_pos = result_list.get_second_element().value
        error_message = result_list.get_third_element().value

        error_dict = {}

        error_dict.update({'code': error_code})
        error_dict.update({'pos': error_pos})
        error_dict.update({'message': error_message})

        # Set the fourth element of the result list expression object as the returned result list

        result_list = result_list.get_fourth_element()

        try:
            response_end = inputhandler.read_line_of_text(socket_object)
        except api.OperationalError:
            raise

        # Get the end line of the response

        if response_end != messages.SECONDO_RESPONSE_END:
            raise api.InterfaceError('Fatal Error: End of response not reached.')

        return response_string, result_list, error_dict

    else:

        return ListExp()


def parse_inquiry_databases(list_expr: ListExp) -> []:
    """
    Returns a list with the database names of a database inquiry.

    :param list_expr: A List Expression object with a database inquiry.
    :return: A list with the database names.
    """

    db_list = []

    second_element = list_expr.get_second_element()
    databases = second_element.get_second_element()

    next_element = databases

    while next_element.next is not None:
        db_list.append(next_element.value.value)
        next_element = next_element.next

    return db_list


def parse_inquiry_algebras(list_expr: ListExp) -> []:
    """
    Returns a list with the algebras of the server.

    :param list_expr: A list expression object.
    :return: The list of the algebras.
    """

    algebra_list = []

    second_element = list_expr.get_second_element()
    algebras = second_element.get_second_element()

    next_element = algebras

    while next_element.next is not None:
        algebra_list.append(next_element.value.value)
        next_element = next_element.next

    return algebra_list


def parse_inquiry_algebra(list_expr: ListExp) -> Algebra:
    """
    Returns a list with the objects of a database.

    :param list_expr: A response list.
    :return: An Algebra object.
    """

    second_element = list_expr.get_second_element()

    algebra_name = second_element.get_second_element().value.value

    type_constructors = second_element.get_second_element().get_second_element().get_first_element()
    operators = second_element.get_second_element().get_second_element().get_second_element()

    # ------------------------------------------------ Process type constructors

    type_list = []

    next_element = type_constructors

    while next_element.next is not None:

        type_le = next_element.value

        first_element = type_le.get_first_element()  # Type name

        type_name = first_element.value

        # Process properties headers

        second_element = type_le.get_second_element()  # Headers

        headers = []

        while second_element.next is not None:
            headers.append(second_element.value.value)
            second_element = second_element.next

        # Process properties values

        third_element = type_le.get_third_element()  # Values

        values = []

        while third_element.next is not None:
            values.append(third_element.value.value)
            third_element = third_element.next

        properties = {}

        properties_iter = list(zip(headers, values))

        for prop_name, prop_value in properties_iter:
            properties.update({prop_name: prop_value})

        type_list.append(Type(type_name=type_name, properties=properties))

        next_element = next_element.next

    # ------------------------------------------------ Process operators

    operator_list = []

    next_element = operators

    while next_element.next is not None:

        operator_le = next_element.value

        first_element = operator_le.get_first_element()

        operator_name = first_element.value

        # Process properties headers

        second_element = operator_le.get_second_element()  # Headers

        headers = []

        while second_element.next is not None:
            headers.append(second_element.value.value)
            second_element = second_element.next

        # Process properties values

        third_element = operator_le.get_third_element()  # Values

        values = []

        while third_element.next is not None:
            values.append(third_element.value.value)
            third_element = third_element.next

        properties = {}

        properties_iter = list(zip(headers, values))

        for prop_name, prop_value in properties_iter:
            properties.update({prop_name: prop_value})

        operator_list.append(Operator(operator_name=operator_name, properties=properties))

        next_element = next_element.next

    return Algebra(algebra_name=algebra_name, type_list=type_list, operator_list=operator_list)


def parse_inquiry_type_constructors(list_expr: ListExp) -> []:
    """
    Parses a types inquiry from a |sec| server response.

    :param list_expr: A list expression object with the types of the response.

    :return: A list containing the type constructors.
    """

    type_list = []

    second_element = list_expr.get_second_element()
    types = second_element.get_second_element()

    next_element = types

    while next_element.next is not None:
        type_list.append(next_element.value.value.value)
        next_element = next_element.next

    return type_list


def parse_inquiry_types(list_expr: ListExp) -> []:
    """
    Parses a types inquiry from a |sec| server response.

    :param list_expr: A list expression object with the types of the response.

    :return: A list with objects of the class Type containing the types.
    """

    type_list = []

    second_element = list_expr.get_second_element()
    types = second_element.get_second_element().next
    next_element = types

    while next_element.next is not None:

        single_value = next_element.value

        type_name = single_value.get_second_element().value
        properties = single_value.get_third_element().value

        type_list.append(Type(type_name=type_name, properties=properties))

        next_element = next_element.next

    return type_list


def parse_type_definition(list_expr: ListExp):
    """
    Parses a type definition element from a |sec| server response.

    :param list_expr: A list expression object with the type definition of the response.
    :return: A string value with the type and a list with the attributes, if the type corresponds to a relation.
             Otherwise the list parameter will be set to None. Every single attribute is an object of the class
             Attribute.
    """

    attr_list = None

    type_definition = list_expr.get_first_element()

    if isinstance(type_definition, ListExp):

        # Describe relation

        dtype = type_definition.value

        if dtype == 'rel':

            tuple_as_le = list_expr.get_second_element()

            attributes_as_le = tuple_as_le.get_second_element()
            number_of_attributes = attributes_as_le.get_list_length()

            attr_list = []
            for i in range(1, number_of_attributes):

                single_attribute_as_le = attributes_as_le.get_the_n_element(i)

                attribute_name = single_attribute_as_le.get_first_element().value
                attribute_type = single_attribute_as_le.get_second_element().value

                attr_list.append(Attribute(attribute_type=attribute_type, attribute_name=attribute_name))

        else:
            pass

    else:  # Atom
        dtype = type_definition

    return dtype, attr_list


def parse_inquiry_objects(list_expr: ListExp) -> []:
    """
    Parses an objects inquiry from a |sec| server response.

    :param list_expr: A list expression object with the types of the response.

    :return: A list with objects of the class Object containing the types.
    """

    objects = []

    list_objects_as_le = list_expr.get_second_element().get_second_element()
    number_of_objects = list_objects_as_le.get_list_length()

    for i in range(2, number_of_objects):

        single_object_as_le = list_objects_as_le.get_the_n_element(i)

        # Do something with the object, which comes with the format (OBJECT BGrenzenLine () (line))

        object_name_as_le = single_object_as_le.get_second_element()
        object_type_as_le = single_object_as_le.get_fourth_element().value

        dtype, attr_list = parse_type_definition(object_type_as_le)

        object_name = object_name_as_le.value
        object_type = dtype
        attributes = attr_list

        objects.append(Object(object_name=object_name, object_type=object_type, attributes=attributes))

    return objects


def parse_query(list_expr: ListExp):
    """
    Converts the result of a query in list expression format into Python objects.

    :param list_expr: A list expression object.
    :return: A Python object.
    """

    first_element = list_expr.get_first_element()

    dtype, attr_list = parse_type_definition(first_element)

    object_type = dtype
    object_data = list_expr.next.value

    # ---------------------------------------------------- STANDARD ALGEBRA

    if object_type == 'int':
        int_value = standardalgebra.parse_int(object_data)
        return int_value, object_type
    elif object_type == 'real':
        real_value = standardalgebra.parse_real(object_data)
        return real_value, object_type
    elif object_type == 'bool':
        bool_value = standardalgebra.parse_bool(object_data)
        return bool_value, object_type
    elif object_type == 'string':
        string_value = standardalgebra.parse_string(object_data)
        return string_value, object_type
    elif object_type == 'longint':
        longint_value = standardalgebra.parse_longint(object_data)
        return longint_value, object_type
    elif object_type == 'rational':
        rational_value = standardalgebra.parse_rational(object_data)
        return rational_value, object_type

    # ---------------------------------------------------- SPATIAL ALGEBRA

    elif object_type == 'point':
        point = spatialalgebra.parse_point(object_data)
        return point, object_type
    elif object_type == 'points':
        points = spatialalgebra.parse_points(object_data)
        return points, object_type
    elif object_type == 'line':
        line = spatialalgebra.parse_line(object_data)
        return line, object_type
    elif object_type == 'region':
        region = spatialalgebra.parse_region(object_data)
        return region, object_type

    # ---------------------------------------------------- RELATIONAL ALGEBRA

    elif object_type == 'rel':
        rel = relationalalgebra.parse_relation(object_data, attr_list)
        return rel, object_type

    # ---------------------------------------------------- SPATIO TEMPORAL ALGEBRA

    elif object_type == 'mpoint':
        mpoint = spatiotemporalalgebra.parse_mpoint(object_data)
        return mpoint, object_type
    elif object_type == 'mregion':
        mregion = spatiotemporalalgebra.parse_mregion(object_data)
        return mregion, object_type
    elif object_type == 'mreal':
        mreal = spatiotemporalalgebra.parse_mreal(object_data)
        return mreal, object_type
    elif object_type == 'mint':
        mint = spatiotemporalalgebra.parse_mint(object_data)
        return mint, object_type
    elif object_type == 'mstring':
        mstring = spatiotemporalalgebra.parse_mstring(object_data)
        return mstring, object_type
    elif object_type == 'iregion':
        iregion = spatiotemporalalgebra.parse_iregion(object_data)
        return iregion, object_type
    elif object_type == 'ipoint':
        ipoint = spatiotemporalalgebra.parse_ipoint(object_data)
        return ipoint, object_type

    else:
        raise api.InterfaceError('The type of the response (' + object_type + ') is currently not supported')


def check_identifier(identifier: str) -> bool:
    """
    Checks the validity of a given value expression.

    :param identifier:
    :return: True, if the value expression is valid.
    """

    if bool(re.match('^[0-9a-zA-Z_]+$', identifier)):
        if identifier[0] == '_':
            return False
        else:
            if bool(re.match('^[0-9]', identifier)):
                return False
            else:
                return True


def check_port(port: str) -> bool:
    """
    Checks the validity of a given port value.

    :param port: The number of the port as string.
    :return: True, if the port value is valid.
    """
    if bool(re.match('^[0-9]+$', port)):
        return True
    else:
        return False


def check_validity_string(validity_string: str) -> bool:
    """
    Checks the validity string of a |sec| response.

    :param validity_string: The string with the value 'bnl'.
    :return: True, if the string has the bnl-value. Otherwise a DataError will be raised.
    """

    if not validity_string == com.SECONDO_VALIDITY_STRING:
        raise api.DataError('The response is not valid.')
    else:
        return True


def __progress_display(position, total, message):
    """
    Internal function for displaying the execution progress of a query on the Python console.

    :param position: Integer value representing the time progress of the query.
    :param total: Integer value representing the total time amount of the query.
    :param message: A text message, which will be appended next to the progress indicator.
    :return: None
    """
    if position != 0 and total != 255:
        print("\r" + str(position) + "/" + str(total) + " " + message, end='', flush=True)


def __line_flush():
    """
    Internal function for reseting a line of the Python console.

    :return: None
    """
    print("\r" + "")

