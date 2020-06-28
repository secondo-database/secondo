 # ----------------------------------------------------------------------------------------------------------------------
# The Secondo Python API (pySecondo)
# Victor Silva (victor.silva@posteo.de)
# January 2020
# ----------------------------------------------------------------------------------------------------------------------
# Module
# ----------------------------------------------------------------------------------------------------------------------
# Secondo List Expression
# secondolistexpr.py
# ----------------------------------------------------------------------------------------------------------------------
"""
The module Secondo List Expression implements methods for the reception and construction of a list expression object
originated through the response of the |sec| server to an inquiry or a command. The implementation is based on the
Java class ListExpr of the Secondo JavaGUI.
"""

# Constants describing the atom types

NO_ATOM = 0
INT_ATOM = 1
REAL_ATOM = 2
BOOL_ATOM = 3
STRING_ATOM = 4
SYMBOL_ATOM = 5
TEXT_ATOM = 6


def append_to_last_element(last_element, new_node):
    """
    Appends a new element to a list expression object. The new element will be stored in the "value" variable of the
    last element passed and stores an empty list in his "next" variable.

    :param last_element: The list expression object to be extended.
    :param new_node: The list expression object to be appended.
    :return: The new list expression object with the new appended element.
    """

    list_expr = ListExp()
    list_expr.value = new_node
    list_expr.next = ListExp()
    last_element.next = list_expr
    return list_expr


def build_new_list_node(left_list, right_list):
    """
    Builds a new node for a list expression. A node is formed by a left and a right list. The right list, which will be
    appended in the "next"-parameter, must not be an atom.

    :param left_list: The left list expression object for the parameter "value".
    :param right_list:  The right list expression object for the parameter "next".
    :return: A list expression object.
    """

    list_expr = ListExp()
    list_expr.value = left_list
    list_expr.next = right_list

    return list_expr


def one_element_list(element_1):
    """
    Creates a one element list expression. The left node contains the element value, and the right node an empty list.

    :param element_1: A list expression object.
    :return: A list expression object with one element.
    """
    return build_new_list_node(element_1, ListExp())


def two_element_list(element_1, element_2):
    """
    Creates a two element list expression.

    :param element_1: A list expression object.
    :param element_2: A list expression object.
    :return: A list expression object with two elements.
    """
    return build_new_list_node(element_1,
                               build_new_list_node(element_2, ListExp()))


def three_element_list(element_1, element_2, element_3):
    """
    Creates a three element list expression.

    :param element_1: A list expression object.
    :param element_2: A list expression object.
    :param element_3: A list expression object.
    :return: A list expression object with three elements.
    """
    return build_new_list_node(element_1,
                               build_new_list_node(element_2,
                                                   build_new_list_node(element_3, ListExp())))


def four_element_list(element_1, element_2, element_3, element_4):
    """
    Creates a four element list expression.

    :param element_1: A list expression object.
    :param element_2: A list expression object.
    :param element_3: A list expression object.
    :param element_4: A list expression object.
    :return: A list expression object with four elements.
    """
    return build_new_list_node(element_1,
                               build_new_list_node(element_2,
                                                   build_new_list_node(element_3,
                                                                       build_new_list_node(element_4, ListExp()))))


def five_element_list(element_1, element_2, element_3, element_4, element_5):
    """
    Creates a five element list expression.

    :param element_1: A list expression object.
    :param element_2: A list expression object.
    :param element_3: A list expression object.
    :param element_4: A list expression object.
    :param element_5: A list expression object.
    :return: A list expression object with five elements.
    """
    return build_new_list_node(element_1,
                               build_new_list_node(element_2,
                                                   build_new_list_node(element_3,
                                                                       build_new_list_node(element_4,
                               build_new_list_node(element_5, ListExp())))))


def six_element_list(element_1, element_2, element_3, element_4, element_5, element_6):
    """
    Creates a six element list expression.

    :param element_1: A list expression object.
    :param element_2: A list expression object.
    :param element_3: A list expression object.
    :param element_4: A list expression object.
    :param element_5: A list expression object.
    :param element_6: A list expression object.
    :return: A list expression object with six elements.
    """
    return build_new_list_node(element_1,
                               build_new_list_node(element_2,
                               build_new_list_node(element_3,
                               build_new_list_node(element_4,
                               build_new_list_node(element_5,
                               build_new_list_node(element_6, ListExp()))))))


def check_if_atom(in_list):
    """
    Verifies if the ListExpr object is an atom. The method verifies the type of the list object.

    :param in_list: A ListExpr Object.
    :return: True, if the list element is not an atom, otherwise false.
    """

    if in_list.type != NO_ATOM:
        return True
    else:
        return False


def create_text_atom(value):
    """
    Creates a new text atom using a passed text as value.

    :param value: A text.
    :return: A ListExp object with the newly created text atom.
    """

    new_text_atom = ListExp()
    new_text_atom.type = TEXT_ATOM
    new_text_atom.value = str(value)

    return new_text_atom


def create_symbol_atom(value):
    """
    Creates a new symbol atom using its identifier as value.

    :param value: The identifier of the symbol (like DATABASE).
    :return: A ListExp object with the newly created symbol atom.
    """

    new_symbol_atom = ListExp()
    new_symbol_atom.type = SYMBOL_ATOM
    new_symbol_atom.value = str(value)

    return new_symbol_atom


def create_string_atom(value):
    """
    Creates a new string atom using a string as value.

    :param value: A string.
    :return: A ListExp object with the newly created string atom.
    """

    new_string_atom = ListExp()
    new_string_atom.type = STRING_ATOM
    new_string_atom.value = str(value)

    return new_string_atom


def create_integer_atom(value):
    """
    Creates a new integer atom using an integer as value.

    :param value: An integer.
    :return: A ListExp object with the newly created integer atom.
    """

    new_integer_atom = ListExp()
    new_integer_atom.type = INT_ATOM

    new_integer_atom.value = int(value)

    return new_integer_atom


def create_real_atom(value):
    """
    Creates a new real atom using an real as value.

    :param value: A real value.
    :return: A ListExp object with the newly created real atom.
    """

    new_real_atom = ListExp()
    new_real_atom.type = REAL_ATOM
    new_real_atom.value = float(value)

    return new_real_atom


def create_bool_atom(value):
    """
    Creates a new boolean atom using a boolean as value.

    :param value: A boolean.
    :return: A ListExp object with the newly created boolean atom.
    """

    new_boolean_atom = ListExp()
    new_boolean_atom.type = BOOL_ATOM
    new_boolean_atom.value = bool(value)

    return new_boolean_atom


# def write_list_to_string(list_expr, str_rep, identation):
#
#     separator = ' '
#     has_sub_lists = False
#     identation = identation + "    "
#
#     if not list_expr.is_atom():
#         str_rep += '('
#
#     while not list_expr.is_empty():
#
#         if list_expr.type == NO_ATOM:
#             if not has_sub_lists and (not list_expr.value.is_empty() and list_expr.value.type == NO_ATOM):
#                 has_sub_lists = True
#                 separator = "\n" + identation
#                 str_rep += separator
#             str_rep = write_list_to_string(list_expr.value, str_rep, identation)
#         elif list_expr.type == INT_ATOM:
#             str_rep += str(list_expr.value)
#             return str_rep
#         elif list_expr.type == REAL_ATOM:
#             str_rep += str(list_expr.value)
#             return str_rep
#         elif list_expr.type == BOOL_ATOM:
#             str_rep += str(list_expr.value)
#             return str_rep
#         elif list_expr.type == STRING_ATOM:
#             str_rep += str(list_expr.value)
#             return str_rep
#         elif list_expr.type == SYMBOL_ATOM:
#             str_rep += str(list_expr.value)
#             return str_rep
#         elif list_expr.type == TEXT_ATOM:
#             str_rep += str(list_expr.value)
#             return str_rep
#
#         list_expr = list_expr.next
#
#         if not list_expr.is_empty():
#             str_rep += separator
#
#     str_rep += ')'
#     return str_rep


def write_list_to_string(list_expr, str_list: [], identation) -> []:

    separator = ' '
    has_sub_lists = False
    identation = identation + "    "

    if not list_expr.is_atom():
        str_list.append('(')

    while not list_expr.is_empty():

        if list_expr.type == NO_ATOM:
            if not has_sub_lists and (not list_expr.value.is_empty() and list_expr.value.type == NO_ATOM):
                has_sub_lists = True
                separator = "\n" + identation
                str_list.append(separator)
            str_list = write_list_to_string(list_expr.value, str_list, identation)
        else:
            str_list.append(str(list_expr.value))
            return str_list

        list_expr = list_expr.next

        if not list_expr.is_empty():
            str_list.append(separator)

    str_list.append(')')
    return str_list


class ListExp:

    def __init__(self):
        """
        Constructor of the list expression for handling |sec| objects.
        """
        self.type = NO_ATOM
        self.value = None
        self.next = None

    def __str__(self):

        str_list = []
        identation = ''

        str_list = write_list_to_string(self, str_list, identation)
        string = "".join(str_list)

        return string

    def is_empty(self):

        if self.type == NO_ATOM and self.value is None and self.next is None:
            return True
        else:
            return False

    def is_atom(self):

        if self.type == NO_ATOM:
            return False
        else:
            return True

    def end_of_list(self):

        if self.type == NO_ATOM and not self.is_empty() and self.next.is_empty():
            return True
        else:
            return False

    def get_the_n_element(self, n):
        """
        Returns the n element of the list.

        :param n: The number of the n-element to be retrieved.
        :return: A list expression object representing the requested n-element.
        """

        i = 1
        tmp_le = self

        while i < n:
            tmp_le = tmp_le.next
            i += 1

        return tmp_le.value

    def get_first_element(self):
        """
        Returns the first element of the list.

        :return: A ListExp object with the first element of the current list.
        """
        return self.get_the_n_element(1)

    def get_second_element(self):
        """
        Returns the second element of the list.

        :return: A ListExp object with the second element of the current list.
        """
        return self.get_the_n_element(2)

    def get_third_element(self):
        """
        Returns the third element of the list.

        :return: A ListExp object with the third element of the current list.
        """
        return self.get_the_n_element(3)

    def get_fourth_element(self):
        """
        Returns the fourth element of the list.

        :return: A ListExp object with the fourth element of the current list.
        """
        return self.get_the_n_element(4)

    def get_fifth_element(self):
        """
        Returns the fifth element of the list.

        :return: A ListExp object with the fifth element of the current list.
        """
        return self.get_the_n_element(5)

    def get_sixth_element(self):
        """
        Returns the sixth element of the list.

        :return: A ListExp object with the sixth element of the current list.
        """
        return self.get_the_n_element(6)

    def get_list_length(self):
        """
        Determines the length of the current list.

        :return: An integer with the depth of the list.
        """

        i = 1
        tmp_le = self

        while tmp_le.next is not None:
            tmp_le = tmp_le.next
            i += 1

        return i

    def get_last_node(self):
        """
        Returns the last node of the list expression object.

        :return: The last node of the list expression object.
        """

        last_node = self

        if self.next is None:
            return last_node
        else:
            while last_node.next is not None:
                last_node = self.next
            return last_node

    def add(self, in_list):
        """
        Adds a new node to the list.

        :param in_list: A list expression object.
        :return: The current instance of the list expression object.
        """

        if self.get_list_length() == 1 and self.type == NO_ATOM:
            # The list has the original node only
            self.value = in_list.value
            self.type = in_list.value
            self.next = None
        else:
            # The list has more than one node
            last_node = self.get_last_node()
            last_node.next = in_list









