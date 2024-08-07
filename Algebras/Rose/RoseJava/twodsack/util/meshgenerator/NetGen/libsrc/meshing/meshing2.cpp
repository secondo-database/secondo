#include <mystdlib.h>
#include "meshing.hpp"

namespace netgen
{
  static void glrender (int wait);


  // global variable for visualization

  static ARRAY<Point3d> locpoints;
  static ARRAY<int> legalpoints;
  static ARRAY<Point2d> plainpoints;
  static ARRAY<int> plainzones;
  static ARRAY<INDEX_2> loclines;
  static int geomtrig;
  //static const char * rname;
  static int cntelem, trials, nfaces;
  static int oldnl;
  static int qualclass;
 

  Meshing2 :: Meshing2 (const Box3d & aboundingbox)
  {
    boundingbox = aboundingbox;

    LoadRules (NULL);
    // LoadRules ("rules/quad.rls");
    // LoadRules ("rules/triangle.rls");

    adfront = new AdFront2(boundingbox);
    starttime = GetTime();
  }


  Meshing2 :: ~Meshing2 ()
  {
    delete adfront;
    for (int i = 0; i < rules.Size(); i++)
      delete rules[i];
  }

  void Meshing2 :: AddPoint (const Point3d & p, PointIndex globind, 
			     MultiPointGeomInfo * mgi)
  {
    // (*testout) << "add point " << globind << endl;
    adfront ->AddPoint (p, globind, mgi);
  }

  void Meshing2 :: AddBoundaryElement (int i1, int i2,
				       const PointGeomInfo & gi1, const PointGeomInfo & gi2)
  {
    //    (*testout) << "add line " << i1 << " - " << i2 << endl;
    if (!gi1.trignum || !gi2.trignum)
      {
	PrintSysError ("addboundaryelement: illegal geominfo");
      }
    adfront -> AddLine (i1, i2, gi1, gi2);
  }



  void Meshing2 :: StartMesh ()
  {
    foundmap.SetSize (rules.Size());
    canuse.SetSize (rules.Size());
    ruleused.SetSize (rules.Size());

    foundmap = 0;
    canuse = 0;
    ruleused = 0;

    cntelem = 0;
    trials = 0;
  }

  void Meshing2 :: EndMesh ()
  {
    for (int i = 0; i < ruleused.Size(); i++)
      (*testout) << std::setw(4) << ruleused[i]
		 << " times used rule " << rules[i] -> Name() << std::endl;
  }

  void Meshing2 :: SetStartTime (double astarttime)
  {
    starttime = astarttime;
  }

  double Meshing2 :: CalcLocalH (const Point3d & /* p */, double gh) const
  {
    return gh;
  }



  // should be class variables !!(?)
  static Vec3d ex, ey;
  static Point3d globp1;

  void Meshing2 :: DefineTransformation (Point3d & p1, Point3d & p2,
					 const PointGeomInfo * geominfo1,
					 const PointGeomInfo * geominfo2)
  {
    globp1 = p1;
    ex = p2 - p1;
    ex /= ex.Length();
    ey.X() = -ex.Y();
    ey.Y() =  ex.X();
    ey.Z() = 0;
  }

  void Meshing2 :: TransformToPlain (const Point3d & locpoint, 
				     const MultiPointGeomInfo & geominf,
				     Point2d & plainpoint, double h, int & zone)
  {
    Vec3d p1p (globp1, locpoint);

    //    p1p = locpoint - globp1;
    p1p /= h;
    plainpoint.X() = p1p * ex;
    plainpoint.Y() = p1p * ey;
    zone = 0;
  }

  int Meshing2 :: TransformFromPlain (Point2d & plainpoint,
				      Point3d & locpoint, 
				      PointGeomInfo & gi, 
				      double h)
  {
    Vec3d p1p;
    gi.trignum = 1;

    p1p = plainpoint.X() * ex + plainpoint.Y() * ey;
    p1p *= h;
    locpoint = globp1 + p1p;
    return 0;
  }


  int Meshing2 :: BelongsToActiveChart (const Point3d & p, 
					const PointGeomInfo & gi)
  {
    return 1;
  }


  int Meshing2 :: ComputePointGeomInfo (const Point3d & p, PointGeomInfo & gi)
  {
    gi.trignum = 1;
    return 0;
  }


  int Meshing2 :: ChooseChartPointGeomInfo (const MultiPointGeomInfo & mpgi, 
					    PointGeomInfo & pgi)
  {
    pgi = mpgi.GetPGI(1);
    return 0;
  }



  int Meshing2 :: 
  IsLineVertexOnChart (const Point3d & p1, const Point3d & p2,
		       int endpoint, const PointGeomInfo & geominfo)
  {
    return 1;
  }

  void Meshing2 ::
  GetChartBoundary (ARRAY<Point2d> & points, 
		    ARRAY<Point3d> & points3d, 
		    ARRAY<INDEX_2> & lines, double h) const
  {
    points.SetSize (0);
    points3d.SetSize (0);
    lines.SetSize (0);
  }

  double Meshing2 :: Area () const
  {
    return -1;
  }





  MESHING2_RESULT Meshing2 :: GenerateMesh (Mesh & mesh, double gh, int facenr)
  {
    ARRAY<int> pindex, lindex;
    ARRAY<int> delpoints, dellines;

    ARRAY<PointGeomInfo> upgeominfo;  // unique info
    ARRAY<MultiPointGeomInfo> mpgeominfo;  // multiple info

    ARRAY<Element2d> locelements;

    int i, k, z1, z2, j, oldnp;
    SurfaceElementIndex sei;
    int baselineindex;
    bool found;
    int rulenr;
    int globind;
    Point3d p1, p2;

    const PointGeomInfo * blgeominfo1;
    const PointGeomInfo * blgeominfo2;

    bool morerisc;
    bool debugflag;

    double h, his, hshould;


    // test for 3d overlaps
    Box3dTree surfeltree (boundingbox.PMin(),
			  boundingbox.PMax());

    ARRAY<int> intersecttrias;
    ARRAY<Point3d> critpoints;

    // test for doubled edges
    //INDEX_2_HAHTABLE<int> doubleedge(300000);


    testmode = 0;

    StartMesh();

    ARRAY<Point2d> chartboundpoints;
    ARRAY<Point3d> chartboundpoints3d;
    ARRAY<INDEX_2> chartboundlines;

    // illegal points: points with more then 50 elements per node
    int maxlegalpoint, maxlegalline;
    ARRAY<int,PointIndex::BASE> trigsonnode;
    ARRAY<int,PointIndex::BASE> illegalpoint;

    trigsonnode.SetSize (mesh.GetNP());
    illegalpoint.SetSize (mesh.GetNP());

    trigsonnode = 0;
    illegalpoint = 0;
  

    double totalarea = Area ();
    double meshedarea = 0;

    std::cout << "searchtree..." << std::endl;
    // search tree for surface elements:
    for (sei = 0; sei < mesh.GetNSE(); sei++)
      {
	const Element2d & sel = mesh[sei];

	if (sel.IsDeleted())  continue;

	if (sel.GetIndex() == facenr)
	  {
	    const Point3d & sep1 = mesh[sel.PNum(1)];
	    const Point3d & sep2 = mesh[sel.PNum(2)];
	    const Point3d & sep3 = mesh[sel.PNum(3)];
	    Point3d sepmin(sep1), sepmax(sep2);
	    sepmin.SetToMin (sep2);
	    sepmin.SetToMin (sep3);
	    sepmin.SetToMax (sep2);
	    sepmin.SetToMax (sep3);

	    surfeltree.Insert (sepmin, sepmax, sei);
	  }

      
	double trigarea = Cross (Vec3d (mesh.Point (sel.PNum(1)),
					mesh.Point (sel.PNum(2))),
				 Vec3d (mesh.Point (sel.PNum(1)),
					mesh.Point (sel.PNum(3)))).Length() / 2;;

	if (sel.GetNP() == 4)
	  trigarea += Cross (Vec3d (mesh.Point (sel.PNum(1)),
				    mesh.Point (sel.PNum(3))),
			     Vec3d (mesh.Point (sel.PNum(1)),
				    mesh.Point (sel.PNum(4)))).Length() / 2;;
	meshedarea += trigarea;

      }


    char * savetask = multithread.task;
    multithread.task = "Surface meshing";

    adfront ->SetStartFront ();

    std::cout << "adfront..." << std::endl;
    int plotnexttrial = 999;
    //  starttime = GetTime();
    while (!adfront ->Empty()) //  && !multithread.terminate)
      {
	if (multithread.terminate)
	  throw NgException ("Meshing stopped");


	// known for STL meshing
	if (totalarea > 0)
	  multithread.percent = 100 * meshedarea / totalarea;
	/*
	  else
	  multithread.percent = 0;
	*/

	locpoints.SetSize(0);
	loclines.SetSize(0);
	pindex.SetSize(0);
	lindex.SetSize(0);
	delpoints.SetSize(0);
	dellines.SetSize(0);
	locelements.SetSize(0);



	// plot statistics
	if (trials > plotnexttrial)
	  {
	    /*
	    PrintMessage (5, 
			  "faces = ", nfaces,
			  " trials = ", trials,
			  " elements = ", mesh.GetNSE(),
			  " els/sec = ",
			  (mesh.GetNSE() / (GetTime() - starttime + 0.0001)));
	    plotnexttrial += 1000;
	    */
	  }


	// unique-pgi, multi-pgi
	upgeominfo.SetSize(0);
	mpgeominfo.SetSize(0);


	nfaces = adfront->GetNFL();
	trials ++;
    
	/* removed by Dirk Ansorge
	   if (trials % 1000 == 0)
	   {
	   (*testout) << "\n";
	   for (i = 1; i <= canuse.Size(); i++)
	   {
	   (*testout) << foundmap.Get(i) << "/" 
	   << canuse.Get(i) << "/"
	   << ruleused.Get(i) << " map/can/use rule " << rules.Get(i)->Name() << "\n";
	   }
	   (*testout) << "\n";
	   }
	*/

	baselineindex = adfront -> SelectBaseLine (p1, p2, blgeominfo1, 
                                             blgeominfo2, qualclass);

	found = 1;

	his = Dist (p1, p2);

	Point3d pmid = Center (p1, p2);

	hshould = CalcLocalH (pmid, mesh.GetH (pmid));
	if (gh < hshould)
	  hshould = gh;

	mesh.RestrictLocalH (pmid, hshould);

	h = hshould;

	double hinner = (3 + qualclass) * max2 (his, hshould);

	adfront ->GetLocals (baselineindex, locpoints, mpgeominfo, loclines, 
			     pindex, lindex, 2*hinner);
	

	if (qualclass > 200)
	  {
	    PrintMessage (3, "give up with qualclass ", qualclass);
	    PrintMessage (3, "number of frontlines = ", adfront->GetNFL());
	    // throw NgException ("Give up 2d meshing");
	    break;
	  }

	morerisc = 0;

	PointIndex gpi1 = adfront -> GetGlobalIndex (pindex.Get(loclines[0].I1()));
	PointIndex gpi2 = adfront -> GetGlobalIndex (pindex.Get(loclines[0].I2()));

	debugflag = 
	  debugparam.haltsegment &&
	  ( (debugparam.haltsegmentp1 == gpi1) && 
	    (debugparam.haltsegmentp2 == gpi2) || 
	    (debugparam.haltsegmentp1 == gpi2) && 
	    (debugparam.haltsegmentp2 == gpi1)) ||
	  debugparam.haltnode &&
	  ( (debugparam.haltsegmentp1 == gpi1) ||
	    (debugparam.haltsegmentp2 == gpi1));
	
	
	if (debugparam.haltface && debugparam.haltfacenr == facenr)
	  {
	    debugflag = 1;
	    std::cout << "set debugflag" << std::endl;
	  }
	
	if (debugparam.haltlargequalclass && qualclass > 50)
	  debugflag = 1;
	

	// problem recognition !
	if (found && 
	    (gpi1 < illegalpoint.Size()+PointIndex::BASE) && 
	    (gpi2 < illegalpoint.Size()+PointIndex::BASE) )
	  {
	    if (illegalpoint[gpi1] || illegalpoint[gpi2])
	      found = 0;
	  }


	Point2d p12d, p22d;

	//cout << "if found" << endl;
	if (found)
	  {
	    oldnp = locpoints.Size();
	    oldnl = loclines.Size();
	  
	    if (debugflag)
	      (*testout) << "define new transformation" << std::endl;

	    DefineTransformation (p1, p2, blgeominfo1, blgeominfo2);
	  
	    plainpoints.SetSize (locpoints.Size());
	    plainzones.SetSize (locpoints.Size());

	
	    for (i = 1; i <= locpoints.Size(); i++)
	      {
		TransformToPlain (locpoints.Get(i), 
				  mpgeominfo.Get(i),
				  plainpoints.Elem(i), h, plainzones.Elem(i));
	      }
	
	    p12d = plainpoints.Get(1);
	    p22d = plainpoints.Get(2);

	  
	    for (i = 2; i <= loclines.Size(); i++)  // don't remove first line
	      {
		z1 = plainzones.Get(loclines.Get(i).I1());
		z2 = plainzones.Get(loclines.Get(i).I2());
	      
	      
		// one inner point, one outer
		if ( (z1 >= 0) != (z2 >= 0))
		  {
		    int innerp = (z1 >= 0) ? 1 : 2;
		    if (IsLineVertexOnChart (locpoints.Get(loclines.Get(i).I1()),
					     locpoints.Get(loclines.Get(i).I2()),
					     innerp,
					     adfront->GetLineGeomInfo (lindex.Get(i), innerp)))
		      // pgeominfo.Get(loclines.Get(i).I(innerp))))
		      {		

			if (!morerisc)
			  {
			    // use one end of line
			    int pini, pouti;
			    Vec2d v;
			  
			    pini = loclines.Get(i).I(innerp);
			    pouti = loclines.Get(i).I(3-innerp);
			  
			    Point2d pin (plainpoints.Get(pini));
			    Point2d pout (plainpoints.Get(pouti));
			    v = pout - pin;
			    double len = v.Length();
			    if (len <= 1e-6)
			      (*testout) << "WARNING(js): inner-outer: short vector" << std::endl;
			    else
			      v /= len;
			  
			    Point2d newpout = pin + 1000 * v;
			    newpout = pout;

			  
			    plainpoints.Append (newpout);
			    Point3d pout3d = locpoints.Get(pouti);
			    locpoints.Append (pout3d);

			    plainzones.Append (0);
			    pindex.Append (0);
			    oldnp++;
			    loclines.Elem(i).I(3-innerp) = oldnp;
			  }
			else
			  plainzones.Elem(loclines.Get(i).I(3-innerp)) = 0;
			

			//		  (*testout) << "inner - outer correction" << endl;
		      }
		    else
		      {
			// remove line
			loclines.DeleteElement(i);
			lindex.DeleteElement(i);
			oldnl--;
			i--;
		      }			
		  }
	      
		else if (z1 > 0 && z2 > 0 && (z1 != z2) || (z1 < 0) && (z2 < 0) )
		  {
		    loclines.DeleteElement(i);
		    lindex.DeleteElement(i);
		    oldnl--;
		    i--;
		  }
	      }
	  


	    legalpoints.SetSize(plainpoints.Size());
	    for (i = 1; i <= legalpoints.Size(); i++)
	      legalpoints.Elem(i) = 1;


	    for (i = 1; i <= plainpoints.Size(); i++)
	      {
		if (plainzones.Elem(i) < 0)
		  {
		    plainpoints.Elem(i) = Point2d (1e4, 1e4);
		    legalpoints.Elem(i) = 0;
		  }
		if (pindex.Elem(i) == 0)
		  legalpoints.Elem(i) = 0;

		if (plainpoints.Elem(i).Y() < 0)
		  legalpoints.Elem(i) = 0;
	      }
	

	    GetChartBoundary (chartboundpoints, 
			      chartboundpoints3d,
			      chartboundlines, h);

	    oldnp = plainpoints.Size();

	    maxlegalpoint = locpoints.Size();
	    maxlegalline = loclines.Size();



	    if (mparam.checkchartboundary)
	      {
		for (i = 1; i <= chartboundpoints.Size(); i++)
		  {
		    plainpoints.Append (chartboundpoints.Get(i));
		    locpoints.Append (chartboundpoints3d.Get(i));
		    legalpoints.Append (0);
		  }
	      

		for (i = 1; i <= chartboundlines.Size(); i++)
		  {
		    INDEX_2 line (chartboundlines.Get(i).I1()+oldnp,
				  chartboundlines.Get(i).I2()+oldnp);
		    loclines.Append (line);
		  }
	      }

	    oldnl = loclines.Size();
	    oldnp = plainpoints.Size();
	  }

	//cout << "if found(2)" << endl;
	if (found)
	  {
	    rulenr = ApplyRules (plainpoints, legalpoints, maxlegalpoint,
				 loclines, maxlegalline, locelements,
				 dellines, qualclass);
	    //	    (*testout) << "Rule Nr = " << rulenr << endl;
	    if (!rulenr)
	      {
		found = 0;
		if ( debugflag || debugparam.haltnosuccess )
		  PrintWarning ("no rule found");
	      }
	  }
      
	
	for (i = 1; i <= locelements.Size() && found; i++)
	  {
	    const Element2d & el = locelements.Get(i);

	    for (j = 1; j <= el.GetNP(); j++)
	      if (el.PNum(j) <= oldnp && !pindex.Get(el.PNum(j)))
		{
		  found = 0;
		  PrintSysError ("meshing2, index missing");
		}
	  }

	//cout << "if found(3)" << endl;
	if (found)
	  {
	    locpoints.SetSize (plainpoints.Size());
	    upgeominfo.SetSize(locpoints.Size());

	    for (i = oldnp+1; i <= plainpoints.Size(); i++)
	      {
		int err =
		  TransformFromPlain (plainpoints.Elem(i), locpoints.Elem(i), 
				      upgeominfo.Elem(i), h);

		if (err)
		  {
		    found = 0;

		    if ( debugflag || debugparam.haltnosuccess )
		      PrintSysError ("meshing2, Backtransformation failed");

		    break;
		  }
	      }
	  }
	  

	//cout << "if found(4)" << endl;
	if (found) 
	  {
	    double violateminh = 3 + 0.1 * sqr (qualclass);
	    double minh = 1e8;
	    double newedgemaxh = 0;
	    for (i = oldnl+1; i <= loclines.Size(); i++)
	      {
		double eh = Dist (locpoints.Get(loclines.Get(i).I1()),
				  locpoints.Get(loclines.Get(i).I2()));
		if (eh > newedgemaxh)
		  newedgemaxh = eh;
	      }

	    for (i = 1; i <= locelements.Size(); i++)
	      {
		Point3d pmin = locpoints.Get(locelements.Get(i).PNum(1));
		Point3d pmax = pmin;
		for (j = 2; j <= locelements.Get(i).GetNP(); j++)
		  {
		    const Point3d & hp = 
		      locpoints.Get(locelements.Get(i).PNum(j));
		    pmin.SetToMin (hp);
		    pmax.SetToMax (hp);
		  }
		double eh = mesh.GetMinH (pmin, pmax);
		if (eh < minh)
		  minh = eh;
	      }

	    for (i = 1; i <= locelements.Size(); i++)
	      for (j = 1; j <= locelements.Get(i).GetNP(); j++)
		if (Dist2 (locpoints.Get(locelements.Get(i).PNum(j)), pmid) > hinner*hinner)
		  found = 0;

	    //	  cout << "violate = " << newedgemaxh / minh << endl;
	    static double maxviolate = 0;
	    if (newedgemaxh / minh > maxviolate)
	      {
		maxviolate = newedgemaxh / minh;
		//	      cout << "max minhviolate = " << maxviolate << endl;
	      }


	    if (newedgemaxh > violateminh * minh)
	      {
		found = 0;
		loclines.SetSize (oldnl);
		locpoints.SetSize (oldnp);

		if ( debugflag || debugparam.haltnosuccess )
		  PrintSysError ("meshing2, maxh too large");


	      }
	  }


	//cout << "if found(5)" << endl;
	// changed for OCC meshing
	if (found)
	  {
	    // take geominfo from dellines
	    // upgeominfo.SetSize(locpoints.Size());


	    for (i = 1; i <= locelements.Size(); i++)
	      for (j = 1; j <= locelements.Get(i).GetNP(); j++)
		{
		  int pi = locelements.Get(i).PNum(j);
		  if (pi <= oldnp)
		    {
		    
		      if (ChooseChartPointGeomInfo (mpgeominfo.Get(pi), upgeominfo.Elem(pi)))
			{
			  // cannot select, compute new one
			  PrintWarning ("calc point geominfo instead of using");
			  if (ComputePointGeomInfo (locpoints.Get(pi), upgeominfo.Elem(pi)))
			    {
			      found = 0;
			      PrintSysError ("meshing2d, geominfo failed");
			    }
			}
		    }
		}

	  }


	//cout << "if found(6)" << endl;
	if (found && mparam.checkoverlap)
	  {
	    // cout << "checkoverlap" << endl;
	    // test for overlaps
	  
	    Point3d hullmin(1e10, 1e10, 1e10);
	    Point3d hullmax(-1e10, -1e10, -1e10);
	  
	    for (i = 1; i <= locelements.Size(); i++)
	      for (j = 1; j <= locelements.Get(i).GetNP(); j++)
		{
		  const Point3d & p = locpoints.Get(locelements.Get(i).PNum(j));
		  hullmin.SetToMin (p);
		  hullmax.SetToMax (p);
		}
	    hullmin += Vec3d (-his, -his, -his);
	    hullmax += Vec3d ( his,  his,  his);

	    surfeltree.GetIntersecting (hullmin, hullmax, intersecttrias);

	    critpoints.SetSize (0);
	    for (i = oldnp+1; i <= locpoints.Size(); i++)
	      critpoints.Append (locpoints.Get(i));

	    for (i = 1; i <= locelements.Size(); i++)
	      {
		const Element2d & tri = locelements.Get(i);
		if (tri.GetNP() == 3)
		  {
		    const Point3d & tp1 = locpoints.Get(tri.PNum(1));
		    const Point3d & tp2 = locpoints.Get(tri.PNum(2));
		    const Point3d & tp3 = locpoints.Get(tri.PNum(3));
		  
		    Vec3d tv1 (tp1, tp2);
		    Vec3d tv2 (tp1, tp3);
		  
		    double lam1, lam2;
		    for (lam1 = 0.2; lam1 <= 0.8; lam1 += 0.2)
		      for (lam2 = 0.2; lam2 + lam1 <= 0.8; lam2 += 0.2)
			{
			  Point3d hp = tp1 + lam1 * tv1 + lam2 * tv2;
			  critpoints.Append (hp);
			}
		  }
		else if (tri.GetNP() == 4)
		  {
		    const Point3d & tp1 = locpoints.Get(tri.PNum(1));
		    const Point3d & tp2 = locpoints.Get(tri.PNum(2));
		    const Point3d & tp3 = locpoints.Get(tri.PNum(3));
		    const Point3d & tp4 = locpoints.Get(tri.PNum(4));
		  
		    double l1, l2;
		    for (l1 = 0.1; l1 <= 0.9; l1 += 0.1)
		      for (l2 = 0.1; l2 <= 0.9; l2 += 0.1)
			{
			  Point3d hp;
			  hp.X() = 
			    (1-l1)*(1-l2) * tp1.X() +
			    l1*(1-l2) * tp2.X() +
			    l1*l2 * tp3.X() +
			    (1-l1)*l2 * tp4.X();
			  hp.Y() = 
			    (1-l1)*(1-l2) * tp1.Y() +
			    l1*(1-l2) * tp2.Y() +
			    l1*l2 * tp3.Y() +
			    (1-l1)*l2 * tp4.Y();
			  hp.Z() = 
			    (1-l1)*(1-l2) * tp1.Z() +
			    l1*(1-l2) * tp2.Z() +
			    l1*l2 * tp3.Z() +
			    (1-l1)*l2 * tp4.Z();


			  critpoints.Append (hp);
			}
		  }
	      }

	    for (i = 1; i <= critpoints.Size(); i++)
	      {
		const Point3d & p = critpoints.Get(i);
		 

		int jj;
		for (jj = 1; jj <= intersecttrias.Size(); jj++)
		  {
		    j = intersecttrias.Get(jj);
		    const Element2d & el = mesh.SurfaceElement(j);
		  
		    int ntrig = (el.GetNP() == 3) ? 1 : 2;

		    int jl;
		    for (jl = 1; jl <= ntrig; jl++)
		      {
			Point3d tp1, tp2, tp3;

			if (jl == 1)
			  {
			    tp1 = mesh.Point(el.PNum(1));
			    tp2 = mesh.Point(el.PNum(2));
			    tp3 = mesh.Point(el.PNum(3));
			  }
			else
			  {
			    tp1 = mesh.Point(el.PNum(1));
			    tp2 = mesh.Point(el.PNum(3));
			    tp3 = mesh.Point(el.PNum(4));
			  }

			int onchart = 0;
			for (k = 1; k <= el.GetNP(); k++)
			  if (BelongsToActiveChart (mesh.Point(el.PNum(k)),
						    el.GeomInfoPi(k)))
			    onchart = 1;
			if (!onchart)
			  continue;
		      
			Vec3d e1(tp1, tp2);
			Vec3d e2(tp1, tp3);
			Vec3d n = Cross (e1, e2);
			n /= n.Length();
			double lam1, lam2, lam3;
			lam3 = n * Vec3d (tp1, p);
			LocalCoordinates (e1, e2, Vec3d (tp1, p), lam1, lam2);
		      
			if (fabs (lam3) < 0.1 * hshould && 
			    lam1 > 0 && lam2 > 0 && (lam1 + lam2) < 1)
			  {
#ifdef DEVELOP
			    cout << "overlap" << std::endl;
			    (*testout) << "overlap:" << std::endl
				       << "tri = " << tp1 << "-" << tp2 << "-" << tp3 << std::endl
				       << "point = " << p << std::endl
				       << "lam1, 2 = " << lam1 << ", " << lam2 << std::endl
				       << "lam3 = " << lam3 << std::endl;
			  
			    //		      cout << "overlap !!!" << endl;
#endif
			    for (int k = 1; k <= 5; k++)
			      adfront -> IncrementClass (lindex.Get(1));

			    found = 0;
			  
			    if ( debugflag || debugparam.haltnosuccess )
			      PrintWarning ("overlapping");
			  
			  
			    if (debugparam.haltoverlap)
			      {
				debugflag = 1;
			      }
			  
			    /*
			      multithread.drawing = 1;
			      glrender(1);
			    */
			  }
		      }
		  }
	      }
	  }

	//cout << "if found(7)" << endl;
	if (found)
	  {
	    // check, whether new front line already exists

	    for (i = oldnl+1; i <= loclines.Size(); i++)
	      {
		int nlgpi1 = loclines.Get(i).I1();
		int nlgpi2 = loclines.Get(i).I2();
		if (nlgpi1 <= pindex.Size() && nlgpi2 <= pindex.Size())
		  {
		    nlgpi1 = adfront->GetGlobalIndex (pindex.Get(nlgpi1));
		    nlgpi2 = adfront->GetGlobalIndex (pindex.Get(nlgpi2));

		    int exval = adfront->ExistsLine (nlgpi1, nlgpi2);
		    if (exval)
		      {
			std::cout << "ERROR: new line exits, val = " << exval << std::endl;
			(*testout) << "ERROR: new line exits, val = " << exval << std::endl;
			found = 0;


			if (debugparam.haltexistingline)
			  debugflag = 1;

		      }
		  }
	      }
	  
	  }


	//cout << "if found(8)" << endl;
	if (found)
	  {
	    // everything is ok, perform mesh update

	    ruleused.Elem(rulenr)++;


	    pindex.SetSize(locpoints.Size());
	      
	    for (i = oldnp+1; i <= locpoints.Size(); i++)
	      {
		globind = mesh.AddPoint (locpoints.Get(i));
		pindex.Elem(i) = adfront -> AddPoint (locpoints.Get(i), globind);
	      }
	      
	    for (i = oldnl+1; i <= loclines.Size(); i++)
	      {

		if (pindex.Get(loclines.Get(i).I1()) == 0 || 
		    pindex.Get(loclines.Get(i).I2()) == 0)
		  {
		    (*testout) << "pindex is 0" << std::endl;
		  }

		if (!upgeominfo.Get(loclines.Get(i).I1()).trignum || 
		    !upgeominfo.Get(loclines.Get(i).I2()).trignum)
		  {
		    std::cout << "new el: illegal geominfo" << std::endl;
		  }

		adfront -> AddLine (pindex.Get(loclines.Get(i).I1()),
				    pindex.Get(loclines.Get(i).I2()),
				    upgeominfo.Get(loclines.Get(i).I1()),
				    upgeominfo.Get(loclines.Get(i).I2()));
	      }
	    for (i = 1; i <= locelements.Size(); i++)
	      {
		Element2d mtri(locelements.Get(i).GetNP());
		mtri = locelements.Get(i);
		mtri.SetIndex (facenr);


		// compute triangle geominfo:
		//	      (*testout) << "triggeominfo: ";
		for (j = 1; j <= locelements.Get(i).GetNP(); j++)
		  {
		    mtri.GeomInfoPi(j) = upgeominfo.Get(locelements.Get(i).PNum(j));
		    //		  (*testout) << mtri.GeomInfoPi(j).trignum << " ";
		  }
		//	      (*testout) << endl;

		for (j = 1; j <= locelements.Get(i).GetNP(); j++)
		  {
		    mtri.PNum(j) = 
		      locelements.Elem(i).PNum(j) =
		      adfront -> GetGlobalIndex (pindex.Get(locelements.Get(i).PNum(j)));
		  }
	      
		
	      
	      
		mesh.AddSurfaceElement (mtri);
		cntelem++;
		//	      cout << "elements: " << cntelem << endl;


	      

		const Point3d & sep1 = mesh.Point (mtri.PNum(1));
		const Point3d & sep2 = mesh.Point (mtri.PNum(2));
		const Point3d & sep3 = mesh.Point (mtri.PNum(3));

		Point3d sepmin(sep1), sepmax(sep1);
		for (j = 2; j <= mtri.GetNP(); j++)
		  {
		    sepmin.SetToMin (mesh.Point (mtri.PNum(j)));
		    sepmax.SetToMax (mesh.Point (mtri.PNum(j)));
		  }

		surfeltree.Insert (sepmin, sepmax, mesh.GetNSE());


		double trigarea = Cross (Vec3d (sep1, sep2), 
					 Vec3d (sep1, sep3)).Length() / 2;

		if (mtri.GetNP() == 4)
		  {
		    const Point3d & sep4 = mesh.Point (mtri.PNum(4));
		    trigarea += Cross (Vec3d (sep1, sep3), 
				       Vec3d (sep1, sep4)).Length() / 2;
		  }

		meshedarea += trigarea;

	      


		for (j = 1; j <= locelements.Get(i).GetNP(); j++)
		  {
		    int gpi = locelements.Get(i).PNum(j);

		    int oldts = trigsonnode.Size();
		    if (gpi >= oldts+PointIndex::BASE)
		      {
			trigsonnode.SetSize (gpi+1-PointIndex::BASE);
			illegalpoint.SetSize (gpi+1-PointIndex::BASE);
			for (k = oldts+PointIndex::BASE; 
			     k <= gpi; k++)
			  {
			    trigsonnode[k] = 0;
			    illegalpoint[k] = 0;
			  }
		      }

		    trigsonnode[gpi]++;
		  
		    if (trigsonnode[gpi] > 20)
		      {
			illegalpoint[gpi] = 1;
			//		      cout << "illegal point: " << gpi << endl;
			(*testout) << "illegal point: " << gpi << std::endl;
		      }

		    static int mtonnode = 0;
		    if (trigsonnode[gpi] > mtonnode)
		      mtonnode = trigsonnode[gpi];
		  }
		//	      cout << "els = " << cntelem << " trials = " << trials << endl;
		//	      if (trials > 100)		return;
	      }
	      
	    for (i = 1; i <= dellines.Size(); i++)
	      adfront -> DeleteLine (lindex.Get(dellines.Get(i)));
	      
	    //	  rname = rules.Get(rulenr)->Name();
#ifdef MYGRAPH
	    if (silentflag<3) 
	      {
		plotsurf.DrawPnL(locpoints, loclines);
		plotsurf.Plot(testmode, testmode);
	      }
#endif

	    if (morerisc)
	      {
		std::cout << "generated due to morerisc" << std::endl;
		//	      multithread.drawing = 1;
		//	      glrender(1);
	      }



	  
	    if ( debugparam.haltsuccess || debugflag )
	      {
		std::cout << "success of rule" << rules.Get(rulenr)->Name() << std::endl;
		multithread.drawing = 1;
		multithread.testmode = 1;
		multithread.pause = 1;


		(*testout) << "success of rule" << rules.Get(rulenr)->Name() << std::endl;
		(*testout) << "trials = " << trials << std::endl;

		(*testout) << "old number of lines = " << oldnl << std::endl;
		for (i = 1; i <= loclines.Size(); i++)
		  {
		    (*testout) << "line ";
		    for (j = 1; j <= 2; j++)
		      {
			int hi = 0;
			if (loclines.Get(i).I(j) >= 1 &&
			    loclines.Get(i).I(j) <= pindex.Size())
			  hi = adfront->GetGlobalIndex (pindex.Get(loclines.Get(i).I(j)));

			(*testout) << hi << " ";
		      }
		    (*testout) << " : " 
			       << plainpoints.Get(loclines.Get(i).I1()) << " - "
			       << plainpoints.Get(loclines.Get(i).I2()) << " 3d: "
			       << locpoints.Get(loclines.Get(i).I1()) << " - "
			       << locpoints.Get(loclines.Get(i).I2()) 
			       << std::endl;
		  }



		glrender(1);
	      }
	  }
	else
	  {
	    adfront -> IncrementClass (lindex.Get(1));

	    if ( debugparam.haltnosuccess || debugflag )
	      {
		std::cout << "Problem with seg " << gpi1 << " - " << gpi2
		     << ", class = " << qualclass << std::endl;

		(*testout) << "Problem with seg " << gpi1 << " - " << gpi2
			   << ", class = " << qualclass << std::endl;

		multithread.drawing = 1;
		multithread.testmode = 1;
		multithread.pause = 1;
		/* removed by Dirk Ansorge
		   for (i = 1; i <= loclines.Size(); i++)
		   {
		   (*testout) << "line ";
		   for (j = 1; j <= 2; j++)
		   {
		   int hi = 0;
		   if (loclines.Get(i).I(j) >= 1 &&
		   loclines.Get(i).I(j) <= pindex.Size())
		   hi = adfront->GetGlobalIndex (pindex.Get(loclines.Get(i).I(j)));
		   
		   (*testout) << hi << " ";
		   }
		   (*testout) << " : " 
		   << plainpoints.Get(loclines.Get(i).I1()) << " - "
		   << plainpoints.Get(loclines.Get(i).I2()) << " 3d: "
		   << locpoints.Get(loclines.Get(i).I1()) << " - "
		   << locpoints.Get(loclines.Get(i).I2()) 
		   << endl;
		   }
		*/


		glrender(1);
	      }

	  
#ifdef MYGRAPH      
	    if (silentflag<3)
	      {
		if (testmode || trials%2 == 0)
		  {
		    plotsurf.DrawPnL(locpoints, loclines);
		    plotsurf.Plot(testmode, testmode);
		  }
	      }
#endif
	  }

      }

    //PrintMessage (3, "Surface meshing done");

    //removed by Dirk Ansorge
    //adfront->PrintOpenSegments (*testout);

    multithread.task = savetask;


    EndMesh ();

    if (!adfront->Empty())
      return MESHING2_GIVEUP;
    
    return MESHING2_OK;
  }









}







#ifdef OPENGL

/* *********************** Draw Surface Meshing **************** */


#include <visual.hpp>
#include <stlgeom.hpp>

namespace netgen 
{

  extern STLGeometry * stlgeometry;
  extern Mesh * mesh;
  VisualSceneSurfaceMeshing vssurfacemeshing;



  void glrender (int wait)
  {
    //  cout << "plot adfront" << endl;

    if (multithread.drawing)
      {
	//      vssurfacemeshing.Render();
	Render ();
      
	if (wait || multithread.testmode)
	  {
	    multithread.pause = 1;
	  }
	while (multithread.pause);
      }
  }



  VisualSceneSurfaceMeshing :: VisualSceneSurfaceMeshing ()
    : VisualScene()
  {
    ;
  }

  VisualSceneSurfaceMeshing :: ~VisualSceneSurfaceMeshing ()
  {
    ;
  }

  void VisualSceneSurfaceMeshing :: DrawScene ()
  {
    int i, j, k;

    if (loclines.Size() != changeval)
      {
	center = Point<3>(0,0,-5);
	rad = 0.1;
  
	CalcTransformationMatrices();
	changeval = loclines.Size();
      }

  glClearColor(backcolor, backcolor, backcolor, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  SetLight();

  //  glEnable (GL_COLOR_MATERIAL);

  //  glDisable (GL_SHADING);
  //  glColor3f (0.0f, 1.0f, 1.0f);
  //  glLineWidth (1.0f);
  //  glShadeModel (GL_SMOOTH);

  //  glCallList (linelists.Get(1));

  //  SetLight();

  glPushMatrix();
  glMultMatrixf (transformationmat);

  glShadeModel (GL_SMOOTH);
  glDisable (GL_COLOR_MATERIAL);
  glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  //  glEnable (GL_LIGHTING);

  double shine = vispar.shininess;
  double transp = vispar.transp;

  glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, shine);
  glLogicOp (GL_COPY);



  /*

  float mat_col[] = { 0.2, 0.2, 0.8, 1 };
  glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_col);

  glPolygonOffset (1, 1);
  glEnable (GL_POLYGON_OFFSET_FILL);

    float mat_colbl[] = { 0.8, 0.2, 0.2, 1 };
    float mat_cololdl[] = { 0.2, 0.8, 0.2, 1 };
    float mat_colnewl[] = { 0.8, 0.8, 0.2, 1 };


    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    glPolygonOffset (1, -1);
    glLineWidth (3);

    for (i = 1; i <= loclines.Size(); i++)
      {
	if (i == 1)
	  {
	    glEnable (GL_POLYGON_OFFSET_FILL);
	    glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_colbl);
	  }
	else if (i <= oldnl) 
	  glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_cololdl);
	else 
	  glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_colnewl);

	int pi1 = loclines.Get(i).I1();
	int pi2 = loclines.Get(i).I2();

	if (pi1 >= 1 && pi2 >= 1)
	  {
	    Point3d p1 = locpoints.Get(pi1);
	    Point3d p2 = locpoints.Get(pi2);
	  
	    glBegin (GL_LINES);
	    glVertex3f (p1.X(), p1.Y(), p1.Z());
	    glVertex3f (p2.X(), p2.Y(), p2.Z());
	    glEnd();
	  }

	glDisable (GL_POLYGON_OFFSET_FILL);
      }
  

    glLineWidth (1);


    glPointSize (5);
    float mat_colp[] = { 1, 0, 0, 1 };
    glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_colp);
    glBegin (GL_POINTS);
    for (i = 1; i <= locpoints.Size(); i++)
      {
	Point3d p = locpoints.Get(i);
	glVertex3f (p.X(), p.Y(), p.Z());
      }
    glEnd();


    glPopMatrix();
  */

    float mat_colp[] = { 1, 0, 0, 1 };

    float mat_col2d1[] = { 1, 0.5, 0.5, 1 };
    float mat_col2d[] = { 1, 1, 1, 1 };
    glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_col2d);
  
    double scalex = 0.1, scaley = 0.1;

    glBegin (GL_LINES);
    for (i = 1; i <= loclines.Size(); i++)
      {
	glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_col2d);
	if (i == 1)
	  glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_col2d1);

	int pi1 = loclines.Get(i).I1();
	int pi2 = loclines.Get(i).I2();

	if (pi1 >= 1 && pi2 >= 1)
	  {
	    Point2d p1 = plainpoints.Get(pi1);
	    Point2d p2 = plainpoints.Get(pi2);
	  
	    glBegin (GL_LINES);
	    glVertex3f (scalex * p1.X(), scaley * p1.Y(), -5);
	    glVertex3f (scalex * p2.X(), scaley * p2.Y(), -5);
	    glEnd();
	  }
      }
    glEnd ();


    glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_colp);
    glBegin (GL_POINTS);
    for (i = 1; i <= plainpoints.Size(); i++)
      {
	Point2d p = plainpoints.Get(i);
	glVertex3f (scalex * p.X(), scaley * p.Y(), -5);
      }
    glEnd();






  glDisable (GL_POLYGON_OFFSET_FILL);
 
  glPopMatrix();
  DrawCoordinateCross ();
  DrawNetgenLogo ();
  glFinish();  

  /*
    glDisable (GL_POLYGON_OFFSET_FILL);

    //  cout << "draw surfacemeshing" << endl;
    //
    //  if (changeval != stlgeometry->GetNT())
    //      BuildScene();
    //      changeval = stlgeometry->GetNT();
    

    glClearColor(backcolor, backcolor, backcolor, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    SetLight();

    glPushMatrix();
    glLoadMatrixf (transmat);
    glMultMatrixf (rotmat);

    glShadeModel (GL_SMOOTH);
    glDisable (GL_COLOR_MATERIAL);
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float mat_spec_col[] = { 1, 1, 1, 1 };
    glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, mat_spec_col);

    double shine = vispar.shininess;
    double transp = vispar.transp;

    glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, shine);
    glLogicOp (GL_COPY);


    float mat_col[] = { 0.2, 0.2, 0.8, transp };
    float mat_colrt[] = { 0.2, 0.8, 0.8, transp };
    glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_col);

    glPolygonOffset (1, 1);
    glEnable (GL_POLYGON_OFFSET_FILL);

    glColor3f (1.0f, 1.0f, 1.0f);

    glEnable (GL_NORMALIZE);
    
    //  glBegin (GL_TRIANGLES);
    //      for (j = 1; j <= stlgeometry -> GetNT(); j++)
    //      {
    //      glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_col);
    //      if (j == geomtrig)
    //      glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_colrt);
	

    //      const STLReadTriangle & tria = stlgeometry -> GetReadTriangle(j);
    //      glNormal3f (tria.normal.X(),
    //      tria.normal.Y(),
    //      tria.normal.Z());
		  
    //      for (k = 0; k < 3; k++)
    //      {
    //      glVertex3f (tria.pts[k].X(),
    //      tria.pts[k].Y(),
    //      tria.pts[k].Z());
    //      }
    //      }    
    //      glEnd ();
    


    glDisable (GL_POLYGON_OFFSET_FILL);

    float mat_colbl[] = { 0.8, 0.2, 0.2, 1 };
    float mat_cololdl[] = { 0.2, 0.8, 0.2, 1 };
    float mat_colnewl[] = { 0.8, 0.8, 0.2, 1 };


    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    glPolygonOffset (1, -1);
    glLineWidth (3);

    for (i = 1; i <= loclines.Size(); i++)
      {
	if (i == 1)
	  {
	    glEnable (GL_POLYGON_OFFSET_FILL);
	    glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_colbl);
	  }
	else if (i <= oldnl) 
	  glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_cololdl);
	else 
	  glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_colnewl);

	int pi1 = loclines.Get(i).I1();
	int pi2 = loclines.Get(i).I2();

	if (pi1 >= 1 && pi2 >= 1)
	  {
	    Point3d p1 = locpoints.Get(pi1);
	    Point3d p2 = locpoints.Get(pi2);
	  
	    glBegin (GL_LINES);
	    glVertex3f (p1.X(), p1.Y(), p1.Z());
	    glVertex3f (p2.X(), p2.Y(), p2.Z());
	    glEnd();
	  }

	glDisable (GL_POLYGON_OFFSET_FILL);
      }


    glLineWidth (1);


    glPointSize (5);
    float mat_colp[] = { 1, 0, 0, 1 };
    glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_colp);
    glBegin (GL_POINTS);
    for (i = 1; i <= locpoints.Size(); i++)
      {
	Point3d p = locpoints.Get(i);
	glVertex3f (p.X(), p.Y(), p.Z());
      }
    glEnd();


    glPopMatrix();


    float mat_col2d1[] = { 1, 0.5, 0.5, 1 };
    float mat_col2d[] = { 1, 1, 1, 1 };
    glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_col2d);
  
    double scalex = 0.1, scaley = 0.1;

    glBegin (GL_LINES);
    for (i = 1; i <= loclines.Size(); i++)
      {
	glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_col2d);
	if (i == 1)
	  glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_col2d1);

	int pi1 = loclines.Get(i).I1();
	int pi2 = loclines.Get(i).I2();

	if (pi1 >= 1 && pi2 >= 1)
	  {
	    Point2d p1 = plainpoints.Get(pi1);
	    Point2d p2 = plainpoints.Get(pi2);
	  
	    glBegin (GL_LINES);
	    glVertex3f (scalex * p1.X(), scaley * p1.Y(), -5);
	    glVertex3f (scalex * p2.X(), scaley * p2.Y(), -5);
	    glEnd();
	  }
      }
    glEnd ();


    glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_colp);
    glBegin (GL_POINTS);
    for (i = 1; i <= plainpoints.Size(); i++)
      {
	Point2d p = plainpoints.Get(i);
	glVertex3f (scalex * p.X(), scaley * p.Y(), -5);
      }
    glEnd();

    glFinish();  
*/
  }


  void VisualSceneSurfaceMeshing :: BuildScene (int zoomall)
  {
    int i, j, k;
    /*
      center = stlgeometry -> GetBoundingBox().Center();
      rad = stlgeometry -> GetBoundingBox().Diam() / 2;

      CalcTransformationMatrices();
    */
  }

}


#else
namespace netgen
{
  void glrender (int wait)
  { ; }
}
#endif
