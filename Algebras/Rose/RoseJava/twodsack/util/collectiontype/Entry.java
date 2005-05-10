package twodsack.util.collectiontype;

public class Entry {
    //members
    public Object value;
    public Entry prev;
    public Entry next;

    //constructors
    public Entry (Object val) {
	this.value = val;
	this.prev = null;
	this.next = null;
    }
   
    //methods
    public void print() {
	System.out.println("\nEntry: value: "+this.value+", prev: "+this.prev+", next: "+this.next);
    }
    public Entry copy () {
	Entry copy = null;
	copy = new Entry(this.value);
	copy.prev = this.prev;
	copy.next = this.next;
	return copy;
    }


}//end class Entry