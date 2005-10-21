/*
 * MeshGenerator.c 2004-11-09
 *
 * Dirk Ansorge, FernUniversitaet Hagen
 * 
 */

#include "MeshGenerator.h"
#include <stdio.h>

/*
 * This C file implements the interface between the JAVA code of the 2DSACK package and the NetGen code which is written in C++. There are two
 * functions in this class which are <tt>triangulateNetGen</tt> and <tt>freeMemoryNetGen</tt>. The first is for computing the triangulation, whereas
 * the latter is for freeing some used memory to prevent memory leaks.<p>
 * Communication between JAVA and C++ is done unsing the JNI (JAVA NATIVE INTERFACE). The .h file for this C code is generated automatically by calling
 * javah.
 */

//nglib.h provides the NetGen interface functions
namespace nglib {
#include "NetGen/libsrc/interface/nglib.h"
}

using namespace nglib;
static jdoubleArray returnArray;
static Ng_Geometry_2D * geom;


JNIEXPORT jdoubleArray JNICALL Java_twodsack_util_meshgenerator_MeshGenerator_triangulateNetGen (JNIEnv *env, jobject obj, jdoubleArray pointArray, int numberofpoints, jintArray lengthArray, int numberOfLengthes, jbooleanArray directionArray, int numberOfDirections) {
 
  Ng_Mesh * mesh;
  //Ng_Geometry_2D * geom;

  Ng_Init();
  
  //convert JAVA arrays to C array
  jdouble* pointListBody = (env)->GetDoubleArrayElements(pointArray,0);
  jint* lengthArrBody = (env)->GetIntArrayElements(lengthArray,0);
  jboolean * directionArrBody = (env)->GetBooleanArrayElements(directionArray,0);
  
  //constructs the polygon structure as a NetGen geometry
  geom = Ng_ConstructGeometry_2D(pointListBody,numberofpoints,(int*)lengthArrBody,numberOfLengthes,directionArrBody,numberOfDirections);
  
  //the Meshing_Parameters define how the triangulation is computed
  Ng_Meshing_Parameters mParam;
  mParam.maxh = 1.141421; //This is a value for the number of generated triangles. Must be 0 < maxh < 1.41422.
  //mParam.fineness = 0; //0 .. coarse, 1 .. fine --> is not implemented by NetGen programmers!!!
  //mParam.secondorder = 0; //--> is not implemented by NetGen programmers!!!

  //computes a mesh from geometry
  Ng_GenerateMesh_2D (geom, &mesh, &mParam); //geom = input data, mesh = output data, mParam = meshing function parameters
  
  int matnum; //number for point/segment enumeration
  int numberTris = Ng_GetNE_2D(mesh);
  int nodes[3]; //point coordinates?!
  double point[2]; //segment's endpoints

  //free all memory used by arrays above
  (env)->ReleaseDoubleArrayElements(pointArray,pointListBody,JNI_ABORT);
  (env)->DeleteLocalRef(pointArray);
  (env)->ReleaseIntArrayElements(lengthArray,lengthArrBody,JNI_ABORT);
  (env)->DeleteLocalRef(lengthArray);
  (env)->ReleaseBooleanArrayElements(directionArray,directionArrBody,JNI_ABORT);
  (env)->DeleteLocalRef(directionArray);

  //write triangle data in java array
  int jArrSize = numberTris*3*2;
  returnArray = (env)->NewDoubleArray(jArrSize);
  int returnArrIdx;
  double *x0,*y0,*x1,*y1,*x2,*y2;
  returnArrIdx = 0;
  for (int j = 1; j <= numberTris; j++) {
    Ng_GetElement_2D(mesh,j,nodes,&matnum);
    //get first triangle point
    Ng_GetPoint_2D(mesh,nodes[0],point);
    x0 = &point[0];
    y0 = &point[1];
    //set point
    (env)->SetDoubleArrayRegion(returnArray,returnArrIdx,1,x0);
    returnArrIdx++;   
    (env)->SetDoubleArrayRegion(returnArray,returnArrIdx,1,y0);
    returnArrIdx++;

    //get second triangle point
    Ng_GetPoint_2D(mesh,nodes[1],point);
    x1 = &point[0];
    y1 = &point[1];
    //set point
    (env)->SetDoubleArrayRegion(returnArray,returnArrIdx,1,x1);
    returnArrIdx++;
    (env)->SetDoubleArrayRegion(returnArray,returnArrIdx,1,y1);
    returnArrIdx++;
    
    //get third triangle point
    Ng_GetPoint_2D(mesh,nodes[2],point);
    x2 = &point[0];
    y2 = &point[1];
    //set point
    (env)->SetDoubleArrayRegion(returnArray,returnArrIdx,1,x2);
    returnArrIdx++;
    (env)->SetDoubleArrayRegion(returnArray,returnArrIdx,1,y2);
    returnArrIdx++;    
  }//for j

  Ng_DeleteMesh(mesh);
  
  Ng_Exit();
  
  return returnArray;
}

/**
 * Frees some memory by deleting the point array. Call this only after the triangles were constructed on the JAVA side.
 */
JNIEXPORT void JNICALL Java_twodsack_util_meshgenerator_MeshGenerator_freeMemoryNetGen (JNIEnv * env, jobject obj) {
  if (returnArray != 0) {
    (env)->ReleaseDoubleArrayElements(returnArray,0,JNI_ABORT);
    (env)->DeleteLocalRef(returnArray);
    returnArray = 0;
    Ng_CleanUp(geom);
  }
}

