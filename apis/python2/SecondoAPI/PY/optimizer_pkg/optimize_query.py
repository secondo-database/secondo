"""
The module optimize_query contains a function which sends the query
to Optimizer to be optimized and returns the optimized query back.
"""
import asyncio
import nest_asyncio
nest_asyncio.apply()

from libs_pkg.exception_handler import *

async def opt_comm_exec(reader, writer, query, db_name, flag):
    """
    This function  sends the query to Optimizer to be optimized and returns the optimized query back.
    
    :param reader: The stream reader of the connection to Optimizer.
    :param writer: The stream writer of the connection to Optimizer.
    :param query: The the query to be optimized given as string.
    :param db_name: The The currently open database related to the given query.
    :param flag: The boolean value that shows if the command should be executed by optimizer (True) or should be optimized (False).
    
    :return: The optimized query as string.
    """
    
    if reader == None or writer == None:
        raise SecondoAPI_Error('Connection to Optimizer reset.')
    else:
        
        #execute query
        if flag:
            writer.write(b'<execute>\n')
        
        #optimize query
        else: 
            writer.write(b'<optimize>\n')
    
        writer.write(b'<database>\n')
        db_name += "\n"
        writer.write(db_name.encode())
    
        writer.write(b'</database>\n')
    
        writer.write(b'<query>\n')
        query += "\n"
        writer.write(query.encode())
    
        writer.write(b'</query>\n')
    
        if flag:
            writer.write(b'</execute>\n')
        
        else: 
            writer.write(b'</optimize>\n')
    
        await writer.drain()
    
        res = await reader.readline()
        res = res.decode()
        if not res:
            raise SecondoAPI_Error("Connection to Optimizer is broken.")
        if res != "<answer>\n":
            raise SecondoError("Wrong Data received from Optimizer.")
        res = await reader.readline()
        res = res.decode()
        if not res:
            raise SecondoAPI_Error('Connection to Optimizer is broken.')
        opt_res = ""
        count = 3
        while res != "</answer>\n":
            if flag:
                opt_res += res
            else:
                opt_res += res.replace(" \n", "")
            res = await reader.readline()
            res = res.decode()
            count += 1
            if not res:
                raise SecondoAPI_Error('Connection to Optimizer is broken.')
        
        if opt_res == "":
            raise SecondoAPI_Error('Optimization failed due to some Errors in the query.')
        return opt_res