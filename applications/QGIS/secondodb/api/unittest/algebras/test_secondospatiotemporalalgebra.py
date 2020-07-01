import unittest
from datetime import datetime

import secondodb.api.algebras.secondospatiotemporalalgebra as spatiotemporal
import secondodb.api.algebras.secondospatialalgebra as spatial
import secondodb.api.secondoapi as api


class TestSpatioTemporalAlgebra(unittest.TestCase):

    def setUp(self):
        HOST = '127.0.0.1'
        PORT = '1234'

        self.connection = api.connect(HOST, PORT, database='BERLINTEST')
        self.cursor = self.connection.cursor()

    def test_parse_mpoint(self):
        response = self.cursor.execute_simple_query('train1')
        self.assertIsInstance(response, spatiotemporal.MPoint)
        self.assertIsInstance(response.intervals, list)
        self.assertTrue(len(response.intervals) == 113)
        self.assertIsInstance(response.intervals[0], spatiotemporal.PointInInterval)

        point_in_interval = response.intervals[0]
        self.assertIsInstance(point_in_interval.interval, spatiotemporal.Interval)
        self.assertIsInstance(point_in_interval.motion_vector, spatiotemporal.MotionVector)

    def test_parse_mregion(self):
        response = self.cursor.execute_simple_query('msnow')

        self.assertIsInstance(response, spatiotemporal.MRegion)
        self.assertIsInstance(response.intervals, list)
        self.assertTrue(len(response.intervals) == 5)
        self.assertIsInstance(response.intervals[0], spatiotemporal.RegionInInterval)

        region_in_interval = response.intervals[0]
        self.assertIsInstance(region_in_interval.interval, spatiotemporal.Interval)
        self.assertIsInstance(region_in_interval.map_faces, list)

    def test_parse_iregion(self):
        response = self.cursor.execute_simple_query('mrain atinstant instant("2003-11-20-06:06")')

        self.assertIsInstance(response, spatiotemporal.IRegion)
        self.assertIsInstance(response.instant, datetime)
        self.assertIsInstance(response.region, spatial.Region)

    def test_parse_ipoint(self):
        response = self.cursor.execute_simple_query('train1 atinstant instant("2003-11-20-06:06")')

        self.assertIsInstance(response, spatiotemporal.IPoint)
        self.assertIsInstance(response.instant, datetime)
        self.assertIsInstance(response.point, spatial.Point)

    def test_parse_mint(self):
        response = self.cursor.execute_simple_query('noAtCenter')

        self.assertIsInstance(response, spatiotemporal.MInt)
        self.assertIsInstance(response.intervals, list)
        self.assertIsInstance(response.intervals[0], spatiotemporal.IntInInterval)
        self.assertIsInstance(response.intervals[0].interval, spatiotemporal.Interval)
        self.assertIsInstance(response.intervals[0].value_vector, spatiotemporal.ValueVectorInt)

    def test_parse_mstring(self):
        response = self.cursor.execute_simple_query('train7downsights')

        self.assertIsInstance(response, spatiotemporal.MString)
        self.assertIsInstance(response.intervals, list)
        self.assertIsInstance(response.intervals[0], spatiotemporal.StringInInterval)
        self.assertIsInstance(response.intervals[0].interval, spatiotemporal.Interval)
        self.assertIsInstance(response.intervals[0].value_vector, spatiotemporal.ValueVectorString)

    def test_parse_mreal(self):
        response = self.cursor.execute_simple_query('mreal5000')

        self.assertIsInstance(response, spatiotemporal.MReal)
        self.assertIsInstance(response.intervals, list)
        self.assertIsInstance(response.intervals[0], spatiotemporal.RealInInterval)
        self.assertIsInstance(response.intervals[0].interval, spatiotemporal.Interval)
        self.assertIsInstance(response.intervals[0].value_vector, spatiotemporal.ValueVectorReal)

    def test_parse_mbool(self):
        response = self.cursor.execute_simple_query('test_mbool3')

        self.assertIsInstance(response, spatiotemporal.MBool)
        self.assertIsInstance(response.intervals, list)
        self.assertIsInstance(response.intervals[0], spatiotemporal.BoolInInterval)
        self.assertIsInstance(response.intervals[0].interval, spatiotemporal.Interval)
        self.assertIsInstance(response.intervals[0].value_vector, spatiotemporal.ValueVectorBool)
