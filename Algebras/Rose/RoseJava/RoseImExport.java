/* 

February 26th, 2002 Mirco Guenster and Ismail Zerrad

This classs contains methods to import and export the data from 
objects of the Rose Algebra to nested lists and byte arrays.

The result of an exprt_nl is a nested list. For this task the static method 
exprt_nl is offered which takes currently a Rational, Point, Points, Segment,
Lines and Regions object and transform it into a Nested List. The type of
the given object is automatically detected.
And there is also a method called exprt_arr which transforms a Rational,
Point, Points, Segment, Lines and Regions object to an array of bytes.
This byte array just constains the result of a serialization and can be 
retransformed by imprt_arr.

On the other hand it is not possible to provide just one method for 
transforming nested lists back to objects of the Rose Algebra because of
the ambiguousness of representation of empty regions and lines objects.
Therefore for each class of the Rose Algebra a static import method is
provided which is called imprt_Rational, imprt_Point, imprt_Points, 
imprt_Segment, imprt_Lines and imprt_Regions.

See also "External Representation of Spatial and Spatio-Temporal Values" by
Jose Antonio Cotelo Lema, 16th June 2000, for details.

*/

import sj.lang.ListExpr;
import java.io.*;
import java.util.*;

public class RoseImExport {
    /* just for testing. */
    private static final int MAX = 5;

    /* 
    This method transforms an object (currently of type 
    Rational, Point, Points, Segment, Lines and Regions)
    to a nested list
    */
    public static void debugM(String message) {
	System.out.println("\t\t" + message);
    }

    public static ListExpr exprt_nl(Object o) {
	debugM("Calculating ListExpr from java object...");
	ListExpr result = exprt_nl_(o);
	debugM("Calculating finished.");
	return result;
    }
    
    public static ListExpr exprt_nl_(Object o) {
	if (o instanceof Rational) {
	    int Rat_n = (int)((Rational)o).getNumerator();
	    int Rat_d = (int)((Rational)o).getDenominator();

	    return
		ListExpr.cons
		(ListExpr.symbolAtom("rat"),
		 ListExpr.cons
		 (ListExpr.symbolAtom( (Math.abs(Rat_n) != Rat_n) ? "-" : "+"),
		  ListExpr.cons
		  (ListExpr.intAtom(Math.abs(Rat_n) / Rat_d),
		   ListExpr.cons
		   (ListExpr.intAtom(Math.abs(Rat_n) % Rat_d),
		    ListExpr.cons
		    (ListExpr.symbolAtom("/"),
		     ListExpr.cons
		     (ListExpr.intAtom(Rat_d),
		      ListExpr.theEmptyList()))))));
	}
	else if (o instanceof Point) {
	    Rational Point_x = ((Point)o).x;
	    Rational Point_y = ((Point)o).y;
	    
	    return 
		ListExpr.cons
		(exprt_nl_(Point_x),
		 ListExpr.cons
		 (exprt_nl_(Point_y),
		  ListExpr.theEmptyList()));
	}
	else if (o instanceof Points) {
	    PointList Points_pl = ((Points)o).pointlist;
	    Object[] Points_array = Points_pl.toArray();
	    ListExpr Points_result = ListExpr.theEmptyList();

	    for (int i = Points_array.length - 1; i >= 0; i--) {
		Points_result = ListExpr.cons
		    (exprt_nl_(Points_array[i]), Points_result);
	    }

	    return Points_result;
	}
	else if (o instanceof Segment) {
	    return
		(ListExpr.cons
		 (exprt_nl_(((Segment)o).getStartpoint().x),
		  ListExpr.cons
		  (exprt_nl_(((Segment)o).getStartpoint().y),
		   ListExpr.cons
		   (exprt_nl_(((Segment)o).getEndpoint().x),
		    ListExpr.cons
		    (exprt_nl_(((Segment)o).getEndpoint().y),
		     ListExpr.theEmptyList())))));
	}
	else if (o instanceof Lines) {
	    SegList Lines_sl = ((Lines)o).seglist;
	    Object[] Lines_array = Lines_sl.toArray();
	    ListExpr Lines_result = ListExpr.theEmptyList();
	    
	    for (int i = Lines_array.length - 1; i >= 0; i--) {
		Lines_result = ListExpr.cons
		    (exprt_nl_(Lines_array[i]), Lines_result);
	    }
	    
	    return Lines_result;
	}
	else if (o instanceof Regions){
	    debugM("cyclesPoints() started.");
	    ElemListListList elll = ((Regions)o).cyclesPoints();
	    debugM("cyclesPoints() finished.");

	    return exprt_nl_(elll);
	}
	else if (o instanceof ElemListListList) {
	    ElemListListList elll = (ElemListListList)o;
	    int elllsize = elll.size();
	    Object[] ell = elll.toArray();
	    ListExpr ElemListListList_result = ListExpr.theEmptyList();

	    for (int i = ell.length - 1; i >= 0; i--) {
		ElemListListList_result = ListExpr.cons
		    (exprt_nl_(ell[i]), ElemListListList_result);
	    }
	    return ElemListListList_result;
	}
	else if (o instanceof ElemListList) {
	    ElemListList ell = (ElemListList)o;
	    int ellsize = ell.size();
	    Object[] el = ell.toArray();
	    ListExpr ElemListList_result = ListExpr.theEmptyList();

	    for (int i = el.length - 1; i >= 0; i--) {
		ElemListList_result = ListExpr.cons
		    (exprt_nl_(el[i]), ElemListList_result);
	    }
	    return ElemListList_result;
	}
	else if (o instanceof ElemList) {
	    ElemList el = (ElemList)o;
	    int elsize = el.size();
	    Object[] e = el.toArray();
	    ListExpr ElemList_result = ListExpr.theEmptyList();
	    
	    for (int i = e.length - 1; i >= 0; i--) {
		ElemList_result = ListExpr.cons
		    (exprt_nl_(e[i]), ElemList_result);
	    }
	    return ElemList_result;
	}
	else {
	    System.out.println("NULL RETURNED.");
	    return null;
	}
    }

    /* Just for debugging */
    private static void printByteArray(byte[] ba) {
	System.out.print("{");
	for (int i = 0; i < ba.length; i++) {
	    System.out.print("" + ba[i]);
	    if (i < ba.length - 1) System.out.print(", ");
	}
	System.out.println("}");
    }

    /* 

    This method transforms an serializable object to a byte array.
    */
    public static byte[] exprt_arr(Object o) {
	try {
	    ByteArrayOutputStream baos = new ByteArrayOutputStream();
	    ObjectOutputStream oos = new ObjectOutputStream(baos);
	    oos.writeObject(o);
	    return baos.toByteArray();
	}
	catch (Exception e) {
	    System.err.println("Error occurred during serialization: " + e);
	    return null;
	}
    }

    /*

    This method transform a string which was created by exprt_arr back to a
    java object. 
    */
    public static Object imprt_arr(byte[] objectdata) {
	try {
	    ByteArrayInputStream bais = new ByteArrayInputStream(objectdata);
	    ObjectInputStream ois = new ObjectInputStream(bais);
	    return ois.readObject();
	}
	catch (Exception e) {
	    System.err.println("Error occured during deserialization:" + e);
	    return null;
	}
    }

    /* Calculates the length of an array. Help method for JNI. */
    public static int calcLength(byte[] barr) {
	return barr.length;
    }

    /* recover an instance of Rational */
    /* changed by DA: changed constructor for Rationals from
       'Rational' to 'RationalFactory' */
    private static Rational imprt_Rational(ListExpr nl) {
	RationalFactory.setClass(RationalBigInteger.class);
	int Rat_sign
	    = nl.second().symbolValue().equals("-") ? -1 : 1;
	int Rat_intPart = nl.third().intValue() * Rat_sign;
	int Rat_numDec 
	    = nl.fourth().intValue() * Rat_sign;
	int Rat_dnmDec 
	    = nl.sixth().intValue();
	Rational result = null;
	try {
	    result = RationalFactory.constRational(Rat_intPart * Rat_dnmDec + Rat_numDec, Rat_dnmDec);
	}
	catch (Exception ex) {
	    ex.printStackTrace();
	}
	return result;
    }

    /* recover an instance of Point */
    /* changed by DA: now accepts also instances of Point with Point(int,int)
       and Point(double,double). */
    private static Point imprt_Point(ListExpr nl) {
	ListExpr first = nl.first();
	ListExpr second = nl.second();
	if (first.isAtom()) {
	    if ((first.atomType() == ListExpr.INT_ATOM) &&
		(second.atomType() == ListExpr.INT_ATOM)) {
		return new Point(first.intValue(),second.intValue());
	    }//if
	    else if ((first.atomType() == ListExpr.REAL_ATOM) &&
		     (second.atomType() == ListExpr.REAL_ATOM)) {
		return new Point(first.realValue(),second.realValue());
	    }//else if
	    else {
		System.err.println("Check Point coordinates (imprt_Point): coordinates are not valid.");
		return new Point();
	    }//else
	}//if
	else {
	    return new Point(imprt_Rational(nl.first()), imprt_Rational(nl.second()) );
	}//else
    }

    /* recover an instance of Points */
    public static Points imprt_Points(ListExpr nl) {
	debugM("Importing Points...");
	Points result = imprt_Points_(nl);
	debugM("Importing finished.");
	return result;
    }

    public static Points imprt_Points_(ListExpr nl) {
	int nll = nl.listLength();
	if (nll > 0) {
	    ListExpr restList = nl;
	    PointList pl = new PointList();
	    for (int i = 0; i < nll; i++) {
		pl.add(imprt_Point(restList.first()));
		restList = restList.rest();
	    }
	    return new Points(pl);
	}
	else return new Points();
    }

    /* recover an instance of Segment */
    /* changed by DA: now accepts also instances of Segments with Segment(int,int,int,int) and
       Segment(double,double,double,double). */
    private static Segment imprt_Segment(ListExpr nl) {
	ListExpr first = nl.first();
	ListExpr second = nl.second();
	ListExpr third = nl.third();
	ListExpr fourth = nl.fourth();
	if (first.isAtom()) {
	    if ((first.atomType() == ListExpr.INT_ATOM) &&
		(second.atomType() == ListExpr.INT_ATOM) &&
		(third.atomType() == ListExpr.INT_ATOM) &&
		(fourth.atomType() == ListExpr.INT_ATOM)) {
		return new Segment(first.intValue(),
				   second.intValue(),
				   third.intValue(),
				   fourth.intValue());
	    }//if
	    else if ((first.atomType() == ListExpr.REAL_ATOM) &&
		     (second.atomType() == ListExpr.REAL_ATOM) &&
		     (third.atomType() == ListExpr.REAL_ATOM) &&
		     (fourth.atomType() == ListExpr.REAL_ATOM)) {
		return new Segment(first.realValue(),
				   second.realValue(),
				   third.realValue(),
				   fourth.realValue());
	    }//else if
	    else {
		System.err.println("Check Segment coordinates (impr_Segment): coordinastes are not valid.");
		return new Segment();
	    }//else
	}//if
	else {
	    return new Segment
		(imprt_Rational(nl.first()),
		 imprt_Rational(nl.second()),
		 imprt_Rational(nl.third()),
		 imprt_Rational(nl.fourth()));
	}//else
    }

    /* recover an instance of Lines */
    public static Lines imprt_Lines(ListExpr nl) {
	debugM("Importing Lines...");
	Lines result = imprt_Lines_(nl);
	debugM("Importing finished.");
	return result;
    }

    public static Lines imprt_Lines_(ListExpr nl) {
	int nll = nl.listLength();
	if (nll > 0) {
	    ListExpr restList = nl;
	    SegList sl = new SegList();
	    for (int i = 0; i < nll; i++) {
		sl.add(imprt_Segment(restList.first()));
		restList = restList.rest();
	    }
	    return new Lines(sl);
	}	
	else return new Lines();
    }

    /* inserts in a SegList all segments of a cycle..
       This cycle is part of a face. */
    private static void imprt_Cycle(SegList sl, ListExpr nl) {
	int nll = nl.listLength();
	ListExpr restList = nl;
	Point[] parr = new Point[nll];
	for (int i = 0; i < nll; i++) {
	    parr[i] = imprt_Point(restList.first());
	    restList = restList.rest();
	}
	for (int i = 0; i < nll - 1; i++) {
	    sl.add(new Segment(parr[i], parr[i+1]));
	}
	sl.add(new Segment(parr[nll - 1], parr[0]));
    }

    /* inserts in a SegList all cycles of a face */
    private static void imprt_Face(SegList sl, ListExpr nl) {
	int nll = nl.listLength();
	ListExpr restList = nl;
	for (int i = 0; i < nll; i++) {
	    imprt_Cycle(sl, restList.first());
	    restList = restList.rest();
	}
    }
    
    /* recover an instance of Regions */
    public static Regions imprt_Regions(ListExpr nl) {
	debugM("Importing Regions...");
	Regions result = imprt_Regions_(nl);
	debugM("Importing finished.");
	return result;
    }

    public static Regions imprt_Regions_(ListExpr nl) {
	// we have to collet all segments to recreate a
	// Regions object.
	SegList sl = new SegList();
	int nll = nl.listLength();
	ListExpr restList = nl;
	for (int i = 0; i < nll; i++) {
	    imprt_Face(sl, restList.first());
	    restList = restList.rest();
	}
	return new Regions(sl);
    }
 
    /*
      The following methods test the import and export mechanism of
      different rose objects.
    */

    /* test import and export of a rational object. */
    private static void rationalTest() {
	Rational rat1 = RationalFactory.constRational(-5, 3);
	StringBuffer result = new StringBuffer();
	ListExpr le = exprt_nl(rat1);
	le.writeToString(result);
	System.out.println(result.toString());

	Rational rat2 = imprt_Rational(le);
	System.out.println(rat2);

	byte[] strexprt = exprt_arr(rat2);
	System.out.print("Export byte array:");
	printByteArray(strexprt);
	
	Rational rat3 = (Rational)imprt_arr(strexprt);
	System.out.println("Reimported Rational: " +  rat3);
    }

    /* test import and export mechanism for a point object. */
    private static void pointTest() {
	Rational rat1 = RationalFactory.constRational(-5, 3);
	Rational rat2 = RationalFactory.constRational(11, 5);
	Point p1 = new Point(rat1, rat2);
	StringBuffer sb = new StringBuffer();
	ListExpr le = exprt_nl(p1);
	le.writeToString(sb);
	System.out.println(sb.toString());
	
	Point p2 = imprt_Point(le);
	p2.print();

	byte[] strexprt = exprt_arr(p2);
	System.out.println("Export byte array:");
	printByteArray(strexprt);

	Point p3 = (Point)imprt_arr(strexprt);
	System.out.print("Reimported Point: ");
	p3.print();
    }

    /* test import and export mechanism for a points object. */
    private static void pointsTest() {
	Rational[] rat1 = new Rational[MAX];
	Rational[] rat2 = new Rational[MAX];
	Point[] p = new Point[MAX];
	PointList pl = new PointList();

	for (int i = 0; i < MAX; i++) {
	    rat1[i] = RationalFactory.constRational(i+1, i+2);
	    rat2[i] = RationalFactory.constRational(i+3, i+4);
	    p[i] = new Point(rat1[i], rat2[i]);
	    pl.add(p[i]);   
	}

	Points ps = new Points(pl);
	StringBuffer sb = new StringBuffer();
	ListExpr le = exprt_nl(ps);
	le.writeToString(sb);
	System.out.println(sb.toString());

	Points ps2 = imprt_Points(le);
	ps2.pointlist.print();

	byte[] strexprt = exprt_arr(ps2);
	System.out.println("Export byte array:");
	printByteArray(strexprt);

	Points ps3 = (Points)imprt_arr(strexprt);
	System.out.print("Reimported Points: ");
	ps3.pointlist.print();
    }

    /* test import and export mechanism for a lines object. */
    private static void linesTest() {
	SegList sl = new SegList();

	for (int i = 0; i < MAX; i++) {
	    sl.add
		(new Segment
		 (RationalFactory.constRational(i+1, i+2),
		  RationalFactory.constRational(i+3, i+4), 
		  RationalFactory.constRational(i+5, i+6), 
		  RationalFactory.constRational(i+7, i+8))); 
	}
	
	Lines ls = new Lines(sl);
	
	StringBuffer sb = new StringBuffer();
	ListExpr le = exprt_nl(ls);
	le.writeToString(sb);
	System.out.println(sb.toString());

	Lines result = imprt_Lines(le);
	result.seglist.print();

	byte[] strexprt = exprt_arr(result);
	System.out.println("Export byte array:");
	printByteArray(strexprt);

	Lines l3 = (Lines)imprt_arr(strexprt);
	System.out.print("Reimported Lines: ");
	l3.seglist.print();
    }

    /* test import and export mechanism for a regions object. */
    private static void regionsTestOld() {
	Rational rx01 = RationalFactory.constRational(1, 1);
	Rational ry01 = RationalFactory.constRational(1, 1);

	Rational rx02 = RationalFactory.constRational(6, 1);
	Rational ry02 = RationalFactory.constRational(1, 1);

	Rational rx03 = RationalFactory.constRational(6, 1);
	Rational ry03 = RationalFactory.constRational(6, 1);

	Rational rx04 = RationalFactory.constRational(1, 1);
	Rational ry04 = RationalFactory.constRational(6, 1);

	Rational rx05 = RationalFactory.constRational(2, 1);
	Rational ry05 = RationalFactory.constRational(2, 1);

	Rational rx06 = RationalFactory.constRational(3, 1);
	Rational ry06 = RationalFactory.constRational(2, 1);

	Rational rx07 = RationalFactory.constRational(3, 1);
	Rational ry07 = RationalFactory.constRational(3, 1);

	Rational rx08 = RationalFactory.constRational(2, 1);
	Rational ry08 = RationalFactory.constRational(3, 1);

	Rational rx09 = RationalFactory.constRational(4, 1);
	Rational ry09 = RationalFactory.constRational(2, 1);

	Rational rx10 = RationalFactory.constRational(5, 1);
	Rational ry10 = RationalFactory.constRational(2, 1);

	Rational rx11 = RationalFactory.constRational(5, 1);
	Rational ry11 = RationalFactory.constRational(3, 1);

	Rational rx12 = RationalFactory.constRational(4, 1);
	Rational ry12 = RationalFactory.constRational(3, 1);

	Rational rx13 = RationalFactory.constRational(4, 1);
	Rational ry13 = RationalFactory.constRational(4, 1);

	Rational rx14 = RationalFactory.constRational(5, 1);
	Rational ry14 = RationalFactory.constRational(4, 1);

	Rational rx15 = RationalFactory.constRational(5, 1);
	Rational ry15 = RationalFactory.constRational(5, 1);

	Rational rx16 = RationalFactory.constRational(4, 1);
	Rational ry16 = RationalFactory.constRational(5, 1);

	Rational rx17 = RationalFactory.constRational(2, 1);
	Rational ry17 = RationalFactory.constRational(4, 1);

	Rational rx18 = RationalFactory.constRational(3, 1);
	Rational ry18 = RationalFactory.constRational(4, 1);

	Rational rx19 = RationalFactory.constRational(3, 1);
	Rational ry19 = RationalFactory.constRational(5, 1);

	Rational rx20 = RationalFactory.constRational(2, 1);
	Rational ry20 = RationalFactory.constRational(5, 1);

 
	Point p01 = new Point(rx01, ry01);
	Point p02 = new Point(rx02, ry02);
	Point p03 = new Point(rx03, ry03);
	Point p04 = new Point(rx04, ry04);
	Point p05 = new Point(rx05, ry05);
	Point p06 = new Point(rx06, ry06);
	Point p07 = new Point(rx07, ry07);
	Point p08 = new Point(rx08, ry08);
	Point p09 = new Point(rx09, ry09);
	Point p10 = new Point(rx10, ry10);
	Point p11 = new Point(rx11, ry11);
	Point p12 = new Point(rx12, ry12);
	Point p13 = new Point(rx13, ry13);
	Point p14 = new Point(rx14, ry14);
	Point p15 = new Point(rx15, ry15);
	Point p16 = new Point(rx16, ry16);
	Point p17 = new Point(rx17, ry17);
	Point p18 = new Point(rx18, ry18);
	Point p19 = new Point(rx19, ry19);
	Point p20 = new Point(rx20, ry20);

	// outer cycle
	Segment s01 = new Segment(p01, p02);
	Segment s02 = new Segment(p02, p03);
	Segment s03 = new Segment(p03, p04);
	Segment s04 = new Segment(p04, p01);

	// hole cycle No. 1
	Segment s05 = new Segment(p05, p06);
	Segment s06 = new Segment(p06, p07);
	Segment s07 = new Segment(p07, p08);
	Segment s08 = new Segment(p08, p05);
       
	// hole cycle No. 2
	Segment s09 = new Segment(p09, p10);
	Segment s10 = new Segment(p10, p11);
	Segment s11 = new Segment(p11, p12);
	Segment s12 = new Segment(p12, p09);

	// hole cycle No. 3
	Segment s13 = new Segment(p13, p14);
	Segment s14 = new Segment(p14, p15);
	Segment s15 = new Segment(p15, p16);
	Segment s16 = new Segment(p16, p13);

	// hole cycle No. 4
	Segment s17 = new Segment(p17, p18);
	Segment s18 = new Segment(p18, p19);
	Segment s19 = new Segment(p19, p20);
	Segment s20 = new Segment(p20, p17);

	SegList sl = new SegList();
	sl.add(s01);
	sl.add(s02);
	sl.add(s03);
	sl.add(s04);
	sl.add(s05);
	sl.add(s06);
	sl.add(s07);
	sl.add(s08);
	sl.add(s09);
	sl.add(s10);
	sl.add(s11);
	sl.add(s12);
	sl.add(s13);
	sl.add(s14);
	sl.add(s15);
	sl.add(s16);
	sl.add(s17);
	sl.add(s18);
	sl.add(s19);
	sl.add(s20);

	System.out.println("check1");
	System.out.println("sl:"); sl.print();

	Regions r = new Regions(sl);
	r.trilist.print();
	System.out.println("-----");

	StringBuffer sb = new StringBuffer();
	ListExpr le = exprt_nl(r);
	le.writeToString(sb);
	System.out.println(sb.toString());

	Regions r2 = imprt_Regions(le);
	r2.trilist.print();

	byte[] strexprt = exprt_arr(r2);
	System.out.println("Export byte array:");
	printByteArray(strexprt);

	Regions r3 = (Regions)imprt_arr(strexprt);
	System.out.print("Reimported Regions: ");
	r3.trilist.print();
    }

    /* test import and export mechanism for a regions object. */
    private static void regionsTest() {
	// build segments of outer cycle.
	Rational x1 = RationalFactory.constRational(1, 1);
	Rational y1 = RationalFactory.constRational(1, 1);

	Rational x2 = RationalFactory.constRational(2 * MAX + 1, 1);
	Rational y2 = RationalFactory.constRational(1, 1);

	Rational x3 = RationalFactory.constRational(2 * MAX + 1, 1);
	Rational y3 = RationalFactory.constRational(4, 1);

	Rational x4 = RationalFactory.constRational(1, 1);
	Rational y4 = RationalFactory.constRational(4, 1);

	Point p1 = new Point(x1, y1);
	Point p2 = new Point(x2, y2);
	Point p3 = new Point(x3, y3);
	Point p4 = new Point(x4, y4);

	Segment s1 = new Segment(p1, p2);
	Segment s2 = new Segment(p2, p3);
	Segment s3 = new Segment(p3, p4);
	Segment s4 = new Segment(p4, p1);

	SegList sl = new SegList();
	sl.add(s1);
	sl.add(s2);
	sl.add(s3);
	sl.add(s4);

	// build segments of hole cycles.
	for (int i = 0; i < MAX; i++) {
	    x1 = RationalFactory.constRational(2 + i * 2, 1);
	    y1 = RationalFactory.constRational(2, 1);
	
	    x2 = RationalFactory.constRational(3 + i * 2, 1);
	    y2 = RationalFactory.constRational(2, 1);
	    
	    x3 = RationalFactory.constRational(3 + i * 2, 1);
	    y3 = RationalFactory.constRational(3, 1);

	    x4 = RationalFactory.constRational(2 + i * 2, 1);
	    y4 = RationalFactory.constRational(3, 1);

	    p1 = new Point(x1, y1);
	    p2 = new Point(x2, y2);
	    p3 = new Point(x3, y3);
	    p4 = new Point(x4, y4);

	    s1 = new Segment(p1, p2);
	    s2 = new Segment(p2, p3);
	    s3 = new Segment(p3, p4);
	    s4 = new Segment(p4, p1);
	    
	    sl.add(s1);
	    sl.add(s2);
	    sl.add(s3);
	    sl.add(s4);
	}

	System.out.println("check1");
	System.out.println("sl:"); sl.print();

	Regions r = new Regions(sl);
	r.trilist.print();
	System.out.println("-----");

	StringBuffer sb = new StringBuffer();
	ListExpr le = exprt_nl(r);
	le.writeToString(sb);
	System.out.println(sb.toString());

	Regions r2 = imprt_Regions(le);
	r2.trilist.print();

	byte[] strexprt = exprt_arr(r2);
	System.out.println("Export byte array:");
	printByteArray(strexprt);

	Regions r3 = (Regions)imprt_arr(strexprt);
	System.out.print("Reimported Regions: ");
	r3.trilist.print();
    }

    /* help method for Relations test. */
    public static String createPoints(int n) {
	Rational[] rat1 = new Rational[MAX];
	Rational[] rat2 = new Rational[MAX];
	Point[] p = new Point[MAX];
	PointList pl = new PointList();

	for (int i = 0; i < MAX; i++) {
	    rat1[i] = RationalFactory.constRational(i+1+n, i+2+n);
	    rat2[i] = RationalFactory.constRational(i+3+n, i+4+n);
	    p[i] = new Point(rat1[i], rat2[i]);
	    pl.add(p[i]);   
	}

	Points ps = new Points(pl);
	StringBuffer sb = new StringBuffer();
	ListExpr le = exprt_nl(ps);
	le.writeToString(sb);
	return sb.toString();
    }

    /* help method for Relations test. */
    public static String createLines(int n) {
	SegList sl = new SegList();

	for (int i = 0; i < MAX; i++) {
	    sl.add
		(new Segment
		 (RationalFactory.constRational(i+1+n, i+2+n),
		  RationalFactory.constRational(i+3+n, i+4+n), 
		  RationalFactory.constRational(i+5+n, i+6+n), 
		  RationalFactory.constRational(i+7+n, i+8+n))); 
	}
	
	Lines ls = new Lines(sl);
	
	StringBuffer sb = new StringBuffer();
	ListExpr le = exprt_nl(ls);
	le.writeToString(sb);
	return sb.toString();
    }

 /* help method for Relations test. */
    public static String createRegions(int n) {
	// build segments of outer cycle.
	Rational x1 = RationalFactory.constRational(1 + n, 1 + n);
	Rational y1 = RationalFactory.constRational(1 + n, 1 + n);

	Rational x2 = RationalFactory.constRational(2 * MAX + 1 + n, 1 + n);
	Rational y2 = RationalFactory.constRational(1 + n, 1);

	Rational x3 = RationalFactory.constRational(2 * MAX + 1 + n, 1 + n);
	Rational y3 = RationalFactory.constRational(4 + n, 1 + n);

	Rational x4 = RationalFactory.constRational(1 + n, 1 + n);
	Rational y4 = RationalFactory.constRational(4 + n, 1 + n);

	Point p1 = new Point(x1, y1);
	Point p2 = new Point(x2, y2);
	Point p3 = new Point(x3, y3);
	Point p4 = new Point(x4, y4);

	Segment s1 = new Segment(p1, p2);
	Segment s2 = new Segment(p2, p3);
	Segment s3 = new Segment(p3, p4);
	Segment s4 = new Segment(p4, p1);

	SegList sl = new SegList();
	sl.add(s1);
	sl.add(s2);
	sl.add(s3);
	sl.add(s4);

	// build segments of hole cycles.
	for (int i = 0; i < MAX; i++) {
	    x1 = RationalFactory.constRational(2 + i * 2+n, 1+n);
	    y1 = RationalFactory.constRational(2+n, 1+n);
	
	    x2 = RationalFactory.constRational(3 + i * 2+n, 1+n);
	    y2 = RationalFactory.constRational(2+n, 1+n);
	    
	    x3 = RationalFactory.constRational(3+n + i * 2, 1+n);
	    y3 = RationalFactory.constRational(3+n, 1+n);

	    x4 = RationalFactory.constRational(2+n + i * 2, 1+n);
	    y4 = RationalFactory.constRational(3+n, 1+n);

	    p1 = new Point(x1, y1);
	    p2 = new Point(x2, y2);
	    p3 = new Point(x3, y3);
	    p4 = new Point(x4, y4);

	    s1 = new Segment(p1, p2);
	    s2 = new Segment(p2, p3);
	    s3 = new Segment(p3, p4);
	    s4 = new Segment(p4, p1);
	    
	    sl.add(s1);
	    sl.add(s2);
	    sl.add(s3);
	    sl.add(s4);
	}

	Regions r = new Regions(sl);

	StringBuffer sb = new StringBuffer();
	ListExpr le = exprt_nl(r);
	le.writeToString(sb);
	return sb.toString();
    }

    /* Relations test */
    public static void relationsTest() {
	String result = "(create RoseRel : (rel(tuple((punkte ccpoints) (linien cclines) (regionen ccregions)))));";
	result = result + "\n(update RoseRel : ((rel(tuple((punkte ccpoints) (linien cclines) (regionen ccregions))))(";
	for (int i = 0; i < 3; i++) {
	    result = result + createPoints(i);
	    result = result + createLines(i);
	    result = result + createRegions(0);
	}
	result = result + ")));";
	System.out.println(result);
    }

    public static void main (String args[]) {
	RationalFactory.setClass(RationalBigInteger.class);
	//System.out.println("---Test Rational---");
	//rationalTest();
	
	//System.out.println("\n\n\n\n\n---Test Point------");
	//pointTest();
	
	//System.out.println("\n\n\n\n\n---Test Points-----");
	//pointsTest();
	
	//System.out.println("\n\n\n\n\n---Test Lines------");
	//linesTest();

	//System.out.println("\n\n\n\n\n---Test Regions----");
	//regionsTest();

	System.out.println("\n\n\n\n\n---Test Relations---");
	relationsTest();
    }
}
