"""
The module secondo_int defines a class for representing int type of Secondo in python.
"""
from secondo_datatypes_pkg.general_type import *
from libs_pkg.exception_handler import *

class secondo_int(general_type):
    """
    The class secondo_int implements the boolean type of Secondo.
    """
    def __init__(self, value = None, defined = None):
        """
        The constructor of the class.
        
        :param value: Represents the value of the object and is optional.
        :param defined: Shows if the value of the object falls in defined range and is optional.
        :return: None.
        """
        if value is not None:
            if not isinstance(value, int):
                raise SecondoAPI_Error('The given parameter is not of type int.')
        if defined is not None:
            if not isinstance(defined, bool):
                raise SecondoAPI_Error('The given parameter is not of type bool.')
        super(secondo_int, self).__init__(value, defined)
        
    
    def get_type_name(self):
        """
        The getter method of the class secondo_int.
        
        :return: Returns the type name of the class secondo_int.
        """
        return 'int'
            
    def get_type_class(self):
        """
        The getter method of the class secondo_int.
        
        :return: Returns the type class of the class secondo_int.
        """
        return self.__class__.__name__
            
    def __str__(self):
        """
        This method of the class secondo_int returns the
        type and value of the object in a nested list representation by
        modifying the magic method __str__.
        
        :return: Returns a nested list with two elements for type and value of the object.
        """
        return "(int " + str(self.get_value()) + ")"
    
    def parse(self, item):
        """
        This method parses a string containing an integer value 
        and constructs an object of type secondo_int according to the
        value parsed from the string.
        
        :param text: A string containing an integer value.
        :return: Returns the object constructed from the parsed value.
        """
        value = int(item)
        result = secondo_int(value)
        return result
    
    def to_float(self):
        """
        This method returns the converted value of the object of type
        secondo_int to type float.
        
        :return: Returns the converted value of the object of type
        secondo_int to type float.
        """
        return float(self.get_value())
   
    