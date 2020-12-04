"""
The module secondo_string defines a class for representing string type of Secondo in python.
"""
from secondo_datatypes_pkg.general_type import *

class secondo_string(general_type):
    """
    The class secondo_string implements the string type of Secondo.
    """
    
    def __init__(self, value = None, defined = None):
        """
        The constructor of the class.
        
        :param value: Represents the value of the object and is optional.
        :param defined: Shows if the value of the object falls in defined range and is optional.
        :return: None.
        """
        if value is not None:
            assert type(value) is str, 'The given value must be of Type string.'
        super(secondo_string, self).__init__(value, defined)
    
    def get_type_name(self):
        """
        The getter method of the class secondo_string.
        
        :return: Returns the type name of the class secondo_string.
        """
        return 'string'
            
    def get_type_class(self):
        """
        The getter method of the class secondo_string.
        
        :return: Returns the type class of the class secondo_string.
        """
        return self.__class__.__name__
            
    def __str__(self):
        """
        This method of the class secondo_string returns the
        type and value of the object in a nested list representation by
        modifying the magic method __str__.
        
        :return: Returns a nested list with two elements for type and value of the object.
        """
        if self.get_value() == None:
            return "(string undefined)"
        return "(String " + str(self.get_value()) + ")"
    
    def parse(self, text):
        """
        This method parses a string and modifies the value attribute of 
        the object of type secondo_string according to the value parsed from the string.
        
        :param text: A string containing the value.
        :return: Returns the modified object according to the parsed value.
        """
        if text is None:
            return None
        else:
            self.set_value(text)
            return self
    
