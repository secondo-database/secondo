class StarterPolTest {

    public static void main (String[] args) {
	//This class generates two intersecting polygons (27,18 vertices)
	//and computes intersection, minus and plus for them.

	Segment j01 = new Segment(100,100,600,100);
	Segment j02 = new Segment(600,100,600,600);
	Segment j03 = new Segment(100,100,100,600);
	Segment j04 = new Segment(100,600,600,600);
	
	Segment j05 = new Segment(200,200,200,300);
	Segment j06 = new Segment(200,200,300,200);
	Segment j07 = new Segment(300,200,300,300);
	Segment j08 = new Segment(300,300,200,300);
	
	Segment j09 = new Segment(200,400,200,500);
	Segment j10 = new Segment(200,400,300,400);
	Segment j11 = new Segment(200,500,300,500);
	Segment j12 = new Segment(300,500,300,400);
	
	Segment j13 = new Segment(400,200,500,200);
	Segment j14 = new Segment(400,200,400,300);
	Segment j15 = new Segment(400,300,500,300);
	Segment j16 = new Segment(500,300,500,200);
	
	Segment j17 = new Segment(400,400,400,500);
	Segment j18 = new Segment(400,400,500,400);
	Segment j19 = new Segment(500,400,400,500);

	//second component
	Segment j20 = new Segment(700,100,1000,100);
	Segment j21 = new Segment(700,100,700,600);
	Segment j22 = new Segment(700,600,1000,600);
	Segment j23 = new Segment(1000,600,1000,100);

	Segment j24 = new Segment(800,200,900,200);
	Segment j25 = new Segment(800,200,800,400);
	Segment j26 = new Segment(800,400,900,200);

	Segment j27 = new Segment(900,300,900,500);
	Segment j28 = new Segment(900,300,800,500);
	Segment j29 = new Segment(900,500,800,500);

	SegList demJ = new SegList();

	
	demJ.add(j01); demJ.add(j02); demJ.add(j03); demJ.add(j04);
	demJ.add(j05); demJ.add(j06); demJ.add(j07); demJ.add(j08);
	demJ.add(j09); demJ.add(j10); demJ.add(j11); demJ.add(j12);
	demJ.add(j13); demJ.add(j14); demJ.add(j15); demJ.add(j16);
	demJ.add(j17); demJ.add(j18); demJ.add(j19);
	
	demJ.add(j20); demJ.add(j21); demJ.add(j22); demJ.add(j23);
	demJ.add(j24); demJ.add(j25); demJ.add(j26); demJ.add(j27);
	demJ.add(j28); demJ.add(j29);
	
	/*
	SegList demJ1 = new SegList();
	demJ1.add(j20); demJ1.add(j21); demJ1.add(j22); demJ1.add(j23);
	SegList demJ2 = new SegList();
	demJ2.add(j24); demJ2.add(j25); demJ2.add(j26);
	*/
	//System.out.println("lr_inside: "+Algebra.lr_inside(demJ2,Polygons.computeTriangles(demJ1)));

	
	//Polygons demP = new Polygons(demJ);
	Regions demP = new Regions(demJ);
	ElemListListList rList = demP.cyclesPoints();
	

	System.out.println("rList:"); rList.print();
	
	
	//demJ2.zoom(new Rational(0.5));
	//demJ1.zoom(new Rational(0.5));
	/*
	GFXout hh = new GFXout();
	hh.initWindow();
	hh.addList(Polygons.computeTriangles(demJ1));
	hh.addList(demJ2);
	hh.showIt();
	*/



	/*
	Triangle st01 = new Triangle(new Point(100,100),
				     new Point(200,100),
				     new Point(100,300));
	Triangle st02 = new Triangle(new Point(200,100),
				     new Point(100,300),
				     new Point(200,300));
	Triangle st03 = new Triangle(new Point(200,100),
				     new Point(200,300),
				     new Point(300,300));
	Triangle st04 = new Triangle(new Point(200,100),
				     new Point(300,100),
				     new Point(300,300));

	TriList stl = new TriList();
	stl.add(st01);
	stl.add(st02);
	stl.add(st03);
	stl.add(st04);
	SegList contour = Algebra.contour(stl);
	System.out.println("contour:"); contour.print();

	//System.exit(0);
	GFXout hji = new GFXout();
	hji.initWindow();
	//hji.addList(stl);
	hji.addList(contour);
	//hji.add(uz1);
	//hji.add(uz2);
	hji.showIt();
	*/


	/*
	Segment s1 = new Segment(new Rational(77,10),
				 new Rational(2725,10),
				 new Rational(29828967,948035),
				 new Rational(254365608,948035));
	Triangle z1 = new Triangle(new Point(601.3,7.7),
				   new Point(340.9,686),
				   new Point(345.8,21));
	Triangle z2 = new Triangle(new Point(379.2,334.4),
				   new Point(12,400),
				   new Point(171.2,584));

	System.out.println("pintersects:"+SegTri_Ops.pintersects(s1,z1));
	System.exit(0);
	Rational fact = new Rational(1);
	z1.zoom(fact);
	z2.zoom(fact);
	//Segment nz = SegTri_Ops.intersection(z1,z2);
	//TriList nz = z1.intersection(z2);
	
	GFXout h = new GFXout();
	h.initWindow();
	h.add(s1);
	h.add(z1);
	//h.add(new Point(282,449));
	//h.add(new Segment(115,357,148,375));
	//h.add(new Segment(609,102,519,193));
	h.showIt();
	*/




	/*
	Segment seg11 = new Segment(100,100,300,300);
	Segment seg12 = new Segment(-100,50,500,50);
	Segment seg13 = new Segment(500,50,700,50);
	Segment seg21 = new Segment(200,200,300,300);
	Segment seg22 = new Segment(0,50,100,50);
	Segment seg31 = new Segment(400,100,600,100);
	SegList segl01 = new SegList();
	SegList segl02 = new SegList();


	segl01.add(seg11);
	segl01.add(seg12);
	segl01.add(seg13);
	segl02.add(seg21);
	segl02.add(seg22);
	segl01.add(seg31);
	segl02.add(seg31);

	SegList segl03 = (SegList)segl01.clone();
	SetOps.quicksortX(segl03);
	System.out.println("segl01:");
	segl01.print();
	System.out.println("segl03:");
	segl03.print();
	System.exit(0);

	Lines resList = ROSEAlgebra.ll_minus(new Lines(segl01),new Lines(segl02));
	System.out.println("resList:");
	resList.seglist.print();
	System.exit(0);
*/



	/*
	Triangle r01 = new Triangle(new Point(225,240),
				    new Point(195,345),
				    new Point(195,270));
	Triangle r02 = new Triangle(new Point(270,420),
				    new Point(225,240),
				    new Point(240,375));
	Triangle r03 = new Triangle(new Point(285,240),
				    new Point(270,420),
				    new Point(225,240));
	TriList rl1 = new TriList();
	rl1.add(r01); rl1.add(r02); rl1.add(r03);
	
	Triangle r04 = new Triangle(new Point(240,300),
				    new Point(225,405),
				    new Point(195,210));
	Triangle r05 = new Triangle(new Point(285,360),
				    new Point(270,240),
				    new Point(225,405));
	TriList rl2 = new TriList();
	rl2.add(r04); rl2.add(r05);
	
	Algebra.rr_intersection(rl1,rl2);
	System.exit(0);
	
	GFXout gfx3 = new GFXout();
	gfx3.initWindow();
	//gfx3.addList(rl1);
	gfx3.addList(rl2);
	gfx3.showIt();
	*/

	/*
	Triangle t90 = new Triangle(new Point(225,240),
				    new Point(195,245),
				    new Point(195,270));
	Triangle t91 = new Triangle(new Point(240,300),
				    new Point(225,405),
				    new Point(195,210));
	System.out.println("pintersects:"+t90.pintersects(t91));
	System.exit(0);
	*/

	/*
	Triangle uz1 = new Triangle(new Point(195,210),
				    new Point(195,465),
				    new Point(120,240));
	Triangle uz2 = new Triangle(new Point(195,270),
				    new Point(195,345),
				    new Point(165,315));
	//TriList uzl = uz1.minus(uz2);
	//TriList uzl1 = new TriList();
	//uzl1.add(uz1);
	//TriList uzl2 = new TriList();
	//uzl2.add(uz2);
	//Polygon uzp1 = new Polygon(uzl1);
	//Polygon uzp2 = new Polygon(uzl2);
	
	//TriList retList = Algebra.rr_intersection(uzl1,uzl2);

	GFXout gfx0 = new GFXout();
	gfx0.initWindow();
	//gfx0.addList(retList);
	gfx0.add(uz1);
	gfx0.add(uz2);
	gfx0.showIt();
	*/	  

	/*	
	Triangle tt01 = new Triangle(new Point(new Rational(345,1),
					       new Rational(210,1)),
				     new Point(new Rational(330,1),
					       new Rational(195,1)),
				     new Point(new Rational(345,1),
					       new Rational(495,2)));
	Triangle tt02 = new Triangle(new Point(345,165),
				     new Point(345,270),
				     new Point(285,105));
	*/
	/*
	Triangle tt01 = new Triangle(new Point(270,420),
				     new Point(225,240),
				     new Point(240,375));
	Triangle tt02 = new Triangle(new Point(240,300),
				     new Point(225,405),
				     new Point(195,210));
	
	
	System.out.println("pintersects:"+tt01.pintersects(tt02));
	System.out.println("minus:");
	TriList ttl = tt01.minus(tt02);ttl.print();
	
	//System.exit(0);
	
	//resize for display
	tt01.print();
	tt02.print();
	Rational fac1 = new Rational(1);
	tt01.set(new Point(tt01.vertices[0].x.times(fac1),tt01.vertices[0].y.times(fac1)),
		 new Point(tt01.vertices[1].x.times(fac1),tt01.vertices[1].y.times(fac1)),
		 new Point(tt01.vertices[2].x.times(fac1),tt01.vertices[2].y.times(fac1)));
	tt02.set(new Point(tt02.vertices[0].x.times(fac1),tt02.vertices[0].y.times(fac1)),
		 new Point(tt02.vertices[1].x.times(fac1),tt02.vertices[1].y.times(fac1)),
		 new Point(tt02.vertices[2].x.times(fac1),tt02.vertices[2].y.times(fac1)));
	tt01.print();
	tt02.print();
	
	
	GFXout gfx1 = new GFXout();
	gfx1.initWindow();
	gfx1.add(tt01);
	gfx1.add(tt02);
	gfx1.showIt();
	*/
	/*
	Segment u01 = new Segment(200,100,100,200);
	Segment u02 = new Segment(100,200,100,400);
	Segment u03 = new Segment(100,400,400,300);
	Segment u04 = new Segment(400,300,200,100);

	Segment v01 = new Segment(300,300,300,400);
	Segment v02 = new Segment(300,400,300,500);
	Segment v03 = new Segment(300,500,400,500);
	Segment v04 = new Segment(400,500,500,400);
	Segment v05 = new Segment(500,400,300,300);

	SegList ul = new SegList();
	ul.add(u01); ul.add(u02); ul.add(u03); ul.add(u04);
	SegList vl = new SegList();
	vl.add(v01); vl.add(v02); vl.add(v03); vl.add(v04); vl.add(v05);
	
	Polygons polU = new Polygons(ul);
	Polygons polV = new Polygons(vl);

	//polU.trilist.print();
	//polV.trilist.print();
	//System.exit(0);

	//TriList retListUV = Algebra.rr_minus(polU.trilist,polV.trilist);
	//TriList retListUV = Algebra.rr_minus(polV.trilist,polU.trilist);
	TriList retListUV = Algebra.rr_intersection(polU.trilist,polV.trilist);
	retListUV.print();
	System.exit(0);
	
	GFXout gfx = new GFXout();
	gfx.initWindow();
	//gfx.addList(ul);
	//gfx.addList(vl);
	gfx.addList(retListUV);
	gfx.showIt();
	*/
	
	/*
	Segment s01 = new Segment(300,1050,400,1300);
	Segment s02 = new Segment(400,1300,500,1550);
	Segment s03 = new Segment(500,1550,650,1550);
	Segment s04 = new Segment(650,1550,850,1550);
	Segment s05 = new Segment(850,1550,750,1500);
	Segment s06 = new Segment(750,1500,750,1350);
	Segment s07 = new Segment(750,1350,950,1200);
	Segment s08 = new Segment(950,1200,1050,1000);
	Segment s09 = new Segment(1050,1000,1200,1100);
	Segment s10 = new Segment(1200,1100,1200,1250);
	Segment s11 = new Segment(1200,1250,1300,1400);
	Segment s12 = new Segment(1300,1400,1450,1200);
	Segment s13 = new Segment(1450,1200,1450,1050);
	Segment s14 = new Segment(1450,1050,1450,900);
	Segment s15 = new Segment(1450,900,1300,900);
	Segment s16 = new Segment(1300,900,1150,900);
	Segment s17 = new Segment(1150,900,1250,750);
	Segment s18 = new Segment(1250,750,1150,550);
	Segment s19 = new Segment(1150,550,950,350);
	Segment s20 = new Segment(950,350,750,450);
	Segment s21 = new Segment(750,450,800,550);
	Segment s22 = new Segment(800,550,900,450);
	Segment s23 = new Segment(900,450,900,600);
	Segment s24 = new Segment(900,600,900,800);
	Segment s25 = new Segment(900,800,800,1000);
	Segment s26 = new Segment(800,1000,650,700);
	Segment s27 = new Segment(650,700,400,800);
	Segment s28 = new Segment(400,800,300,1050);
	
	SegList sl01 = new SegList();
	sl01.add(s01); sl01.add(s02); sl01.add(s03); sl01.add(s04);
	sl01.add(s05); sl01.add(s06); sl01.add(s07); sl01.add(s08);
	sl01.add(s09); sl01.add(s10); sl01.add(s11); sl01.add(s12);
	sl01.add(s13); sl01.add(s14); sl01.add(s15); sl01.add(s16);
	sl01.add(s17); sl01.add(s18); sl01.add(s19); sl01.add(s20);
	sl01.add(s21); sl01.add(s22); sl01.add(s23); sl01.add(s24);
	sl01.add(s25); sl01.add(s26); sl01.add(s27); sl01.add(s28);


	Segment t01 = new Segment(550,1050,650,1150);
	Segment t02 = new Segment(650,1150,600,1250);
	Segment t03 = new Segment(600,1250,800,1250);
	Segment t04 = new Segment(800,1250,900,1400);
	Segment t05 = new Segment(900,1400,1050,1150);
	Segment t06 = new Segment(1050,1150,1150,1250);
	Segment t07 = new Segment(1150,1250,1350,1250);
	Segment t08 = new Segment(1350,1250,1500,1300);
	Segment t09 = new Segment(1500,1300,1500,1150);
	Segment t10 = new Segment(1500,1150,1500,1050);
	Segment t11 = new Segment(1500,1050,1300,850);
	Segment t12 = new Segment(1300,850,1250,1000);
	Segment t13 = new Segment(1250,1000,1100,900);
	Segment t14 = new Segment(1100,900,1200,750);
	Segment t15 = new Segment(1200,750,1100,650);
	Segment t16 = new Segment(1100,650,950,800);
	Segment t17 = new Segment(950,800,750,800);
	Segment t18 = new Segment(750,800,650,900);
	Segment t19 = new Segment(650,900,550,1050);

	SegList sl02 = new SegList();
	sl02.add(t01); sl02.add(t02); sl02.add(t03); sl02.add(t04);
	sl02.add(t05); sl02.add(t06); sl02.add(t07); sl02.add(t08);
	sl02.add(t09); sl02.add(t10); sl02.add(t11); sl02.add(t12);
	sl02.add(t13); sl02.add(t14); sl02.add(t15); sl02.add(t16);
	sl02.add(t17); sl02.add(t18); sl02.add(t19);
	    
	//resize for display
	Rational factor = new Rational(0.3);
	for (int i = 0; i < sl01.size(); i++) {
	    Segment actSeg = (Segment)sl01.get(i);
	    sl01.set(i,actSeg.set(new Point(actSeg.startpoint.x.times(factor),
					    actSeg.startpoint.y.times(factor)),
				  new Point(actSeg.endpoint.x.times(factor),
					    actSeg.endpoint.y.times(factor))));
	}//for i
	for (int i = 0; i < sl02.size(); i++) {
	    Segment actSeg2 = (Segment)sl02.get(i);
	    sl02.set(i,actSeg2.set(new Point(actSeg2.startpoint.x.times(factor),
					    actSeg2.startpoint.y.times(factor)),
				  new Point(actSeg2.endpoint.x.times(factor),
					    actSeg2.endpoint.y.times(factor))));
	}//for i

	Polygons pol01 = new Polygons(sl01);
	Polygons pol02 = new Polygons(sl02);
	//long time1 = System.currentTimeMillis();
	//TriList retList = Algebra.rr_minus(pol02.trilist,pol01.trilist);
	//long time2 = System.currentTimeMillis();
	//long diff = time2-time1;
	//System.out.println("all computation done... time:"+(diff/1000)+"."+(diff%1000)+" s");
	//Polygon retPol = new Polygon(retList);
	//System.out.println("generated polygon...");
	
	TriList retList = Algebra.rr_intersection(pol01.trilist,pol02.trilist);
	
	GFXout gfx = new GFXout();
	gfx.initWindow();
	//gfx.addList(sl01);
	//gfx.addList(Polygons.computeTriangles(sl02));
	//gfx.addList(sl02);
	//gfx.addList(Polygons.computeTriangles(sl01));
	//gfx.addList(sl01);
	gfx.addList(retList);
	//gfx.addList(retPol.border);
	gfx.showIt();
	*/

    }//end method main
}//end class StarterPolTest
