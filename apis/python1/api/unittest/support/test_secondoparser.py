# from unittest import TestCase
# from unittest.mock import Mock, patch
#
# from secondodb.api.secondoapi import Connection, InternalError, OperationalError, Cursor
# from secondodb.api.secondoapi import connect
# from socket import socket
#
#
# class TestConnection(TestCase):
#
#     def setUp(self):
#         HOST = '127.0.0.1'
#         PORT = '1234'
#
#         self.connection = connect(HOST, PORT, database='BERLINTEST')
#         self.cursor = self.connection.cursor()
#
#     def test_parse_inquiry_databases(self):
#         pass