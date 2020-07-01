# ----------------------------------------------------------------------------------------------------------------------
# The Secondo Python API (pySecondo)
# Victor Silva (victor.silva@posteo.de)
# January 2020
# ----------------------------------------------------------------------------------------------------------------------
# Module
# ----------------------------------------------------------------------------------------------------------------------
# Secondo Spatial Algebra
# secondospatialalgebra.py
# ----------------------------------------------------------------------------------------------------------------------
"""
The module Secondo Spatial Algebra implements the data types for the conversion of list expression objects with values
of the types contained in the SpatialAlgebra of the |sec| system. The data types are implemented in Python
using Data Classes. Data Classes are implemented in the API like normal classes without behaviour.
Like regular classes their attributes can be called through the given names.
"""

from dataclasses import dataclass
from secondodb.api.support.secondolistexpr import ListExp


@dataclass
class Segment:
    """
    Implements a single segment of a line.
    """

    __slots__ = ["x1", "y1", "x2", "y2"]

    x1: float
    y1: float
    x2: float
    y2: float


@dataclass
class Point:
    """
    Implements a single point expressing two coordinates X and Y.
    """

    __slots__ = ["x", "y"]

    x: float
    y: float


@dataclass
class Line:
    """
    Implements a line as a list of single segments of the class Segment.
    """

    __slots__ = ["segments"]

    segments: []


@dataclass
class Region:
    """
    Implements a region as a list of faces of the class Face.
    """

    __slots__ = ["faces"]

    faces:  []


@dataclass
class Face:
    """
    Implements a single face of a region, containing an outercycle (list of single points) and holecycles (a list of
    cycles, where each cycle is a list of points).
    """

    __slots__ = ["outercycle", "holecycles"]

    outercycle: []
    holecycles: []


def convert_point_to_list_exp_str(point: Point) -> str:
    """
    Converts a point object to a nested list in string format.

    :param point: A point object.
    :return: The nested list as string.
    """

    point: Point

    list_exp_str = '(' \
                   + str(point.x) \
                   + ' ' \
                   + str(point.y) \
                   + ')'
    return list_exp_str


def convert_points_to_list_exp_str(points: []) -> str:
    """
    Converts a points object to a nested list in string format.

    :param points: A points object.
    :return: The nested list as string.
    """

    list_exp_str = '('

    point_count = 1
    for point in points:
        point_le = convert_point_to_list_exp_str(point)
        list_exp_str = list_exp_str + point_le
        if point_count < len(points):
            list_exp_str = list_exp_str + ' '
        else:
            list_exp_str = list_exp_str + ')'
        point_count += 1

    return list_exp_str


def convert_line_to_list_exp_str(line: Line) -> str:
    """
    Converts a line object to a nested list in string format.

    :param line: A line object.
    :return: The nested list as string.
    """

    line: Line

    list_exp_str = '('
    segments = line.segments

    segment_count = 1
    for segment in segments:
        segment_str = '(' \
                      + str(segment.x1) \
                      + ' ' \
                      + str(segment.y1) \
                      + ' ' \
                      + str(segment.x2) \
                      + ' ' \
                      + str(segment.y2) \
                      + ')'
        list_exp_str = list_exp_str + segment_str
        if segment_count < len(segments):
            list_exp_str = list_exp_str + ' '
        else:
            list_exp_str = list_exp_str + ')'
        segment_count += 1

    return list_exp_str


def convert_region_to_list_exp_str(region: Region) -> str:
    """
    Converts a region object to a nested list in string format.

    :param region: A region object.
    :return: The nested list as string.
    """

    region: Region

    face_str = ''
    list_exp_str = '('

    face_count = 1
    for face in region.faces:
        face_str = '('

        outercycle_str = '('
        point_count = 1
        for point in face.outercycle:

            point_str = convert_point_to_list_exp_str(point)
            outercycle_str = outercycle_str + point_str
            if point_count < len(face.outercycle):
                outercycle_str = outercycle_str + ' '
            else:
                outercycle_str = outercycle_str + ')'
            point_count += 1

        face_str = face_str + outercycle_str

        if len(face.holecycles) > 0:

            holecycle_count = 1
            for holecycle in face.holecycles:

                face_str = face_str + ' '

                holecycle_str = '('
                point_count = 1
                for point in holecycle:
                    point_str = convert_point_to_list_exp_str(point)
                    holecycle_str = holecycle_str + point_str
                    if point_count < len(holecycle):
                        holecycle_str = holecycle_str + ' '
                    else:
                        holecycle_str = holecycle_str + ')'
                    point_count += 1

                if holecycle_count < len(face.holecycles):
                    face_str = face_str + holecycle_str + ' '
                else:
                    face_str = face_str + holecycle_str + ')'

                holecycle_count += 1

        else:  # no holes
            face_str = face_str + ')'

        if face_count < len(region.faces):
            list_exp_str = list_exp_str + face_str + ' '
        else:
            list_exp_str = list_exp_str + face_str + ')'

        face_count += 1

    return list_exp_str


def parse_point(list_expr: ListExp) -> Point:
    """
    Transforms a list expression object containing a point (point) to a named tuple.

    :param list_expr: A list expression object containing a point (point).
    :return: A named tuple with the point.
    """

    x = list_expr.get_first_element().value
    y = list_expr.get_second_element().value

    return Point(x=x, y=y)


def parse_points(list_expr: ListExp) -> []:
    """
    Transforms a list expression object containing points (points) to a named tuple.

    :param list_expr: A list expression object containing points (points).
    :return: A named tuple with the points.
    """
    points = []

    next_element = list_expr
    while next_element.next is not None:
        point = parse_point(next_element.value)
        points.append(point)
        next_element = next_element.next

    return points


def parse_line(list_expr: ListExp) -> Line:
    """
    Transforms a list expression object containing a line (line) to a named tuple.

    :param list_expr: A list expression object containing a line (line).
    :return: A named tuple with the line.
    """

    segments = []
    length = list_expr.get_list_length()

    for i in range(1, length):
        segment = parse_segment(list_expr.get_the_n_element(i))
        segments.append(segment)

    return Line(segments)


def parse_region(list_expr: ListExp) -> Region:
    """
    Transforms a list expression object containing a region (region) to a named tuple.

    :param list_expr: A list expression object containing a region (region).
    :return: A named tuple with the region.
    """

    qty_faces = list_expr.get_list_length()

    faces = []
    for i in range(1, qty_faces):

        face_le: ListExp = list_expr.get_the_n_element(i)

        cycles_count = face_le.get_list_length()

        outercycle_le = face_le.get_first_element()

        outercycle = []
        if outercycle_le is not None:
            outercycle = parse_points(outercycle_le)

        holecycles = []
        if cycles_count > 1:

            for j in range(2, cycles_count):

                holecycle_le = face_le.get_the_n_element(j)

                holecycle = []
                if holecycle_le is not None:
                    holecycle = parse_points(holecycle_le)

                holecycles.append(holecycle)

        face = Face(outercycle, holecycles)

        faces.append(face)

    return Region(faces)


def parse_segment(list_expr: ListExp) -> Segment:
    """
    Transforms a list expression object containing a segment (segment) to a named tuple.

    :param list_expr: A list expression object containing a segment (segment).
    :return: A named tuple with the segment.
    """

    x1 = list_expr.get_first_element().value
    y1 = list_expr.get_second_element().value
    x2 = list_expr.get_third_element().value
    y2 = list_expr.get_fourth_element().value

    return Segment(x1=x1, y1=y1, x2=x2, y2=y2)

