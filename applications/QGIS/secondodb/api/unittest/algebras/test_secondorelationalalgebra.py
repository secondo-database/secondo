import unittest
import secondodb.api.secondoapi as api
import secondodb.api.algebras.secondorelationalalgebra as relational


class TestRelationalAlgebra(unittest.TestCase):

    def setUp(self):

        HOST = '127.0.0.1'
        PORT = '1234'

        self.connection = api.connect(HOST, PORT, database='BERLINTEST')
        self.cursor = self.connection.cursor()

    def test_parse_relation_with_point(self):
        response = self.cursor.execute_simple_query('Kinos')
        self.assertIsInstance(response, object)
        self.assertIsInstance(response.attributes, list)
        self.assertIsInstance(response.data, list)

    def test_parse_relation_with_line(self):
        response = self.cursor.execute_simple_query('WStrassen')
        self.assertIsInstance(response, object)
        self.assertIsInstance(response.attributes, list)
        self.assertIsInstance(response.data, list)

    def test_parse_relation_with_region(self):
        response = self.cursor.execute_simple_query('WFlaechen')
        self.assertIsInstance(response, object)
        self.assertIsInstance(response.attributes, list)
        self.assertIsInstance(response.data, list)







