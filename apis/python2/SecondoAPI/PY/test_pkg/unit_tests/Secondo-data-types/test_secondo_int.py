from unittest import TestCase
from unittest.mock import Mock, patch

from Secondoapi.PySecondo import Secondo
from Secondoapi.libs_pkg.exception_handler import *
from Secondoapi.secondo_datatypes_pkg.secondo_int import *

class test_int(TestCase):

    def setUp(self):
        self.secondo = None

    def tearDown(self):
        if self.secondo is not None:
            if self.secondo.get_initialized():
                self.secondo.close()



    def test_query_result_int(self):

        secondo = secondo()
        query_response = await secondo.command_exec('open database berlintest')
        
        self.assertIsInstance(secondo.fetch_result(), list)
        self.assertEqual(secondo.get_result_error_code(), 0)
        self.assertEqual(secondo.get_result_error_message(), '"<text></text--->"')
        
        query_response = await secondo.command_exec('query Staedte feed filter[(.Bev > 100000)] count')
              
        self.assertIsInstance(secondo.fetch_result(), list)
        self.assertEqual(secondo.get_result_error_code(), 0)
        self.assertEqual(secondo.get_result_error_message(), '"<text></text--->"')
        
        int_result = secondo_int()
        int_result.in_from_list(secondo.fetch_result_rows())
        
        self.assertIsInstance(int_result, secondo_int)
        self.assertEqual(int_result.get_value(), (secondo.fetch_result_rows())
        
        
       
        
        
    