from unittest import TestCase
from unittest.mock import Mock, patch

from Secondoapi.PySecondo import Secondo
from Secondoapi.libs_pkg.exception_handler import *


class TestQuery(TestCase):

    def setUp(self):
        self.secondo = None

    def tearDown(self):
        if self.secondo is not None:
            if self.secondo.get_initialized():
                self.secondo.close()



    def test_execute_open_database(self):

        secondo = secondo()
        query_response = await secondo.command_exec('open database berlintest')
        
        self.assertIsInstance(secondo.fetch_result(), list)
        self.assertEqual(secondo.get_result_error_code(), 0)
        self.assertEqual(secondo.get_result_error_message(), '')
        
              
        
    def test_execute_sql_query(self):

        secondo = secondo()
        query_response = await secondo.command_exec('sql select * from staedte where bev > 100000')

        self.assertIsInstance(secondo.fetch_result(), list)
        self.assertEqual(secondo.get_result_error_code(), 0)
        self.assertEqual(secondo.get_result_error_message(), '')
        
        
        
    def test_execute_query(self):

        secondo = secondo()
        query_response = await secondo.command_exec('query Staedte feed filter[(.Bev > 100000)] consume')
   
        self.assertIsInstance(secondo.fetch_result(), list)
        self.assertEqual(secondo.get_result_error_code(), 0)
        self.assertEqual(secondo.get_result_error_message(), '')    
        
        
    
    def test_execute_create_relation(self):

        secondo = secondo()
        query_response = await secondo.command_exec('let rel_new = [const rel(tuple([Name: string, Age: int])) value ((“Peter“ 17)(“Klaus“ 31))]')
        
        self.assertIsInstance(secondo.fetch_result(), list)
        self.assertEqual(secondo.get_result_error_code(), 0)
        self.assertEqual(secondo.get_result_error_message(), '')    
        
   
    
    def test_execute_restore(self):

        secondo = secondo()
        query_response = await secondo.command_exec('restore database berlintest from berlintest')
        
        self.assertIsInstance(secondo.fetch_result(), list)
        self.assertEqual(secondo.get_result_error_code(), 0)
        self.assertEqual(secondo.get_result_error_message(), '')    
        
    
    
    def test_execute_list_algebras(self):

        secondo = secondo()
        query_response = await secondo.command_exec('list algebras')
        
        self.assertIsInstance(secondo.fetch_result(), list)
        self.assertEqual(secondo.get_result_error_code(), 0)
        self.assertEqual(secondo.get_result_error_message(), '')    
        
    
    def test_execute_list_algebra(self):

        secondo = secondo()
        query_response = await secondo.command_exec('list algebra Spatial')
        
        self.assertIsInstance(secondo.fetch_result(), list)
        self.assertEqual(secondo.get_result_error_code(), 0)
        self.assertEqual(secondo.get_result_error_message(), '')    
    
    
    
    def test_execute_close_database(self):

        secondo = secondo()
        query_response = await secondo.command_exec('close database')
        
        self.assertIsInstance(secondo.fetch_result(), list)
        self.assertEqual(secondo.get_result_error_code(), 0)
        self.assertEqual(secondo.get_result_error_message(), '') 
    
    
   
        #self.assertIsInstance(response_list, list)
        #self.assertTrue(len(response_list) == 2)
        #self.assertIsNone(result3)

        #with self.assertRaises(InternalError):
         #   cursor.execute_simple_query('mehringdamm')


        #with patch('secondodb.api.secondoapi.Cursor.execute',
         #          return_value=['Success']):
          #  result = cursor.execute_create('testrel', 'type_expression')
           # self.assertTrue(result == 'Success')

   

        #with patch('secondodb.api.secondoapi.Cursor.execute',
         #          side_effect=ProgrammingError('error')):
          #  with self.assertRaises(ProgrammingError):
           #     cursor.execute_create('testrel', 'type_expression')

    