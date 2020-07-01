from unittest import TestCase
import secondodb.api.support.secondocommands as com
from secondodb.api.secondoapi import ProgrammingError


class TestCommands(TestCase):

    def test_apply_parameters_to_operation(self):
        with self.assertRaises(ProgrammingError):
            com.apply_parameters_to_operation('oper {1} and {2}', ['par1'])
