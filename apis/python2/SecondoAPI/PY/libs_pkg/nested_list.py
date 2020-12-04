import ast
"""
The module nested_list contains functions for parsing and manipulating the nested lists.
"""
from pyparsing import *
import pprint
import re

"""
The class dbl_quot returns a double quoted string while the default in python is single quotation mark.
"""
class dbl_quot(str):
    def __repr__(self):
        return ''.join(('"', super().__repr__()[1:-1], '"'))
    

#old
#def change(seq, what, make):
#    for i, item in enumerate(seq):
#        if item == what:
#            seq[i] = make
#        elif type(item) == list:
#            change(item, what, make)
#    return seq

#new
def change_nl_item_val(seq, what, make):
    """
    The function change_nl_item_val updates the value of a specific item in a nested list with 
    unknown depth and structure to another given value.

    :param seq: An iterable data structure like nested list.
    :param what: The value that should be modified.
    :param make: The vlue that should replace the original value.
    :return: The modified iterable.
    """
    for i, item in enumerate(seq):
        
        if type(item) != list and item == what:
            seq[i] = make
        elif type(item) == list:
            change_nl_item_val(item, what, make)
    return seq



def change_nl_text_form(seq):
    """
    The function change_nl_text_form() modifies the value of items of type text in a nested list with form '****' to <text>****</text---> form.

    :param seq: An iterable data structure like nested list.
    :return: The modified iterable.
    """
    for i, item in enumerate(seq):
        if seq[i] and seq[i] != '<text>' and item[0] == "'" and item[1] == "(":
            seq[i] = "<text>" + item[1:-1] + "</text--->"
                
        elif seq[i] and seq[i] == '<text>':
            item = ["<text>",seq[i+1],"</text--->"] 
            seq[i] = item
            del seq[i+1], seq[i+1]
            
        elif seq[i] and type(item) == list:
            change_nl_text_form(item)
        
    return seq


#old
#def NL_items_type_Check(NL):
#    flattend_list = flatten_list(NL)
#    for item in flattend_list:
#        
#        if item.isdigit() or item in ('True', 'False'):
#            try:
#                if type(eval(item)) in(int, float, bool):
#                    change(NL, item, ast.literal_eval(item))
#            except (ValueError, NameError):
#                pass
#    return(NL)


#new
def change_nl_item_type(seq):
    """
    The function change_nl_item_type() modifies the type of string items in a nested list to proper types in python according to their values.

    :param seq: An iterable data structure like nested list.
    :return: The modified iterable.
    """
    
    for i, item in enumerate(seq):
        try:
            if type(item) != list and (item.isdigit() or item in ('True', 'False') or float(item)):
                
                if type(eval(item)) in(int, float, bool):
                    seq[i] = ast.literal_eval(item)
            elif type(item) == list:
                change_nl_item_type(item)
        except (ValueError, NameError):
            pass       
    return seq



def item_type(item):
    """
    The function item_type() detect the type of an item in a pythonic list and maps it to a secondo data type.

    :param item: An elemt in an iterable like a nested list.
    :return: The relevant data type in Secondo nested lists as a string.
    """
    
    if isinstance(item, str) and item[0] == '"':
        return "string"
    if isinstance(item, str) and item[0] != '"' and item[0] != '<':
        return "symbol"
    if isinstance(item, int):
        return "integer"
    if isinstance(item, float):
        return "real"
    if isinstance(item, bool):
        return "boolean"
    if (isinstance(item, list) and item[0] == "<text>") or (isinstance(item, str) and item.startswith('<text>')):
        return "text"
    if isinstance(item, list):
        return "list"
    


def nl_parse(content, ftype):  #ftype = True: file as argument, ftype = False: string as argument
    
    """
    The function nl_parse() parses a  Secondo NestedList to a Python Nestedlist.

    :param content: The nested list to be parsed.
    :param ftype: A boolean value which tells if the first parameter is a string containing the nested list(False) or a file name(True).
    :return: The python nested list.
    """
    #ident = Word(alphas, alphanums + "-_.")
    #number = Word('-'+'+'+nums+'.')
    #nestedParens = nestedExpr('(', ')', content=enclosed) 
    
    enclosed = Forward()
    nestedParens = nestedExpr('(', ')')
    enclosed << (OneOrMore(nestedParens))
    if ftype:
        try:
            with open(content, encoding="utf8", errors='ignore') as content_file:
        
                content = content_file.read()
                if not content.startswith('('):
                    content = '(' + content + ')'
                NLresult = enclosed.parseString(content).asList()
        
        except (ParseException, ParseFatalException) as err:
            NLresult = []
            print(err)
            print("Invalid syntax at line {}, column {}: '{}': {}.".format(err.lineno,err.column,err.markInputline(),err.msg))

    else:
        
        try:
            if not content.startswith('('):
                content = '(' + content + ')'
            NLresult = enclosed.parseString(content).asList()
        
        except (ParseException, ParseFatalException) as err:
            NLresult = []
            print(err)
            print("Invalid syntax at line {}, column {}: '{}': {}.".format(err.lineno,err.column,err.markInputline(),err.msg))
       
    NLresult = change_nl_text_form(NLresult[0])
    NLresult = change_nl_item_type(NLresult)
    return NLresult

    
def nl_to_String(NL):
    """
    The function nl_to_String() converts a pythonic nested list to a textual nestedlist with nested parenthesis that matches the nested list format in Secondo.

    :param NL: The nested list to be changed.
    :return: The Secondo nested list.
    """
    strresult = pprint.pformat(NL)
    replacements = {"[": "(", "]": ")", "'": ""}
    strresult = "".join([replacements.get(c, c) for c in strresult])
    return strresult

