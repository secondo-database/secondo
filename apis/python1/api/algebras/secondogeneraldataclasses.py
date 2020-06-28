# ----------------------------------------------------------------------------------------------------------------------
# The Secondo Python API (pySecondo)
# Victor Silva (victor.silva@posteo.de)
# June 2020
# ----------------------------------------------------------------------------------------------------------------------
# Module
# ----------------------------------------------------------------------------------------------------------------------
# Secondo General Data Classes
# secondogeneraldataclasses.py
# ----------------------------------------------------------------------------------------------------------------------
"""
The module Secondo General Data Classes implements data classes for the storing and handling of general objects, such as
type, algebra or attribute specifications, which are use in the API parser module and in other algebra modules.
"""

from dataclasses import dataclass


@dataclass
class Algebra:
    """
    Implements a type for the storage of a single algebra specification of |sec|.
    """

    __slots__ = ["algebra_name", "type_list", "operator_list"]

    algebra_name: str
    type_list: []
    operator_list: []


@dataclass
class Type:
    """
    Implements a type for the storage of a single type specification of |sec|.
    """

    __slots__ = ["type_name", "properties"]

    type_name: str
    properties: {}


@dataclass
class Object:
    """
    Implements a type for the storage of a single object specification of |sec|.
    """

    __slots__ = ["object_name", "object_type", "attributes"]

    object_name: str
    object_type: str
    attributes: []


@dataclass
class Attribute:
    """
    Implements a type for the storage of a single attribute specification of a relation from |sec|.
    """

    __slots__ = ["attribute_type", "attribute_name"]

    attribute_type: str
    attribute_name: str


@dataclass
class Operator:
    """
    Implements a type for the storage of a single operator specification of |sec|.
    """

    __slots__ = ["operator_name", "properties"]

    operator_name: str
    properties: {}