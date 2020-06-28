# ----------------------------------------------------------------------------------------------------------------------
# The Secondo Python API (pySecondo)
# Victor Silva (victor.silva@posteo.de)
# March 2020
# ----------------------------------------------------------------------------------------------------------------------
# Module
# ----------------------------------------------------------------------------------------------------------------------
# Secondo Spatial Algebra
# secondospatialalgebra.py
# ----------------------------------------------------------------------------------------------------------------------
"""
The module Secondo Relational Algebra implements the data types for the conversion of list expression objects with
values of the types contained in the RelationalAlgebra of the |sec| system. The data types are implemented in Python
using Data Classes. Data Classes are implemented in the API like normal classes without behaviour.
Like regular classes their attributes can be called through the given names.
"""


from dataclasses import dataclass

import secondodb.api.algebras.secondospatialalgebra as spatial
import secondodb.api.algebras.secondostandardalgebra as standard
import secondodb.api.algebras.secondospatiotemporalalgebra as spatiotemporal
from secondodb.api.algebras.secondogeneraldataclasses import Attribute
import secondodb.api.secondoapi as api
from secondodb.api.support.secondolistexpr import ListExp


@dataclass
class Relation:
    """
    Implements the type relation (rel) of the relational algebra of |sec|.
    """

    __slots__ = ["attributes", "data"]

    attributes: []
    data: []


def parse_relation(list_expr: ListExp, attr_list: []) -> Relation:
    """
    Parses a relation (rel) object. A relation named tuple with two attributes (attributes and data) will be returned.
    The attribute "attributes" contains the fields and types of the relation. The attribute "data" contains the entries.

    :param list_expr: A list expression object with a relation.
    :param attr_list: The list of the attributes. Every attribute is an object of the class Attribute.
    :return: An object of the class Relation with the data. Every entry of the relation data is a Python dictionary,
             which allows to call the single fields of the tuple using the corresponding attribute name. The retrieval
             of the value using an index (like in lists) is allowed as well.
    """

    relation_data = []

    while list_expr.next is not None:

        single_row = {}
        single_row_as_le = list_expr.value

        for j in range(0, len(attr_list)):

            single_attribute: Attribute = attr_list[j]

            attr_name = single_attribute.attribute_name

            if single_attribute.attribute_type == 'string':
                attr_value = standard.parse_string(single_row_as_le.get_the_n_element(j + 1))

            elif single_attribute.attribute_type == 'text':
                attr_value = standard.parse_string(single_row_as_le.get_the_n_element(j + 1))

            elif single_attribute.attribute_type == 'int':
                attr_value = standard.parse_int(single_row_as_le.get_the_n_element(j + 1))

            elif single_attribute.attribute_type == 'longint':
                attr_value = standard.parse_longint(single_row_as_le.get_the_n_element(j + 1))

            elif single_attribute.attribute_type == 'real':
                attr_value = standard.parse_real(single_row_as_le.get_the_n_element(j + 1))

            elif single_attribute.attribute_type == 'rational':
                attr_value = standard.parse_rational(single_row_as_le.get_the_n_element(j + 1))

            elif single_attribute.attribute_type == 'bool':
                attr_value = standard.parse_bool(single_row_as_le.get_the_n_element(j + 1))

            elif single_attribute.attribute_type == 'point':
                attr_value = spatial.parse_point(single_row_as_le.get_the_n_element(j + 1))

            elif single_attribute.attribute_type == 'points':
                attr_value = spatial.parse_points(single_row_as_le.get_the_n_element(j + 1))

            elif single_attribute.attribute_type == 'line':
                attr_value = spatial.parse_line(single_row_as_le.get_the_n_element(j + 1))

            elif single_attribute.attribute_type == 'sline':
                attr_value = spatial.parse_line(single_row_as_le.get_the_n_element(j + 1))

            elif single_attribute.attribute_type == 'region':
                attr_value = spatial.parse_region(single_row_as_le.get_the_n_element(j + 1))

            elif single_attribute.attribute_type == 'mpoint':
                attr_value = spatiotemporal.parse_mpoint(single_row_as_le.get_the_n_element(j + 1))

            elif single_attribute.attribute_type == 'mregion':
                attr_value = spatiotemporal.parse_mregion(single_row_as_le.get_the_n_element(j + 1))

            else:
                raise api.InterfaceError('Error: Attribute type ' + single_attribute.attribute_type + ' not supported.')

            single_row.update({attr_name: attr_value})

        relation_data.append(single_row)
        list_expr = list_expr.next

    return Relation(attr_list, relation_data)

