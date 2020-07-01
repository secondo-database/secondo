from unittest import TestCase
from unittest.mock import Mock, patch

from secondodb.api.secondoapi import Connection, InternalError, OperationalError, Cursor, DataError
from secondodb.api.secondoapi import connect
import secondodb.api.support.secondoparser as parser
from socket import socket


class TestConnection(TestCase):

    def setUp(self):
        HOST = '127.0.0.1'
        PORT = '1234'

        self.connection = connect(HOST, PORT, database='BERLINTEST')
        self.cursor = self.connection.cursor()

    def test_parse_query_int(self):
        result = self.cursor.execute("query Trains count")
        self.assertTrue(result[0] == 562)
        self.assertTrue(result[1] == 'int')

    def test_parse_query_bool(self):
        result = self.cursor.execute("query test_bool")
        self.assertTrue(result[0])
        self.assertTrue(result[1] == 'bool')

    def test_parse_query_string(self):
        result = self.cursor.execute("query test_string")
        self.assertTrue(result[0] == 'test')
        self.assertTrue(result[1] == 'string')

    def test_parse_query_real(self):
        result = self.cursor.execute("query test_real")
        self.assertTrue(result[0] > 0)
        self.assertTrue(result[1] == 'real')

    def test_parse_query_point(self):
        result = self.cursor.execute("query mehringdamm")
        self.assertTrue(result[1] == 'point')

    def test_parse_query_points(self):
        result = self.cursor.execute("query Sehenswuerdaspoints")
        self.assertTrue(result[1] == 'points')

    def test_parse_query_mregion(self):
        result = self.cursor.execute("query msnow")
        self.assertTrue(result[1] == 'mregion')

    def test_parse_query_mpoint(self):
        result = self.cursor.execute("query train1")
        self.assertTrue(result[1] == 'mpoint')

    def test_check_identifier_with_underscore(self):
        self.assertFalse(parser.check_identifier("_szsafga"))

    def test_check_identifier_numerical(self):
        self.assertFalse(parser.check_identifier("5156156"))

    def test_check_validity_string_true(self):
        self.assertTrue(parser.check_validity_string("bnl"))

    def test_check_validity_string_false(self):
        with self.assertRaises(DataError):
            parser.check_validity_string("dskahdl")



