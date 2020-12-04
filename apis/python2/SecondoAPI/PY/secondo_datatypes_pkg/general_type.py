"""
The module general_type defines a class which is a base for other data structure classes.
"""


class general_type():
    """
    The class general_type is the base class for other data structures like
    secondo_int and secondo_string and summarizes the shared attributes and methods
    between the data structures.
    """
    
    def __init__(self, value = None, defined = None):
        """
        The constructor of the class.
        
        :param value: Substitutes the value of the object and is optional.
        :param defined: A boolean value showing the value of the object is defined or not and is optional.
        :return: None.
        """
        
        if value is not None:
            self.value = value
        else:
            self.value = None
            
        if defined is None or defined == True:
            self.defined = True
        else:
            self.defined = defined
        
    
    def in_from_list(self, NList):
        """
        The method that sets the value and defined attributes respectively from a given list.
        
        :param att_list: A list in python that contains the value as first element and defined as second element.
        :return: None.
        """
        assert len(NList) >= 1, 'The list must contain at least one element!'
        self.set_value(NList[0])
        if len(NList) >= 2:
            self.set_defined(NList[1])
        
    
    def out_to_list(self):
        """
        The method that returns the value and defined attributes in a list object.
        
        :return: A list in python that contains the value and defined attributes.
        """
        NList = []
        NList.append(self.get_value())
        NList.append(self.get_defined())
        return NList
    
    
    def get_type_name(self):
        pass
        
        
    def get_type_class(self, TypeName):
        pass
        
   
    def set_value(self, value):
        """
        The setter method for value attribute.
        
        :param value: The value to replace the old attribute's value.
        """
        self.value = value
    
    def get_value(self):
        """
        The getter method for value attribute.
        
        :return: Returns the value attribute of the object.
        """
        return self.value
        
    
    def set_defined(self, value):
        """
        The setter method for defined attribute.
        
        :param value: The value to replace the old attribute's value.
        """
        self.defined = value
    
    def get_defined(self):
        """
        The getter method for defined attribute.
        
        :return: Returns the defined attribute of the object.
        """
        return self.defined
    
    def __eq__(self, other):
        """
        The method that implements the equality operation for the data structure 
        object by modifying the magic method __eq__.
        
        :param other: Another object of the same data type.
        :return: True or False according to the result of comparison.
        """
        if other.get_value() is None or not other.get_defined() or self.get_value() is None or not self.get_defined():
            return False
        assert other.__class__ == self.__class__, "Type Error!"
        
        if (self.get_value() == other.get_value()):
            return True
        return False
    
    def __hash__(self):
        """
        The method that defines a hash code for the object
        by modifying the magic method __hash__.
        
        :return: The calculated hash code.
        """
        return hash(self.get_value())
    
    def __gt__(self, other):
        """
        The method that implements the > comparison for the data structure 
        object by modifying the magic method __gt__.
        
        :param other: Another object of the same data type.
        :return: True or False according to the result of comparison.
        """
        if (other.get_value() is None) or (not other.get_defined()) or (self.get_value() is None) or (not self.get_defined()):
            return False
        assert other.__class__ == self.__class__, "Type Error!"
        return other.get_value() < self.get_value()
    
    def __ge__(self, other):
        """
        The method that implements the >= comparison for the 
        data structure object by modifying the magic method __ge__.
        
        :param other: Another object of the same data type.
        :return: True or False according to the result of comparison.
        """
        if (other.get_value() is None) or (not other.get_defined()) or (self.get_value() is None) or (not self.get_defined()):
            return False
        assert other.__class__ == self.__class__, "Type Error!"
        return not self.get_value() < other.get_value()
       
    def __lt__(self, other):
        """
        The method that implements the < comparison for the data structure 
        object by modifying the magic method __lt__.
        
        :param other: Another object of the same data type.
        :return: True or False according to the result of comparison.
        """
        if (other.get_value() is None) or (not other.get_defined()) or (self.get_value() is None) or (not self.get_defined()):
            return False
        assert other.__class__ == self.__class__, "Type Error!"
        return other.get_value() > self.get_value()
        
    def __le__(self, other):
        """
        The method that implements the <= comparison for the data structure 
        object by modifying the magic method __le__.
        
        :param other: Another object of the same data type.
        :return: True or False according to the result of comparison.
        """
        if (other.get_value() is None) or (not other.get_defined()) or (self.get_value() is None) or (not self.get_defined()):
            return False
        assert other.__class__ == self.__class__, "Type Error!"
        return not other.get_value() < self.get_value()
    
    def parse(self, text):
        pass
    
    def __add__(self,other):
        """
        The method that implements the + operation for the data structure 
        object by modifying the magic method __add__.
        
        :param other: Another object of the same data type.
        :return: The result of addition.
        """
        if (other.get_value() is None) or (not other.get_defined()) or (self.get_value() is None) or (not self.get_defined()):
            return self()
        assert other.__class__ == self.__class__, "Type Error!"
        result = self
        result.set_defined(True)
        result.set_value(self.get_value() + other.get_value())
        return result

