"""
The module Attribute defines the Attribute class, which represents the Attribute data type.
"""
from secondo_datatypes_pkg.general_type import *
from secondo_datatypes_pkg.secondo_int import *
from secondo_datatypes_pkg.secondo_real import *
from secondo_datatypes_pkg.secondo_bool import *
from secondo_datatypes_pkg.secondo_string import *
from secondo_datatypes_pkg.secondo_text import *
from libs_pkg.exception_handler import *

class Attribute():
    """
    The class Attribute implements the attribute data type in python.
    Each Tupel and relation header consist of one or more attributes.
    
    """
    #identifier: The attribute's identifier
    #typeName: The attribute's type as String
    #att_type: The attribute's type as Class
    #projected: If set to false, the attribute shouldnt be visible
    
    def __init__(self, identifier = None, typeName = None):
        """
        The constructor of the class Attribute.
        
        :param identifier: The attribute's identifier.
        :param typeName: The attribute's type as String.
        :return: None.
        """
        self.cls_name = {'bool':globals()["secondo_bool"],'int':globals()["secondo_int"],'float':globals()["secondo_real"],'string':globals()["secondo_string"],'text':globals()["secondo_text"]}
        if identifier is not None:
            self.identifier = identifier
        else:    
            self.identifier = ''
        if typeName is not None:
            self.type_name = typeName
        else:    
            self.type_name = ''    
        if typeName is not None:
            self.att_type = self.cls_name[typeName].__name__
        else:
            self.att_type = None
        self.projected = True

    def get_Identifier(self):
        """
        The getter method of the class Attribute.
        
        :return: returns the identifier attribute of the class Attribute.
        """
        return self.identifier
    
    
    def get_type_name(self):
        """
        The getter method of the class Attribute.
        
        :return: returns the type_name attribute of the class Attribute.
        """
        return self.type_name
       
    def get_type_class(self):
        """
        The getter method of the class Attribute.
        
        :return: returns the att_type attribute of the class Attribute.
        """
        return self.att_type

    def get_Projected(self):
        """
        The getter method of the class Attribute.
        
        :return: A boolean value that shows if the attribute is visible (True) or not (False).
        """
        return self.projected
    
    def out_to_list(self):
        """
        The method that returns the identifier and type name of the attribute in a list object.
        
        :return: A list in python that contains the identifier and type name of the attribute.
        """
        att_list = []
        att_list.append(self.identifier)
        att_list.append(self.type_name)
        return att_list
    
    def in_from_list(self, att_list):
        """
        The method that sets the identifier and type name of the attribute respectively from a given list.
        
        :param att_list: A list in python that contains the identifier as first element and type name as second element.
        """
        if len(att_list) < 2:
            raise SecondoAPI_Error('The list containing the attribute must have two elements: identifier and type.') 
        self.identifier = att_list[0]
        self.type_name = att_list[1]
        self.att_type = self.cls_name[att_list[1]].__name__

    def set_Projected(self, prj):
        """
        The setter method of the class Attribute.
        
        :param prj: A boolean value that shows if the attribute is visible (True) or not (False).
        """
        self.projected = prj


    def __eq__(self, obj):
        """
        The method that implements the equality operation for the Attribute data structure
        by modifying the magic method __eq__.
        
        :param obj: Another Attribute object.
        :return: True or False according to the result of comparison.
        """
        if obj is None:
            return False
       
        if not (self.identifier == obj.identifier and self.att_type == obj.att_type):
            return False
        return True
    
    
    def __hash__(self):
        """
        The method that defines a hash code for the Attribute object
        by modifying the magic method __hash__.
        
        :return: The calculated hash code.
        """
        prm = 31
        result = 1
        result = prm * result + (0 if self.identifier is None else hash(self.identifier)) 
        return result
