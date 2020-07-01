import secondodb.api.support.secondolistexpr as le
from unittest import TestCase


class TestListExp(TestCase):

    def test_one_element_list(self):
        elem1 = le.ListExp()

        result = le.one_element_list(elem1)
        self.assertIsInstance(result, le.ListExp)
        self.assertTrue(result.get_list_length() == 2)

    def test_two_element_list(self):
        elem1 = le.ListExp()
        elem2 = le.ListExp()

        result = le.two_element_list(elem1, elem2)
        self.assertIsInstance(result, le.ListExp)
        self.assertTrue(result.get_list_length() == 3)

    def test_three_element_list(self):
        elem1 = le.ListExp()
        elem2 = le.ListExp()
        elem3 = le.ListExp()

        result = le.three_element_list(elem1, elem2, elem3)
        self.assertIsInstance(result, le.ListExp)
        self.assertTrue(result.get_list_length() == 4)

    def test_four_element_list(self):
        elem1 = le.ListExp()
        elem2 = le.ListExp()
        elem3 = le.ListExp()
        elem4 = le.ListExp()

        result = le.four_element_list(elem1, elem2, elem3, elem4)
        self.assertIsInstance(result, le.ListExp)
        self.assertTrue(result.get_list_length() == 5)

    def test_five_element_list(self):
        elem1 = le.ListExp()
        elem2 = le.ListExp()
        elem3 = le.ListExp()
        elem4 = le.ListExp()
        elem5 = le.ListExp()

        result = le.five_element_list(elem1, elem2, elem3, elem4, elem5)
        self.assertIsInstance(result, le.ListExp)
        self.assertTrue(result.get_list_length() == 6)

    def test_six_element_list(self):
        elem1 = le.ListExp()
        elem2 = le.ListExp()
        elem3 = le.ListExp()
        elem4 = le.ListExp()
        elem5 = le.ListExp()
        elem6 = le.ListExp()

        result = le.six_element_list(elem1, elem2, elem3, elem4, elem5, elem6)
        self.assertIsInstance(result, le.ListExp)
        self.assertTrue(result.get_list_length() == 7)

    def test_create_text_atom_if_true(self):

        result = le.create_text_atom('test')

        self.assertIsInstance(result, le.ListExp)
        self.assertTrue(le.check_if_atom(result))

    def test_create_text_atom_if_false(self):

        result = le.ListExp()

        self.assertIsInstance(result, le.ListExp)
        self.assertFalse(le.check_if_atom(result))

    def test_end_of_list_if_true(self):
        result = le.ListExp()
        self.assertFalse(result.end_of_list())

    def test_end_of_list_if_false(self):
        result = le.create_text_atom('test')
        self.assertFalse(result.end_of_list())

    def test_get_fifth_element(self):
        elem1 = le.ListExp()
        elem2 = le.ListExp()
        elem3 = le.ListExp()
        elem4 = le.ListExp()
        elem5 = le.ListExp()

        five_element_list = le.five_element_list(elem1, elem2, elem3, elem4, elem5)
        result = five_element_list.get_fifth_element()

        self.assertIsInstance(result, le.ListExp)

    def test_get_sixth_element(self):
        elem1 = le.ListExp()
        elem2 = le.ListExp()
        elem3 = le.ListExp()
        elem4 = le.ListExp()
        elem5 = le.ListExp()
        elem6 = le.ListExp()

        six_element_list = le.six_element_list(elem1, elem2, elem3, elem4, elem5, elem6)
        result = six_element_list.get_sixth_element()

        self.assertIsInstance(result, le.ListExp)

    def test_get_last_node_one_element(self):

        elem = le.ListExp()
        result = elem.get_last_node()
        self.assertIsInstance(result, le.ListExp)
        self.assertIsNone(result.next)

    def test_get_last_node_many_elements(self):

        elem1 = le.ListExp()
        elem2 = le.ListExp()
        elem3 = le.ListExp()
        elem4 = le.ListExp()
        elem5 = le.ListExp()
        elem6 = le.ListExp()

        six_element_list = le.six_element_list(elem1, elem2, elem3, elem4, elem5, elem6)
        result = six_element_list.get_last_node()

        self.assertIsInstance(result, le.ListExp)
        self.assertIsNone(result.next)

    def test_add_length_2(self):

        add_element = le.create_text_atom('test1')
        list_exp = le.create_text_atom('test2')

        list_exp.add(add_element)
        self.assertEqual(list_exp.get_list_length(), 2)

    def test_add_length_1(self):

        add_element = le.create_text_atom('test1')
        list_exp = le.ListExp()

        list_exp.add(add_element)
        self.assertEqual(list_exp.get_list_length(), 1)




