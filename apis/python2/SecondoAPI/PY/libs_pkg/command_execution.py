"""
The module command_execution contains three functions to execute three categories of commands and
return the results by calling other functions in other modules that read and parse the results to
proper data structures in python.

"""

import os
from io import *
import asyncio
import nest_asyncio
nest_asyncio.apply()
from pathlib import Path

from libs_pkg.secondo_results import *
from libs_pkg.nested_list import *
from libs_pkg.exception_handler import *
  
async def restore_command(reader, writer, bin_list, command):
    """
    This function executes the commands containing restore keyword and returns the result message.

    :param reader: The stream reader of the Secondo object.
    :param writer: The stream writer of the Secondo object.
    :param bin_list: A boolean value for result format of the result list returned from Secondo server.
    :param command: The command to be executed.
    :return: The result received from Secondo by calling the function receive_secondo_response().
    """
    
    if reader and writer:
        if not command.startswith('(') :
            command = '(' + command.strip() + ')'
       
        nl_cmd = nl_parse(command, False)
        length = len(nl_cmd)
        if length != 4 and length != 5: 
            return None
        if length == 5: #restore database
            if nl_cmd[0] != 'restore' or nl_cmd[1] != 'database' or nl_cmd[3] != 'from':
                return None
            name = nl_cmd[2]
            filename = str(Path.home()) + '/secondo/bin/' + nl_cmd[4]
            database = True
        if length == 4:
            if nl_cmd[0] != 'restore' or nl_cmd[2] != 'from':
                return None
            name = nl_cmd[1]
            filename = str(Path.home()) + '/secondo/bin/' + nl_cmd[3]
            database = False
        
        typ = item_type(filename)
        if not typ in ("string", "symbol", "text"):  
            return None
        
        tag = "DbRestore" if database else "ObjectRestore"
        message = "<" + tag + ">\n"
        message = message.encode()
        writer.write(message)
        
        message = name + "\n"
        message = message.encode()
        writer.write(message)
              
        message = "<FileData>\n"
        message = message.encode()
        writer.write(message)
        
        with open(filename, "rb") as f:    
            byte = f.read(1)
            size = 0
            while byte:
                size += 1
                byte = f.read(1)
                
            message = str(size) + "\n"
            message = message.encode()
            writer.write(message)
            await writer.drain()

        stream = open(filename, 'rb')
        message = stream.read()
        stream.close()
        writer.write(message)
       
        message = "</FileData>\n"
        message = message.encode()
        writer.write(message)
        
        message = "</" + tag + ">\n"
        message = message.encode()
        writer.write(message)
        await writer.drain()

        try:
            resp = await receive_secondo_response(reader, bin_list)
        except UnexpectedSystemError:
            raise UnexpectedSystemError('Error occured while receiving the result from Secondo.')

        if resp[0] == 0:
            print('restore was seccessful!')
        else:
            print(str(resp[0]) + '-' + resp[2])
            raise SecondoError('Error occured during restore.')
        
        return resp
    
    else:
        reader.close()
        writer.close()
        raise SecondoAPI_Error('Connection to Secondo reset.')
    


#Command format: save database/<object_name> to <filename>  
async def save_command(reader, writer, bin_list, command):
    """
    This function executes the commands containing save keyword and returns the result message.

    :param reader: The stream reader of the Secondo object.
    :param writer: The stream writer of the Secondo object.
    :param bin_list: A boolean value for result format of the result list returned from Secondo server.
    :param command: The command to be executed.
    :return: The result received from Secondo by calling the function receive_secondo_response().
    """
    
    if reader and writer:
        if not command.startswith('(') :
            command = '(' + command.strip() + ')'
       
        nl_cmd = nl_parse(command, False)
        
        if nl_cmd[0] != 'save' or nl_cmd[2] != 'to':
                print("save_command: command wrong")  
                return None
        obj_name = nl_cmd[1]
        filename = nl_cmd[3]
        
        typ = item_type(filename)
        if not typ in ("string", "symbol", "text"):
            print("save_command: filename wrong")  
            return None
        
        if obj_name == "database":
            writer.write("<DbSave/>\n".encode())
        else:
            writer.write("<ObjectSave>\n".encode())
            writer.write((obj_name + "\n").encode())
            writer.write("</ObjectSave>\n".encode())
        await writer.drain()
           
        try:
            resp = await receive_secondo_response(reader, bin_list)
        except UnexpectedSystemError:
            raise UnexpectedSystemError('Error occured while receiving the result from secondo.')
        if resp:
            if resp[0] == 0:
                stream = open(filename, 'w')
                stream.writelines(resp[3])
                stream.close()
                print('Save Successful!')
            return resp
        
        else:
            raise SecondoAPI_Error('Empty result received from Secondo.')
            
    else:
        reader.close()
        writer.close()
        raise SecondoAPI_Error('Connection to Secondo reset.')


async def general_command(reader, writer, bin_list, command, stream_source = None):
    """
    This function executes the query-commands and returns the result message.

    :param reader: The stream reader of the Secondo object.
    :param writer: The stream writer of the Secondo object.
    :param bin_list: A boolean value for result format of the result list returned from Secondo server.
    :param command: The command to be executed.
    :param stream_source: This is an optional parameter given as a list containning the tuples
    to be sent to Secondo as a stream when the query contains the operator pyreceive[].
    :return: The result received from Secondo by calling the function receive_secondo_response().
    """
    
    if reader and writer:
       
        if 'pysend[' in command.lower():
            
            stream = []
            async def handle_Receive_From_Secondo(reader, writer):
    
                count = 0
                while True:
                    line = await reader.readline()
                    line = line.decode()
                    if 'quit' in line:
                        break
                    
                    if line:
                        count +=1
                        tupel = nl_parse(line, False)
                        if count == 1:
                            stream.append(tupel[1][1])
                        else:
                            stream.append(tupel)
        
                print('Stream of {} Tuples received from SecondoServer.'.format(str(count)))
                writer.close()
    
            
        
            idx1 = command.find('pysend[') + 7
            idx2 = command.find(']', int(idx1))
            port = int(command[idx1:idx2])
            
            loop = asyncio.get_event_loop()
            
            try:
                loop.create_task(asyncio.start_server(handle_Receive_From_Secondo, '127.0.0.1', port, reuse_address = True, reuse_port = True))
            except UnexpectedSystemError:
                raise UnexpectedSystemError('Error occured while trying to contact the port to receive stream of tupels.')
        
        if 'pyreceive[' in command.lower():
            
            if stream_source is None:
                raise SecondoAPI_Error('The Tupel source is empty.')
            async def handle_send_to_Secondo(reader, writer, t_source = stream_source):
                
                count = 0
                for tupel in t_source:
                    tupel_str = list_to_nl(tupel) + "\n"
                    print (tupel_str)
                    writer.write(tupel_str.encode())
                    await writer.drain()
                    count +=1
            
                print('Stream of {} Tuples sent to SecondoServer.'.format(str(count)))
                writer.close()
            
            
            
            idx1 = command.find('pyreceive[') + 10
            idx2 = command.find(']', int(idx1))
            port = int(command[idx1:idx2])
            
            loop = asyncio.get_event_loop()
            
            try:
                    #loop.create_task(asyncio.start_server(lambda r, w: handle_send_to_Secondo(r, w, stream_source), '127.0.0.1', port, reuse_address = True, reuse_port = True))
                loop.create_task(asyncio.start_server(handle_send_to_Secondo, '127.0.0.1', port, reuse_address = True, reuse_port = True))
            except UnexpectedSystemError:
                raise UnexpectedSystemError('Error occured while trying to contact the port to send stream of tupels.')
                    
        cmd_level = str(0 if ((command.lstrip()).lower()).startswith('(') else 1)
    
        message = "<Secondo>\n"
        message = message.encode()
        writer.write(message)
    
        cmd_level += "\n"
        message = cmd_level.encode()
        writer.write(message)
    
        message = command
        message = message.encode()
        writer.write(message)

        message = "\n</Secondo>\n"
        message = message.encode()
        writer.write(message)
        await writer.drain()
                
        
        try:
            result = await receive_secondo_response(reader, bin_list)
        except UnexpectedSystemError:
            raise UnexpectedSystemError('Error occured while receiving the result from Secondo .')
        
        if result and 'pysend[' in command.lower():
            return result, stream
        elif result:
            return result
        else: 
             raise SecondoAPI_ERROR('No response received from Secondo.')
    else:
        reader.close()
        writer.close()
        raise SecondoAPI_ERROR('Connection to Secondo reset.')
    
