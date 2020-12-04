"""
The module secondo_bool defines a class for representing boolean type of Secondo in python.
"""
from secondo_datatypes_pkg.general_type import *

class secondo_bool(general_type):
    """
    The class secondo_bool implements the boolean type of Secondo.
    """
    
    def __init__(self, value = None, defined = None):
        """
        The constructor of the class.
        
        :param value: Represents the value of the object and is optional.
        :param defined: Shows if the value of the object falls in defined range and is optional.
        :return: None.
        """
        if value is not None:
            assert type(value) is bool, 'The given value must be of Type bool.'
        super(secondo_bool, self).__init__(value, defined)
        
    def get_type_name(self):
        """
        The getter method of the class secondo_bool.
        
        :return: Returns the type name of the class secondo_bool.
        """
        return 'bool'
            
    def get_type_class(self):
        """
        The getter method of the class secondo_bool.
        
        :return: Returns the type class of the class secondo_bool.
        """
        return self.__class__.__name__
            
    def __str__(self):
        """
        This method of the class secondo_bool returns the
        type and value of the object in a nested list representation by
        modifying the magic method __str__.
        
        :return: Returns a nested list with two elements for type and value of the object.
        """
        return "(bool " + str(self.value).upper() + ")"
    
    def parse(self, text):
        """
        This method parses a string and constructs an object
        of type secondo_bool according to the value parsed from the string.
        
        :param text: A string containing the value True or False.
        :return: Returns the object constructed from the parsed value.
        """
        if "true" == text.lower():
            result = secondo_bool(True)
        elif "false" == text.lower():
            result = secondo_bool(False)
        return result
