//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science,
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307

import project.*;
import sj.lang.ListExpr;
import viewer.hoese.LEUtils;

/* Can be used to project spatial data into a
 * desired space. The data itself must be given in geographic coordinates.
 */

public class Projector{

/** Holds the projection which is used 
  * for the transformation.
  **/
private Projection projection;
/** avoids a frequently creation of a point objects **/
private java.awt.geom.Point2D.Double aPoint = new java.awt.geom.Point2D.Double();

/* some constants */
private static final int POINT    = 0;
private static final int POINTS   = 1;
private static final int LINE     = 2;
private static final int REGION   = 3;
private static final int RELATION = 4;
private static final int OTHER    = 5;


public Projector(Projection projection){
   this.projection = projection;
}



/** Projects a point value using the currenmt projection.
 * returns a listexpr containing the projected point.
 * If any errors occur, the result will be null.
 **/
private ListExpr projectPoint(ListExpr original){
   if(original.listLength()!=2){
     return null;  
   }
   Double x = LEUtils.readNumeric(original.first());
   Double y = LEUtils.readNumeric(original.second());
   if(x==null || y==null){
       System.err.println("wrong List format for a point");
       return null;
   }
   if(!projection.project(x.doubleValue(),y.doubleValue(),aPoint)){
      System.err.println("values outside of the valid range for the choosed projection");
      return null;
   }
   return ListExpr.twoElemList(ListExpr.realAtom(aPoint.getX()),
                               ListExpr.realAtom(aPoint.getY()));
}

/** Projects a Line segment using the current projection.
  * If any error the result will be null.
  **/

private ListExpr projectSegment(ListExpr original){
   if(original.listLength()!=4){
      return null;
   }
   Double x1 = LEUtils.readNumeric(original.first());
   Double y1 = LEUtils.readNumeric(original.second());
   Double x2 = LEUtils.readNumeric(original.third());
   Double y2 = LEUtils.readNumeric(original.fourth());
   if(x1==null || y1==null || x2==null || y2==null){
        return null;
   }
   if(!projection.project(x1.doubleValue(),y1.doubleValue(),aPoint)){
       return null;
   }
   x1 = aPoint.getX();
   y1 = aPoint.getY();
   if(!projection.project(x1.doubleValue(),y1.doubleValue(),aPoint)){
        return null;
   }
   return ListExpr.fourElemList(ListExpr.realAtom(x1),
                                ListExpr.realAtom(y1),
                                ListExpr.realAtom(aPoint.getX()),
                                ListExpr.realAtom(aPoint.getY()));
}

/** project a complete line, returns null when
  *  an error occurs.
  **/
private ListExpr projectLine(ListExpr original){
  if(original.atomType()!=ListExpr.NO_ATOM){
     return null;
  }
  if(original.isEmpty()){
     return new ListExpr();
  }
  ListExpr segment = projectSegment(original.first());
  if(segment==null){
     return null;
  }
  ListExpr result = ListExpr.oneElemList(segment);
  ListExpr last = result;
  original = original.rest();
  while(!original.isEmpty()){
      segment = projectSegment(original.first());    
      if(segment==null){
         return null;
      }
      last = ListExpr.append(last,segment);
      original = original.rest();
  }
  return result; 
}

/** Projects a cycle of points **/
private ListExpr projectCycle(ListExpr original){
   if(original.atomType()!=ListExpr.NO_ATOM){
      return null;
   }
   if(original.isEmpty()){
     return new ListExpr();
   }
   ListExpr point = projectPoint(original.first());
   if(point ==null){
       return null;
   } 
   ListExpr result = ListExpr.oneElemList(point);
   ListExpr last = result;
   original = original.rest();
   while(!original.isEmpty()){
       point = projectPoint(original.first());
       if(point ==null){
         return null;
       } 
       last = ListExpr.append(last,point);
       original = original.rest();
   }
   return result;
}

/** projects a single face of a region **/
private ListExpr projectFace(ListExpr original){
   if(original.atomType()!=ListExpr.NO_ATOM){
      return null;
   }
   if(original.isEmpty()){
     return new ListExpr();
   }
   ListExpr cycle = projectCycle(original.first());
   if(cycle ==null){
       return null;
   } 
   ListExpr result = ListExpr.oneElemList(cycle);
   ListExpr last = result;
   original = original.rest();
   while(!original.isEmpty()){
       cycle = projectCycle(original.first());
       if(cycle ==null){
         return null;
       } 
       last = ListExpr.append(last,cycle);
       original = original.rest();
   }
   return result;
} 


/** projects a value of type points **/
private ListExpr projectPoints(ListExpr original){
   return projectCycle(original);
}

/** projects a region value **/
private ListExpr projectRegion(ListExpr original){
   if(original.atomType()!=ListExpr.NO_ATOM){
      return null;
   }
   if(original.isEmpty()){
     return new ListExpr();
   }
   ListExpr face = projectFace(original.first());
   if(face ==null){
       return null;
   } 
   ListExpr result = ListExpr.oneElemList(face);
   ListExpr last = result;
   original = original.rest();
   while(!original.isEmpty()){
       face = projectFace(original.first());
       if(face==null){
         return null;
       } 
       last = ListExpr.append(last,face);
       original = original.rest();
   }
   return result;
} 


/** projects a single tuple of a relation**/
private ListExpr projectTuple(int[] types,ListExpr original){
  if(original.listLength()!=types.length){
     return null;
  }
  ListExpr first = original.first();
  switch(types[0]){
     case POINT:  first = projectPoint(first); break;
     case POINTS: first = projectPoints(first); break;
     case LINE:   first = projectLine(first); break;
     case REGION: first = projectRegion(first); break;
  } 
  if(first==null){
    return null;
  }
  ListExpr result =  ListExpr.oneElemList(first);
  ListExpr last = result;
  original = original.rest();
  int index = 1;
  while(!original.isEmpty()){
			first = original.first();
			switch(types[index]){
				 case POINT:  first = projectPoint(first); break;
				 case POINTS: first = projectPoints(first); break;
				 case LINE:   first = projectLine(first); break;
				 case REGION: first = projectRegion(first); break;
			} 
			if(first==null){
				return null;
			}
      last = ListExpr.append(last,first);
      original = original.rest();
      index++;
  }  
  return result;
}


/** projects a complete relation 
  * The result will be the value of the region where all entries of
  * type line, point and region are projected
  * type must be of type (rel(tuple( (...)(...))))
**/
private ListExpr projectRelation(ListExpr type, ListExpr value){
  if(type.listLength()!=2)
     return null;
  if(    type.first().atomType()!=ListExpr.SYMBOL_ATOM 
      || !type.first().symbolValue().equals("rel")){
      return null;
  } 
  type = type.second();
  if(type.listLength()!=2)
     return null;
  if(    type.first().atomType()!=ListExpr.SYMBOL_ATOM 
      || !type.first().symbolValue().equals("tuple")){
      return null;
  } 
  type = type.second(); // set to the attributelist
  int[] attribs = new int[type.listLength()];
  if(attribs.length==0){
      return null;
  }
  for(int i=0;i<attribs.length;i++){
     ListExpr attrib = type.first();
     if(attrib.listLength()!=2){
        return null;
     }
     ListExpr attrType = attrib.second();
     if(attrType.atomType()!=ListExpr.SYMBOL_ATOM){
        attribs[i] = OTHER;
     } else{
        String typename = attrType.symbolValue();
        if(typename.equals("point")){
           attribs[i] = POINT;
        } else if(typename.equals("points")){
           attribs[i] = POINTS;
        } else if(typename.equals("line")){
           attribs[i] = LINE;
        } else if(typename.equals("region")){
           attribs[i] = REGION; 
        } else{
           attribs[i] = OTHER;
        }
    }
    type = type.rest();
  } // analysing type done
  
  if(value.atomType()!=ListExpr.NO_ATOM){
    return null;
  }     
  if(value.isEmpty()){
      return new ListExpr();
  }
 
  ListExpr first = projectTuple(attribs,value.first());
  if(first==null){
     return null;
  } 
  ListExpr result = ListExpr.oneElemList(first);
  ListExpr last =   result;
  value = value.rest();
  while(!value.isEmpty()){
    first = projectTuple(attribs,value.first());
    if(first==null){
       return null;
    }
    last = ListExpr.append(last,first); 
    value = value.rest();
  } 
  return result;
}


private int getType(ListExpr type){
   if(type.atomType()==ListExpr.SYMBOL_ATOM){
      String name = type.symbolValue();
      if(name.equals("point")){
        return POINT;
      }
      if(name.equals("points")){
        return POINTS;
      }
      if(name.equals("line")){
         return LINE;
      }
      if(name.equals("region")){
         return REGION;
      }
      return OTHER;
   }
   if(type.atomType()!=ListExpr.NO_ATOM){
      return OTHER;
   }
   if(type.listLength()!=2){
      return OTHER;
   }
   if(type.first().atomType()==ListExpr.SYMBOL_ATOM &&
      type.first().symbolValue().equals("rel")){
      return RELATION;
   }
   return OTHER;
}


private ListExpr projectTypeValue(ListExpr original){
    if(original.listLength()!=2)
       return null;
    // listexpr in format (type, value)
    int type = getType(original.first()); 
    ListExpr value;
    switch(type){
      case POINT:     value = projectPoint(original.second()); break;
      case POINTS:    value = projectPoints(original.second()); break;
      case LINE:      value = projectLine(original.second()); break;
      case REGION:    value = projectRegion(original.second()); break;
      case RELATION:  value = projectRelation(original.first(),original.second()); break;
      default:        value = original.second();
    }
    if(value==null){
       return null;
    }
    return ListExpr.twoElemList(original.first(), value);

}

/** projects a nested list in object format **/
private ListExpr projectObject(ListExpr original){
  int len = original.listLength();
  if(len!=5 && len!=6){
     return null;
  }  
  ListExpr tv = projectTypeValue(ListExpr.twoElemList(original.fourth(),original.fifth()));
  if(tv==null){
     return null;
  }
  if(len==5){
    return ListExpr.fiveElemList( original.first(),
                                  original.second(),
                                  original.third(),
                                  tv.first(),
                                  tv.second());
  } else{
    return ListExpr.sixElemList( original.first(),
                                  original.second(),
                                  original.third(),
                                  tv.first(),
                                  tv.second(),
                                  original.sixth());
  }
}

/** projects a nested list representing a whole database **/
private ListExpr projectDatabase(ListExpr original){
  if(original.listLength()!=4){
     return null;
  }
  ListExpr objects = original.fourth();
  if(objects.atomType()!=ListExpr.NO_ATOM){
      return null;
  }
  if(objects.first().atomType()!=ListExpr.SYMBOL_ATOM  ||
     !objects.first().symbolValue().equals("OBJECTS")){
     return null;
  }
  ListExpr result = ListExpr.oneElemList(ListExpr.symbolAtom("OBJECTS"));
  ListExpr last = result;
  objects = objects.rest();
  while(!objects.isEmpty()){
     ListExpr object = projectObject(objects.first());
     if(object==null){
        return null;
     }
     last = ListExpr.append(last,object);
     objects = objects.rest();
  }
  return ListExpr.fourElemList( original.first(), original.second(),
                                original.third(), result);

}




/** This functions takes any ListExpr and returns the 
  * corresponding ListExpr where the values are 
  * projected using the current projection. 
  * ListExpr in format of point, points,region, line, relation,object, 
  * database are recognized and projected. If an error occurs, the resulot will
  * be null. List which are not corresponding to the formats mentioned above, are
  * returned without any changes.
  */
public ListExpr projectList(ListExpr original){
  int len = original.listLength();
  if(len<2){ // nothing known about lists with less than two elements
     return original;
  }
  if(len==2){
     return projectTypeValue(original);
  }
  // check for OBJECT or DATABASE at the first position
  ListExpr first = original.first();
  if(first.atomType()!=ListExpr.SYMBOL_ATOM){
     return original;
  }
  String h = first.symbolValue();
  if(h.equals("OBJECT")){
     return projectObject(original);
  } 
  if(h.equals("DATABASE")){
     return projectDatabase(original);
  }
  return original;
}

private static void showUsage(){
   System.err.println(" java Projector <projection> infile [outfile]");
   System.exit(0);
}

public static void main(String[] args){
   if(args.length!=2 && args.length!=3){
      showUsage();
   }
   ListExpr.initialize(100000);
   // try to load the projection
   Projection prj = null;
   try{
     prj = (Projection) Class.forName("project."+args[0]).newInstance();
   }catch(Exception e){
       System.err.println("cannot load the projection");
       System.exit(1);
   }
   Projector projector = new Projector(prj);
   ListExpr source = new ListExpr();
   if(source.readFromFile(args[1])!=0){
       System.err.println("cannot load list from "+args[1]);
       System.exit(1);
   }
   ListExpr dest = projector.projectList(source);
   if(dest==null){
     System.err.println("problem in converting the list");
     System.exit(1);
   }
   if(args.length==2){
      dest.writeListExpr();
      System.exit(0);
   }
   if(dest.writeToFile(args[2])!=0){
      System.err.println("problem in writing the list to "+args[2]);
      System.exit(1);
   }
   
   


}




}


