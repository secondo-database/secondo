package viewer.hoese.algebras.pmregion;

import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;


/**
 *
 * @author Florian Heinz <fh@sysv.de>
 */
public class SegSet {
    private Map<Point, Set<Point>> m = new TreeMap();
    
    public void add (Seg s) {
        if (s == null)
            return;
        Set<Point> ps = m.get(s.s);
        if (ps == null) {
            ps = new HashSet();
            m.put(s.s, ps);
        }
        ps.add(s.e);
        ps = m.get(s.e);
        if (ps == null) {
            ps = new HashSet();
            m.put(s.e, ps);
        }
        ps.add(s.s);
    }
    
    public boolean isEmpty() {
        return m.isEmpty();
    }
    
    public int size() {
        return m.size();
    }
    
    public Seg[] get (Point s) {
        Set<Point> ps = m.get(s);
        if (ps == null)
            return null;
        Seg[] ret = new Seg[ps.size()];
        int i = 0;
        for (Point e : ps) {
            ret[i++] = new Seg(s, e);
        }
        
        return ret;
    }
    
    public Seg getSuccessor (Seg s) {
        Seg ret = null;
        Set<Point> ps = m.get(s.e);
        if (ps != null) {
            for (Point p : ps) {
                if (!p.equals(s.s)) {
                    ret = new Seg(s.e, p);
                    break;
                }
            }
        }
        
        if (ret != null)
            remove(ret);
        
        return ret;
    }
    
    public Seg getSomeSeg () {
        Point s = m.keySet().iterator().next();
        Point e = m.get(s).iterator().next();
        
        Seg ret = new Seg(s, e);
        remove(ret);
        return ret;
    }
    
    public void remove (Seg s) {
        Set<Point> ps = m.get(s.s);
        if (ps != null) {
            ps.remove(s.e);
            if (ps.isEmpty())
                m.remove(s.s);
        }
        ps = m.get(s.e);
        if (ps != null) {
            ps.remove(s.s);
            if (ps.isEmpty())
                m.remove(s.e);
        }
    }
}
