# ----------------------------------------------------------------------------------------------------------------------
# The Secondo Python API (pySecondo)
# Victor Silva (victor.silva@posteo.de)
# January 2020
# ----------------------------------------------------------------------------------------------------------------------
# Module
# ----------------------------------------------------------------------------------------------------------------------
# Secondo Standard Algebra
# secondosstandardalgebra.py
# ----------------------------------------------------------------------------------------------------------------------
"""
The module Secondo Standard Algebra implements the data types for the conversion of list expression objects with values
of the types contained in the StandardAlgebra of the |sec| system. The data types are implemented in Python
using Data Classes. Data Classes are implemented in the API like normal classes without behaviour.
Like regular classes their attributes can be called through the given names.
"""

from secondodb.api.support.secondolistexpr import ListExp


def parse_int(list_expr: ListExp) -> int:
    """
    Parses a list expression for an integer object.

    :param list_expr: A list expression object containing an integer.
    :return: An integer.
    """
    return list_expr.value


def parse_real(list_expr: ListExp) -> float:
    """
    Parses a list expression for a real object.

    :param list_expr: A list expression object containing a real.
    :return: A float.
    """
    return list_expr.value


def parse_bool(list_expr: ListExp) -> bool:
    """
    Parses a list expression for a boolean object.

    :param list_expr: A list expression object containing a boolean.
    :return: A boolean.
    """
    return list_expr.value


def parse_string(list_expr: ListExp) -> str:
    """
    Parses a list expression for a string object.

    :param list_expr: A list expression object containing a string.
    :return: A string.
    """
    return list_expr.value


def parse_longint(list_expr: ListExp) -> int:
    """
    Parses a list expression for a long integer object.

    :param list_expr: A list expression object containing a long integer.
    :return: An integer.
    """
    return list_expr.value


def parse_rational(list_expr: ListExp) -> float:
    """
    Parses a list expression for a rational object.

    :param list_expr: A list expression object containing a rational.
    :return: A float.
    """
    return list_expr.value

