/*

2d Spline curve for Mesh generator

*/

#include <mystdlib.h>
#include <csg.hpp>
#include <linalg.hpp>
#include <meshing.hpp>


namespace netgen

{

#include "spline2d.hpp"
#include "splinegeometry2.hpp"



SplineGeometry2d :: ~SplineGeometry2d()
{
  //cout << "deconstructor SplineGeometry" << endl;
  for(int i=0; i<splines.Size(); i++)
    {
      delete splines[i];
    }
  splines.DeleteAll();
  geompoints.DeleteAll();
}

void SplineGeometry2d :: Load (const char * filename)
{
  ifstream infile;
  int nump, numseg, leftdom, rightdom;
  double x, y;
  int hi1, hi2, hi3;
  double hd;
  char buf[50], ch;

  infile.open (filename);

  if (! infile.good() ) {
    cout << "File not found!" << endl;
    throw NgException(string ("2D Input file '") + 
		      string (filename) +
		      string ("' not available!"));
  }

  infile >> buf;   // file recognition
  infile >> elto0;

  infile >> nump;
  for (int i = 0; i < nump; i++)
    {
      infile >> x >> y >> hd;

      Flags flags;

      ch = 'a';
      // infile >> ch;
      do {
	infile.get (ch);
      } while (isspace(ch) && ch != '\n');
      while (ch == '-')
	{
	  char flag[100];
	  flag[0]='-';
	  infile >> (flag+1);
	  flags.SetCommandLineFlag (flag);
	  ch = 'a';
	  do {
	    infile.get (ch);
	  } while (isspace(ch) && ch != '\n');
	}
    
      if (infile.good())
	infile.putback (ch);

      geompoints.Append (GeomPoint2d(x, y, hd));
      geompoints.Last().hpref = flags.GetDefineFlag ("hpref");
    }

  infile >> numseg;
  SplineSegment * spline = 0;
  for (int i = 0; i < numseg; i++)
    {
      infile >> leftdom >> rightdom;

      // cout << "add spline " << i << ", left = " << leftdom << endl;

      infile >> buf;
      // type of spline segement
      if (strcmp (buf, "2") == 0)
	{ // a line
	  infile >> hi1 >> hi2;
	  spline = new LineSegment(geompoints[hi1-1],
				   geompoints[hi2-1]);
	}
      else if (strcmp (buf, "3") == 0)
	{ // a rational spline
	  infile >> hi1 >> hi2 >> hi3;
	  spline = new SplineSegment3 (geompoints[hi1-1],
				       geompoints[hi2-1],
				       geompoints[hi3-1]);
	}
      else if (strcmp (buf, "4") == 0)
	{ // an arc
	  infile >> hi1 >> hi2 >> hi3;
	  spline = new CircleSegment (geompoints[hi1-1],
				      geompoints[hi2-1],
				      geompoints[hi3-1]);
	  break;
	}
      else if (strcmp (buf, "discretepoints") == 0)
	{
	  int npts;
	  infile >> npts;
	  ARRAY<Point<2> > pts(npts);
	  for (int j = 0; j < npts; j++)
	    infile >> pts[j](0) >> pts[j](1);

	  spline = new DiscretePointsSegment (pts);
	  cout << "pts = " << pts << endl;
	}
    
      infile >> spline->reffak;
      spline -> leftdom = leftdom;
      spline -> rightdom = rightdom;
      splines.Append (spline);


      Flags flags;
      ch = 'a';
      infile >> ch;
      while (ch == '-')
	{
	  char flag[100];
	  flag[0]='-';
	  infile >> (flag+1);
	  flags.SetCommandLineFlag (flag);
	  ch = 'a';
	  infile >> ch;
	}
    
      if (infile.good())
	infile.putback (ch);
    
      splines.Last()->bc = int (flags.GetNumFlag ("bc", i+1));
      splines.Last()->hpref_left = int (flags.GetDefineFlag ("hpref")) || 
	int (flags.GetDefineFlag ("hprefleft"));
      splines.Last()->hpref_right = int (flags.GetDefineFlag ("hpref")) || 
	int (flags.GetDefineFlag ("hprefright"));
      splines.Last()->copyfrom = int (flags.GetNumFlag ("copy", -1));
    }


  infile.close();
}


/* The following function was added by Dirk Ansorge to support construction of a 
 * SplineGeometry2d without reading from a file.
 * Passed to this function are three arrays. The first array stores the point coordinates for several cycles. First, the coordinates
 * for the outer cycle is stored. The length of that cycle is stored in the first field of the lengtharray. The direction of that 
 * cycle is stored in the first field of the directionarray. There may be further point coordinates which belong to hole cycles.
 * They are treated the same way as the outer cycle, i.e. the lengthes and directions are stored in the appropriate arrays.
 */

void SplineGeometry2d ::
ConstructFromArray (double * pointArr, int arrSize, int * lengtharray, int lsize, unsigned char * directionarray, int dsize)
{
  
  //cout << "SplineGeometry2d::ConstructFromArray, pointArr: " << arrSize << ", lengtharray: " << lsize << ", directionarray: " << dsize << endl;
  //set the grading factor; describes how fast the meshsize decreases
  elto0 = 3; 
  
  //transfer point data
  Flags flags;
  for (int i = 0; i < arrSize; i++) {
    //cout << "constructing GeomPoint2d from (" << pointArr[i] << "," << pointArr[i+1] << ")" << endl;
    geompoints.Append(GeomPoint2d(pointArr[i],pointArr[i+1],1));
    i++;
    geompoints.Last().hpref = flags.GetDefineFlag("hpref");  
  }//for i

  /*
  //printing geompoints
  cout << "geompoints:" << endl;
  int numberofpoints = arrSize/2;
  for (int i = 0; i < numberofpoints; i++) {
    cout << "[" << i << "] " << geompoints[i] << endl;
  }
  */
  
  //construct spline segment data
  //The segments must be constructed depending on the different cycles stored in the pointarr.
  SplineSegment * spline = 0;
  //int numberofsegs = numberofpoints-1;
  int actNumbOfPoints = 0;
  int baseAddress = 0;
  //lsize is the total number of cycles
  for (int i = 0; i < lsize; i++) {
    //baseAddress += lengtharray[i];
    //for (int i = 0; i < numberofsegs; i++)
    actNumbOfPoints = lengtharray[i];
    //build splines for cycle i
    for (int j = 0; j < actNumbOfPoints-1; j++) {
      //cout << "j: " << j << ", constructing new segment from " << geompoints[j+baseAddress] << "," << geompoints[j+baseAddress+1] << ", baseAddress: " << baseAddress << endl;
      spline = new LineSegment(geompoints[j+baseAddress], geompoints[j+baseAddress+1]);
      spline->reffak = 1;
      //spline->leftdom = 1;
      spline->leftdom = directionarray[i];
      //spline->rightdom = 0;
      spline->rightdom = !directionarray[i];
      splines.Append(spline);

      Flags flags;
      splines.Last()->bc = int(flags.GetNumFlag("bc",j+baseAddress+1));
      splines.Last()->hpref_left = int(flags.GetDefineFlag("hpref")) ||
	int(flags.GetDefineFlag("hprefleft"));
      splines.Last()->hpref_right = int(flags.GetDefineFlag("hpref")) ||
	int(flags.GetDefineFlag("hprefright"));
      splines.Last()->copyfrom = int(flags.GetNumFlag("copy",-1));
    }//for j
    
    
    //set last spline
    //cout << "constructing final new segment from " << geompoints[baseAddress+actNumbOfPoints-1] << "," << geompoints[baseAddress] << ", baseAddress: " << baseAddress << endl;
    spline = new LineSegment(geompoints[baseAddress+actNumbOfPoints-1],geompoints[baseAddress]);
    spline->reffak = 1;
    //spline->leftdom = 1;
    spline->leftdom = directionarray[i];
    //spline->rightdom = 0;
    spline->rightdom = !directionarray[i];
    splines.Append(spline);
    
    //splines.Last()->bc = int(flags.GetNumFlag("bc",arrSize+1));
    splines.Last()->bc = int(flags.GetNumFlag("bc",actNumbOfPoints+1));
    splines.Last()->hpref_left = int(flags.GetDefineFlag("hpref")) ||
      int(flags.GetDefineFlag("hprefleft"));
    splines.Last()->hpref_right = int(flags.GetDefineFlag("hpref")) ||
      int(flags.GetDefineFlag("hprefright"));
    splines.Last()->copyfrom = int(flags.GetNumFlag("copy",-1));
    baseAddress += lengtharray[i];
  }//for i

}//end ConstructFromArray

void SplineGeometry2d :: 
PartitionBoundary (double h, Mesh & mesh2d)
{
  Box<2> bbox;
  GetBoundingBox (bbox);
  double dist = Dist (bbox.PMin(), bbox.PMax());
  Point<3> pmin(bbox.PMin()(0), bbox.PMin()(1), -dist);
  Point<3> pmax(bbox.PMax()(0), bbox.PMax()(1), dist);

  //cout << "searchtree from " << pmin << " to " << pmax << endl;
  Point3dTree searchtree (pmin, pmax);
  
  for (int i = 0; i < splines.Size(); i++)
    if (splines[i]->copyfrom == -1)
      splines[i]->Partition(h, elto0, mesh2d, searchtree, i+1);
    else
      CopyEdgeMesh (splines[i]->copyfrom, i+1, mesh2d, searchtree);
}


void SplineGeometry2d :: CopyEdgeMesh (int from, int to, Mesh & mesh, Point3dTree & searchtree)
{
  int i, j, k;

  ARRAY<int, PointIndex::BASE> mappoints (mesh.GetNP());
  ARRAY<double, PointIndex::BASE> param (mesh.GetNP());
  mappoints = -1;
  param = 0;

  Point3d pmin, pmax;
  mesh.GetBox (pmin, pmax);
  double diam2 = Dist2(pmin, pmax);

  //cout << "copy edge, from = " << from << " to " << to << endl;
  
  for (i = 1; i <= mesh.GetNSeg(); i++)
    {
      const Segment & seg = mesh.LineSegment(i);
      if (seg.edgenr == from)
	{
	  mappoints.Elem(seg.p1) = 1;
	  param.Elem(seg.p1) = seg.epgeominfo[0].dist;

	  mappoints.Elem(seg.p2) = 1;
	  param.Elem(seg.p2) = seg.epgeominfo[1].dist;
	}
    }

  for (i = 1; i <= mappoints.Size(); i++)
    {
      if (mappoints.Get(i) != -1)
	{
	  Point<2> newp = splines.Get(to)->GetPoint (param.Get(i));
	  Point<3> newp3 (newp(0), newp(1), 0);
	  
	  int npi = -1;
	  
	  for (PointIndex pi = PointIndex::BASE; 
	       pi < mesh.GetNP()+PointIndex::BASE; pi++)
	    if (Dist2 (mesh.Point(pi), newp3) < 1e-12 * diam2)
	      npi = pi;
	  
	  if (npi == -1)
	    {
	      npi = mesh.AddPoint (newp3);
	      searchtree.Insert (newp3, npi);
	    }

	  mappoints.Elem(i) = npi;

	  mesh.GetIdentifications().Add (i, npi, to);
	}
    }

  // copy segments
  int oldnseg = mesh.GetNSeg();
  for (i = 1; i <= oldnseg; i++)
    {
      const Segment & seg = mesh.LineSegment(i);
      if (seg.edgenr == from)
	{
	  Segment nseg;
	  nseg.edgenr = to;
	  nseg.si = splines.Get(to)->bc;
	  nseg.p1 = mappoints.Get(seg.p1);
	  nseg.p2 = mappoints.Get(seg.p2);
	  nseg.domin = splines.Get(to)->leftdom;
	  nseg.domout = splines.Get(to)->rightdom;
	  
	  nseg.epgeominfo[0].edgenr = to;
	  nseg.epgeominfo[0].dist = param.Get(seg.p1);
	  nseg.epgeominfo[1].edgenr = to;
	  nseg.epgeominfo[1].dist = param.Get(seg.p2);
	  mesh.AddSegment (nseg);
	}
    }
}
  

void SplineGeometry2d :: 
GetBoundingBox (Box<2> & box) const
{
  if (!splines.Size())
    {
      box.Set (Point<2> (0,0));
      return;
    }

  ARRAY<Point<2> > points;
  for (int i = 0; i < splines.Size(); i++)
    {
      splines[i]->GetPoints (20, points);

      if (i == 0) box.Set(points[0]);
      for (int j = 0; j < points.Size(); j++)
	box.Add (points[j]);
    }
}

void SplineGeometry2d :: 
SetGrading (const double grading)
{ elto0 = grading;}

void SplineGeometry2d :: 
AppendPoint (const double x, const double y, const double reffac, const bool hpref)
{
  geompoints.Append (GeomPoint2d(x, y, reffac));
  geompoints.Last().hpref = hpref;
}


void SplineGeometry2d :: 
AppendSegment(SplineSegment * spline, const int leftdomain, const int rightdomain, 
	      const int bc, 
	      const double reffac, const bool hprefleft, const bool hprefright,
	      const int copyfrom)
{
  spline -> leftdom = leftdomain;
  spline -> rightdom = rightdomain;
  spline -> bc = (bc >= 0) ? bc : (splines.Size()+1);
  spline -> reffak = reffac;
  spline -> hpref_left = hprefleft;
  spline -> hpref_right = hprefright;
  spline -> copyfrom = copyfrom;
  
  splines.Append(spline);
}

void SplineGeometry2d :: 
AppendLineSegment (const int n1, const int n2, const int leftdomain, const int rightdomain,
		   const int bc, 
		   const double reffac, const bool hprefleft, const bool hprefright,
		   const int copyfrom)
{
  SplineSegment * spline = new LineSegment(geompoints[n1],geompoints[n2]);
  AppendSegment(spline,leftdomain,rightdomain,bc,reffac,hprefleft,hprefright,copyfrom);  
}
void SplineGeometry2d :: 
AppendSplineSegment (const int n1, const int n2, const int n3, const int leftdomain, const int rightdomain, 
		     const int bc,
		     const double reffac, const bool hprefleft, const bool hprefright,
		     const int copyfrom)
{
  SplineSegment * spline = new SplineSegment3(geompoints[n1],geompoints[n2],geompoints[n3]);
  AppendSegment(spline,leftdomain,rightdomain,bc,reffac,hprefleft,hprefright,copyfrom);
}
void SplineGeometry2d :: 
AppendCircleSegment (const int n1, const int n2, const int n3, const int leftdomain, const int rightdomain,
		     const int bc,  
		     const double reffac, const bool hprefleft, const bool hprefright,
		     const int copyfrom)
{
  SplineSegment * spline = new CircleSegment(geompoints[n1],geompoints[n2],geompoints[n3]);
  AppendSegment(spline,leftdomain,rightdomain,bc,reffac,hprefleft,hprefright,copyfrom);
}
void SplineGeometry2d :: 
AppendDiscretePointsSegment (const ARRAY< Point<2> > & points, const int leftdomain, const int rightdomain, 
			     const int bc, 
			     const double reffac, const bool hprefleft, const bool hprefright,
			     const int copyfrom)
{
  SplineSegment * spline = new DiscretePointsSegment(points);
  AppendSegment(spline,leftdomain,rightdomain,bc,reffac,hprefleft,hprefright,copyfrom);
}

}
