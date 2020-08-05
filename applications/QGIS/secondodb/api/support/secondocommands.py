# ----------------------------------------------------------------------------------------------------------------------
# The Secondo Python API (pySecondo)
# Victor Silva (victor.silva@posteo.de)
# October 2019
# ----------------------------------------------------------------------------------------------------------------------
# Module
# ----------------------------------------------------------------------------------------------------------------------
# Secondo Commands
# secondocommands.py
# ----------------------------------------------------------------------------------------------------------------------
"""
The module Secondo Commands contains the strings of the commands to interact with the |sec| server and its objects.
The strings can be formatted using the Python method format() to replace the placeholders (i.e. {0} or {1}).
"""

# ---- Basic commands ---- #
import re
# import secondodb.api.secondoapi as api
from secondodb.api import secondoapi as api

SECONDO_COM_QUERY = 'query {0}'
SECONDO_COM_LET = 'let {0} = {1}'
SECONDO_COM_DERIVE = 'derive {0} = {1}'
SECONDO_COM_UPDATE = 'update {0} := {1}'
SECONDO_COM_DELETE = 'delete {0}'
SECONDO_COM_TYPE = 'type {0} = {1}'
SECONDO_COM_DELETE_TYPE = 'delete type {0}'
SECONDO_COM_CREATE = 'create {0} : {1}'
SECONDO_COM_KILL = 'kill {0}'

# ---- Databases ----- #

SECONDO_COM_CREATE_DB = 'create database {0}'
SECONDO_COM_OPEN_DB = 'open database {0}'
SECONDO_COM_CLOSE_DB = 'close database'
SECONDO_COM_DELETE_DB = 'delete database {0}'
SECONDO_COM_RESTORE_DB = 'restore database {0}'

# ---- Transactions ----- #

SECONDO_COM_BEGIN_TR = 'begin transaction'
SECONDO_COM_COMMIT_TR = 'commit transaction'
SECONDO_COM_ABORT_TR = 'abort transaction'

# ---- Inquiries ----- #

SECONDO_COM_LIST_DB = 'list databases'
SECONDO_COM_LIST_TYPE_CONS = 'list type constructors'
SECONDO_COM_LIST_OPERATORS = 'list operators'
SECONDO_COM_LIST_ALGEBRAS = 'list algebras'
SECONDO_COM_LIST_ALGEBRA = 'list algebra {0}'
SECONDO_COM_LIST_TYPES = 'list types'
SECONDO_COM_LIST_OBJECTS = 'list objects'

# ---- Relations ----- #

SECONDO_COM_REL_CREATE_EMPTY = 'let {0} = [ const rel (tuple({1})) value () ]'
SECONDO_COM_REL_INSERT_TUPLE = 'query {0} inserttuple[{1}] count'
SECONDO_COM_REL_CONST_VALUE = '[const {0} value {1}]'

# ---- Import and Export ----- #

SECONDO_COM_SAVE_DB = 'save database to {0}'
SECONDO_COM_RESTORE_DB_FROM = 'restore database {0} from {1}'
SECONDO_COM_SAVE_OBJECT = 'save {0} to {1}'
SECONDO_COM_RESTORE_OBJECT = 'restore {0} from {1}'

SECONDO_VALIDITY_STRING = 'bnl'

SECONDO_COM_ID_OBJECT = 'OBJECT'
SECONDO_COM_ID_OBJECTS = 'OBJECTS'

SECONDO_COM_TYPE_REL = 'rel'
SECONDO_COM_TYPE_TUPLE = 'tuple'


def apply_parameters_to_operation(operation, parameter_list):
    """
    Replaces the placeholders of the operation with the parameters of the list.

    :param operation: A string with an operation, which includes placeholders for the parameters.
    :param parameter_list: A list of parameters.
    :return: The formatted operation.
    """

    try:
        formatted_operation = operation.format(*parameter_list)
    except IndexError:
        param_supplied = len(parameter_list)
        param_needed = len(re.findall(r"{(\w+)}", operation))
        raise api.ProgrammingError('Error in parameters: ' + str(param_supplied) + ' parameter(s) supplied, '
                                   + str(param_needed) + ' where expected.')
    else:
        return formatted_operation
