class SegSeg_Ops {

  //variables

  //constructors

  //methods
    public static boolean pointsInCommon(Segment s1, Segment s2) {
	//true if both segments have at least one common point
	
	if (s1.equal(s2)) { return true; }
	if (s1.pintersects(s2)) { return true; }
	if (formALine(s1,s2)) { return true; }
	if (formASegment(s1,s2)) { return true; }
	if (adjacent(s1,s2)) { return true; }
	if (overlap(s1,s2)) { return true; }
	if (PointSeg_Ops.liesOn(s1.startpoint,s2)) { return true; }
	if (PointSeg_Ops.liesOn(s1.endpoint,s2)) { return true; }
	if (PointSeg_Ops.liesOn(s2.startpoint,s1)) { return true; }
	if (PointSeg_Ops.liesOn(s2.endpoint,s1)) { return true; }
	    
	return false;
    }//end method pointsInCommon


    public static boolean pmeet (Segment s1, Segment s2) {
	//returns true if an endpoint of one segment lies on the other one
	//but is NOT an endpoint of the other one

	if ((PointSeg_Ops.liesOn(s1.startpoint,s2) ||
	     PointSeg_Ops.liesOn(s1.endpoint,s2) ||
	     PointSeg_Ops.liesOn(s2.startpoint,s1) ||
	     PointSeg_Ops.liesOn(s2.endpoint,s1)) &&
	    !(formALine(s1,s2)) || overlap(s1,s2)) {
	    //System.out.println("***pmeet holds");
	    return true; }
	else { return false; }
    }//end method pmeet


    public static boolean formALine(Segment s1, Segment s2) {
	//true if one(!) of the vertices of both segments is the same, false else
	boolean endpointEQ = false;
	if (s1.equal(s2)) {
	    //System.out.println("SSO.fAL: equal.");
	    return false; }
	
	if (PointSeg_Ops.isEndpoint(s1.startpoint,s2) ||
	    PointSeg_Ops.isEndpoint(s1.endpoint,s2) ||
	    PointSeg_Ops.isEndpoint(s2.startpoint,s1) ||
	    PointSeg_Ops.isEndpoint(s2.endpoint,s1)) {
	    endpointEQ = true;
	    //System.out.println("SSO.fAL: endpointEQ: "+endpointEQ);
	}//if
	
	if (endpointEQ) {
	    //System.out.println("SSO.fAL: Step 2");
	    boolean s1sONs2 = PointSeg_Ops.liesOn(s1.startpoint,s2);
	    boolean s1eONs2 = PointSeg_Ops.liesOn(s1.endpoint,s2);
	    //System.out.println("liesOn: "+s1sONs2+", "+s1eONs2);
	    if (s1sONs2 && s1eONs2) {
		//System.out.println("SSO.fAL: false (case1)");
		return false; }
	    boolean s2sONs1 = PointSeg_Ops.liesOn(s2.startpoint,s1);
	    boolean s2eONs1 = PointSeg_Ops.liesOn(s2.endpoint,s1);
	    //System.out.println("liesOn2: "+s2sONs1+", "+s2eONs1);
	    if (s2sONs1 && s2eONs1) { 
		//System.out.println("SSO.fAL: false (case2)");
		return false; }
	    //System.out.println("SSO.fAL: true");
	    return true;
	}//if
	//System.out.println("SSO.fAL: false (case3)");
	return false;
    }//end formALine
    
  protected static boolean formASegment(Segment s1, Segment s2) {
    //true if one(!) of the vertices of both segments is the same and 
    //if together they form a straight line, false else
    if (formALine(s1,s2) && Mathset.linearly_dependent(s1,s2))
      { return true; }
    else { return false; }
  }//end method formASegment
  
    public static boolean adjacent(Segment s1, Segment s2) {
	//returns true if s1,s2 are collinear and their intersection is a point
	//if (formALine(s1,s2) && (!overlap(s1,s2)) &&
	if (formALine(s1,s2) && 
		Mathset.linearly_dependent(s1,s2)) { return true; }
	else { return false; }
    }//end method adjacent


    public static boolean overlap(Segment s1, Segment s2){
	//true if s1,s2 overlap but don't only meet, false else
	//System.out.println("entering SSO.overlap..."); 
	
	if (s1.equal(s2))
	    { return true; }
	else {
	    boolean linDep = Mathset.linearly_dependent(s1,s2);
	    boolean formALin = formALine(s1,s2);
	    //System.out.println("SSO.overlap: linDep:"+linDep+", formALin: "+formALin);
	    if (linDep && !formALin) {
		if (PointSeg_Ops.isEndpoint(s1.startpoint,s2) ||
		    PointSeg_Ops.isEndpoint(s1.endpoint,s2) ||
		    PointSeg_Ops.isEndpoint(s2.startpoint,s1) ||
		    PointSeg_Ops.isEndpoint(s2.endpoint,s1)) { 
		    //System.out.println("SSO.overlap: true (case1)");
		    return true; }
		if (PointSeg_Ops.liesOn(s1.startpoint,s2) &&
		    PointSeg_Ops.liesOn(s1.endpoint,s2)) {
		    //System.out.println("SSO.overlap: true (case2)");
		    return true; }
		if (PointSeg_Ops.liesOn(s2.startpoint,s1) &&
		    PointSeg_Ops.liesOn(s2.endpoint,s1)) {
		    //System.out.println("SSO.overlap: true (case3)");
		    return true; }
	    }//if
	}//else
	return false;
    }//end method overlap


  public static Segment commonPart(Segment s1, Segment s2){
    //returns the common part of s1,s2 if any, dummy segment else
    if (overlap(s1,s2)) {
	if (PointSeg_Ops.liesOn(s1.startpoint,s2) &&
	    PointSeg_Ops.liesOn(s1.endpoint,s2)) {
	    return (Segment)s1.copy();
	}//if
	if (PointSeg_Ops.liesOn(s2.startpoint,s1) &&
	    PointSeg_Ops.liesOn(s2.endpoint,s1)) {
	    return (Segment)s2.copy();
	}//if

      if (PointSeg_Ops.liesOn(s1.startpoint,s2) &&
	  PointSeg_Ops.liesOn(s2.startpoint,s1)) {
	Segment e = new Segment(s2.startpoint,s1.startpoint);
	return e;
      }//if
      if (PointSeg_Ops.liesOn(s1.endpoint,s2) &&
	  PointSeg_Ops.liesOn(s2.startpoint,s1)) {
	Segment e = new Segment(s2.startpoint,s1.endpoint);
	return e;
      }//if
      if (PointSeg_Ops.liesOn(s1.startpoint,s2) &&
	  PointSeg_Ops.liesOn(s2.endpoint,s1)) {
	Segment e = new Segment(s2.endpoint,s1.startpoint);
	return e;
      }//if
      if (PointSeg_Ops.liesOn(s1.endpoint,s2) &&
	  PointSeg_Ops.liesOn(s2.endpoint,s1)) {
	Segment e = new Segment(s2.endpoint,s1.endpoint);
	return e;
      }//if
    }//if
    Segment e = new Segment();
    return e;
  }//end method common_part
 

    public static Segment union (Segment s1, Segment s2)
	throws NoOverlapException {
	
	//returns for two overlapping segments a single
	//segment describing their union
	//null else
	
	Segment retSeg = null;
	Point p1 = new Point();
	Point p2 = new Point();

	
	if (s1.equal(s2)) { return s1; }
	
	if (overlap(s1,s2)) {
	    if (PointSeg_Ops.liesOn(s1.startpoint,s2) &&
		!PointSeg_Ops.liesOn(s1.endpoint,s2)) { p1 = s1.endpoint; }//if
	    else {
		if (PointSeg_Ops.liesOn(s1.endpoint,s2) &&
		    !PointSeg_Ops.liesOn(s1.startpoint,s2)) { p1 = s1.startpoint; }//else
		else {
		    //both points lie on s2
		    return (Segment)s2.copy();
		}//else
	    }//else
	    if (PointSeg_Ops.liesOn(s2.startpoint,s1) &&
		!PointSeg_Ops.liesOn(s2.endpoint,s1)) { p2 = s2.endpoint; }//if
	    else {
		if (PointSeg_Ops.liesOn(s2.endpoint,s1) &&
		    !PointSeg_Ops.liesOn(s2.startpoint,s1)) { p2 = s2.startpoint; }//else
		else {
		    //both points lie on s1
		    return (Segment)s1.copy();
		}//else
	    }//else
	    retSeg = new Segment(p1,p2);
	}//if
	else { throw new NoOverlapException(); }

	return retSeg;
    }//union

    public static SegList symDiff(Segment s1, Segment s2) {
	//returns the 'symmetric difference' of two segments
	
	//System.out.println("entering SSO.symDiff...");
	//s1.print();
	//s2.print();

	SegList retSeg = new SegList();
	Point p1 = new Point();
	Point p2 = new Point();
	Point p3 = new Point();
	Point p4 = new Point();
	
	//if s1,s2 equal return mt
	if (s1.equal(s2)) { return retSeg; }

	//if s1,s2 overlap...
	if (overlap(s1,s2)) {
	    //if s1 fully lies on s2
	    if (PointSeg_Ops.liesOn(s1.startpoint,s2) &&
		PointSeg_Ops.liesOn(s1.endpoint,s2)) {
		//if s1's vertices aren't vertices of s2: we've two segments
		if (!PointSeg_Ops.isEndpoint(s1.startpoint,s2) &&
		    !PointSeg_Ops.isEndpoint(s1.endpoint,s2)) {
		    //use the proper vertices of s2 to build two new segments
		    if ((new Segment(s1.startpoint,s2.startpoint).length()) <
			(new Segment(s1.startpoint,s2.endpoint)).length()) {
			//if ((new Segment(s1.startpoint,s2.startpoint)).length.less((new Segment(s1.startpoint,s2.endpoint)).length)) {
			retSeg.add(new Segment(s1.startpoint,s2.startpoint));
			retSeg.add(new Segment(s1.endpoint,s2.endpoint));
			return retSeg;
		    }//if
		    else {
			retSeg.add(new Segment(s1.startpoint,s2.endpoint));
			retSeg.add(new Segment(s1.endpoint,s2.startpoint));
			return retSeg;
		    }//if
		}//if

		//s1.startpoint is vertex of s2: build one new segment
		if (PointSeg_Ops.isEndpoint(s1.startpoint,s2)) { 
		    if (s1.startpoint.equal(s2.startpoint)) {
			retSeg.add(new Segment(s1.endpoint,s2.endpoint));
			return retSeg;
		    }//if
		    else {
			retSeg.add(new Segment(s1.endpoint,s2.startpoint));
			return retSeg;
		    }//else
		}//if
		
		//s1.endpoint is vertex of s2: build one new segment
		if (PointSeg_Ops.isEndpoint(s1.endpoint,s2)) {
		    
		    if (s1.endpoint.equal(s2.startpoint)) {
			retSeg.add(new Segment(s1.startpoint,s2.endpoint));
			return retSeg;
		    }//if
		    else {
			retSeg.add(new Segment(s1.startpoint,s2.startpoint));
			return retSeg;
		    }//else
		}//if
	    }//if

	    //now do the same checks for s2
	    
	    //if s2 fully lies on s1
	    if (PointSeg_Ops.liesOn(s2.startpoint,s1) &&
		PointSeg_Ops.liesOn(s2.endpoint,s1)) {
		//if s2's vertices aren't vertices of s1: we've two segments
		if (!PointSeg_Ops.isEndpoint(s2.startpoint,s1) &&
		    !PointSeg_Ops.isEndpoint(s2.endpoint,s1)) {
		    //use the proper vertices of s1 to build two new segments
		    if ((new Segment(s2.startpoint,s1.startpoint)).length() <
			((new Segment(s2.startpoint,s1.endpoint)).length())) {
			//if ((new Segment(s2.startpoint,s1.startpoint)).length.less((new Segment(s2.startpoint,s1.endpoint)).length)) {
			retSeg.add(new Segment(s2.startpoint,s1.startpoint));
			retSeg.add(new Segment(s2.endpoint,s1.endpoint));
			return retSeg;
		    }//if
		    else {
			retSeg.add(new Segment(s2.startpoint,s1.endpoint));
			retSeg.add(new Segment(s2.endpoint,s1.startpoint));
			return retSeg;
		    }//if
		}//if

		//s2.startpoint is vertex of s1: build one new segment
		if (PointSeg_Ops.isEndpoint(s2.startpoint,s1)) {
		    if (s2.startpoint.equal(s1.startpoint)) {
			retSeg.add(new Segment(s2.endpoint,s1.endpoint));
			return retSeg;
		    }//if
		    else {
			retSeg.add(new Segment(s2.endpoint,s1.startpoint));
			return retSeg;
		    }//else
		}//if

		//if s2.endpoint is vertex of s1: build new segment
		if (PointSeg_Ops.isEndpoint(s2.endpoint,s1)) {
		    if (s2.endpoint.equal(s1.startpoint)) {
			retSeg.add(new Segment(s2.startpoint,s1.endpoint));
			return retSeg;
		    }//if
		    else {
			retSeg.add(new Segment(s2.startpoint,s1.startpoint));
			return retSeg;
		    }//else
		}//if
	    }//if
		    
	    else {
		if (PointSeg_Ops.liesOn(s1.startpoint,s2)) {
		    p1 = s1.endpoint;
		    p3 = s1.startpoint;
		}//if
		else {
		    p1 = s1.startpoint;
		    p3 = s1.endpoint;
		}//if
		if (PointSeg_Ops.liesOn(s2.startpoint,s1)) {
		    p2 = s2.startpoint;
		    p4 = s2.endpoint;
		}//if
		else {
		    p2 = s2.endpoint;
		    p4 = s2.startpoint;
		}//else
		retSeg.add(new Segment(p1,p2));
		retSeg.add(new Segment(p3,p4));
	    }//if
	}//else
	return retSeg;
    }//end method symDiff


    public static Segment theOverlap(Segment s1, Segment s2)
	throws NoOverlapException{
	
	//returns the overlapping part of two overlapping segments
	
	if (!overlap(s1,s2)) { throw new NoOverlapException(); }
	else {
	    if (s1.equal(s2)) { return s1; }//if
	    else { return commonPart(s1,s2); }
	}//else
    }//end method theOverlap


    public static Segment concat (Segment s1, Segment s2)
	throws NoAdjacentSegmentException{
	
	//System.out.println("\nentering SSO.concat...");
	//s1.print();
	//s2.print();

	//returns the concatenation of two adjacent segments
	Point p1;
	Point p2;
	if (!adjacent(s1,s2)) { throw new NoAdjacentSegmentException(); }
	else {
	    if (PointSeg_Ops.isEndpoint(s1.startpoint,s2)) { p1 = s1.endpoint; }
	    else { p1 = s1.startpoint; }
	    if (PointSeg_Ops.isEndpoint(s2.startpoint,s1)) { p2 = s2.endpoint; }
	    else { p2 = s2.startpoint; }
	}//else
	return (new Segment(p1,p2));
    }//end method concat

    
    public static SegList split (Segment s1, Segment s2) {
	//if s1,s2 properly intersect, or an endpoint lies on the other one but 
	//the don't only meet in their endpoints, 
	//the segments are split and the set of split segments are returned
	//if s1,s2 overlap they're also split!
	
	//System.out.println("entering SSO.split");
	//s1.print();
	//s2.print();


	SegList retList = new SegList();
	Point intPoint;// = new Point();
	Point intPoint1;// = new Point();
	Point intPoint2;// = new Point();
	
	//segments are equal
	if (s1.equal(s2)) {
	    //System.out.println("case1");
	    retList.add(s1);
	    return retList;
	}//if

	//segments properly intersect
	if (s1.pintersects(s2)) {
	    //System.out.println("case2");
	    //System.out.println("pintersects:"+s1.pintersects(s2));
	    intPoint = s1.intersection(s2);
	    //System.out.println("intPoint:"); intPoint.print();
	    retList.add(new Segment(s1.startpoint,intPoint));
	    retList.add(new Segment(s1.endpoint,intPoint));
	    retList.add(new Segment(s2.startpoint,intPoint));
	    retList.add(new Segment(s2.endpoint,intPoint));
	    return retList;
	}//if
	
	//segments overlap and form two segments
	if (overlap(s1,s2) &&
	    (PointSeg_Ops.isEndpoint(s2.startpoint,s1) ||
	     PointSeg_Ops.isEndpoint(s2.endpoint,s1) ||
	     PointSeg_Ops.isEndpoint(s1.startpoint,s2) ||
	     PointSeg_Ops.isEndpoint(s1.endpoint,s2))) {
	    //System.out.println("case3");
	    try {
		Segment ovLap = theOverlap(s1,s2);
		retList.add(ovLap);
		if (ovLap.equal(s1)) {
		    //System.out.println("case3.1");
		    retList.addAll(symDiff(s2,ovLap));
		}//if
		else if (ovLap.equal(s2)) {
		    //System.out.println("case3.2");
		    retList.addAll(symDiff(s1,ovLap));
		}//if
		else if (overlap(s1,ovLap)) {
		    //System.out.println("case3.3");
		    retList.addAll(symDiff(s1,ovLap));
		}//if
		else {
		    //System.out.println("case3.4");
		    retList.add(symDiff(s2,ovLap));
		}//else
		return retList;
	    }//try
	    catch (Exception e) {
	    System.out.println("Exception: "+e.getClass()+" --- "+e.getMessage());
	    System.exit(0);
	    }//catch
	}//if


	//segment overlap and form three segments:
	//one segment fully lies on the other one
	if (overlap(s1,s2) &&
	    (PointSeg_Ops.liesOn(s1.startpoint,s2) &&
	     PointSeg_Ops.liesOn(s1.endpoint,s2)) ||
	    (PointSeg_Ops.liesOn(s2.startpoint,s1) &&
	     PointSeg_Ops.liesOn(s2.endpoint,s1))) {
	    try {
		Segment ov = theOverlap(s1,s2);
		retList.add(ov);
		if (PointSeg_Ops.liesOn(s1.startpoint,s2)) {
		    retList.addAll(symDiff(ov,s2)); }
		else { retList.addAll(symDiff(ov,s1)); }
	    } catch (Exception e) {}
	    return retList;
	}//if

	//segments overlap and form three segments
	if (overlap(s1,s2)) {
	    //System.out.println("case4");
	    if (PointSeg_Ops.liesOn(s1.startpoint,s2)) { intPoint1 = s1.startpoint; }
	    else { intPoint1 = s1.endpoint; }
	    if (PointSeg_Ops.liesOn(s2.startpoint,s1)) { intPoint2 = s2.startpoint; }
	    else { intPoint2 = s2.endpoint; }
	    //System.out.println("intPoints:");
	    //System.out.println("splitted:");
	    retList.add(new Segment(intPoint1,intPoint2)); //(new Segment(intPoint1,intPoint2)).print();
	    retList.add(new Segment(s1.theOtherOne(intPoint1),intPoint1)); //(new Segment(s1.theOtherOne(intPoint1),intPoint1)).print();
	    retList.add(new Segment(s2.theOtherOne(intPoint2),intPoint2)); //(new Segment(s2.theOtherOne(intPoint2),intPoint2)).print();
	    return retList;
	}//if

	//segments form a line
	if (formALine(s1,s2)) {
	    //System.out.println("case5");
	    retList.add(s1);
	    retList.add(s2);
	    return retList;
	}//if

	//one endpoint lies on the other segment
	if (PointSeg_Ops.liesOn(s1.startpoint,s2)) {
	    //System.out.println("case6");
	    intPoint = s1.startpoint;
	    retList.add(s1);
	    retList.add(new Segment(s2.startpoint,intPoint));
	    retList.add(new Segment(s2.endpoint,intPoint));
	    return retList;
	}//if
	else if (PointSeg_Ops.liesOn(s1.endpoint,s2)) {
	    intPoint = s1.endpoint;
	    retList.add(s1);
	    retList.add(new Segment(s2.startpoint,intPoint));
	    retList.add(new Segment(s2.endpoint,intPoint));
	    return retList;
	}//if
	else if (PointSeg_Ops.liesOn(s2.startpoint,s1)) {
	    intPoint = s2.startpoint;
	    retList.add(s2);
	    retList.add(new Segment(s1.startpoint,intPoint));
	    retList.add(new Segment(s1.endpoint,intPoint));
	    return retList;
	}//if
	
	//so both segments don't even have common points
	//System.out.println("case8");
	retList.add(s1);
	retList.add(s2);
		
	return retList;
    }//end method split


    public static SegList minus (Segment seg1, Segment seg2) {
	//subtracts seg2 from seg1 if they overlap
	//return set of segments that don't overlap
	//full seg1 else
	SegList retList = new SegList();
	if (!overlap(seg1,seg2)) { 
	    retList.add(seg1);
	    return retList;
	}//if

	//segments are equal
	if (seg1.equal(seg2)) {
	    return retList; }
	
	//one point is equal
	boolean s1s = PointSeg_Ops.isEndpoint(seg1.startpoint,seg2);
	boolean s1e = PointSeg_Ops.isEndpoint(seg1.endpoint,seg2);
	
	if (s1s || s1e) {
	    boolean s2s = PointSeg_Ops.isEndpoint(seg2.startpoint,seg1);
	    boolean s2e = PointSeg_Ops.isEndpoint(seg2.endpoint,seg1);
	    if (s1s && s2s) {
		retList.add(new Segment(seg1.endpoint,seg2.endpoint)); }
	    else if (s1s && s2e) {
		retList.add(new Segment(seg1.endpoint,seg2.startpoint)); }
	    else if (s1e && s2s) {
		retList.add(new Segment(seg1.startpoint,seg2.endpoint)); }
	    else {
		retList.add(new Segment(seg1.startpoint,seg2.startpoint)); }
	    return retList;
	}//if

	//no equal points
	//seg1 lies fully on seg2
	if (PointSeg_Ops.liesOn(seg1.startpoint,seg2) &&
	    PointSeg_Ops.liesOn(seg1.endpoint,seg2)) {
	    return retList; }
	//seg2 lies fully on seg1
	if (PointSeg_Ops.liesOn(seg2.startpoint,seg1) &&
	    PointSeg_Ops.liesOn(seg2.endpoint,seg1)) {
	    if (seg1.startpoint.dist(seg2.startpoint).less
		(seg1.startpoint.dist(seg2.endpoint))) {
		retList.add(new Segment(seg1.startpoint,seg2.startpoint));
		retList.add(new Segment(seg1.endpoint,seg2.endpoint));
		return retList; }
	    else {
		retList.add(new Segment(seg1.startpoint,seg2.endpoint));
		retList.add(new Segment(seg1.endpoint,seg2.startpoint));
		return retList; }
	}//if

	System.out.println("Error in SSO.minus: uncaught case.");
	System.exit(0);
	return retList;
    }//end method minus

							 

} //end class SegSeg_Ops
