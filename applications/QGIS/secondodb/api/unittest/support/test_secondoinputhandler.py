import unittest

from mock import patch, Mock

import secondodb.api.secondoapi as api
import secondodb.api.support.secondoinputhandler as handler
from secondodb.api.support.secondolistexpr import ListExp


class TestInputHandler(unittest.TestCase):

    @patch("secondodb.api.support.secondoinputhandler.read_string")
    def test_build_list_expr_from_binary_bad_signature(self, mocked_read_string):

        mocked_read_string.return_value = 'bad_string'
        mocked_socket = Mock()

        with self.assertRaises(api.InterfaceError):
            handler.build_list_expr_from_binary(mocked_socket)

    @patch("secondodb.api.support.secondoinputhandler.read_string")
    @patch("secondodb.api.support.secondoinputhandler.read_short")
    def test_build_list_expr_from_binary_bad_version(self, mocked_read_short, mocked_read_string):

        mocked_read_string.return_value = 'bnl'
        mocked_read_short.return_value = 9
        mocked_socket = Mock()

        with self.assertRaises(api.InterfaceError):
            handler.build_list_expr_from_binary(mocked_socket)

    @patch("secondodb.api.support.secondoinputhandler.read_int")
    @patch("secondodb.api.support.secondoinputhandler.read_byte")
    def test_read_binary_record_int(self, mocked_read_byte, mocked_read_int):

        mocked_read_byte.return_value = [1]
        mocked_read_int.return_value = 25
        mocked_socket = Mock()

        self.assertIsInstance(handler.read_binary_record(mocked_socket), ListExp)

    @patch("secondodb.api.support.secondoinputhandler.read_real")
    @patch("secondodb.api.support.secondoinputhandler.read_byte")
    def test_read_binary_record_real(self, mocked_read_byte, mocked_read_real):

        mocked_read_byte.return_value = [2]
        mocked_read_real.return_value = 2.1244
        mocked_socket = Mock()

        self.assertIsInstance(handler.read_binary_record(mocked_socket), ListExp)

    @patch("secondodb.api.support.secondoinputhandler.read_string")
    @patch("secondodb.api.support.secondoinputhandler.read_int")
    @patch("secondodb.api.support.secondoinputhandler.read_byte")
    def test_read_binary_record_longstring(self, mocked_read_byte, mocked_read_int, mocked_read_string):

        mocked_read_byte.return_value = [4]
        mocked_read_int.return_value = 10
        mocked_read_string.return_value = 'abcdeefghi'
        mocked_socket = Mock()

        self.assertIsInstance(handler.read_binary_record(mocked_socket), ListExp)

    @patch("secondodb.api.support.secondoinputhandler.read_string")
    @patch("secondodb.api.support.secondoinputhandler.read_int")
    @patch("secondodb.api.support.secondoinputhandler.read_byte")
    def test_read_binary_record_longsymbol(self, mocked_read_byte, mocked_read_int, mocked_read_string):

        mocked_read_byte.return_value = [5]
        mocked_read_int.return_value = 10
        mocked_read_string.return_value = 'abcdeefghi'
        mocked_socket = Mock()

        self.assertIsInstance(handler.read_binary_record(mocked_socket), ListExp)

    @patch("secondodb.api.support.secondoinputhandler.read_string")
    @patch("secondodb.api.support.secondoinputhandler.read_int")
    @patch("secondodb.api.support.secondoinputhandler.read_byte")
    def test_read_binary_record_longtext(self, mocked_read_byte, mocked_read_int, mocked_read_string):

        mocked_read_byte.return_value = [6]
        mocked_read_int.return_value = 10
        mocked_read_string.return_value = 'abcdeefghi'
        mocked_socket = Mock()

        self.assertIsInstance(handler.read_binary_record(mocked_socket), ListExp)

    @patch("secondodb.api.support.secondoinputhandler.read_short")
    @patch("secondodb.api.support.secondoinputhandler.read_byte")
    def test_read_binary_record_int(self, mocked_read_byte, mocked_read_short):

        mocked_read_byte.return_value = [12]
        mocked_read_short.return_value = 25
        mocked_socket = Mock()

        self.assertIsInstance(handler.read_binary_record(mocked_socket), ListExp)

    @patch("secondodb.api.support.secondoinputhandler.read_string")
    @patch("secondodb.api.support.secondoinputhandler.read_short")
    @patch("secondodb.api.support.secondoinputhandler.read_byte")
    def test_read_binary_record_string(self, mocked_read_byte, mocked_read_short, mocked_read_string):

        mocked_read_byte.return_value = [14]
        mocked_read_short.return_value = 10
        mocked_read_string.return_value = 'abcdeefghi'
        mocked_socket = Mock()

        self.assertIsInstance(handler.read_binary_record(mocked_socket), ListExp)

    @patch("secondodb.api.support.secondoinputhandler.read_string")
    @patch("secondodb.api.support.secondoinputhandler.read_short")
    @patch("secondodb.api.support.secondoinputhandler.read_byte")
    def test_read_binary_record_symbol(self, mocked_read_byte, mocked_read_short, mocked_read_string):

        mocked_read_byte.return_value = [16]
        mocked_read_short.return_value = 10
        mocked_read_string.return_value = 'abcdeefghi'
        mocked_socket = Mock()

        self.assertIsInstance(handler.read_binary_record(mocked_socket), ListExp)