class PointPol_Ops {

  //variables

  //constructors

  //methods
    /*
  public static boolean lies_on_border(Point poi, Polygon pol){
    //true if p lies on the border of pol, false else
    Segment s = new Segment();
    boolean w = false;
    for (int i = 0; i < pol.border.size(); i++) {
      s = (Segment)pol.border.get(i);
      if (PointSeg_Ops.lies_on(poi,s)){
	w = true;
	break;
      }//if
    }//for
    return w;
  }//end method lies_on_border
    */
    /*
  public static boolean inside(Point poi, Polygon pol) {
    //returns true if poi lies inside one of pol's triangles
    for (int i = 0; i < pol.trilist.size(); i++) {
      if (PointTri_Ops.inside(poi,(Triangle)pol.trilist.get(i))) {
	return true;
      }//if
    }//for i
    return false;
  }//end method inside
    */
}//end class PointPol_Ops
