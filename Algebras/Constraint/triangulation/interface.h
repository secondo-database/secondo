/*
pd comments needed

*/

#ifndef __interface_h
#define __interface_h

#define TRUE 1
#define FALSE 0

extern "C" int triangulate_polygon(int, int *, double (*)[2], int (*)[3]);
extern "C" int is_point_inside_polygon(double *);

#define SEGSIZE_TRIANGULATION 10000	
	/* max# of segments. Determines how */
	/* many points can be specified as */
	/* input. If your datasets have large */
	/* number of points, increase this */
	/* value (here and SEGSIZE in triangulat.h) accordingly. */

#endif /* __interface_h */
