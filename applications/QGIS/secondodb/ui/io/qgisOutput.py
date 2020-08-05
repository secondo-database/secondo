# ----------------------------------------------------------------------------------------------------------------------
# The Secondo Python API (pySecondo)
# Victor Silva (victor.silva@posteo.de)
# June 2020
# ----------------------------------------------------------------------------------------------------------------------
# Module
# ----------------------------------------------------------------------------------------------------------------------
# QGIS Output
# qgisOutput.py
# ----------------------------------------------------------------------------------------------------------------------
"""
The module QGIS Output implements methods to convert QGIS geometry objects and features into the internal
representations of the |sec| objects and to call and process objects from the currently active QGIS environment.

Following QGIS objects are currently supported for the conversion into |sec| objects:

Point (QgsPointXY) -> point
MultiPoint (QgsMultiPoint) -> points
PolyLine (QgsLineString) -> line
MultiPolyLine (QgsMultiLineString) -> line
Polygon (QgsPolygon) -> region
MultiPolygon (QgsMultiPolygon) -> region

The module contains further methods to read and collect information from the active layers of the current QGIS project.

"""

from collections import namedtuple

from qgis.core import QgsMultiPoint, QgsPointXY, QgsLineString, QgsMultiLineString, QgsPolygon, QgsMultiPolygon, \
    QgsMapLayer, QgsVectorLayer, QgsProject, QgsWkbTypes
from qgis._gui import QgisInterface

SPATIAL_TYPES = ['point', 'points', 'line', 'region']
SPATIOTEMPORAL_TYPES = ['mpoint', 'mregion']
MAX_FEATURES_DISPLAY = 2000

CONST_POINT = 'Point'
CONST_LINESTRING = 'LineString'
CONST_POLYGON = 'Polygon'
CONST_MEMORY = 'memory'


def convert_multipoint_to_points(multipoints: QgsMultiPoint) -> []:
    """
    Converts a QgsMultiPoint object into an internal points list (SecondoDB API).

    :param multipoints: A QgsMultiPoint object.
    :return: A list of points.
    """

    points = []

    single_point: QgsPointXY
    for single_point in multipoints:
        point = namedtuple('point', ['x', 'y'])
        point.x = single_point.x()
        point.y = single_point.y()
        points.append(point)

    return points


def convert_point_to_point(single_point: QgsPointXY):
    """
    Converts a QgsPointXY object into an internal point object (SecondoDB API).

    :param single_point: A QgsPointXY object.
    :return: A point.
    """

    point = namedtuple('point', ['x', 'y'])
    point.x = single_point.x()
    point.y = single_point.y()

    return point


def convert_polyline_to_line(polyline: QgsLineString):
    """
    Converts a QhsLineString object into an internal line object.

    :param polyline: A QgsLineString.
    :return: A line.
    """
    line = namedtuple('line', ['segments'])
    line.segments = []

    points = []
    for point in polyline:
        points.append(point)

    length_points = len(points)

    for i in range(length_points - 1):

        segment = namedtuple('segment', ['x1', 'y1', 'x2', 'y2'])
        segment.x1 = points[i].x()
        segment.y1 = points[i].y()
        segment.x2 = points[i + 1].x()
        segment.y2 = points[i + 1].y()
        line.segments.append(segment)

    return line


def convert_multipolyline_to_line(multipolyline: QgsMultiLineString) -> []:
    """
    Converts a QgsMultiLineString object into an internal line object (SecondoDB API).

    :param multipolyline: A QgsMultiLineString object.
    :return: A line.
    """

    line = namedtuple('line', ['segments'])
    line.segments = []

    for polyline in multipolyline:

        points = []
        for point in polyline:
            points.append(point)

        length_points = len(points)

        for i in range(length_points - 1):
            segment = namedtuple('segment', ['x1', 'y1', 'x2', 'y2'])
            segment.x1 = points[i].x()
            segment.y1 = points[i].y()
            segment.x2 = points[i + 1].x()
            segment.y2 = points[i + 1].y()
            line.segments.append(segment)

    return line


def convert_polygon_to_region(polygon: QgsPolygon):
    """
    Converts a QgsPolygon object into a region object (SecondoDB API).

    :param polygon: A QgsPolygon object.
    :return: A region object (SecondoDB API).
    """
    region = namedtuple('region', ['faces'])
    faces = []

    face = namedtuple('face', ['outercycle', 'holecycles'])

    cycle_count = len(polygon)

    if cycle_count == 1:

        outercycle = []
        for point in polygon[0]:
            point_tuple = namedtuple('point', ['x', 'y'])

            point_x = point.x()
            point_y = point.y()

            point_tuple.x = point_x
            point_tuple.y = point_y

            outercycle.append(point_tuple)

        face.outercycle = outercycle
        face.holecycles = []

    elif cycle_count == 2:

        outercycle = []
        for point in polygon[0]:
            point_tuple = namedtuple('point', ['x', 'y'])

            point_x = point.x()
            point_y = point.y()

            point_tuple.x = point_x
            point_tuple.y = point_y

            outercycle.append(point_tuple)

        holecycle = []
        for point in polygon[1]:
            point_tuple = namedtuple('point', ['x', 'y'])

            point_x = point.x()
            point_y = point.y()

            point_tuple.x = point_x
            point_tuple.y = point_y

            holecycle.append(point_tuple)

        face.outercycle = outercycle
        face.holecycles = holecycle

    faces.append(face)

    region.faces = faces

    return region


def convert_multi_polygon_to_region(multi_polygon: QgsMultiPolygon):
    """
    Converts a QgsMultiPolygon object into a region object (SecondoDB API).

    :param multi_polygon: A QgsMultiPolygon object.
    :return: A region object (SecondoDB API).
    """
    region = namedtuple('region', ['faces'])
    faces = []

    for polygon in multi_polygon:

        face = namedtuple('face', ['outercycle', 'holecycles'])

        cycle_count = len(polygon)

        outercycle = []
        holecycles = []
        for point in polygon[0]:
            point_tuple = namedtuple('point', ['x', 'y'])

            point_x = point.x()
            point_y = point.y()

            point_tuple.x = point_x
            point_tuple.y = point_y

            outercycle.append(point_tuple)

        if cycle_count > 1:  # with holes

            for i in range(1, cycle_count):

                holecycle = []
                for point in polygon[i]:
                    point_tuple = namedtuple('point', ['x', 'y'])

                    point_x = point.x()
                    point_y = point.y()

                    point_tuple.x = point_x
                    point_tuple.y = point_y

                    holecycle.append(point_tuple)

                holecycles.append(holecycle)

        face.outercycle = outercycle
        face.holecycles = holecycles

        faces.append(face)

    region.faces = faces

    return region


def get_active_layer_name(qgis_interface: QgisInterface):
    """
    Retrieves the name of the active layer in the QGIS project.

    :param qgis_interface: The QGIS interface object.
    :return: The name of the layer.
    """
    layer = qgis_interface.activeLayer()
    return layer.name()


def get_geometry_type_of_active_layer(qgis_interface: QgisInterface):
    """
    Retrieves the geometry type of the active layer of the QGIS project.

    :param qgis_interface: The QGIS interface object.
    :return: The geometry type as string.
    """

    geometry_type_str = ""

    layer: QgsVectorLayer = qgis_interface.activeLayer()
    geometry_type = layer.geometryType()

    if geometry_type == 0:
        geometry_type_str = "Point"
    elif geometry_type == 1:
        geometry_type_str = "Line"
    elif geometry_type == 2:
        geometry_type_str = "Region"

    return geometry_type_str


def get_wkb_type_of_active_layer(qgis_interface: QgisInterface):
    """
    Retrieves the WKB type from the active layer, which describes the geometry type of the layer.

    :param qgis_interface: The QGIS interface object.
    :return: A string with the WKB type.
    """
    layer: QgsVectorLayer = qgis_interface.activeLayer()
    wkb_type = QgsWkbTypes.displayString(layer.wkbType())
    return wkb_type


def get_geometry_type_for_layer_name(qgis_interface: QgisInterface, layer_name: str):
    """
    Retrieves the geometry type for a given layer.

    :param qgis_interface: The QGIS interface object.
    :param layer_name: The name of the layer.
    :return: The geometry type as string.
    """

    layer = QgsProject.instance().mapLayersByName(layer_name)[0]
    try:
        geometry_type = layer.geometryType()
    except:
        geometry_type = None

    if geometry_type == 0:
        return "Point"
    elif geometry_type == 1:
        return "Line"
    elif geometry_type == 2:
        return "Region"


def get_pending_fields_of_active_layer(qgis_interface: QgisInterface):
    """
    Retrieves the fields of the active layer of the QGIS project.

    :param qgis_interface: The QGIS interface object.
    :return: The fields object.
    """
    layer: QgsVectorLayer = qgis_interface.activeLayer()
    return layer.fields()


def get_pending_fields_for_layer_name(qgis_interface: QgisInterface, layer_name: str):
    """
    Retrieves the fields for a given layer.

    :param qgis_interface: The QGIS interface object.
    :param layer_name: The name of the layer
    :return: The fields object.
    """

    layer = QgsProject.instance().mapLayersByName(layer_name)[0]
    return layer.fields()


def get_selected_features_of_active_layer(qgis_interface: QgisInterface):
    """
    Retrieves the selected features of the active layer of the QGIS project.

    :param qgis_interface: The QGIS interface object.
    :return: An iterator with the selected features.
    """
    layer: QgsVectorLayer = qgis_interface.activeLayer()
    return layer.getSelectedFeatures()


def get_all_features_of_active_layer(qgis_interface: QgisInterface):
    """
    Retrieves the selected features of the active layer of the QGIS project.

    :param qgis_interface: The QGIS interface object.
    :return: An iterator with the selected features.
    """
    layer: QgsVectorLayer = qgis_interface.activeLayer()
    print("getting all!!")
    return layer.getFeatures()


def get_features_for_layer_name(qgis_interface: QgisInterface, layer_name: str):
    """
    Retrieves the features for a given layer of the QGIS project.

    :param qgis_interface: The QGIS interface object.
    :param layer_name: The name of the layer.
    :return: An iterator with the features.
    """

    layer = QgsProject.instance().mapLayersByName(layer_name)[0]

    feat_count = get_features_count_for_layer_name(qgis_interface, layer_name)

    features = []

    if feat_count <= MAX_FEATURES_DISPLAY:
        return layer.getFeatures()
    else:
        for i in range(MAX_FEATURES_DISPLAY):
            feature = layer.getFeature(i)
            features.append(feature)
        return features


def get_attributes_of_selected_features_of_active_layer(qgis_interface: QgisInterface):
    """
    Retrieves the attributes of the active layer of the QGIS project.

    :param qgis_interface: The QGIS interface object.
    :return: A list with the attributes.
    """
    layer: QgsVectorLayer = qgis_interface.activeLayer()
    features = layer.getSelectedFeatures()

    attributes = []
    for feature in features:
        attributes.append(feature.relation_fields())
    return attributes


def get_attributes_of_features_for_layer_name(qgis_interface: QgisInterface, layer_name: str):
    """
    Retrieves the attributes for a given layer.

    :param qgis_interface: The QGIS interface object.
    :param layer_name: The name of the layer.
    :return: A list with the attributes.
    """
    layer = QgsProject.instance().mapLayersByName(layer_name)[0]

    feat_count = get_features_count_for_layer_name(qgis_interface, layer_name)

    attributes = []

    if feat_count <= MAX_FEATURES_DISPLAY:
        features = layer.getFeatures()
        for feature in features:
            attributes.append(feature.relation_fields())
    else:
        for i in range(MAX_FEATURES_DISPLAY):
            feature = layer.getFeature(i)
            attributes.append(feature.relation_fields())

    return attributes


def get_selected_features_count_of_active_layer(qgis_interface: QgisInterface):
    """
    Retrieves the quantity of the features of the active layer in the QGIS project.

    :param qgis_interface: The QGIS interface object.
    :return: The quantity of features.
    """
    layer: QgsVectorLayer = qgis_interface.activeLayer()
    return layer.selectedFeatureCount()


def get_features_count_for_layer_name(qgis_interface: QgisInterface, layer_name: str):
    """
    Retrieves the quantity of the features for a given layer.

    :param qgis_interface: The QGIS interface object.
    :param layer_name: The name of the layer.
    :return: The quantity of features.
    """
    layer = QgsProject.instance().mapLayersByName(layer_name)[0]
    return layer.featureCount()


def get_vector_layer_names(qgis_interface: QgisInterface):
    """
    Retrieves the names of the vector layers of the current QGIS project.

    :param qgis_interface: The QGIS interface object.
    :return: A list with the layer names.
    """
    layers = qgis_interface.mapCanvas().layers()
    layer_names = []
    for layer in layers:
        if layer.type() == QgsMapLayer.VectorLayer:
            layer_names.append(layer.name())
    return layer_names


def get_vector_layer_names_for_geometry_type(qgis_interface: QgisInterface, geometry_type: int):
    """
    Retrieves the layer names for a given geometry type.

    :param qgis_interface: The QGIS interface object.
    :param geometry_type: The type of the geometry as integer (0: point, 1: line, 2: region).
    :return: A list with the layer names.
    """
    layers = qgis_interface.mapCanvas().layers()
    layer_names = []
    layer: QgsVectorLayer
    for layer in layers:
        if layer.type() == QgsMapLayer.VectorLayer\
                and layer.geometryType() == geometry_type:
            layer_names.append(layer.name())
    return layer_names