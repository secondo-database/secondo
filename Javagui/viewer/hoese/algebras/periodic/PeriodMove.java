package viewer.hoese.algebras.periodic;
import sj.lang.ListExpr;

public class PeriodMove implements Move{

public RelInterval getInterval(){ return interval;}

/** creates an undefined PeriodMove */
public PeriodMove(){
   defined = false;
}

/** sets this PeriodMove to an undefined state */
private void setUndefined(){
   defined=false;
   this.subMove = null;
   this.interval = null;
   this.repeatations=0;
}

/** sets this PeriodMove to the given values.
  * If the arguments don't decribe a valid PeriodMove,
  * the resulting state will be undefined and the returnvalue
  * will be false
  */
public boolean set(int repeatations, Move subMove){
   RelInterval smInterval = subMove.getInterval();
   if(smInterval.isLeftInfinite() || smInterval.isRightInfinite()){
      if(Environment.DEBUG_MODE)
         System.err.println("PeriodMove.set: try to repeat a infinite move ");
      setUndefined();
      return false;
   }
   if(repeatations==LEFTINFINITE){
      (interval = new RelInterval()).setLeftInfinite(smInterval.isRightClosed());
      this.repeatations = repeatations;
      this.subMove = subMove;
      defined = true;
      return true;
   }
   if(repeatations==RIGHTINFINITE){
      (interval =  new RelInterval()).setRightInfinite(smInterval.isLeftClosed());
      this.repeatations = repeatations;
      this.subMove = subMove;
      defined = true;
      return true;
   }
   if(repeatations<=0){
      if(Environment.DEBUG_MODE)
         System.err.println("PeriodMove.set: invlaid number of repeatation :"+repeatations);
      setUndefined();
      return false;
   }
   if(!(smInterval.isLeftClosed() ^ smInterval.isRightClosed())){
      if(Environment.DEBUG_MODE)
         System.err.println("PeriodMove.set: try to repeat an move with an completely open (closed) interval");
      setUndefined();
      return false;
   }
   this.repeatations = repeatations;
   this.interval = smInterval.copy();
   this.interval.setLength(this.interval.getLength().mul(repeatations));
   this.subMove = subMove;
   defined=true;
   return true;
}



/** returns the object at the given time.
  * If the object is not defined at this time null is returned.
  */
public Object getObjectAt(Time T){

   if(!defined){
     if(Environment.DEBUG_MODE)
        System.err.println("PeriodMove.getObjectAt on an undefined move called");
     return null;
   }

   if(!interval.contains(T)){
      return null;
   }
   RelInterval SMInterval = subMove.getInterval();
   Time SMLength = SMInterval.getLength().copy();
   if(T.compareTo(Time.ZeroTime)<0){ // must be left-infinite
      while(!SMInterval.contains(T)){ // has to be improve by direct computation
          T = T.add(SMLength);
      }
      return subMove.getObjectAt(T);
   }

   long Fact = SMInterval.getExtensionFactor(T);
   if(Fact<0){
      if(Environment.DEBUG_MODE)
         System.err.println("PeriodMove.getObjectAt ExtensionFactor <0 ");
      return null;
   }

   T = T.minus(SMLength.mul(Fact-1));

   return subMove.getObjectAt(T);
}

/** reads this PeriodMove from its nested list representation.
  * If the list don't represent a valid period move this will
  * be undefined and false is returned.
  */
public boolean readFrom(ListExpr LE,Class linearClass){
   if(LE.listLength()!=2){
      if(Environment.DEBUG_MODE)
         System.err.println("PeriodMove.readFrom :: wrong ListLength()");
      setUndefined();
      return false;
   }
   if(LE.first().atomType()!=LE.SYMBOL_ATOM){
      if(Environment.DEBUG_MODE)
         System.err.println("PeriodMove.readFrom :: wrong type of typedescriptor");
      setUndefined();
      return false;
   }
   if(!LE.first().symbolValue().equals("period")){
      if(Environment.DEBUG_MODE)
         System.err.println("PeriodMove.readFrom :: wrong Value for typedescriptor");
      setUndefined();
      return false;
   }
   if(LE.second().atomType()!=LE.NO_ATOM){
      if(Environment.DEBUG_MODE)
         System.err.println("PeriodMove.readFrom :: value is not a list");
      setUndefined();
      return false;
   }
   ListExpr Value = LE.second();
   if(Value.listLength()!=2){
      if(Environment.DEBUG_MODE)
         System.err.println("PeriodMove.readFrom :: wrong ListLength() for value list");
      setUndefined();
      return false;
   }
   if(Value.first().atomType()!=LE.INT_ATOM){
     if(Environment.DEBUG_MODE)
         System.err.println("PeriodMove.readFrom :: wrong type for repeatations");
      setUndefined();
      return false;
   }
   int rep = Value.first().intValue();
   if(rep<=1 && rep!=LEFTINFINITE && rep!=RIGHTINFINITE){
      if(Environment.DEBUG_MODE)
         System.err.println("PeriodMove.readFrom :: not a valid number of repeatations");
      setUndefined();
      return false;
   }
   if(Value.second().atomType()!=LE.NO_ATOM || Value.second().listLength()<1){
      if(Environment.DEBUG_MODE)
         System.err.println("PeriodMove.readFrom :: wrong list type for submove");
      setUndefined();
      return false;
   }
   if(Value.second().first().atomType()!=LE.SYMBOL_ATOM){
      if(Environment.DEBUG_MODE)
         System.err.println("PeriodMove.readFrom :: wrong listtype for type descriptor of the submove");
      setUndefined();
      return false;
   }
   String s = Value.second().first().symbolValue();
   if(!s.equals("linear") && !s.equals("composite")){
      if(Environment.DEBUG_MODE)
         System.err.println("PeriodMove.readFrom :: wrong type descriptor for subtype");
      setUndefined();
      return false;
   }
   Move sm;
   try{
      if(s.equals("linear"))
         sm = (Move) linearClass.newInstance();
      else
         sm = new CompositeMove();
   }catch(Exception e){
      if(Environment.DEBUG_MODE){
         System.err.println("PeriodMove.readFrom :: error in creating submove\n"+e);
	 e.printStackTrace();
      }
      setUndefined();
      return false;
   }

   if(!sm.readFrom(Value.second(),linearClass)){
      if(Environment.DEBUG_MODE)
         System.err.println("PeriodMove.readFrom :: error in reading submove ");
      setUndefined();
      return false;
   }
   return set(rep,sm);
}

public BBox getBoundingBox(){
   if(!defined){
      if(Environment.DEBUG_MODE)
        System.err.println("PeriodMove.getBoundingBox with an undefined instance");
      return null;
   }
   if(subMove==null){
     if(Environment.DEBUG_MODE)
        System.err.println("PeriodMove.getBoundingBox without a submove (null)");
       return null;
   }
   return subMove.getBoundingBox();
}

public String toString(){
  return "PeriodMove  [defined ="+defined+"   repeats ="+repeatations+
         "   interval ="+interval + "\nSubmove ="+subMove;
}

private boolean defined;
private int repeatations;
private RelInterval interval;
private Move subMove;
private Class linearClass;
public static final int LEFTINFINITE=-1;
public static final int RIGHTINFINITE=-2;


}


