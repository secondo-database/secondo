#returns the type of the given item of NestedList
from struct import *
import asyncio
import nest_asyncio
nest_asyncio.apply()


from libs_pkg.nested_list import *
from libs_pkg.exception_handler import *
    
def binary_type(item):
    """
    The function binary_type() returns the aquivalent binary type of the item in a 
    nested list according to the data types of the nested lists in Secondo.
    :param item: An element in nested list.
    :return: The binary data type.
    """
    itemType = item_type(item)
    if itemType == "string":
        length = len(item)
        if length < 256:
            return 'BIN_SHORTSTRING'
        if length < 65536:
            return 'BIN_STRING'
        return 'BIN_LONGSTRING'
    if itemType == "symbol":
        length = len(item)
        if length < 256:
            return 'BIN_SHORTSYMBOL'
        if length < 65536:
            return 'BIN_SYMBOL'
        return 'BIN_LONGSYMBOL'
    if itemType == "integer":
        if item in range(-128, 128):
            return 'BIN_BYTE'
        if item in range(-32768, 32768):
            return 'BIN_SHORTINT'
        return 'BIN_INTEGER'
    if itemType == "real":
        return 'BIN_DOUBLE'
    if itemType == "boolean":
        return 'BIN_BOOLEAN'
    if itemType == "text":
        length = len(item)
        if length < 256:
            return 'BIN_SHORTTEXT'
        if length < 65536:
            return 'BIN_TEXT'
        return 'BIN_LONGTEXT'
    if itemType == "list":
        length = len(item)
        if length < 256:
            return 'BIN_SHORTLIST'
        if length < 65536:
            return 'BIN_LIST'
        return 'BIN_LONGLIST'
    
 #main function       
async def write_binary_header(writer, NL):
    """
    The function write_binary_header() writes the binary header which contains
    signature and version to the stream writer of Secondo object which is 
    connected to Secondo server and converts each element of the nested list
    to binary format by calling the function binary_encode().
    
    :param writer: Stream writer of the Secondo object.
    :param NL: The nested list to be converted.
    :return: True if the conversion is succesful Flase when not.
    """
    
    if not writer:
        writer.close()
        raise SecondoAPI_ERROR('Connection to secondo reset.')
        
    sig = bytearray("bnl")
    major = pack('>H', 1)
    minor = pack('>H', 2)
    writer.write(sig)
    writer.write(major)
    writer.write(minor)
    await writer.drain()
    ok = await binary_encode(writer, NL)
    await writer.drain()
    return ok
        
       
async def binary_encode(writer, NL):
    """
    The function binary_encode() converts each element of the nested list
    to binary format and writes them to the stream writer of Secondo object 
    which is connected to Secondo server.
    
    :param writer: Stream writer of the Secondo object.
    :param NL: The nested list to be converted.
    :return: True if the conversion is succesful and Flase when not.
    """
    
    if not writer:
        writer.close()
        raise SecondoAPI_ERROR('Connection to secondo reset.')
        
    Type_dict = {"BIN_LONGLIST": 0,
                    "BIN_INTEGER": 1,
                    "BIN_REAL": 2,
                    "BIN_BOOLEAN": 3,
                    "BIN_LONGSTRING": 4,
                    "BIN_LONGSYMBOL": 5,
                    "BIN_LONGTEXT": 6,
                    "BIN_LIST": 10,
                    "BIN_SHORTLIST": 11,
                    "BIN_SHORTINT": 12,
                    "BIN_BYTE":13,
                    "BIN_STRING": 14,
                    "BIN_SHORTSTRING": 15,
                    "BIN_SYMBOL": 16,
                    "BIN_SHORTSYMBOL": 17,
                    "BIN_TEXT": 18,
                    "BIN_SHORTTEXT": 19,
                    "BIN_DOUBLE": 20}
    for item in NL:
        btype = binary_type(item)
        typeCode = None
        for typ, val in Type_dict.items():
            if btype == typ:
                typeCode = val
                break
    
        if btype == 'BIN_BOOLEAN':
            writer.write(typeCode.to_bytes(1, byteorder='big'))
            val = 1 if item else 0
            writer.write(pack('?', val)[0])
            await writer.drain()
            return True
        
        if btype == 'BIN_INTEGER':
            writer.write(typeCode.to_bytes(1, byteorder='big'))
            writer.write(pack('>I', item))
            await writer.drain()
            return True
        if btype == 'BIN_SHORTINT':
            writer.write(typeCode.to_bytes(1, byteorder='big'))
            writer.write(pack('>H', item))
            await writer.drain()
            return True
        if btype == 'BIN_BYTE':
            writer.write(typeCode.to_bytes(1, byteorder='big'))
            writer.write(item.to_bytes(1, byteorder='big'))
            await writer.drain()
            return True
    
        if btype == 'BIN_REAL':
            writer.write(typeCode.to_bytes(1, byteorder='big'))
            writer.write(pack('>f', item)[0])
            await writer.drain()
            return True
    
        if btype == 'BIN_DOUBLE':
            writer.write(typeCode.to_bytes(1, byteorder='big'))
            writer.write(pack('>d', item)[0])
            await writer.drain()
            return True
    
        if btype == 'BIN_DOUBLE':
            writer.write(typeCode.to_bytes(1, byteorder='big'))
            writer.write(pack('>d', item)[0])
            await writer.drain()
            return True
    
        if btype == 'BIN_SHORTSTRING':
            writer.write(typeCode.to_bytes(1, byteorder='big'))
            writer.write(len(item).to_bytes(1, byteorder='big'))
            writer.write(item.encode())
            await writer.drain()
            return True

        if btype == 'BIN_STRING':
            writer.write(typeCode.to_bytes(1, byteorder='big'))
            writer.write(pack('>H', len(item)))
            writer.write(item.encode())
            await writer.drain()
            return True
                     
        if btype == 'BIN_LONGSTRING':
            writer.write(typeCode.to_bytes(1, byteorder='big'))
            writer.write(pack('>I', len(item)))
            writer.write(item.encode())
            await writer.drain()
            return True

        if btype == 'BIN_SHORTSYMBOL':
            writer.write(typeCode.to_bytes(1, byteorder='big'))
            writer.write(len(item).to_bytes(1, byteorder='big'))
            writer.write(item.encode())
            await writer.drain()
            return True

        if btype == 'BIN_SYMBOL':
            writer.write(typeCode.to_bytes(1, byteorder='big'))
            writer.write(pack('>H', len(item)))
            writer.write(item.encode())
            await writer.drain()
            return True
                     
        if btype == 'BIN_LONGSYMBOL':
            writer.write(typeCode.to_bytes(1, byteorder='big'))
            writer.write(pack('>I', len(item)))
            writer.write(item.encode())
            await writer.drain()
            return True

        if btype == 'BIN_SHORTTEXT':
            writer.write(typeCode.to_bytes(1, byteorder='big'))
            writer.write(len(item).to_bytes(1, byteorder='big'))
            writer.write(item.encode())
            await writer.drain()
            return True
                     
        if btype == 'BIN_TEXT':
            writer.write(typeCode.to_bytes(1, byteorder='big'))
            writer.write(pack('>H', len(item)))
            writer.write(item.encode())
            await writer.drain()
            return True

        if btype == 'BIN_LONGTEXT':
            writer.write(typeCode.to_bytes(1, byteorder='big'))
            writer.write(pack('>I', len(item)))
            writer.write(item.encode())
            await writer.drain()
            return True

        if btype == 'BIN_SHORTLIST':
            writer.write(typeCode.to_bytes(1, byteorder='big'))
            writer.write(len(item).to_bytes(1, byteorder='big'))
            for i in range(len(item)):
                res = await binary_encode(writer, item[i])
                if not res:
                    return False
            await writer.drain()
            return True
                     
        if btype == 'BIN_LIST':
            writer.write(typeCode.to_bytes(1, byteorder='big'))
            writer.write(pack('>H', len(item)))
            for i in range(len(item)):
                res = await binary_encode(writer, item[i])
                if not res:
                    return False
            await writer.drain()
            return True
                     
        if btype == 'BIN_LONGLIST':
            writer.write(typeCode.to_bytes(1, byteorder='big'))
            writer.write(pack('>I', len(item)))
            for i in range(len(item)):
                res = await binary_encode(writer, item[i])
                if not res:
                    return False
            await writer.drain()
            return True
