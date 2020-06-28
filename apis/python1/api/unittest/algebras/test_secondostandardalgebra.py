import unittest
import secondodb.api.algebras.secondostandardalgebra as standard
from secondodb.api.support.secondolistexpr import ListExp


class TestStandardAlgebra(unittest.TestCase):

    def test_parse_int(self):

        listexp = ListExp()
        listexp.value = 123

        self.assertIsInstance(standard.parse_int(listexp), int)

    def test_parse_real(self):

        listexp = ListExp()
        listexp.value = 1.123

        self.assertIsInstance(standard.parse_real(listexp), float)

    def test_parse_bool(self):

        listexp = ListExp()
        listexp.value = True

        self.assertIsInstance(standard.parse_bool(listexp), bool)

    def test_parse_string(self):

        listexp = ListExp()
        listexp.value = 'test'

        self.assertIsInstance(standard.parse_string(listexp), str)

    def test_parse_longint(self):
        listexp = ListExp()
        listexp.value = 118515818

        self.assertIsInstance(standard.parse_longint(listexp), int)

    def test_parse_rational(self):
        listexp = ListExp()
        listexp.value = 1/2

        self.assertIsInstance(standard.parse_rational(listexp), float)
