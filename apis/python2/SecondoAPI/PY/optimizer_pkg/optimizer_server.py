"""
The module optimizer_server implements the connection to Optimizer server. 

"""

import asyncio
import nest_asyncio
nest_asyncio.apply()

class Optimizer():
    """
    This class contains attributes and methods for connecting to a running Optimizer server, 
    optimizing commands and queries, returning the result queries to be executed by Secondo.
    
    """
    
    def __init__(self, server, port):
        """
        The constructor of the calss Optimizer.
        
        :param server: The IP-address of Optimizer server.
        :param port: The port number of the socket.
        :return: None.
    
        """
         
        self.server = server
        self.port = port
        self.initialized = False
        self.conn = None
        self.optreader = None
        self.optwriter = None
        loop = asyncio.get_event_loop()
        loop.run_until_complete(self.connect())
        
    
    async def connect(self):
        """
        This method connects to Optimizer server using asyncio methods and sets the connection, 
        stream- reader and writer attributes of the Optimizer class. 
        
        """
        
        if self.initialized == True:
            raise SecondoAPI_Error('Optimizer already initialised.')
            
        elif self.initialized == False:
            
            try:
                self.conn = asyncio.open_connection(self.server, int(self.port))
            except ConnectionRefusedError as e:
                raise SecondoAPI_Error(e.args[1] + ' - Connection to Optimizer server refused.')
            except OSError as e:
                raise SecondoAPI_Error(e.args[1] + ' - Connection to Optimizer couldnt be stablished.')
            
            
            try:
                self.optreader, self.optwriter = await asyncio.wait_for(self.conn, timeout = 10)
            except ConnectionRefusedError as e:
                raise SecondoAPI_Error(e.args[1] + ' - Connection to Optimizer server refused.')
            except OSError as e:
                raise SecondoAPI_Error(e.args[1] + ' - Connection to Optimizer couldnt be stablished.')    
            
            
            message = "<who>\n"
            while True:
                self.optwriter.write(message.encode("utf-8"))
                await self.optwriter.drain()
        
                
                line = await self.optreader.readline()
                
                line = line.decode("utf-8")
           
                if not line:
                    break
            
                if line != "<optimizer>\n":
                    break
            
                if line == "<optimizer>\n":
                    self.initialized = True
                    print("Connection to Optimizer stablished...")
                    break
                
     
    def close(self):
        """
        This method closes the connections to Optimizer server.

        """
        if not self.initialized:
              raise SecondoAPI_Error('The connection to Optimizer has not been initialised.')
        self.initialized = False
        if self.optwriter is not None:
            self.optwriter.close()
        if self.conn is not None:
            self.conn.close()
    
    
    
    def get_opt_conn(self):
        """
        This method returns the connection attribute of Optimizer object.

        :return: The connection attribute of Optimizer object.
        """
        if self.conn is not None:
            return self.conn
        else:
            raise SecondoAPI_Error('The connection to Optimizer is reset.')
        
        
    def get_opt_initialized(self):
        if self.initialized:
            return True
        else:
            raise SecondoAPI_Error('Optimizer is already initialised.')
        
            
    def get_opt_streams(self):
        """
        This method returns the stream reader/ writer attributes of Optimizer object.
        
        :return: The stream reader/ writer attributes of Optimizer object.
        """
        if self.optreader is not None and self.optwriter is not None:
            return self.optreader, self.optwriter
        else:
            raise SecondoAPI_Error('Connection Error, Streams not connected.')
            #return None
    
    def get_server(self):
        """
        This method returns the server attributes of Optimizer object.
        
        :return: The server attributes of Optimizer object, which consists of an IP-address eg. 127.0.0.0.
        """
        return self.server
    
    
    def get_port(self):
        
        """
        This method returns the port attributes of Optimizer object.
        :return: The port attributes of Optimizer object, which consists of an integer number as port eg. 5678.
        """
        return self.port
    
    
    def set_opt_initialized(self, val):
        """
        This method sets an attribute of Optimizer object that shows if the Optimizer is initialised and running.
        :param initialized: A boolean value, when True means the Optimizer is initialised and running.
        """
        self.initialized = val
        
    