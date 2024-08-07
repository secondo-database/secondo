/*

genmesh2d.cpp is part of the NETGEN package

*/

#include <mystdlib.h>
#include <csg.hpp>
#include <geometry2d.hpp>
#include "meshing.hpp"

using namespace std;

namespace netgen
{

  // static ARRAY<Point<2> > points2;
  //  static ARRAY<int> lp1, lp2;


  extern void Optimize2d (Mesh & mesh, MeshingParameters & mp);




  void MeshFromSpline2D (SplineGeometry2d & geometry,
			 Mesh *& mesh, 
			 MeshingParameters & mp)
  {
    int i, j, domnr;
    double elto0, minx, miny, maxx, maxy;

    //    mp.Print(*testout);

    PointIndex pi;
    SegmentIndex si;
    SurfaceElementIndex sei;

    double h = mp.maxh;

    Box<2> bbox;
    geometry.GetBoundingBox (bbox);

    if (bbox.Diam() < h) 
      {
	h = bbox.Diam();
	mp.maxh = h;
      }

    mesh = new Mesh;
    mesh->SetDimension (2);
    //PrintMessage (1, "Generate Mesh from spline geometry");

    //cout << "changing values of optsteps2d, opimize2d." << endl;
    mp.maxh = 1;
    mp.optsteps2d = 5;
    mp.optimize2d = "smcsmcsmcmSmcSmcSmcm";
    mp.blockfill = 0;
    mp.filldist = 0;
    mp.grading = 10;
    mp.parthread = 0; //make sure, this is ALWAYS = 0
    /*
    cout << "Meshing Parameters:" << endl;
    cout << "optimize2d: " << (&mp)->optimize2d << endl;
    cout << "optsteps2d: " << (&mp)->optsteps2d << endl;
    cout << "opterrpow: " << (&mp)->opterrpow << endl;
    cout << "blockfill: " << (&mp)->blockfill << endl;
    cout << "filldist: " << (&mp)->filldist << endl;
    cout << "safety: " << (&mp)->safety << endl;
    cout << "relinnersafety: " << (&mp)->relinnersafety << endl;
    cout << "uselocalh: " << (&mp)->uselocalh << endl;
    cout << "grading: " << (&mp)->grading << endl;
    cout << "delaunay: " << (&mp)->delaunay << endl;
    cout << "maxh: " << (&mp)->maxh << endl;
    cout << "startinsurface: " << (&mp)->startinsurface << endl;
    cout << "checkoverlap: " << (&mp)->checkoverlap << endl;
    cout << "checkchartboundary: " << (&mp)->checkchartboundary << endl;
    cout << "curvaturesafety: " << (&mp)->curvaturesafety << endl;
    cout << "segmentsperedge: " << (&mp)->segmentsperedge << endl;
    cout << "parthread: " << (&mp)->parthread << endl;
    cout << "elsizeweight: " << (&mp)->elsizeweight << endl;
    cout << "badellimit: " << (&mp)->badellimit << endl;
    */

    geometry.PartitionBoundary (h, *mesh);

    for (i = 0; i < geometry.GetNP(); i++)
      if (geometry.GetPoint(i).hpref)
	{
	  double mindist = 1e99;
	  PointIndex mpi;
	  Point<2> gp = geometry.GetPoint(i);
	  Point<3> gp3(gp(0), gp(1), 0);
	  for (PointIndex pi = PointIndex::BASE; 
	       pi < mesh->GetNP()+PointIndex::BASE; pi++)
	    if (Dist2(gp3, (*mesh)[pi]) < mindist)
	      {
		mpi = pi;
		mindist = Dist2(gp3, (*mesh)[pi]);
	      }
	  (*mesh)[mpi].SetSingular();
	}

    int maxdomnr = 0;
    for (si = 0; si < mesh->GetNSeg(); si++)
      {
	if ( (*mesh)[si].domin > maxdomnr) maxdomnr = (*mesh)[si].domin;
	if ( (*mesh)[si].domout > maxdomnr) maxdomnr = (*mesh)[si].domout;
      }

    mesh->ClearFaceDescriptors();
    for (i = 1; i <= maxdomnr; i++)
      mesh->AddFaceDescriptor (FaceDescriptor (i, 0, 0, i));

    Point3d pmin(bbox.PMin()(0), bbox.PMin()(1), -bbox.Diam());
    Point3d pmax(bbox.PMax()(0), bbox.PMax()(1), bbox.Diam());

    mesh->SetLocalH (pmin, pmax, mparam.grading);
    mesh->SetGlobalH (h);

    mesh->CalcLocalH();

    int bnp = mesh->GetNP(); // boundary points

    for (domnr = 1; domnr <= maxdomnr; domnr++)
      {
	//PrintMessage (3, "Meshing domain ", domnr, " / ", maxdomnr);

	int oldnf = mesh->GetNSE();
	
	Meshing2 meshing (Box3d (pmin, pmax));

	for (pi = PointIndex::BASE; 
	     pi < bnp+PointIndex::BASE; pi++)
	  meshing.AddPoint ( (*mesh)[pi], pi);
      
	PointGeomInfo gi;
	gi.trignum = 1;
	for (si = 0; si < mesh->GetNSeg(); si++)
	  {
	    if ( (*mesh)[si].domin == domnr)
	      meshing.AddBoundaryElement ( (*mesh)[si].p1 + 1 - PointIndex::BASE, 
					   (*mesh)[si].p2 + 1 - PointIndex::BASE, gi, gi);
	    if ( (*mesh)[si].domout == domnr)
	      meshing.AddBoundaryElement ( (*mesh)[si].p2 + 1 - PointIndex::BASE, 
					   (*mesh)[si].p1 + 1 - PointIndex::BASE, gi, gi);
	  }

	//h = 1.14142;
	cout << "GenerateMesh(*mesh,h,domnr), h: " << h << endl;
	mparam.checkoverlap = 0;
	meshing.GenerateMesh (*mesh, h, domnr);

	for (sei = oldnf; sei < mesh->GetNSE(); sei++)
	  (*mesh)[sei].SetIndex (domnr);
      }


    int hsteps = mp.optsteps2d;

    mp.optimize2d = "smcm"; 
    mp.optsteps2d = hsteps/2;
    cout << "Optimize2d(*mesh,smcm)" << endl;
    Optimize2d (*mesh, mp);

    /* removed by Dirk Ansorge
       mp.optimize2d = "Smcm"; 
       mp.optsteps2d = (hsteps+1)/2;
       
       cout << "check 07" << endl;
       
       cout << "Optimize2d(mesh,Smcm)" << endl;
       Optimize2d (*mesh, mp);
    */

    mp.optsteps2d = hsteps;

    mesh->Compress();
    mesh -> SetNextMajorTimeStamp();


#ifdef OPENGL
    extern void Render();
    Render();
#endif

  }


}
