import java.util.*;
import java.lang.reflect.*;

class Starter {

    public static void main (String[] args) {

	Segment q01 = new Segment(150,450,250,500);
	Segment q02 = new Segment(150,450,100,600);
        Triangle r01 = new Triangle(q01,q02);
	Segment q03 = new Segment(150,450,350,550);
	Segment q04 = new Segment(150,450,100,750);
	Triangle r02 = new Triangle(q03,q04);
	Triangle r03 = new Triangle(new Segment(150,150,350,200),new Segment(350,200,250,400));
	Triangle r04 = new Triangle(new Segment(250,100,400,300),new Segment(400,300,150,300));
	Triangle r05 = new Triangle(new Segment(750,550,750,850),new Segment(750,550,1050,600));
	Triangle r06 = new Triangle(new Segment(750,600,750,750),new Segment(750,750,1200,550));
	Triangle r07 = new Triangle(new Segment(100,950,400,1100),new Segment(100,950,400,800));
	Triangle r08 = new Triangle(new Segment(200,900,400,950),new Segment(400,950,400,800));
	//resize
	Rational factor = new Rational(1);
	r01.set(new Point(r01.vertices[0].x.times(factor),r01.vertices[0].y.times(factor)),
		new Point(r01.vertices[1].x.times(factor),r01.vertices[1].y.times(factor)),
		new Point(r01.vertices[2].x.times(factor),r01.vertices[2].y.times(factor)));

	r02.set(new Point(r02.vertices[0].x.times(factor),r02.vertices[0].y.times(factor)),
		new Point(r02.vertices[1].x.times(factor),r02.vertices[1].y.times(factor)),
		new Point(r02.vertices[2].x.times(factor),r02.vertices[2].y.times(factor)));
	

	//System.out.println("---------->retList01");
	//TriList retList01 = r01.minus(r02);
	//System.out.println("---------->retList11");
	//TriList retList11 = r01.intersection(r02);
	System.out.println("---------->retList21");
	TriList retList21 = r01.plus(r02);
	//System.out.println("---------->retList02");
	//TriList retList02 = r02.minus(r01);
	//System.out.println("---------->retList12");
	//TriList retList12 = r02.intersection(r01);
	//System.out.println("---------->retList03");
	//TriList retList03 = r03.minus(r04);
	//System.out.println("--------->retList13");
	//TriList retList13 = r03.intersection(r04);
	System.out.println("---------->retList23");
	TriList retList23 = r03.plus(r04);
	//System.out.println("---------->retList04");
	//TriList retList04 = r04.minus(r03);
	//System.out.println("---------->retList05");
	//TriList retList05 = r05.minus(r06);
	//System.out.println("--------->retList15");
	//TriList retList15 = r05.intersection(r06);
	System.out.println("---------->retList25");
	TriList retList25 = r05.plus(r06);
	//System.out.println("---------->retList06");
	//TriList retList06 = r06.minus(r05);
	//System.out.println("---------->retList07");
	//TriList retList07 = r07.minus(r08);
	//System.out.println("--------->retList17");
	//TriList retList17 = r07.intersection(r08);
	System.out.println("---------->retList28");
	TriList retList28 = r07.plus(r08);
	//System.out.println("---------->retList08");
	//TriList retList08 = r08.minus(r07);
	//System.out.println();
	System.out.println("*************************");
	//System.out.println("retList01"); retList01.print();
	//System.out.println("retList11"); retList11.print();
	System.out.println("retList21"); retList21.print();
	//System.out.println("retList02"); retList02.print();
	//System.out.println("retList12"); retList12.print();
	//System.out.println("retList03"); retList03.print();
	//System.out.println("retList13"); retList13.print();
	System.out.println("retList23"); retList23.print();
	//System.out.println("retList04"); retList04.print();
	//System.out.println("retList05"); retList05.print();
	//System.out.println("retList15"); retList15.print();
	System.out.println("retList25"); retList25.print();
	//System.out.println("retList06"); retList06.print();
	//System.out.println("retList07"); retList07.print();
	//System.out.println("retList17"); retList17.print();
	System.out.println("retList28"); retList28.print();
	//System.out.println("retList08"); retList08.print();


	/*
	GFXout gfx = new GFXout();
	gfx.initWindow();
	gfx.add(r07);
	gfx.add(r08);
	//gfx.addList(retList13);
	gfx.showIt();
	*/


	/*
	Class c = (new SegSeg_Ops()).getClass();
	Class c2 = (new Segment()).getClass();
	Class[] paramList = { c2, c2 };
	PairList retList = new PairList();
	
	try {
	    Method m = c.getMethod("formALine",paramList);
	    retList = SetOps.join(ql01,ql01,m);
	}//try
	catch (Exception e) {
	    System.out.println("ERROR!");
	    System.exit(0);
	}//catch
	Graph g = new Graph(retList);
	//g.print();
	ElemListList comp = g.connectedComponents();
	comp.print();

	System.exit(0);
	*/

	/*
	//country's border
	Segment cb01 = new Segment(150,250,250,200);
	Segment cb02 = new Segment(250,200,350,100);
	Segment cb03 = new Segment(350,100,500,200);
	Segment cb04 = new Segment(500,200,550,350);
	Segment cb05 = new Segment(550,350,700,300);
	Segment cb06 = new Segment(700,300,750,200);
	Segment cb07 = new Segment(750,200,800,450);
	Segment cb08 = new Segment(800,450,650,600);
	Segment cb09 = new Segment(650,600,500,500);
	Segment cb10 = new Segment(500,500,350,600);
	Segment cb11 = new Segment(350,600,350,400);
	Segment cb12 = new Segment(350,400,150,250);
	
	SegList coBo = new SegList();
	coBo.add(cb01); coBo.add(cb02); coBo.add(cb03); coBo.add(cb04); coBo.add(cb05);
      	coBo.add(cb06); coBo.add(cb07); coBo.add(cb08); coBo.add(cb09); coBo.add(cb10);
	coBo.add(cb11); coBo.add(cb12);

	Polygon country = new Polygon(coBo);

	//ways
	Segment w01 = new Segment(100,300,200,400);
	Segment w02 = new Segment(200,400,200,200);
	Segment w03 = new Segment(200,400,300,300);
	Segment w04 = new Segment(200,200,300,300);
	Segment w05 = new Segment(250,100,350,200);
	Segment w06 = new Segment(350,200,400,350);
	Segment w07 = new Segment(350,200,450,250);
	Segment w08 = new Segment(450,250,400,350);
	Segment w09 = new Segment(400,350,300,450);
	Segment w10 = new Segment(400,350,400,450);
	Segment w11 = new Segment(400,450,550,600);
	Segment w12 = new Segment(400,350,550,450); 
	Segment w13 = new Segment(550,600,650,500);
	Segment w14 = new Segment(650,500,550,450);
	Segment w15 = new Segment(550,450,700,400);
	Segment w16 = new Segment(700,400,800,600);
	Segment w17 = new Segment(700,400,850,350);
	Segment w18 = new Segment(700,400,750,250);
	Segment w19 = new Segment(700,400,600,250);
	Segment w20 = new Segment(600,250,450,250);
	
	SegList ways = new SegList();
	//ways.add(w01); ways.add(w02); ways.add(w03); ways.add(w04);
	ways.add(w05);
	ways.add(w06); ways.add(w07); ways.add(w08); ways.add(w09); ways.add(w10);
	ways.add(w11); ways.add(w12); ways.add(w13); ways.add(w14); ways.add(w15);
	ways.add(w16); ways.add(w17); ways.add(w18); ways.add(w19); ways.add(w20);

	SegList ways2 = new SegList();
	ways2.add(w01); ways2.add(w02); ways2.add(w03); ways2.add(w04);

	SegList allways = new SegList();
	allways.addAll(ways); allways.addAll(ways2);

	//airports
	Point a01 = new Point(250,100); Point a02 = new Point(200,400);
	Point a03 = new Point(350,200); Point a04 = new Point(550,450);
	Point a05 = new Point(650,500); Point a06 = new Point(850,350);
	Point a07 = new Point(750,250);

	PointList airp = new PointList();
	airp.add(a01); airp.add(a02); airp.add(a03); airp.add(a04); airp.add(a05);
	airp.add(a06); airp.add(a07);
	*/

	/*
	GFXout gfx = new GFXout();
	gfx.initWindow();
	gfx.add(country);
	gfx.addList(allways);
	gfx.addList(airp);
	gfx.showIt();
	*/

	/*
	System.out.println("isCoherent(ways): "+AdditionalOps.isCoherent(ways));
	System.out.println("isCoherent(ways2): "+AdditionalOps.isCoherent(ways2));
	System.out.println("isCoherent(allways): "+AdditionalOps.isCoherent(allways));
	System.out.println("findAccessiblePoints: "); AdditionalOps.findAccessiblePoints(airp,ways).print();
	System.out.println("findAccessiblePoints: "); AdditionalOps.findAccessiblePoints(airp,ways2).print();
	//System.out.println("findAccessiblePoints: "); AdditionalOps.findAccessiblePoints(airp,allways).print();
	System.out.println("flightsOutOfBorder: "); AdditionalOps.flightsOutOfBorder(country.triangles(),airp).print();
	*/
	
	/*
	Segment s01 = new Segment(100,100,200,250);
	Segment s02 = new Segment(200,250,400,300);
	Segment s03 = new Segment(400,300,550,450);
	Segment s04 = new Segment(550,450,650,300);
	Segment s05 = new Segment(650,300,500,150);
	Segment s06 = new Segment(500,150,650,100);

	SegList sl01 = new SegList();
	sl01.add(s01); sl01.add(s02); sl01.add(s03);
	sl01.add(s04); sl01.add(s05); sl01.add(s06);

	Segment s07 = new Segment(250,150,250,350);
	Segment s08 = new Segment(250,350,550,500);
	Segment s09 = new Segment(550,500,700,450);
	Segment s10 = new Segment(700,450,700,150);
	Segment s11 = new Segment(700,150,450,200);
	Segment s12 = new Segment(450,200,250,150);

	SegList sl02 = new SegList();
	sl02.add(s07); sl02.add(s08); sl02.add(s09);
	sl02.add(s10); sl02.add(s11); sl02.add(s12);

	PairList npl = new PairList();
	npl = SetOps.overlappingPairs(sl01,sl02);

	System.out.println("# of overlapping pairs: "+npl.size());
	*/





	/*
	Segment s01 = new Segment(100,500,500,400);
	Segment s02 = new Segment(500,400,400,0);
	Segment s03 = new Segment(400,0,200,100);
	Segment s04 = new Segment(200,100,100,500);
	
	Segment h01 = new Segment(200,400,400,300);
	Segment h02 = new Segment(400,300,300,200);
	Segment h03 = new Segment(300,200,200,200);
	Segment h04 = new Segment(200,200,200,400);
	
	Segment h11 = new Segment(300,100,400,200);
	Segment h12 = new Segment(400,200,400,0);
	Segment h13 = new Segment(400,0,300,100);

	SegList sl01 = new SegList();
	//sl01.add(s01); sl01.add(s02); sl01.add(s03);
	//sl01.add(s04); sl01.add(h01); sl01.add(h02);
	//sl01.add(h03); sl01.add(h04);

	SegList sl02 = new SegList();
	sl02.add(s01); sl02.add(s02); sl02.add(s03);
	sl02.add(s04);

	SegList sl03 = new SegList();
	sl03.add(h11); sl03.add(h12); sl03.add(h13);

	sl01.add(h11); sl01.add(h04); sl01.add(s01); sl01.add(h02);
	sl01.add(s03); sl01.add(h12); sl01.add(h03); sl01.add(s04);
	sl01.add(h01); sl01.add(s02); sl01.add(h13);

	System.out.println("isCoherent(sl01): "+AdditionalOps.isCoherent(sl01));
	System.out.println("isCoherent(sl02): "+AdditionalOps.isCoherent(sl02));
	System.out.println("isCoherent(sl03): "+AdditionalOps.isCoherent(sl03));

       


	//Polygon pol01 = new Polygon(sl01);

	Polygon pol02 = new Polygon(sl02);
	Polygon pol03 = new Polygon(sl03);
	
	System.out.println("isCoherent(tl02): "+AdditionalOps.isCoherent(pol02.triangles()));
	System.out.println("isCoherent(tl03): "+AdditionalOps.isCoherent(pol03.triangles()));
	TriList tl10 = pol02.triangles();
	tl10.addAll(pol03.triangles());
	System.out.println("isCoherent(tl02+tl03): "+AdditionalOps.isCoherent(tl10));
	*/


	//GFXout gfx = new GFXout();
	//gfx.initWindow();
	//gfx.addList(sl01);
	//gfx.add(pol01);
	//gfx.addList(sl02);
	//gfx.addList(sl03);
	//gfx.addList((ElemList)ell01.get(0));
	//gfx.showIt();

    }//end method main
}//end class Starter
