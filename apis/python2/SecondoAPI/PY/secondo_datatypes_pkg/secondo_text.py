"""
The module secondo_text defines a class for representing text type of Secondo in python.
"""
from secondo_datatypes_pkg.general_type import *

class secondo_text(general_type):
    """
    The class secondo_text implements the text type of Secondo.
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
        super(secondo_text, self).__init__(value, defined)
        self.PREFIX = "<text>"
        self.SUFFIX = "</text--->"
    
    def get_type_name(self):
        """
        The getter method of the class secondo_text.
        
        :return: Returns the type name of the class secondo_text.
        """
        return 'text'
            
    def get_type_class(self):
        """
        The getter method of the class secondo_text.
        
        :return: Returns the type class of the class secondo_text.
        """
        return self.__class__.__name__
    
    def in_from_list(self, NList):
        """
        The method that sets the value and defined attributes respectively from a given list.
        
        :param NList: A list in python that contains the value as first element and defined as second element.
        :return: None.
        """
        assert len(NList) >= 1, 'The list must contain at least one element!'
        value = NList[0] if not NList[0].startswith(self.PREFIX) else NList[0][len(self.PREFIX):]
        value = value if not value.endswith(self.SUFFIX) else value[:-len(self.SUFFIX)]
        self.set_value(value)
        if len(NList) >= 2:
            self.set_defined(NList[1])
        
    def out_to_list(self):
        """
        The method that returns the value and defined attributes in a list object.
        
        :return: A list in python that contains the value and defined attributes.
        """
        value = self.PREFIX + self.get_value() + self.SUFFIX
        NList = []
        NList.append(value)
        NList.append(self.defined)
        return NList
        
    def __str__(self):
        """
        This method of the class secondo_text returns the
        type and value of the object in a nested list representation by
        modifying the magic method __str__.
        
        :return: Returns a nested list with two elements for type and value of the object.
        """
        if self.get_value() == None:
            return "(text undefined)"
        return "(text " + str(self.get_value()) + ")"
    
    def parse(self, text):
        """
        This method parses a string and modifies the value attribute of 
        the object of type secondo_text according to the value parsed from the string.
        
        :param text: A string containing the value.
        :return: Returns the modified object according to the parsed value.
        """
        if text is not None:
            self.set_value(text)