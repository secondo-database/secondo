

package tools;


/** class representing a pair of objects **/

public class Pair<F, S>{


  /** creates a pair from the arguments **/
   public Pair(F first, S second){
      this.first = first;
      this.second = second;
   }

   /** returns the first element of this pair. **/
   public F first(){ return first; }

   /** returns the second element of this par **/
   public S second(){ return second; }

   /** sets the first element of this pair **/
   public void setFirst(F first){
       this.first = first;
   }

  /** sets the second element of this pair **/
   public void setSecond(S second){
     this.second = second;
   }

   /** the first element of this pair **/
   private F first;
   /** the second element of this pair **/
   private S second;
}
