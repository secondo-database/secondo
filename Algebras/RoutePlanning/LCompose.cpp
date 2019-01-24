/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software{} you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation{} either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY{} without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO{} if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

*/

#include "Algebras/LineFunction/LMapping.h"
#include "LCompose.h"
#include "Algebras/Pointcloud/PointCloud.h"
#include "TinForRoutePlanning.h"

#ifdef USE_CGAL_LIBRARY

#include <CGAL/Simple_cartesian.h>
#include <CGAL/wlop_simplify_and_regularize_point_set.h>

#endif

//
// This class implements the functions of LCompose.h
//

//Uncomment this, if you don't want the algorithm to use the points from the
//pointclouds, but specific ones.
//#define DEBUG_MODE

//Uncomment this, if the set of points should be static and not random.
//#define STATIC_NET

//Uncommend this, if you want to skip the whole process of making a tin from
//Points and rather compute a lreal
//#define EXAMPLE_DATA

//Uncommend this, if precision parameter should be adjusted, if not enough
//points are found in proximity of sline. This can help prevent quality loss in
//low point density regions.
//#define ADJUST_PRECISION

using namespace tin;

namespace routeplanningalgebra {
  // at the equator about 110km make 1̀deg of longitude
  const double PROXIMITY_THRESHOLD = 0.00005; // about 5m
  const double MAX_PROXIMITY_THRESHOLD = 0.0002; // about 20m
  const double MIN_POINTS_PER_PROXIMITY_LENGTH = 10;

  // Precision for double calculations
  const double PRECISION = 1e-10;

  // (at equator, less anywere else);

  //Logger
  Logger MSG(0);
  Logger ERROR(1);
  Logger WARN(2);
  Logger INFO(3);
  Logger DEBUG(4);
  Logger DETAIL(5);
  Logger DETAIL2(6);
  //parameters for random tin generation
  //change these values to your liking
  const double marginX = 0.001;
  const double marginY = 0.1;

  // Small part of San Francisco
  const double minX = -122.409 - marginX;
  const double spanX = .004 + 2 * marginX;
  const double minY = 37.6 - marginY;
  const double spanY = .19 + 2 * marginY;
  const double minZ = -5;
  const double spanZ = 20;
  const unsigned numberOfPoints = 100000;

//  const double minX = -122.57;
//  const double spanX = .29;
//  const double minY = 37.48;
//  const double spanY = .54;
//  const double minZ = -5;
//  const double spanZ = 100;
//  const unsigned numberOfPoints = 1000000;

//  const double minX = -100;
//  const double spanX = 200;
//  const double minY = -100;
//  const double spanY = 200;
//  const double minZ = -10;
//  const double spanZ = 50;
//  const unsigned numberOfPoints = 100000;

  /*
  Auxiliary class for 2D vector operations
  */
  class Vector2D {
  public:
    Vector2D(const pointcloud::Cpoint cpoint) : x(cpoint.getX()), 
                                                y(cpoint.getY()) {}

    Vector2D(const ::Point point) : x(point.GetX()), y(point.GetY()) {}

    Vector2D(const double x, const double y) : x(x), y(y) {}

    Vector2D operator-() const {
      return Vector2D(-x, -y);
    }

    Vector2D operator+(const Vector2D &v) const {
      return Vector2D(x + v.x, y + v.y);
    }

    Vector2D operator-(const Vector2D &v) const {
      return Vector2D(x - v.x, y - v.y);
    }

    Vector2D operator*(const double &d) const {
      return Vector2D(x * d, y * d);
    }

    double operator*(const Vector2D &v) const {
      return x * v.x + y * v.y;
    }

    static double dot(const Vector2D &v, const Vector2D &w) {
      return v * w;
    }

    static double distance(const Vector2D &v, const Vector2D &w) {
      return (v - w).length();
    }

    double lengthSqr() const {
      return x * x + y * y;
    }

    double length() const {
      return std::sqrt(lengthSqr());
    }

  private:
    const double x;
    const double y;
  };

  bool isInProximity(const pointcloud::Cpoint &point,
                     const SimpleLine &sline,
                     const double proximity) {
    //Bounding box of point with margin
    double min[2] = {point.getX() - proximity,
                     point.getY() - proximity};
    double max[2] = {point.getX() + proximity,
                     point.getY() + proximity};
    ::Rectangle<2> proximityBox(true, (double *) &min, (double *) &max);
    DETAIL2 << "Check bbox((" << min[0] << ", " << min[1] << "), ("
            << max[0]
            << ", " << max[1] << "))\r\n";

    HalfSegment hs;
    for (int i = 0; i < sline.Size(); i++) {
      sline.Get(i, hs);
      Vector2D p(point);
      Vector2D v(hs.GetLeftPoint());
      Vector2D w(hs.GetRightPoint());

      if (!hs.BoundingBox().Intersects(proximityBox)) {
        return false;
      }

      // Return minimum distance between line segment vw and point p
      const double l2 = (v - w).lengthSqr();  // i.e. |w-v|^2 -  avoid a sqrt
      if (l2 < PRECISION &&
          Vector2D::distance(p, v) <= proximity) {
        return true;
      }   // v == w case

      // Consider the line extending the segment,
      // parameterized as v + t (w - v).
      // We find projection of point p onto the line.
      // It falls where t = [(p-v) . (w-v)] / |w-v|^2
      // We clamp t from [0,1] to handle points outside the segment vw.
      double dot = (p - v) * (w - v);
      const double t = std::max(0.0, std::min(1.0, dot / l2));
      const Vector2D projection = v + ((w - v) * t);
      // Projection falls on the segment
      DETAIL2 << "distance to sline: " << (p - projection).length() << "\r\n";
      if ((p - projection).length() <= proximity) {
        return true;
      }
    }

    return false;
  }

  /*
  Function that generates Points either random or static
  */
  std::vector<pointcloud::Cpoint> generatePoints() {
    std::vector<pointcloud::Cpoint> points;
#ifdef STATIC_NET
    points.push_back(Cpoint(1, 1, 2));
    points.push_back(Cpoint(3, 6, 5));
    points.push_back(Cpoint(5, 3, 5));
    points.push_back(Cpoint(7, 6, 6));
    points.push_back(Cpoint(10, 1, 1));
    points.push_back(Cpoint(11, 4, 0));
    points.push_back(Cpoint(9, 8, 3));
    points.push_back(Cpoint(6, 11, 4));
    points.push_back(Cpoint(1, 10, 3));
//      points.push_back(Cpoint(-122.4069, 37.632801, 2.512));
//      points.push_back(Cpoint(-122.40671, 37.632801, 2.512));
//      points.push_back(Cpoint(-122.40679, 37.632801, 2.83));
//      points.push_back(Cpoint(-122.40678, 37.632566, 2.8));
//      points.push_back(Cpoint(-122.4069, 37.632507, 2.771));
//      points.push_back(Cpoint(-122.4068, 37.63239, 2.78));
//      points.push_back(Cpoint(-122.40672, 37.632131, 2.821));
//      points.push_back(Cpoint(-122.40683, 37.632063, 2.951));
//      points.push_back(Cpoint(-122.40674, 37.631842, 2.771));
//      points.push_back(Cpoint(-122.40684, 37.631754, 2.873));
//      points.push_back(Cpoint(-122.40672, 37.631656, 2.792));
//      points.push_back(Cpoint(-122.40675, 37.631533, 2.751));
//      points.push_back(Cpoint(-122.40673, 37.631359, 2.582));
//      points.push_back(Cpoint(-122.40679, 37.631311, 2.611));
//      points.push_back(Cpoint(-122.40684, 37.631011, 2.551));
//      points.push_back(Cpoint(-122.40671, 37.630949, 2.512));
//      points.push_back(Cpoint(-122.40683, 37.630923, 2.591));
//      points.push_back(Cpoint(-122.4069, 37.630891, 2.512));
//      points.push_back(Cpoint(-122.40671, 37.630891, 2.512));
#else
    srand(numberOfPoints);
    for (unsigned i = 0; i < numberOfPoints; i++) {
      double x = minX + (double) rand() / RAND_MAX * spanX;
      double y = minY + (double) rand() / RAND_MAX * spanY;
      //mountain peak like appearance
      double z = minZ +
                 (1 - (std::abs(2 * (x - minX - (spanX / 2)) / spanX))) *
                 (1 - (std::abs(2 * (y - minY - (spanY / 2)) / spanY))) *
                 spanZ;
      pointcloud::Cpoint cpoint(x, y, z);
      points.push_back(cpoint);
    }
    // completely random height
//    for (int i = 0; i < 100; i++) {
//      points.push_back(
//          *(new Cpoint(5.0 + (double) rand() / RAND_MAX / 10000, 10.0, 1.0)));
//    }
#endif
    DEBUG << "Custom points loaded." << "\r\n";
    return points;
  }

  /*
  Function that generates Points either random or static
  */
  std::vector<pointcloud::Cpoint>
  generatePoints(const double minx, const double spanx, const double miny,
                 const double spany, const double minz, const double spanz,
                 const unsigned number) {
    std::vector<pointcloud::Cpoint> points;

    srand(number);
    for (unsigned i = 0; i < number; i++) {
      double x = minx + (double) rand() / RAND_MAX * spanx;
      double y = miny + (double) rand() / RAND_MAX * spany;
      //mountain peak like appearance
      double z = minz +
                 (1 - (std::abs(2 * (x - minx - (spanx / 2)) / spanx))) *
                 (1 - (std::abs(2 * (y - miny - (spany / 2)) / spanx))) *
                 spanz;
      pointcloud::Cpoint cpoint(x, y, z);
      points.push_back(cpoint);
    }

    DEBUG << "Custom points loaded." << "\r\n";
    return points;
  }

  /*
  Function that filters only the points in proximity of the sline
  */
  std::vector<pointcloud::Cpoint> getPointsInProximityToSline(SimpleLine *sline,
                               Stream<pointcloud::PointCloud> &pointcloudStream,
                                                  const double proximity) {
    assert(sline);
    assert(proximity > 0);
    std::vector<pointcloud::Cpoint> points;
    pointcloudStream.open();

    pointcloud::PointCloud *pointCloud;
    unsigned inRange = 0;
    while ((pointCloud = pointcloudStream.request()) != 0) {
      const double startx = sline->StartPoint().GetX();
      const double starty = sline->StartPoint().GetY();
      const double endx = sline->EndPoint().GetX();
      const double endy = sline->EndPoint().GetY();
      pointcloud::Cpoints *pointContainer = pointCloud->getAllPointsInRange(
          std::min(startx, endx) - proximity,
          std::min(starty, endy) - proximity,
          std::max(startx, endx) + proximity,
          std::max(starty, endy) + proximity
      );
      pointCloud->DeleteIfAllowed();
      if (!pointContainer) {
        continue;
      }
      inRange += pointContainer->GetNoCpoints();
      for (int i = 0; i < pointContainer->GetNoCpoints(); i++) {
        const pointcloud::Cpoint &cpoint = pointContainer->GetCpoint(i);
        if (isInProximity(cpoint, *sline, proximity)) {
          points.push_back(cpoint);
        }
      }
      delete pointContainer;
    }

    pointcloudStream.close();
    // success
    if (points.size() > 0) {
      DEBUG << "Found " << inRange
            << " Points in Range, " << points.size()
            << " of which are close to sline.\r\n";
    } else {
      DEBUG << "There are no points in range of sline.\r\n";
    }

    return points;
  }

  /*
  Function that returns all points from the Pointcloud stream
  */
  std::vector<pointcloud::Cpoint> getAllPoints(Stream<pointcloud::PointCloud> 
                                               &pointcloudStream) {
    std::vector<pointcloud::Cpoint> points;
    pointcloudStream.open();

    pointcloud::PointCloud *pointCloud;
    while ((pointCloud = pointcloudStream.request()) != 0) {
      pointcloud::Cpoints *pointContainer = pointCloud->getAllPointsInRange(
          pointCloud->getMinX(),
          pointCloud->getMinY(),
          pointCloud->getMaxX(),
          pointCloud->getMaxY()
      );
      pointCloud->DeleteIfAllowed();
      for (int i = 0; i < pointContainer->GetNoCpoints(); i++) {
        const pointcloud::Cpoint &cpoint = pointContainer->GetCpoint(i);
        points.push_back(cpoint);
      }
      delete pointContainer;
    }

    pointcloudStream.close();
    // success
    return points;
  }

  /*
  Function that reads the points from the Pointcloud stream
  */
  std::vector<pointcloud::Cpoint> readPoints(SimpleLine *sline,
                             Stream<pointcloud::PointCloud> &pointcloudStream) {
#ifdef DEBUG_MODE
    unsigned number = 0;
    std::vector<pointcloud::Cpoint> points = generatePoints();
    if (sline) {
      for (unsigned i = 0; i < points.size(); i++) {
//          if (isInProximity(points[i], *sline)) {
        if (i < points.size()) {
          points[number] = points[i];
          number++;
        }
      }
      points.resize(number);
    }
    DEBUG << "Found " << points.size() << " Points near sline.";
    return points;
#else
    if (sline) {
      std::vector<pointcloud::Cpoint> points = 
                                           getPointsInProximityToSline(sline,
                                                          pointcloudStream,
                                                          PROXIMITY_THRESHOLD);
      if (points.empty()) {
        points = getPointsInProximityToSline(sline,
                                             pointcloudStream,
                                             MAX_PROXIMITY_THRESHOLD);
      }
      return points;
    } else {
      return getAllPoints(pointcloudStream);
    }
#endif
  }

  /*
  Function that reduces point density for performance enhancement
  */
  int preprocessPoints(std::vector<pointcloud::Cpoint> &points,
                       const SimpleLine *sline,
                       const double precision) {
    assert(precision >= 0);
    assert(precision <= 1);

    unsigned long size = points.size();
    DEBUG << "Number of Points: " << size << "\r\n";
    if (size == 0) {
      return 0;
    }

#ifdef ADJUST_PRECISION
    const double usedPrecision = sline ?
        std::min(
            std::max(precision,
                          (MIN_POINTS_PER_PROXIMITY_LENGTH * sline->Length()) /
                              (PROXIMITY_THRESHOLD * size))
        , 1.0) : precision;

    if (usedPrecision > precision) {
      DEBUG << "Not enough points: Use higher precision:"
            << usedPrecision << "\r\n";
    }
#else
    const double usedPrecision = precision;
#endif

#ifdef USE_CGAL_LIBRARY

    /*
    1. https://github.com/CGAL/cgal/releases
    2. Download CGAL-4.11.tar.xz 3.
    Entpacken
    4. sudo mv CGAL-4.11 /usr/include (vermutlich unnötig)
    5. cd /usr/include/CGAL-4.11
    6. cmake . 7. make 8. sudo make install 9. Einstellungen/yast
    10. <Abhängigkeiten überprüfen>
    (Da ich libcgal zuerst über die Paketquellen
    installiert hatte, könnte es hier zu Problemen kommen-
    zur Not eben zuerst die alte Version installieren.)

    11. makefile.algebras 12.
    ifdef SECONDO_ACTIVATE_ALL_ALGEBRAS
    ALGEBRA_DEP_DIRS += /usr/local/include/CGAL
    ALGEBRA_DEPS += cgal
    endif
    (der Vollständigkeit halber)
    13. Nochmal nach dem #endif eintragen:
    ALGEBRA_DEP_DIRS += /usr/local/include/CGAL
    ALGEBRA_DEPS += CGAL
    ALGEBRA_DIRS += RoutePlanning
    ALGEBRAS += RoutePlanningAlgebra

    14. makefile in RoutePlanning:
    CCFLAGS += $(ALG_INC_DIRS) -I/usr/local/include/CGAL/include

    15. Das sollte es sein.
     */

    typedef CGAL::Simple_cartesian<double> Kernel;
    typedef Kernel::Point_3 CGALPoint;

    // Converting to CGAL-class
    std::vector<CGALPoint> cgalPoints;
    for (Cpoint p : points) {
      CGALPoint cp(p.getX(), p.getY(), p.getZ());
      cgalPoints.push_back(cp);
    }

    //parameters
    const double retain_percentage = usedPrecision * 100;

    std::vector<CGALPoint> output;
    CGAL::wlop_simplify_and_regularize_point_set<CGAL::Sequential_tag>(
        cgalPoints.begin(),
        cgalPoints.end(),
        std::back_inserter(output),
        retain_percentage
    );

    points.resize(output.size());
    for (unsigned int i = 0; i < output.size(); i++) {
      CGALPoint cp = output[i];
      points[i] = Cpoint(cp.x(), cp.y(), cp.z());
    }

#else
    // DUMMY IMPLEMENTATION:

    // just remove every 1 / (1-usedPrecision) point
    // and always keep the first and do nothing if ~usedPrecision~ == 1

    unsigned long number = 0;

    if (usedPrecision == 1) {
      return 0;
    } else if (usedPrecision == 0) {
      points[0] = points[size / 2]; // keep middle point
      number = 1; // discard all other points
    } else {
      // sift points
      for (unsigned long i = 0; i < size; i++) {
        if (number <=
            (double) i * usedPrecision) { // definitely keeps the first
          points[number] = points[i];
          number++;
        }
      }
    }
    points.resize(number);

#endif
    DEBUG << "Reduced to: " << points.size() << "\r\n";
    return 0;
  }

  /*
  Add corners to make sure the sline is on the tin
  */
  void addCorners(std::vector<pointcloud::Cpoint> &points, 
                const SimpleLine *sline, const pointcloud::Cpoint *bounds = 0) {
    double min[3] = {DBL_MAX, DBL_MAX, DBL_MAX};
    double max[2] = {-DBL_MAX, -DBL_MAX};
    auto search = [&min, &max](const pointcloud::Cpoint &p) {
      const double x = p.getX();
      const double y = p.getY();
      const double z = p.getZ();
      min[0] = x < min[0] ? x : min[0];
      min[1] = y < min[1] ? y : min[1];
      min[2] = z < min[2] ? z : min[2];

      max[0] = x > max[0] ? x : max[0];
      max[1] = y > max[1] ? y : max[1];
      DETAIL2 << "New min/max: (" << min[0] << ", " << min[1] << ", "
              << min[2] << "), (" << max[0] << ", " << max[1] << ")\r\n";
    };
    if (!bounds) {
      for_each(points.begin(), points.end(), search);
    } else {
      search(bounds[0]);
      search(bounds[1]);
    }

    // Bounding Box of sline
    HalfSegment hs;
    for (int i = 0; i < sline->Size() / 2; i++) {
      sline->Get(i, hs);

      search(pointcloud::Cpoint(
          hs.GetSecPoint().GetX(), hs.GetSecPoint().GetY(), min[2]
      ));
      if (i == 0) {
        search(pointcloud::Cpoint(
            hs.GetDomPoint().GetX(), hs.GetDomPoint().GetY(), min[2]
        ));
      }
    }

    // four corners
    points.push_back(pointcloud::Cpoint(min[0], min[1], min[2]));
    points.push_back(pointcloud::Cpoint(max[0], min[1], min[2]));
    points.push_back(pointcloud::Cpoint(max[0], max[1], min[2]));
    points.push_back(pointcloud::Cpoint(min[0], max[1], min[2]));

    DETAIL << "Added Corners from: (" << min[0] << ", " << min[1] << ", "
           << min[2] << ") to (" << max[0] << ", " << max[1] << ")\r\n";
  }

  /*
  Comparator for sorting the points by y value
  */
  bool CompareByYDesc(const pointcloud::Cpoint i, const pointcloud::Cpoint j) {
    return (i.getY() > j.getY());
  }

  /*
  Function that creates the tin from the points
  -uses TinForRoutePlanning class for TIN Algebra API
  */
  tin::Tin *makeTinFromPoints(std::vector<pointcloud::Cpoint> points, 
                              const bool isTemp) {
    unsigned long number = points.size();
    if (number == 0) {
      return 0;
    }

    double coordinates[number][3];

    //sort points before making tin
    // (tin operator needs points with ascending y values)

    std::sort(points.begin(), points.end(), CompareByYDesc);
    DEBUG << "Points sorted" << "\r\n";

    //Tin Algebra throws maths errors if points are too close to each other.
    auto tooClose = [](const double *c, const pointcloud::Cpoint &p) -> bool {
      return (std::abs(c[0] - p.getX()) +
              std::abs(c[1] - p.getY())) < 1e-10;
    };

    unsigned skippedPoints = 0;
    for (unsigned int i = 0; i < number; i++) {
      if (i > 0 && tooClose(coordinates[i - skippedPoints - 1], points[i])) {
        //skip and remember how many were skipped
        skippedPoints++;
      } else {
        //add if not too close
        coordinates[i - skippedPoints][0] = points[i].getX();
        coordinates[i - skippedPoints][1] = points[i].getY();
        coordinates[i - skippedPoints][2] = points[i].getZ();
      }
    }

    if (skippedPoints > 0) {
      DETAIL << "Skipped " << skippedPoints
             << " (almost) identical points.\r\n";
    }

    return TinForRoutePlanning::createTinFrom3DCoordinates(
        coordinates, number - skippedPoints, 800, isTemp);
  }

  /*
  Simple 2D distance function for type Point_p
  */
  double distance(const Point_p a, const Point_p b) {
    return std::sqrt(pow(a.x - b.x, 2.0) + pow(a.y - b.y, 2.0));
  }

  /*
  Function that calculates the intersection of two segments,
   given they intersect
  */
  tin::Point_p
  getIntersection(Point_p &point, Point_p &destination, Edge &edge) {
    double x;
    double y;
    const double x1 = point.x;
    const double y1 = point.y;
    const double x2 = destination.x;
    const double y2 = destination.y;
    const double x3 = edge.getV1()->getX();
    const double y3 = edge.getV1()->getY();
    const double x4 = edge.getV2()->getX();
    const double y4 = edge.getV2()->getY();
    DETAIL << "Trying to get intersection of "
           << "((" << x1 << ", " << y1 << "), (" << x2 << ", " << y2 << "))"
           << "((" << x3 << ", " << y3 << "), (" << x4 << ", " << y4 << "))"
           << "\r\n";

    const double denominator =
        (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

    if (std::abs(denominator) < PRECISION) {
      DETAIL << "Denominator ~= 0." << "\r\n";
      const bool swapX = (x1 > x2);
      const bool swapY = (y1 > y2);

      if (std::abs(x1 - x2) < PRECISION) {
        DETAIL << "x = const." << "\r\n";
        if (std::abs(y1 - y2) < PRECISION) {
          DETAIL << "x, y = const." << "\r\n";
          x = x2;
          y = y2;
        } else {
          const double delta3 = std::abs(y1 - y3) + std::abs(y2 - y3);
          const double delta4 = std::abs(y1 - y4) + std::abs(y2 - y4);
          if (std::abs(delta3 - delta4) < PRECISION) { // both inside
            y = swapY xor (y3 > y4) ? y3 : y4;
            DETAIL << "both y inside." << "\r\n";
          } else {
            DETAIL << "only one y inside." << "\r\n";
            y = swapY xor (delta3 < delta4) ? y3 : y4;
          }

          x = x2 - ((y2 - y) * (x1 - x2)) / (y1 - y2);
        }
      } else {
        const double delta3 = std::abs(x1 - x3) + std::abs(x2 - x3);
        const double delta4 = std::abs(x1 - x4) + std::abs(x2 - x4);
        if (std::abs(delta3 - delta4) < PRECISION) { // both inside
          x = swapX xor (x3 > x4) ? x3 : x4;
          DETAIL << "both x inside." << "\r\n";
        } else {
          DETAIL << "only one x inside." << "\r\n";
          x = swapX xor (delta3 < delta4) ? x3 : x4;
        }

        y = y2 - ((x2 - x) * (y1 - y2)) / (x1 - x2);
      }
      //return edge point that is closer to destination
      return Point_p(x, y);
    }

    x = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4))
        / denominator;
    y = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4))
        / denominator;
    return Point_p(x, y);
  }

  /*
  Function that adds a function to a lreal
  */
  void
  addFunToLreal(const double from, const double to,
                const double heightFrom, const double heightTo,
                LReal *lreal,
                const bool openLeft,
                const bool openRight) {
    double gradient;
    if (std::abs(to - from) < PRECISION) {
      gradient = 0;
    } else {
      gradient = (heightTo - heightFrom) / (to - from);
    }

    CcReal left(from);
    CcReal right(to);

    LInterval interval(left, right, openLeft, openRight);
    LUReal fun(interval, gradient, heightFrom);

    lreal->Add(fun);
    DEBUG << "Add height fun." << "\r\n" << "\r\n";
  }

  // initialize walker
  struct CustomWalker {
    CustomWalker(Tin *tin, const SimpleLine *sline, const Point_p start)
        : start(start), walker(tin->getWalker(start)) {

      // get walker (first search triangle)
      // then walk to starting point
      walker.addPoint(start);

      // add end of each segment to the walker to create its path
      HalfSegment hs;
      for (int i = 0; i < sline->Size() / 2; i++) {
        sline->Get(i, hs);
        const double segmentLength = hs.Length();
        if (segmentLength < PRECISION) {
          continue;
        }

        const ::Point secPoint = hs.GetSecPoint();
        const Point_p destination(secPoint.GetX(), secPoint.GetY());

        walker.addPoint(destination);
        DETAIL << "Point (" << destination.x << ", " << destination.y
               << ") added to Walker." << "\r\n";
      }

      initWalk();
    }

    const Point_p start;
    Triangle::triangleWalker walker;
    Triangle triangleNow;

    void initWalk() {
      DETAIL << "Init walk" << "\r\n";
      triangleNow = *(walker.getCurrentTriangle());
      // walk up to starting point
      bool endOfWalk = false;
      while (!(walkToNextPoint(start, endOfWalk) == start) &&
             !endOfWalk) {}
    }

    Point_p walkToNextPoint(Point_p point, bool &end) {
      end = false;
      Triangle triangle = triangleNow;
      Point_p destination = walker.getCurrentDestination();
      Edge edge;
      int status;
      while (triangle == triangleNow) {
        status = walker.walk_step_sec_return(edge);
        triangleNow = *(walker.getCurrentTriangle());
        if (endOfWalk(status)) {
          end = true;
          return destination;
        }
        if (endOfSegment(status)) {
          if (walker.getCurrentDestination() == destination) {
            DETAIL << "Problem: destination hasn't changed."
                   << "\r\n";
          }
          return destination;
        }
      }
      // in other triangle now
      return getIntersection(point, destination, edge);
    }

    bool endOfSegment(const int status) {
      if (status == Triangle::triangleWalker::END_OF_SEGMENT) {
        DETAIL << "Reached end of segment." << "\r\n";
        return true;
      }
      return false;
    }

    bool endOfWalk(const int status) {
      if (status == Triangle::triangleWalker::END_OF_WALK) {
        DETAIL << "Reached end of walk." << "\r\n";
        return true;
      }
      return false;
    }
  };

  /*
  Function that combines the sline and a tin to calculate the height function
  */
  int mapSlineOnTin(tin::Tin *tin, SimpleLine *sline, LReal *lreal) {
    assert(tin);
    assert(sline);
    assert(lreal);

    if (!tin->isDefined()) {
      return 0;
    }
    if (sline->Size() == 0) {
      return 0;
    }

    // Calculation
    double position = 0;

    HalfSegment hs;
    sline->Get(0, hs);
    const ::Point domPoint = hs.GetDomPoint();
    Point_p point(domPoint.GetX(), domPoint.GetY());

    // initialize walker
    if (tin->atlocation(point) ==
        -std::numeric_limits<VERTEX_Z>::max()) {
      return -1;
    }
    CustomWalker walker(tin, sline, point);

    // lreal -> defined
    lreal->SetDefined(true);

    // walk
    try {
      double height = tin->atlocation(point);
      if (height == -std::numeric_limits<VERTEX_Z>::max()) {
        height = tin->maximum();
      }

      double prevHeight;
      Point_p prevPoint = point;
      bool end;

      do {
        point = walker.walkToNextPoint(point, end);
        DETAIL << "Stopped at: (" << point.x << ", " << point.y << ")"
               << "\r\n";

        //distance walked
        double walkDistance = distance(point, prevPoint);

        DETAIL << "Length: " << walkDistance << "\r\n";
        if (walkDistance < PRECISION && !end) {
          continue;
        }

        prevPoint = point;

        //get height in Tin at new point
        prevHeight = height;
        height = tin->atlocation(point);
        if (height == -std::numeric_limits<VERTEX_Z>::max()) {
          height = tin->maximum();
        }
        DETAIL << "Heights: from " << prevHeight << " to " << height
               << "\r\n";

        addFunToLreal(position, position + walkDistance, prevHeight,
                      height,
                      lreal, true, end);
        position += walkDistance;
      } while (!end);
    } catch (std::runtime_error e1) {
      lreal->SetDefined(false);
      ERROR << "Exception when walking tin: " << e1.what() << "\r\n";
    } catch (std::invalid_argument e2) {
      lreal->SetDefined(false);
      ERROR << "Exception when walking tin: " << e2.what() << "\r\n";
    }

    return 0;
  };

  /*
  Function that calculates a height function for ~sline~ based on its segments
  */
  int getExampleLreal(SimpleLine *sline, const double minx, const double miny,
                      const double spanx, const double spany, LReal *lreal) {
    assert(sline);
    assert(lreal);

    if (sline->Size() == 0) {
      return 0;
    }

    double position = 0;

    HalfSegment hs;
    for (int i = 0; i < sline->Size(); i++) {
      sline->Get(i, hs);

      const ::Point domPoint = hs.GetDomPoint();
      Point_p point(domPoint.GetX(), domPoint.GetY());
      double height = minZ +
                      (1 - (std::abs(2 * (point.x - minx - (spanx / 2)) /
                                     spanx))) *
                      (1 - (std::abs(2 * (point.y - miny - (spany / 2)) /
                                     spany))) *
                      spanZ;

      const ::Point secPoint = hs.GetSecPoint();
      Point_p point2(secPoint.GetX(), secPoint.GetY());
      double height2 = minZ +
                       (1 -
                        (std::abs(2 * (point2.x - minx - (spanx / 2)) /
                                  spanx))) *
                       (1 -
                        (std::abs(2 * (point2.y - miny - (spany / 2)) /
                                  spany))) *
                       spanZ;

      //distance walked
      double walkDistance = distance(point, point2);

      if (walkDistance < PRECISION) {
        continue;
      }

      addFunToLreal(position, position + walkDistance, height, height2,
                    lreal, true, i == sline->Size() - 1);
      position += walkDistance;
    }
    return 0;
  };

  /*
  Value Mapping
  */
  int LCompose::lcomposeVM(
      Word *args, Word &result, int message, Word &local, Supplier s) {
    result = qp->ResultStorage(s);
    LReal *lreal = (LReal *) result.addr;
    lreal->Clear();

    SimpleLine *sline = (SimpleLine *) args[0].addr;
    Stream<pointcloud::PointCloud> stream(args[1]);
    CcReal *ccReal = (CcReal *) args[2].addr;

    lreal->SetDefined(false);
#ifdef EXAMPLE_DATA
    //return a lreal
    //get BBox of all Pointclouds
    double minx = DBL_MAX;
    double miny = DBL_MAX;
    double maxx = -DBL_MAX;
    double maxy = -DBL_MAX;

    cout.precision(8);
    PointCloud *pointCloud;
    stream.open();
    while ((pointCloud = stream.request()) != 0) {
      minx = std::min(minx, pointCloud->getMinX());
      miny = std::min(miny, pointCloud->getMinY());
      DETAIL << "Min y: " << pointCloud->getMinY() << "\r\n";
      maxx = std::max(maxx, pointCloud->getMaxX());
      maxy = std::max(maxy, pointCloud->getMaxY());
    }

    DEBUG << "Bounding box: ((" << minx << ", " << miny << "), (" << maxx
            << ", " << maxy << "))\r\n";
    stream.close();

    getExampleLreal(sline, minx, miny, maxx - minx, maxy - miny, lreal);
    return 0;
#endif

    if (!sline->IsDefined()) {
      WARN << "Sline is not defined.\r\n";
      return -1;
    }
    if (!ccReal->IsDefined() ||
        ccReal->GetRealval() > 1 || ccReal->GetRealval() < 0) {
      ERROR << "Please provide a valid density parameter.\r\n";
      return -1;
    }
    double precision = ccReal->GetRealval();

    std::vector<pointcloud::Cpoint> points;

    // Get only the points that are close to the sline
    points = readPoints(sline, stream);
    if (points.empty()) {
      WARN << "No points found in range.\r\n";
      return 0;
    }

    // Reduce the number of points by discarding redundant ones and outliers
    // Keep a number of points according to ~precision~
    // (if 0 => keep 1, if 1 => keep all)
    preprocessPoints(points, sline, precision);

    if (points.empty()) {
      ERROR << "Preprocessing failed.\r\n";
      return -1;
    }
    // Add corners so that every point of sline is on tin.
    addCorners(points, sline);

    // Make TIN from points
    tin::Tin *tin = 0;
    try {
      tin = makeTinFromPoints(points, true);
    } catch (std::exception &e) {
      ERROR << "Exception at making tin:" << e.what() << "\r\n";
      delete tin;
      return -1;
    }

    if (!tin) {
      ERROR << "Tin could not be build. (returned nullptr., but no error.)\r\n";
      return -1;
    }

    // put sline and tin together
    mapSlineOnTin(tin, sline, lreal);
    lreal->SetDefined(true);
    delete tin;

    DEBUG << "lreal created." << "\r\n";
    return 0;
  }

  /*
  Type Mapping
  */
  ListExpr LCompose::lcomposeTM(ListExpr args) {
    if (!nl->HasLength(args, 3)) {
      return listutils::typeError("You must provide 3 arguments.");
    }
    const ListExpr sline = nl->First(args);
    const ListExpr pointCloudStream = nl->Second(args);
    const ListExpr precision = nl->Third(args);

    if (!SimpleLine::checkType(sline)) {
      return listutils::typeError(
          "The first argument must be of type sline");
    }
    if (!Stream<pointcloud::PointCloud>::checkType(pointCloudStream)) {
      return listutils::typeError(
          "The second argument must be of type Stream<PointCloud>");
    }
    if (!CcReal::checkType(precision)) {
      return listutils::typeError(
          "The second argument must be of type real");
    }

    return nl->SymbolAtom(LReal::BasicType());
  }

  /*
  Specification
  */
  OperatorSpec LCompose::lcomposeSpec(
      "sline x stream(pointcloud) x real -> lreal",
      "_ # [_,_]",
      "sline x pointclouds x precision -> slineWithHeights",
      "query sline Pointcloud feed transformstream lcompose [.5];");

  /*
  Operator
  */
  Operator LCompose::lcompose("lcompose",
                              lcomposeSpec.getStr(),
                              LCompose::lcomposeVM,
                              Operator::SimpleSelect,
                              LCompose::lcomposeTM);

  /*
  Auxiliary operator ~pointcloud2Tin
  */

  /*
  Value Mapping
  */
  int LCompose::PointcloudToTin::pointcloud2TinVM(Word *args, Word &result,
                                                  int message, Word &local,
                                                  Supplier s) {
    Stream<pointcloud::PointCloud> stream(args[0]);
    CcReal *ccReal = (CcReal *) args[1].addr;

    if (!ccReal->IsDefined()) {
      // TODO: return undefined or throw error
      return -1;
    }
    double precision = ccReal->GetRealval();

    std::vector<pointcloud::Cpoint> points = readPoints(0, stream);





    // Reduce the number of points by discarding redundant ones and outliers
    // Keep a number of points according to ~precision~
    // (if 0 => keep 1, if 1 => keep all)
    if (preprocessPoints(points, 0, precision) == -1) {
      // TODO: Throw error
      return -1;
    }
    DEBUG << "Points preprocessed." << "\r\n";

    // Make TIN from points
    tin::Tin *tin = makeTinFromPoints(points, false);

    if (!tin) {
      ERROR << "Tin was not created. (makeTinFromPoints returned null)";
      return -1;
    }

    //result.setAddr(tin);
    qp->DeleteResultStorage(s);
    qp->ChangeResultStorage(s, SetWord(tin));
    result = qp->ResultStorage(s);
    return 0;
  }

  /*
  Type Mapping
  */
  ListExpr LCompose::PointcloudToTin::pointcloud2TinTM(ListExpr args) {
    if (!nl->HasLength(args, 2)) {
      return listutils::typeError("You must provide 2 arguments.");
    }
    const ListExpr pointCloudStream = nl->First(args);
    const ListExpr precision = nl->Second(args);

    if (!Stream<pointcloud::PointCloud>::checkType(pointCloudStream)) {
      return listutils::typeError(
          "The first argument must be of type Stream<PointCloud>");
    }
    if (!CcReal::checkType(precision)) {
      return listutils::typeError(
          "The second argument must be of type real");
    }

    return nl->SymbolAtom(Tin::BasicType());
  }

  /*
  Specification
  */
  OperatorSpec LCompose::PointcloudToTin::pointcloud2TinSpec(
      "stream(pointcloud) x real -> tin",
      "_ # [_]",
      "pointclouds x precision -> tin",
      "query Pointcloud feed transformstream pointcloud2Tin [.5];");

  /*
  Operator
  */
  Operator LCompose::PointcloudToTin::pointcloud2Tin(
      "pointcloud2Tin",
      PointcloudToTin::pointcloud2TinSpec.getStr(),
      LCompose::PointcloudToTin::pointcloud2TinVM,
      Operator::SimpleSelect,
      LCompose::PointcloudToTin::pointcloud2TinTM);
}
