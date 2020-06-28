# ----------------------------------------------------------------------------------------------------------------------
# The Secondo Python API (pySecondo)
# Victor Silva (victor.silva@posteo.de)
# January 2020
# ----------------------------------------------------------------------------------------------------------------------
# Module
# ----------------------------------------------------------------------------------------------------------------------
# Secondo Spatio-Temporal Algebra
# secondospatiotemporalalgebra.py
# ----------------------------------------------------------------------------------------------------------------------
"""
The module Secondo Spatio-Temporal Algebra implements the data types for the conversion of list expression objects with
values of the types contained in the TemporalAlgebra of the |sec| system. The data types are implemented in Python
using Data Classes. Data Classes are implemented in the API like normal classes without behaviour.
Like regular classes their attributes can be called through the given names.
"""

from collections import namedtuple
from dataclasses import dataclass

from secondodb.api.support.secondolistexpr import ListExp
import secondodb.api.algebras.secondospatialalgebra as spatial
from datetime import datetime


@dataclass
class Interval:
    """
    Implements a time interval.
    """

    __slots__ = ["start_time", "end_time", "close_left", "close_right"]

    start_time: datetime
    end_time: datetime
    close_left: bool
    close_right: bool


@dataclass
class MotionVector:
    """
    Implements a motion vector of a moving point.
    """

    __slots__ = ["x1", "y1", "x2", "y2"]

    x1: float
    y1: float
    x2: float
    y2: float


@dataclass
class PointInInterval:
    """
    Implements a structure to store the time interval and the motion vector of a moving point.
    """

    __slots__ = ["interval", "motion_vector"]

    interval: Interval
    motion_vector: MotionVector


@dataclass
class MPoint:
    """
    Implements a moving point as a series of points in intervals.
    """

    __slots__ = ['intervals']

    intervals: []


@dataclass
class MapPoint:
    """
    Implements a map point, i.e. a motion vector for a point of a moving region.
    """

    __slots__ = ["x1", "y1", "x2", "y2"]

    x1: float
    y1: float
    x2: float
    y2: float


@dataclass
class RegionInInterval:
    """
    Implements a structure to store the time interval and the motion vector of a moving region.
    """

    __slots__ = ["interval", "map_faces"]

    interval: Interval
    map_faces: []


@dataclass
class MRegion:
    """
    Implements a moving region as a series of regions in intervals.
    """

    __slots__ = ['intervals']

    intervals: []


@dataclass
class MBool:
    """
    Implements a moving boolean as a series of booleans in intervals.
    """

    __slots__ = ['intervals']

    intervals: []


@dataclass
class BoolInInterval:
    """
    Implements a structure to store the time interval and the value vector of a moving boolean.
    """

    __slots__ = ["interval", "value_vector"]

    interval: Interval
    value_vector: []


@dataclass
class ValueVectorBool:
    """
    Implements the value vector for a moving boolean.
    """

    __slots__ = ["value"]

    value: bool


@dataclass
class RealInInterval:
    """
    Implements a structure to store the time interval and the value vector of a moving real.
    """

    __slots__ = ["interval", "value_vector"]

    interval: Interval
    value_vector: []


@dataclass
class ValueVectorReal:
    """
    Implements the value vector for a moving real.
    """

    __slots__ = ["start_value", "end_value", "boolean"]

    start_value: float
    end_value: float
    boolean: bool


@dataclass
class MReal:
    """
    Implements a moving real as a series of real values in intervals.
    """
    __slots__ = ['intervals']

    intervals: []


@dataclass
class MString:
    """
    Implements a moving string as a series of string values in intervals.
    """

    __slots__ = ['intervals']

    intervals: []


@dataclass
class StringInInterval:
    """
    Implements a structure to store the time interval and the value vector of a moving string.
    """

    __slots__ = ["interval", "value_vector"]

    interval: Interval
    value_vector: []


@dataclass
class ValueVectorString:
    """
    Implements the value vector for a moving string.
    """

    __slots__ = ["value"]

    value: str


@dataclass
class MInt:
    """
    Implements a moving integer as a series of integer values in intervals.
    """
    __slots__ = ['intervals']

    intervals: []


@dataclass
class IntInInterval:
    """
    Implements a structure to store the time interval and the value vector of a moving integer.
    """

    __slots__ = ["interval", "value_vector"]

    interval: Interval
    value_vector: []


@dataclass
class ValueVectorInt:
    """
    Implements the value vector for a moving integer.
    """

    __slots__ = ["value"]

    value: int


@dataclass
class IRegion:
    """
    Implements an instant region as a region for a specific instant.
    """

    __slots__ = ["instant", "region"]

    instant: datetime
    region: spatial.Region


@dataclass
class IPoint:
    """
    Implements an instant point as a point for a specific instant.
    """

    __slots__ = ["instant", "point"]

    instant: datetime
    point: spatial.Point


def parse_timestamp(timestamp: str) -> datetime:
    """
    Parses a timestamp string using the Python module datetime.

    Formats supported:

    '%Y-%m-%d-%H:%M'
    '%Y-%m-%d-%H:%M:%S'
    '%Y-%m-%d-%H:%M:%S.%f'

    :param timestamp: A string with a timestamp.
    :return: A datetime object.
    """

    try:
        timestamp_as_datetime = datetime.strptime(timestamp, '%Y-%m-%d-%H:%M')
    except ValueError:
        pass
    else:
        return timestamp_as_datetime

    try:
        timestamp_as_datetime = datetime.strptime(timestamp, '%Y-%m-%d-%H:%M:%S')
    except ValueError:
        pass
    else:
        return timestamp_as_datetime

    try:
        timestamp_as_datetime = datetime.strptime(timestamp, '%Y-%m-%d-%H:%M:%S.%f')
    except ValueError:
        pass
    else:
        return timestamp_as_datetime

    try:
        timestamp_as_datetime = datetime.strptime(timestamp, '%Y-%m-%d')
    except ValueError:
        pass
    else:
        return timestamp_as_datetime

    if timestamp == 'begin of time':
        try:
            timestamp_as_datetime = datetime.strptime('1900-01-01', '%Y-%m-%d')
        except ValueError:
            pass
        else:
            return timestamp_as_datetime

    if timestamp == 'end of time':
        try:
            timestamp_as_datetime = datetime.strptime('9999-12-31', '%Y-%m-%d')
        except ValueError:
            pass
        else:
            return timestamp_as_datetime


def parse_mpoint(list_expr: ListExp) -> MPoint:
    """
    Transforms a list expression object containing a moving point (mpoint) to a named tuple.

    :param list_expr: A list expression object containing a moving point (mpoint).
    :return: An object of the data class MPoint with the moving point.
    """

    number_of_intervals = list_expr.get_list_length()

    intervals = []
    for i in range(1, number_of_intervals):

        point_in_interval_as_le: ListExp = list_expr.get_the_n_element(i)

        # Parse interval

        interval_as_le: ListExp = point_in_interval_as_le.get_first_element()

        start_time = parse_timestamp(interval_as_le.get_first_element().value)
        end_time = parse_timestamp(interval_as_le.get_second_element().value)
        close_left = interval_as_le.get_third_element().value
        close_right = interval_as_le.get_fourth_element().value

        interval = Interval(start_time=start_time, end_time=end_time, close_left=close_left, close_right=close_right)

        # Parse motion vector

        motion_vector_as_le: ListExp = point_in_interval_as_le.get_second_element()

        x1 = motion_vector_as_le.get_first_element().value
        y1 = motion_vector_as_le.get_second_element().value
        x2 = motion_vector_as_le.get_third_element().value
        y2 = motion_vector_as_le.get_fourth_element().value

        motion_vector = MotionVector(x1=x1, y1=y1, x2=x2, y2=y2)

        point_in_interval = PointInInterval(interval, motion_vector)

        intervals.append(point_in_interval)

    return MPoint(intervals)


def parse_mregion(list_expr: ListExp) -> MRegion:
    """
    Transforms a list expression object containing a moving region (mregion) to a named tuple.

    :param list_expr: A list expression object containing a moving region (mregion).
    :return: An object of the data class MRegion with the moving region.
    """

    number_of_intervals = list_expr.get_list_length()

    intervals = []
    for i in range(1, number_of_intervals):

        region_in_interval_as_le: ListExp = list_expr.get_the_n_element(i)

        # Parse interval

        interval_as_le: ListExp = region_in_interval_as_le.get_first_element()

        start_time = parse_timestamp(interval_as_le.get_first_element().value)
        end_time = parse_timestamp(interval_as_le.get_second_element().value)
        close_left = interval_as_le.get_third_element().value
        close_right = interval_as_le.get_fourth_element().value

        interval = Interval(start_time=start_time, end_time=end_time, close_left=close_left, close_right=close_right)

        # Parse map faces

        map_faces_as_le: ListExp = region_in_interval_as_le.get_second_element()
        number_of_map_faces = map_faces_as_le.get_list_length()

        map_faces = []
        for j in range(1, number_of_map_faces):

            map_face_as_le: ListExp = map_faces_as_le.get_the_n_element(j)

            number_of_cycles = map_face_as_le.get_list_length() - 1

            if number_of_cycles == 1:

                # Parse outercycle

                outercycle_as_le: ListExp = map_face_as_le.get_first_element()
                number_of_map_points = outercycle_as_le.get_list_length()

                map_points = []
                for k in range(1, number_of_map_points):

                    map_point_as_le: ListExp = outercycle_as_le.get_the_n_element(k)
                    x1 = map_point_as_le.get_first_element().value
                    y1 = map_point_as_le.get_second_element().value
                    x2 = map_point_as_le.get_third_element().value
                    y2 = map_point_as_le.get_fourth_element().value
                    map_points.append(MapPoint(x1=x1, y1=y1, x2=x2, y2=y2))

                map_faces.append(spatial.Face(outercycle=map_points, holecycles=[]))

            elif number_of_cycles == 2:

                # Parse outercycle

                outercycle_as_le: ListExp = map_face_as_le.get_first_element()
                number_of_map_points = outercycle_as_le.get_list_length()

                outercycle = []
                for k in range(1, number_of_map_points):

                    map_point_as_le: ListExp = outercycle_as_le.get_the_n_element(k)
                    x1 = map_point_as_le.get_first_element().value
                    y1 = map_point_as_le.get_second_element().value
                    x2 = map_point_as_le.get_third_element().value
                    y2 = map_point_as_le.get_fourth_element().value
                    outercycle.append(MapPoint(x1=x1, y1=y1, x2=x2, y2=y2))

                # Parse holecycle

                holecycle_as_le: ListExp = map_face_as_le.get_second_element()
                number_of_map_points = holecycle_as_le.get_list_length()

                holecycle = []
                for k in range(1, number_of_map_points):
                    map_point = namedtuple('map_point', ['x1', 'y1', 'x2', 'y2'])
                    map_point_as_le: ListExp = holecycle_as_le.get_the_n_element(k)
                    map_point.x1 = map_point_as_le.get_first_element().value
                    map_point.y1 = map_point_as_le.get_second_element().value
                    map_point.x2 = map_point_as_le.get_third_element().value
                    map_point.y2 = map_point_as_le.get_fourth_element().value
                    holecycle.append(map_point)

                map_faces.append(spatial.Face(outercycle=outercycle, holecycles=[holecycle]))

            intervals.append(RegionInInterval(interval, map_faces))

    return MRegion(intervals)


def parse_mreal(list_expr) -> MReal:
    """
    Transforms a list expression object containing a moving real (mreal) to a named tuple.

    :param list_expr: A list expression object containing a moving real (mreal).
    :return: An object of the data class MReal with the moving real.
    """

    number_of_intervals = list_expr.get_list_length()

    intervals = []
    for i in range(1, number_of_intervals):

        real_in_interval_as_le: ListExp = list_expr.get_the_n_element(i)

        # Parse interval

        interval_as_le: ListExp = real_in_interval_as_le.get_first_element()

        start_time = parse_timestamp(interval_as_le.get_first_element().value)
        end_time = parse_timestamp(interval_as_le.get_second_element().value)
        close_left = interval_as_le.get_third_element().value
        close_right = interval_as_le.get_fourth_element().value

        interval = Interval(start_time=start_time, end_time=end_time, close_left=close_left, close_right=close_right)

        # Parse value vector

        motion_vector_as_le: ListExp = real_in_interval_as_le.get_second_element()

        start_value = motion_vector_as_le.get_second_element().value
        end_value = motion_vector_as_le.get_third_element().value
        boolean = motion_vector_as_le.get_fourth_element().value

        value_vector = ValueVectorReal(start_value=start_value, end_value=end_value, boolean=boolean)

        intervals.append(RealInInterval(interval=interval, value_vector=value_vector))

    return MReal(intervals=intervals)


def parse_mbool(list_expr: ListExp) -> MBool:
    """
    Transforms a list expression object containing a moving boolean (mbool) to a named tuple.

    :param list_expr: A list expression object containing a moving boolean (mbool).
    :return: An object of the data class MBool with the moving boolean.
    """

    number_of_intervals = list_expr.get_list_length()

    intervals = []
    for i in range(1, number_of_intervals):

        bool_in_interval_as_le: ListExp = list_expr.get_the_n_element(i)

        # Parse interval

        interval_as_le: ListExp = bool_in_interval_as_le.get_first_element()

        start_time = parse_timestamp(interval_as_le.get_first_element().value)
        end_time = parse_timestamp(interval_as_le.get_second_element().value)
        close_left = interval_as_le.get_third_element().value
        close_right = interval_as_le.get_fourth_element().value

        interval = Interval(start_time=start_time, end_time=end_time, close_left=close_left, close_right=close_right)

        # Parse value vector

        value_vector_as_le: ListExp = bool_in_interval_as_le.get_second_element()

        value = value_vector_as_le.value

        value_vector = ValueVectorBool(value=value)

        bool_in_interval = BoolInInterval(interval=interval, value_vector=value_vector)

        intervals.append(bool_in_interval)

    return MBool(intervals)


def parse_mstring(list_expr: ListExp) -> MString:
    """
    Transforms a list expression object containing a moving string (mstring) to a named tuple.

    :param list_expr: A list expression object containing a moving string (mstring).
    :return: An object of the data class MString with the moving string.
    """

    number_of_intervals = list_expr.get_list_length()

    intervals = []
    for i in range(1, number_of_intervals):

        string_in_interval_as_le: ListExp = list_expr.get_the_n_element(i)

        # Parse interval

        interval_as_le: ListExp = string_in_interval_as_le.get_first_element()

        start_time = parse_timestamp(interval_as_le.get_first_element().value)
        end_time = parse_timestamp(interval_as_le.get_second_element().value)
        close_left = interval_as_le.get_third_element().value
        close_right = interval_as_le.get_fourth_element().value

        interval = Interval(start_time=start_time, end_time=end_time, close_left=close_left, close_right=close_right)

        # Parse value vector

        value_vector_as_le: ListExp = string_in_interval_as_le.get_second_element()

        value_vector = ValueVectorString(value=value_vector_as_le.value)

        string_in_interval = StringInInterval(interval=interval, value_vector=value_vector)

        intervals.append(string_in_interval)

    return MString(intervals=intervals)


def parse_mint(list_expr: ListExp) -> MInt:
    """
    Transforms a list expression object containing a moving integer (mint) to a named tuple.

    :param list_expr: A list expression object containing a moving integer (mint).
    :return: An object of the data class MInt with the moving integer.
    """

    number_of_intervals = list_expr.get_list_length()

    intervals = []
    for i in range(1, number_of_intervals):

        int_in_interval_as_le: ListExp = list_expr.get_the_n_element(i)

        # Parse interval

        interval_as_le: ListExp = int_in_interval_as_le.get_first_element()

        start_time = parse_timestamp(interval_as_le.get_first_element().value)
        end_time = parse_timestamp(interval_as_le.get_second_element().value)
        close_left = interval_as_le.get_third_element().value
        close_right = interval_as_le.get_fourth_element().value

        interval = Interval(start_time=start_time, end_time=end_time, close_left=close_left, close_right=close_right)

        # Parse value vector

        value_vector_as_le: ListExp = int_in_interval_as_le.get_second_element()

        value_vector = ValueVectorInt(value_vector_as_le.value)

        int_in_interval = IntInInterval(interval=interval, value_vector=value_vector)

        intervals.append(int_in_interval)

    return MInt(intervals=intervals)


def parse_iregion(list_expr: ListExp) -> IRegion:
    """
    Transforms a list expression object containing an instant region (iregion) to a named tuple.

    :param list_expr: A list expression object containing an instant region (iregion).
    :return: An object of the data class IRegion with the instant region.
    """

    # Parse instant

    instant_as_le: ListExp = list_expr.get_first_element()
    instant = parse_timestamp(instant_as_le.value)

    # Parse region

    region_as_le: ListExp = list_expr.get_second_element()
    region = spatial.parse_region(region_as_le)

    return IRegion(instant=instant, region=region)


def parse_ipoint(list_expr: ListExp) -> IPoint:
    """
    Transforms a list expression object containing an instant point (ipoint) to a named tuple.

    :param list_expr: A list expression object containing an instant region (ipoint).
    :return: An object of the data class IPoint with the instant point.
    """

    # Parse instant

    instant_as_le: ListExp = list_expr.get_first_element()
    instant = parse_timestamp(instant_as_le.value)

    # Parse point

    point_as_le: ListExp = list_expr.get_second_element()
    point = spatial.parse_point(point_as_le)

    return IPoint(instant=instant, point=point)



