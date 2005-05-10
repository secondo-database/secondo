package twodsack.util.iterator;

import twodsack.util.collectiontype.*;

public interface ProIterator {

    public boolean hasNext();

    public Object next();

    public Entry nextEntry();
    
    public boolean hasPrevious();

    public Object previous();

    public int nextIndex();
    
    public int previousIndex();

    public void remove();

    public void set(Object o);

    public void addBefore(Object o);

    public void add(Object o);

    //public final void checkForComodification();

    public void reset();

}//end interface ProIterator