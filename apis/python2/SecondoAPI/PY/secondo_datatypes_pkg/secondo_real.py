"""
The module secondo_real defines a class for representing real type of Secondo in python.
"""

from secondo_datatypes_pkg.general_type import *

class secondo_real(general_type):
    """
    The class secondo_real implements the real type of Secondo.
    """
    
    def __init__(self, value = None, defined = None):
        """
        The constructor of the class.
        
        :param value: Represents the value of the object and is optional.
        :param defined: Shows if the value of the object falls in defined range and is optional.
        :return: None.
        """
        if value is not None:
            assert type(value) is float, 'The given value must be of Type float.'
        super(secondo_real, self).__init__(value, defined)
    
    def get_type_name(self):
        """
        The getter method of the class secondo_real.
        
        :return: Returns the type name of the class secondo_real.
        """
        return 'float'
            
    def get_type_class(self):
        """
        The getter method of the class secondo_real.
        
        :return: Returns the type class of the class secondo_real.
        """
        return self.__class__.__name__
    
    
    def __str__(self):
        """
        This class method of the class secondo_real returns the
        type and value of the object in a nested list representation by
        modifying the magic method __str__.
        
        :return: Returns a nested list with two elements for type and value of the object.
        """
        return "(Real " + str(self.get_value()) + ")"
    
    def parse(self, text):
        """
        This method parses a string containing an float value 
        and constructs an object of type secondo_real according to the
        value parsed from the string.
        
        :param text: A string containing a float value.
        :return: Returns the object constructed from the parsed value.
        """
        value = float(text)
        result = secondo_real(value)
        return result
    
    def to_float(self):
        """
        This method returns the converted value of the object of type
        secondo_int to type float.
        
        :return: Returns the converted value of the object of type
        secondo_int to type float.
        """
        return float(self.get_value())
   
