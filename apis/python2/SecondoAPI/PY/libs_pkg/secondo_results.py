"""
The module secondo_results contains two functions for receiving the results from Secondo
and decoding the result to textual format if the default seocndo result format is binary.
"""
import asyncio
import nest_asyncio
nest_asyncio.apply()
from struct import *


from libs_pkg.nested_list import nl_parse
from libs_pkg.read_binary_format import *
from libs_pkg.exception_handler import *

async def receive_query_result_list(reader, final_token, bin_list):
    """
        The function receive_query_result_list() reads the result list after execution of
        commands/ queries from stream reader of the Secondo object until reaching the 
        final token then stops reading. If the  the default seocndo result format is 
        binary it calls another function for decoding the result to textual format.

        :param reader: The stream reader of the Secondo object.
        :param final_token: Stops reading by receiving this element from the stream.
        :param bin_list: A binary value that defines the default seocndo result format, True when binary and False when textual nested list.
        :return: The result list in textual/ binary format.
        """
    result_list = []
    if not bin_list:
        result = ''
        count = 1
        
        if reader == None:
            reader.close()
            raise SecondoAPI_ERROR('Connection to Secondo reset.')
        
        while True: 
            line = await reader.readline()
            count += 1
            line = line.decode('ISO-8859-1').rstrip()
            if final_token in line:
                result += line
                break
                
            if line:
                result += line # + ' '
            else:
                raise SecondoAPI_ERROR('Empty line read from Secondo.')
        result_list = nl_parse(result, False)
    
    else: #bin_list = True --> readBinaryFrom(inputStream)
        line = await reader.readexactly(3)
        signature = line.decode().rstrip()
        
        #readShort()
        barray = bytearray(2)
        barray = await reader.readexactly(2)
        major = unpack('>H',barray)
        major = major[0]
        
        barray = await reader.readexactly(2)
        minor = unpack('>H',barray)
        minor = minor[0]
        
        
        if signature != "bnl":
            raise SecondoAPI_ERROR(signatur + ' is wrong Signatur.')
        
        
        if (major != 1 or (minor != 0 and minor != 1 and minor != 2)):
            raise SecondoAPI_ERROR(str(major) + '.' + str(minor) + ' is wrong Signatur.')
            
        result_list = await binary_decode(reader)
        line = await reader.readline()
        
    return result_list



async def receive_secondo_response(reader, bin_list):
    
    """
    This function receive_secondo_response() reads the response of the Secondo server after execution of
    commands/ queries from stream reader of the Secondo object until reaching specific tags. 
    It calls the function receive_query_result_list() when receiving the <SecondoResponse> tag to get the actual result list.

    :param reader: The stream reader of the Secondo object.
    :param bin_list: A binary value that defines the default seocndo result format, True when binary and False when textual nested list.
    :return: The result list in textual/ binary format.
    """
    
    if not reader:
        raise SecondoAPI_ERROR('Connection to SecondoServer reset.')
    line = await reader.readline()
    line = line.decode().rstrip()
    
    if not line:
        raise SecondoAPI_ERROR('Connection to SecondoServer reset.')
    count = 2
    while line == "<Message>":
        Message_List = await receive_query_result_list(reader, "</Message>", bin_list)
        
        if not Message_List:
            raise SecondoAPI_ERROR('Error- MessageList from Secondo is empty.')
        
        line = await reader.readline()
        count += 1
        line = line.decode().rstrip()
        
        if not line:
            raise SecondoAPI_ERROR('Empty line read from Secondo.')
   
    if line == "<SecondoError>":
        line = await reader.readline()
        line = line.decode().rstrip()
        if line:
            line = await reader.readline()
            line = line.decode().rstrip()
       
        else:
            raise SecondoERROR('Empty result while trying to read Error from Secondo.')
        return None
    
    if line != "<SecondoResponse>":
        raise SecondoERROR('Wrong Data received from Secondo.')
        
    result_list = await receive_query_result_list(reader, "</SecondoResponse>", bin_list)
    if not result_list:
        return None
    return result_list

