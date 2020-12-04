"""
The module read_binary_format defines the function that reads 
the stream of data from Secondo server which contains the result
of commands/ queries in binary format and decodes them to textual format.
"""
from struct import *

import asyncio
import nest_asyncio
nest_asyncio.apply()

from libs_pkg.exception_handler import *

async def binary_decode(reader):
    """
    This function  binary_decode() reads a binary byte from stream of data from Secondo server 
    which contains the result of commands/ queries in binary format. The read byte represents
    the type of next element in the stream. The function then decodes the element to textual format according
    to read type.

    :param reader: The stream reader of the Secondo object that receives the result from Secondo Server.
    :return: The decoded element in the result list.
    """
    
    if reader == None:
        reader.close()
        raise SecondoAPI_ERROR('Connection to Secondo reset.')
    else:    
        Typ = await reader.readexactly(1)
        Typ = int.from_bytes(Typ, byteorder='big', signed=False)
    
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
        type_code = None
        for typ, val in Type_dict.items():
            if val == Typ:
                type_code = val
                break
    
        if type_code == 0: #LongList
            LE = []
            barray = bytearray(4) ####readInt()
            barray = await reader.readexactly(4)
            assert (int.from_bytes(barray, byteorder = 'big', signed = True)) >= 0, "Invalid Integer-Byte read from Stream!"
            r = unpack('>I',barray)[0]
            if r == 0:
                return LE
            else:
                for i in range(r):
                    LE.append(await binary_decode(reader))
                return LE
            
        elif type_code == 1:
            ####readInt()
            barray = bytearray(4)
            barray = await reader.readexactly(4)
            assert (int.from_bytes(barray, byteorder = 'big', signed = True)) >= 0, "Invalid Integer-Byte read from Stream!"
            r = unpack('>I',barray)[0]
            return r 
        
        elif type_code == 2:
            ####readReal()
            barray = bytearray(4)
            barray = await reader.readexactly(4)
            assert (int.from_bytes(barray, order = 'big', signed = True)) >= 0, "Invalid Real-Byte read from Stream!"
            r = unpack('>f',barray)[0]
            return r 
        
        elif type_code == 3:
            ####readBool()
            barray = bytearray(1)
            barray = await reader.readexactly(1)
            assert (int.from_bytes(barray, order = 'big', signed = True)) >= 0, "Invalid Boolean-Byte read from Stream!"
            r = unpack('?',barray)[0]
            return r
        
            
        elif type_code == 4:
            ####readLongString()
            barray = bytearray(4) #readInt()
            barray = await reader.readexactly(4)
            for b in barray:
                assert (int.from_bytes(b, order = 'big', signed = True)) >= 0, "Invalid Int-Byte read from Stream!"
            len = unpack('>I',barray)[0]
            barray = bytearray(len)
            barray = await reader.readexactly(len)
            return '"' + barray.decode() + '"'
        
        elif type_code == 5:
            ####readLongSymbol()
            barray = bytearray(4) #readInt()
            barray = await reader.readexactly(4)
            for b in barray:
                assert (int.from_bytes(b, order = 'big', signed = True)) >= 0, "Invalid Int-Byte read from Stream!"
            len = unpack('>I',barray)[0]
            barray = bytearray(len)
            barray = await reader.readexactly(len)
            return barray.decode()
        
        elif type_code == 6:
            ####readLongText()
            barray = bytearray(4) #readInt()
            barray = await reader.readexactly(4)
            for b in barray:
                assert (int.from_bytes(b, order = 'big', signed = True)) >= 0, "Invalid Int-Byte read from Stream!"
            len = unpack('>I',barray)[0]
            barray = bytearray(len)
            barray = await reader.readexactly(len)
            txt = barray.decode()
            if not txt.startswith('<text>'):
                txt = '"<text>' + txt + '</text--->"'
            return txt
        
        elif type_code == 10: #List
            LE = []
            ####readShort()
            barray = bytearray(2)
            barray = await reader.readexactly(2)
            for b in barray:
                assert (int.from_bytes(b, order = 'big', signed = True)) >= 0, "Invalid ShortInt-Byte read from Stream!"
            r = unpack('>H',barray)[0]
            if r == 0:
                return LE
            else:
                for i in range(r):
                    LE.append(await binary_decode(reader))
                return LE
        
        elif type_code == 11: #ShortList
            LE = []
            ####readByte()
            barray = bytearray(1)
            barray = await reader.readexactly(1)
            r = int.from_bytes(barray, byteorder = 'big', signed = False)
            if r == 0:
                return LE
            else:
                for i in range(r):
                    LE.append(await binary_decode(reader))
                return LE
        
        elif type_code == 12:
            ####readShort()
            barray = bytearray(2)
            barray = await reader.readexactly(2)
            r = unpack('>H',barray)[0]
            return r
        
        elif type_code == 13:
            ####readByte()
            barray = bytearray(1)
            barray = await reader.readexactly(1)
            r = int.from_bytes(barray, byteorder = 'big', signed = True)
            return r
        
        elif type_code == 14:
            ####readString()
            barray = bytearray(2)
            barray = await reader.readexactly(2)
            for b in barray:
                assert (int.from_bytes(b, byteorder = 'big', signed = True)) >= 0, "Invalid ShortInt-Byte read from Stream!"
            len = unpack('>H',barray)[0]
            barray = bytearray(len)
            barray = await reader.readexactly(len)
            return '"' + barray.decode() + '"'
        
        elif type_code == 15:
            ####readShortString()
            barray = bytearray(1)
            barray = await reader.readexactly(1)
            len = int.from_bytes(barray, byteorder = 'big', signed = False)
            barray = bytearray(len)
            barray = await reader.readexactly(len)
            return '"' + barray.decode() + '"'
    
        elif type_code == 16:
            ####readSymbol()
            barray = bytearray(2) #readShort()
            barray = await reader.readexactly(2)
            for b in barray:
                assert (int.from_bytes(b, byteorder = 'big', signed = True)) >= 0, "Invalid ShortInt-Byte read from Stream!"
            len = unpack('>H',barray)[0]
            barray = bytearray(len)
            barray = await reader.readexactly(len)
            return barray.decode()
        
        elif type_code == 17:
            ####readShortSymbol()
            barray = bytearray(1) #readByte()
            barray = await reader.readexactly(1)
            len = int.from_bytes(barray, byteorder = 'big', signed = False)
            barray = bytearray(len)
            barray = await reader.readexactly(len)
            return barray.decode()
        
        elif type_code == 18:
            ####readText()
            barray = bytearray(2) #readShort()
            barray = await reader.readexactly(2)
            len = unpack('>H',barray)[0]
            barray = bytearray(len)
            barray = await reader.readexactly(len)
            ###checking Environment.ENCODING not considered yet
            txt = barray.decode()
            if not txt.startswith('<text>'):
                txt = '"<text>' + txt + '</text--->"'
            
            return txt
        
        
        elif type_code == 19:
            ####readShortText()
            barray = bytearray(1) #readByte()
            barray = await reader.readexactly(1)
            len = int.from_bytes(barray, byteorder = 'big', signed = False)
            barray = bytearray(len)
            barray = await reader.readexactly(len)
            txt = barray.decode()
            if not txt.startswith('<text>'):
                txt = '"<text>' + txt + '</text--->"'
            
            return txt
        
        elif type_code == 20:
            ####readDouble()
            barray = bytearray(8)
            barray = await reader.readexactly(8)
            assert (int.from_bytes(barray, byteorder = 'big', signed = True)) >= 0, "Invalid Double-Byte read from Stream!"
            r = unpack('>d',barray)[0]
            return r
