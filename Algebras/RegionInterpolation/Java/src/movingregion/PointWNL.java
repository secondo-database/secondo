package movingregion;


import java.util.*;    
/**
     * This class is used to store one node in the graph structure.
     * the name is short for Point With Neighbour List.
     *
     * @author Erlend Tøssebro
     */
    public class PointWNL
    {    // Point with neighbour list
        public int x;             // x-coordinate of point
        public int y;             // y-coordinate of point
        public int t;             // temporal coordinate of point
        public Vector neighbours; // Index to neighbours in TriangleRep
        
        /**
         * Constructor without parameters. Initializes variables to default
         * values.
         */
        public PointWNL()
        {
            x=0;
            y=0;
            t=0;
            neighbours = new Vector();
        }
        
        /**
         * Constructor with parameters. Initializes variables to the given
         * values.
         *
         * @param px The X-coordinate of the point
         * @param py The Y-coordinate of the point
         * @param pt Can be either 0 or 1, and indicates to which snapshot the
         *           point belongs.
         */
        public PointWNL(int px, int py, int pt)
        {
            x=px;
            y=py;
            t=pt;
            neighbours = new Vector();
        }
        
        public int hashCode()
        {
            return((this.x * 165132 + this.y * 11231 + this.t * 54321) % 6654321);
        }
        
        public boolean equals(Object o)
        {
            if(o instanceof PointWNL)
            {
                PointWNL tmp=(PointWNL)o;
                return(tmp.x==x && tmp.y==y&&tmp.t==t);
            }
            else
            {
                return(false);
            }
        }
        
        public String toString()
        {
            return("("+x+";"+y+";"+t+")");
        }
    }
    
