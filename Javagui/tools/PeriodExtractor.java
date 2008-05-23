package tools;
import sj.lang.ListExpr;
import viewer.hoese.algebras.periodic.Time;
import viewer.hoese.algebras.periodic.RelInterval;
import java.io.*;
import java.util.Vector;


/*
This class extracts a periodic moving point to
a linear moving one.
An infinite Period is extracted two times and
a message informing about this is printed out to
the standardoutput.

*/

public class PeriodExtractor{

private static void extractLinearMove(Time T, ListExpr LinearMove){
  int len = LinearMove.listLength();
  if( (len!=2) && (len!=3)){
     error("invalid length for linear move");
  }
  RelInterval Duration = new RelInterval();
  if(!Duration.readFrom(LinearMove.first())){
      error("Error in reading duration of a linear move ");
  }
  if(len==2){  // represents a undefined linear move
     if(LinearMove.second().atomType()!=ListExpr.BOOL_ATOM ||
        LinearMove.second().boolValue()!=false)
        error("Invalid value for undefined linear move ");
     T.addInternal(Duration.getLength()); // we have to print nothing
     return;
  }
  System.out.print("   ( ("); // open unit and interval
  System.out.print(T.getListExprString(true)+" "); //  print startTime
  T.addInternal(Duration.getLength());
  System.out.print(T.getListExprString(true)+" ");
  String B = Duration.isLeftClosed()? "TRUE":"FALSE";
  System.out.print(B+" ");
  B = Duration.isRightClosed()? "TRUE":"FALSE";
  System.out.print(B+"  )");  // close Interval
  System.out.print("("); // open pointmap
  System.out.print(LinearMove.second().first().writeListExprToString()); // startpoint
  System.out.print(LinearMove.second().second().writeListExprToString());
  System.out.print(LinearMove.third().first().writeListExprToString()); // endpoint
  System.out.print(LinearMove.third().second().writeListExprToString());
  System.out.println(") )"); // close pointmap, Unit
}

private static void extractCompositeMove(Time T, ListExpr Moves){
   while(!(Moves.isEmpty())){
      extractSubMove(T,Moves.first());
      Moves = Moves.rest();
   }
}

private static void extractPeriodMove(Time T, ListExpr Move){
   if(Move.listLength()!=2)
      error("wrong listlength for periodic move ");

   if(Move.first().atomType()!=ListExpr.INT_ATOM)
      error("not an int-atom for number of repeatations");
   int R = Move.first().intValue();
   if(R==0 || R < -2 ){
     error("wrong number of repeatations");
   }
   if(R<0){
      System.err.println("Warning : Infinite Period found, repeat only 3 times ");
   }
   if(R==-1){ // LeftInfinite, adjust StartTime
       error("Can't handle leftinifinite periods");
   }
   if(R==-2){  // cut to 3 repeatations
      R = 3;
   }
   for(int i=0; i<R;i++)
       extractSubMove(T,Move.second());
}


private static void extractSubMove(Time T, ListExpr SubMove){
   if(SubMove.listLength()!=2)
       error("Invalid SubMove, wrong list length");
   if(SubMove.first().atomType()!=ListExpr.SYMBOL_ATOM)
       error("Invalid SubMove, type not a symvbol");
   String Type = SubMove.first().symbolValue();
   if(Type.equals("linear"))
       extractLinearMove(T,SubMove.second());
   else if(Type.equals("composite"))
        extractCompositeMove(T,SubMove.second());
   else if(Type.equals("period"))
        extractPeriodMove(T,SubMove.second());
   else
       error("invalid SubMove Type :"+Type);
}

private static void extractPoint(ListExpr LE,boolean printType){
   if(LE.listLength()!=2)  // (starttime move)
      showUsage("Invalid value list for periodic moving point");
   Time T = new Time();
   if(!T.readFrom(LE.first()))
      showUsage("invalid start time");
   if(printType)
      System.out.println("  ( mpoint (");
   else
      System.out.println("  ( ");
   extractSubMove(T,LE.second());
   if(printType)
      System.out.println("  ))");
   else
      System.out.println("  ) ");
}


private static void showUsage(String Message){
  System.err.println(Message);
  System.err.println("Usage: java PeriodExtractor File");
  System.exit(1);
}

private static void error(String Message){
  System.err.println(Message);
  System.exit(1);
}

private static void extractRelation(ListExpr type, ListExpr value){
   extractRelation(ListExpr.twoElemList(type,value),false);
}

private static void extractRelation(ListExpr LE,boolean printOuterBrackets){
   // process a relation
   if(LE.first().atomType()!=LE.NO_ATOM)
      error("can't handle this type");
   ListExpr Type = LE.first();
   ListExpr Value = LE.second();

   if(Type.listLength()!=2){
     error("unknown type");
   }
   if(Type.first().atomType()!=LE.SYMBOL_ATOM  ||
      !Type.first().symbolValue().equals("rel"))
      error("not a pmpoint or a relation");
   if(Type.second().listLength()!=2)
       error("not a valid relation");
   ListExpr TypeTupleList = Type.second().second();
   if(printOuterBrackets) System.out.print("( ");
   System.out.println("( rel (tuple (");
   Vector V = new Vector();  // here we store the attribute numbers for pmpoints
   int no = 0;
   while(!TypeTupleList.isEmpty()){
      ListExpr Tuple = TypeTupleList.first();
      TypeTupleList=TypeTupleList.rest();
      if(Tuple.listLength()==2 && Tuple.second().atomType()==LE.SYMBOL_ATOM
         && Tuple.second().symbolValue().equals("pmpoint")){
         System.out.println("  (" + Tuple.first().symbolValue() + " mpoint )");
         V.add(new Integer(no));
      }
      else
         System.out.println("  "+Tuple.writeListExprToString());
      no++;
   }
   System.out.println(" )))"); // close tuplelist,relation and type

   // process tuples
   System.out.println("("); // open valuelist
   while(!Value.isEmpty()){
      ListExpr Tuple = Value.first();
      Value = Value.rest();
      no =0;
      System.out.println("("); // open tuple
      while(!Tuple.isEmpty()){
           ListExpr  attr = Tuple.first();
           Tuple=Tuple.rest();
           if(V.contains(new Integer(no)))
              extractPoint(attr,false);
           else
              attr.writeListExpr();
           no++;
      }
      System.out.println(")"); // close tuple
   }
   System.out.println(")"); // close value list
   if(printOuterBrackets) System.out.print(")");
   System.out.println("");
}


private static void extractObject(ListExpr LE){

  ListExpr Type = LE.fourth();
  if(Type.atomType()==ListExpr.SYMBOL_ATOM &&
     Type.symbolValue().equals("pmpoint")){
     System.out.println("(OBJECT ");
     LE.second().writeTo(System.out,false);     
     LE.third().writeTo(System.out,false);
     System.out.println("mpoint");
     extractPoint(LE.fifth(),false);
     LE.sixth().writeTo(System.out,false);
     System.out.println(")");
     return;
  }
  if(Type.atomType()==ListExpr.NO_ATOM &&
     Type.listLength()==2 &&
     Type.first().atomType()==ListExpr.SYMBOL_ATOM &&
     Type.first().symbolValue().equals("rel")){
     System.out.println("(OBJECT ");
     LE.second().writeTo(System.out,false);
     LE.third().writeTo(System.out,false);
     extractRelation(LE.fourth(),LE.fifth());
     LE.sixth().writeTo(System.out,false);
     
     System.out.println(")");
     return;  
  }
  LE.writeTo(System.out,false); 
}

private static void extractLE(ListExpr LE){
   int AT = LE.atomType();
   switch(AT){
      case ListExpr.SYMBOL_ATOM : { System.out.print(" "+LE.symbolValue()+" ");
                                    break;
				  }
      case ListExpr.STRING_ATOM : { System.out.print(" \""+LE.stringValue()+"\" ");
                                    break;
				  }
      case ListExpr.BOOL_ATOM :   { if(LE.boolValue())
                                       System.out.print(" TRUE ");
				    else 
				       System.out.print(" FALSE ");
				    break;
				  }
      case ListExpr.TEXT_ATOM :   { System.out.print(" <text>"+LE.textValue()+"</text---> ");
                                    break;
                                   }       
      case ListExpr.INT_ATOM  :   { System.out.print(" "+LE.intValue()+" ");
                                    break;
				  }
      case ListExpr.REAL_ATOM :   { System.out.print(" "+LE.realValue()+" ");
                                    break;
				  }
      case ListExpr.NO_ATOM : {   boolean processed=false;
                                  if(LE.listLength()==2){
				     if(LE.first().atomType()==ListExpr.SYMBOL_ATOM &&
				        LE.first().symbolValue().equals("pmpoint")){
					   extractPoint(LE.second(),true);
					   processed=true;
			             }else{
				        if(LE.first().atomType()==ListExpr.NO_ATOM &&
					   LE.first().listLength()==2 &&
					   LE.first().first().atomType()==ListExpr.SYMBOL_ATOM &&
					   LE.first().first().symbolValue().equals("rel")){
					   extractRelation(LE,true);
					   processed=true;   
					}   
				     }
                                  } 
				  if(LE.listLength()==6 &&
				     LE.first().atomType()==ListExpr.SYMBOL_ATOM &&
				     LE.first().symbolValue().equals("OBJECT")){
				     extractObject(LE);
				     processed=true;
				  }
          			 if(!processed){
				   System.out.print("(");
				   while(!LE.isEmpty()){
				      extractLE(LE.first());
				      LE=LE.rest();
				   }
				   System.out.println(")");
				 }
				 break;
                              }
      default               : error("unknow atom type detected ");
   }				  
}


public static void main(String[] args){
   if(args.length!=1)
      showUsage("Missing Argument");
   File F = new File(args[0]);
   if(!F.exists())
      error("File not found");
   ListExpr LE = new ListExpr();
   if(!(LE.readFromFile(args[0])==0))
      error("File "+args[0]+" don't contains a nested list");

   if(LE.atomType()!=ListExpr.NO_ATOM){
       LE.writeListExpr();
       return;
   }    
   
   if(LE.listLength()==2){
        // a single periodic moving point
        if(LE.first().atomType()==LE.SYMBOL_ATOM &&
           LE.first().symbolValue().equals("pmpoint")){
             extractPoint(LE.second(),true);
             return;
	}
	// extract a relation
	if(LE.first().atomType()==ListExpr.NO_ATOM &&
	   LE.first().listLength()==2 &&
	   LE.first().first().atomType()==ListExpr.SYMBOL_ATOM &&
	   LE.first().first().symbolValue().equals("rel")){
	   extractRelation(LE,true);
	   return;
	}   
   }
   extractLE(LE);

}

}
