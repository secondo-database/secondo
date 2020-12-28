
"""
The module PySecondo implements an API for interaction between Python and Secondo. 
After stablishing connections to Secondo server and Optimizer server reading the cofiguration parameters from a file, 
the queries canbe executed.
"""

import asyncio
import nest_asyncio
nest_asyncio.apply()
import time

from config_pkg.load_secondo_config import *
from optimizer_pkg.optimizer_server import *
from optimizer_pkg.optimize_query import *
from libs_pkg.nested_list import *
from libs_pkg.command_execution import *
from libs_pkg.exception_handler import *
from secondo_datatypes_pkg.secondo_int import *
from secondo_datatypes_pkg.secondo_real import *
from secondo_datatypes_pkg.Attribute import *
from secondo_datatypes_pkg.Tuple import *
from secondo_datatypes_pkg.Relation import *




class Secondo():
    """
    This class contains attributes and methods for connecting to a running Secondo server, 
    executing commands and queries, returning the result sets and parsing the results to implemented
    data structurs in python.
    
    """
    
    def __init__(self):
        
        """
        This constructor which reads from configuration file, connects to secondo server
        and initialises the object to connect to Optimizer server.
        
        """
        
        cfg = Config("config_pkg/secondo.cfg")
        params = cfg.initialize()
        self.server = params[0]
        self.port = params[1]
        self.user = params[2]
        self.bin_format = params[6]
        self.password = params[3]
        self.initialized = False
        self.conn = None
        self.reader = None
        self.writer = None
        self.opendb = ""
        #loop = asyncio.get_event_loop()
        #loop.run_until_complete(self.connect())
        asyncio.run(self.connect())
        self.opt = Optimizer(params[4], params[5])
        self.opt_reader, self.opt_writer = self.opt.get_opt_streams()
        self.result_error_code = None
        self.result_error_message = None
        self.result = []
        self.stream_result = []
        
    async def connect(self):
        """
        This method connects to Secondo server using asyncio methods and sets the connection, 
        stream- reader and writer attributes of the Secondo class. 
        
        """
        
        if self.initialized == True:
            raise SecondoAPI_Error('Secondo already initialised.')
            
        try:
            self.conn = asyncio.open_connection(self.server, int(self.port))
        except ConnectionRefusedError as e:
            raise SecondoAPI_Error(e.args[1] + ' - Connection to Secondo server refused.')
        except OSError as e:
            raise SecondoAPI_Error(e.args[1] + ' - Connection to Secondo couldnt be stablished.')
                                    
        try:
            self.reader, self.writer = await asyncio.wait_for(self.conn, timeout = 10)
        except ConnectionRefusedError as e:
            raise SecondoAPI_Error(e.args[1] + ' - Stream reader and stream writer for Secondo couldnt be initialised.')
        except OSError as e:
            raise SecondoAPI_Error(e.args[1] + ' - Stream reader and stream writer for Secondo couldnt be initialised.')
            
            
        message = "<Connect>\n" + self.user + "\n" + self.password + "\n" + "</Connect>\n"
        
        while True:
            line = await self.reader.readline()
            if not line:
                raise SecondoAPI_Error('An empty line was read from Secondo - Connection to Server failed.')
                break
            line = line.decode()
            if line != '<SecondoOk/>\n':
                #print(f'Received: {line!r}')
                break
            
            if line == '<SecondoOk/>\n':
                #print(f'Received: {line}')
                #print(message)
                self.writer.write(message.encode())
                await self.writer.drain()
    
                while True:
                    line = await self.reader.readline()
        
                    if not line:
                        raise SecondoAPI_Error('An empty line was read from Secondo - Connection to Server failed.')
                        break

                    line = line.decode()
                    if line == '<SecondoIntro>\n':
                        #print(f'Received: {line!r}')
                        while True:
                            line = await self.reader.readline()
        
                            if not line:
                                 raise SecondoAPI_Error('An empty line was read from Secondo - Connection to Server failed.')
                            line = line.decode()
                        
                            if line == '</SecondoIntro>\n':
                                #print(f'Received: {line!r}')
                                self.initialized = True
                                print("Connection To Secondo Server established...")
                                break
                    
                            elif line == '<SecondoError>\n':
                                raise SecondoAPI_Error('Error Tag received from Secondo- Connection to Server failed.')
                                #print(f'Received: {line!r}')
                            #else:
                                #print(f'Received: {line!r}')
                        break
                break    
        #print(self.initialized)
        
        
        
    async def optimization_check(self, command):
        """
        This method checks if the query need to be optimized then returns the optimized command 
        by calling the function opt_comm_exec() otherwise returns the unchanged query.

        :param command: The command or query to be executed.
        :return: The optimized query.
        """
        update_comm = False
        catalog_comm = False
        select_comm = True
        if command.startswith('sql') or command.startswith('sql\n'):
            update_comm = True
        elif "insert into " in command:
            update_comm = True
        elif "delete from " in command:
            update_comm = True
        elif "update " in command:
            update_comm = True
        elif "create table" in command:
            update_comm = True
            catalog_comm = True
            select_comm = False
        elif "drop" in command:
            update_comm = True
            catalog_comm = True
            select_comm = False
        elif "delete" in command:
            update_comm = True
            catalog_comm = True
            select_comm = False    
        elif "select" in command:
            update_comm = True
    
        if update_comm:
            if len(self.get_opendb()) == 0:
                raise SecondoError("No open Database.")
            
            if not self.opt.get_opt_conn():
                raise SecondoAPI_Error('Connection to OptimizerServer reset.')
            
            if catalog_comm:    
                opt_res = await opt_comm_exec(self.opt_reader, self.opt_writer, command, self.get_opendb(), True)
                if opt_res:
                    await self.reopen_db()
                    return None
                else:
                    raise SecondoError("Optimization failed.")
            elif select_comm:
                opt_res = await opt_comm_exec(self.opt_reader, self.opt_writer, command, self.get_opendb(), False)
                if opt_res:
                    return "query " + opt_res
                else:
                    raise SecondoError("Optimization failed.")
            
                
        else:
            return command
  

    
    async def reopen_db(self):
        
        """
        This method closes the currently open database and opens it again. 
        It is needed after executing some query types like catalog updates.

        :return: True/ False according to the success of operation.
        """
        
        if len(self.get_opendb()) == 0:
            return False
        db = self.get_opendb()
        
        if not self.initialized:
            raise SecondoAPI_Error('Connection to ScondoServer has not been initialised.')
        
        if self.conn is None:
            raise SecondoAPI_Error('Connection to ScondoServer reset.')
        res = await self.command_exec("close database")
        
        self.set_opendb('')
        
        if not self.initialized:
            raise SecondoAPI_Error('Connection to ScondoServer has not been initialised.')
        
        if self.conn is None:
            raise SecondoAPI_Error('Connection to ScondoServer reset.')
        res = await self.command_exec("open database " + db)
        
        self.set_opendb(db)
        return res[0] == 0
    
            
    async def command_exec(self, command, tupel_source = None):
        """
        This method executes the command/query in three categories (save, restore, general)
        by calling relevant functions, sets the attributes result_error_code, 
        result_error_message and result of the Secondo object.

        :param command: The command or query to be executed.
        :param tupel_source: the source for producing a stream of tupels, needed by queries like pyreceive
        which send a stream of tuples to Secondo server.
        :return: the result of command/ query as a python nested list.
        """
        
        self.result_error_code = None
        self.result_error_message = None
        self.result = []
        self.stream_result = []
        
        if 'open database ' in command.lower() and len(self.opendb) > 0:
            raise SecondoError('A database is already open, close the database before openning a new one.')
        if 'close database' in command.lower() and len(self.opendb) == 0:  
            raise SecondoError('No database is open.')
        
        if not self.initialized:
            raise SecondoAPI_Error('Connection to ScondoServer has not been initialised.')
        
        if self.conn is None:
            raise SecondoAPI_Error('Connection to ScondoServer reset.')
        
        if not self.opt.get_opt_initialized():
            raise SecondoAPI_Error('Connection to OptimizerServer has not been initialised.')
        
        if self.opt.get_opt_conn() is None:
            raise SecondoAPI_Error('Connection to OptimizerServer reset.')
            
        if self.get_sec_streams() is None:
            raise SecondoAPI_Error('Connection to Secondo Server reset!')
            
        command = await self.optimization_check(command)
        if command:
            #handliung restore-command
            if (((command.lstrip()).lower()).startswith('(restore') or ((command.lstrip()).lower()).startswith('( restore') or ((command.lstrip()).lower()).startswith('restore')):
            
                if not self.initialized:
                    raise SecondoAPI_Error('Connection to ScondoServer has not been initialised.')
        
                if self.conn is None:
                    raise SecondoAPI_Error('Connection to ScondoServer reset.')
            
                result = await restore_command(self.reader, self.writer, self.get_bin_format(), command)
                self.result_error_code = result[0]
                self.result_error_message = result[2]
                self.result = result[3]
                if result is None:
                    raise SecondoAPI_Error('Command execution returned an empty list.')
            
                if result[0] != 0 :
                    raise SecondoError(secondo_errors[result[0]] + ' Command execution was not successful.')
            
                if result[0] == 0 :
                    print('Restore was successful.')
                return result
    
            #handliung save-command
            if (((command.lstrip()).lower()).startswith('(save') or ((command.lstrip()).lower()).startswith('( save') or ((command.lstrip()).lower()).startswith('save')):
            
                if not self.initialized:
                    raise SecondoAPI_Error('Connection to ScondoServer has not been initialised.')
        
                if self.conn == None:
                    raise SecondoAPI_Error('Connection to ScondoServer reset.')
            
                result = await save_command(self.reader, self.writer, self.get_bin_format(), command)
                self.result_error_code = result[0]
                self.result_error_message = result[2]
                self.result = result[3]
                if result is None:
                    raise SecondoAPI_Error('Command execution returned an empty list.')
            
                if result[0] != 0 :
                    raise SecondoError(secondo_errors[result[0]] + ' Command execution was not successful.')
                if result[0] == 0 :
                    print('Save was successful.')
            
                return result
    
            #handling general-commands
        
            if not self.initialized:
                raise SecondoAPI_Error('Connection to ScondoServer has not been initialised.')
        
            if self.conn is None:
                raise SecondoAPI_Error('Connection to ScondoServer reset.')
        
            if 'pysend[' in command.lower():
                result, self.stream_result = await general_command(self.reader, self.writer, self.get_bin_format(), command)
            
            if 'pyreceive[' in command.lower():
                if tupel_source is None:
                    raise SecondoAPI_Error('No Tupel source has been given.')
                result = await general_command(self.reader, self.writer, self.get_bin_format(), command, stream_source = tupel_source)    
            
            if not ('pysend[' in command.lower() or 'pyreceive[' in command.lower()):
                result = await general_command(self.reader, self.writer, self.get_bin_format(), command)
        
            self.result_error_code = result[0]
            self.result_error_message = result[2]
            self.result = result[3]
        
            if result is None:
                raise SecondoAPI_Error('Command execution returned an empty list.') 
            
            if result[0] != 0 :
                print(self.get_result_error_message())
                raise SecondoError(secondo_errors[result[0]] + ' Command execution was not successful.')
    
            if result[0] == 0 :
                print('Command execution was successful.')
    
            if result[0] == 0 and "open database " in command:
                self.set_opendb(command[14:])
    
            if result[0] == 0 and "close database" in command.lower():
                self.opendb = ''
            return result
        
        else:
            return None
    
    
    def close(self):
        
        """
        This method closes the connections to Secondo server and consequently to Optimizer server.

        """
        if not self.initialized:
              raise SecondoAPI_Error('The connection to Secondo has not been initialised.')
        self.initialized = False
        self.opt.close()
        if self.writer is not None:
            self.writer.close()
        if self.conn is not None:
            self.conn.close()
    
    def get_sec_conn(self):
        """
        This method returns the connection attribute of Secondo object.

        :return: The connection attribute of Secondo object.
        """
        if self.conn is not None:
            return self.conn
        else:
            raise SecondoAPI_Error('No connection to SecondoServer.')
    
    def get_sec_streams(self):
        """
        This method returns the stream reader/ writer attributes of Secondo object.
        
        :return: The stream reader/ writer attributes of Secondo object.
        """
        if self.reader is not None and self.writer is not None:
            return self.reader, self.writer
        else:
            raise SecondoAPI_Error("Connection Error, no open stream reade/writer!")
    
    
    def get_server(self):
        """
        This method returns the server attributes of Secondo object.
        
        :return: The server attributes of Secondo object, which consists of an IP-address eg. 127.0.0.0.
        """
        return self.server
    
    def get_port(self):
        
        """
        This method returns the port attributes of Secondo object.
        :return: The port attributes of Secondo object, which consists of an integer number as port eg. 5678.
        """
        return self.port
    
    def get_user(self):
        """
        This method returns the user attributes of secondo object.
        :return: The user attributes of secondo object needed for authentication in Secondo.
        """
        return self.user
    
    def get_password(self):
        """
        This method returns the password attributes of secondo object.
        :return: The password attributes of secondo object, which is given as
        plain text and needed for authentication in Secondo.
        """
        return self.password
    
    def get_opendb(self):
        """
        This method returns the open database attribute of secondo object.
        :return: The currently open database in Secondo server as string.
        """
        return self.opendb
    
    def get_bin_format(self):
        """
        This method returns the format of returned result of queries.
        :return: True if the result is a nested list in binary formet/ False if in plain text nested list.
        """
        return self.bin_format
    
    
    def get_opt(self):
        """
        This method returns the opt attribute of secondo object.
        :return: The opt attribute of secondo represents an optimizer object.
        """
        return self.opt
    
    def fetch_result_type(self):
        """
        This method returns the result_type attribute of secondo object.
        :return: The result_type attribute of secondo represents the datatype
        of result set e.g. int or nested list.
        """
        if self.result:
            return self.result[0]
        return self.result
    
    def fetch_result_rows(self):
        """
        This method returns the result attribute of secondo object.
        :return: The result attribute of secondo represents the result as a list.
        """
        if self.result:
            if isinstance(self.result[1], list):
                return self.result[1]
            else:
                res = [self.result[1]]
                return res
        return self.result
    
    def fetch_relation_header(self):
        """
        This method returns the header of the relation in result set of secondo object.
        :return: The header of the relation in result set as a nested list.
        """
        return self.fetch_result_type()[1][1]
     
    def parse_result_to_secondo_int(self):
        """
        This method parses the integer result of a query and converts it to a secondo_int type.
        :return: An object of type secondo_int.
        
        """
        int_result = secondo_int()
        int_result.in_from_list(self.fetch_result_rows())
        print('int object: ')
        print(int_result.get_value())
        return int_result

    def parse_result_to_secondo_real(self):
        
        """
        This method parses the float result of a query and converts it to a secondo_real type.
        :return: An object of type secondo_real.
        
        """
        real_result = secondo_real()
        real_result.in_from_list(self.fetch_result_rows())
        print('real object: ')
        print(real_result.get_value())
        return real_result

    def parse_result_to_relation(self):
        """
        This method parses the relation result of a query and converts it to a Relation type.
        :return: An object of type Relation.
        
        """
        header = []
        for attrib in self.fetch_relation_header():
            att = Attribute()
            att.in_from_list(attrib)
            header.append(att)
        rel = []
        rel.append(header)
        rel.append(self.fetch_result_rows())
        relation = Relation()
        relation.in_from_list(rel)
        print('header: ')   
        for item in relation.get_header():
            print (item.get_Identifier() + '-' + item.get_type_name() + '-' +  item.get_type_class())
        print(relation.get_tuples())
        return relation

    def fetch_result(self):
        """
        This method returns the result attribute of the Secondo object.
        :return: The result attribute of the Secondo object as list.
        
        """
        return self.result
    
    def fetch_stream_result(self):
        """
        This method returns the attribute containing the stream of tupels received from
        Secondo server when the query contains the operator pysend.
        :return: The stream of tuples collected in a list of tuples after receiving.
        
        """
        return self.stream_result
    
    
    def parse_stream_result_to_relation(self):
        """
        This method parses the stream of tuples received from Secondo server
        when the query contains the operator pysend and converts them to a relation.
        :return: A Relation containing the stream of tuples received from Secondo server.
        
        """
        header = []
        for attrib in self.stream_result[0]:
            att = Attribute()
            att.in_from_list(attrib)
            header.append(att)
        rel = []
        rel.append(header)
        records = []
        for i in range(1, len(self.stream_result)):
            records.append(self.stream_result[i])
        rel.append(records)
        relation = Relation()
        relation.in_from_list(rel)
        print('header: ')   
        for item in relation.get_header():
            print (item.get_Identifier() + '-' + item.get_type_name() + '-' +  item.get_type_class())
        print(relation.get_tuples())
        return relation
    
    
    def get_result_error_code(self):
        """
        This method returns the result_error_code attribute of the Secondo object.
        :return: An integer representing the error code after executing a command/ query.
        
        """
        return self.result_error_code
    
    def get_result_error_message(self):
        """
        This method returns the result_error_message attribute of the Secondo object.
        :return: A string representing the error message after executing a command/ query.
        
        """
        return self.result_error_message
    
    def set_opendb(self, db):
        """
        This method returns the opendb attribute of the Secondo object.
        :return: A string representing the currently open database.
        
        """
        self.opendb = db
    
    def set_bin_format(self, val):
        """
        This method returns the bin_format attribute of the Secondo object.
        :return: A Boolean representing the format of received result from Secondo,
        when True corresponds to Binary nested list, when False textual nested list.
        
        """
        self.bin_format = val
        
