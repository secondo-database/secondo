class Application {
    //This is a demo application which calls
    //some of the operations from algebra.java.
    //
    //***v1.01***
    //
    
    public static void main (String[] args) {
	System.out.println("*** demo application ***");
	System.out.println();


	System.out.println("*** operations on points ***");
	//definition of some points
	Point p01 = new Point(150,400); Point p02 = new Point(200,250);
	Point p03 = new Point(250,400); Point p04 = new Point(300,450);
	Point p05 = new Point(300,300); Point p06 = new Point(350,200);
	Point p07 = new Point(400,350);

	//definition of some sets of points
	PointList pl01 = new PointList();
	PointList pl02 = new PointList();
	//add some points
	pl01.add(p01); pl01.add(p02);
	pl02.add(p06); pl02.add(p07);

	//make copies of both point sets
	PointList pl01copy = (PointList)pl01.copy();
	PointList pl02copy = (PointList)pl02.copy();

	//print out the point sets
	System.out.println("pl01:");
	pl01.print();
	System.out.println("pl02:");
	pl02.print();
	//perform some operations on these sets
	System.out.println("pp_equal:"+Algebra.pp_equal(pl01,pl02));
	System.out.println("pp_unequal:"+Algebra.pp_unequal(pl01,pl02));
	System.out.println("pp_disjoint:"+Algebra.pp_disjoint(pl01,pl02));
	System.out.println("pp_intersection:"); Algebra.pp_intersection(pl01,pl02).print();
	System.out.println("pp_plus():"); Algebra.pp_plus(pl01,pl02).print();
	System.out.println("pp_minus():"); Algebra.pp_minus(pl01,pl02).print();
	System.out.println("pp_dist:"+Algebra.pp_dist(pl01,pl02));
	
	System.out.println("\n\n");

	System.out.println("*** operations on segments ***");

	//definition of some segments
	Segment s01 = new Segment(p01,p02); Segment s02 = new Segment(p03,p04);
	Segment s03 = new Segment(p03,p07); Segment s04 = new Segment(p04,p05);
	Segment s05 = new Segment(p04,p06);
	//definition of some sets of segments
	SegList sl01 = new SegList();
	SegList sl02 = new SegList();
	//add some segments
	sl01.add(s01); sl01.add(s02); sl01.add(s03);
	sl01.add(s05); sl02.add(s03); sl02.add(s04);
	sl02.add(s05);

	//make copies of both segment sets
	SegList sl01copy = (SegList)sl01.copy();
	SegList sl02copy = (SegList)sl02.copy();

	//print out the segment sets
	System.out.println("sl01:");
	sl01.print();
	System.out.println("sl02:");
	sl02.print();
	//perform some operations on these sets
	System.out.println("ll_equal:"+Algebra.ll_equal(sl01,sl02));
	System.out.println("ll_unequal:"+Algebra.ll_unequal(sl01,sl02));
	System.out.println("ll_disjoint:"+Algebra.ll_disjoint(sl01,sl02));
	System.out.println("ll_intersects:"+Algebra.ll_intersects(sl01,sl02));
	System.out.println("ll_border_in_common:"+Algebra.ll_border_in_common(sl01,sl02));
	System.out.println("ll_intersection:"); Algebra.ll_intersection(sl01,sl02).print();
	System.out.println("ll_plus:"); Algebra.ll_plus(sl01,sl02).print();
	System.out.println("ll_minus(sl01,sl02):"); Algebra.ll_minus(sl01,sl02).print();
	System.out.println("ll_common_border():"); Algebra.ll_common_border(sl01,sl02).print();
	System.out.println("l_vertices(sl01):"); Algebra.l_vertices(sl01).print();
	//System.out.println("l_interior(xxx):"); Algebra.l_interior(xxx).print();
	System.out.println("l_no_of_components(sl01):"+Algebra.l_no_of_components(sl01));
	System.out.println("ll_dist():"+Algebra.ll_dist(sl01,sl02));
	System.out.println("l_diameter(sl01):"+Algebra.l_diameter(sl01));
	System.out.println("l_length(sl01):"+Algebra.l_length(sl01));
	System.out.println("l_length(sl02):"+Algebra.l_length(sl02));
	
	System.out.println("\n\n");

	System.out.println("*** operations on triangles ***");
	//definition of some triangles
	Triangle t01 = new Triangle(new Point(50,500), new Point(600,450), new Point(100,150));
	Triangle t02 = new Triangle(new Point(100,150), new Point(600,450), new Point(400,100));
	Triangle t03 = new Triangle(new Point(400,100), new Point(600,450), new Point(650,200));

	Triangle t04 = new Triangle(new Point(250,650), new Point(250,100), new Point(400,650));
	Triangle t05 = new Triangle(new Point(400,650), new Point(250,100), new Point(550,650));

	//definition of some sets of triangles
	TriList tl01 = new TriList();
	TriList tl02 = new TriList();
	//add some triangles
	tl01.add(t01); tl01.add(t02); tl01.add(t03);
	tl02.add(t04); tl02.add(t05);

	//make copies of both triangle sets
	TriList tl01copy = (TriList)tl01.copy();
	TriList tl02copy = (TriList)tl02.copy();

	//print out the triangle sets
	System.out.println("tl01");
	tl01.print();
	System.out.println("tl02");
	tl02.print();
	//perform some operations on these sets
	System.out.println("rr_equal:"+Algebra.rr_equal(tl01,tl02));
	System.out.println("rr_unequal:"+Algebra.rr_unequal(tl01,tl02));
	System.out.println("rr_disjoint:"+Algebra.rr_disjoint(tl01,tl02));
	System.out.println("rr_intersects:"+Algebra.rr_intersects(tl01,tl02));
	System.out.println("rr_border_in_common:"+Algebra.rr_border_in_common(tl01,tl02));
	System.out.println("rr_adjacent:"+Algebra.rr_adjacent(tl01,tl02));
	System.out.println("rr_common_border:"); Algebra.rr_common_border(tl01,tl02).print();
	System.out.println("r_vertices(tl02):"); Algebra.r_vertices(tl02).print();
	System.out.println("r_contour(tl01):"); Algebra.r_contour(tl01).print();
	System.out.println("r_no_of_components(tl01):"+Algebra.r_no_of_components(tl01));
	System.out.println("rr_dist:"+Algebra.rr_dist(tl01,tl02));
	System.out.println("r_diameter(tl02):"+Algebra.r_diameter(tl02));
	System.out.println("r_area(tl01):"+Algebra.r_area(tl01));
	System.out.println("r_perimeter(tl01):"+Algebra.r_perimeter(tl01));

	System.out.println("\n\n");


	System.out.println("*** further operations ***");
	//perform some further operations on the sets defined above
	System.out.println("pr_inside(pl01,tl01):"+Algebra.pr_inside(pl01,tl01));
	System.out.println("lr_inside(sl01,tl02):"+Algebra.lr_inside(sl01,tl02));
	System.out.println("lr_intersects(sl01,tl01):"+Algebra.lr_intersects(sl01,tl01));
	System.out.println("lr_border_in_common(sl01,tl01):"+Algebra.lr_border_in_common(sl01,tl01));
	System.out.println("rl_border_in_common(tl01,sl01):"+Algebra.rl_border_in_common(tl01,sl01));
	System.out.println("pl_on_border_of(pl01,sl01):"+Algebra.pl_on_border_of(pl01,sl01));
	System.out.println("pr_on_border_of(pl01,tl01):"+Algebra.pr_on_border_of(pl01,tl01));
	System.out.println("rl_intersection(tl01,sl01):"); (Algebra.rl_intersection(tl01,sl01)).print();
	System.out.println("lr_common_border(sl01,tl01):"); (Algebra.lr_common_border(sl01,tl01)).print();
	System.out.println("pl_dist(pl01,sl02):"+Algebra.pl_dist(pl01,sl02));
	System.out.println("pr_dist(pl01,tl01):"+Algebra.pr_dist(pl01,tl01));
	System.out.println("lr_dist(sl01,tl01):"+Algebra.lr_dist(sl01,tl01));
	
	System.out.println();
	//finally put some graphics on the screen
	GFXout gfx = new GFXout();
	gfx.initWindow();
	gfx.addList(sl01copy);
	gfx.addList(sl02copy);
	gfx.addList(pl01copy);
	gfx.addList(pl02copy);
	gfx.addList(tl01copy);
	gfx.addList(tl02copy);
	gfx.showIt();

	System.out.println("*** demo finished ***");

    }//end method main



}//end class Application
