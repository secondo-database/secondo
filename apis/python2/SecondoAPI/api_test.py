#!/usr/bin/env python3

# Add the module to your pathon path
# export PYTHONPATH=$PYTHONPATH:/home/nidzwetzki/secondo/apis/python2/SecondoAPI/PY
# 
# Install needed modules
# python3 -m venv .venv
# source .venv/bin/activate
# pip3 install nest_asyncio
# pip3 install pyparsing

import asyncio
from PySecondo import *

async def execute_api_calls():
   scn = Secondo()

   print("Open database opt")
   await scn.command_exec("open database opt")
   
   print("Performing plz query")
   plz_output = await scn.command_exec('query plz')
   print(plz_output)
 
   print("Performing plz query (count)")
   plz_output_count = await scn.command_exec('query plz count')
   print(plz_output_count)
   
   print("Performing plz sql query")
   plz_output_sql = await scn.command_exec('select * from plz')
   print(plz_output_sql)

   print("Performing query with pysend")
   await scn.command_exec('query plz feed pysend[30000] count')

   for item in scn.fetch_stream_result():
       print(item)

   #print ("Performing delete query")
   #await scn.command_exec('delete testrel')
   
   print ("Performing insert query with pyreceive")
   tuples = [[1059,"Dresden"], [1060,"Dresden"], [1001,"Dresden"], [1002,"Dresden"], 
             [1003,"Dresden"],[1004,"Dresden"],[1005,"Dresden"],[1006,"Dresden"],
             [1007,"Dresden"],[1008,"Dresden"]]
   await scn.command_exec('let testrel = [ const rel(tuple([Plz: int, Ort: string])) value() ] pyreceive[30000] consume', tupel_source = tuples)

   print("Performing testrel query")
   testrel_output = await scn.command_exec('query testrel')
   print(testrel_output)

   print ("Closing connection")
   scn.close()

# Python 3.7+
# asyncio.run(execute_api_calls())

# Python 3.6
asyncio.get_event_loop().run_until_complete(execute_api_calls())

