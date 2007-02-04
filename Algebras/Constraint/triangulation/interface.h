/*
Header File of the Triangulation Library

	This triangulation library is an implementation of a fast polygon
triangulation algorithm based on the paper ''A simple and fast
incremental randomized algorithm for computing trapezoidal
decompositions and for triangulating polygons'' by Raimund Seidel.

	The algorithm handles simple polygons with holes. The input is
specified as contours. The outermost contour is anti-clockwise, while
all the inner contours must be clockwise. No point should be repeated
in the input.

	The output is a list of triangles. Each triangle gives a pair
(i, j, k) where i, j, and k are indices of the vertices specified in
the input array. (The index numbering starts from 1, since the first
location v[0] in the input array of vertices is unused). The number of
output triangles produced for a polygon with n points is, (n - 2) + 2(nr of holes).

*/

#ifndef __interface_h
#define __interface_h

#define TRUE 1
#define FALSE 0

extern "C" int triangulate_polygon(int ncontours,
                                   int cntr[],
                                   double (*vertices)[2],
                                   int (*triangles)[3]);

/*
Input specified as contours.
Outer contour must be anti-clockwise.
All inner contours must be clockwise.

Every contour is specified by giving all its points in order. No
point shoud be repeated. i.e. if the outer contour is a square,
only the four distinct endpoints shopudl be specified in order.

ncontours: nr of contours
cntr: An array describing the number of points in each
contour. Thus, cntr[i] = nr of points in the i'th contour.
vertices: Input array of vertices. Vertices for each contour
           immediately follow those for previous one. Array location
           vertices[0] must NOT be used (i.e. i/p starts from
           vertices[1] instead. The output triangles are
	     specified  w.r.t. the indices of these vertices.

triangles: Output array to hold triangles.

Enough space must be allocated for all the arrays before calling this routine

*/


extern "C" int is_point_inside_polygon(double * vertex);
/*
This function returns TRUE or FALSE depending upon whether the
vertex is inside the polygon or not. The polygon must already have
been triangulated before this routine is called.
This routine will always detect all the points belonging to the
set (polygon-area - polygon-boundary). The return value for points
on the boundary is not consistent!!!

*/

#define SEGSIZE_TRIANGULATION 10000
/*
max nr of segments.
Determines how many points can be specified as input.
If your datasets have large number of points,
increase this value (here and SEGSIZE in triangulat.h) accordingly.

*/

#endif
