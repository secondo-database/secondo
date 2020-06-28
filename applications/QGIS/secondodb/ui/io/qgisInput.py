# ----------------------------------------------------------------------------------------------------------------------
# SecondoDB Plugin for QGIS
# Victor Silva (victor.silva@posteo.de)
# June 2020
# ----------------------------------------------------------------------------------------------------------------------
# Module
# ----------------------------------------------------------------------------------------------------------------------
# QGIS Input
# qgisInput.py
# ----------------------------------------------------------------------------------------------------------------------
"""
This module contains the methods to convert the internal representation of |sec| objects from the Python-API into QGIS
objects. The following objects are currently supported:

- Relational Algebra: Relations (with geometry - GeoData)
- Spatial Algebra: point, points, line, region
- Spatio-Temporal Algebra: mpoint, mregion

Every geometry is translated into a single feature of a QGIS vector layer. In case of a relation, the columns with
non-spatial related data will be translated into attributes of the created feature. The corresponding QGIS geometry type
is mapped as follows:

Spatial Algebra

point -> PointXY
points -> PointXY (array of PointXY through QGIS method addPointsXY)
line -> MultiPolyLineXY
line (from points) -> MultiPolyLineXY (through method fromPolyLineXY)
region -> MultiPolygonXY

Spatio-Temporal Algebra

mpoint -> PointXY (every frame is a feature of the layer)
mregion -> MultiPolygonXY (every frame is a feature of the layer)

Spatio-Temporal objects are created using common geometries (point, polygon). The frames for the animation are created
using the methods create_frames_for_mpoint() and create_frames_for_mregion() for a point and a region respectively,
using a frame rate which can be setted on function call (parameter frames_per_second). Every geometry frame corresponds
to a feature with a timestamp, which can be animated in QGIS using the plugin TimeManager.
"""


from collections import namedtuple
from datetime import timedelta, datetime
from qgis._gui import QgisInterface
from PyQt5.QtCore import QVariant
from PyQt5.QtGui import QFont, QColor
from qgis import processing
from qgis.core import QgsGeometry, QgsVectorLayer, QgsField, QgsPointXY, QgsFeature, QgsPalLayerSettings, QgsTextFormat, \
    QgsTextBufferSettings, QgsVectorLayerSimpleLabeling

from secondodb.api.algebras.secondospatiotemporalalgebra import MPoint, MRegion
from secondodb.api.algebras.secondospatialalgebra import Point, Region, Line

SPATIAL_TYPES = ['point', 'points', 'line', 'region']
SPATIOTEMPORAL_TYPES = ['mpoint', 'mregion']
MAX_FEATURES_DISPLAY = 2000

CONST_POINT = 'Point'
CONST_LINESTRING = 'LineString'
CONST_POLYGON = 'Polygon'
CONST_MEMORY = 'memory'


def interpolate_point(timestamp, start_time, end_time, x1, y1, x2, y2) -> Point:
    """
    Calculates the point for a given timestamp, given the segment between two points in two different instants (start
    and end time).

    :param timestamp: The instant for the calculation of the point.
    :param start_time: The start time.
    :param end_time: The end time.
    :param x1: X-coordinate of the start point of the segment.
    :param y1: Y-coordinate of the start point of the segment.
    :param x2: X-coordinate of the end point of the segment.
    :param y2: Y-coordinate of the end point of the segment.
    :return: A point object.
    """

    delta_time = end_time - start_time
    millisec = delta_time.total_seconds() * 1000

    delta_x = x2 - x1
    delta_y = y2 - y1

    vel_x = delta_x / millisec
    vel_y = delta_y / millisec

    time_position = timestamp - start_time
    time_position = time_position.total_seconds() * 1000

    new_x = vel_x * time_position + x1
    new_y = vel_y * time_position + y1

    # point = namedtuple('point', ['x', 'y'])
    # point.x = new_x
    # point.y = new_y

    return Point(x=new_x, y=new_y)


def create_frames_for_mpoint(mpoint_object: MPoint, frames_per_second=1) -> []:
    """
    Creates the frames for a moving point, given a frame per second (FPS) rate.

    :param mpoint_object: The mpoint object.
    :param frames_per_second: The frame rate.
    :return: A list with the frames.
    """

    row_count = len(mpoint_object.intervals)

    start_time = mpoint_object.intervals[0].interval.start_time
    end_time = mpoint_object.intervals[row_count - 1].interval.end_time

    time_delta_for_frames = 1 / int(frames_per_second) * 1000

    time_frame = timedelta(milliseconds=time_delta_for_frames)
    current_time = start_time

    current_interval_end_time = mpoint_object.intervals[0].interval.end_time

    points = []
    counter = 0
    int_id = 1
    while current_time < end_time:
        if current_time >= current_interval_end_time:
            counter += 1

        current_interval_start_time = \
            mpoint_object.intervals[counter].interval.start_time
        current_interval_end_time = \
            mpoint_object.intervals[counter].interval.end_time

        current_interval_x1 = float(mpoint_object.intervals[counter].motion_vector.x1)
        current_interval_y1 = float(mpoint_object.intervals[counter].motion_vector.y1)
        current_interval_x2 = float(mpoint_object.intervals[counter].motion_vector.x2)
        current_interval_y2 = float(mpoint_object.intervals[counter].motion_vector.y2)

        new_point = interpolate_point(current_time,
                                      current_interval_start_time,
                                      current_interval_end_time,
                                      current_interval_x1,
                                      current_interval_y1,
                                      current_interval_x2,
                                      current_interval_y2)

        points.append([new_point, current_time, int_id])
        current_time += time_frame
        int_id += 1

    return points


def create_frames_for_mregion(mregion_object: MRegion, frames_per_second=1) -> []:
    """
    Creates the frames for a moving region, given a frame per second (FPS) rate.

    :param mregion_object: The mregion object.
    :param frames_per_second: The frame rate.
    :return: A list with the frames.
    """

    row_count = len(mregion_object.intervals)

    start_time = mregion_object.intervals[0].interval.start_time
    end_time = mregion_object.intervals[row_count - 1].interval.end_time

    time_delta_for_frames = 1 / int(frames_per_second) * 1000

    time_frame = timedelta(milliseconds=time_delta_for_frames)
    current_time = start_time

    current_interval_end_time = mregion_object.intervals[0].interval.end_time

    regions = []
    counter = 0
    int_id = 1
    while current_time < end_time:
        if current_time >= current_interval_end_time:
            counter += 1

        current_interval_start_time = \
            mregion_object.intervals[counter].interval.start_time
        current_interval_end_time = \
            mregion_object.intervals[counter].interval.end_time

        region = namedtuple('region', ['faces'])

        faces = []
        for map_face in mregion_object.intervals[counter].map_faces:

            face = namedtuple('face', ['outercycle', 'holecycles'])

            outercycle = []
            for map_point in map_face.outercycle:

                current_interval_x1 = float(map_point.x1)
                current_interval_y1 = float(map_point.y1)
                current_interval_x2 = float(map_point.x2)
                current_interval_y2 = float(map_point.y2)

                new_point = interpolate_point(current_time,
                                              current_interval_start_time,
                                              current_interval_end_time,
                                              current_interval_x1,
                                              current_interval_y1,
                                              current_interval_x2,
                                              current_interval_y2)

                outercycle.append(new_point)

            holecycles = []
            for single_holecycle in map_face.holecycles:

                holecycle = []
                for map_point in single_holecycle:
                    current_interval_x1 = float(map_point.x1)
                    current_interval_y1 = float(map_point.y1)
                    current_interval_x2 = float(map_point.x2)
                    current_interval_y2 = float(map_point.y2)

                    new_point = interpolate_point(current_time,
                                                  current_interval_start_time,
                                                  current_interval_end_time,
                                                  current_interval_x1,
                                                  current_interval_y1,
                                                  current_interval_x2,
                                                  current_interval_y2)

                    holecycle.append(new_point)

                holecycles.append(holecycle)

            face.outercycle = outercycle
            face.holecycles = holecycles
            faces.append(face)

        region.faces = faces
        regions.append([region, current_time, int_id])

        current_time += time_frame
        int_id += 1

    return regions


def create_layer_for_mpoint_as_points_sequence(layer_name: str,
                                               relation_tuples: [],
                                               relation_fields: [],
                                               frames_per_second=1) -> QgsVectorLayer:
    """
    Creates a new vector layer for a moving point as a sequence of points. The points will be generated considering a
    frame rate. The frames can be later animated with the QGIS plugin TimeManager.

    :param layer_name: The name of the layer.
    :param relation_tuples: The tuples of the relation.
    :param relation_fields: The fields of the relation.
    :param frames_per_second: Frames per second for the generation of the frames.
    :return: The layer.
    """

    features = []

    layer = QgsVectorLayer(CONST_POINT, layer_name, CONST_MEMORY)

    layer_attributes = []
    for attribute in relation_fields:
        field = QgsField(attribute.attribute_name, QVariant.String)
        layer_attributes.append(field)

    provider = layer.dataProvider()
    provider.addAttributes(layer_attributes)
    layer.updateFields()

    frames_for_mpoint = create_frames_for_mpoint(relation_tuples[1], frames_per_second=frames_per_second)

    for frame in frames_for_mpoint:

        item_data_x = frame[0].x
        item_data_y = frame[0].y

        point_geometry = QgsPointXY(float(item_data_x), float(item_data_y))
        geometry = QgsGeometry.fromPointXY(point_geometry)

        feature = QgsFeature()
        feature.setGeometry(geometry)

        feature.setAttributes([relation_tuples[0], datetime.strftime(frame[1], '%Y-%m-%d %H:%M:%S.%f'),
                              str(item_data_x), str(item_data_y)])

        features.append(feature)

    provider.addFeatures(features)

    layer.updateExtents()

    return layer


def create_layer_for_mregion_as_regions_sequence(layer_name: str,
                                                 relation_tuples: [],
                                                 relation_fields: [],
                                                 frames_per_second=1) -> QgsVectorLayer:
    """
    Creates a new vector layer for a moving region as a sequence of polygons. The polygons will be generated considering
    a frame rate. The frames can be later animated with the QGIS plugin TimeManager.

    :param layer_name: The name of the layer.
    :param relation_tuples: The tuples of the relation.
    :param relation_fields: The fields of the relation.
    :param frames_per_second: Frames per second for the generation of the frames.
    :return: The layer.
    """

    features = []

    layer = QgsVectorLayer(CONST_POLYGON, layer_name, CONST_MEMORY)

    layer_attributes = []
    for attribute in relation_fields:
        field = QgsField(attribute.attribute_name, QVariant.String)
        layer_attributes.append(field)

    provider = layer.dataProvider()
    provider.addAttributes(layer_attributes)
    layer.updateFields()

    frames_for_mregion = create_frames_for_mregion(relation_tuples[1], frames_per_second=frames_per_second)

    for frame in frames_for_mregion:

        region_geometry = create_geometry_for_region(frame[0])

        feature = QgsFeature()
        feature.setGeometry(region_geometry)

        feature.setAttributes([relation_tuples[0], datetime.strftime(frame[1], '%Y-%m-%d %H:%M:%S.%f')])
        features.append(feature)

    provider.addFeatures(features)

    layer.updateExtents()

    return layer


def create_layer_for_mpoint_as_linestring(layer_name: str,
                                          relation_tuples: [],
                                          relation_fields: []) -> QgsVectorLayer:
    """
    Creates a vector layer for a moving point with a line string geometry. This can be used to display the trajectory
    of the moving point.

    :param layer_name: The name of the layer.
    :param relation_tuples: The tuples of the relation.
    :param relation_fields: The fields of the relation.
    :return: The layer.
    """

    features = []

    layer = QgsVectorLayer(CONST_LINESTRING, layer_name, CONST_MEMORY)

    layer_attributes = []
    for attribute in relation_fields:
        field = QgsField(attribute.attribute_name, QVariant.String)
        layer_attributes.append(field)

    provider = layer.dataProvider()
    provider.addAttributes(layer_attributes)
    layer.updateFields()

    for interval in relation_tuples[1].intervals:

        item_data_x1 = interval.motion_vector.x1
        item_data_y1 = interval.motion_vector.y1
        item_data_x2 = interval.motion_vector.x2
        item_data_y2 = interval.motion_vector.y2

        start_point = QgsPointXY(float(item_data_x1), float(item_data_y1))
        end_point = QgsPointXY(float(item_data_x2), float(item_data_y2))
        geometry = QgsGeometry.fromPolylineXY([start_point, end_point])

        feature = QgsFeature()
        feature.setGeometry(geometry)
        feature.setAttributes([relation_tuples[0],
                              datetime.strftime(interval.interval.start_time, '%Y-%m-%d %H:%M:%S.%f'),
                              datetime.strftime(interval.interval.end_time, '%Y-%m-%d %H:%M:%S.%f'),
                              str(item_data_x1), str(item_data_y1), str(item_data_x2), str(item_data_y2)])

        features.append(feature)

    provider.addFeatures(features)
    layer.updateExtents()

    return layer


def create_new_vector_layer_for_relation(layer_name: str,
                                         relation_tuples: [],
                                         relation_fields: [],
                                         geometry_type: str,
                                         with_labels: bool = False) -> QgsVectorLayer:
    """
    Creates a new vector layer for a relation.

    :param layer_name: The name of the layer.
    :param relation_tuples: The tuples of the relation.
    :param relation_fields: The fields of the relation.
    :param geometry_type: The geometry type.
    :param with_labels: Set True, if labels should be displayed along the geometry.
    :return: The layer.
    """

    layer = None
    features = []
    spatial_attributes = []
    other_attributes = []
    index = 0
    attr_indexes = []
    spatial_index = 0

    if geometry_type == 'point' or geometry_type == 'points':
        layer = QgsVectorLayer(CONST_POINT, layer_name, CONST_MEMORY)
    elif geometry_type == 'line':
        layer = QgsVectorLayer(CONST_LINESTRING, layer_name, CONST_MEMORY)
    elif geometry_type == 'region':
        layer = QgsVectorLayer(CONST_POLYGON, layer_name, CONST_MEMORY)
    else:  # Geometry type is none
        layer = QgsVectorLayer(CONST_POINT, layer_name, CONST_MEMORY)

    for attribute in relation_fields:
        if attribute.attribute_type in SPATIAL_TYPES:
            spatial_attributes.append(attribute)
            spatial_index = index  # Position of the geometry object
        else:
            other_attributes.append(attribute)
            attr_indexes.append(index)  # Positions of the other attributes in the tuple
        index += 1

    layer_attributes = []
    for attribute in other_attributes:
        field = QgsField(attribute.attribute_name, QVariant.String)
        layer_attributes.append(field)

    provider = layer.dataProvider()
    provider.addAttributes(layer_attributes)
    layer.updateFields()

    geometry = None

    for single_object in relation_tuples:

        if geometry_type == 'point':
            geometry = create_geometry_for_point(single_object[spatial_index])
        elif geometry_type == 'points':
            geometry = create_geometry_for_points(single_object[spatial_index])
        elif geometry_type == 'line':
            geometry = create_geometry_for_line(single_object[spatial_index])
        elif geometry_type == 'region':
            geometry = create_geometry_for_region(single_object[spatial_index])

        relation_fields = []
        for index in attr_indexes:
            relation_fields.append(str(single_object[index]))

        feature = QgsFeature()
        if geometry is not None:
            feature.setGeometry(geometry)
        feature.setAttributes(relation_fields)
        features.append(feature)

    provider.addFeatures(features)
    layer.updateExtents()

    # Set labels, if activated - it doesn't work with regions though...

    if with_labels:

        layer_settings = QgsPalLayerSettings()
        text_format = QgsTextFormat()
        text_format.setFont(QFont('Arial', 10))
        text_format.setSize(10)

        buffer_settings = QgsTextBufferSettings()
        buffer_settings.setEnabled(True)
        buffer_settings.setSize(1)
        buffer_settings.setColor(QColor("white"))

        text_format.setBuffer(buffer_settings)
        layer_settings.setFormat(text_format)

        layer_settings.fieldName = other_attributes[0].attribute_name

        layer_settings.placement = 4

        layer_settings.enabled = True

        layer_settings = QgsVectorLayerSimpleLabeling(layer_settings)
        layer.setLabelsEnabled(True)
        layer.setLabeling(layer_settings)

        layer.updateExtents()

    return layer


def merge_multilines_of_layer(layer: QgsVectorLayer, qgis_interface: QgisInterface) -> QgsVectorLayer:
    """
    Merge a vector layer with a MultiLineString geometry to a LineString geometry calling the processing toolbox of the
    QGIS application. Please notice that the QGIS application must be previously initialized before the execution of the
    processing algorithm. The QGIS application initializes a singleton instance, which can be reached through the
    interface object. Other components, like the processing toolbox, are available after initialization as well.

    Be aware that segments, which are not connected to others, will be visualized as standalone features in the new
    layer and cannot be merged.

    :param layer: The layer to be merged.
    :param qgis_interface: The QGIS interface object.
    :return: The merged layer.
    """

    if qgis_interface is not None:
        try:
            merged_layer = processing.run("native:mergelines", {'INPUT': layer, 'OUTPUT': 'TEMPORARY_OUTPUT'})
        except:
            pass
        else:
            layer = merged_layer['OUTPUT']
    else:
        print('QGIS is not initialized')
    return layer


def create_geometry_for_point(point_object: Point) -> QgsGeometry:
    """
    Creates a geometry object for a point object to be inserted in a vector layer.

    :param point_object: The point object.
    :return: The geometry object.
    """

    point_x = point_object.x
    point_y = point_object.y
    qgis_point = QgsPointXY(float(point_x), float(point_y))
    geometry = QgsGeometry.fromPointXY(qgis_point)

    return geometry


def create_geometry_for_points(points_object: []) -> QgsGeometry:
    """
    Creates a geometry object for a points object to be inserted in a vector layer.

    :param points_object: The points object.
    :return: The geometry object.
    """

    geometry = QgsGeometry.fromMultiPointXY([])
    for point in points_object:
        point_x = point.x
        point_y = point.y
        single_point = QgsPointXY(float(point_x), float(point_y))
        geometry.addPointsXY([single_point])

    return geometry


def create_geometry_for_line(line_object: Line) -> QgsGeometry:
    """
    Creates a geometry object for a line object to be inserted in a vector layer.

    :param line_object: The line object.
    :return: The geometry objects.
    """

    segments = line_object.segments

    multi_lines = []
    for segment in segments:
        point_x1 = segment.x1
        point_y1 = segment.y1
        point_x2 = segment.x2
        point_y2 = segment.y2
        start_point = QgsPointXY(float(point_x1), float(point_y1))
        end_point = QgsPointXY(float(point_x2), float(point_y2))
        multi_lines.append([start_point, end_point])

    geometry = QgsGeometry.fromMultiPolylineXY(multi_lines)

    return geometry


def create_geometry_for_line_from_points(line_object: Line) -> QgsGeometry:
    """
    Creates a geometry object for a line object to be inserted in a vector layer.

    :param line_object: The line object.
    :return: The geometry objects.
    """

    segments = line_object.segments

    length_segments = len(segments)

    points = []
    for i in range(length_segments):
        point_x = segments[i].x1
        point_y = segments[i].y1
        point = QgsPointXY(float(point_x), float(point_y))
        points.append(point)

    point_x = segments[length_segments - 1].x2
    point_y = segments[length_segments - 1].y2
    end_point = QgsPointXY(float(point_x), float(point_y))

    points.append(end_point)

    geometry = QgsGeometry.fromPolylineXY(points)

    return geometry


def create_geometry_for_region(region_object: Region) -> QgsGeometry:
    """
    Creates a geometry object for a region object to be inserted in a vector layer.

    :param region_object: The region object.
    :return: The geometry object.
    """

    faces = region_object.faces

    geo_multipolygon = QgsGeometry.fromMultiPolygonXY([])

    for face in faces:

        face_list = []

        outercycle = face.outercycle
        holecycles = face.holecycles

        points_outercycle = []
        for point in outercycle:
            point_x = point.x
            point_y = point.y
            point_object = QgsPointXY(float(point_x), float(point_y))
            points_outercycle.append(point_object)

        face_list.append(points_outercycle)

        for holecycle in holecycles:
            points_holecycle = []
            for point in holecycle:
                point_x = point.x
                point_y = point.y
                point_object = QgsPointXY(float(point_x), float(point_y))
                points_holecycle.append(point_object)
            face_list.append(points_holecycle)

        # geo_polygon = QgsGeometry.fromPolygonXY([points_outercycle, points_holecycles])

        geo_polygon = QgsGeometry.fromPolygonXY(face_list)
        geo_multipolygon.addPartGeometry(geo_polygon)

    return geo_multipolygon

