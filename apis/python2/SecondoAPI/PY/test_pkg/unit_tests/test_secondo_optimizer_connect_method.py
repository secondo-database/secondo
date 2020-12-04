from unittest import TestCase
from unittest.mock import Mock, patch

from Secondoapi.PySecondo import secondo
from Secondoapi.config_pkg.load_secondo_config import *
from Secondoapi.optimizer_pkg.optimizer_server import *
#from api.PySecondo import InterfaceError, connect
from Secondoapi.libs_pkg.exception_handler import SecondoAPI_Error


class Test_Secondo(TestCase):

    def setUp(self):
        self.secondo = None

    def tearDown(self):
        if self.secondo is not None:
            if self.secondo.get_initialized():
                self.secondo.close()
            if self.secondo.get_sec_conn() is not None:
                self.secondo.get_sec_conn.close()
                self.secondo.get_sec_streams[0].close()
                self.secondo.get_sec_streams[1].close()
                
                
                
                
    @patch('config_pkg.load_secondo_config.Config')
    def test_call_Config_from_secondo(self, mock_config):
        secondo =  secondo()
        self.assertTrue(mock_config.called)
      
    
    @patch('optimizer_pkg.optimizer_server.Optimizer')
    def test_call_optimizer_from_secondo(self, mock_optimizer):
        secondo =  secondo()
        self.assertTrue(mock_optimizer.called)
    
    
    @patch.object(config_pkg.load_secondo_config.Config, 'initialize')
    def test_secondo_config(self, mock_cfg_initialize):
        
        mock_cfg_initialize.return_value = ['127.0.0.1',1234,'','']
        secondo = secondo()
        result = [secondo.get_server(), secondo.get_port(), secondo.get_user(), secondo.get_password()] 
        self.assertEqual(result, mock_cfg_initialize.return_value)
        
        
        
    @patch.object('optimizer_pkg.optimizer_server.Optimizer')
    def test_secondo_optimizer(self, mock_optimizer):
        
        opt_values = ['127.0.0.1',1235]
        secondo = secondo()
        result = [secondo.get_opt.get_server(), secondo.get_opt.get_port()] 
        self.assertEqual(result, opt_values)    
        
        
        
        
        
        
    def test_connect(self):    
        
        self.secondo = secondo()
        self.assertTrue(self.secondo.get_sec_conn())
        self.assertTrue(self.secondo.get_initialized())
        self.assertTrue(self.secondo.get_sec_streams()[0])
        self.assertTrue(self.secondo.get_sec_streams()[1])
        self.assertTrue(self.secondo.get_opt())
        


    def test_init_with_non_existant_db(self):

        HOST = '127.0.0.1'
        PORT = 1234
        user = 'username'
        pswd = 'passwd'
        db = 'nonexistantdb'

        with self.assertRaises(InterfaceError):
            self.connection = Connection(HOST, PORT, user, pswd, db)

   
    def test_close_secondo(self):

        self.test_connect()
        result = self.secondo.close()
        self.assertTrue(result)

    def test_close_secondo_without_init(self):

        self.test_connect()
        self.test_close_secondo()
        with self.assertRaises(SecondoAPI_Error):
            self.secondo.close()

  