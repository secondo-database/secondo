/*
 * MeshGenerator.c 2004-11-09
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 *
 * 
 * To compile this code without the Secondo make files, use a MSYS shell (under windows) and type 'make -f makefile.windows'. 
 * Make sure that you have the makefile.windows file.
 */

#include <jni.h>
#define REAL double
#include "MeshGenerator.h"
#include "Triangle/triangle.h"
#include <stdio.h>

/*
 * This C file implements only one single function. It is the triangulate() function which is a native function that is called by Java
 * code of the class twodsack.util.meshgenerator.Meshgenerator. Communication between Java and C is possible using the JNI (Java Native
 * Interface). It allows to call C code from Java and vice versa. Since the mesh generator that is used inside of the 2DSACK package
 * is written in C code, we need the JNI here. The .h file for this file is generated automatically by calling javah.<p>
 * The implementation of this function takes the big number of parameters and transforms it in a way that the C mesh generator can use it.
 * Then, the mesh generator's main function is called. When it returns, the list of triangle coordinates is passed back to the JNI, which
 * gives it back to the Java code.
 */
JNIEXPORT jobjectArray JNICALL Java_twodsack_util_meshgenerator_MeshGenerator_triangulate (JNIEnv *env,
							       jobject obj,
							       jcharArray arguments,
							       jdoubleArray pointlist,
							       jdoubleArray pointattributelist,
							       jintArray pointmarkerlist,
							       jint numberofpoints,
							       jint numberofpointattributes,
							       jintArray trianglelist,
							       jdoubleArray triangleattributelist,
							       jdoubleArray trianglearealist,
							       jintArray neighborlist,
							       jint numberoftriangles,
							       jint numberofcorners,
							       jint nuberoftriangleattributes,
							       jintArray segmentlist,
							       jintArray segmentmarkerlist,
							       jint numberofsegments,
							       jint numberofholes,
							       jdoubleArray holelist,
							       jint numberofregions,
							       jdoubleArray regionlist) {
  struct triangulateio in, out, mid;

  /* define input */
  in.numberofpoints = numberofpoints;
  jdouble* plbody = (*env)->GetDoubleArrayElements(env,pointlist,0);
  in.pointlist = plbody;

  in.numberofpointattributes = 0;
  in.pointattributelist = (REAL*) NULL;

  in.pointmarkerlist = (int*) NULL;

  in.numberoftriangles = 0;
  in.trianglelist = (int*) NULL;
  in.numberoftriangleattributes = 0;
  in.triangleattributelist = (REAL*) NULL;
  in.trianglearealist = (REAL*) NULL;
  in.numberofcorners = 0;
  in.neighborlist = (int*) NULL;

  in.numberofsegments = numberofsegments;
  in.segmentmarkerlist = (int*) NULL;
  int* slbody = (int*)(*env)->GetIntArrayElements(env,segmentlist,0);
  in.segmentlist = slbody;

  in.numberofholes = numberofholes;
  jdouble* hlbody;
  if (numberofholes > 0) {
    hlbody = (*env)->GetDoubleArrayElements(env,holelist,0);
    in.holelist = hlbody;
  }//if
  else in.holelist = (REAL*) NULL;

  in.numberofregions = 0;
  in.regionlist = (REAL*) NULL;
  
  in.numberofedges = 0;
  in.edgelist = (int*) NULL;


  mid.pointlist = (REAL *) NULL;
  mid.pointattributelist = (REAL *) NULL;
  mid.pointmarkerlist = (int *) NULL;
  mid.trianglelist = (int *) NULL;
  mid.triangleattributelist = (REAL *) NULL;
  mid.neighborlist = (int *) NULL;
  mid.segmentlist = (int *) NULL;
  mid.segmentmarkerlist = (int *) NULL;
  mid.edgelist = (int *) NULL;
  mid.edgemarkerlist = (int *) NULL;
  
  out.pointlist = (REAL *) NULL;
  out.pointattributelist = (REAL *) NULL;
  out.trianglelist = (int *) NULL;
  out.triangleattributelist = (REAL *) NULL;

  /* 
   * call the meshing algorithm
   * the switches mean the following:
   * p : reads a Planar Straight Line Graph, which can specifiy vertices, segments, holes...
   *     generates a constrained Delaunay triangulation
   * q : quality mesh generation
   * Q : no output until an error occurs
   */
  const char *str = (*env)->GetStringUTFChars(env,arguments,0);
  
  triangulate(str, &in, &mid, (struct triangulateio *) NULL);
  
  //free all the memory used by the arrays above
  (*env)->ReleaseDoubleArrayElements(env,pointlist,plbody,0);
  (*env)->ReleaseIntArrayElements(env,segmentlist,slbody,0);
  if (numberofholes > 0) (*env)->ReleaseDoubleArrayElements(env,holelist,hlbody,0);
  (*env)->ReleaseStringUTFChars(env,arguments,str);

  jdoubleArray returnArray;
  returnArray = (*env)->NewDoubleArray(env,mid.numberoftriangles*3*2);
  int pointIdx,returnArrIdx;
  double *x,*y;
  int j;
  returnArrIdx = 0;
  for (j = 0; j < mid.numberoftriangles*3; j++) {
    //get pointIdx
    pointIdx = mid.trianglelist[j];
    //get coordinates
    x = (double*)&mid.pointlist[pointIdx*2-2];
    y = (double*)&mid.pointlist[pointIdx*2-1];
    //store coordinates in returnArray
    (*env)->SetDoubleArrayRegion(env,returnArray,returnArrIdx,1,x);
    returnArrIdx++;
    (*env)->SetDoubleArrayRegion(env,returnArray,returnArrIdx,1,y);
    returnArrIdx++;
  }//for i

  return returnArray;
  
}
