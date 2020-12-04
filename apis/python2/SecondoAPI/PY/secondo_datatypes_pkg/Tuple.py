
"""
The module Tuple defines a class for representing Tuple data structure in python.
"""
from secondo_datatypes_pkg.general_type import *
from secondo_datatypes_pkg.Attribute import *
from libs_pkg.exception_handler import *

class Tuple():
    """
    The class Tuple implements the Tuple data structure of Secondo.
    """
    
    def __init__(self, attrib_list = None, value_list = None):
        """
        The constructor of the class.
        
        :param attrib_list: A list containing Attributes of types seconndo_int, secondo_real, ... and is optional.
        :param value_list: A list of values for each attribute in self.attrib_list and is optional.
        :return: None.
        """
        self.attrib_list = []
        if attrib_list is not None:
            self.attrib_list = attrib_list
        self.value_list = []
        if value_list is not None:
            self.value_list = value_list
    
    def add_value(self, val):
        """
        The method add_value() adds a list of values to the attribute value_list .
        
        :param val: A list of values for each Attribute in attrib_list.
        :return: None.
        """
        self.value_list.append(val)
        
    def get_value(self, index):
        """
        The getter method for value_list attribute.
        
        :param index: The index of the desired value record in the value_list.
        :return: Returns a list of values given the index of the desired value record in the value_list.
        """
        return self.value_list[index]
    
    def get_value_list(self):
        """
        The getter method for value_list attribute.
        
        :return: Returns the value_list attribute of the object.
        """
        return self.value_list
    
    def set_value_list(self, ls):
        """
        The setter method for value_list attribute.
        
        :param ls: A list of value records for Attribute objects.
        :return: None.
        """
        self.value_list = ls
    
    def add_attrib(self, attrib):
        """
        The method add_attrib adds an Attribute object to the attrib_list attribute of the Tuple.
        
        :param attrib: An Attribute object.
        :return: None.
        """
        if not isinstance(attrib, Attribute):
             raise SecondoAPI_Error('The given parameter is not of type Attribute.')
        self.attrib_list.append(attrib)
        
    def get_attrib(self, index):
        """
        The getter method for attrib_list attribute.
        
        :param index: The index of the desired Attribute in the attrib_list.
        :return: Returns an Attribute object given the index of the desired Attribute in the attrib_list.
        """
        return self.attrib_list[index]
    
    def get_attrib_list(self):
        """
        The getter method for attrib_list attribute.
        
        :return: Returns the attrib_list attribute of the object.
        """
        return self.attrib_list
    
    #ls is a list of two elements the first is a list of attributes 
    #the secondo is a list containing the values of attributes
    def in_from_list(self, ls):
        """
        This method updates the attrib_list and value_list
        of the Tuple object respectively from a given list.
        
        :param ls: A list of two elements the first is a list of attributes 
        the secondo is a list containing the values of attributes.
        :return: None.
        """
        if len(ls) < 2:
            raise SecondoAPI_Error('The list containing the Tupel must have two elements: list of attributes and values.')
        self.attrib_list = ls[0]
        self.value_list = ls[1]
        
    def out_to_list(self):
        """
        This method saves the attrib_list and value_list
        of the Tuple object to a list.
        
        :return: A list of two lists.
        """
        tupel_list = []
        tupel_list.append(self.attrib_list)
        tupel_list.append(self.value_list)
        return tupel_list
             
    
    def set_attrib_list(self, ls):
        """
        The setter method for attrib_list attribute.
        
        :param ls: A list of Attribute objects.
        :return: None.
        """
        self.attrib_list = ls
    
    def __eq__(self, obj):
        """
        The method that implements the equality operation for the Tuple 
        objects by modifying the magic method __eq__.
        
        :param obj: Another object of the same data type.
        :return: True or False according to the result of comparison.
        """
        if obj == self:
            return True
        if obj is None:
            return False
        if not isinstance(obj, Tuple):
            return False
        otherTuple = Tuple()
        otherTuple = obj
        if not(len(self.value_list) == len(otherTuple.get_value_list())):
            return False
        for idx, att in enumerate(self.value_list):
            if att != otherTuple.get_value(idx):
                return False
        for idx, att in enumerate(self.attrib_list):
            if att != otherTuple.get_attrib(idx):
                return False
        
        return True

    # usage: tuple = Tuple.create_tuple_instance(attrib_list)
    @classmethod
    def create_tuple_instance(cls, attrib_list):
        """
        The class method that constructs a new Tuple instance given
        a list of Attribute objects attrib_list as parameter.
        
        :param attrib_list: A list of Attribute objects.
        :return: A new instance of the class Tuple.
        """
        return cls(attrib_list)
