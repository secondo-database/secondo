"""
The module load_secondo_config implements the class to read the needed parametes for API from
a configuration file and return them.
"""
import configparser as cfg
import os

from libs_pkg.exception_handler import *

class Config():
    """
    This class implements the needed attributes and methods to read the parametes related to Secondo from
    a configuration file and return them.
    """
    
    def __init__(self,file=None,sec_srv=None,user=None,passw=None,sec_port=None,opt_srv=None,opt_port=None, binary_result=None):
        if file and not (sec_srv and user and passw and sec_port and opt_srv and opt_port and binary_result):
            self.file_name = file
            self.sec_srv = ""
            self.user = ""
            self.passw = ""
            self.sec_port = 0
            self.opt_srv = ""
            self.opt_port = 0
            self.binary_result = False
            self.cfg = cfg.ConfigParser(delimiters=('=', ':'), comment_prefixes=('#', ';'))
            self.get_from_file()
            
        elif not (file and sec_srv and user and passw and sec_port and opt_srv and opt_port and binary_result):
            print("A needed parameter is missing!")
        elif not file and sec_srv and user and passw and sec_port and opt_srv and opt_port and binary_result:
            
            if sec_srv == "":
                raise SecondoAPI_Error('SecondoServer address is not set.')
            self.sec_srv = sec_srv
            if user == "":
                raise SecondoAPI_Error('User name is not set.')
            self.user = user
            if passw == "":
                raise SecondoAPI_Error('Password is not set.')
            self.passw = passw
            if sec_port == "":
                raise SecondoAPI_Error('SecondoServer port is not set.')
            self.sec_port = sec_port
            if opt_srv == "":
                raise SecondoAPI_Error('OptimizerServer address is not set.')
            self.opt_srv = opt_srv
            if opt_port == "":
                raise SecondoAPI_Error('OptimizerServer port is not set.')
            self.opt_port = opt_port
            if binary_result == "":
                raise SecondoAPI_Error('Result format of queries is not set.')
            self.binary_result = binary_result
            
    def ConfigSectionMap(self, section):
        dict1 = {}
        options = self.cfg.options(section)
        for option in options:
            try:
                dict1[option] = self.cfg.get(section, option)
                if dict1[option] == -1:
                    DebugPrint("skip: %s" % option)
            except:
                print("exception on %s!" % option)
                dict1[option] = None
        return dict1

    def get_from_file(self):
        
        if os.path.isfile(os.getcwd() + '/' + self.file_name):
            self.cfg.read(os.getcwd() + '/' + self.file_name)
            self.sec_srv = self.ConfigSectionMap("General")['servername']
            if self.sec_srv == "":
                raise SecondoAPI_Error('SecondoServer address is not set.')
            self.sec_port = self.ConfigSectionMap("General")['serverport']
            if self.sec_port == "":
                raise SecondoAPI_Error('SecondoServer port is not set.')
            self.user = self.ConfigSectionMap("General")['user']
            if self.user == "":
                raise SecondoAPI_Error('User name is not set.')
            self.passw = self.ConfigSectionMap("General")['passwd']
            if self.passw == "":
                raise SecondoAPI_Error('Password is not set.')
            self.binary_result = True if self.ConfigSectionMap("General")['usebinarylists'] == 'BinaryTransfer' else False
            if self.binary_result == "":
                raise SecondoAPI_Error('Result format of queries is not set.')
            self.opt_srv = self.ConfigSectionMap("General")['optimizerhost']
            if self.opt_srv == "":
                raise SecondoAPI_Error('OptimizerServer address is not set.')
            self.opt_port = self.ConfigSectionMap("General")['optimizerport']
            if self.opt_port == "":
                raise SecondoAPI_Error('OptimizerServer port is not set.')
    
        else:
            
            raise SecondoAPI_Error('The Configuration File not found.')
               
        
    def initialize(self):
        
        """
        This mezhod returns the needed parametes for Secondo.
        
        :return: The IP-Address of Secondo sever.
        :return: The port number of Secondo sever.
        :return: The username for authentication in Secondo sever.
        :return: The password for authentication in Secondo sever.
        :return: The IP-Address of Optimizer sever.
        :return: The port number of Optimizer sever.
        :return: The format of the returned result list from Secondo.
        
        """
       
        return self.sec_srv, self.sec_port, self.user, self.passw, self.opt_srv, self.opt_port, self.binary_result 
