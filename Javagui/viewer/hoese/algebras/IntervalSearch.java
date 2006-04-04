
package viewer.hoese.algebras;

import viewer.hoese.Interval;
import java.util.Vector;

public class IntervalSearch{

/** computes the location of t in a sorted set of intervals .
  * @param time the search time
  * @param intervals a vector containing a sorted set of Interval instances
  * @return the index of the interval containing t or -1 if not found
  **/
public static int getTimeIndex(double time, Vector intervals){
    for(int i=0;i<intervals.size();i++){
         if( ((Interval) intervals.get(i)).isDefinedAt(time)){
            return i;
         }
    }
    return -1;
}

}
