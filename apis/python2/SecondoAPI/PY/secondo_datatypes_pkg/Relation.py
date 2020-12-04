"""
The module Relation defines a class for representing Relation data structure in python.
"""
from secondo_datatypes_pkg.general_type import *
from secondo_datatypes_pkg.Tuple import *
from libs_pkg.exception_handler import *

class Relation():
    """
    The class Relation implements the Relation data structure of Secondo.
    """
    def __init__(self, header = None, tuples = None):
        """
        The constructor of the class.
        
        :param header: A list of one or more objects of type Attribute and is optional.
        :param tuples: A list of one or more objects of type Tuple and is optional.
        :return: None.
        """
        self.header = []
        if header is not None:
            self.header = header
        self.tuples = []
        if tuples is not None:
            self.tuples = tuples
        

    def create_tuple_from_list(self, NList):
        """
        The method create_tuple_from_list() creates a new object
        of type Tuple and updates the attrib_list of the Tuple object
        with the header attribute of the Realtion and its value_list
        with thw given list as parameter and adds the new Tuple object
        to the tuples attribute of the Relation.
        
        :param NList: A list to replace the value_list attribute of the Tuple object.
        :return: the new Tuple object.
        """
        
        new_tuple = Tuple()
        new_tuple.set_attrib_list = self.header
        new_tuple.set_value_list = NList
        self.tuples.append(new_tuple.get_value_list())
        return new_tuple

    def get_tuples(self):
        
        """
        The getter method for tuples attribute.
        
        :return: Returns a list of tuples.
        """
        return self.tuples

    def set_tuples(self, tuples):
        """
        The setter method for tuples attribute.
        
        :param tuples: A list of tuples containing the values of Attributesin header.
        """
        self.tuples = tuples


    def get_header(self):
        """
        The getter method for header attribute.
        
        :return: Returns a list of Attributes.
        """
        return self.header

    #usage a, b = get_att_index_type(identifier)
    def get_att_index_type(self, identifier):
        """
        This method returns the index and data type of an specific
        Attribute given by its header in the header list.
        
        :param identifier: The identifier of the searched Attribute.
        :retutn: Tthe index and data type of Attribute.
        """
        for idx, att in enumerate(self.header):
            if att.get_Identifier() == identifier:
                return idx, att.get_type_name()

    def get_rel_header(self):
        """
        The getter method for header attribute.
        
        :return: Returns a list of Attributes.
        """
        
        return self.header

    def in_from_list(self, rel_list):
        """
        This method updates the attributes header and tuples
        of the Relation object respectively from a given list.
        
        :param rel_list: A list of lists containing the values of header 
        and tuples of the Relation object.
        :return: None.
        """
        if len(rel_list) < 2:
            raise SecondoAPI_Error('The list must contain at least two elements for header and records of the relation.')
        self.header = rel_list[0]
        self.tuples = rel_list[1]
    
    
    def out_to_list(self):
        """
        This method saves the attributes header and tuples
        of the Relation object to a list.
        
        :return: A list of two lists.
        """
        rel_list = []
        rel_list.append(self.header)
        rel_list.append(self.tuples)
        return rel_list
    
    def __eq__(self, obj):
        """
        The method that implements the equality operation for the Relation 
        objects by modifying the magic method __eq__.
        
        :param obj: Another object of the same data type.
        :return: True or False according to the result of comparison.
        """
        if obj == self:
            return True
        if obj is None:
            return False
        if not isinstance(obj, Relation):
            return False
        other = Relation()
        othet = obj
        if not other.get_Header() == self.header:
            return False
        if not other.get_Tuples() == self.tuples:
            return False
        return True
